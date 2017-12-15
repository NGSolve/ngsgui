from ngsolve import *
from netgen.geom2d import unit_square
from netgen.csg import unit_cube
import ngsolve.gui as GUI
import threading
import time

ngsglobals.msg_level = 8

from netgen.geom2d import SplineGeometry
geo = SplineGeometry()
geo.AddRectangle( (0, 0), (2, 0.41), bcs = ("wall", "outlet", "wall", "inlet"))
geo.AddCircle ( (0.2, 0.2), r=0.05, leftdomain=0, rightdomain=1, bc="cyl")
mesh = Mesh( geo.GenerateMesh(maxh=0.08))

mesh.Curve(3)

fes = L2(mesh, order=3, all_dofs_together=True)
gf_draw = GridFunction(fes)
gf_draw.Set(CoefficientFunction(0.0))

gui = GUI.GUI()

def work():
    nu = 0.001

    tau = 0.001
    tend = 10


    V = H1(mesh,order=3, dirichlet="wall|cyl|inlet")
    Q = H1(mesh,order=2)

    X = FESpace([V,V,Q])

    ux,uy,p = X.TrialFunction()
    vx,vy,q = X.TestFunction()

    div_u = grad(ux)[0]+grad(uy)[1]
    div_v = grad(vx)[0]+grad(vy)[1]

    stokes = nu*grad(ux)*grad(vx)+nu*grad(uy)*grad(vy)+div_u*q+div_v*p - 1e-10*p*q
    print(2)
    a = BilinearForm(X)
    print(2)
    a += SymbolicBFI(stokes)
    print(2)
    a.Assemble()
    print(2)

    # nothing here ...
    f = LinearForm(X)   
    f.Assemble()

    # gridfunction for the solution
    gfu = GridFunction(X)

    # parabolic inflow at bc=1:
    uin = 1.5*4*y*(0.41-y)/(0.41*0.41)
    gfu.components[0].Set(uin, definedon=mesh.Boundaries("inlet"))

    # solve Stokes problem for initial conditions:
    inv_stokes = a.mat.Inverse(X.FreeDofs())

    res = f.vec.CreateVector()
    res.data = f.vec - a.mat*gfu.vec
    gfu.vec.data += inv_stokes * res


    # matrix for implicit Euler 
    mstar = BilinearForm(X)
    mstar += SymbolicBFI(ux*vx+uy*vy + tau*stokes)
    mstar.Assemble()
    inv = mstar.mat.Inverse(X.FreeDofs(), inverse="sparsecholesky")

    # the non-linear term 
    conv = BilinearForm(X, nonassemble = True)
    conv += SymbolicBFI( CoefficientFunction( (ux,uy) ) * (grad(ux)*vx+grad(uy)*vy) )

    # for visualization
    velocity = CoefficientFunction (gfu.components[0:2])

    # implicit Euler/explicit Euler splitting method:
    t = 0
    for i in range(100):
            print ("t=", t, end="\r")

            conv.Apply (gfu.vec, res)
            res.data += a.mat*gfu.vec
            gfu.vec.data -= tau * inv * res    

            t = t + tau
            gf_draw.Set(Norm(velocity))
            gui.redraw()

# scene = GUI.ClippingPlaneScene(gf)
# gui.draw(scene)
# scene1 = GUI.MeshScene(mesh)
# gui.draw(scene1)

# scene1 = GUI.ClippingPlaneScene(gf_draw)
# gui.draw(scene1)
scene = GUI.SolutionScene(gf_draw)
gui.draw(scene)
print(1)

thread = threading.Thread(target=work, daemon=True)
thread.start()
gui.run()
print(1)

