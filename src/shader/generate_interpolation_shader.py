import ngsolve as ngs
from ngsolve import ET, ElementTransformation

import sympy
from sympy import *
import sympy.printing.ccode as ccode
from scipy.special import legendre

# from numpy import * 
import numpy 
import time

def getReferenceRules(order, sd):
  n = S(order)*S(sd+1)+1;
  h = S(1)/(n-1);
  res = {}
  n2 = n*(n+1)//2
  n3 = n*(n+1)*(n+2)//6
  res[ngs.ET.SEGM] = [ (S(1)-i*h,S(0),S(0)) for i in range(n) ]
  res[ngs.ET.TRIG] = [ (    i*h,j*h,S(0)) for j in range(n) for i in range(n-j) ]
  res[ngs.ET.QUAD] = [ (    i*h,j*h,S(0)) for j in range(n) for i in range(n) ]
  res[ngs.ET.TET]  = [ (    i*h,j*h,k*h) for k in range(n) for j in range(n-k) for i in range(n-k-j) ]
  res[ngs.ET.HEX]  = [ (    i*h,j*h,k*h) for k in range(n) for j in range(n) for i in range(n) ]
  res[ngs.ET.PRISM]= [ (    i*h,j*h,k*h) for k in range(n) for j in range(n) for i in range(n-j) ]

  # no high order pyramids
  n = S(2);
  h = S(1)/(n-1);
  res[ngs.ET.PYRAMID]= [ (    i*h,j*h,k*h) for k in range(n) for j in range(n-k) for i in range(n-k) ]
  return res

functions = {}
functions[ET.SEGM] = """\
{type} EvaluateElement{vec}(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {{
    int n = subdivision+1;
    int N = ORDER*n+1;
    int values_per_element = N;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);

    int X = ORDER*x;

    int first = element*values_per_element + X;
    return EvaluateElementInternal{vec}(values, lam, first, component);
}}
"""
functions[ET.TRIG] = """\
{type} EvaluateElement{vec}(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {{
    int n = subdivision+1;
    int N = ORDER*n+1;
    int values_per_element = N*(N+1)/2;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);
    int y = int(lamn.y);
    int z = int(lamn.z);

    int X = ORDER*x;
    int Y = ORDER*y;
    int Z = ORDER*z;

    int first, dx, dy;
    if(lam.x+lam.y<1.0) {{ // lower left trig of quad
        first = element*values_per_element+getIndex(N,X,Y);
        dx = getIndex(N,X+1, Y)-getIndex(N,X,Y);
        dy = getIndex(N,X, Y+1)-getIndex(N,X,Y);
    }}
    else {{ // upper right trig of quad
        first = element*values_per_element+getIndex(N,X+ORDER,Y+ORDER);
        dx = getIndex(N,X, Y)-getIndex(N,X+1,Y);
        dy = getIndex(N,X, Y+ORDER-1)-getIndex(N,X,Y+ORDER);
        lam.x = 1-lam.x;
        lam.y = 1-lam.y;
        lam.z = 1-lam.x-lam.y;
    }}
    return EvaluateElementInternal{vec}(values, lam, first, dx, dy, component);
}}
"""

functions[ET.QUAD] = """\
{type} EvaluateElement{vec}(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {{
    int n = subdivision+1;
    int N = ORDER*n+1; // number of values on one edge
    int values_per_element = N*N;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);
    int y = int(lamn.y);
    int z = int(lamn.z);

    int X = ORDER*x;
    int Y = ORDER*y;
    int Z = ORDER*z;

    int first, dy;
    dy = N;
    first = element*values_per_element+Y*N+X;
    return EvaluateElementInternal{vec}(values, lam, first, dy, component);
}}
"""

