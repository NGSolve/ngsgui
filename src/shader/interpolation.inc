vec4 getValue(samplerBuffer coefficients, int offset) {
  return texelFetch(coefficients, 2+offset+0);
}

float InterpolateTrigP1(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
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
float InterpolateTrigP2(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
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
float InterpolateTrigP3(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
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
  return 1.0*f[0] + pow(x, 3)*(-4.5*f[0] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3]) - pow(x, 2)*y*(13.5*f[0] - 27.0*f[1] + 13.5*f[2] - 13.5*f[4] + 27.0*f[5] - 13.5*f[6]) + pow(x, 2)*(9.0*f[0] - 22.5*f[1] + 18.0*f[2] - 4.49999999999999*f[3]) - x*pow(y, 2)*(13.5*f[0] - 13.5*f[1] - 27.0*f[4] + 27.0*f[5] + 13.5*f[7] - 13.5*f[8]) + x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8] + 4.9960036108132e-16*f[9]) - x*(5.5*f[0] - 9.0*f[1] + 4.5*f[2] - 0.999999999999998*f[3]) + pow(y, 3)*(-4.5*f[0] + 13.5*f[4] - 13.5*f[7] + 4.5*f[9]) + pow(y, 2)*(9.0*f[0] - 22.5*f[4] + 18.0*f[7] - 4.49999999999999*f[9]) - y*(5.5*f[0] - 9.0*f[4] + 4.5*f[7] - 0.999999999999998*f[9]);
}
float InterpolateTrigP4(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[15];
  int ii=0;
  int offsety = 0;
  for (int i=0; i<=4; i++) {
    int offsetx = 0;
    for (int j=0; j<=4-i; j++) {
      f[ii] = getValue(coefficients, first+offsetx+offsety)[component];
      offsetx += dx;
      ii++;
    }
    offsety += dy-i;
  }
  return 1.0*f[0] + pow(x, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[1] + 64.0*f[2] - 42.6666666666667*f[3] + 10.6666666666667*f[4]) - pow(x, 3)*y*(-42.6666666666667*f[0] + 3.5527136788005e-15*f[14] + 128.0*f[1] - 128.0*f[2] + 42.6666666666667*f[3] + 42.6666666666667*f[5] - 128.0*f[6] + 128.0*f[7] - 42.6666666666667*f[8]) + pow(x, 3)*(-26.6666666666667*f[0] + 96.0*f[1] - 128.0*f[2] + 74.6666666666667*f[3] - 16.0*f[4]) + pow(x, 2)*pow(y, 2)*(64.0*f[0] - 128.0*f[10] + 64.0*f[11] - 128.0*f[1] + 64.0*f[2] - 128.0*f[5] + 256.0*f[6] - 128.0*f[7] + 64.0*f[9]) + pow(x, 2)*y*(-80.0*f[0] + 32.0*f[10] - 16.0*f[11] + 9.473903143468e-15*f[12] + 4.736951571734e-15*f[13] + 3.25665420556713e-15*f[14] + 192.0*f[1] - 144.0*f[2] + 32.0*f[3] + 96.0*f[5] - 224.0*f[6] + 160.0*f[7] - 32.0*f[8] - 16.0*f[9]) + pow(x, 2)*(23.3333333333333*f[0] - 69.3333333333333*f[1] + 76.0*f[2] - 37.3333333333333*f[3] + 7.33333333333334*f[4]) + x*pow(y, 3)*(42.6666666666667*f[0] - 128.0*f[10] - 42.6666666666667*f[12] + 42.6666666666667*f[13] + 3.5527136788005e-15*f[14] - 42.6666666666667*f[1] - 128.0*f[5] + 128.0*f[6] + 128.0*f[9]) - x*pow(y, 2)*(80.0*f[0] - 160.0*f[10] + 16.0*f[11] - 32.0*f[12] + 32.0*f[13] + 3.5527136788005e-15*f[14] - 96.0*f[1] + 16.0*f[2] - 192.0*f[5] + 224.0*f[6] - 32.0*f[7] + 144.0*f[9]) + x*y*(46.6666666666667*f[0] - 32.0*f[10] + 4.0*f[11] - 5.33333333333334*f[12] + 5.33333333333333*f[13] + 1.03620815631681e-15*f[14] - 69.3333333333333*f[1] + 28.0*f[2] - 5.33333333333333*f[3] - 69.3333333333333*f[5] + 96.0*f[6] - 32.0*f[7] + 5.33333333333333*f[8] + 28.0*f[9]) - x*(8.33333333333333*f[0] - 16.0*f[1] + 12.0*f[2] - 5.33333333333334*f[3] + 1.0*f[4]) + pow(y, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[12] + 10.6666666666667*f[14] - 42.6666666666667*f[5] + 64.0*f[9]) + pow(y, 3)*(-26.6666666666667*f[0] + 74.6666666666667*f[12] - 16.0*f[14] + 96.0*f[5] - 128.0*f[9]) + pow(y, 2)*(23.3333333333333*f[0] - 37.3333333333333*f[12] + 7.33333333333334*f[14] - 69.3333333333333*f[5] + 76.0*f[9]) - y*(8.33333333333333*f[0] - 5.33333333333334*f[12] + 1.0*f[14] - 16.0*f[5] + 12.0*f[9]);
}

