from ngsolve import *
from netgen.geom2d import unit_square

ngsglobals.msg_level = 1

mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))

import ngui
gui = ngui.GUI()
gui.AddScene(ngui.MeshScene(mesh))
while 1:
    gui.Update()
    gui.Render()
    gui.Update()
    gui.Render()
input()
