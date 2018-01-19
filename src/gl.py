from OpenGL.GL import *
from ngui import *
import array
import ctypes
import time
import ngsolve

from . import glmath, shader
from .gui import ColorMapSettings, Qt, RangeGroup, CollColors

try:
    from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
    from PySide2.QtCore import Qt
except:
    from PyQt5 import QtCore, QtGui, QtWidgets, QtOpenGL
    from PyQt5.QtCore import Qt

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

    def __init__(self, filename, shader_type=None):
        import os, glob

        shaderpath = os.path.join(os.path.dirname(__file__), 'shader')
        fullpath = os.path.join(shaderpath, filename)
        self._code = open(fullpath,'r').read()

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
            self._type = shader_types[ext]
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

class CMeshData:
    def __init__(self, mesh):
        import weakref
        self.mesh = weakref.ref(mesh)
        self.ntrigs, trig_coordinates_data, trig_bary_coordinates_data, trig_element_number_data, trig_element_index_data, self.trig_max_index, self.min, self.max = GetFaceData(mesh)
        self.ntets, self.tet_max_index, tet_coordinates_data, tet_bary_coordinates_data, tet_element_number_data, tet_element_index_data = GetTetData(mesh)

        self.tet_bary_coordinates = ArrayBuffer()
        self.tet_bary_coordinates.store(tet_bary_coordinates_data)
        self.tet_coordinates = ArrayBuffer()
        self.tet_coordinates.store(tet_coordinates_data)
        self.tet_element_index = ArrayBuffer()
        self.tet_element_index.store(tet_element_index_data)
        self.tet_element_number = ArrayBuffer()
        self.tet_element_number.store(tet_element_number_data)
        self.trig_bary_coordinates = ArrayBuffer()
        self.trig_bary_coordinates.store(trig_bary_coordinates_data)
        self.trig_coordinates = ArrayBuffer()
        self.trig_coordinates.store(trig_coordinates_data)
        self.trig_element_index = ArrayBuffer()
        self.trig_element_index.store(trig_element_index_data)
        self.trig_element_number = ArrayBuffer()
        self.trig_element_number.store(trig_element_number_data)

        mesh._opengl_data = self

def MeshData(mesh):
    try:
        return mesh._opengl_data
    except:
        return CMeshData(mesh)



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

class SceneObject():
    action_counter = 1
    def __init__(self):
        self.actions = {}
        self.active_action = None
        self.timestamp = -1

    def getBoundingBox(self):
        raise RuntimeError("getBoundingBox not implemented for {}".format(type(self)))

    def getQtWidget(self, updateGL):
        widgets = {}
        self.actionCheckboxes = []

        class cbHolder:
            def __init__(self,cb,scene,name):
                self.scene = scene
                self.name = name
                self.cb = cb

            def __call__(self,state):
                if state:
                    self.scene.active_action = self.name
                    for cb in self.scene.actionCheckboxes:
                        if cb is not self.cb:
                            cb.setCheckState(QtCore.Qt.Unchecked)
                else:
                    if self.scene.active_action == self.name:
                        self.scene.active_action = None

        if self.actions:
            layout = QtWidgets.QVBoxLayout()
            for name,action in self.actions.items():
                cb = QtWidgets.QCheckBox(name)
                if self.active_action == name:
                    cb.setCheckState(QtCore.Qt.Checked)
                cb.stateChanged.connect(cbHolder(cb,self,name))
                self.actionCheckboxes.append(cb)
                layout.addWidget(cb)
            widget = QtWidgets.QWidget()
            widget.setLayout(layout)
            widgets["Actions"] = widget

        return widgets

    def addAction(self,action,name=None):
        if name is None:
            name = "Action" + str(action_counter)
            action_counter += 1
        self.actions[name] = action
        self.active_action = name

    def doubleClickAction(self,point):
        if self.active_action:
            self.actions[self.active_action](point)

class BaseMeshSceneObject(SceneObject):
    """Base class for all scenes that depend on a mesh"""
    def __init__(self, mesh):
        super().__init__()
        self.mesh = mesh

    def initGL(self):
        self.mesh_data = MeshData(self.mesh)

    def getBoundingBox(self):
        return self.mesh_data.min, self.mesh_data.max

