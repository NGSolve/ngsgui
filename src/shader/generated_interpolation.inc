#line 1
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

#if ORDER==1
float InterpolateHex(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[8];
  int ii=0;
  for (int i=0; i<=1; i++) {
    for (int j=0; j<=1; j++) {
      for (int k=0; k<=1; k++) {
        f[ii] = getValue(coefficients, first+k+j*dy+i*dz)[component];
        ii++;
      }
    }
  }
  return 1.0*f[0] - 1.0*x*y*z*(f[0] - f[1] - f[2] + f[3] - f[4] + f[5] + f[6] - f[7]) + 1.0*x*y*(f[0] - f[1] - f[2] + f[3]) + 1.0*x*z*(f[0] - f[1] - f[4] + f[5]) - 1.0*x*(f[0] - f[1]) + 1.0*y*z*(f[0] - f[2] - f[4] + f[6]) - 1.0*y*(f[0] - f[2]) - 1.0*z*(f[0] - f[4]);
}
#endif

#if ORDER==1
vec3 InterpolateHexVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[8];
  int ii=0;
  for (int i=0; i<=1; i++) {
    for (int j=0; j<=1; j++) {
      for (int k=0; k<=1; k++) {
        f[ii] = getValue(coefficients, first+k+j*dy+i*dz).xyz;
        ii++;
      }
    }
  }
  return 1.0*f[0] - 1.0*x*y*z*(f[0] - f[1] - f[2] + f[3] - f[4] + f[5] + f[6] - f[7]) + 1.0*x*y*(f[0] - f[1] - f[2] + f[3]) + 1.0*x*z*(f[0] - f[1] - f[4] + f[5]) - 1.0*x*(f[0] - f[1]) + 1.0*y*z*(f[0] - f[2] - f[4] + f[6]) - 1.0*y*(f[0] - f[2]) - 1.0*z*(f[0] - f[4]);
}
#endif

