from netgen.csg import *

geo = CSGeometry()
left = Plane(Pnt(0,0,0),Vec(-1,0,0))
right = Plane(Pnt(1,0,0),Vec(1,0,0))
brick = OrthoBrick(Pnt(-1,0,0),Pnt(2,1,1)).bc("outer")
cube = brick * left * right
geo.Add(cube)
geo.PeriodicSurfaces(left,right)

from ngsolve import *
mesh = Mesh(geo.GenerateMesh(maxh=0.3))
Draw(mesh)