class BaseFunctionSceneObject(BaseMeshSceneObject):
    """Base class for all scenes that depend on a coefficient function and a mesh"""
    def __init__(self, cf, mesh=None):
        if mesh==None:
            if not isinstance(cf, ngsolve.comp.GridFunction):
                raise RuntimeError("A mesh is needed if the given function is no GridFunction")
            mesh = cf.space.mesh

        super().__init__(mesh)

        self.colormap_min = -1
        self.colormap_max = 1
        self.colormap_linear = False


    def initGL(self):
        super().initGL()

    def setColorMapMin(self, value):
        self.colormap_min = value

    def setColorMapMax(self, value):
        self.colormap_max = value

    def setColorMapLinear(self, value):
        self.colormap_linear = value


    def getQtWidget(self, updateGL):

        settings = ColorMapSettings(min=-2, max=2, min_value=self.colormap_min, max_value=self.colormap_max)
        settings.layout().setAlignment(Qt.AlignTop)

        settings.minChanged.connect(self.setColorMapMin)
        settings.minChanged.connect(updateGL)

        settings.maxChanged.connect(self.setColorMapMax)
        settings.maxChanged.connect(updateGL)

        settings.linearChanged.connect(self.setColorMapLinear)
        settings.linearChanged.connect(updateGL)

        widgets = super().getQtWidget(updateGL)
        widgets["Colormap"] = settings
        return widgets

class ClippingPlaneScene(BaseFunctionSceneObject):
    def __init__(self, gf):
        super().__init__(gf)

        self.gl_initialized = False

        self.gf = gf

    def initGL(self):
        if self.gl_initialized:
            return

        super().initGL()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        Shader.includes['shader_functions'] = ngsolve.fem.GenerateShader(self.gf.space.globalorder)

        shaders = [
            Shader('solution.vert'),
            Shader('clipping.geom'),
            Shader('solution.frag')
        ]
        self.program = Program(shaders)
        glUseProgram(self.program.id)

        self.coefficients = glGenBuffers(1)
        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );

        tex = glGenTextures  (1)
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture  ( GL_TEXTURE_BUFFER, tex )
        glTexBuffer    ( GL_TEXTURE_BUFFER, GL_R32F, self.coefficients );

        attributes = self.program.attributes
        attributes.bind('vPos', self.mesh_data.tet_coordinates)
        attributes.bind('vLam', self.mesh_data.tet_bary_coordinates)
        attributes.bind('vElementNumber', self.mesh_data.tet_element_number)

        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );
        vec = ConvertCoefficients(self.gf)
        ncoefs = len(vec)
        size_float=ctypes.sizeof(ctypes.c_float)
        glBufferData   ( GL_TEXTURE_BUFFER, size_float*ncoefs, ctypes.c_void_p(), GL_DYNAMIC_DRAW ) # alloc

        self.gl_initialized = True
        glBindVertexArray(0)


    def update(self):
        self.initGL()
        glBindVertexArray(self.vao)
        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );
        vec = ConvertCoefficients(self.gf)
        ncoefs = len(vec)
        size_float=ctypes.sizeof(ctypes.c_float)

        glBufferSubData( GL_TEXTURE_BUFFER, 0, size_float*ncoefs, vec) # fill
        glBindVertexArray(0)


    def render(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection
        glUseProgram(self.program.id)
        glBindVertexArray(self.vao)

        uniforms = self.program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('colormap_min', self.colormap_min)
        uniforms.set('colormap_max', self.colormap_max)
        uniforms.set('colormap_linear', self.colormap_linear)
        uniforms.set('clipping_plane_deformation', False)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', False);

        if(self.mesh.dim==2):
            uniforms.set('element_type', 10)
        if(self.mesh.dim==3):
            uniforms.set('element_type', 20)

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_LINES_ADJACENCY, 0, 4*self.mesh_data.ntets)
        glBindVertexArray(0)



