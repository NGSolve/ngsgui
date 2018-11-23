from OpenGL.GL import *
from OpenGL import GL, constant

from ngsgui import _debug

from OpenGL.GL.ARB import debug_output
from OpenGL.extensions import alternate
import ctypes, ngsolve, numpy, cmath, math

import qtpy
from qtpy import QtCore, QtGui

from ngsgui.shader import locations as shaderpaths

_DEVELOP=True

glGetDebugMessageLog = alternate(
    'glGetDebugMessageLog', GL.glGetDebugMessageLog,
    debug_output.glGetDebugMessageLogARB)

# the size specifications of some debug_output tokens seem to be missing so
# we add them manually even though it does not seem to be a nice way to do it
# GL.glget.GL_GET_SIZES[debug_output.GL_MAX_DEBUG_MESSAGE_LENGTH_ARB] = (1,)
# GL.glget.GL_GET_SIZES[debug_output.GL_DEBUG_LOGGED_MESSAGES_ARB] = (1,)


def get_constant(value, namespace):
    """Get symbolic constant.

    value - the (integer) token value to look for
    namespace - the module object to search in

    """

    for var in dir(namespace):
        attr = getattr(namespace, var)
        if isinstance(attr, constant.Constant) and attr == value:
            return var
    return value


def check_debug_output():
    if not _debug:
        return
    """Get the contents of the OpenGL debug log as a string."""

    # details for the available log messages
    msgmaxlen = GL.glGetInteger(debug_output.GL_MAX_DEBUG_MESSAGE_LENGTH_ARB)
    msgcount = GL.glGetInteger(debug_output.GL_DEBUG_LOGGED_MESSAGES_ARB)

    # ctypes arrays to receive the log data
    msgsources = (ctypes.c_uint32 * msgcount)()
    msgtypes = (ctypes.c_uint32 * msgcount)()
    msgids = (ctypes.c_uint32 * msgcount)()
    msgseverities = (ctypes.c_uint32 * msgcount)()
    msglengths = (ctypes.c_uint32 * msgcount)()
    msglog = (ctypes.c_char * (msgmaxlen * msgcount))()

    glGetDebugMessageLog(msgcount, msgmaxlen, msgsources, msgtypes, msgids,
                         msgseverities, msglengths, msglog)

    offset = 0
    logdata = zip(msgsources, msgtypes, msgids, msgseverities, msglengths)
    for msgsource, msgtype, msgid, msgseverity, msglen in logdata:
        msgtext = msglog.raw[offset:offset + msglen].decode("ASCII")
        offset += msglen
        msgsource = get_constant(msgsource, debug_output)
        msgtype = get_constant(msgtype, debug_output)
        msgseverity = get_constant(msgseverity, debug_output)
        print("SOURCE: {0}\nTYPE: {1}\nID: {2}\nSEVERITY: {3}\n"
               "MESSAGE: {4}".format(msgsource, msgtype, msgid,
               msgseverity, msgtext))

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

    @staticmethod
    def preloadShaderIncludes():
        import glob, os
        from . import shader
        for shaderpath in shader.locations:
            for incfile in glob.glob(os.path.join(shaderpath, '*.inc')):
                Shader.includes[os.path.basename(incfile)] = open(incfile,'r').read()

    def __init__(self, code=None, filename=None, shader_type=None, **replacements):

        if code == None:
            code = readShaderFile(filename, **replacements)
        self._code = code
        shader_types = {
                'vert': GL_VERTEX_SHADER,
                'frag': GL_FRAGMENT_SHADER,
                'tese': GL_TESS_EVALUATION_SHADER,
                'tesc': GL_TESS_CONTROL_SHADER,
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
#             numerated_shader_code = ""
#             for i,line in enumerate(self._code.split('\n')):
#                 numerated_shader_code += str(i)+":\t"+line+'\n'
            raise RuntimeError('Error when compiling ' + filename + ': '+glGetShaderInfoLog(self.id).decode())

def readShaderFile(filename, defines):
    import os
    for shaderpath in shaderpaths:
        fullpath = os.path.join(shaderpath, filename)
        if os.path.exists(fullpath):
            break
    else:
        raise FileNotFoundError("Couldn't find shader file: " + filename)

    code = open(fullpath,'r').read()

    while code.find('{include ')>-1:
        start = code.find('{include ')+9
        end = code.find('}',start)
        token = code[start:end].strip()
        if token not in Shader.includes:
            raise Exception("Can't find include file " + token)
        code = code.replace('{include '+token+'}', Shader.includes[token])

    pos = code.find('\n', code.find('version'))
    code = code[:pos+1] + defines + code[pos+1:]

    return code

def getProgramHash(filenames, defines):
    import hashlib
    res = ""
    h = hashlib.sha256()
    h.update(glGetString(GL_VENDOR))
    h.update(glGetString(GL_VERSION))
    h.update(glGetString(GL_RENDERER))
    for filename in sorted(filenames):
        h.update((filename + readShaderFile(filename, defines)).encode('ascii'))
    return h.hexdigest()

def compileProgram(filenames, defines, feedback=[]):
    shaders = []
    for f in filenames:
        code = readShaderFile(f, defines)
        shaders.append(Shader(code, f))

    return Program(shaders, feedback=feedback)

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

        def __contains__(self, name):
            name = name.encode('ascii','ignore')
            return name in self.uniforms

        def __getitem__(self, name):
            name = self.check(name)
            return self.uniforms[name][0]

        def set(self, name, value):
            try:
                name = self.check(name)
            except Exception as e:
                if _DEVELOP:
                    # skip error on undefined (or optimized out) uniforms in develop mode
                    return
                else:
                    raise e

            loc, type_ = self.uniforms[name]
            convert_matrix = lambda m,size: (ctypes.c_float*(size**2))(*[m[j,i] for i in range(size) for j in range(size)])
            functions = {
                    GL_SAMPLER_1D:        lambda v: glUniform1i(loc, v),
                    GL_SAMPLER_2D:        lambda v: glUniform1i(loc, v),
                    GL_SAMPLER_3D:        lambda v: glUniform1i(loc,v),
                    GL_INT_SAMPLER_3D:    lambda v: glUniform1i(loc,v),
                    GL_UNSIGNED_INT_SAMPLER_3D: lambda v: glUniform1i(loc,v),
                    GL_INT_SAMPLER_BUFFER:lambda v: glUniform1i(loc, v),
                    GL_SAMPLER_BUFFER:    lambda v: glUniform1i(loc, v),
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
                    GL_DOUBLE:            lambda v: glUniform1d(loc, v),
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

        def bind(self, name, vbo, size=None, stride=0, offset=0):
            try:
                name = self.check(name)
            except Exception as e:
                if _DEVELOP:
                    # skip error on undefined (or optimized out) uniforms in develop mode
                    return
                else:
                    raise e

            loc, type_, size_ = self.attributes[name]

            if size==None:
                size = size_
            p = ctypes.c_void_p(offset)
            vbo.bind()
            glEnableVertexAttribArray(loc)
            if type_ == GL_INT:
                glVertexAttribIPointer(loc,1,GL_INT,stride,p)
            elif type_ == GL_INT_VEC2:
                glVertexAttribIPointer(loc,2,GL_INT,stride,p)
            elif type_ == GL_INT_VEC3:
                glVertexAttribIPointer(loc,3,GL_INT,stride,p)
            elif type_ == GL_INT_VEC4:
                glVertexAttribIPointer(loc,4,GL_INT,stride,p)
            elif type_ == GL_UNSIGNED_INT:
                glVertexAttribIPointer(loc,1,GL_UNSIGNED_INT,stride,p)
            elif type_ == GL_UNSIGNED_INT_VEC2:
                glVertexAttribIPointer(loc,2,GL_UNSIGNED_INT,stride,p)
            elif type_ == GL_UNSIGNED_INT_VEC3:
                glVertexAttribIPointer(loc,3,GL_UNSIGNED_INT,stride,p)
            elif type_ == GL_UNSIGNED_INT_VEC4:
                glVertexAttribIPointer(loc,4,GL_UNSIGNED_INT,stride,p)
            elif type_ == GL_FLOAT:
                glVertexAttribPointer(loc,1,GL_FLOAT,GL_FALSE,stride,p)
            elif type_ == GL_FLOAT_VEC2:
                glVertexAttribPointer(loc,2,GL_FLOAT,GL_FALSE,stride,p)
            elif type_ == GL_FLOAT_VEC3:
                glVertexAttribPointer(loc,3,GL_FLOAT,GL_FALSE,stride,p)
            elif type_ == GL_FLOAT_VEC4:
                glVertexAttribPointer(loc,4,GL_FLOAT,GL_FALSE,stride,p)
            else:
                raise RuntimeError("Unknown attribute type", name, type_)

            glEnableVertexAttribArray(0)

        def __getitem__(self, name):
            name = self.check(name)
            return self.attributes[name][0]

        def __contains__(self, name):
            return name.encode('ascii', 'ignore') in self.attributes

    def __init__(self, shaders=[], feedback=[], binary=None):
        self.locations = {}
        self._shaders = shaders
        self._id = glCreateProgram()

        if binary:
            from OpenGL.arrays import GLbyteArray
            p,format = binary
            glProgramBinary( self.id, format, p, len(p))
        else:
            for shader in shaders:
                glAttachShader(self.id, shader.id)

            if len(feedback):

                feedback = [ctypes.create_string_buffer(name.encode('ascii', 'ignore')) for name in feedback]

                LP_c_char = ctypes.POINTER(ctypes.c_char)
                LP_LP_c_char = ctypes.POINTER(LP_c_char)

                buff = (LP_c_char * (len(feedback)))()
                for i, arg in enumerate(feedback):
                    buff[i] = arg

                glTransformFeedbackVaryings(self.id, len(feedback), buff, GL_INTERLEAVED_ATTRIBS)
            glLinkProgram(self.id)
            if glGetProgramiv( self.id, GL_LINK_STATUS ) != GL_TRUE:
                log = glGetProgramInfoLog( self.id )
                print(log)

        glValidateProgram( self.id )
        if glGetProgramiv( self.id, GL_VALIDATE_STATUS ) != GL_TRUE:
            log = glGetProgramInfoLog( self.id )
            print(log)

        if glGetProgramiv(self.id, GL_LINK_STATUS) != GL_TRUE:
                print(glGetProgramInfoLog(self.id))

        self.uniforms = Program.Uniforms(self.id)
        self.attributes = Program.Attributes(self.id)

    def setFunction(self, scene, elements, cf=None, values=None, index=0):
        cf = cf or scene.cf
        values = values or scene.values[ngsolve.VOL]
        tex_base = 2+2*index
        name = 'functions['+str(index)+']'
        u = self.uniforms
        u.set(name+'.subdivision', 2**scene.getSubdivision()-1)
        u.set(name+'.is_complex', cf.is_complex)
        glActiveTexture(GL_TEXTURE0+tex_base)
        values['real'][elements.type, elements.curved].bind()
        u.set(name+'.coefficients', tex_base)
        u.set(name+'.component', scene.getComponent() if cf.dim>1 else 0)

        if cf.is_complex:
            glActiveTexture(GL_TEXTURE0+tex_base+1)
            values['imag'][elements.type, elements.curved].bind()
            u.set(name+'.coefficients_imag', tex_base+1)
            u.set(name+'.complex_vis_function', scene._complex_eval_funcs[scene.getComplexEvalFunc()])
            w = cmath.exp(1j*scene.getComplexPhaseShift()/180.0*math.pi)
            u.set(name+'.complex_factor', [w.real, w.imag])

def getProgram(*shader_files, feedback=[], elements=None, params=None, scene=None, **define_flags):
    import base64, zlib
    check_debug_output()
    cache = getProgram._cache

    defines = '\n'
    if elements != None:
        defines += """
#define ELEMENT_DIM {DIM}
#define ELEMENT_TYPE {ELEMENT_TYPE}
#define {ELEMENT_TYPE_NAME}
#define ELEMENT_SIZE {ELEMENT_SIZE}
#define ELEMENT_N_VERTICES {ELEMENT_N_VERTICES}
""".format(
        ELEMENT_TYPE = str(elements.type)[3:],
        ELEMENT_SIZE = str(elements.size),
        ELEMENT_N_VERTICES = elements.nverts,
        DIM = elements.dim,
        ELEMENT_TYPE_NAME = str(elements.type).replace('.','_')
        )
        if elements.curved:
            defines += "#define CURVED\n"
    import sys
    if 'darwin' in sys.platform:
        define_flags['MACOS'] = 1
    if scene and scene.getLightDisable():
        define_flags['NOLIGHT'] = 1
    do_clipping = (hasattr(scene, 'getClippingEnable') and scene.getClippingEnable()) or 'CLIPPING' in define_flags
    do_clipping = do_clipping and scene.getClippingNPlanes()>0
    if do_clipping:
        define_flags['CLIPPING'] = 1
        define_flags['N_CLIPPING_PLANES'] = scene.getClippingNPlanes()
        define_flags['CLIPPING_EXPRESSION'] = scene.getClippingExpression()

    if scene and hasattr(scene,'getOrder'):
        define_flags['ORDER'] = scene.getOrder()

    for d in define_flags:
        flag = define_flags[d]
        if flag != None:
            if type(flag)==bool:
                flag = int(flag)
            defines += "#define {} {}\n".format(d, str(flag))
    key = str(tuple([tuple(sorted(shader_files))]+feedback+[defines]))
    key = key.replace('(','').replace(')','').replace(',','-').replace("'","").replace(' ','')

    if key in cache:
        prog = cache[key]
    else:
        # try to find on-disk cached shader
        settings = getProgram._settings
        h = getProgramHash(shader_files, defines=defines)
        if str(h) == settings.value(key+'/hash'):
            # load program from binary blob
            enc = settings.value(key+'/program')
            binary = numpy.frombuffer(base64.b64decode(enc), dtype=numpy.int8)
            binary = zlib.decompress(binary)
            format = int(settings.value(key+'/format'))
            prog = Program(binary=(binary,format))
            cache[key] = prog
        else:
            # no cached version - recompile shader 
            prog = compileProgram(shader_files, defines=defines, feedback=feedback)
            cache[key] = prog

            # get binary blob and store it on disk
            size = glGetProgramiv( prog.id, GL_PROGRAM_BINARY_LENGTH )
            if size:
                result = numpy.zeros(size,dtype=numpy.uint8)
                size2 = GLint()
                format = GLenum()
                res = glGetProgramBinary( prog.id, size, size2, format, result )
                s = len(result)
                result = zlib.compress(result)
                enc = base64.b64encode(result).decode('ascii')
                settings.setValue(key+'/format', str(format.value))
                settings.setValue(key+'/program', enc)
                settings.setValue(key+'/hash', str(h))

    glUseProgram(prog.id)
    u = prog.uniforms
    print('uniforms', u.uniforms)
    if scene != None:
        if 'P' in u:
            u.set('P',scene.projection)
        if 'MV' in u:
            u.set('MV',scene.view*scene.model)
        if 'light.diffuse' in u:
            u.set('light.ambient', scene.getLightAmbient())
            u.set('light.diffuse', scene.getLightDiffuse())
            u.set('light.dir', [1.,3.,3.])
            u.set('light.spec', scene.getLightSpecular())
            u.set('light.shininess', scene.getLightShininess())
        if do_clipping and 'clipping_planes[0]' in u:
            n = scene.getClippingNPlanes()
            planes = scene.getClippingPlanes()
            planes = (ctypes.c_float*(4*n))(*planes)
            glUniform4fv(u['clipping_planes[0]'], n, planes)

        if 'colormap.colors' in u and scene:
            u.set('colormap.n', scene.getColormapSteps())
            u.set('colormap.min', scene.getColormapMin())
            u.set('colormap.max', scene.getColormapMax())
            u.set('colormap.linear', scene.getColormapLinear())
            glActiveTexture(GL_TEXTURE6)
            scene.getColormapTex().bind()
            u.set('colormap.colors', 6)

    if elements != None:
        u.set('element_type', int(elements.type))
        glActiveTexture(GL_TEXTURE0)
        elements.tex_vertices.bind()
        u.set('mesh.vertices', 0)
        u.set('mesh.dim', elements.dim);
        if elements.dim>0:
            glActiveTexture(GL_TEXTURE1)
            elements.tex.bind()
            u.set('mesh.elements', 1)

    check_debug_output()

    return prog

getProgram._cache = {}
getProgram._settings = QtCore.QSettings('ngsolve','shaders')

class VertexArray(GLObject):
    def __init__(self):
        self._id = glGenVertexArrays(1)

    def __enter__(self):
        self.__prev_id = glGetInteger(GL_VERTEX_ARRAY_BINDING)
        glBindVertexArray(self.id)
        return self

    def __exit__(self, *args):
        glBindVertexArray(self.__prev_id)

    def bind(self):
        glBindVertexArray(self.id)

    def unbind(self):
        glBindVertexArray(0)


class ArrayBuffer(GLObject):
    def __init__(self, buffer_type=GL_ARRAY_BUFFER, usage=GL_STATIC_DRAW):
        self._type = buffer_type
        self._usage = usage
        self._id = glGenBuffers(1)

    def bind(self):
        glBindBuffer(self._type, self.id)

    def store(self, data, size=None):
        self.bind()
        glBufferData(self._type, size if size else len(data) * ctypes.sizeof(ctypes.c_float), data, self._usage)

class Texture(GLObject):
    def __init__(self, buffer_type, format, format2=None):
        if isinstance(format, ngsolve.CoefficientFunction):
            formats = [None, GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F];
            format = formats[format.dim]
        self._type = buffer_type
        self._format = format
        if format2 is None:
            self._format2 = self._format
        else:
            self._format2 = format2
        self._id = glGenTextures(1)

        if self._type == GL_TEXTURE_BUFFER:
            self._buffer = ArrayBuffer( GL_TEXTURE_BUFFER, GL_DYNAMIC_DRAW )
            self.bind()
            glTexBuffer ( GL_TEXTURE_BUFFER, format, self._buffer.id );
        else:
            self.bind()
            glTexParameteri( self._type, GL_TEXTURE_MAG_FILTER, GL_NEAREST )
            glTexParameteri( self._type, GL_TEXTURE_MIN_FILTER, GL_NEAREST )


    def bind(self):
        glBindTexture( self._type, self.id )
        if self._type == GL_TEXTURE_BUFFER:
            self._buffer.bind()

    def store(self, data, data_format=None, width=0, height=0, depth=0, entry_size=None):
        self.bind()
        if self._type == GL_TEXTURE_1D:
            glTexImage1D(GL_TEXTURE_1D, 0, self._format, len(data), 0, self._format, data_format, data)
        if self._type == GL_TEXTURE_2D:
            glTexImage2D( GL_TEXTURE_2D, 0, self._format, width, height, 0, self._format, data_format, data )
        if self._type == GL_TEXTURE_3D:
            glTexImage3D(GL_TEXTURE_3D, 0, self._format, width, height, depth, 0, self._format2, data_format, data)
        if self._type == GL_TEXTURE_BUFFER:
            if entry_size is None:
                data_size = ctypes.sizeof(ctypes.c_float)*len(data)
            else:
                data_size = entry_size * len(data)
            glBufferData ( GL_TEXTURE_BUFFER, data_size, ctypes.c_void_p(), GL_DYNAMIC_DRAW ) # alloc
            glBufferSubData( GL_TEXTURE_BUFFER, 0, data_size, data) # fill


class Query(GLObject):
    def __init__(self, query_type):
        self._type = query_type
        self._id = glGenQueries(1)[0]

    def __enter__(self):
        glBeginQuery(self._type , self.id)
        return self

    def __exit__(self ,type, value, traceback):
        glEndQuery(self._type)
        ready = False
        while not ready:
            ready = glGetQueryObjectiv(self.id,GL_QUERY_RESULT_AVAILABLE)
        self.value = glGetQueryObjectuiv(self.id, GL_QUERY_RESULT )
        glDeleteQueries( [self.id] )

class TextRenderer:
    class Font:
        pass

    def __init__(self):
        self.fonts = {}

        self._vao = VertexArray()

        with self._vao:
            self.characters = ArrayBuffer(usage=GL_DYNAMIC_DRAW)

    def addFont(self, font_size):
        with self._vao:
            font = TextRenderer.Font()
            font.size = font_size

            db = QtGui.QFontDatabase()
            qfont = db.systemFont(db.FixedFont)
            if font_size>0:
                qfont.setPointSize(font_size)
            else:
                self.fonts[0] = font

            self.fonts[qfont.pointSize()] = font

            metrics = QtGui.QFontMetrics(qfont)

            font.width = metrics.maxWidth()
            font.height = metrics.height()

            font.tex_width = (1+128-32)*metrics.maxWidth()
            font.tex_width = (font.tex_width+3)//4*4 # should be multiple of 4
            font.tex_height = metrics.height()
            for i in range(32,128):
                c = bytes([i]).decode()

            image = QtGui.QImage(font.tex_width, font.tex_height, QtGui.QImage.Format_Grayscale8)
            image.fill(QtCore.Qt.black)

            painter = QtGui.QPainter()
            painter.begin(image)
            painter.setFont(qfont)
            painter.setPen(QtCore.Qt.white)
            for i in range(32,128):
                w = metrics.maxWidth()
                text = bytes([i]).decode()
                painter.drawText((i-32)*w,0, (i+1-32)*w, font.height, QtCore.Qt.AlignTop | QtCore.Qt.AlignLeft, text)
            painter.end()
            ptr = image.constBits()
            if qtpy.API == "pyqt5":
                ptr.setsize(image.byteCount())
            Z = numpy.array(ptr).reshape(font.tex_height, font.tex_width)

            font.tex = Texture(GL_TEXTURE_2D, GL_RED)
            font.tex.store(Z, GL_UNSIGNED_BYTE, Z.shape[1], Z.shape[0] )
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST )
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST )

    def draw(self, rendering_params, text, pos, font_size=0, use_absolute_pos=True, alignment=QtCore.Qt.AlignTop|QtCore.Qt.AlignLeft):
        with self._vao:
            if not font_size in self.fonts:
                self.addFont(font_size)

            prog = getProgram('font.vert', 'font.geom', 'font.frag')

            viewport = glGetIntegerv( GL_VIEWPORT )
            screen_width = viewport[2]-viewport[0]
            screen_height = viewport[3]-viewport[1]

            uniforms = prog.uniforms

            font = self.fonts[font_size]
            glActiveTexture(GL_TEXTURE0)
            uniforms.set('font', 0)
            font.tex.bind()

            uniforms.set('font_width_in_texture', font.width/font.tex_width)
            uniforms.set('font_height_in_texture', font.height/font.tex_height)
            uniforms.set('font_width_on_screen', 2*font.width/screen_width)
            uniforms.set('font_height_on_screen', 2*font.height/screen_height)

            if not use_absolute_pos:
                x = ngsolve.bla.Vector(4)
                for i in range(3):
                    x[i] = pos[i]
                x[3] = 1.0
                model, view, projection = rendering_params.model, rendering_params.view, rendering_params.projection
                x = projection*view*model*x
                for i in range(3):
                    pos[i] = x[i]/x[3]


            text_width = len(text)*2*font.width/screen_width
            text_height = 2*font.height/screen_height

            if alignment&QtCore.Qt.AlignRight:
                pos[0] -= text_width
            if alignment&QtCore.Qt.AlignBottom:
                pos[1] += text_height

            if alignment&QtCore.Qt.AlignCenter:
                pos[0] -= 0.5*text_width
            if alignment&QtCore.Qt.AlignVCenter:
                pos[1] += 0.5*text_height

            uniforms.set('start_pos', pos)

            s = numpy.array(list(text.encode('ascii', 'ignore')), dtype=numpy.uint8)
            self.characters.store(s)

            char_id = glGetAttribLocation(prog.id, b'char_')
            glVertexAttribIPointer(char_id, 1, GL_UNSIGNED_BYTE, 0, ctypes.c_void_p());
            glEnableVertexAttribArray( char_id )

            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            glDrawArrays(GL_POINTS, 0, len(s))
