from ngsolve import *
from netgen.geom2d import unit_square
import ngsolve.gui as GUI

ngsglobals.msg_level = 0
mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))

fes = L2(mesh, order=10, all_dofs_together=True)
gf = GridFunction(fes)
with TaskManager():
    gf.Set(sin(40*x)*sin(40*y))


gui = GUI.GUI()
scene = GUI.SolutionScene(gf)
scene.colormap_min = -1
scene.colormap_max = 1
scene.colormap_linear = True
gui.draw(scene)
gui.run()
