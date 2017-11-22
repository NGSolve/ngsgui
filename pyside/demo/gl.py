from OpenGL.GL import *
import shader
from ngui import *
import array
import ctypes
import glmath

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
        self.trig_indices = ArrayBuffer()

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
        coordinates_data, trig_indices_data = GetVisData(self.mesh)
        self.coordinates.store(coordinates_data)

        glEnableVertexAttribArray(self.attributes[b'vPos'])
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, 0);


    def render(self, model, view, projection):
        modelview = view*model*glmath.Translate(-0.5, -0.5,0) #move to center
        mv = [modelview[i,j] for i in range(4) for j in range(4)]
        p = [projection[i,j] for i in range(4) for j in range(4)]

        glUseProgram(self.program.id)
        glUniformMatrix4fv(self.uniforms[b'MV'], 1, GL_TRUE, (ctypes.c_float*16)(*mv))
        glUniformMatrix4fv(self.uniforms[b'P'], 1, GL_TRUE, (ctypes.c_float*16)(*p))

        self.coordinates.bind();
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        glUniform4f(self.uniforms[b'fColor'], 0.0,1.0,0.0,1.0)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh.ne)

        glUniform4f(self.uniforms[b'fColor'], 0.0,0.0,0.0,1.0)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh.ne)

class SolutionScene(SceneObject):
    uniform_names = [b"MV", b"P", b"colormap_min", b"colormap_max", b"colormap_linear"]#, b"coefficients"]
    attribute_names = [b"vPos", b"vIndex"]
    uniforms = {}
    attributes = {}

    def __init__(self, gf):
        super(SolutionScene, self).__init__()
        self.mesh = gf.space.mesh
        self.gf = gf
#         self.meshscene = MeshScene(gf.space.mesh)

        fragment_shader = GenerateShader(gf.space.globalorder)

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

#         self.coefficients = glGenBuffers(1)
#         glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );
# 
#         tex = glGenTextures  (1)
#         glActiveTexture( GL_TEXTURE0 );
#         glBindTexture  ( GL_TEXTURE_BUFFER, tex )
#         glTexBuffer    ( GL_TEXTURE_BUFFER, GL_R32F, self.coefficients );

        self.coordinates = ArrayBuffer()
        self.trig_indices = ArrayBuffer()


        glEnableVertexAttribArray(self.attributes[b'vPos'])
        self.coordinates.bind();
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        glEnableVertexAttribArray(self.attributes[b'vIndex'])
        self.trig_indices.bind();
        glVertexAttribIPointer(self.attributes[b'vIndex'], 1, GL_BYTE, 0, ctypes.c_void_p());



    def update(self):
        coordinates_data, trig_indices_data = GetVisData(self.mesh)

        self.coordinates.store(coordinates_data)
        glEnableVertexAttribArray(self.attributes[b'vPos'])
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, 0);

        self.trig_indices.store(trig_indices_data)
        glEnableVertexAttribArray(self.attributes[b'vIndex'])
        glVertexAttribIPointer(self.attributes[b'vIndex'], 1, GL_BYTE, 0, ctypes.c_void_p());

#         self.meshscene.update()
#         glBindBuffer   ( GL_TEXTURE_BUFFER, self.coefficients );
        # todo
#         glBufferData   ( GL_TEXTURE_BUFFER, sizeof(GLfloat)*coefficients_float.Size(), NULL, GL_STATIC_DRAW );  // Alloc
#         glBufferSubData( GL_TEXTURE_BUFFER, 0, sizeof(GLfloat)*coefficients_float.Size(), &coefficients_float[0]); // Fill


    def render(self, model, view, projection):
        glBindVertexArray(self.vao)
        modelview = view*model*glmath.Translate(-0.5, -0.5,0) #move to center
        mv = [modelview[i,j] for i in range(4) for j in range(4)]
        p = [projection[i,j] for i in range(4) for j in range(4)]

        glUseProgram(self.program.id)
        glUniformMatrix4fv(self.uniforms[b'MV'], 1, GL_TRUE, (ctypes.c_float*16)(*mv))
        glUniformMatrix4fv(self.uniforms[b'P'], 1, GL_TRUE, (ctypes.c_float*16)(*p))

        glEnableVertexAttribArray(self.attributes[b'vPos']);
        self.coordinates.bind();
        glVertexAttribPointer(self.attributes[b'vPos'], 3, GL_FLOAT, GL_FALSE, 0, ctypes.c_void_p());

        glEnableVertexAttribArray(self.attributes[b'vIndex']);
        self.trig_indices.bind();
        glVertexAttribIPointer(self.attributes[b'vIndex'], 1, GL_BYTE, 0, ctypes.c_void_p());

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh.ne);

