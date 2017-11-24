from ngsolve import *
from netgen.geom2d import unit_square
import ngui
import gui_pyside
import threading
import gl

try:
    from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
    from PySide2.QtCore import QThread
    print("Using PySide2")
except:
    from PyQt5 import QtCore, QtGui, QtWidgets, QtOpenGL
    from PyQt5.QtCore import QThread
    print("Using PyQt5")

ngsglobals.msg_level = 0
mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))

fes = L2(mesh, order=10, all_dofs_together=True)
gf = GridFunction(fes)
with TaskManager():
    gf.Set(sin(40*x)*sin(40*y))


gui = gui_pyside.GUI()
scene = gl.SolutionScene(gf)
scene.colormap_min = -1
scene.colormap_max = 1
scene.colormap_linear = True
gui.draw(scene)
gui.run()
