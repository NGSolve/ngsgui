from netgen.csg import *
from netgen.geom2d import unit_square
from ngsolve import *

geometry = CSGeometry()
p1 = Plane(Pnt(0,0,0),Vec(0,0,-1))
p2 = Plane(Pnt(0,0,0),Vec(0,-1,0))
p3 = Plane(Pnt(0,0,0),Vec(-1,0,0))
p4 = Plane(Pnt(1,0,0),Vec(1,1,1))
geometry.Add (p1*p2*p3*p4)

ngmesh = geometry.GenerateMesh(maxh=5)
mesh = Mesh(ngmesh)
cf = x
s = Draw(cf,mesh, sd=2, order=2)

print('vertices')
for v in mesh.vertices:
    print(v.nr,v.point)

print('elements')
for el in mesh.Elements(VOL):
    print(el.vertices)

