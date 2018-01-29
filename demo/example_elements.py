from ngsolve import *
from netgen.csg import *
import ngsolve.gui as GUI

ngsglobals.msg_level = 0

def Coil():
    geometry = CSGeometry()
    box = OrthoBrick(Pnt(-1,-1,-1),Pnt(2,1,2)).bc("outer")

    core = OrthoBrick(Pnt(0,-0.05,0),Pnt(0.8,0.05,1))- \
           OrthoBrick(Pnt(0.1,-1,0.1),Pnt(0.7,1,0.9))- \
           OrthoBrick(Pnt(0.5,-1,0.4),Pnt(1,1,0.6)).maxh(0.2).mat("core")

    coil = (Cylinder(Pnt(0.05,0,0), Pnt(0.05,0,1), 0.3) - \
            Cylinder(Pnt(0.05,0,0), Pnt(0.05,0,1), 0.15)) * \
            OrthoBrick (Pnt(-1,-1,0.3),Pnt(1,1,0.7)).maxh(0.2).mat("coil")

    geometry.Add ((box-core-coil).mat("air"))
    geometry.Add (core)
    geometry.Add (coil)
    return geometry


mesh = Mesh(Coil().GenerateMesh(maxh=0.3))
# mesh = Mesh(unit_cube.GenerateMesh(maxh=0.1))

gui = GUI.GUI()
scene2 = GUI.MeshElementsScene(mesh,name="Mesh Elements")
gui.draw(scene2)
gui.run()
