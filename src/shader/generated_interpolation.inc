#if ORDER==1
float InterpolateSegm(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[2];
  int offsety = 0;
  for (int i=0; i<=1; i++) {
    f[i] = getValue(coefficients, first+i)[component];
  }
  return 1.0*f[1] + 1.0*x*(f[0] - f[1]);
}
#endif

#if ORDER==2
float InterpolateSegm(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[3];
  int offsety = 0;
  for (int i=0; i<=2; i++) {
    f[i] = getValue(coefficients, first+i)[component];
  }
  return 1.0*f[2] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) - x*(1.0*f[0] - 4.0*f[1] + 3.0*f[2]);
}
#endif

#if ORDER==3
float InterpolateSegm(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[4];
  int offsety = 0;
  for (int i=0; i<=3; i++) {
    f[i] = getValue(coefficients, first+i)[component];
  }
  return 1.0*f[3] + pow(x, 3)*(4.5*f[0] - 13.5*f[1] + 13.5*f[2] - 4.5*f[3]) + pow(x, 2)*(-4.5*f[0] + 18.0*f[1] - 22.5*f[2] + 9.0*f[3]) + x*(1.0*f[0] - 4.5*f[1] + 9.0*f[2] - 5.5*f[3]);
}
#endif

float InterpolateSegm(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
    int n = subdivision+1;
    int N = ORDER*n+1;
    int values_per_element = N;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);

    int X = ORDER*x;

    int first = element*values_per_element + X;
    return InterpolateSegm(values, lam, first, component);
}
#if ORDER==1
vec3 InterpolateSegmVec(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[2];
  int offsety = 0;
  for (int i=0; i<=1; i++) {
    f[i] = getValue(coefficients, first+i).xyz;
  }
  return 1.0*f[1] + 1.0*x*(f[0] - f[1]);
}
#endif

#if ORDER==2
vec3 InterpolateSegmVec(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[3];
  int offsety = 0;
  for (int i=0; i<=2; i++) {
    f[i] = getValue(coefficients, first+i).xyz;
  }
  return 1.0*f[2] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) - x*(1.0*f[0] - 4.0*f[1] + 3.0*f[2]);
}
#endif

#if ORDER==3
vec3 InterpolateSegmVec(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[4];
  int offsety = 0;
  for (int i=0; i<=3; i++) {
    f[i] = getValue(coefficients, first+i).xyz;
  }
  return 1.0*f[3] + pow(x, 3)*(4.5*f[0] - 13.5*f[1] + 13.5*f[2] - 4.5*f[3]) + pow(x, 2)*(-4.5*f[0] + 18.0*f[1] - 22.5*f[2] + 9.0*f[3]) + x*(1.0*f[0] - 4.5*f[1] + 9.0*f[2] - 5.5*f[3]);
}
#endif

vec3 InterpolateSegmVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
    int n = subdivision+1;
    int N = ORDER*n+1;
    int values_per_element = N;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);

    int X = ORDER*x;

    int first = element*values_per_element + X;
    return InterpolateSegmVec(values, lam, first, component);
}
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

#if ORDER==3
float InterpolateTrig(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[10];
  int ii=0;
  int offsety = 0;
  for (int i=0; i<=3; i++) {
    int offsetx = 0;
    for (int j=0; j<=3-i; j++) {
      f[ii] = getValue(coefficients, first+offsetx+offsety)[component];
      offsetx += dx;
      ii++;
    }
    offsety += dy-i;
  }
  return 1.0*f[0] + pow(x, 3)*(-4.5*f[0] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3]) - pow(x, 2)*y*(13.5*f[0] - 27.0*f[1] + 13.5*f[2] + 3.60822483003181e-16*f[3] - 13.5*f[4] + 27.0*f[5] - 13.5*f[6]) + pow(x, 2)*(9.0*f[0] - 22.5*f[1] + 18.0*f[2] - 4.49999999999999*f[3]) - x*pow(y, 2)*(13.5*f[0] - 13.5*f[1] - 7.49400541621981e-16*f[2] + 1.8041124150159e-16*f[3] - 27.0*f[4] + 27.0*f[5] + 7.49400541621981e-16*f[6] + 13.5*f[7] - 13.5*f[8]) + x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] + 6.43004168428737e-16*f[3] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8]) - x*(5.5*f[0] - 9.0*f[1] + 4.5*f[2] - 0.999999999999998*f[3]) + pow(y, 3)*(-4.5*f[0] + 13.5*f[4] - 13.5*f[7] + 4.5*f[9]) + pow(y, 2)*(9.0*f[0] - 22.5*f[4] + 18.0*f[7] - 4.5*f[9]) - y*(5.5*f[0] - 9.0*f[4] + 4.5*f[7] - 0.999999999999999*f[9]);
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

