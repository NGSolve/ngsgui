from OpenGL.GL import *
from ngui import *
import array
import ctypes
import time
import ngsolve

from . import glmath, shader

from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
from PySide2.QtCore import Qt

class GLObject:
    @property
    def id(self):
        return self._id

class Shader(GLObject):
    # map to fake 'include' directives like in C
    # Used for instance for generated code to evaluate shape functions:
    # {include shader_functions}
    # to make this work, set Shader.includes['shader_functions'] to the desired code before creating the Shader object

    includes = {}

    def __init__(self, filename=None, string=None, shader_type=None):
        import os, glob

        shaderpath = os.path.join(os.path.dirname(__file__), 'shader')
        if filename:
            fullpath = os.path.join(shaderpath, filename)
            self._code = open(fullpath,'r').read()
        if string:
            fullpath = ""
            self._code = string

        for incfile in glob.glob(os.path.join(shaderpath, '*.inc')):
            Shader.includes[os.path.basename(incfile)] = open(incfile,'r').read()

        for token in Shader.includes:
            self._code = self._code.replace('{include '+token+'}', Shader.includes[token])

        shader_types = {
                'vert': GL_VERTEX_SHADER,
                'frag': GL_FRAGMENT_SHADER,
                'geom': GL_GEOMETRY_SHADER
                }
        if shader_type == None:
            ext = filename.split('.')[-1]
            if not ext in shader_types:
                raise RuntimeError('Unknown shader file extension: '+ext)
            shader_type = shader_types[ext]
        self._type = shader_type
        self._id = glCreateShader(self._type)

        glShaderSource(self.id, self._code)
        glCompileShader(self.id)

        if glGetShaderiv(self.id, GL_COMPILE_STATUS) != GL_TRUE:
            raise RuntimeError('Error when compiling ' + fullpath + ': '+glGetShaderInfoLog(self.id).decode()+'\ncompiled code:\n'+self._code)

