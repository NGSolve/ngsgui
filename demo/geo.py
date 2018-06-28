
from netgen.csg import *
from ngsolve import *
from netgen.geom2d import *

geo = unit_square
# geo = CSGeometry()
# geo.Add(Sphere(Pnt(0,0,0),1).bc("sph1") * Sphere(Pnt(1,0,0),1).bc("sph2"))
Draw(geo)