#if ORDER==3
vec3 InterpolateTrigVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[10];
  int ii=0;
  int offsety = 0;
  for (int i=0; i<=3; i++) {
    int offsetx = 0;
    for (int j=0; j<=3-i; j++) {
      f[ii] = getValue(coefficients, first+offsetx+offsety).xyz;
      offsetx += dx;
      ii++;
    }
    offsety += dy-i;
  }
  return 1.0*f[0] + pow(x, 3)*(-4.5*f[0] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3]) - pow(x, 2)*y*(13.5*f[0] - 27.0*f[1] + 13.5*f[2] + 3.60822483003181e-16*f[3] - 13.5*f[4] + 27.0*f[5] - 13.5*f[6]) + pow(x, 2)*(9.0*f[0] - 22.5*f[1] + 18.0*f[2] - 4.49999999999999*f[3]) - x*pow(y, 2)*(13.5*f[0] - 13.5*f[1] - 7.49400541621981e-16*f[2] + 1.8041124150159e-16*f[3] - 27.0*f[4] + 27.0*f[5] + 7.49400541621981e-16*f[6] + 13.5*f[7] - 13.5*f[8]) + x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] + 6.43004168428737e-16*f[3] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8]) - x*(5.5*f[0] - 9.0*f[1] + 4.5*f[2] - 0.999999999999998*f[3]) + pow(y, 3)*(-4.5*f[0] + 13.5*f[4] - 13.5*f[7] + 4.5*f[9]) + pow(y, 2)*(9.0*f[0] - 22.5*f[4] + 18.0*f[7] - 4.5*f[9]) - y*(5.5*f[0] - 9.0*f[4] + 4.5*f[7] - 0.999999999999999*f[9]);
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

