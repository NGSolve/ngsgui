
import weakref, numpy
import OpenGL.GL as GL
from .gl import Texture
import ngsolve as ngs
import netgen.meshing
from . import ngui
from .thread import inmain_decorator

class DataContainer:
    """Class to avoid redundant copies of same objects on GPU"""
    def __init__(self, obj):
        self.obj = weakref.ref(obj)
        obj._opengl_data = self
        self.update()

    def update(self):
        self.timestamp = self.getTimestamp()

    def get(self):
        if self.timestamp != self.getTimestamp():
            self.update()
        return self

class MeshData(DataContainer):
    """
    Vertex data:
        vec3 pos

    Surface elements:
        int v0,v1,v2;
        int curved_id; // to fetch curved element data, negative if element is not curved

    Surface curved elements:
        vec3 pos[3];     // Additional points for P2 interpolation
        vec3 normal[3];  // Normals for outer vertices

    Volume elements:
        int v0,v1,v2,v3;
        int curved_id; // to fetch curved element data, negative if element is not curved

    Volume curved elements:
        vec3 pos[6]; // Additional points for p2 interpolation

    Solution data (volume or surface):
        float values[N];   // N depends on order, subdivision
        vec3 gradients[N]; // N depends on order, subdivision

    """
    @inmain_decorator(True)
    def __init__(self, mesh):
        self.vertices = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_RGB32F)
        self.elements = {}
        super().__init__(mesh)

    def getTimestamp(self):
        return self.obj().ngmesh._timestamp

    @inmain_decorator(True)
    def update(self):
        data = ngui.GetMeshData(self.obj())

        self.min = data.pop('min')
        self.max = data.pop('max')
        self.vertices.store(data.pop('vertices'))

        for vb in data:
            for ei in data[vb]:
                ei.tex_vertices = self.vertices
                ei.tex = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_R32I)
                ei.tex.store(numpy.array(ei.data, dtype=numpy.int32))

        self.elements = data

def getMeshData(mesh):
    if hasattr(mesh,"_opengl_data"):
        return mesh._opengl_data.get()
    else:
        return MeshData(mesh)

class GeoData(DataContainer):
    @inmain_decorator(True)
    def __init__(self, geo):
        super().__init__(geo)
        self.update()

    @inmain_decorator(True)
    def initGL(self):
        self.vertices = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_RGB32F)
        self.triangles = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_RGBA32I)
        self.normals = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_RGB32F)
        self.vertices.store(self.geodata["vertices"], data_format=GL.GL_UNSIGNED_BYTE)
        self.triangles.store(self.geodata["triangles"], data_format=GL.GL_UNSIGNED_BYTE)
        self.normals.store(self.geodata["normals"], data_format=GL.GL_UNSIGNED_BYTE)

    @inmain_decorator(True)
    def update(self):
        self.geodata = self.getGeoData()
        self.surfnames = self.geodata["surfnames"]
        self.min = self.geodata["min"]
        self.max = self.geodata["max"]
        self.npoints = len(self.geodata["triangles"])//4*3

    def getGeoData(self):
        return ngui.GetGeoData(self.obj())

_opengl_data_constructors = {ngs.Mesh : MeshData,
                             netgen.meshing.NetgenGeometry : GeoData}

def getOpenGLData(obj):
    if not hasattr(obj, "_opengl_data"):
        for key in _opengl_data_constructors:
            if isinstance(obj, key):
                _opengl_data_constructors[key](obj)
    return obj._opengl_data
