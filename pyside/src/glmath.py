from ngsolve.bla import Vector, Matrix
from math import tan,sin,cos, exp

# default parameters for LookAt
_eye_default = Vector(3)
_eye_default[:] = 0
_eye_default[2] = 6
_center_default = Vector(3)
_center_default[:] = 0
_up_default = Vector(3)
_up_default[:] = 0
_up_default[1] = 1

def Dot(a,b):
    res = 0.0
    for i in range(len(a)):
        res += a[i]*b[i]
    return res

def Cross(a,b):
    res = Vector(3)
    res[0] = a[1]*b[2]-a[2]*b[1]
    res[1] = a[2]*b[0]-a[0]*b[2]
    res[2] = a[0]*b[1]-a[1]*b[0]
    return res

def Identity():
    m = Matrix(4,4)
    m[:,:] = 0.0
    m[0,0] = 1.0
    m[1,1] = 1.0
    m[2,2] = 1.0
    m[3,3] = 1.0
    return m

def RotateX(angle):
    s = sin(angle)
    c = cos(angle)
    res = Identity()
    res[1,1] = c
    res[2,2] = c
    res[1,2] = s
    res[2,1] =-s
    return res

def RotateY(angle):
    s = sin(angle)
    c = cos(angle)
    res = Identity()
    res[0,0] = c
    res[2,2] = c
    res[2,0] = s
    res[0,2] =-s
    return res

def RotateZ(angle):
    s = sin(angle)
    c = cos(angle)
    res = Identity()
    res[0,0] = c
    res[1,1] = c
    res[1,0] = s
    res[0,1] =-s
    return res

def Ortho(l, r, b, t, n, f):
    m = Matrix(4,4)
    m[:,:] = 0.0
    m[0,0] = 2.0/(r-l)
    m[1,1] = 2.0/(t-b)
    m[2,2] = -2.0/(f-n)
    m[3,0] = -(r+l)/(r-l)
    m[3,1] = -(t+b)/(t-b)
    m[3,2] = -(f+n)/(f-n)
    m[3,3] = 1.0
    return m

def Perspective(y_fov, aspect, n, f):
    a = 1.0 / tan(y_fov / 2.0)

    m = Matrix(4,4)
    m[:,:] = 0.0
    m[0,0] = a / aspect
    m[1,1] = a
    m[2,2] = -((f + n) / (f - n))
    m[3,2] = -1.0
    m[2,3] = -((2.0 * f * n) / (f - n))
    return m

def Translate(x, y, z=0.0):
    res = Identity()
    res[0,3] = x
    res[1,3] = y
    res[2,3] = z
    return res

def Scale(s):
    res = Identity();
    res[0,0] = s;
    res[1,1] = s;
    res[2,2] = s;
    return res

def LookAt(eye=_eye_default, center=_center_default, up=_up_default):
    f = center - eye
    f = f[:]*(1.0/f.Norm())
             
    s = Cross(f,up)
    s = s[:]*(1.0/s.Norm())

    t = Cross(s,f)

    m = Identity()
    m[0:3,0] = s
    m[0:3,1] = t
    m[0:3,2] = -1.0*f

    tmp = Vector(4)
    tmp[0:3] = -1.0*eye
    tmp[3] = 0
    m[3,:] = m[3,:] + m*tmp

    return m