#if ORDER==2
float InterpolateHex(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[27];
  int ii=0;
  for (int i=0; i<=2; i++) {
    for (int j=0; j<=2; j++) {
      for (int k=0; k<=2; k++) {
        f[ii] = getValue(coefficients, first+k+j*dy+i*dz)[component];
        ii++;
      }
    }
  }
  return 1.0*f[0] + pow(x, 2)*pow(y, 2)*pow(z, 2)*(8.0*f[0] + 32.0*f[10] - 16.0*f[11] + 32.0*f[12] - 64.0*f[13] + 32.0*f[14] - 16.0*f[15] + 32.0*f[16] - 16.0*f[17] + 8.0*f[18] - 16.0*f[19] - 16.0*f[1] + 8.0*f[20] - 16.0*f[21] + 32.0*f[22] - 16.0*f[23] + 8.0*f[24] - 16.0*f[25] + 8.0*f[26] + 8.0*f[2] - 16.0*f[3] + 32.0*f[4] - 16.0*f[5] + 8.0*f[6] - 16.0*f[7] + 8.0*f[8] - 16.0*f[9]) - pow(x, 2)*pow(y, 2)*z*(12.0*f[0] + 32.0*f[10] - 16.0*f[11] + 32.0*f[12] - 64.0*f[13] + 32.0*f[14] - 16.0*f[15] + 32.0*f[16] - 16.0*f[17] + 4.0*f[18] - 8.0*f[19] - 24.0*f[1] + 4.0*f[20] - 8.0*f[21] + 16.0*f[22] - 8.0*f[23] + 4.0*f[24] - 8.0*f[25] + 4.0*f[26] + 12.0*f[2] - 24.0*f[3] + 48.0*f[4] - 24.0*f[5] + 12.0*f[6] - 24.0*f[7] + 12.0*f[8] - 16.0*f[9]) + pow(x, 2)*pow(y, 2)*(4.0*f[0] - 8.0*f[1] + 4.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 4.0*f[6] - 8.0*f[7] + 4.0*f[8]) - pow(x, 2)*y*pow(z, 2)*(12.0*f[0] + 48.0*f[10] - 24.0*f[11] + 32.0*f[12] - 64.0*f[13] + 32.0*f[14] - 8.0*f[15] + 16.0*f[16] - 8.0*f[17] + 12.0*f[18] - 24.0*f[19] - 24.0*f[1] + 12.0*f[20] - 16.0*f[21] + 32.0*f[22] - 16.0*f[23] + 4.0*f[24] - 8.0*f[25] + 4.0*f[26] + 12.0*f[2] - 16.0*f[3] + 32.0*f[4] - 16.0*f[5] + 4.0*f[6] - 8.0*f[7] + 4.0*f[8] - 24.0*f[9]) + pow(x, 2)*y*z*(18.0*f[0] + 48.0*f[10] - 24.0*f[11] + 32.0*f[12] - 64.0*f[13] + 32.0*f[14] - 8.0*f[15] + 16.0*f[16] - 8.0*f[17] + 6.0*f[18] - 12.0*f[19] - 36.0*f[1] + 6.0*f[20] - 8.0*f[21] + 16.0*f[22] - 8.0*f[23] + 2.0*f[24] - 4.0*f[25] + 2.0*f[26] + 18.0*f[2] - 24.0*f[3] + 48.0*f[4] - 24.0*f[5] + 6.0*f[6] - 12.0*f[7] + 6.0*f[8] - 24.0*f[9]) - pow(x, 2)*y*(6.0*f[0] - 12.0*f[1] + 6.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 2.0*f[6] - 4.0*f[7] + 2.0*f[8]) + pow(x, 2)*pow(z, 2)*(4.0*f[0] + 16.0*f[10] - 8.0*f[11] + 4.0*f[18] - 8.0*f[19] - 8.0*f[1] + 4.0*f[20] + 4.0*f[2] - 8.0*f[9]) - pow(x, 2)*z*(6.0*f[0] + 16.0*f[10] - 8.0*f[11] + 2.0*f[18] - 4.0*f[19] - 12.0*f[1] + 2.0*f[20] + 6.0*f[2] - 8.0*f[9]) + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) - x*pow(y, 2)*pow(z, 2)*(12.0*f[0] + 32.0*f[10] - 8.0*f[11] + 48.0*f[12] - 64.0*f[13] + 16.0*f[14] - 24.0*f[15] + 32.0*f[16] - 8.0*f[17] + 12.0*f[18] - 16.0*f[19] - 16.0*f[1] + 4.0*f[20] - 24.0*f[21] + 32.0*f[22] - 8.0*f[23] + 12.0*f[24] - 16.0*f[25] + 4.0*f[26] + 4.0*f[2] - 24.0*f[3] + 32.0*f[4] - 8.0*f[5] + 12.0*f[6] - 16.0*f[7] + 4.0*f[8] - 24.0*f[9]) + x*pow(y, 2)*z*(18.0*f[0] + 32.0*f[10] - 8.0*f[11] + 48.0*f[12] - 64.0*f[13] + 16.0*f[14] - 24.0*f[15] + 32.0*f[16] - 8.0*f[17] + 6.0*f[18] - 8.0*f[19] - 24.0*f[1] + 2.0*f[20] - 12.0*f[21] + 16.0*f[22] - 4.0*f[23] + 6.0*f[24] - 8.0*f[25] + 2.0*f[26] + 6.0*f[2] - 36.0*f[3] + 48.0*f[4] - 12.0*f[5] + 18.0*f[6] - 24.0*f[7] + 6.0*f[8] - 24.0*f[9]) - x*pow(y, 2)*(6.0*f[0] - 8.0*f[1] + 2.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 6.0*f[6] - 8.0*f[7] + 2.0*f[8]) + x*y*pow(z, 2)*(18.0*f[0] + 48.0*f[10] - 12.0*f[11] + 48.0*f[12] - 64.0*f[13] + 16.0*f[14] - 12.0*f[15] + 16.0*f[16] - 4.0*f[17] + 18.0*f[18] - 24.0*f[19] - 24.0*f[1] + 6.0*f[20] - 24.0*f[21] + 32.0*f[22] - 8.0*f[23] + 6.0*f[24] - 8.0*f[25] + 2.0*f[26] + 6.0*f[2] - 24.0*f[3] + 32.0*f[4] - 8.0*f[5] + 6.0*f[6] - 8.0*f[7] + 2.0*f[8] - 36.0*f[9]) - x*y*z*(27.0*f[0] + 48.0*f[10] - 12.0*f[11] + 48.0*f[12] - 64.0*f[13] + 16.0*f[14] - 12.0*f[15] + 16.0*f[16] - 4.0*f[17] + 9.0*f[18] - 12.0*f[19] - 36.0*f[1] + 3.0*f[20] - 12.0*f[21] + 16.0*f[22] - 4.0*f[23] + 3.0*f[24] - 4.0*f[25] + 1.0*f[26] + 9.0*f[2] - 36.0*f[3] + 48.0*f[4] - 12.0*f[5] + 9.0*f[6] - 12.0*f[7] + 3.0*f[8] - 36.0*f[9]) + x*y*(9.0*f[0] - 12.0*f[1] + 3.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 3.0*f[6] - 4.0*f[7] + 1.0*f[8]) - x*pow(z, 2)*(6.0*f[0] + 16.0*f[10] - 4.0*f[11] + 6.0*f[18] - 8.0*f[19] - 8.0*f[1] + 2.0*f[20] + 2.0*f[2] - 12.0*f[9]) + x*z*(9.0*f[0] + 16.0*f[10] - 4.0*f[11] + 3.0*f[18] - 4.0*f[19] - 12.0*f[1] + 1.0*f[20] + 3.0*f[2] - 12.0*f[9]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*pow(z, 2)*(4.0*f[0] + 16.0*f[12] - 8.0*f[15] + 4.0*f[18] - 8.0*f[21] + 4.0*f[24] - 8.0*f[3] + 4.0*f[6] - 8.0*f[9]) - pow(y, 2)*z*(6.0*f[0] + 16.0*f[12] - 8.0*f[15] + 2.0*f[18] - 4.0*f[21] + 2.0*f[24] - 12.0*f[3] + 6.0*f[6] - 8.0*f[9]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[6]) - y*pow(z, 2)*(6.0*f[0] + 16.0*f[12] - 4.0*f[15] + 6.0*f[18] - 8.0*f[21] + 2.0*f[24] - 8.0*f[3] + 2.0*f[6] - 12.0*f[9]) + y*z*(9.0*f[0] + 16.0*f[12] - 4.0*f[15] + 3.0*f[18] - 4.0*f[21] + 1.0*f[24] - 12.0*f[3] + 3.0*f[6] - 12.0*f[9]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[6]) + pow(z, 2)*(2.0*f[0] + 2.0*f[18] - 4.0*f[9]) - z*(3.0*f[0] + 1.0*f[18] - 4.0*f[9]);
}
#endif

