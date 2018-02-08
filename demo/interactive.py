from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import unit_cube
import ngsolve.gui as GUI
import threading
import time

ngsglobals.msg_level = 0

mesh = Mesh(unit_cube.GenerateMesh(maxh=0.3))

fes = L2(mesh, order=3, all_dofs_together=True)
gf = GridFunction(fes)

running=True

gui = GUI.GUI()

def work():
    n = 7
    s = 0.0
    t = threading.currentThread()
    with TaskManager():
        while True:
            s += 0.05
            gf.Set(cos(n*x+s)*cos(n*y+s)*cos(n*z+s))
            time.sleep(0.01)
            gui.redraw()

# scene = GUI.ClippingPlaneScene(gf)
# gui.draw(scene)
# scene1 = GUI.MeshScene(mesh)
# gui.draw(scene1)

scene1 = GUI.ClippingPlaneScene(gf)
win = gui.make_window()
win.draw(scene1)
scene = GUI.SolutionScene(gf)
win.draw(scene)

thread = threading.Thread(target=work, daemon=True)
thread.start()
gui.run()