float InterpolateTrig(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam, int component) {
  if (component>2 || component<0) return 0.;
    int n = subdivision+1;
    int N = order*n+1;
    int values_per_element = N*(N+1)/2;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);
    int y = int(lamn.y);
    int z = int(lamn.z);

    int X = order*x;
    int Y = order*y;
    int Z = order*z;

    int first, dx, dy;
    if(lam.x+lam.y<1.0) { // lower left trig of quad
        first = element*values_per_element+getIndex(N,X,Y);
        dx = getIndex(N,X+1, Y)-getIndex(N,X,Y);
        dy = getIndex(N,X, Y+1)-getIndex(N,X,Y);
    }
    else { // upper right trig of quad
        first = element*values_per_element+getIndex(N,X+order,Y+order);
        dx = getIndex(N,X, Y)-getIndex(N,X+1,Y);
        dy = getIndex(N,X, Y+order-1)-getIndex(N,X,Y+order);
        lam.x = 1-lam.x;
        lam.y = 1-lam.y;
        lam.z = 1-lam.x-lam.y;
    }

    if(order==1) return InterpolateTrigP1(coefficients, lam, first, dx, dy, component);
    if(order==2) return InterpolateTrigP2(coefficients, lam, first, dx, dy, component);
    if(order==3) return InterpolateTrigP3(coefficients, lam, first, dx, dy, component);
    if(order==4) return InterpolateTrigP4(coefficients, lam, first, dx, dy, component);
    return 0.0;
}

float  InterpolateTetP1(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  float f[4];
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  int p = 1;
  if(special_order==0)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==1)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k)))[component];
  if(special_order==2)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==3)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k)))[component];
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]) - 1.0*z*(f[0] - f[3]);
}

