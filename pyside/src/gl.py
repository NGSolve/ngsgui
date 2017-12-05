from OpenGL.GL import *
from ngui import *
import array
import ctypes

from . import glmath, shader

class GLObject:
    @property
    def id(self):
        return self._id

class Shader(GLObject):
    def __init__(self, code, shader_type):
        self._code = code
        self._type = shader_type
        self._id = glCreateShader(self._type)

        glShaderSource(self.id, code)
        glCompileShader(self.id)

        if glGetShaderiv(self.id, GL_COMPILE_STATUS) != GL_TRUE:
            raise RuntimeError(glGetShaderInfoLog(self.id))

class Program(GLObject):
    locations = {}

    def __init__(self, shaders):
        self._shaders = shaders
        self._id = glCreateProgram()

        for shader in shaders:
            glAttachShader(self.id, shader.id)

        glLinkProgram(self.id)
        if glGetProgramiv(self.id, GL_LINK_STATUS) != GL_TRUE:
                raise RuntimeError(glGetProgramInfoLog(self.id))

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
    timestamp = -1

    def getQtWidget(self, updateGL):
        return None

class MeshScene(SceneObject):
    uniform_names = [b"fColor", b"MV", b"P"]
    attribute_names = [b"vPos"]
    uniforms = {}
    attributes = {}

    def __init__(self, mesh):
        super(MeshScene, self).__init__()
        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        self.coordinates = ArrayBuffer()
        self.bary_coordinates = ArrayBuffer()
        self.element_number = ArrayBuffer()

        self.mesh = mesh
        shaders = [
            Shader(shader.vertex_mesh, GL_VERTEX_SHADER),
            Shader(shader.fragment_mesh, GL_FRAGMENT_SHADER)
        ]
        self.program = Program(shaders)

        for name in self.uniform_names:
            self.uniforms[name] = glGetUniformLocation(self.program.id, name)

        for name in self.attribute_names:
            self.attributes[name] = glGetAttribLocation(self.program.id, name)


    def update(self):
        glBindVertexArray(self.vao)
        self.ntrigs, coordinates_data, bary_coordinates_data, element_number_data, self.min, self.max = GetFaceData(self.mesh)
        self.coordinates.store(coordinates_data)
        self.bary_coordinates.store(bary_coordinates_data)

        glEnableVertexAttribArray(self.attributes[b'vPos'])
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, 0);


    def setupRender(self, model, view, projection):
        center = 0.5*(self.max-self.min)
        modelview = view*model*glmath.Translate(-center[0], -center[1], -center[2]) #move to center
        mv = [modelview[i,j] for i in range(4) for j in range(4)]
        p = [projection[i,j] for i in range(4) for j in range(4)]

        glUseProgram(self.program.id)
        glUniformMatrix4fv(self.uniforms[b'MV'], 1, GL_TRUE, (ctypes.c_float*16)(*mv))
        glUniformMatrix4fv(self.uniforms[b'P'], 1, GL_TRUE, (ctypes.c_float*16)(*p))

        self.coordinates.bind();
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

    def render(self, model, view, projection):
        glBindVertexArray(self.vao)
        self.setupRender(model, view, projection)
        glUniform4f(self.uniforms[b'fColor'], 0.0,1.0,0.0,1.0)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.ntrigs)

        self.renderWireframe(model,view,projection)

    def renderWireframe(self, model, view, projection):
        glBindVertexArray(self.vao)
        self.setupRender(model, view, projection)
        glUniform4f(self.uniforms[b'fColor'], 0.0,0.0,0.0,1.0)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.ntrigs)

