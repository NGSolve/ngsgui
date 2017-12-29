from OpenGL.GL import *
from ngui import *
import array
import ctypes
import time

from . import glmath, shader
from .gui import ColorMapSettings, Qt, RangeGroup, BCColors

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
    def __init__(self, shaders):
        self.locations = {}
        self._shaders = shaders

        self._id = glCreateProgram()
        for shader in shaders:
            glAttachShader(self.id, shader.id)

        glLinkProgram(self.id)
        if glGetProgramiv(self.id, GL_LINK_STATUS) != GL_TRUE:
                raise RuntimeError(glGetProgramInfoLog(self.id))

class CMeshData:
    def __init__(self, mesh):
        import weakref
        print("generate mesh data")
        self.mesh = weakref.ref(mesh)
        self.ntrigs, self.trig_coordinates_data, self.trig_bary_coordinates_data, self.trig_element_number_data, self.trig_element_index_data, self.trig_max_index, self.min, self.max = GetFaceData(mesh)
        self.ntets, self.tet_coordinates_data, self.tet_bary_coordinates_data, self.tet_element_number_data = GetTetData(mesh)
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
    def __init__(self):
        self.timestamp = -1

    def getQtWidget(self, updateGL):
        return None

class ClippingPlaneScene(SceneObject):
    def __init__(self, gf, colormap_min=-1.0, colormap_max=1.0, colormap_linear=False):
        super(ClippingPlaneScene, self).__init__()

        self.uniform_names = [b"MV", b"P", b"colormap_min", b"colormap_max", b"colormap_linear", b"clipping_plane", b"do_clipping", b"clipping_plane_deformation", b"element_type"]
        self.attribute_names = [b"vPos", b"vLam", b"vElementNumber"]
        self.uniforms = {}
        self.attributes = {}
        self.qtWidget = None
        self.gl_initialized = False

        self.colormap_min = colormap_min
        self.colormap_max = colormap_max
        self.colormap_linear = colormap_linear

        self.mesh = gf.space.mesh
        self.gf = gf

    def initGL(self):
        if self.gl_initialized:
            return

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        Shader.includes['shader_functions'] = GenerateShader(self.gf.space.globalorder)

        shaders = [
            Shader('solution.vert'),
            Shader('clipping.geom'),
            Shader('solution.frag')
        ]
        self.program = Program(shaders)
        glUseProgram(self.program.id)

        for name in self.uniform_names:
            self.uniforms[name] = glGetUniformLocation(self.program.id, name)

        for name in self.attribute_names:
            self.attributes[name] = glGetAttribLocation(self.program.id, name)


        self.coefficients = glGenBuffers(1)
        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );

        tex = glGenTextures  (1)
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture  ( GL_TEXTURE_BUFFER, tex )
        glTexBuffer    ( GL_TEXTURE_BUFFER, GL_R32F, self.coefficients );

        self.coordinates = ArrayBuffer()
        self.bary_coordinates = ArrayBuffer()
        self.element_number = ArrayBuffer()


        md = MeshData(self.mesh)
        self.ntrigs = md.ntrigs
        self.ntets = md.ntets
        self.min = md.min
        self.max = md.max

        if(self.attributes[b'vPos']>-1):
            self.coordinates.store(md.tet_coordinates_data)
            glEnableVertexAttribArray(self.attributes[b'vPos'])
            glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        if(self.attributes[b'vLam']>-1):
            self.bary_coordinates.store(md.tet_bary_coordinates_data)
            glEnableVertexAttribArray(self.attributes[b'vLam'])
            glVertexAttribPointer(self.attributes[b'vLam'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());
            glEnableVertexAttribArray(0)

        if(self.attributes[b'vElementNumber']>-1):
            self.element_number.store(md.tet_element_number_data)
            glEnableVertexAttribArray(self.attributes[b'vElementNumber'])
            glVertexAttribIPointer(self.attributes[b'vElementNumber'], 1, GL_INT, 0, ctypes.c_void_p());
            glEnableVertexAttribArray(0)

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
        modelview = view*model
        mv = [modelview[i,j] for i in range(4) for j in range(4)]
        p = [projection[i,j] for i in range(4) for j in range(4)]

        glUniformMatrix4fv(self.uniforms[b'MV'], 1, GL_TRUE, (ctypes.c_float*16)(*mv))
        glUniformMatrix4fv(self.uniforms[b'P'], 1, GL_TRUE, (ctypes.c_float*16)(*p))

        glUniform1f(self.uniforms[b'colormap_min'], self.colormap_min);
        glUniform1f(self.uniforms[b'colormap_max'],  self.colormap_max);
        glUniform1i(self.uniforms[b'colormap_linear'],  self.colormap_linear);
        glUniform4f(self.uniforms[b'clipping_plane'], *settings.clipping_plane)
        glUniform1i(self.uniforms[b'do_clipping'],  False);
        glUniform1i(self.uniforms[b'clipping_plane_deformation'],  False);
        # TODO: element_type should be an attribute!
        if(self.mesh.dim==2):
            glUniform1i(self.uniforms[b'element_type'],  10);
        if(self.mesh.dim==3):
            glUniform1i(self.uniforms[b'element_type'],  20);

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_LINES_ADJACENCY, 0, 4*self.ntets)
        glBindVertexArray(0)


    def setColorMapMin(self, value):
        self.colormap_min = value

    def setColorMapMax(self, value):
        self.colormap_max = value

    def setColorMapLinear(self, value):
        self.colormap_linear = value


    def getQtWidget(self, updateGL):
        if self.qtWidget!=None:
            return self.qtWidget

        settings = ColorMapSettings(min=-2, max=2, min_value=self.colormap_min, max_value=self.colormap_max)
        settings.layout().setAlignment(Qt.AlignTop)

        settings.minChanged.connect(self.setColorMapMin)
        settings.minChanged.connect(updateGL)

        settings.maxChanged.connect(self.setColorMapMax)
        settings.maxChanged.connect(updateGL)

        settings.linearChanged.connect(self.setColorMapLinear)
        settings.linearChanged.connect(updateGL)

        self.qtWidget = settings
        return {"Colormap" : self.qtWidget}