#if ORDER==3
float  InterpolateTet(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  float f[20];
  if(special_order==0)
    for (int k=0; k<=3; k++) for (int j=0; j<=3-k; j++) for (int i=0; i<=3-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==1)
    for (int k=0; k<=3; k++) for (int j=0; j<=3-k; j++) for (int i=0; i<=3-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k)))[component];
  if(special_order==2)
    for (int k=0; k<=3; k++) for (int j=0; j<=3-k; j++) for (int i=0; i<=3-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==3)
    for (int k=0; k<=3; k++) for (int j=0; j<=3-k; j++) for (int i=0; i<=3-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k)))[component];
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] + pow(x, 3)*(-4.5*f[0] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3]) - pow(x, 2)*y*(13.5*f[0] - 27.0*f[1] + 13.5*f[2] + 3.60822483003181e-16*f[3] - 13.5*f[4] + 27.0*f[5] - 13.5*f[6]) - pow(x, 2)*z*(13.5*f[0] - 13.5*f[10] + 27.0*f[11] - 13.5*f[12] - 27.0*f[1] + 13.5*f[2] + 3.60822483003181e-16*f[3]) + pow(x, 2)*(9.0*f[0] - 22.5*f[1] + 18.0*f[2] - 4.49999999999999*f[3]) - x*pow(y, 2)*(13.5*f[0] - 13.5*f[1] - 7.49400541621981e-16*f[2] + 1.8041124150159e-16*f[3] - 27.0*f[4] + 27.0*f[5] + 7.49400541621981e-16*f[6] + 13.5*f[7] - 13.5*f[8]) - x*y*z*(27.0*f[0] - 27.0*f[10] + 27.0*f[11] + 7.49400541621981e-16*f[12] + 27.0*f[13] - 27.0*f[14] - 27.0*f[1] + 4.16000867987643e-32*f[2] - 4.82253126321547e-16*f[3] - 27.0*f[4] + 27.0*f[5] + 7.49400541621981e-16*f[6] - 7.49400541621981e-16*f[7] + 7.49400541621981e-16*f[8]) + x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] + 7.03141248929267e-16*f[3] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8]) - x*pow(z, 2)*(13.5*f[0] - 27.0*f[10] + 27.0*f[11] + 7.49400541621981e-16*f[12] + 13.5*f[16] - 13.5*f[17] - 13.5*f[1] - 7.49400541621981e-16*f[2] + 1.8041124150159e-16*f[3]) + x*z*(18.0*f[0] - 22.5*f[10] + 27.0*f[11] - 4.5*f[12] + 4.5*f[16] - 4.5*f[17] - 22.5*f[1] + 4.5*f[2] + 6.43004168428737e-16*f[3]) - x*(5.5*f[0] - 9.0*f[1] + 4.5*f[2] - 0.999999999999998*f[3]) + pow(y, 3)*(-4.5*f[0] + 13.5*f[4] - 13.5*f[7] + 4.5*f[9]) - pow(y, 2)*z*(13.5*f[0] - 13.5*f[10] + 27.0*f[13] - 13.5*f[15] - 27.0*f[4] + 13.5*f[7] + 3.60822483003181e-16*f[9]) + pow(y, 2)*(9.0*f[0] - 22.5*f[4] + 18.0*f[7] - 4.49999999999999*f[9]) - y*pow(z, 2)*(13.5*f[0] - 27.0*f[10] + 27.0*f[13] + 7.49400541621981e-16*f[15] + 13.5*f[16] - 13.5*f[18] - 13.5*f[4] - 7.49400541621981e-16*f[7] + 1.8041124150159e-16*f[9]) + y*z*(18.0*f[0] - 22.5*f[10] + 27.0*f[13] - 4.5*f[15] + 4.5*f[16] - 4.5*f[18] - 22.5*f[4] + 4.5*f[7] + 6.43004168428737e-16*f[9]) - y*(5.5*f[0] - 9.0*f[4] + 4.5*f[7] - 0.999999999999998*f[9]) + pow(z, 3)*(-4.5*f[0] + 13.5*f[10] - 13.5*f[16] + 4.5*f[19]) + pow(z, 2)*(9.0*f[0] - 22.5*f[10] + 18.0*f[16] - 4.49999999999999*f[19]) - z*(5.5*f[0] - 9.0*f[10] + 4.5*f[16] - 0.999999999999998*f[19]);
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

