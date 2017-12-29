from ngsolve import *
from netgen.csg import *
import ngsolve.gui as GUI

ngsglobals.msg_level = 0

mesh = Mesh(unit_cube.GenerateMesh(maxh=0.1))

gui = GUI.GUI()
scene2 = GUI.MeshElementsScene(mesh)
gui.draw(scene2,name="Mesh Elements")
gui.run()