class MeshScene(SceneObject):
    def __init__(self, mesh):
        super(MeshScene, self).__init__()

        self.uniform_names = [b"fColor", b"fColor_clipped", b"MV", b"P", b"clipping_plane", b"use_index_color"]
        self.attribute_names = [b"pos", b"index"]
        self.uniforms = {}
        self.attributes = {}
        self.qtWidget = None
        self.gl_initialized = False

        self.mesh = mesh

    def initGL(self):
        if self.gl_initialized:
            return

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        self.coordinates = ArrayBuffer()
        self.bary_coordinates = ArrayBuffer()
        self.element_number = ArrayBuffer()
        self.element_index = ArrayBuffer()

        shaders = [
            Shader('mesh.vert'),
            Shader('mesh.frag')
        ]
        self.program = Program(shaders)

        for name in self.uniform_names:
            self.uniforms[name] = glGetUniformLocation(self.program.id, name)
        for name in self.attribute_names:
            self.attributes[name] = glGetAttribLocation(self.program.id, name)

        self.gl_initialized = True
        glBindVertexArray(0)

    def update(self):
        self.initGL()
        glBindVertexArray(self.vao)
        md = MeshData(self.mesh)
        self.ntrigs = md.ntrigs
        self.min = md.min
        self.max = md.max
        self.max_index = md.trig_max_index
        self.coordinates.store(md.trig_coordinates_data)
        self.bary_coordinates.store(md.trig_bary_coordinates_data)
        self.element_index.store(md.trig_element_index_data)
        self.index_colors = [0, 255, 0, 255] * (self.max_index+1)

        if self.attributes[b'pos']>-1:
            self.coordinates.bind();
            glEnableVertexAttribArray(self.attributes[b'pos'])
            glVertexAttribPointer(self.attributes[b'pos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p())

        if self.attributes[b'index']>-1:
            self.element_index.bind()
            glEnableVertexAttribArray(self.attributes[b'index'])
            glVertexAttribIPointer(self.attributes[b'index'], 1, GL_INT, 0, ctypes.c_void_p())


        self.tex_index_color = glGenTextures(1)

        glBindTexture(GL_TEXTURE_1D, self.tex_index_color)

        # copy texture
        glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, self.max_index+1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes(self.index_colors))

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glBindVertexArray(0)

    def setupRender(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection
        modelview = view*model
        mv = [modelview[i,j] for i in range(4) for j in range(4)]
        p = [projection[i,j] for i in range(4) for j in range(4)]

        glBindTexture(GL_TEXTURE_1D, self.tex_index_color)
        glUseProgram(self.program.id)
        glUniformMatrix4fv(self.uniforms[b'MV'], 1, GL_TRUE, (ctypes.c_float*16)(*mv))
        glUniformMatrix4fv(self.uniforms[b'P'], 1, GL_TRUE, (ctypes.c_float*16)(*p))



    def render(self, settings):
        glBindVertexArray(self.vao)
        self.setupRender(settings)
        glUniform4f(self.uniforms[b'fColor'], 0.0,1.0,0.0,1.0)
        glUniform4f(self.uniforms[b'fColor_clipped'], 0.0,1.0,0.0,0.00)
        glUniform4f(self.uniforms[b'clipping_plane'], *settings.clipping_plane)
        glUniform1i(self.uniforms[b'use_index_color'], True)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.ntrigs)

        self.renderWireframe(settings)

    def renderWireframe(self, settings):
        glBindVertexArray(self.vao)
        self.setupRender(settings)
        glUniform4f(self.uniforms[b'fColor'], 0.0,0.0,0.0,1)
        glUniform4f(self.uniforms[b'fColor_clipped'], 0.0,0.0,0.0,0.1)
        glUniform4f(self.uniforms[b'clipping_plane'], *settings.clipping_plane)
        glUniform1i(self.uniforms[b'use_index_color'], False)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.ntrigs)


    def updateIndexColors(self):
        colors = []
        for c in self.bccolors.getColors():
            colors.append(c.red())
            colors.append(c.green())
            colors.append(c.blue())
            colors.append(c.alpha())
        self.index_colors = colors
        glBindTexture(GL_TEXTURE_1D, self.tex_index_color)
        glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, self.max_index+1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes(self.index_colors))

    def getQtWidget(self, updateGL):
        if self.qtWidget!=None:
            return self.qtWidget

        self.bccolors = BCColors(self.mesh)
        self.bccolors.colors_changed.connect(self.updateIndexColors)
        self.bccolors.colors_changed.connect(updateGL)
        self.updateIndexColors()

        return {"BCColors" : self.bccolors }


