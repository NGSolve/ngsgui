#if ORDER==1
float InterpolateTrig(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[3];
  int ii=0;
  int offsety = 0;
  for (int i=0; i<=1; i++) {
    int offsetx = 0;
    for (int j=0; j<=1-i; j++) {
      f[ii] = getValue(coefficients, first+offsetx+offsety)[component];
      offsetx += dx;
      ii++;
    }
    offsety += dy-i;
  }
  return 1.0*f[0] - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]);
}
#endif

#if ORDER==2
float InterpolateTrig(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[6];
  int ii=0;
  int offsety = 0;
  for (int i=0; i<=2; i++) {
    int offsetx = 0;
    for (int j=0; j<=2-i; j++) {
      f[ii] = getValue(coefficients, first+offsetx+offsety)[component];
      offsetx += dx;
      ii++;
    }
    offsety += dy-i;
  }
  return 1.0*f[0] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]);
}
#endif

float InterpolateTrig(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    if(lam.x+lam.y<1.0) { // lower left trig of quad
        first = element*values_per_element+getIndex(N,X,Y);
        dx = getIndex(N,X+1, Y)-getIndex(N,X,Y);
        dy = getIndex(N,X, Y+1)-getIndex(N,X,Y);
    }
    else { // upper right trig of quad
        first = element*values_per_element+getIndex(N,X+ORDER,Y+ORDER);
        dx = getIndex(N,X, Y)-getIndex(N,X+1,Y);
        dy = getIndex(N,X, Y+ORDER-1)-getIndex(N,X,Y+ORDER);
        lam.x = 1-lam.x;
        lam.y = 1-lam.y;
        lam.z = 1-lam.x-lam.y;
    }
    return InterpolateTrig(values, lam, first, dx, dy, component);
}
#if ORDER==1
vec3 InterpolateTrigVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[3];
  int ii=0;
  int offsety = 0;
  for (int i=0; i<=1; i++) {
    int offsetx = 0;
    for (int j=0; j<=1-i; j++) {
      f[ii] = getValue(coefficients, first+offsetx+offsety).xyz;
      offsetx += dx;
      ii++;
    }
    offsety += dy-i;
  }
  return 1.0*f[0] - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]);
}
#endif

#if ORDER==2
vec3 InterpolateTrigVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[6];
  int ii=0;
  int offsety = 0;
  for (int i=0; i<=2; i++) {
    int offsetx = 0;
    for (int j=0; j<=2-i; j++) {
      f[ii] = getValue(coefficients, first+offsetx+offsety).xyz;
      offsetx += dx;
      ii++;
    }
    offsety += dy-i;
  }
  return 1.0*f[0] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]);
}
#endif

vec3 InterpolateTrigVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    if(lam.x+lam.y<1.0) { // lower left trig of quad
        first = element*values_per_element+getIndex(N,X,Y);
        dx = getIndex(N,X+1, Y)-getIndex(N,X,Y);
        dy = getIndex(N,X, Y+1)-getIndex(N,X,Y);
    }
    else { // upper right trig of quad
        first = element*values_per_element+getIndex(N,X+ORDER,Y+ORDER);
        dx = getIndex(N,X, Y)-getIndex(N,X+1,Y);
        dy = getIndex(N,X, Y+ORDER-1)-getIndex(N,X,Y+ORDER);
        lam.x = 1-lam.x;
        lam.y = 1-lam.y;
        lam.z = 1-lam.x-lam.y;
    }
    return InterpolateTrigVec(values, lam, first, dx, dy, component);
}
#if ORDER==1
float  InterpolateTet(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  float f[4];
  if(special_order==0)
    for (int k=0; k<=1; k++) for (int j=0; j<=1-k; j++) for (int i=0; i<=1-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==1)
    for (int k=0; k<=1; k++) for (int j=0; j<=1-k; j++) for (int i=0; i<=1-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k)))[component];
  if(special_order==2)
    for (int k=0; k<=1; k++) for (int j=0; j<=1-k; j++) for (int i=0; i<=1-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==3)
    for (int k=0; k<=1; k++) for (int j=0; j<=1-k; j++) for (int i=0; i<=1-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k)))[component];
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]) - 1.0*z*(f[0] - f[3]);
}
#endif

#if ORDER==2
float  InterpolateTet(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  float f[10];
  if(special_order==0)
    for (int k=0; k<=2; k++) for (int j=0; j<=2-k; j++) for (int i=0; i<=2-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==1)
    for (int k=0; k<=2; k++) for (int j=0; j<=2-k; j++) for (int i=0; i<=2-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k)))[component];
  if(special_order==2)
    for (int k=0; k<=2; k++) for (int j=0; j<=2-k; j++) for (int i=0; i<=2-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==3)
    for (int k=0; k<=2; k++) for (int j=0; j<=2-k; j++) for (int i=0; i<=2-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k)))[component];
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) + 4.0*x*z*(f[0] - f[1] - f[6] + f[7]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) + 4.0*y*z*(f[0] - f[3] - f[6] + f[8]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]) + pow(z, 2)*(2.0*f[0] - 4.0*f[6] + 2.0*f[9]) - z*(3.0*f[0] - 4.0*f[6] + 1.0*f[9]);
}
#endif