class SolutionScene(SceneObject):
    uniform_names = [b"MV", b"P", b"colormap_min", b"colormap_max", b"colormap_linear"]#, b"coefficients"]
    attribute_names = [b"vPos", b"vLam", b"vElementNumber"]
    uniforms = {}
    attributes = {}
    qtWidget = None

    def __init__(self, gf, colormap_min=-1.0, colormap_max=1.0, colormap_linear=False):
        super(SolutionScene, self).__init__()
        self.colormap_min = colormap_min
        self.colormap_max = colormap_max
        self.colormap_linear = colormap_linear

        self.mesh = gf.space.mesh
        self.mesh_scene = MeshScene(self.mesh)
        self.gf = gf

        fragment_shader = shader.fragment_header + GenerateShader(gf.space.globalorder) + shader.fragment_main

        shaders = [
            Shader(shader.vertex_simple, GL_VERTEX_SHADER),
            Shader(fragment_shader, GL_FRAGMENT_SHADER)
        ]
        self.program = Program(shaders)

        for name in self.uniform_names:
            self.uniforms[name] = glGetUniformLocation(self.program.id, name)

        for name in self.attribute_names:
            self.attributes[name] = glGetAttribLocation(self.program.id, name)


        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        self.coefficients = glGenBuffers(1)
        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );

        tex = glGenTextures  (1)
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture  ( GL_TEXTURE_BUFFER, tex )
        glTexBuffer    ( GL_TEXTURE_BUFFER, GL_R32F, self.coefficients );

        self.coordinates = ArrayBuffer()
        self.bary_coordinates = ArrayBuffer()
        self.element_number = ArrayBuffer()


        glEnableVertexAttribArray(self.attributes[b'vLam'])
        self.bary_coordinates.bind();
        glVertexAttribPointer(self.attributes[b'vLam'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        glEnableVertexAttribArray(self.attributes[b'vPos'])
        self.coordinates.bind();
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        glEnableVertexAttribArray(self.attributes[b'vElementNumber'])
        self.element_number.bind();
        glVertexAttribIPointer(self.attributes[b'vElementNumber'], 1, GL_INT, 0, ctypes.c_void_p());



    def update(self):
        self.mesh_scene.update()
        glBindVertexArray(self.vao)
        self.ntrigs, coordinates_data, bary_coordinates_data, element_number_data, self.min, self.max = GetFaceData(self.mesh)

        self.coordinates.store(coordinates_data)
        glEnableVertexAttribArray(self.attributes[b'vPos'])
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, 0);

        self.bary_coordinates.store(bary_coordinates_data)
        glEnableVertexAttribArray(self.attributes[b'vLam'])
        glVertexAttribPointer(self.attributes[b'vLam'], 3, GL_FLOAT, GL_FALSE, 0, 0);

        self.element_number.store(element_number_data)
        glEnableVertexAttribArray(self.attributes[b'vElementNumber'])
        glVertexAttribIPointer(self.attributes[b'vElementNumber'], 1, GL_INT, 0, ctypes.c_void_p());

        glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );
        vec = self.gf.vec
        ncoefs = len(vec)
        size_float=ctypes.sizeof(ctypes.c_float)

        glBufferData   ( GL_TEXTURE_BUFFER, size_float*ncoefs, ctypes.c_void_p(), GL_STATIC_DRAW ) # alloc
        glBufferSubData( GL_TEXTURE_BUFFER, 0, size_float*ncoefs, (ctypes.c_float*ncoefs)(*vec)) # fill


    def render(self, model, view, projection):
        glBindVertexArray(self.vao)
        center = 0.5*(self.max-self.min)
        modelview = view*model*glmath.Translate(-center[0], -center[1], -center[2]) #move to center
        mv = [modelview[i,j] for i in range(4) for j in range(4)]
        p = [projection[i,j] for i in range(4) for j in range(4)]

        glUseProgram(self.program.id)
        glUniformMatrix4fv(self.uniforms[b'MV'], 1, GL_TRUE, (ctypes.c_float*16)(*mv))
        glUniformMatrix4fv(self.uniforms[b'P'], 1, GL_TRUE, (ctypes.c_float*16)(*p))

        glUniform1f(self.uniforms[b'colormap_min'], self.colormap_min);
        glUniform1f(self.uniforms[b'colormap_max'],  self.colormap_max);
        glUniform1i(self.uniforms[b'colormap_linear'],  self.colormap_linear);

        glEnableVertexAttribArray(self.attributes[b'vPos']);
        self.coordinates.bind();
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        glEnableVertexAttribArray(self.attributes[b'vLam']);
        self.bary_coordinates.bind();
        glVertexAttribPointer(self.attributes[b'vLam'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        glEnableVertexAttribArray(self.attributes[b'vElementNumber']);
        self.element_number.bind();
        glVertexAttribIPointer(self.attributes[b'vElementNumber'], 1, GL_INT, 0, ctypes.c_void_p());

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.ntrigs);

        self.mesh_scene.renderWireframe(model,view,projection)

    def setColorMapMin(self, value):
        self.colormap_min = value

    def setColorMapMax(self, value):
        self.colormap_max = value

    def setColorMapLinear(self, value):
        self.colormap_linear = value


    def getQtWidget(self, updateGL):
        if self.qtWidget!=None:
            print("return old widget")
            return self.qtWidget
        print("return new widget")

        from .gui import ColorMapSettings, Qt

        settings = ColorMapSettings(min=-2, max=2, min_value=self.colormap_min, max_value=self.colormap_max)
        settings.layout().setAlignment(Qt.AlignTop)

        settings.minChanged.connect(self.setColorMapMin)
        settings.minChanged.connect(updateGL)

        settings.maxChanged.connect(self.setColorMapMax)
        settings.maxChanged.connect(updateGL)

        settings.linearChanged.connect(self.setColorMapLinear)
        settings.linearChanged.connect(updateGL)

        self.qtWidget = settings
        return self.qtWidget