class MeshScene(BaseMeshSceneObject):
    def __init__(self, mesh):
        super().__init__(mesh)

        self.qtWidget = None
        self.gl_initialized = False

    def initGL(self):
        if self.gl_initialized:
            return

        super().initGL()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        shaders = [
            Shader('mesh.vert'),
            Shader('mesh.frag')
        ]
        self.program = Program(shaders)

        self.gl_initialized = True
        glBindVertexArray(0)

    def update(self):
        self.initGL()
        glBindVertexArray(self.vao)
        self.index_colors = [0, 255, 0, 255] * (self.mesh_data.trig_max_index+1)

        attributes = self.program.attributes
        attributes.bind('pos', self.mesh_data.trig_coordinates)
        attributes.bind('index', self.mesh_data.trig_element_index)

        self.tex_index_color = glGenTextures(1)

        glBindTexture(GL_TEXTURE_1D, self.tex_index_color)

        # copy texture
        glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, self.mesh_data.trig_max_index+1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes(self.index_colors))

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glBindVertexArray(0)

    def setupRender(self, settings):
        glUseProgram(self.program.id)

        model, view, projection = settings.model, settings.view, settings.projection
        uniforms = self.program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        glBindTexture(GL_TEXTURE_1D, self.tex_index_color)

    def render(self, settings):
        glBindVertexArray(self.vao)
        self.setupRender(settings)

        uniforms = self.program.uniforms
        uniforms.set('fColor', [0,1,0,0] )
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('use_index_color', True)
        uniforms.set('do_clipping', self.mesh.dim==3);

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh_data.ntrigs)

        self.renderWireframe(settings)

    def renderWireframe(self, settings):
        glBindVertexArray(self.vao)
        self.setupRender(settings)

        uniforms = self.program.uniforms
        uniforms.set('fColor', [0,0,0,1] )
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('use_index_color', False)
        uniforms.set('do_clipping', self.mesh.dim==3);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh_data.ntrigs)


    def updateIndexColors(self):
        colors = []
        for c in self.bccolors.getColors():
            colors.append(c.red())
            colors.append(c.green())
            colors.append(c.blue())
            colors.append(c.alpha())
        self.index_colors = colors
        glBindTexture(GL_TEXTURE_1D, self.tex_index_color)
        glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, self.mesh_data.trig_max_index+1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes(self.index_colors))

    def getQtWidget(self, updateGL):
        if self.qtWidget!=None:
            return self.qtWidget

        self.bccolors = CollColors(self.mesh.GetBoundaries())
        self.bccolors.colors_changed.connect(self.updateIndexColors)
        self.bccolors.colors_changed.connect(updateGL)
        self.updateIndexColors()

        widgets = super().getQtWidget(updateGL)
        widgets["BCColors"] = self.bccolors
        return widgets