functions[ET.TET] = """\
{type} EvaluateElement{vec}(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam, int component) {{
/*

  Coefficients are stored in a cube-like grid. Cut this cube in two prisms (1-3 and 5-7 are cutting lines) and divide the resulting prisms in 3 tets each. Each of the resulting tet has values assigned to do p-interpolation (i.e. 4 values for P1, 10 values for P2 etc.). This function determines to which subtet the point belongs and does the interpolation appropriately using the corresponding values.

          7+-----+6
          /|    /|
         / |   / |
       4+-----+5 |
        | 3+--|- +2 
        | /   | /
        |/    |/
       0+-----+1 
*/
  int n = subdivision+1;
  int N = ORDER*n+1;
  int values_per_element = N*(N+1)*(N+2)/6;
  vec3 lamn = lam*n;
  lam = lamn-floor(lamn);
  ivec3 s = ORDER*ivec3(lamn);

  ivec3 d = ivec3(1,1,1);
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  int special_order = 0;
  int first = element*values_per_element;
  if(lam.x+lam.y<1.0) {{ // first prism: 0,1,3,4,5,7
    if(lam.x+lam.y+lam.z<1.0) {{ // first tet in first prism 0,1,3,4
      // default settings, nothing to do
    }}
    else if(lam.x<lam.z) {{ // second tet in first prism 1,3,4,7
      z = 1-z;
      s.z+=ORDER;
      d.z = -1;
    }}
    else {{ // third tet in first prism 1,4,5,7
      x = 1-lam.x-lam.y;
      z = 1-lam.z-lam.y;
      s.z+=ORDER;
      s.x+=ORDER;
      d.x = -1;
      d.z = -1;
      special_order = 1;
    }}
  }}
  else {{ // second prism 1,2,3,5,6,7
    if(x+y+z>=2.0) {{ // first tet in second prism 2,5,6,7
      x = 1-x;
      y = 1-y;
      z = 1-z;
      d.x = -1;
      d.y = -1;
      d.z = -1;
      s.x += ORDER;
      s.y += ORDER;
      s.z += ORDER;
    }}
    else if(lam.z<lam.y) {{ // second tet in second prism 1,2,3,7
      x = 1-lam.x-lam.z;
      y = 1-lam.y;
      s.x+=ORDER;
      s.y+=ORDER;
      d.x = -1;
      d.y = -1;
      special_order = 2;
    }}
    else {{ // third tet in second prism 1,2,5,7
      x = 1-lam.x;
      y = 2-lam.x-lam.y-lam.z;
      z = lam.z+lam.x-1;
      s.x+=ORDER;
      s.y+=ORDER;
      d.x = -1;
      d.y = -1;
      special_order = 3;
    }}
  }}
  return EvaluateElementInternal{vec}( element, coefficients, N, d, s, special_order, vec3(x,y,z), component);
}}
"""

functions[ET.HEX] = """\
{type} EvaluateElement{vec}(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {{
    int n = subdivision+1;
    int N = ORDER*n+1; // number of values on one edge
    int values_per_element = N*N*N;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);
    int y = int(lamn.y);
    int z = int(lamn.z);

    int X = ORDER*x;
    int Y = ORDER*y;
    int Z = ORDER*z;

    int dy = N;
    int dz = N*N;
    int first = element*values_per_element+Z*dz+Y*dy+X;
    return EvaluateElementInternal{vec}(values, lam, first, dy, dz, component);
}}
"""

functions[ET.PRISM] = """\
{type} EvaluateElement{vec}(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {{
    int n = subdivision+1;
    int N = ORDER*n+1;
    int values_per_element = N*N*(N+1)/2;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);
    int y = int(lamn.y);
    int z = int(lamn.z);

    int X = ORDER*x;
    int Y = ORDER*y;
    int Z = ORDER*z;

    int first, dx, dy, dz;
    dz = N*(N+1)/2;
    if(lam.x+lam.y<1.0) {{ // lower left trig of quad
        first = element*values_per_element+getIndex(N,X,Y);
        dx = getIndex(N,X+1, Y)-getIndex(N,X,Y);
        dy = getIndex(N,X, Y+1)-getIndex(N,X,Y);
    }}
    else {{ // upper right trig of quad
        first = element*values_per_element+getIndex(N,X+ORDER,Y+ORDER);
        dx = getIndex(N,X, Y)-getIndex(N,X+1,Y);
        dy = getIndex(N,X, Y+ORDER-1)-getIndex(N,X,Y+ORDER);
        lam.x = 1-lam.x;
        lam.y = 1-lam.y;
    }}
    return EvaluateElementInternal{vec}(values, lam, first, dx, dy, dz, component);
}}
"""