#if ORDER==3
vec3  InterpolateTetVec(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  vec3 f[20];
  if(special_order==0)
    for (int k=0; k<=3; k++) for (int j=0; j<=3-k; j++) for (int i=0; i<=3-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k)).xyz;
  if(special_order==1)
    for (int k=0; k<=3; k++) for (int j=0; j<=3-k; j++) for (int i=0; i<=3-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k))).xyz;
  if(special_order==2)
    for (int k=0; k<=3; k++) for (int j=0; j<=3-k; j++) for (int i=0; i<=3-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k)).xyz;
  if(special_order==3)
    for (int k=0; k<=3; k++) for (int j=0; j<=3-k; j++) for (int i=0; i<=3-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k))).xyz;
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] + pow(x, 3)*(-4.5*f[0] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3]) - pow(x, 2)*y*(13.5*f[0] - 27.0*f[1] + 13.5*f[2] + 3.60822483003181e-16*f[3] - 13.5*f[4] + 27.0*f[5] - 13.5*f[6]) - pow(x, 2)*z*(13.5*f[0] - 13.5*f[10] + 27.0*f[11] - 13.5*f[12] - 27.0*f[1] + 13.5*f[2] + 3.60822483003181e-16*f[3]) + pow(x, 2)*(9.0*f[0] - 22.5*f[1] + 18.0*f[2] - 4.49999999999999*f[3]) - x*pow(y, 2)*(13.5*f[0] - 13.5*f[1] - 7.49400541621981e-16*f[2] + 1.8041124150159e-16*f[3] - 27.0*f[4] + 27.0*f[5] + 7.49400541621981e-16*f[6] + 13.5*f[7] - 13.5*f[8]) - x*y*z*(27.0*f[0] - 27.0*f[10] + 27.0*f[11] + 7.49400541621981e-16*f[12] + 27.0*f[13] - 27.0*f[14] - 27.0*f[1] + 4.16000867987643e-32*f[2] - 4.82253126321547e-16*f[3] - 27.0*f[4] + 27.0*f[5] + 7.49400541621981e-16*f[6] - 7.49400541621981e-16*f[7] + 7.49400541621981e-16*f[8]) + x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] + 7.03141248929267e-16*f[3] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8]) - x*pow(z, 2)*(13.5*f[0] - 27.0*f[10] + 27.0*f[11] + 7.49400541621981e-16*f[12] + 13.5*f[16] - 13.5*f[17] - 13.5*f[1] - 7.49400541621981e-16*f[2] + 1.8041124150159e-16*f[3]) + x*z*(18.0*f[0] - 22.5*f[10] + 27.0*f[11] - 4.5*f[12] + 4.5*f[16] - 4.5*f[17] - 22.5*f[1] + 4.5*f[2] + 6.43004168428737e-16*f[3]) - x*(5.5*f[0] - 9.0*f[1] + 4.5*f[2] - 0.999999999999998*f[3]) + pow(y, 3)*(-4.5*f[0] + 13.5*f[4] - 13.5*f[7] + 4.5*f[9]) - pow(y, 2)*z*(13.5*f[0] - 13.5*f[10] + 27.0*f[13] - 13.5*f[15] - 27.0*f[4] + 13.5*f[7] + 3.60822483003181e-16*f[9]) + pow(y, 2)*(9.0*f[0] - 22.5*f[4] + 18.0*f[7] - 4.49999999999999*f[9]) - y*pow(z, 2)*(13.5*f[0] - 27.0*f[10] + 27.0*f[13] + 7.49400541621981e-16*f[15] + 13.5*f[16] - 13.5*f[18] - 13.5*f[4] - 7.49400541621981e-16*f[7] + 1.8041124150159e-16*f[9]) + y*z*(18.0*f[0] - 22.5*f[10] + 27.0*f[13] - 4.5*f[15] + 4.5*f[16] - 4.5*f[18] - 22.5*f[4] + 4.5*f[7] + 6.43004168428737e-16*f[9]) - y*(5.5*f[0] - 9.0*f[4] + 4.5*f[7] - 0.999999999999998*f[9]) + pow(z, 3)*(-4.5*f[0] + 13.5*f[10] - 13.5*f[16] + 4.5*f[19]) + pow(z, 2)*(9.0*f[0] - 22.5*f[10] + 18.0*f[16] - 4.49999999999999*f[19]) - z*(5.5*f[0] - 9.0*f[10] + 4.5*f[16] - 0.999999999999998*f[19]);
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
#if ORDER==1
float InterpolateQuad(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float f[4];
  int ii=0;
  for (int i=0; i<=1; i++) {
    for (int j=0; j<=1; j++) {
      f[ii] = getValue(coefficients, first+j+i*dy)[component];
      ii++;
    }
  }
  return 1.0*f[0] + 1.0*x*y*(f[0] - f[1] - f[2] + f[3]) - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]);
}
#endif

#if ORDER==2
float InterpolateQuad(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float f[9];
  int ii=0;
  for (int i=0; i<=2; i++) {
    for (int j=0; j<=2; j++) {
      f[ii] = getValue(coefficients, first+j+i*dy)[component];
      ii++;
    }
  }
  return 1.0*f[0] + pow(x, 2)*pow(y, 2)*(4.0*f[0] - 8.0*f[1] + 4.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 4.0*f[6] - 8.0*f[7] + 4.0*f[8]) - pow(x, 2)*y*(6.0*f[0] - 12.0*f[1] + 6.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 2.0*f[6] - 4.0*f[7] + 2.0*f[8]) + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) - x*pow(y, 2)*(6.0*f[0] - 8.0*f[1] + 2.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 6.0*f[6] - 8.0*f[7] + 2.0*f[8]) + x*y*(9.0*f[0] - 12.0*f[1] + 3.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 3.0*f[6] - 4.0*f[7] + 1.0*f[8]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[6]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[6]);
}
#endif