#if ORDER==2
vec3 InterpolateHexVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[27];
  int ii=0;
  for (int i=0; i<=2; i++) {
    for (int j=0; j<=2; j++) {
      for (int k=0; k<=2; k++) {
        f[ii] = getValue(coefficients, first+k+j*dy+i*dz).xyz;
        ii++;
      }
    }
  }
  return 1.0*f[0] + pow(x, 2)*pow(y, 2)*pow(z, 2)*(8.0*f[0] + 32.0*f[10] - 16.0*f[11] + 32.0*f[12] - 64.0*f[13] + 32.0*f[14] - 16.0*f[15] + 32.0*f[16] - 16.0*f[17] + 8.0*f[18] - 16.0*f[19] - 16.0*f[1] + 8.0*f[20] - 16.0*f[21] + 32.0*f[22] - 16.0*f[23] + 8.0*f[24] - 16.0*f[25] + 8.0*f[26] + 8.0*f[2] - 16.0*f[3] + 32.0*f[4] - 16.0*f[5] + 8.0*f[6] - 16.0*f[7] + 8.0*f[8] - 16.0*f[9]) - pow(x, 2)*pow(y, 2)*z*(12.0*f[0] + 32.0*f[10] - 16.0*f[11] + 32.0*f[12] - 64.0*f[13] + 32.0*f[14] - 16.0*f[15] + 32.0*f[16] - 16.0*f[17] + 4.0*f[18] - 8.0*f[19] - 24.0*f[1] + 4.0*f[20] - 8.0*f[21] + 16.0*f[22] - 8.0*f[23] + 4.0*f[24] - 8.0*f[25] + 4.0*f[26] + 12.0*f[2] - 24.0*f[3] + 48.0*f[4] - 24.0*f[5] + 12.0*f[6] - 24.0*f[7] + 12.0*f[8] - 16.0*f[9]) + pow(x, 2)*pow(y, 2)*(4.0*f[0] - 8.0*f[1] + 4.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 4.0*f[6] - 8.0*f[7] + 4.0*f[8]) - pow(x, 2)*y*pow(z, 2)*(12.0*f[0] + 48.0*f[10] - 24.0*f[11] + 32.0*f[12] - 64.0*f[13] + 32.0*f[14] - 8.0*f[15] + 16.0*f[16] - 8.0*f[17] + 12.0*f[18] - 24.0*f[19] - 24.0*f[1] + 12.0*f[20] - 16.0*f[21] + 32.0*f[22] - 16.0*f[23] + 4.0*f[24] - 8.0*f[25] + 4.0*f[26] + 12.0*f[2] - 16.0*f[3] + 32.0*f[4] - 16.0*f[5] + 4.0*f[6] - 8.0*f[7] + 4.0*f[8] - 24.0*f[9]) + pow(x, 2)*y*z*(18.0*f[0] + 48.0*f[10] - 24.0*f[11] + 32.0*f[12] - 64.0*f[13] + 32.0*f[14] - 8.0*f[15] + 16.0*f[16] - 8.0*f[17] + 6.0*f[18] - 12.0*f[19] - 36.0*f[1] + 6.0*f[20] - 8.0*f[21] + 16.0*f[22] - 8.0*f[23] + 2.0*f[24] - 4.0*f[25] + 2.0*f[26] + 18.0*f[2] - 24.0*f[3] + 48.0*f[4] - 24.0*f[5] + 6.0*f[6] - 12.0*f[7] + 6.0*f[8] - 24.0*f[9]) - pow(x, 2)*y*(6.0*f[0] - 12.0*f[1] + 6.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 2.0*f[6] - 4.0*f[7] + 2.0*f[8]) + pow(x, 2)*pow(z, 2)*(4.0*f[0] + 16.0*f[10] - 8.0*f[11] + 4.0*f[18] - 8.0*f[19] - 8.0*f[1] + 4.0*f[20] + 4.0*f[2] - 8.0*f[9]) - pow(x, 2)*z*(6.0*f[0] + 16.0*f[10] - 8.0*f[11] + 2.0*f[18] - 4.0*f[19] - 12.0*f[1] + 2.0*f[20] + 6.0*f[2] - 8.0*f[9]) + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) - x*pow(y, 2)*pow(z, 2)*(12.0*f[0] + 32.0*f[10] - 8.0*f[11] + 48.0*f[12] - 64.0*f[13] + 16.0*f[14] - 24.0*f[15] + 32.0*f[16] - 8.0*f[17] + 12.0*f[18] - 16.0*f[19] - 16.0*f[1] + 4.0*f[20] - 24.0*f[21] + 32.0*f[22] - 8.0*f[23] + 12.0*f[24] - 16.0*f[25] + 4.0*f[26] + 4.0*f[2] - 24.0*f[3] + 32.0*f[4] - 8.0*f[5] + 12.0*f[6] - 16.0*f[7] + 4.0*f[8] - 24.0*f[9]) + x*pow(y, 2)*z*(18.0*f[0] + 32.0*f[10] - 8.0*f[11] + 48.0*f[12] - 64.0*f[13] + 16.0*f[14] - 24.0*f[15] + 32.0*f[16] - 8.0*f[17] + 6.0*f[18] - 8.0*f[19] - 24.0*f[1] + 2.0*f[20] - 12.0*f[21] + 16.0*f[22] - 4.0*f[23] + 6.0*f[24] - 8.0*f[25] + 2.0*f[26] + 6.0*f[2] - 36.0*f[3] + 48.0*f[4] - 12.0*f[5] + 18.0*f[6] - 24.0*f[7] + 6.0*f[8] - 24.0*f[9]) - x*pow(y, 2)*(6.0*f[0] - 8.0*f[1] + 2.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 6.0*f[6] - 8.0*f[7] + 2.0*f[8]) + x*y*pow(z, 2)*(18.0*f[0] + 48.0*f[10] - 12.0*f[11] + 48.0*f[12] - 64.0*f[13] + 16.0*f[14] - 12.0*f[15] + 16.0*f[16] - 4.0*f[17] + 18.0*f[18] - 24.0*f[19] - 24.0*f[1] + 6.0*f[20] - 24.0*f[21] + 32.0*f[22] - 8.0*f[23] + 6.0*f[24] - 8.0*f[25] + 2.0*f[26] + 6.0*f[2] - 24.0*f[3] + 32.0*f[4] - 8.0*f[5] + 6.0*f[6] - 8.0*f[7] + 2.0*f[8] - 36.0*f[9]) - x*y*z*(27.0*f[0] + 48.0*f[10] - 12.0*f[11] + 48.0*f[12] - 64.0*f[13] + 16.0*f[14] - 12.0*f[15] + 16.0*f[16] - 4.0*f[17] + 9.0*f[18] - 12.0*f[19] - 36.0*f[1] + 3.0*f[20] - 12.0*f[21] + 16.0*f[22] - 4.0*f[23] + 3.0*f[24] - 4.0*f[25] + 1.0*f[26] + 9.0*f[2] - 36.0*f[3] + 48.0*f[4] - 12.0*f[5] + 9.0*f[6] - 12.0*f[7] + 3.0*f[8] - 36.0*f[9]) + x*y*(9.0*f[0] - 12.0*f[1] + 3.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 3.0*f[6] - 4.0*f[7] + 1.0*f[8]) - x*pow(z, 2)*(6.0*f[0] + 16.0*f[10] - 4.0*f[11] + 6.0*f[18] - 8.0*f[19] - 8.0*f[1] + 2.0*f[20] + 2.0*f[2] - 12.0*f[9]) + x*z*(9.0*f[0] + 16.0*f[10] - 4.0*f[11] + 3.0*f[18] - 4.0*f[19] - 12.0*f[1] + 1.0*f[20] + 3.0*f[2] - 12.0*f[9]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*pow(z, 2)*(4.0*f[0] + 16.0*f[12] - 8.0*f[15] + 4.0*f[18] - 8.0*f[21] + 4.0*f[24] - 8.0*f[3] + 4.0*f[6] - 8.0*f[9]) - pow(y, 2)*z*(6.0*f[0] + 16.0*f[12] - 8.0*f[15] + 2.0*f[18] - 4.0*f[21] + 2.0*f[24] - 12.0*f[3] + 6.0*f[6] - 8.0*f[9]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[6]) - y*pow(z, 2)*(6.0*f[0] + 16.0*f[12] - 4.0*f[15] + 6.0*f[18] - 8.0*f[21] + 2.0*f[24] - 8.0*f[3] + 2.0*f[6] - 12.0*f[9]) + y*z*(9.0*f[0] + 16.0*f[12] - 4.0*f[15] + 3.0*f[18] - 4.0*f[21] + 1.0*f[24] - 12.0*f[3] + 3.0*f[6] - 12.0*f[9]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[6]) + pow(z, 2)*(2.0*f[0] + 2.0*f[18] - 4.0*f[9]) - z*(3.0*f[0] + 1.0*f[18] - 4.0*f[9]);
}
#endif

