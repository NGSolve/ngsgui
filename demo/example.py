from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import unit_cube
import ngsolve.gui as GUI

ngsglobals.msg_level = 0
nrefinements = 2

mesh = Mesh(unit_cube.GenerateMesh(maxh=0.1))
for i in range(nrefinements):
    print('refine')
    mesh.Refine()

fes = L2(mesh, order=4, all_dofs_together=True)
gf = GridFunction(fes)
n = 40

print(mesh.ne,'elements')
print(fes.ndof,'ndofs')

with TaskManager():
    gf.Set(cos(n*x)*cos(n*y)*cos(n*z))

gui = GUI.GUI()
scene = GUI.ClippingPlaneScene(gf)
gui.draw(scene)
scene1 = GUI.MeshScene(mesh)
gui.draw(scene1)
gui.run()