functions[ET.PYRAMID] = """\
{type} EvaluateElement{vec}(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {{
    int N = 1+1;
    int values_per_element = N*(N+1)*(2*N+1)/6;
    int first = values_per_element*element;
    return EvaluateElementInternal{vec}(values, lam, first, component);
}}
"""

def getBasisFunction(et,i,j=0,k=0):
    if et==ET.SEGM:
        def phi(x,y,z):
            return  x**i
    elif et==ET.TRIG:
        def phi(x,y,z):
            return  x**i*y**j
    elif et==ET.QUAD:
        def phi(x,y,z):
            return  x**i*y**j
    elif et==ET.TET:
        def phi(x,y,z):
            return  x**i*y**j*z**k
    elif et==ET.HEX:
        def phi(x,y,z):
            return  x**i*y**j*z**k
    elif et==ET.PRISM:
        def phi(x,y,z):
            return  x**i*y**j*z**k
    elif et==ET.PYRAMID:
        def phi(x,y,z):
            return  x**i*y**j*z**k
    else:
        raise RuntimeError("unknown type: " + str(et))

    phi.i = i
    phi.j = j
    phi.k = k
    return phi

def getBasisFunctions(et, p):
    if et==ET.SEGM:
        return [getBasisFunction(et,i) for i in range(p+1)]
    if et==ET.TRIG:
        return [getBasisFunction(et,i,j) for i in range(p+1) for j in range(p+1-i)]
    if et==ET.QUAD:
        return [getBasisFunction(et,i,j) for i in range(p+1) for j in range(p+1)]
    if et==ET.TET:
        return [getBasisFunction(et,i,j,k) for i in range(p+1) for j in range(p+1-i) for k in range(p+1-i-j)]
    if et==ET.HEX:
        return [getBasisFunction(et,i,j,k) for i in range(p+1) for j in range(p+1) for k in range(p+1)]
    if et==ET.PRISM:
        return [getBasisFunction(et,i,j,k) for k in range(p+1) for j in range(p+1) for i in range(p+1-j)]
    if et==ET.PYRAMID:
        return [getBasisFunction(et,i,j,k) for k in range(1+1) for j in range(1+1-k) for i in range(1+1-k)]

def GetHeader(et, p, basis, scalar):
    comps = '[component]' if scalar else '.xyz'
    type_ = 'float' if scalar else 'vec3'
    vec = '' if scalar else 'Vec'
    if et==ET.SEGM:
        code = """\
{type} EvaluateElementInternal{vec}(samplerBuffer coefficients, vec3 lam, int first, int component) {{
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  {type} f[{ndof}];
  int offsety = 0;
  for (int i=0; i<={p}; i++) {{
    f[i] = getValue(coefficients, first+i){comps};
  }}
"""
    if et==ET.TRIG:
        code = """\
{type} EvaluateElementInternal{vec}(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {{
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  {type} f[{ndof}];
  int ii=0;
  int offsety = 0;
  for (int i=0; i<={p}; i++) {{
    int offsetx = 0;
    for (int j=0; j<={p}-i; j++) {{
      f[ii] = getValue(coefficients, first+offsetx+offsety){comps};
      offsetx += dx;
      ii++;
    }}
    offsety += dy-i;
  }}
"""
    if et==ET.QUAD:
        code = """\
{type} EvaluateElementInternal{vec}(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {{
  float x = lam.x;
  float y = lam.y;
  {type} f[{ndof}];
  int ii=0;
  for (int i=0; i<={p}; i++) {{
    for (int j=0; j<={p}; j++) {{
      f[ii] = getValue(coefficients, first+j+i*dy){comps};
      ii++;
    }}
  }}
"""
    if et==ET.TET:
        code = """\
{type}  EvaluateElementInternal{vec}(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {{
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  {type} f[{ndof}];
  if(special_order==0)
    for (int k=0; k<={p}; k++) for (int j=0; j<={p}-k; j++) for (int i=0; i<={p}-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k)){comps};
  if(special_order==1)
    for (int k=0; k<={p}; k++) for (int j=0; j<={p}-k; j++) for (int i=0; i<={p}-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k))){comps};
  if(special_order==2)
    for (int k=0; k<={p}; k++) for (int j=0; j<={p}-k; j++) for (int i=0; i<={p}-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k)){comps};
  if(special_order==3)
    for (int k=0; k<={p}; k++) for (int j=0; j<={p}-k; j++) for (int i=0; i<={p}-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k))){comps};
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
"""
    if et==ET.HEX:
        code = """\
{type} EvaluateElementInternal{vec}(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {{
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  {type} f[{ndof}];
  int ii=0;
  for (int i=0; i<={p}; i++) {{
    for (int j=0; j<={p}; j++) {{
      for (int k=0; k<={p}; k++) {{
        f[ii] = getValue(coefficients, first+k+j*dy+i*dz){comps};
        ii++;
      }}
    }}
  }}
"""
    if et==ET.PRISM:
        code = """\
{type} EvaluateElementInternal{vec}(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {{
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  {type} f[{ndof}];
  int ii=0;
  for (int k=0; k<={p}; k++) {{
      int offsety = 0;
      for (int i=0; i<={p}; i++) {{
        int offsetx = 0;
        for (int j=0; j<={p}-i; j++) {{
          f[ii] = getValue(coefficients, first+offsetx+offsety){comps};
          offsetx += dx;
          ii++;
        }}
        offsety += dy-i;
      }}
    first += dz;
  }}
"""
    if et==ET.PYRAMID:
        code = """\
{type} EvaluateElementInternal{vec}(samplerBuffer coefficients, vec3 lam, int first, int component) {{
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  {type} f[{ndof}];
  int ii=0;
  for (int k=0; k<=4; k++)
      f[k] = getValue(coefficients, first+k){comps};
  /*
  for (int k=0; k<={p}; k++) {{
      int offsety = 0;
      for (int i=0; i<={p}-k; i++) {{
        int offsetx = 0;
        for (int j=0; j<={p}-k; j++) {{
          f[ii] = getValue(coefficients, first+offsetx+offsety){comps};
          offsetx += 1;
          ii++;
        }}
        offsety += {p}+1;
      }}
    first += ({p}-k+1)*({p}-k+1);
  }}
  */
"""
    return code.format(comps=comps, p=p, type=type_, vec=vec, ndof=len(basis))