#if ORDER==1
float InterpolatePrism(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[6];
  int ii=0;
  for (int k=0; k<=1; k++) {
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
  first += dz;
  }
  return 1.0*f[0] + 1.0*x*z*(f[0] - f[1] - f[3] + f[4]) - 1.0*x*(f[0] - f[1]) + 1.0*y*z*(f[0] - f[2] - f[3] + f[5]) - 1.0*y*(f[0] - f[2]) - 1.0*z*(f[0] - f[3]);
}
#endif

#if ORDER==1
vec3 InterpolatePrismVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[6];
  int ii=0;
  for (int k=0; k<=1; k++) {
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
  first += dz;
  }
  return 1.0*f[0] + 1.0*x*z*(f[0] - f[1] - f[3] + f[4]) - 1.0*x*(f[0] - f[1]) + 1.0*y*z*(f[0] - f[2] - f[3] + f[5]) - 1.0*y*(f[0] - f[2]) - 1.0*z*(f[0] - f[3]);
}
#endif

#if ORDER==2
float InterpolatePrism(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[18];
  int ii=0;
  for (int k=0; k<=2; k++) {
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
  first += dz;
  }
  return 1.0*f[0] + pow(x, 2)*pow(z, 2)*(4.0*f[0] + 4.0*f[12] - 8.0*f[13] + 4.0*f[14] - 8.0*f[1] + 4.0*f[2] - 8.0*f[6] + 16.0*f[7] - 8.0*f[8]) - pow(x, 2)*z*(6.0*f[0] + 2.0*f[12] - 4.0*f[13] + 2.0*f[14] - 12.0*f[1] + 6.0*f[2] - 8.0*f[6] + 16.0*f[7] - 8.0*f[8]) + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + x*y*pow(z, 2)*(8.0*f[0] - 16.0*f[10] + 8.0*f[12] - 8.0*f[13] - 8.0*f[15] + 8.0*f[16] - 8.0*f[1] - 8.0*f[3] + 8.0*f[4] - 16.0*f[6] + 16.0*f[7] + 16.0*f[9]) - x*y*z*(12.0*f[0] - 16.0*f[10] + 4.0*f[12] - 4.0*f[13] - 4.0*f[15] + 4.0*f[16] - 12.0*f[1] - 12.0*f[3] + 12.0*f[4] - 16.0*f[6] + 16.0*f[7] + 16.0*f[9]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*pow(z, 2)*(6.0*f[0] + 6.0*f[12] - 8.0*f[13] + 2.0*f[14] - 8.0*f[1] + 2.0*f[2] - 12.0*f[6] + 16.0*f[7] - 4.0*f[8]) + x*z*(9.0*f[0] + 3.0*f[12] - 4.0*f[13] + 1.0*f[14] - 12.0*f[1] + 3.0*f[2] - 12.0*f[6] + 16.0*f[7] - 4.0*f[8]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*pow(z, 2)*(4.0*f[0] - 8.0*f[11] + 4.0*f[12] - 8.0*f[15] + 4.0*f[17] - 8.0*f[3] + 4.0*f[5] - 8.0*f[6] + 16.0*f[9]) - pow(y, 2)*z*(6.0*f[0] - 8.0*f[11] + 2.0*f[12] - 4.0*f[15] + 2.0*f[17] - 12.0*f[3] + 6.0*f[5] - 8.0*f[6] + 16.0*f[9]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*pow(z, 2)*(6.0*f[0] - 4.0*f[11] + 6.0*f[12] - 8.0*f[15] + 2.0*f[17] - 8.0*f[3] + 2.0*f[5] - 12.0*f[6] + 16.0*f[9]) + y*z*(9.0*f[0] - 4.0*f[11] + 3.0*f[12] - 4.0*f[15] + 1.0*f[17] - 12.0*f[3] + 3.0*f[5] - 12.0*f[6] + 16.0*f[9]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]) + pow(z, 2)*(2.0*f[0] + 2.0*f[12] - 4.0*f[6]) - z*(3.0*f[0] + 1.0*f[12] - 4.0*f[6]);
}
#endif

#if ORDER==2
vec3 InterpolatePrismVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[18];
  int ii=0;
  for (int k=0; k<=2; k++) {
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
  first += dz;
  }
  return 1.0*f[0] + pow(x, 2)*pow(z, 2)*(4.0*f[0] + 4.0*f[12] - 8.0*f[13] + 4.0*f[14] - 8.0*f[1] + 4.0*f[2] - 8.0*f[6] + 16.0*f[7] - 8.0*f[8]) - pow(x, 2)*z*(6.0*f[0] + 2.0*f[12] - 4.0*f[13] + 2.0*f[14] - 12.0*f[1] + 6.0*f[2] - 8.0*f[6] + 16.0*f[7] - 8.0*f[8]) + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + x*y*pow(z, 2)*(8.0*f[0] - 16.0*f[10] + 8.0*f[12] - 8.0*f[13] - 8.0*f[15] + 8.0*f[16] - 8.0*f[1] - 8.0*f[3] + 8.0*f[4] - 16.0*f[6] + 16.0*f[7] + 16.0*f[9]) - x*y*z*(12.0*f[0] - 16.0*f[10] + 4.0*f[12] - 4.0*f[13] - 4.0*f[15] + 4.0*f[16] - 12.0*f[1] - 12.0*f[3] + 12.0*f[4] - 16.0*f[6] + 16.0*f[7] + 16.0*f[9]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*pow(z, 2)*(6.0*f[0] + 6.0*f[12] - 8.0*f[13] + 2.0*f[14] - 8.0*f[1] + 2.0*f[2] - 12.0*f[6] + 16.0*f[7] - 4.0*f[8]) + x*z*(9.0*f[0] + 3.0*f[12] - 4.0*f[13] + 1.0*f[14] - 12.0*f[1] + 3.0*f[2] - 12.0*f[6] + 16.0*f[7] - 4.0*f[8]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*pow(z, 2)*(4.0*f[0] - 8.0*f[11] + 4.0*f[12] - 8.0*f[15] + 4.0*f[17] - 8.0*f[3] + 4.0*f[5] - 8.0*f[6] + 16.0*f[9]) - pow(y, 2)*z*(6.0*f[0] - 8.0*f[11] + 2.0*f[12] - 4.0*f[15] + 2.0*f[17] - 12.0*f[3] + 6.0*f[5] - 8.0*f[6] + 16.0*f[9]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*pow(z, 2)*(6.0*f[0] - 4.0*f[11] + 6.0*f[12] - 8.0*f[15] + 2.0*f[17] - 8.0*f[3] + 2.0*f[5] - 12.0*f[6] + 16.0*f[9]) + y*z*(9.0*f[0] - 4.0*f[11] + 3.0*f[12] - 4.0*f[15] + 1.0*f[17] - 12.0*f[3] + 3.0*f[5] - 12.0*f[6] + 16.0*f[9]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]) + pow(z, 2)*(2.0*f[0] + 2.0*f[12] - 4.0*f[6]) - z*(3.0*f[0] + 1.0*f[12] - 4.0*f[6]);
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
float InterpolateHex(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return InterpolateHex(values, lam, first, dy, dz, component);
}
vec3 InterpolateHexVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return InterpolateHexVec(values, lam, first, dy, dz, component);
}
float InterpolatePrism(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    }
    return InterpolatePrism(values, lam, first, dx, dy, dz, component);
}
vec3 InterpolatePrismVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    }
    return InterpolatePrismVec(values, lam, first, dx, dy, dz, component);
}