class Program(GLObject):
    class Uniforms:
        def __init__(self, pid):
            self.__dict__['id'] = pid
            num_uniforms = glGetProgramiv(self.id, GL_ACTIVE_UNIFORMS);
            uniforms = {}
            for i in range(num_uniforms):
                name,dummy,type_ = glGetActiveUniform(self.id, i)
                loc = glGetUniformLocation(self.id, name)
                uniforms[name] = (loc,type_)
            self.__dict__['uniforms'] = uniforms

        def check(self, name):
            name = name.encode('ascii','ignore')
            if not name in self.uniforms:
                raise RuntimeError("Unknown uniform name {}, allowed values:".format(name)+str(list(self.uniforms.keys())))
            return name

        def __getitem__(self, name):
            name = self.check(name)
            return self.uniforms[name][0]

        def set(self, name, value):
            name = self.check(name)
            loc, type_ = self.uniforms[name]
            convert_matrix = lambda m,size: (ctypes.c_float*(size**2))(*[m[j,i] for i in range(size) for j in range(size)])
            functions = {
                    GL_BOOL:              lambda v: glUniform1i(loc, v),
                    GL_BOOL_VEC2:         lambda v: glUniform2i(loc, *v),
                    GL_BOOL_VEC3:         lambda v: glUniform3i(loc, *v),
                    GL_BOOL_VEC4:         lambda v: glUniform4i(loc, *v),
                    GL_INT:               lambda v: glUniform1i(loc, v),
                    GL_INT_VEC2:          lambda v: glUniform2i(loc, *v),
                    GL_INT_VEC3:          lambda v: glUniform3i(loc, *v),
                    GL_INT_VEC4:          lambda v: glUniform4i(loc, *v),
                    GL_UNSIGNED_INT:      lambda v: glUniform1ui(loc, v),
                    GL_UNSIGNED_INT_VEC2: lambda v: glUniform2ui(loc, *v),
                    GL_UNSIGNED_INT_VEC3: lambda v: glUniform3ui(loc, *v),
                    GL_UNSIGNED_INT_VEC4: lambda v: glUniform4ui(loc, *v),
                    GL_FLOAT:             lambda v: glUniform1f(loc, v),
                    GL_FLOAT_VEC2:        lambda v: glUniform2f(loc, *v),
                    GL_FLOAT_VEC3:        lambda v: glUniform3f(loc, *v),
                    GL_FLOAT_VEC4:        lambda v: glUniform4f(loc, *v),
                    GL_FLOAT_MAT2:        lambda v: glUniformMatrix2fv(loc, 1, GL_FALSE, convert_matrix(v,2)),
                    GL_FLOAT_MAT3:        lambda v: glUniformMatrix3fv(loc, 1, GL_FALSE, convert_matrix(v,3)),
                    GL_FLOAT_MAT4:        lambda v: glUniformMatrix4fv(loc, 1, GL_FALSE, convert_matrix(v,4)),
                    }
            if type_ not in functions:
                raise RuntimeError("Unknown type " + str(type_)+'=hex({})'.format(hex(type_)))
            return functions[type_](value)

        def __contains__(self, name):
            return name.encode('ascii', 'ignore') in self.uniforms

    class Attributes:
        def __init__(self, pid):
            self.id = pid
            attributes = {}
            num_attributes = glGetProgramiv(self.id, GL_ACTIVE_ATTRIBUTES);

            for i in range(num_attributes):
                bufSize = glGetProgramiv(self.id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)
                length = GLsizei()
                size = GLint()
                type_ = GLenum()
                name = (GLchar * bufSize)()
                glGetActiveAttrib(self.id, i, bufSize, length, size, type_, name)
                loc = glGetAttribLocation(self.id, name.value)
                attributes[name.value] = (loc, type_.value, size.value)

            self.attributes = attributes

        def check(self, name):
            name = name.encode('ascii','ignore')
            if not name in self.attributes:
                raise RuntimeError("Unknown attribute name {}, allowed values:".format(name)+str(list(self.attributes.keys())))
            return name

        def bind(self, name, vbo, size=None, stride=0):
            name = self.check(name)

            loc, type_, size_ = self.attributes[name]

            if size==None:
                size = size_

            vbo.bind()
            glEnableVertexAttribArray(loc)
            null = ctypes.c_void_p()
            if type_ == GL_INT:
                glVertexAttribIPointer(loc,1,GL_INT,stride,null)
            if type_ == GL_INT_VEC2:
                glVertexAttribIPointer(loc,2,GL_INT,stride,null)
            if type_ == GL_INT_VEC3:
                glVertexAttribIPointer(loc,3,GL_INT,stride,null)
            if type_ == GL_INT_VEC4:
                glVertexAttribIPointer(loc,4,GL_INT,stride,null)
            if type_ == GL_UNSIGNED_INT:
                glVertexAttribIPointer(loc,1,GL_UNSIGNED_INT,stride,null)
            if type_ == GL_UNSIGNED_INT_VEC2:
                glVertexAttribIPointer(loc,2,GL_UNSIGNED_INT,stride,null)
            if type_ == GL_UNSIGNED_INT_VEC3:
                glVertexAttribIPointer(loc,3,GL_UNSIGNED_INT,stride,null)
            if type_ == GL_UNSIGNED_INT_VEC4:
                glVertexAttribIPointer(loc,4,GL_UNSIGNED_INT,stride,null)
            if type_ == GL_FLOAT:
                glVertexAttribPointer(loc,1,GL_FLOAT,GL_FALSE,stride,null)
            if type_ == GL_FLOAT_VEC2:
                glVertexAttribPointer(loc,2,GL_FLOAT,GL_FALSE,stride,null)
            if type_ == GL_FLOAT_VEC3:
                glVertexAttribPointer(loc,3,GL_FLOAT,GL_FALSE,stride,null)
            if type_ == GL_FLOAT_VEC4:
                glVertexAttribPointer(loc,4,GL_FLOAT,GL_FALSE,stride,null)

            glEnableVertexAttribArray(0)

        def __getitem__(self, name):
            name = self.check(name)
            return self.attributes[name][0]

        def __contains__(self, name):
            return name.encode('ascii', 'ignore') in self.attributes

    def __init__(self, shaders):
        self.locations = {}
        self._shaders = shaders

        self._id = glCreateProgram()
        for shader in shaders:
            glAttachShader(self.id, shader.id)

        glLinkProgram(self.id)
        if glGetProgramiv(self.id, GL_LINK_STATUS) != GL_TRUE:
                raise RuntimeError(glGetProgramInfoLog(self.id))

        self.uniforms = Program.Uniforms(self.id)
        self.attributes = Program.Attributes(self.id)


class ArrayBuffer(GLObject):
    def __init__(self, buffer_type=GL_ARRAY_BUFFER, usage=GL_STATIC_DRAW):
        self._type = buffer_type
        self._usage = usage
        self._id = glGenBuffers(1)

    def bind(self):
        glBindBuffer(self._type, self.id)

    def store(self, data):
        self.bind()
        glBufferData(self._type, data, self._usage)

class Texture(GLObject):
    def __init__(self, buffer_type, format, unit=GL_TEXTURE0 ):
        self._type = buffer_type
        self._format = format
        self._unit = unit
        self._id = glGenTextures(1)

        if self._type == GL_TEXTURE_BUFFER:
            self._buffer = ArrayBuffer( GL_TEXTURE_BUFFER, GL_DYNAMIC_DRAW )
            self.bind()
            glTexBuffer ( GL_TEXTURE_BUFFER, GL_R32F, self._buffer.id );

    def bind(self):
        glActiveTexture( self._unit );
        glBindTexture( self._type, self.id )
        if self._type == GL_TEXTURE_BUFFER:
            self._buffer.bind()

    def store(self, data, data_format=None, width=0, height=0):
        self.bind()
        if self._type == GL_TEXTURE_1D:
            glTexImage1D(GL_TEXTURE_1D, 0, self._format, width, 0, self._format, data_format, data)
        if self._type == GL_TEXTURE_2D:
            glTexImage2D( GL_TEXTURE_2D, 0, self._format, width, height, 0, self._format, data_format, data )
        if self._type == GL_TEXTURE_BUFFER:
            data_size = ctypes.sizeof(ctypes.c_float)*len(data)
            glBufferData ( GL_TEXTURE_BUFFER, data_size, ctypes.c_void_p(), GL_DYNAMIC_DRAW ) # alloc
            glBufferSubData( GL_TEXTURE_BUFFER, 0, data_size, data) # fill

