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

def Prism():
    m = Mesh()
    m.dim = 3
    pnums = []
    pnums.append (m.Add (MeshPoint (Pnt(0,0,0))))
    pnums.append (m.Add (MeshPoint (Pnt(1,0,0))))
    pnums.append (m.Add (MeshPoint (Pnt(0,1,0))))
    pnums.append (m.Add (MeshPoint (Pnt(0,0,1))))
    pnums.append (m.Add (MeshPoint (Pnt(1,0,1))))
    pnums.append (m.Add (MeshPoint (Pnt(0,1,1))))


    m.Add (FaceDescriptor(surfnr=1,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=2,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=3,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=4,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=5,domin=1,bc=1))
    m.SetMaterial(1, "mat")

    m.Add (Element2D (1,[pnums[0],pnums[1], pnums[2]]))
    m.Add (Element2D (2,[pnums[3],pnums[4], pnums[5]]))
    m.Add (Element2D (3,[pnums[0],pnums[1], pnums[4], pnums[3]]))
    m.Add (Element2D (4,[pnums[1],pnums[2], pnums[5], pnums[4]]))
    m.Add (Element2D (5,[pnums[2],pnums[0], pnums[3], pnums[5]]))

    m.Add (Element3D (1,pnums))
    return ngsolve.Mesh(m)

def Hex():
    m = Mesh()
    m.dim = 3
    pnums = []
    pnums.append (m.Add (MeshPoint (Pnt(0,0,0))))
    pnums.append (m.Add (MeshPoint (Pnt(1,0,0))))
    pnums.append (m.Add (MeshPoint (Pnt(1,1,0))))
    pnums.append (m.Add (MeshPoint (Pnt(0,1,0))))
    pnums.append (m.Add (MeshPoint (Pnt(0,0,1))))
    pnums.append (m.Add (MeshPoint (Pnt(1,0,1))))
    pnums.append (m.Add (MeshPoint (Pnt(1,1,1))))
    pnums.append (m.Add (MeshPoint (Pnt(0,1,1))))


    m.Add (FaceDescriptor(surfnr=1,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=2,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=3,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=4,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=5,domin=1,bc=1))
    m.Add (FaceDescriptor(surfnr=6,domin=1,bc=1))
    m.SetMaterial(1, "mat")

    m.Add (Element2D (1,[pnums[0],pnums[1], pnums[2], pnums[3]]))
    m.Add (Element2D (2,[pnums[0],pnums[1], pnums[5], pnums[4]]))
    m.Add (Element2D (3,[pnums[1],pnums[2], pnums[6], pnums[5]]))
    m.Add (Element2D (4,[pnums[2],pnums[3], pnums[7], pnums[6]]))
    m.Add (Element2D (5,[pnums[3],pnums[0], pnums[4], pnums[7]]))
    m.Add (Element2D (6,[pnums[4],pnums[5], pnums[6], pnums[7]]))

    m.Add (Element3D (1,pnums))
    return ngsolve.Mesh(m)

meshes_3d = [
        ('tet', Tet()),
        ('pyramid', Pyramid()),
        ('prism', Prism()),
        ('hex', Hex())
        ]

for name, mesh in meshes_3d:
    from ngsolve import x,y,z
    ngsolve.Draw(3*x+y**2+z, mesh, name=name)
