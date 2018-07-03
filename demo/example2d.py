from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import unit_cube

ngsglobals.msg_level = 0
nrefinements = 1

mesh = Mesh(unit_square.GenerateMesh(maxh=4, quad_dominated=True))
# mesh = Mesh(unit_cube.GenerateMesh(maxh=0.3))
for i in range(nrefinements):
    print('refine')
    mesh.Refine()

fes = L2(mesh, order=2, all_dofs_together=True)
gf = GridFunction(fes)
n = 10

print(mesh.ne,'elements')
print(fes.ndof,'ndofs')

cf = cos(n*x)*cos(n*y)*cos(n*z)
cf = x+1.0j*y
cf = CoefficientFunction((x-1,y,x*y))
scene1 = Draw(x, mesh,'func', tab="foobar", sd=0, order=1)
# scene1 = Draw(mesh)
# print(scene1.values)
# from ngsolve.gui import ngui
# print(ngui.GetValues2(x,mesh, VOL, 0,1))
# scene2 = Draw(cf, mesh,'func 2', tab="foooo")
# scene3 = Draw(cf, mesh,'func 3', tab="foobar")
# scene4 = Draw(cf, mesh,'func 4', tab="baaaar")