float  InterpolateTetP2(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  float f[10];
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  int p = 2;
  if(special_order==0)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==1)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k)))[component];
  if(special_order==2)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==3)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k)))[component];
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) + 4.0*x*z*(f[0] - f[1] - f[6] + f[7]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) + 4.0*y*z*(f[0] - f[3] - f[6] + f[8]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]) + pow(z, 2)*(2.0*f[0] - 4.0*f[6] + 2.0*f[9]) - z*(3.0*f[0] - 4.0*f[6] + 1.0*f[9]);
}
float  InterpolateTetP3(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  float f[20];
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  int p = 3;
  if(special_order==0)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==1)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k)))[component];
  if(special_order==2)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==3)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k)))[component];
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] + pow(x, 3)*(-4.5*f[0] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3]) - pow(x, 2)*y*(13.5*f[0] - 27.0*f[1] + 13.5*f[2] - 13.5*f[4] + 27.0*f[5] - 13.5*f[6]) - pow(x, 2)*z*(13.5*f[0] - 13.5*f[10] + 27.0*f[11] - 13.5*f[12] - 27.0*f[1] + 13.5*f[2]) + pow(x, 2)*(9.0*f[0] - 22.5*f[1] + 18.0*f[2] - 4.49999999999999*f[3]) - x*pow(y, 2)*(13.5*f[0] - 13.5*f[1] - 27.0*f[4] + 27.0*f[5] + 13.5*f[7] - 13.5*f[8]) - 27.0*x*y*z*(f[0] - f[10] + f[11] + f[13] - f[14] - f[1] - f[4] + f[5]) + x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8] + 4.9960036108132e-16*f[9]) - x*pow(z, 2)*(13.5*f[0] - 27.0*f[10] + 27.0*f[11] + 13.5*f[16] - 13.5*f[17] - 13.5*f[1]) + x*z*(18.0*f[0] - 22.5*f[10] + 27.0*f[11] - 4.5*f[12] + 4.5*f[16] - 4.5*f[17] + 4.9960036108132e-16*f[19] - 22.5*f[1] + 4.5*f[2]) - x*(5.5*f[0] - 9.0*f[1] + 4.5*f[2] - 0.999999999999998*f[3]) + pow(y, 3)*(-4.5*f[0] + 13.5*f[4] - 13.5*f[7] + 4.5*f[9]) - pow(y, 2)*z*(13.5*f[0] - 13.5*f[10] + 27.0*f[13] - 13.5*f[15] - 27.0*f[4] + 13.5*f[7]) + pow(y, 2)*(9.0*f[0] - 22.5*f[4] + 18.0*f[7] - 4.49999999999999*f[9]) - y*pow(z, 2)*(13.5*f[0] - 27.0*f[10] + 27.0*f[13] + 13.5*f[16] - 13.5*f[18] - 13.5*f[4]) + y*z*(18.0*f[0] - 22.5*f[10] + 27.0*f[13] - 4.5*f[15] + 4.5*f[16] - 4.5*f[18] + 4.9960036108132e-16*f[19] - 22.5*f[4] + 4.5*f[7]) - y*(5.5*f[0] - 9.0*f[4] + 4.5*f[7] - 0.999999999999998*f[9]) + pow(z, 3)*(-4.5*f[0] + 13.5*f[10] - 13.5*f[16] + 4.5*f[19]) + pow(z, 2)*(9.0*f[0] - 22.5*f[10] + 18.0*f[16] - 4.49999999999999*f[19]) - z*(5.5*f[0] - 9.0*f[10] + 4.5*f[16] - 0.999999999999998*f[19]);
}