float InterpolateTet(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam, int component) {
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
  if(lam.x+lam.y<1.0) { // first prism: 0,1,3,4,5,7
    if(lam.x+lam.y+lam.z<1.0) { // first tet in first prism 0,1,3,4
      // default settings, nothing to do
    }
    else if(lam.x<lam.z) { // second tet in first prism 1,3,4,7
      z = 1-z;
      s.z+=ORDER;
      d.z = -1;
    }
    else { // third tet in first prism 1,4,5,7
      x = 1-lam.x-lam.y;
      z = 1-lam.z-lam.y;
      s.z+=ORDER;
      s.x+=ORDER;
      d.x = -1;
      d.z = -1;
      special_order = 1;
    }
  }
  else { // second prism 1,2,3,5,6,7
    if(x+y+z>=2.0) { // first tet in second prism 2,5,6,7
      x = 1-x;
      y = 1-y;
      z = 1-z;
      d.x = -1;
      d.y = -1;
      d.z = -1;
      s.x += ORDER;
      s.y += ORDER;
      s.z += ORDER;
    }
    else if(lam.z<lam.y) { // second tet in second prism 1,2,3,7
      x = 1-lam.x-lam.z;
      y = 1-lam.y;
      s.x+=ORDER;
      s.y+=ORDER;
      d.x = -1;
      d.y = -1;
      special_order = 2;
    }
    else { // third tet in second prism 1,2,5,7
      x = 1-lam.x;
      y = 2-lam.x-lam.y-lam.z;
      z = lam.z+lam.x-1;
      s.x+=ORDER;
      s.y+=ORDER;
      d.x = -1;
      d.y = -1;
      special_order = 3;
    }
  }
  return InterpolateTet( element, coefficients, N, d, s, special_order, vec3(x,y,z), component);
}
#if ORDER==1
vec3  InterpolateTetVec(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  vec3 f[4];
  if(special_order==0)
    for (int k=0; k<=1; k++) for (int j=0; j<=1-k; j++) for (int i=0; i<=1-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k)).xyz;
  if(special_order==1)
    for (int k=0; k<=1; k++) for (int j=0; j<=1-k; j++) for (int i=0; i<=1-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k))).xyz;
  if(special_order==2)
    for (int k=0; k<=1; k++) for (int j=0; j<=1-k; j++) for (int i=0; i<=1-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k)).xyz;
  if(special_order==3)
    for (int k=0; k<=1; k++) for (int j=0; j<=1-k; j++) for (int i=0; i<=1-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k))).xyz;
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]) - 1.0*z*(f[0] - f[3]);
}
#endif

#if ORDER==2
vec3  InterpolateTetVec(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  vec3 f[10];
  if(special_order==0)
    for (int k=0; k<=2; k++) for (int j=0; j<=2-k; j++) for (int i=0; i<=2-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k)).xyz;
  if(special_order==1)
    for (int k=0; k<=2; k++) for (int j=0; j<=2-k; j++) for (int i=0; i<=2-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k))).xyz;
  if(special_order==2)
    for (int k=0; k<=2; k++) for (int j=0; j<=2-k; j++) for (int i=0; i<=2-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k)).xyz;
  if(special_order==3)
    for (int k=0; k<=2; k++) for (int j=0; j<=2-k; j++) for (int i=0; i<=2-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k))).xyz;
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) + 4.0*x*z*(f[0] - f[1] - f[6] + f[7]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) + 4.0*y*z*(f[0] - f[3] - f[6] + f[8]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]) + pow(z, 2)*(2.0*f[0] - 4.0*f[6] + 2.0*f[9]) - z*(3.0*f[0] - 4.0*f[6] + 1.0*f[9]);
}
#endif

vec3 InterpolateTetVec(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam, int component) {
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
  if(lam.x+lam.y<1.0) { // first prism: 0,1,3,4,5,7
    if(lam.x+lam.y+lam.z<1.0) { // first tet in first prism 0,1,3,4
      // default settings, nothing to do
    }
    else if(lam.x<lam.z) { // second tet in first prism 1,3,4,7
      z = 1-z;
      s.z+=ORDER;
      d.z = -1;
    }
    else { // third tet in first prism 1,4,5,7
      x = 1-lam.x-lam.y;
      z = 1-lam.z-lam.y;
      s.z+=ORDER;
      s.x+=ORDER;
      d.x = -1;
      d.z = -1;
      special_order = 1;
    }
  }
  else { // second prism 1,2,3,5,6,7
    if(x+y+z>=2.0) { // first tet in second prism 2,5,6,7
      x = 1-x;
      y = 1-y;
      z = 1-z;
      d.x = -1;
      d.y = -1;
      d.z = -1;
      s.x += ORDER;
      s.y += ORDER;
      s.z += ORDER;
    }
    else if(lam.z<lam.y) { // second tet in second prism 1,2,3,7
      x = 1-lam.x-lam.z;
      y = 1-lam.y;
      s.x+=ORDER;
      s.y+=ORDER;
      d.x = -1;
      d.y = -1;
      special_order = 2;
    }
    else { // third tet in second prism 1,2,5,7
      x = 1-lam.x;
      y = 2-lam.x-lam.y-lam.z;
      z = lam.z+lam.x-1;
      s.x+=ORDER;
      s.y+=ORDER;
      d.x = -1;
      d.y = -1;
      special_order = 3;
    }
  }
  return InterpolateTetVec( element, coefficients, N, d, s, special_order, vec3(x,y,z), component);
}
