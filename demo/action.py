
from ngsolve import *
from netgen.csg import *

ngsglobals.msg_level = 0



mesh = Mesh(unit_cube.GenerateMesh(maxh=0.1))

fes = H1(mesh)
u,v = fes.TnT()
laplace = SymbolicBFI(grad(u) * grad(v))
mass = SymbolicBFI(u * v)

def ShowLaplace(p):
    ei = mesh(*p).elementid
    element = fes.GetFE(ei)
    trafo = mesh.GetTrafo(ei)
    print("lap ", ei.nr, " = ")
    print(laplace.CalcElementMatrix(element,trafo))

def ShowMass(p):
    ei = mesh(*p).elementid
    element = fes.GetFE(ei)
    trafo = mesh.GetTrafo(ei)
    print("mass ", ei.nr, " = ")
    print(mass.CalcElementMatrix(element,trafo))

scene = Draw(mesh)
scene.addAction(ShowLaplace,name="ShowLaplace")
scene.addAction(ShowMass,name="ShowMass")
