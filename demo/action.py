
from ngsolve import *
from netgen.csg import *
import ngsolve.gui as GUI

ngsglobals.msg_level = 0



mesh = Mesh(unit_cube.GenerateMesh(maxh=0.1))

fes = H1(mesh)
u,v = fes.TnT()
bfi = SymbolicBFI(grad(u) * grad(v))

def ShowElementMatrix(p):
    ei = mesh(*p).elementid
    element = fes.GetFE(ei)
    trafo = mesh.GetTrafo(ei)
    print("elmat ", ei.nr, " = ")
    print(bfi.CalcElementMatrix(element,trafo))

gui = GUI.GUI()
scene2 = GUI.MeshElementsScene(mesh)
scene2.addAction(ShowElementMatrix,name="ShowElementMatrix")
gui.draw(scene2,name="Mesh Elements")
gui.run()
