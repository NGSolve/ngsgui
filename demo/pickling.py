
from ngsolve import *
from netgen.geom2d import unit_square
from ngsolve.gui.scenes import *
import pickle

ngsglobals.msg_level = 0

mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))

cf = x*y
# Draw(cf,mesh,"cf")
# Draw(mesh)

scene = SolutionScene(cf, mesh)
print("draw scene")
Draw(scene)
print("start pickling")
dump = pickle.dumps(scene)
print("pickle worked")
load = pickle.loads(dump)
print("load worked")

Draw(load)
