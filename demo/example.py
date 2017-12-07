from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import unit_cube
import ngsolve.gui as GUI

ngsglobals.msg_level = 0
nrefinements = 2
mesh_file = "/tmp/mesh_{}.vol".format(nrefinements)

try:
    print("load mesh...")
    mesh = Mesh(mesh_file)
    print("done", mesh.ne)
except:
    print("mesh not found, generate it...")
#     mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))
    mesh = Mesh(unit_cube.GenerateMesh(maxh=0.1))
    for i in range(nrefinements):
        print('refine')
        mesh.Refine()
    print('save')
#     mesh.ngmesh.Save(mesh_file)
    print('done', mesh.ne)
# input()

fes = L2(mesh, order=4, all_dofs_together=True)
gf = GridFunction(fes)
n = 40

print(fes.ndof,'ndofs')

with TaskManager():
    gf.Set(cos(n*x)*cos(n*y)*cos(n*z))

gui = GUI.GUI()
# scene = GUI.SolutionScene(gf)
scene = GUI.ClippingPlaneScene(gf)
gui.draw(scene)
scene1 = GUI.MeshScene(mesh)
gui.draw(scene1)
gui.run()
# Draw(gf)
