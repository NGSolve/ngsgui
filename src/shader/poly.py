import sympy
from sympy import *
import sympy.printing.ccode as ccode
from scipy.special import legendre

from numpy import * 
import numpy 
import time

legs = []
for i in range(20):
    legs.append(legendre(i).coeffs)

log_time = {}
def timeit(method):
    def timed(*args, **kw):
        ts = time.time()
        result = method(*args, **kw)
        te = time.time()
        name = method.__name__
        if not name in log_time:
            log_time[name] = 0
        log_time[name] += (te - ts) * 1000
        return result
    return timed

@timeit
def evalLegendre(order, x):
    leg = legendre(order)
    return sum( [leg.coeffs[order-i]*(x**i) for i in range(order+1)] )


@timeit
def getBasisFunction1(order, i,j):
    def phi(x,y):
        return  x**j*y**i
    return phi

@timeit
def getBasisFunction2(order, i,j):
    def phileg(x,y):
        return  evalLegendre(j,x)*evalLegendre(i,y)
    return phileg

@timeit
def getBasisFunction3d(order, i,j,l):
    def phi(x,y,z):
        return  x**l*y**j*z**i
#     def phileg(x,y,z):
#         return  evalLegendre(j,x)*evalLegendre(i,y)*evalLegendre(l,z)
    return phi

@timeit
def getBasisPoly(order, i,j):
    coeffs = [0]*((order+1)*(order+2)//2)
    k = 0
    for i in range(order):
        for j in range(order-i):
            coeffs[k] = legs[i]*legs[j]
            k+=1
    def phileg(x,y):
        return  evalLegendre(j,x)*evalLegendre(i,y)
    return phileg


@timeit
def evalBasisFunction2(order, i,j, x, y):
    return  evalLegendre(j,x)*evalLegendre(i,y)


@timeit
def invert(mat):
    return numpy.linalg.inv(mat)


tcode  = """float Interpolate{el}(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam) {{
  int trigx=0;
  int trigy=0;
  int dy=1;
  getSubTrigStart(order, subdivision, lam, trigx, trigy, dy);
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float value = 0.0;
"""

@timeit
def generateFunc(dim, max_order, b):
    x, y, z = symbols("x y z")
    code = tcode.format(el=["Segm","Trig","Tet"][dim-1])
    for order in range(1,max_order+1):
        code += "  if (order=={}) {{\n".format(order)
#         if dim==1:
#             ndofs = order+1
#             dofs = symbols(("p{} "*ndofs).format(*range(ndofs)))
#             values = symbols(("f[{}] "*ndofs).format(*range(ndofs)))
#             basis = []
#             kto2d = []
#             for i in range(order+1):
#                 basis.append(b(order,0,i))
#                 kto2d.append((i/(order),0))
#             rhs = []
#             mat = zeros((ndofs,ndofs), numpy.float64)
#             for l in range(ndofs):
#                 for k in range(ndofs):
#                     mat[l][k] = basis[k](*kto2d[l]) #.subs([(x, kto2d[l][0]) , (y,kto2d[l][1])])
#             inv = invert(mat)
#             code = code+"    float f[{}];\n".format(ndofs)
#             for d in range(ndofs):
#                 code += "    f[{d}] = loadValue({npoints}*0+{d});\n".format(d=d, npoints=ndofs)
        if dim==2:
            ndofs = (order+1)*(order+2)//2
            dofs = symbols(("p{} "*ndofs).format(*range(ndofs)))
            values = symbols(("f[{}] "*ndofs).format(*range(ndofs)))
            basis = []
            kto2d = []
            for i in range(order+1):
                for j in range(order+1-i):
                    # monomial basis functions
                    basis.append(b(order,i,j))
                    kto2d.append((i,j))
            rhs = []
            mat = zeros((ndofs,ndofs), numpy.float64)
            for l in range(ndofs):
                i,j = kto2d[l]
                for k in range(ndofs):
                    mat[l][k] = basis[k](j/order, i/order) #.subs([(x, kto2d[l][0]) , (y,kto2d[l][1])])
            inv = invert(mat)
            code = code+"    float f[{}];\n".format(ndofs)
            for d in range(ndofs):
                i,j = kto2d[d]
                code += "    f[{d}] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, {j},{i});\n".format(d=d, i=i, j=j)
            func = 0*x
            for i in range(ndofs):
                func += basis[i](x,y)*sum([inv[i][j]*values[j] for j in range(ndofs)])
        if dim==3:
            ndofs = (order+1)*(order+2)*(order+3)//6
            dofs = symbols(("p{} "*ndofs).format(*range(ndofs)))
            values = symbols(("f[{}] "*ndofs).format(*range(ndofs)))
            basis = []
            kto3d = []
            for i in range(order+1):
                for j in range(order+1-i):
                    for l in range(order+1-i-j):
                        # monomial basis functions
                        basis.append(b(order,i,j,l))
                        kto3d.append((i,j,l))
            rhs = []
            mat = zeros((ndofs,ndofs), numpy.float64)
            for row in range(ndofs):
                i,j,l = kto3d[row]
                for col in range(ndofs):
                    mat[row][col] = basis[col](l/order, j/order, i/order) #.subs([(x, kto2d[l][0]) , (y,kto2d[l][1])])
            inv = invert(mat)
            code = code+"    float f[{}];\n".format(ndofs)
            for d in range(ndofs):
                i,j,l = kto3d[d]
                code += "    f[{d}] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, {l},{j},{i});\n".format(d=d, i=i, j=j, l=l)

            func = 0*x
            for i in range(ndofs):
                func += basis[i](x,y,z)*sum([inv[i][j]*values[j] for j in range(ndofs)])
        code += "    value = " + ccode(simplify(func)) + ";\n"
        code += "  }\n"
    code += "  return value;\n"
    code += "}\n"
    return code

# f = generateFunc(1,2)
f1 = generateFunc(2,4, getBasisFunction1)
f2 = generateFunc(3,4, getBasisFunction3d)

print(f1)
print(f2)
# print(simplify(f2-f1))

# for f in log_time:
#     print(f, log_time[f])