#if ORDER==3
float InterpolateQuad(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float f[16];
  int ii=0;
  for (int i=0; i<=3; i++) {
    for (int j=0; j<=3; j++) {
      f[ii] = getValue(coefficients, first+j+i*dy)[component];
      ii++;
    }
  }
  return 1.0*f[0] + pow(x, 3)*pow(y, 3)*(20.2500000000002*f[0] + 182.249999999999*f[10] - 60.7499999999997*f[11] - 20.2499999999999*f[12] + 60.7499999999997*f[13] - 60.7499999999997*f[14] + 20.2499999999999*f[15] - 60.7500000000004*f[1] + 60.7500000000003*f[2] - 20.25*f[3] - 60.7500000000002*f[4] + 182.25*f[5] - 182.25*f[6] + 60.7499999999999*f[7] + 60.7499999999999*f[8] - 182.25*f[9]) + pow(x, 3)*pow(y, 2)*(-40.5000000000002*f[0] - 242.999999999999*f[10] + 80.9999999999996*f[11] + 20.2499999999999*f[12] - 60.7499999999996*f[13] + 60.7499999999996*f[14] - 20.2499999999998*f[15] + 121.5*f[1] - 121.5*f[2] + 40.5*f[3] + 101.25*f[4] - 303.75*f[5] + 303.75*f[6] - 101.25*f[7] - 80.9999999999998*f[8] + 242.999999999999*f[9]) + pow(x, 3)*y*(24.75*f[0] + 60.7499999999997*f[10] - 20.2499999999999*f[11] - 4.49999999999996*f[12] + 13.4999999999999*f[13] - 13.4999999999999*f[14] + 4.49999999999995*f[15] - 74.25*f[1] + 74.2499999999999*f[2] - 24.75*f[3] - 40.5*f[4] + 121.5*f[5] - 121.5*f[6] + 40.4999999999999*f[7] + 20.2499999999999*f[8] - 60.7499999999997*f[9]) + pow(x, 3)*(-4.5*f[0] - 3.7470027081099e-16*f[11] - 7.4940054162198e-16*f[12] + 3.7470027081099e-16*f[15] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3] + 7.4940054162198e-16*f[4] + 3.7470027081099e-16*f[8]) + pow(x, 2)*pow(y, 3)*(-40.5000000000002*f[0] - 242.999999999999*f[10] + 60.7499999999995*f[11] + 40.4999999999998*f[12] - 101.249999999999*f[13] + 80.9999999999994*f[14] - 20.2499999999998*f[15] + 101.25*f[1] - 81.0000000000001*f[2] + 20.25*f[3] + 121.5*f[4] - 303.75*f[5] + 242.999999999999*f[6] - 60.7499999999996*f[7] - 121.5*f[8] + 303.749999999999*f[9]) + pow(x, 2)*pow(y, 2)*(81.0000000000001*f[0] + 323.999999999998*f[10] - 80.9999999999993*f[11] - 40.4999999999997*f[12] + 101.249999999999*f[13] - 80.9999999999992*f[14] + 20.2499999999997*f[15] - 202.5*f[1] + 162.0*f[2] - 40.4999999999999*f[3] - 202.5*f[4] + 506.249999999999*f[5] - 404.999999999999*f[6] + 101.249999999999*f[7] + 161.999999999999*f[8] - 404.999999999998*f[9]) - pow(x, 2)*y*(49.5*f[0] + 80.9999999999994*f[10] - 20.2499999999998*f[11] - 8.99999999999991*f[12] + 22.4999999999998*f[13] - 17.9999999999998*f[14] + 4.49999999999992*f[15] - 123.75*f[1] + 98.9999999999998*f[2] - 24.7499999999999*f[3] - 80.9999999999999*f[4] + 202.499999999999*f[5] - 161.999999999999*f[6] + 40.4999999999998*f[7] + 40.4999999999998*f[8] - 101.249999999999*f[9]) + pow(x, 2)*(8.99999999999999*f[0] - 3.86727686911095e-15*f[10] + 5.40771131577836e-15*f[11] + 2.94209101525666e-15*f[12] - 4.52415882534751e-15*f[13] + 8.51170985545949e-16*f[14] + 2.22044604925033e-16*f[15] - 22.5*f[1] + 18.0*f[2] - 4.5*f[3] - 5.73615229389666e-16*f[4] - 9.37213269954402e-15*f[5] + 3.58972111295466e-15*f[6] + 2.65528340056184e-15*f[7] - 8.96505092384813e-15*f[8] - 1.53118258812887e-15*f[9]) + x*pow(y, 3)*(24.75*f[0] + 60.7499999999991*f[10] - 13.4999999999997*f[11] - 24.7499999999999*f[12] + 40.4999999999997*f[13] - 20.2499999999997*f[14] + 4.49999999999991*f[15] - 40.4999999999999*f[1] + 20.2499999999998*f[2] - 4.49999999999993*f[3] - 74.2499999999997*f[4] + 121.499999999999*f[5] - 60.7499999999993*f[6] + 13.4999999999997*f[7] + 74.2499999999997*f[8] - 121.499999999999*f[9]) - x*pow(y, 2)*(49.4999999999999*f[0] + 80.9999999999988*f[10] - 17.9999999999997*f[11] - 24.7499999999998*f[12] + 40.4999999999996*f[13] - 20.2499999999996*f[14] + 4.49999999999988*f[15] - 80.9999999999997*f[1] + 40.4999999999997*f[2] - 8.99999999999988*f[3] - 123.75*f[4] + 202.499999999999*f[5] - 101.249999999999*f[6] + 22.4999999999996*f[7] + 98.9999999999996*f[8] - 161.999999999999*f[9]) + x*y*(30.25*f[0] + 20.2499999999997*f[10] - 4.4999999999999*f[11] - 5.49999999999996*f[12] + 8.99999999999987*f[13] - 4.49999999999991*f[14] + 0.999999999999966*f[15] - 49.4999999999999*f[1] + 24.7499999999999*f[2] - 5.49999999999997*f[3] - 49.4999999999999*f[4] + 80.9999999999996*f[5] - 40.4999999999997*f[6] + 8.99999999999988*f[7] + 24.7499999999999*f[8] - 40.4999999999996*f[9]) + x*(-5.5*f[0] + 2.74317605667799e-15*f[10] - 3.19189119579732e-15*f[11] - 9.89948863624097e-16*f[12] + 4.01530660572765e-15*f[13] + 1.17498603439496e-15*f[14] + 1.66533453693773e-16*f[15] + 9.0*f[1] - 4.49999999999999*f[2] + 1.0*f[3] + 2.63677968348475e-15*f[4] + 2.9920510513648e-14*f[5] + 1.76062867988473e-14*f[6] - 1.46179364908979e-15*f[7] + 3.41856172999163e-15*f[8] + 1.61907524424503e-15*f[9]) + pow(y, 3)*(-4.5*f[0] + 4.5*f[12] + 13.5*f[4] - 13.5*f[8]) + pow(y, 2)*(9.0*f[0] - 4.49999999999999*f[12] - 22.5*f[4] + 18.0*f[8]) - y*(5.5*f[0] - 0.999999999999998*f[12] - 9.0*f[4] + 4.5*f[8]);
}
#endif

