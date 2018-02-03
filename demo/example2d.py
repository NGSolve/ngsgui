from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import unit_cube

ngsglobals.msg_level = 0
nrefinements = 1

mesh = Mesh(unit_square.GenerateMesh(maxh=1.3))
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

# Draw(cf, mesh, "lskjdf")
import ngsolve.gui as GUI
gui = GUI.GUI()
scene = GUI.SolutionScene(cf, mesh)
gui.draw(scene)
scene2 = GUI.MeshScene(mesh,surface=False)
gui.draw(scene2)
gui.run()
