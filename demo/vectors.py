from ngsolve import *
from netgen.csg import *

ngsglobals.msg_level = 0

mesh = Mesh(unit_cube.GenerateMesh(maxh=0.2))
mesh.Refine()

scene = Draw(CoefficientFunction((y-0.5,-(x-0.5),0.2*sqrt((y-0.5)**2+(x-0.5)**2))), mesh,name="Flow")
scene.setShowSurface(False)
scene.setShowVectors(True)