float InterpolateQuad(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return InterpolateQuad(values, lam, first, dy, component);
}
#if ORDER==1
vec3 InterpolateQuadVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  vec3 f[4];
  int ii=0;
  for (int i=0; i<=1; i++) {
    for (int j=0; j<=1; j++) {
      f[ii] = getValue(coefficients, first+j+i*dy).xyz;
      ii++;
    }
  }
  return 1.0*f[0] + 1.0*x*y*(f[0] - f[1] - f[2] + f[3]) - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]);
}
#endif

#if ORDER==2
vec3 InterpolateQuadVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  vec3 f[9];
  int ii=0;
  for (int i=0; i<=2; i++) {
    for (int j=0; j<=2; j++) {
      f[ii] = getValue(coefficients, first+j+i*dy).xyz;
      ii++;
    }
  }
  return 1.0*f[0] + pow(x, 2)*pow(y, 2)*(4.0*f[0] - 8.0*f[1] + 4.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 4.0*f[6] - 8.0*f[7] + 4.0*f[8]) - pow(x, 2)*y*(6.0*f[0] - 12.0*f[1] + 6.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 2.0*f[6] - 4.0*f[7] + 2.0*f[8]) + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) - x*pow(y, 2)*(6.0*f[0] - 8.0*f[1] + 2.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 6.0*f[6] - 8.0*f[7] + 2.0*f[8]) + x*y*(9.0*f[0] - 12.0*f[1] + 3.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 3.0*f[6] - 4.0*f[7] + 1.0*f[8]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[6]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[6]);
}
#endif

