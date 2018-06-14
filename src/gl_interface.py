
import weakref
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
        self.elements = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_R32I)
        self.vertices = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_RGB32F)
        super().__init__(mesh)

    def getTimestamp(self):
        return self.obj().ngmesh._timestamp

    @inmain_decorator(True)
    def update(self):
        meshdata = ngui.GetMeshData(self.obj())

        self.vertices.store(meshdata['vertices'])
        self.elements.store(meshdata["elements"])
        self.nedge_elements = meshdata["n_edge_elements"]
        self.nedges = meshdata["n_edges"]
        self.nperiodic_vertices = meshdata["n_periodic_vertices"]

        self.edges_offset = meshdata["edges_offset"]
        self.periodic_vertices_offset = meshdata["periodic_vertices_offset"]
        self.nsurface_elements = meshdata["n_surface_elements"]
        self.volume_elements_offset = meshdata["volume_elements_offset"]
        self.surface_elements_offset = meshdata["surface_elements_offset"]

        self.min = meshdata['min']
        self.max = meshdata['max']

def getMeshData(mesh):
    if hasattr(mesh,"_opengl_data"):
        return mesh._opengl_data.get()
    else:
        return MeshData(mesh)

class GeoData(DataContainer):
    @inmain_decorator(True)
    def __init__(self, geo):
        self.vertices = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_RGB32F)
        self.triangles = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_RGBA32I)
        self.normals = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_RGB32F)
        super().__init__(geo)

    @inmain_decorator(True)
    def update(self):
        geodata = self.getGeoData()
        self.vertices.store(geodata["vertices"])
        self.triangles.store(geodata["triangles"])
        self.normals.store(geodata["normals"])
        self.surfnames = geodata["surfnames"]
        self.min = geodata["min"]
        self.max = geodata["max"]
        self.npoints = len(geodata["triangles"])//4*3

    def getGeoData(self):
        return ngui.GetGeoData(self.obj())

_opengl_data_constructors = {ngs.Mesh : MeshData,
                             netgen.meshing.NetgenGeometry : GeoData}

def getOpenGLData(obj):
    if not hasattr(obj, "_opengl_data"):
        _opengl_data_constructors[type(obj)](obj)
    return obj._opengl_data
