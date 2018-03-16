from netgen.meshing import *
from netgen.csg import *

m = Mesh()
m.dim = 1
nel = 10
pnums = []
for i in range(0, nel+1):
    pnums.append (m.Add (MeshPoint (Pnt(-1+2*i/nel, 0, 0))))

for i in range(0,nel):
    m.Add (Element1D ([pnums[i],pnums[i+1]], index=1))

m.Add (Element0D (pnums[0], index=1))
m.Add (Element0D (pnums[nel], index=2))    

from ngsolve import *
mesh = Mesh(m)

import ngsolve.gui as GUI

gui = GUI.GUI()
scene = GUI.SolutionScene(exp(-x*x), mesh,name="Function")
gui.draw(scene)
gui.run()
