from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import unit_cube
import ngsolve.gui as GUI

ngsglobals.msg_level = 0
# mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))
mesh = Mesh(unit_cube.GenerateMesh(maxh=0.4))

fes = L2(mesh, order=6, all_dofs_together=True)
gf = GridFunction(fes)
n = 10
with TaskManager():
    gf.Set(cos(n*x)*cos(n*y)*cos(n*z))

gui = GUI.GUI()
scene = GUI.SolutionScene(gf)
gui.draw(scene)
gui.run()
