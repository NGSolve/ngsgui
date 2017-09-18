from ngsolve import *
from netgen.geom2d import unit_square

ngsglobals.msg_level = 1

mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))

import ngui
ngui.Draw(mesh)
