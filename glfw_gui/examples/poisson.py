# solve the Poisson equation -Delta u = f
# with Dirichlet boundary condition u = 0

from ngsolve import *
from netgen.geom2d import unit_square

ngsglobals.msg_level = 1

# generate a triangular mesh of mesh-size 0.2
mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))
exact = 16*x*(1-x)*y*(1-y)

import ngui
fesdraw = L2(mesh, order=4, all_dofs_together=True)
print('ndof', fesdraw.ndof)
gfdraw = GridFunction(fesdraw)
# gfdraw.Set(sin(100*x)*sin(100*y))
gfdraw.Set(exact)
# gfdraw.Set(x*y)
ngui.Draw(gfdraw)
# ngui.Draw(mesh)
# for el in fesdraw.Elements():
#     print(el.nr, el.vertices, el.dofs)
