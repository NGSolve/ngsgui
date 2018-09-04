from netgen.meshing import *
from netgen.csg import *
import ngsolve


def Tet():
    m = Mesh()
    m.dim = 3
    pnums = []
    pnums.append (m.Add (MeshPoint (Pnt(0,0,0))))
    pnums.append (m.Add (MeshPoint (Pnt(1,0,0))))
    pnums.append (m.Add (MeshPoint (Pnt(0,1,0))))
    pnums.append (m.Add (MeshPoint (Pnt(0,0,1))))


    m.Add (FaceDescriptor(surfnr=1,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=2,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=3,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=4,domin=1,bc=1))
    m.SetMaterial(1, "mat")

    m.Add (Element2D (1,[pnums[0],pnums[1], pnums[2]]))
    m.Add (Element2D (2,[pnums[0],pnums[1], pnums[3]]))
    m.Add (Element2D (3,[pnums[0],pnums[2], pnums[3]]))
    m.Add (Element2D (4,[pnums[1],pnums[2], pnums[3]]))

    m.Add (Element3D (1,pnums))
    return ngsolve.Mesh(m)

def Pyramid():
    m = Mesh()
    m.dim = 3
    pnums = []
    pnums.append (m.Add (MeshPoint (Pnt(0,0,0))))
    pnums.append (m.Add (MeshPoint (Pnt(1,0,0))))
    pnums.append (m.Add (MeshPoint (Pnt(1,1,0))))
    pnums.append (m.Add (MeshPoint (Pnt(0,1,0))))
    pnums.append (m.Add (MeshPoint (Pnt(0.5,0.5,1))))


    m.Add (FaceDescriptor(surfnr=1,domin=1,bc=1))
    m.SetMaterial(1, "mat")
    m.Add (Element2D (1,[pnums[0],pnums[1], pnums[4]]))
    m.Add (Element2D (1,[pnums[1],pnums[2], pnums[4]]))
    m.Add (Element2D (1,[pnums[2],pnums[3], pnums[4]]))
    m.Add (Element2D (1,[pnums[3],pnums[0], pnums[4]]))
    m.Add (Element2D (1,[pnums[0],pnums[1], pnums[2], pnums[3]]))

    m.Add (Element3D (1,pnums))
    return ngsolve.Mesh(m)