float  InterpolateTetP4(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
  float f[35];
  int values_per_element = N*(N+1)*(N+2)/6;
  int first = element*values_per_element;
  int ii = 0;
  int p = 4;
  if(special_order==0)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==1)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+j), s.y+d.y*j, s.z+d.z*(j+k)))[component];
  if(special_order==2)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*(i+k), s.y+d.y*j, s.z+d.z*k))[component];
  if(special_order==3)
    for (int k=0; k<=p; k++) for (int j=0; j<=p-k; j++) for (int i=0; i<=p-k-j; i++)
          f[ii++] = getValue(coefficients, first+getIndex(N,s.x+d.x*i, s.y+d.y*(j+k), s.z+d.z*(i+k)))[component];
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  return 1.0*f[0] + pow(x, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[1] + 64.0*f[2] - 42.6666666666667*f[3] + 10.6666666666667*f[4]) - pow(x, 3)*y*(-42.6666666666667*f[0] + 3.5527136788005e-15*f[14] + 128.0*f[1] - 128.0*f[2] + 42.6666666666667*f[3] + 42.6666666666667*f[5] - 128.0*f[6] + 128.0*f[7] - 42.6666666666667*f[8]) - pow(x, 3)*z*(-42.6666666666667*f[0] + 42.6666666666667*f[15] - 128.0*f[16] + 128.0*f[17] - 42.6666666666667*f[18] + 128.0*f[1] - 128.0*f[2] + 3.5527136788005e-15*f[34] + 42.6666666666667*f[3]) + pow(x, 3)*(-26.6666666666667*f[0] + 96.0*f[1] - 128.0*f[2] + 74.6666666666667*f[3] - 16.0*f[4]) + pow(x, 2)*pow(y, 2)*(64.0*f[0] - 128.0*f[10] + 64.0*f[11] - 128.0*f[1] + 64.0*f[2] - 128.0*f[5] + 256.0*f[6] - 128.0*f[7] + 64.0*f[9]) + pow(x, 2)*y*z*(128.0*f[0] - 128.0*f[15] + 256.0*f[16] - 128.0*f[17] + 128.0*f[19] - 256.0*f[1] - 256.0*f[20] + 128.0*f[21] + 128.0*f[2] + 1.1842378929335e-14*f[31] + 2.368475785867e-15*f[33] - 9.473903143468e-15*f[34] - 128.0*f[5] + 256.0*f[6] - 128.0*f[7]) + pow(x, 2)*y*(-80.0*f[0] + 32.0*f[10] - 16.0*f[11] + 9.473903143468e-15*f[12] + 4.736951571734e-15*f[13] + 3.25665420556713e-15*f[14] + 192.0*f[1] - 144.0*f[2] + 32.0*f[3] + 96.0*f[5] - 224.0*f[6] + 160.0*f[7] - 32.0*f[8] - 16.0*f[9]) + pow(x, 2)*pow(z, 2)*(64.0*f[0] - 128.0*f[15] + 256.0*f[16] - 128.0*f[17] - 128.0*f[1] + 64.0*f[25] - 128.0*f[26] + 64.0*f[27] + 64.0*f[2]) + pow(x, 2)*z*(-80.0*f[0] + 96.0*f[15] - 224.0*f[16] + 160.0*f[17] - 32.0*f[18] + 192.0*f[1] - 16.0*f[25] + 32.0*f[26] - 16.0*f[27] - 144.0*f[2] + 9.473903143468e-15*f[31] + 4.736951571734e-15*f[32] + 3.25665420556713e-15*f[34] + 32.0*f[3]) + pow(x, 2)*(23.3333333333333*f[0] - 69.3333333333333*f[1] + 76.0*f[2] - 37.3333333333333*f[3] + 7.33333333333334*f[4]) + x*pow(y, 3)*(42.6666666666667*f[0] - 128.0*f[10] - 42.6666666666667*f[12] + 42.6666666666667*f[13] + 3.5527136788005e-15*f[14] - 42.6666666666667*f[1] - 128.0*f[5] + 128.0*f[6] + 128.0*f[9]) + x*pow(y, 2)*z*(128.0*f[0] - 128.0*f[10] - 128.0*f[15] + 128.0*f[16] + 256.0*f[19] - 128.0*f[1] - 256.0*f[20] - 128.0*f[22] + 128.0*f[23] + 7.105427357601e-15*f[31] - 7.105427357601e-15*f[34] - 256.0*f[5] + 256.0*f[6] + 128.0*f[9]) - x*pow(y, 2)*(80.0*f[0] - 160.0*f[10] + 16.0*f[11] - 32.0*f[12] + 32.0*f[13] + 3.5527136788005e-15*f[14] - 96.0*f[1] + 16.0*f[2] - 192.0*f[5] + 224.0*f[6] - 32.0*f[7] + 144.0*f[9]) + x*y*pow(z, 2)*(128.0*f[0] - 256.0*f[15] + 256.0*f[16] + 256.0*f[19] - 128.0*f[1] - 256.0*f[20] + 128.0*f[25] - 128.0*f[26] - 128.0*f[28] + 128.0*f[29] - 128.0*f[5] + 128.0*f[6]) + x*y*z*(-160.0*f[0] + 32.0*f[10] + 192.0*f[15] - 224.0*f[16] + 32.0*f[17] - 224.0*f[19] + 192.0*f[1] + 256.0*f[20] - 32.0*f[21] + 32.0*f[22] - 32.0*f[23] - 32.0*f[25] + 32.0*f[26] + 32.0*f[28] - 32.0*f[29] - 32.0*f[2] + 1.83556873404693e-14*f[31] + 3.5527136788005e-15*f[32] + 2.96059473233375e-15*f[33] + 5.03301104496738e-15*f[34] + 192.0*f[5] - 224.0*f[6] + 32.0*f[7] - 32.0*f[9]) + x*y*(46.6666666666667*f[0] - 32.0*f[10] + 4.0*f[11] - 5.33333333333334*f[12] + 5.33333333333333*f[13] + 1.03620815631681e-15*f[14] - 69.3333333333333*f[1] + 28.0*f[2] - 5.33333333333333*f[3] - 69.3333333333333*f[5] + 96.0*f[6] - 32.0*f[7] + 5.33333333333333*f[8] + 28.0*f[9]) + x*pow(z, 3)*(42.6666666666667*f[0] - 128.0*f[15] + 128.0*f[16] - 42.6666666666667*f[1] + 128.0*f[25] - 128.0*f[26] - 42.6666666666667*f[31] + 42.6666666666667*f[32] + 3.5527136788005e-15*f[34]) - x*pow(z, 2)*(80.0*f[0] - 192.0*f[15] + 224.0*f[16] - 32.0*f[17] - 96.0*f[1] + 144.0*f[25] - 160.0*f[26] + 16.0*f[27] + 16.0*f[2] - 32.0*f[31] + 32.0*f[32] + 3.5527136788005e-15*f[34]) + x*z*(46.6666666666667*f[0] - 69.3333333333333*f[15] + 96.0*f[16] - 32.0*f[17] + 5.33333333333333*f[18] - 69.3333333333333*f[1] + 28.0*f[25] - 32.0*f[26] + 4.0*f[27] + 28.0*f[2] - 5.33333333333334*f[31] + 5.33333333333333*f[32] + 1.03620815631681e-15*f[34] - 5.33333333333333*f[3]) - x*(8.33333333333333*f[0] - 16.0*f[1] + 12.0*f[2] - 5.33333333333334*f[3] + 1.0*f[4]) + pow(y, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[12] + 10.6666666666667*f[14] - 42.6666666666667*f[5] + 64.0*f[9]) - pow(y, 3)*z*(-42.6666666666667*f[0] + 42.6666666666667*f[12] + 42.6666666666667*f[15] - 128.0*f[19] + 128.0*f[22] - 42.6666666666667*f[24] + 3.5527136788005e-15*f[34] + 128.0*f[5] - 128.0*f[9]) + pow(y, 3)*(-26.6666666666667*f[0] + 74.6666666666667*f[12] - 16.0*f[14] + 96.0*f[5] - 128.0*f[9]) + pow(y, 2)*pow(z, 2)*(64.0*f[0] - 128.0*f[15] + 256.0*f[19] - 128.0*f[22] + 64.0*f[25] - 128.0*f[28] + 64.0*f[30] - 128.0*f[5] + 64.0*f[9]) + pow(y, 2)*z*(-80.0*f[0] + 32.0*f[12] + 96.0*f[15] - 224.0*f[19] + 160.0*f[22] - 32.0*f[24] - 16.0*f[25] + 32.0*f[28] - 16.0*f[30] + 9.473903143468e-15*f[31] + 4.736951571734e-15*f[33] + 3.25665420556713e-15*f[34] + 192.0*f[5] - 144.0*f[9]) + pow(y, 2)*(23.3333333333333*f[0] - 37.3333333333333*f[12] + 7.33333333333334*f[14] - 69.3333333333333*f[5] + 76.0*f[9]) + y*pow(z, 3)*(42.6666666666667*f[0] - 128.0*f[15] + 128.0*f[19] + 128.0*f[25] - 128.0*f[28] - 42.6666666666667*f[31] + 42.6666666666667*f[33] + 3.5527136788005e-15*f[34] - 42.6666666666667*f[5]) - y*pow(z, 2)*(80.0*f[0] - 192.0*f[15] + 224.0*f[19] - 32.0*f[22] + 144.0*f[25] - 160.0*f[28] + 16.0*f[30] - 32.0*f[31] + 32.0*f[33] + 3.5527136788005e-15*f[34] - 96.0*f[5] + 16.0*f[9]) + y*z*(46.6666666666667*f[0] - 5.33333333333333*f[12] - 69.3333333333333*f[15] + 96.0*f[19] - 32.0*f[22] + 5.33333333333333*f[24] + 28.0*f[25] - 32.0*f[28] + 4.0*f[30] - 5.33333333333334*f[31] + 5.33333333333333*f[33] + 1.03620815631681e-15*f[34] - 69.3333333333333*f[5] + 28.0*f[9]) - y*(8.33333333333333*f[0] - 5.33333333333334*f[12] + 1.0*f[14] - 16.0*f[5] + 12.0*f[9]) + pow(z, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[15] + 64.0*f[25] - 42.6666666666667*f[31] + 10.6666666666667*f[34]) + pow(z, 3)*(-26.6666666666667*f[0] + 96.0*f[15] - 128.0*f[25] + 74.6666666666667*f[31] - 16.0*f[34]) + pow(z, 2)*(23.3333333333333*f[0] - 69.3333333333333*f[15] + 76.0*f[25] - 37.3333333333333*f[31] + 7.33333333333334*f[34]) - z*(8.33333333333333*f[0] - 16.0*f[15] + 12.0*f[25] - 5.33333333333334*f[31] + 1.0*f[34]);
}

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
  if (component>2 || component<0) return 0.;
  int n = subdivision+1;
  int N = order*n+1;
  int values_per_element = N*(N+1)*(N+2)/6;
  vec3 lamn = lam*n;
  lam = lamn-floor(lamn);
  ivec3 s = order*ivec3(lamn);

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
      s.z+=order;
      d.z = -1;
    }
    else { // third tet in first prism 1,4,5,7
      x = 1-lam.x-lam.y;
      z = 1-lam.z-lam.y;
      s.z+=order;
      s.x+=order;
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
      s.x += order;
      s.y += order;
      s.z += order;
    }
    else if(lam.z<lam.y) { // second tet in second prism 1,2,3,7
      x = 1-lam.x-lam.z;
      y = 1-lam.y;
      s.x+=order;
      s.y+=order;
      d.x = -1;
      d.y = -1;
      special_order = 2;
    }
    else { // third tet in second prism 1,2,5,7
      x = 1-lam.x;
      y = 2-lam.x-lam.y-lam.z;
      z = lam.z+lam.x-1;
      s.x+=order;
      s.y+=order;
      d.x = -1;
      d.y = -1;
      special_order = 3;
    }
  }
  if(order==1)
    return InterpolateTetP1( element, coefficients, N, d, s, special_order, vec3(x,y,z) , component);
  if(order==2)
    return InterpolateTetP2( element, coefficients, N, d, s, special_order, vec3(x,y,z), component );
  if(order==3)
    return InterpolateTetP3( element, coefficients, N, d, s, special_order, vec3(x,y,z) , component);
  if(order==4)
    return InterpolateTetP4( element, coefficients, N, d, s, special_order, vec3(x,y,z) , component);
}