class MeshElementsScene(SceneObject):
    def __init__(self, mesh):
        super(MeshElementsScene, self).__init__()

        self.uniform_names = [b"MV", b"P", b"clipping_plane", b'shrink_elements']
        self.attribute_names = [b"pos", b"index"]
        self.uniforms = {}
        self.attributes = {}
        self.qtWidget = None
        self.gl_initialized = False
        self.shrink = 1.0

        self.mesh = mesh

    def initGL(self):
        if self.gl_initialized:
            return

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        self.coordinates = ArrayBuffer()
        self.element_number = ArrayBuffer()
        self.element_index = ArrayBuffer()

        shaders = [
            Shader('elements.vert'),
            Shader('elements.geom'),
            Shader('elements.frag')
        ]
        self.program = Program(shaders)

        for name in self.uniform_names:
            self.uniforms[name] = glGetUniformLocation(self.program.id, name)
        for name in self.attribute_names:
            self.attributes[name] = glGetAttribLocation(self.program.id, name)

        self.gl_initialized = True
        glBindVertexArray(0)

    def update(self):
        self.initGL()
        glBindVertexArray(self.vao)
        md = MeshData(self.mesh)
        self.ntets = md.ntets
        self.min = md.min
        self.max = md.max
        self.coordinates.store(md.tet_coordinates_data)

        if self.attributes[b'pos']>-1:
            self.coordinates.bind();
            glEnableVertexAttribArray(self.attributes[b'pos'])
            glVertexAttribPointer(self.attributes[b'pos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p())

        glBindVertexArray(0)

    def render(self, settings):
        glBindVertexArray(self.vao)
        model, view, projection = settings.model, settings.view, settings.projection
        modelview = view*model
        mv = [modelview[i,j] for i in range(4) for j in range(4)]
        p = [projection[i,j] for i in range(4) for j in range(4)]

        glUseProgram(self.program.id)
        glUniformMatrix4fv(self.uniforms[b'MV'], 1, GL_TRUE, (ctypes.c_float*16)(*mv))
        glUniformMatrix4fv(self.uniforms[b'P'], 1, GL_TRUE, (ctypes.c_float*16)(*p))
        glUniform1f(self.uniforms[b'shrink_elements'], self.shrink)

        glUniform4f(self.uniforms[b'clipping_plane'], *settings.clipping_plane)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_LINES_ADJACENCY, 0, 4*self.ntets)

    def setShrink(self, value):
        self.shrink = value

    def getQtWidget(self, updateGL):
        if self.qtWidget!=None:
            return self.qtWidget


        shrink = RangeGroup("Shrink", min=0.0, max=1.0, value=1.0)
        shrink.valueChanged.connect(self.setShrink)
        shrink.valueChanged.connect(updateGL)
        return {"Shrink": shrink}


class SolutionScene(SceneObject):
    def __init__(self, gf, colormap_min=-1.0, colormap_max=1.0, colormap_linear=False):
        super(SolutionScene, self).__init__()

        self.uniform_names = [b"MV", b"P", b"element_type", b"colormap_min", b"colormap_max", b"colormap_linear", b"clipping_plane", b"do_clipping"]#, b"coefficients"]
        self.attribute_names = [b"vPos", b"vLam", b"vElementNumber"]
        self.uniforms = {}
        self.attributes = {}
        self.qtWidget = None
        self.gl_initialized = False

        self.colormap_min = colormap_min
        self.colormap_max = colormap_max
        self.colormap_linear = colormap_linear

        self.mesh = gf.space.mesh
        self.mesh_scene = MeshScene(self.mesh)
        self.gf = gf

    def initGL(self):
        if self.gl_initialized:
            return

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        Shader.includes['shader_functions'] = GenerateShader(self.gf.space.globalorder)

        shaders = [
            Shader('solution.vert'),
            Shader('solution.frag')
        ]
        self.program = Program(shaders)

        for name in self.uniform_names:
            self.uniforms[name] = glGetUniformLocation(self.program.id, name)

        for name in self.attribute_names:
            self.attributes[name] = glGetAttribLocation(self.program.id, name)


        self.coefficients = glGenBuffers(1)
        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );

        tex = glGenTextures  (1)
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture  ( GL_TEXTURE_BUFFER, tex )
        glTexBuffer    ( GL_TEXTURE_BUFFER, GL_R32F, self.coefficients );

        self.coordinates = ArrayBuffer()
        self.bary_coordinates = ArrayBuffer()
        self.element_number = ArrayBuffer()

        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );
        vec = self.gf.vec
        ncoefs = len(vec)
        size_float=ctypes.sizeof(ctypes.c_float)

        glBufferData   ( GL_TEXTURE_BUFFER, size_float*ncoefs, ctypes.c_void_p(), GL_DYNAMIC_DRAW ) # alloc

        self.mesh_scene.update()
        glBindVertexArray(self.vao)
        md = MeshData(self.mesh)
        self.ntrigs = md.ntrigs
        self.min = md.min
        self.max = md.max

        self.coordinates.store(md.trig_coordinates_data)
        glEnableVertexAttribArray(self.attributes[b'vPos'])
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        self.bary_coordinates.store(md.trig_bary_coordinates_data)
        glEnableVertexAttribArray(self.attributes[b'vLam'])
        glVertexAttribPointer(self.attributes[b'vLam'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        self.element_number.store(md.trig_element_number_data)
        glEnableVertexAttribArray(self.attributes[b'vElementNumber'])
        glVertexAttribIPointer(self.attributes[b'vElementNumber'], 1, GL_INT, 0, ctypes.c_void_p());
        glBindVertexArray(0)


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
        modelview = view*model
        mv = [modelview[i,j] for i in range(4) for j in range(4)]
        p = [projection[i,j] for i in range(4) for j in range(4)]

        glUseProgram(self.program.id)
        glUniformMatrix4fv(self.uniforms[b'MV'], 1, GL_TRUE, (ctypes.c_float*16)(*mv))
        glUniformMatrix4fv(self.uniforms[b'P'], 1, GL_TRUE, (ctypes.c_float*16)(*p))

        glUniform1f(self.uniforms[b'colormap_min'], self.colormap_min);
        glUniform1f(self.uniforms[b'colormap_max'],  self.colormap_max);
        glUniform1i(self.uniforms[b'colormap_linear'],  self.colormap_linear);
        if(self.mesh.dim==2):
            glUniform1i(self.uniforms[b'element_type'],  10);
        if(self.mesh.dim==3):
            glUniform1i(self.uniforms[b'element_type'],  20);
        glUniform4f(self.uniforms[b'clipping_plane'], *settings.clipping_plane)
        glUniform1i(self.uniforms[b'do_clipping'], self.mesh.dim==3);

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.ntrigs);

        self.mesh_scene.renderWireframe(settings)

    def setColorMapMin(self, value):
        self.colormap_min = value

    def setColorMapMax(self, value):
        self.colormap_max = value

    def setColorMapLinear(self, value):
        self.colormap_linear = value


    def getQtWidget(self, updateGL):
        if self.qtWidget!=None:
            return self.qtWidget

        settings = ColorMapSettings(min=self.colormap_min, max=self.colormap_max, min_value=self.colormap_min, max_value=self.colormap_max)
        settings.layout().setAlignment(Qt.AlignTop)

        settings.minChanged.connect(self.setColorMapMin)
        settings.minChanged.connect(updateGL)

        settings.maxChanged.connect(self.setColorMapMax)
        settings.maxChanged.connect(updateGL)

        settings.linearChanged.connect(self.setColorMapLinear)
        settings.linearChanged.connect(updateGL)

        self.qtWidget = settings
        return {"Colormap" : self.qtWidget}