#if ORDER==3
vec3 InterpolateQuadVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  vec3 f[16];
  int ii=0;
  for (int i=0; i<=3; i++) {
    for (int j=0; j<=3; j++) {
      f[ii] = getValue(coefficients, first+j+i*dy).xyz;
      ii++;
    }
  }
  return 1.0*f[0] + pow(x, 3)*pow(y, 3)*(20.2500000000002*f[0] + 182.249999999999*f[10] - 60.7499999999997*f[11] - 20.2499999999999*f[12] + 60.7499999999997*f[13] - 60.7499999999997*f[14] + 20.2499999999999*f[15] - 60.7500000000004*f[1] + 60.7500000000003*f[2] - 20.25*f[3] - 60.7500000000002*f[4] + 182.25*f[5] - 182.25*f[6] + 60.7499999999999*f[7] + 60.7499999999999*f[8] - 182.25*f[9]) + pow(x, 3)*pow(y, 2)*(-40.5000000000002*f[0] - 242.999999999999*f[10] + 80.9999999999996*f[11] + 20.2499999999999*f[12] - 60.7499999999996*f[13] + 60.7499999999996*f[14] - 20.2499999999998*f[15] + 121.5*f[1] - 121.5*f[2] + 40.5*f[3] + 101.25*f[4] - 303.75*f[5] + 303.75*f[6] - 101.25*f[7] - 80.9999999999998*f[8] + 242.999999999999*f[9]) + pow(x, 3)*y*(24.75*f[0] + 60.7499999999997*f[10] - 20.2499999999999*f[11] - 4.49999999999996*f[12] + 13.4999999999999*f[13] - 13.4999999999999*f[14] + 4.49999999999995*f[15] - 74.25*f[1] + 74.2499999999999*f[2] - 24.75*f[3] - 40.5*f[4] + 121.5*f[5] - 121.5*f[6] + 40.4999999999999*f[7] + 20.2499999999999*f[8] - 60.7499999999997*f[9]) + pow(x, 3)*(-4.5*f[0] - 3.7470027081099e-16*f[11] - 7.4940054162198e-16*f[12] + 3.7470027081099e-16*f[15] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3] + 7.4940054162198e-16*f[4] + 3.7470027081099e-16*f[8]) + pow(x, 2)*pow(y, 3)*(-40.5000000000002*f[0] - 242.999999999999*f[10] + 60.7499999999995*f[11] + 40.4999999999998*f[12] - 101.249999999999*f[13] + 80.9999999999994*f[14] - 20.2499999999998*f[15] + 101.25*f[1] - 81.0000000000001*f[2] + 20.25*f[3] + 121.5*f[4] - 303.75*f[5] + 242.999999999999*f[6] - 60.7499999999996*f[7] - 121.5*f[8] + 303.749999999999*f[9]) + pow(x, 2)*pow(y, 2)*(81.0000000000001*f[0] + 323.999999999998*f[10] - 80.9999999999993*f[11] - 40.4999999999997*f[12] + 101.249999999999*f[13] - 80.9999999999992*f[14] + 20.2499999999997*f[15] - 202.5*f[1] + 162.0*f[2] - 40.4999999999999*f[3] - 202.5*f[4] + 506.249999999999*f[5] - 404.999999999999*f[6] + 101.249999999999*f[7] + 161.999999999999*f[8] - 404.999999999998*f[9]) - pow(x, 2)*y*(49.5*f[0] + 80.9999999999994*f[10] - 20.2499999999998*f[11] - 8.99999999999991*f[12] + 22.4999999999998*f[13] - 17.9999999999998*f[14] + 4.49999999999992*f[15] - 123.75*f[1] + 98.9999999999998*f[2] - 24.7499999999999*f[3] - 80.9999999999999*f[4] + 202.499999999999*f[5] - 161.999999999999*f[6] + 40.4999999999998*f[7] + 40.4999999999998*f[8] - 101.249999999999*f[9]) + pow(x, 2)*(8.99999999999999*f[0] - 3.86727686911095e-15*f[10] + 5.40771131577836e-15*f[11] + 2.94209101525666e-15*f[12] - 4.52415882534751e-15*f[13] + 8.51170985545949e-16*f[14] + 2.22044604925033e-16*f[15] - 22.5*f[1] + 18.0*f[2] - 4.5*f[3] - 5.73615229389666e-16*f[4] - 9.37213269954402e-15*f[5] + 3.58972111295466e-15*f[6] + 2.65528340056184e-15*f[7] - 8.96505092384813e-15*f[8] - 1.53118258812887e-15*f[9]) + x*pow(y, 3)*(24.75*f[0] + 60.7499999999991*f[10] - 13.4999999999997*f[11] - 24.7499999999999*f[12] + 40.4999999999997*f[13] - 20.2499999999997*f[14] + 4.49999999999991*f[15] - 40.4999999999999*f[1] + 20.2499999999998*f[2] - 4.49999999999993*f[3] - 74.2499999999997*f[4] + 121.499999999999*f[5] - 60.7499999999993*f[6] + 13.4999999999997*f[7] + 74.2499999999997*f[8] - 121.499999999999*f[9]) - x*pow(y, 2)*(49.4999999999999*f[0] + 80.9999999999988*f[10] - 17.9999999999997*f[11] - 24.7499999999998*f[12] + 40.4999999999996*f[13] - 20.2499999999996*f[14] + 4.49999999999988*f[15] - 80.9999999999997*f[1] + 40.4999999999997*f[2] - 8.99999999999988*f[3] - 123.75*f[4] + 202.499999999999*f[5] - 101.249999999999*f[6] + 22.4999999999996*f[7] + 98.9999999999996*f[8] - 161.999999999999*f[9]) + x*y*(30.25*f[0] + 20.2499999999997*f[10] - 4.4999999999999*f[11] - 5.49999999999996*f[12] + 8.99999999999987*f[13] - 4.49999999999991*f[14] + 0.999999999999966*f[15] - 49.4999999999999*f[1] + 24.7499999999999*f[2] - 5.49999999999997*f[3] - 49.4999999999999*f[4] + 80.9999999999996*f[5] - 40.4999999999997*f[6] + 8.99999999999988*f[7] + 24.7499999999999*f[8] - 40.4999999999996*f[9]) + x*(-5.5*f[0] + 2.74317605667799e-15*f[10] - 3.19189119579732e-15*f[11] - 9.89948863624097e-16*f[12] + 4.01530660572765e-15*f[13] + 1.17498603439496e-15*f[14] + 1.66533453693773e-16*f[15] + 9.0*f[1] - 4.49999999999999*f[2] + 1.0*f[3] + 2.63677968348475e-15*f[4] + 2.9920510513648e-14*f[5] + 1.76062867988473e-14*f[6] - 1.46179364908979e-15*f[7] + 3.41856172999163e-15*f[8] + 1.61907524424503e-15*f[9]) + pow(y, 3)*(-4.5*f[0] + 4.5*f[12] + 13.5*f[4] - 13.5*f[8]) + pow(y, 2)*(9.0*f[0] - 4.49999999999999*f[12] - 22.5*f[4] + 18.0*f[8]) - y*(5.5*f[0] - 0.999999999999998*f[12] - 9.0*f[4] + 4.5*f[8]);
}
#endif

vec3 InterpolateQuadVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return InterpolateQuadVec(values, lam, first, dy, component);
}
