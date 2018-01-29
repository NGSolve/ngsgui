from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import *
import ngsolve.gui as GUI
import sys

ngsglobals.msg_level = 0
nrefinements = 0
mesh_file = "/tmp/mesh_{}.vol".format(nrefinements)

cyls = Cylinder ( Pnt(-100, 0, 0), Pnt(200, 0, 0), 40 ) + Cylinder ( Pnt(100, -100, 100), Pnt(100, 200, 100), 40) + Cylinder ( Pnt(0, 100, -100), Pnt(0, 100, 200), 40)
sculpture = Sphere (Pnt(50, 50, 50), 80) - cyls - Sphere(Pnt(50, 50, 50), 50)
geom = CSGeometry()
geom.Add(sculpture)

mesh = Mesh(geom.GenerateMesh(maxh=20))
mesh.Curve(int(sys.argv[1]))
for i in range(nrefinements):
    print('refine')
    mesh.Refine()
print('save')
print('done', mesh.ne)

fes = L2(mesh, order=4, all_dofs_together=True)
gf = GridFunction(fes)
n = 0.04

print(fes.ndof,'ndofs')

with TaskManager():
    gf.Set(cos(n*x)*cos(n*y)*cos(n*z))

gui = GUI.GUI()
# scene = GUI.SolutionScene(gf)
scene = GUI.ClippingPlaneScene(gf, name="Solution")
gui.draw(scene)
scene1 = GUI.MeshScene(mesh, name="Mesh")
gui.draw(scene1)
scene2 = GUI.MeshElementsScene(mesh,active=False, name="Elements")
gui.draw(scene2)
gui.run()
# Draw(gf)
