from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import unit_cube

ngsglobals.msg_level = 1

mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))

fes = L2(mesh, order=10, all_dofs_together=True)
gf = GridFunction(fes)
gf.Set(sin(40*x)*sin(40*y))

import ngui
gui = ngui.GUI()
gui.AddScene(ngui.SolutionScene(gf))
gui.Update()
gui.Render()
for i in range(100):
    gui.Update()
    gui.Render()
input()
