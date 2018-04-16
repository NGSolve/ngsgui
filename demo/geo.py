
from netgen.csg import *
from ngsolve import *

geo = CSGeometry()
geo.Add(Sphere(Pnt(0,0,0),1))
Draw(geo)
