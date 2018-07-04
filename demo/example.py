from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import *

ngsglobals.msg_level = 7
nrefinements = 2

def MakeGeometry():
    geometry = CSGeometry()
    box = OrthoBrick(Pnt(-1,-1,-1),Pnt(2,1,2)).bc("outer")

    core = OrthoBrick(Pnt(0,-0.05,0),Pnt(0.8,0.05,1))- \
           OrthoBrick(Pnt(0.1,-1,0.1),Pnt(0.7,1,0.9))- \
           OrthoBrick(Pnt(0.5,-1,0.4),Pnt(1,1,0.6)).maxh(0.2).mat("core")
    
    coil = (Cylinder(Pnt(0.05,0,0), Pnt(0.05,0,1), 0.3) - \
            Cylinder(Pnt(0.05,0,0), Pnt(0.05,0,1), 0.15)) * \
            OrthoBrick (Pnt(-1,-1,0.3),Pnt(1,1,0.7)).maxh(0.2).mat("coil")
    
    geometry.Add ((box-core-coil).mat("air"))
    geometry.Add (core)
    geometry.Add (coil)
    return geometry



mesh = Mesh(unit_cube.GenerateMesh(maxh=0.2))
# ngmesh = MakeGeometry().GenerateMesh(maxh=0.3)
# mesh = Mesh(ngmesh)
for i in range(nrefinements):
    print('refine')
    mesh.Refine()

# mesh.Curve(2)
print(mesh.ne,'elements')
# Draw(mesh)

n = 5
cf = cos(n*x)*cos(n*y)*cos(n*z)
cf = x+1j*y
# cf = (x-0.5)**2+(y-0.5)**2+(z-0.5)**2

Draw(cf, mesh,'sldkf')
