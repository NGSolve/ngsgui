
import weakref, numpy
import OpenGL.GL as GL
from .gl import Texture
import ngsolve as ngs
import netgen.meshing
from .thread import inmain_decorator

def getP2Rules():
    res = {}
    res[ngs.ET.SEGM] = ngs.IntegrationRule([(0,0,0), (1.0,0,0), (0.5,0,0)], [0.0]*3)

    # for 2d elements we need to get the normal vectors at the corner vertices plus mapped coordinates of edge midpoints
    res[ngs.ET.TRIG] = ngs.IntegrationRule([(1,0,0), (0,1,0), (0,0,0),
        (0.5,0.0,0.0), (0.0,0.5,0.0), (0.5,0.5,0.0)], [0.0]*6)
    res[ngs.ET.QUAD] = ngs.IntegrationRule([(0,0,0), (1,0,0), (1,1,0), (0,1,0),
        (0.5,0.0,0.0), (0.0,0.5,0.0), (0.5,0.5,0.0), (1.0,0.5,0.0), (0.5,1.0,0.0)], [0.0]*9)

    # 3d elements have no normal vectors, so only evaluate at edge midpoints
    res[ngs.ET.TET] = ngs.IntegrationRule([ (0.5,0.0,0.0), (0.0,0.5,0.0), (0.5,0.5,0.0), (0.5,0.0,0.5), (0.0,0.5,0.5), (0.0,0.0,0.5)], [0.0]*6)

#         // PRISM
#         for (auto & ip : ir_trig.Range(3,6))
#             ir_prism.Append(ip);
#         for (auto & ip : ir_trig.Range(0,3))
#             ir_prism.Append(IntegrationPoint(ip(0), ip(1), 0.5));
#         for (auto & ip : ir_trig.Range(3,6))
#             ir_prism.Append(IntegrationPoint(ip(0), ip(1), 1.0));
# 
#         // PYRAMID
#         for (auto & ip : ir_quad.Range(4,9))
#             ir_pyramid.Append(ip);
#         for (auto & ip : ir_quad.Range(0,4))
#             ir_pyramid.Append(IntegrationPoint(ip(0), ip(1), 0.5));
# 
#         // HEX
#         for (auto & ip : ir_quad.Range(4,9))
#             ir_hex.Append(ip);
#         for (auto x : {0.0, 0.5, 1.0})
#           for (auto y : {0.0, 0.5, 1.0})
#             ir_hex.Append(IntegrationPoint(x,y,0.5));
#         for (auto & ip : ir_quad.Range(4,9))
#             ir_hex.Append(IntegrationPoint(ip(0), ip(1), 1.0));
    return res

def getReferenceRules(order, sd):
  n = (order)*(sd+1)+1;
  h = 1.0/(n-1);
  res = {}
  res[ngs.ET.SEGM] = ngs.IntegrationRule([ (1.0-i*h,0.0,0.0) for i in range(n) ], [0.0 for i in range(n)])
  res[ngs.ET.TRIG] = ngs.IntegrationRule([ (    i*h,j*h,0.0) for j in range(n) for i in range(n-j) ], [0.0 for i in range(n*(n+1)//2)])
  res[ngs.ET.QUAD] = ngs.IntegrationRule([ (    i*h,j*h,0.0) for j in range(n) for i in range(n) ],   [0.0 for i in range(n**2)])
  res[ngs.ET.TET]  = ngs.IntegrationRule([ (    i*h,j*h,k*h) for k in range(n) for j in range(n-k) for i in range(n-k-j) ],   [0.0 for i in range(n*(n+1)*(n+2)//6)])
  return res

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

    class ElementData():
        """ Contains data of a mesh belonging one type of element (e.g. curved Trigs) """
        nverts = { 
                ngs.ET.POINT: 1,
                ngs.ET.SEGM: 2,
                ngs.ET.TRIG: 3,
                ngs.ET.QUAD: 4,
                ngs.ET.TET: 4,
                ngs.ET.PRISM: 6,
                ngs.ET.PYRAMID: 5,
                ngs.ET.HEX: 6
                }
        dims = { 
                ngs.ET.POINT: 0,
                ngs.ET.SEGM: 1,
                ngs.ET.TRIG: 2,
                ngs.ET.QUAD: 2,
                ngs.ET.TET: 3,
                ngs.ET.PRISM: 3,
                ngs.ET.PYRAMID: 3,
                ngs.ET.HEX: 3
                }

        def __init__(self, ei, vertices):
            self.type = ei['type']
            self.nelements = ei['nelements']
            self.data = ei['data']
            self.curved = ei['curved']
            self.nverts = MeshData.ElementData.nverts[self.type]
            self.dim = MeshData.ElementData.dims[self.type]
            self.size = self.nverts+2
            if self.curved:
                self.size += 1
            if self.type == ngs.ET.POINT:
                self.size = 0
            assert len(self.data) == self.nelements*self.size
            self.tex_vertices = vertices
            self.tex = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_R32I)
            self.tex.store(numpy.array(self.data, dtype=numpy.int32))

    @inmain_decorator(True)
    def __init__(self, mesh):
        self.vertices = Texture(GL.GL_TEXTURE_BUFFER, GL.GL_RGB32F)
        self.elements = {}
        super().__init__(mesh)

    def getTimestamp(self):
        return self.obj().ngmesh._timestamp

    @inmain_decorator(True)
    def update(self):
        data = ngs.solve._GetVisualizationData( self.obj(), getP2Rules() )

        self.elements = {}
        self.min = data.pop('min')
        self.max = data.pop('max')
        self.vertices.store(data.pop('vertices'))

        for vb in data:
            els = []
            for ei in data[vb]:
                els.append(MeshData.ElementData(ei, self.vertices))
            self.elements[vb] = els

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
        return self.obj()._visualizationData()

_opengl_data_constructors = {ngs.Mesh : MeshData,
                             netgen.meshing.NetgenGeometry : GeoData}

def getOpenGLData(obj):
    if not hasattr(obj, "_opengl_data"):
        for key in _opengl_data_constructors:
            if isinstance(obj, key):
                _opengl_data_constructors[key](obj)
    return obj._opengl_data
