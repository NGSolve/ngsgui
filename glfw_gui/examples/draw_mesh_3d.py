from ngsolve import *
from netgen.csg import unit_cube

ngsglobals.msg_level = 1

mesh = Mesh(unit_cube.GenerateMesh(maxh=0.2))

import ngui
ngui.Draw(mesh)