def GenerateInterpolationFunction(et, p, scalar):
    ir = getReferenceRules(p, 0)[et]
    basis = getBasisFunctions(et, p)
    ndof = len(basis)
    nips = len(ir)
    if nips!=ndof:
        raise RuntimeError("Number of ips ({}) and dofs ({}) doesn't match for element {} and order {}".format(nips, ndof, et,p))
#     mat = zeros((ndof,ndof), numpy.float64)
    mat = zeros(ndof)
    for i,ip in enumerate(ir):
        for j,phi in enumerate(basis):
            mat[i,j] = phi(*ip)

    
    invmat = mat**-1
#     invmat = numpy.linalg.inv(mat)
    code = "#ifdef {}\n".format(str(et).replace('.','_'))
    code += "#if ORDER=={}\n".format(p)
    code += GetHeader(et, p, basis, scalar)
    x, y, z = symbols("x y z")
    func = 0*x
    values = symbols(("f[{}] "*ndof).format(*range(ndof)))
    for i in range(ndof):
        func += basis[i](x,y,z)*sum([invmat[i,j]*values[j] for j in range(ndof)])
    code += "  return " + str(ccode(horner(func,wrt=x))) + ";\n"
    code += "}\n"
    code += "#endif\n"
    code += "#endif\n\n"
    return code

code = "#line 1\n"


from multiprocessing import Pool

if __name__ == '__main__':
    p = Pool(24)

    ets = [ET.SEGM, ET.TRIG, ET.TET, ET.QUAD, ET.HEX, ET.PRISM, ET.PYRAMID]
    maxp = 3
    args = [(et,p,scalar) for et in ets for p in range(1,maxp+1) for scalar in [True,False]]

    codes = p.starmap(GenerateInterpolationFunction, args)
#     codes = []
#     for arg in args:
#         codes.append(GenerateInterpolationFunction(*arg))
    code += ''.join(codes)
    for et in ets:
        for scalar in [True, False]:
            code += '#ifdef {}\n'.format(str(et).replace('.','_'))
            code += functions[et].format(type='float' if scalar else 'vec3', vec='' if scalar else 'Vec')
            code += '#endif\n'

    open('generated_interpolation.inc', 'w').write(code)