class MeshElementsScene(BaseMeshSceneObject):
    def __init__(self, mesh):
        super().__init__(mesh)

        self.qtWidget = None
        self.gl_initialized = False
        self.shrink = 1.0

    def initGL(self):
        if self.gl_initialized:
            return

        super().initGL()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        shaders = [
            Shader('elements.vert'),
            Shader('elements.geom'),
            Shader('elements.frag')
        ]
        self.program = Program(shaders)

        self.gl_initialized = True
        glBindVertexArray(0)

    def update(self):
        self.initGL()
        glBindVertexArray(self.vao)
        self.mat_colors = [0,0,255,255] * (self.mesh_data.tet_max_index+1)

        attributes = self.program.attributes
        attributes.bind('pos', self.mesh_data.tet_coordinates)
        attributes.bind('index', self.mesh_data.tet_element_index)

        self.tex_mat_color = glGenTextures(1)
        glBindTexture(GL_TEXTURE_1D,self.tex_mat_color)

        # copy texture
        glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, (self.mesh_data.tet_max_index+1), 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes(self.mat_colors))

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)

        glBindVertexArray(0)

    def render(self, settings):
        glBindVertexArray(self.vao)
        glUseProgram(self.program.id)

        model, view, projection = settings.model, settings.view, settings.projection

        uniforms = self.program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('shrink_elements', self.shrink)
        uniforms.set('clipping_plane', settings.clipping_plane)

        glBindTexture(GL_TEXTURE_1D,self.tex_mat_color)

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_LINES_ADJACENCY, 0, 4*self.mesh_data.ntets)

    def setShrink(self, value):
        self.shrink = value

    def updateMatColors(self):
        colors = []
        for c in self.matcolors.getColors():
            colors.append(c.red())
            colors.append(c.green())
            colors.append(c.blue())
            colors.append(c.alpha())
        self.mat_colors = colors
        glBindTexture(GL_TEXTURE_1D, self.tex_mat_color)
        glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA, (self.mesh_data.tet_max_index + 1), 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes(self.mat_colors))

    def getQtWidget(self, updateGL):
        shrink = RangeGroup("Shrink", min=0.0, max=1.0, value=1.0)
        shrink.valueChanged.connect(self.setShrink)
        shrink.valueChanged.connect(updateGL)
        self.matcolors = CollColors(self.mesh.GetMaterials(),initial_color=(0,0,255,255))
        self.matcolors.colors_changed.connect(self.updateMatColors)
        self.matcolors.colors_changed.connect(updateGL)
        self.updateMatColors()
        widgets = super().getQtWidget(updateGL)
        widgets["Shrink"] = shrink
        widgets["MatColors"] = self.matcolors
        return widgets


class SolutionScene(BaseFunctionSceneObject):
    def __init__(self, gf):
        super().__init__(gf)

        self.qtWidget = None
        self.gl_initialized = False

        self.gf = gf

    def initGL(self):
        if self.gl_initialized:
            return

        super().initGL()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        Shader.includes['shader_functions'] = ngsolve.fem.GenerateShader(self.gf.space.globalorder)

        shaders = [
            Shader('solution.vert'),
            Shader('solution.frag')
        ]
        self.program = Program(shaders)

        self.coefficients = glGenBuffers(1)
        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );

        tex = glGenTextures  (1)
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture  ( GL_TEXTURE_BUFFER, tex )
        glTexBuffer    ( GL_TEXTURE_BUFFER, GL_R32F, self.coefficients );

        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );
        vec = self.gf.vec
        ncoefs = len(vec)
        size_float=ctypes.sizeof(ctypes.c_float)

        glBufferData   ( GL_TEXTURE_BUFFER, size_float*ncoefs, ctypes.c_void_p(), GL_DYNAMIC_DRAW ) # alloc

        glBindVertexArray(self.vao)

        attributes = self.program.attributes
        attributes.bind('vPos', self.mesh_data.trig_coordinates)
        attributes.bind('vLam', self.mesh_data.trig_bary_coordinates)
        attributes.bind('vElementNumber', self.mesh_data.trig_element_number)

        glBindVertexArray(0)

        self.mesh_scene = MeshScene(self.mesh)
        self.mesh_scene.update()


    def update(self):
        self.initGL()
        # Todo: assumes the mesh is unchanged, also update mesh-related data if necessary (timestamps!)
        glBindVertexArray(self.vao)
        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );
        vec = self.gf.vec
        ncoefs = len(vec)
        size_float=ctypes.sizeof(ctypes.c_float)

        glBufferSubData( GL_TEXTURE_BUFFER, 0, size_float*ncoefs, (ctypes.c_float*ncoefs)(*vec)) # fill
        glBindVertexArray(0)


    def render(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection
        glBindVertexArray(self.vao)
        glUseProgram(self.program.id)

        uniforms = self.program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        uniforms.set('colormap_min', self.colormap_min)
        uniforms.set('colormap_max', self.colormap_max)
        uniforms.set('colormap_linear', self.colormap_linear)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', self.mesh.dim==3);

        if(self.mesh.dim==2):
            uniforms.set('element_type', 10)
        if(self.mesh.dim==3):
            uniforms.set('element_type', 20)

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh_data.ntrigs);

        self.mesh_scene.renderWireframe(settings)

