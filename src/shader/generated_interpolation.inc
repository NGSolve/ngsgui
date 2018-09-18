#line 1
#ifdef ET_SEGM
#if ORDER==1
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[2];
  int offsety = 0;
  for (int i=0; i<=1; i++) {
    f[i] = getValue(coefficients, first+i)[component];
  }
  return f[0] + x*(-f[0] + f[1]);
}
#endif
#endif

#ifdef ET_SEGM
#if ORDER==1
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[2];
  int offsety = 0;
  for (int i=0; i<=1; i++) {
    f[i] = getValue(coefficients, first+i).xyz;
  }
  return f[0] + x*(-f[0] + f[1]);
}
#endif
#endif

#ifdef ET_SEGM
#if ORDER==2
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[3];
  int offsety = 0;
  for (int i=0; i<=2; i++) {
    f[i] = getValue(coefficients, first+i)[component];
  }
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2]));
}
#endif
#endif

#ifdef ET_SEGM
#if ORDER==2
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[3];
  int offsety = 0;
  for (int i=0; i<=2; i++) {
    f[i] = getValue(coefficients, first+i).xyz;
  }
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2]));
}
#endif
#endif

#ifdef ET_SEGM
#if ORDER==3
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[4];
  int offsety = 0;
  for (int i=0; i<=3; i++) {
    f[i] = getValue(coefficients, first+i)[component];
  }
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3])));
}
#endif
#endif

#ifdef ET_SEGM
#if ORDER==3
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[4];
  int offsety = 0;
  for (int i=0; i<=3; i++) {
    f[i] = getValue(coefficients, first+i).xyz;
  }
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3])));
}
#endif
#endif

#ifdef ET_TRIG
#if ORDER==1
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
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
  return f[0] + x*(-f[0] + f[1]) + y*(-f[0] + f[2]);
}
#endif
#endif

#ifdef ET_TRIG
#if ORDER==1
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
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
  return f[0] + x*(-f[0] + f[1]) + y*(-f[0] + f[2]);
}
#endif
#endif

#ifdef ET_TRIG
#if ORDER==2
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2]) + y*(4*f[0] - 4*f[1] - 4*f[3] + 4*f[4])) + y*(-3*f[0] + 4*f[3] - f[5] + y*(2*f[0] - 4*f[3] + 2*f[5]));
}
#endif
#endif

#ifdef ET_TRIG
#if ORDER==2
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2]) + y*(4*f[0] - 4*f[1] - 4*f[3] + 4*f[4])) + y*(-3*f[0] + 4*f[3] - f[5] + y*(2*f[0] - 4*f[3] + 2*f[5]));
}
#endif
#endif

#ifdef ET_TRIG
#if ORDER==3
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
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
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3]) + y*(-27.0/2.0*f[0] + 27*f[1] - 27.0/2.0*f[2] + (27.0/2.0)*f[4] - 27*f[5] + (27.0/2.0)*f[6])) + y*(18*f[0] - 45.0/2.0*f[1] + (9.0/2.0)*f[2] - 45.0/2.0*f[4] + 27*f[5] - 9.0/2.0*f[6] + (9.0/2.0)*f[7] - 9.0/2.0*f[8] + y*(-27.0/2.0*f[0] + (27.0/2.0)*f[1] + 27*f[4] - 27*f[5] - 27.0/2.0*f[7] + (27.0/2.0)*f[8]))) + y*(-11.0/2.0*f[0] + 9*f[4] - 9.0/2.0*f[7] + f[9] + y*(9*f[0] - 45.0/2.0*f[4] + 18*f[7] - 9.0/2.0*f[9] + y*(-9.0/2.0*f[0] + (27.0/2.0)*f[4] - 27.0/2.0*f[7] + (9.0/2.0)*f[9])));
}
#endif
#endif

#ifdef ET_TRIG
#if ORDER==3
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int component) {
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
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3]) + y*(-27.0/2.0*f[0] + 27*f[1] - 27.0/2.0*f[2] + (27.0/2.0)*f[4] - 27*f[5] + (27.0/2.0)*f[6])) + y*(18*f[0] - 45.0/2.0*f[1] + (9.0/2.0)*f[2] - 45.0/2.0*f[4] + 27*f[5] - 9.0/2.0*f[6] + (9.0/2.0)*f[7] - 9.0/2.0*f[8] + y*(-27.0/2.0*f[0] + (27.0/2.0)*f[1] + 27*f[4] - 27*f[5] - 27.0/2.0*f[7] + (27.0/2.0)*f[8]))) + y*(-11.0/2.0*f[0] + 9*f[4] - 9.0/2.0*f[7] + f[9] + y*(9*f[0] - 45.0/2.0*f[4] + 18*f[7] - 9.0/2.0*f[9] + y*(-9.0/2.0*f[0] + (27.0/2.0)*f[4] - 27.0/2.0*f[7] + (9.0/2.0)*f[9])));
}
#endif
#endif

#ifdef ET_TET
#if ORDER==1
float  EvaluateElementInternal(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
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
  return f[0] + x*(-f[0] + f[1]) + y*(-f[0] + f[2]) + z*(-f[0] + f[3]);
}
#endif
#endif

#ifdef ET_TET
#if ORDER==1
vec3  EvaluateElementInternalVec(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
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
  return f[0] + x*(-f[0] + f[1]) + y*(-f[0] + f[2]) + z*(-f[0] + f[3]);
}
#endif
#endif

#ifdef ET_TET
#if ORDER==2
float  EvaluateElementInternal(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2]) + y*(4*f[0] - 4*f[1] - 4*f[3] + 4*f[4]) + z*(4*f[0] - 4*f[1] - 4*f[6] + 4*f[7])) + y*(-3*f[0] + 4*f[3] - f[5] + y*(2*f[0] - 4*f[3] + 2*f[5]) + z*(4*f[0] - 4*f[3] - 4*f[6] + 4*f[8])) + z*(-3*f[0] + 4*f[6] - f[9] + z*(2*f[0] - 4*f[6] + 2*f[9]));
}
#endif
#endif

#ifdef ET_TET
#if ORDER==2
vec3  EvaluateElementInternalVec(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2]) + y*(4*f[0] - 4*f[1] - 4*f[3] + 4*f[4]) + z*(4*f[0] - 4*f[1] - 4*f[6] + 4*f[7])) + y*(-3*f[0] + 4*f[3] - f[5] + y*(2*f[0] - 4*f[3] + 2*f[5]) + z*(4*f[0] - 4*f[3] - 4*f[6] + 4*f[8])) + z*(-3*f[0] + 4*f[6] - f[9] + z*(2*f[0] - 4*f[6] + 2*f[9]));
}
#endif
#endif

#ifdef ET_TET
#if ORDER==3
float  EvaluateElementInternal(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
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
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3]) + y*(-27.0/2.0*f[0] + 27*f[1] - 27.0/2.0*f[2] + (27.0/2.0)*f[4] - 27*f[5] + (27.0/2.0)*f[6]) + z*(-27.0/2.0*f[0] + (27.0/2.0)*f[10] - 27*f[11] + (27.0/2.0)*f[12] + 27*f[1] - 27.0/2.0*f[2])) + y*(18*f[0] - 45.0/2.0*f[1] + (9.0/2.0)*f[2] - 45.0/2.0*f[4] + 27*f[5] - 9.0/2.0*f[6] + (9.0/2.0)*f[7] - 9.0/2.0*f[8] + y*(-27.0/2.0*f[0] + (27.0/2.0)*f[1] + 27*f[4] - 27*f[5] - 27.0/2.0*f[7] + (27.0/2.0)*f[8]) + z*(-27*f[0] + 27*f[10] - 27*f[11] - 27*f[13] + 27*f[14] + 27*f[1] + 27*f[4] - 27*f[5])) + z*(18*f[0] - 45.0/2.0*f[10] + 27*f[11] - 9.0/2.0*f[12] + (9.0/2.0)*f[16] - 9.0/2.0*f[17] - 45.0/2.0*f[1] + (9.0/2.0)*f[2] + z*(-27.0/2.0*f[0] + 27*f[10] - 27*f[11] - 27.0/2.0*f[16] + (27.0/2.0)*f[17] + (27.0/2.0)*f[1]))) + y*(-11.0/2.0*f[0] + 9*f[4] - 9.0/2.0*f[7] + f[9] + y*(9*f[0] - 45.0/2.0*f[4] + 18*f[7] - 9.0/2.0*f[9] + y*(-9.0/2.0*f[0] + (27.0/2.0)*f[4] - 27.0/2.0*f[7] + (9.0/2.0)*f[9]) + z*(-27.0/2.0*f[0] + (27.0/2.0)*f[10] - 27*f[13] + (27.0/2.0)*f[15] + 27*f[4] - 27.0/2.0*f[7])) + z*(18*f[0] - 45.0/2.0*f[10] + 27*f[13] - 9.0/2.0*f[15] + (9.0/2.0)*f[16] - 9.0/2.0*f[18] - 45.0/2.0*f[4] + (9.0/2.0)*f[7] + z*(-27.0/2.0*f[0] + 27*f[10] - 27*f[13] - 27.0/2.0*f[16] + (27.0/2.0)*f[18] + (27.0/2.0)*f[4]))) + z*(-11.0/2.0*f[0] + 9*f[10] - 9.0/2.0*f[16] + f[19] + z*(9*f[0] - 45.0/2.0*f[10] + 18*f[16] - 9.0/2.0*f[19] + z*(-9.0/2.0*f[0] + (27.0/2.0)*f[10] - 27.0/2.0*f[16] + (9.0/2.0)*f[19])));
}
#endif
#endif

#ifdef ET_TET
#if ORDER==3
vec3  EvaluateElementInternalVec(int element, samplerBuffer coefficients, int N, ivec3 d, ivec3 s, int special_order, vec3 lam, int component) {
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
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3]) + y*(-27.0/2.0*f[0] + 27*f[1] - 27.0/2.0*f[2] + (27.0/2.0)*f[4] - 27*f[5] + (27.0/2.0)*f[6]) + z*(-27.0/2.0*f[0] + (27.0/2.0)*f[10] - 27*f[11] + (27.0/2.0)*f[12] + 27*f[1] - 27.0/2.0*f[2])) + y*(18*f[0] - 45.0/2.0*f[1] + (9.0/2.0)*f[2] - 45.0/2.0*f[4] + 27*f[5] - 9.0/2.0*f[6] + (9.0/2.0)*f[7] - 9.0/2.0*f[8] + y*(-27.0/2.0*f[0] + (27.0/2.0)*f[1] + 27*f[4] - 27*f[5] - 27.0/2.0*f[7] + (27.0/2.0)*f[8]) + z*(-27*f[0] + 27*f[10] - 27*f[11] - 27*f[13] + 27*f[14] + 27*f[1] + 27*f[4] - 27*f[5])) + z*(18*f[0] - 45.0/2.0*f[10] + 27*f[11] - 9.0/2.0*f[12] + (9.0/2.0)*f[16] - 9.0/2.0*f[17] - 45.0/2.0*f[1] + (9.0/2.0)*f[2] + z*(-27.0/2.0*f[0] + 27*f[10] - 27*f[11] - 27.0/2.0*f[16] + (27.0/2.0)*f[17] + (27.0/2.0)*f[1]))) + y*(-11.0/2.0*f[0] + 9*f[4] - 9.0/2.0*f[7] + f[9] + y*(9*f[0] - 45.0/2.0*f[4] + 18*f[7] - 9.0/2.0*f[9] + y*(-9.0/2.0*f[0] + (27.0/2.0)*f[4] - 27.0/2.0*f[7] + (9.0/2.0)*f[9]) + z*(-27.0/2.0*f[0] + (27.0/2.0)*f[10] - 27*f[13] + (27.0/2.0)*f[15] + 27*f[4] - 27.0/2.0*f[7])) + z*(18*f[0] - 45.0/2.0*f[10] + 27*f[13] - 9.0/2.0*f[15] + (9.0/2.0)*f[16] - 9.0/2.0*f[18] - 45.0/2.0*f[4] + (9.0/2.0)*f[7] + z*(-27.0/2.0*f[0] + 27*f[10] - 27*f[13] - 27.0/2.0*f[16] + (27.0/2.0)*f[18] + (27.0/2.0)*f[4]))) + z*(-11.0/2.0*f[0] + 9*f[10] - 9.0/2.0*f[16] + f[19] + z*(9*f[0] - 45.0/2.0*f[10] + 18*f[16] - 9.0/2.0*f[19] + z*(-9.0/2.0*f[0] + (27.0/2.0)*f[10] - 27.0/2.0*f[16] + (9.0/2.0)*f[19])));
}
#endif
#endif

#ifdef ET_QUAD
#if ORDER==1
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
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
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3])) + y*(-f[0] + f[2]);
}
#endif
#endif

#ifdef ET_QUAD
#if ORDER==1
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
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
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3])) + y*(-f[0] + f[2]);
}
#endif
#endif

#ifdef ET_QUAD
#if ORDER==2
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2] + y*(-6*f[0] + 12*f[1] - 6*f[2] + 8*f[3] - 16*f[4] + 8*f[5] - 2*f[6] + 4*f[7] - 2*f[8] + y*(4*f[0] - 8*f[1] + 4*f[2] - 8*f[3] + 16*f[4] - 8*f[5] + 4*f[6] - 8*f[7] + 4*f[8]))) + y*(9*f[0] - 12*f[1] + 3*f[2] - 12*f[3] + 16*f[4] - 4*f[5] + 3*f[6] - 4*f[7] + f[8] + y*(-6*f[0] + 8*f[1] - 2*f[2] + 12*f[3] - 16*f[4] + 4*f[5] - 6*f[6] + 8*f[7] - 2*f[8]))) + y*(-3*f[0] + 4*f[3] - f[6] + y*(2*f[0] - 4*f[3] + 2*f[6]));
}
#endif
#endif

#ifdef ET_QUAD
#if ORDER==2
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2] + y*(-6*f[0] + 12*f[1] - 6*f[2] + 8*f[3] - 16*f[4] + 8*f[5] - 2*f[6] + 4*f[7] - 2*f[8] + y*(4*f[0] - 8*f[1] + 4*f[2] - 8*f[3] + 16*f[4] - 8*f[5] + 4*f[6] - 8*f[7] + 4*f[8]))) + y*(9*f[0] - 12*f[1] + 3*f[2] - 12*f[3] + 16*f[4] - 4*f[5] + 3*f[6] - 4*f[7] + f[8] + y*(-6*f[0] + 8*f[1] - 2*f[2] + 12*f[3] - 16*f[4] + 4*f[5] - 6*f[6] + 8*f[7] - 2*f[8]))) + y*(-3*f[0] + 4*f[3] - f[6] + y*(2*f[0] - 4*f[3] + 2*f[6]));
}
#endif
#endif

#ifdef ET_QUAD
#if ORDER==3
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
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
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3] + y*((99.0/4.0)*f[0] + (243.0/4.0)*f[10] - 81.0/4.0*f[11] - 9.0/2.0*f[12] + (27.0/2.0)*f[13] - 27.0/2.0*f[14] + (9.0/2.0)*f[15] - 297.0/4.0*f[1] + (297.0/4.0)*f[2] - 99.0/4.0*f[3] - 81.0/2.0*f[4] + (243.0/2.0)*f[5] - 243.0/2.0*f[6] + (81.0/2.0)*f[7] + (81.0/4.0)*f[8] - 243.0/4.0*f[9] + y*(-81.0/2.0*f[0] - 243*f[10] + 81*f[11] + (81.0/4.0)*f[12] - 243.0/4.0*f[13] + (243.0/4.0)*f[14] - 81.0/4.0*f[15] + (243.0/2.0)*f[1] - 243.0/2.0*f[2] + (81.0/2.0)*f[3] + (405.0/4.0)*f[4] - 1215.0/4.0*f[5] + (1215.0/4.0)*f[6] - 405.0/4.0*f[7] - 81*f[8] + 243*f[9] + y*((81.0/4.0)*f[0] + (729.0/4.0)*f[10] - 243.0/4.0*f[11] - 81.0/4.0*f[12] + (243.0/4.0)*f[13] - 243.0/4.0*f[14] + (81.0/4.0)*f[15] - 243.0/4.0*f[1] + (243.0/4.0)*f[2] - 81.0/4.0*f[3] - 243.0/4.0*f[4] + (729.0/4.0)*f[5] - 729.0/4.0*f[6] + (243.0/4.0)*f[7] + (243.0/4.0)*f[8] - 729.0/4.0*f[9])))) + y*(-99.0/2.0*f[0] - 81*f[10] + (81.0/4.0)*f[11] + 9*f[12] - 45.0/2.0*f[13] + 18*f[14] - 9.0/2.0*f[15] + (495.0/4.0)*f[1] - 99*f[2] + (99.0/4.0)*f[3] + 81*f[4] - 405.0/2.0*f[5] + 162*f[6] - 81.0/2.0*f[7] - 81.0/2.0*f[8] + (405.0/4.0)*f[9] + y*(81*f[0] + 324*f[10] - 81*f[11] - 81.0/2.0*f[12] + (405.0/4.0)*f[13] - 81*f[14] + (81.0/4.0)*f[15] - 405.0/2.0*f[1] + 162*f[2] - 81.0/2.0*f[3] - 405.0/2.0*f[4] + (2025.0/4.0)*f[5] - 405*f[6] + (405.0/4.0)*f[7] + 162*f[8] - 405*f[9] + y*(-81.0/2.0*f[0] - 243*f[10] + (243.0/4.0)*f[11] + (81.0/2.0)*f[12] - 405.0/4.0*f[13] + 81*f[14] - 81.0/4.0*f[15] + (405.0/4.0)*f[1] - 81*f[2] + (81.0/4.0)*f[3] + (243.0/2.0)*f[4] - 1215.0/4.0*f[5] + 243*f[6] - 243.0/4.0*f[7] - 243.0/2.0*f[8] + (1215.0/4.0)*f[9])))) + y*((121.0/4.0)*f[0] + (81.0/4.0)*f[10] - 9.0/2.0*f[11] - 11.0/2.0*f[12] + 9*f[13] - 9.0/2.0*f[14] + f[15] - 99.0/2.0*f[1] + (99.0/4.0)*f[2] - 11.0/2.0*f[3] - 99.0/2.0*f[4] + 81*f[5] - 81.0/2.0*f[6] + 9*f[7] + (99.0/4.0)*f[8] - 81.0/2.0*f[9] + y*(-99.0/2.0*f[0] - 81*f[10] + 18*f[11] + (99.0/4.0)*f[12] - 81.0/2.0*f[13] + (81.0/4.0)*f[14] - 9.0/2.0*f[15] + 81*f[1] - 81.0/2.0*f[2] + 9*f[3] + (495.0/4.0)*f[4] - 405.0/2.0*f[5] + (405.0/4.0)*f[6] - 45.0/2.0*f[7] - 99*f[8] + 162*f[9] + y*((99.0/4.0)*f[0] + (243.0/4.0)*f[10] - 27.0/2.0*f[11] - 99.0/4.0*f[12] + (81.0/2.0)*f[13] - 81.0/4.0*f[14] + (9.0/2.0)*f[15] - 81.0/2.0*f[1] + (81.0/4.0)*f[2] - 9.0/2.0*f[3] - 297.0/4.0*f[4] + (243.0/2.0)*f[5] - 243.0/4.0*f[6] + (27.0/2.0)*f[7] + (297.0/4.0)*f[8] - 243.0/2.0*f[9])))) + y*(-11.0/2.0*f[0] + f[12] + 9*f[4] - 9.0/2.0*f[8] + y*(9*f[0] - 9.0/2.0*f[12] - 45.0/2.0*f[4] + 18*f[8] + y*(-9.0/2.0*f[0] + (9.0/2.0)*f[12] + (27.0/2.0)*f[4] - 27.0/2.0*f[8])));
}
#endif
#endif

#ifdef ET_QUAD
#if ORDER==3
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int component) {
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
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3] + y*((99.0/4.0)*f[0] + (243.0/4.0)*f[10] - 81.0/4.0*f[11] - 9.0/2.0*f[12] + (27.0/2.0)*f[13] - 27.0/2.0*f[14] + (9.0/2.0)*f[15] - 297.0/4.0*f[1] + (297.0/4.0)*f[2] - 99.0/4.0*f[3] - 81.0/2.0*f[4] + (243.0/2.0)*f[5] - 243.0/2.0*f[6] + (81.0/2.0)*f[7] + (81.0/4.0)*f[8] - 243.0/4.0*f[9] + y*(-81.0/2.0*f[0] - 243*f[10] + 81*f[11] + (81.0/4.0)*f[12] - 243.0/4.0*f[13] + (243.0/4.0)*f[14] - 81.0/4.0*f[15] + (243.0/2.0)*f[1] - 243.0/2.0*f[2] + (81.0/2.0)*f[3] + (405.0/4.0)*f[4] - 1215.0/4.0*f[5] + (1215.0/4.0)*f[6] - 405.0/4.0*f[7] - 81*f[8] + 243*f[9] + y*((81.0/4.0)*f[0] + (729.0/4.0)*f[10] - 243.0/4.0*f[11] - 81.0/4.0*f[12] + (243.0/4.0)*f[13] - 243.0/4.0*f[14] + (81.0/4.0)*f[15] - 243.0/4.0*f[1] + (243.0/4.0)*f[2] - 81.0/4.0*f[3] - 243.0/4.0*f[4] + (729.0/4.0)*f[5] - 729.0/4.0*f[6] + (243.0/4.0)*f[7] + (243.0/4.0)*f[8] - 729.0/4.0*f[9])))) + y*(-99.0/2.0*f[0] - 81*f[10] + (81.0/4.0)*f[11] + 9*f[12] - 45.0/2.0*f[13] + 18*f[14] - 9.0/2.0*f[15] + (495.0/4.0)*f[1] - 99*f[2] + (99.0/4.0)*f[3] + 81*f[4] - 405.0/2.0*f[5] + 162*f[6] - 81.0/2.0*f[7] - 81.0/2.0*f[8] + (405.0/4.0)*f[9] + y*(81*f[0] + 324*f[10] - 81*f[11] - 81.0/2.0*f[12] + (405.0/4.0)*f[13] - 81*f[14] + (81.0/4.0)*f[15] - 405.0/2.0*f[1] + 162*f[2] - 81.0/2.0*f[3] - 405.0/2.0*f[4] + (2025.0/4.0)*f[5] - 405*f[6] + (405.0/4.0)*f[7] + 162*f[8] - 405*f[9] + y*(-81.0/2.0*f[0] - 243*f[10] + (243.0/4.0)*f[11] + (81.0/2.0)*f[12] - 405.0/4.0*f[13] + 81*f[14] - 81.0/4.0*f[15] + (405.0/4.0)*f[1] - 81*f[2] + (81.0/4.0)*f[3] + (243.0/2.0)*f[4] - 1215.0/4.0*f[5] + 243*f[6] - 243.0/4.0*f[7] - 243.0/2.0*f[8] + (1215.0/4.0)*f[9])))) + y*((121.0/4.0)*f[0] + (81.0/4.0)*f[10] - 9.0/2.0*f[11] - 11.0/2.0*f[12] + 9*f[13] - 9.0/2.0*f[14] + f[15] - 99.0/2.0*f[1] + (99.0/4.0)*f[2] - 11.0/2.0*f[3] - 99.0/2.0*f[4] + 81*f[5] - 81.0/2.0*f[6] + 9*f[7] + (99.0/4.0)*f[8] - 81.0/2.0*f[9] + y*(-99.0/2.0*f[0] - 81*f[10] + 18*f[11] + (99.0/4.0)*f[12] - 81.0/2.0*f[13] + (81.0/4.0)*f[14] - 9.0/2.0*f[15] + 81*f[1] - 81.0/2.0*f[2] + 9*f[3] + (495.0/4.0)*f[4] - 405.0/2.0*f[5] + (405.0/4.0)*f[6] - 45.0/2.0*f[7] - 99*f[8] + 162*f[9] + y*((99.0/4.0)*f[0] + (243.0/4.0)*f[10] - 27.0/2.0*f[11] - 99.0/4.0*f[12] + (81.0/2.0)*f[13] - 81.0/4.0*f[14] + (9.0/2.0)*f[15] - 81.0/2.0*f[1] + (81.0/4.0)*f[2] - 9.0/2.0*f[3] - 297.0/4.0*f[4] + (243.0/2.0)*f[5] - 243.0/4.0*f[6] + (27.0/2.0)*f[7] + (297.0/4.0)*f[8] - 243.0/2.0*f[9])))) + y*(-11.0/2.0*f[0] + f[12] + 9*f[4] - 9.0/2.0*f[8] + y*(9*f[0] - 9.0/2.0*f[12] - 45.0/2.0*f[4] + 18*f[8] + y*(-9.0/2.0*f[0] + (9.0/2.0)*f[12] + (27.0/2.0)*f[4] - 27.0/2.0*f[8])));
}
#endif
#endif

#ifdef ET_HEX
#if ORDER==1
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
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
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3] + z*(-f[0] + f[1] + f[2] - f[3] + f[4] - f[5] - f[6] + f[7])) + z*(f[0] - f[1] - f[4] + f[5])) + y*(-f[0] + f[2] + z*(f[0] - f[2] - f[4] + f[6])) + z*(-f[0] + f[4]);
}
#endif
#endif

#ifdef ET_HEX
#if ORDER==1
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
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
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3] + z*(-f[0] + f[1] + f[2] - f[3] + f[4] - f[5] - f[6] + f[7])) + z*(f[0] - f[1] - f[4] + f[5])) + y*(-f[0] + f[2] + z*(f[0] - f[2] - f[4] + f[6])) + z*(-f[0] + f[4]);
}
#endif
#endif

#ifdef ET_HEX
#if ORDER==2
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2] + y*(-6*f[0] + 12*f[1] - 6*f[2] + 8*f[3] - 16*f[4] + 8*f[5] - 2*f[6] + 4*f[7] - 2*f[8] + y*(4*f[0] - 8*f[1] + 4*f[2] - 8*f[3] + 16*f[4] - 8*f[5] + 4*f[6] - 8*f[7] + 4*f[8] + z*(-12*f[0] - 32*f[10] + 16*f[11] - 32*f[12] + 64*f[13] - 32*f[14] + 16*f[15] - 32*f[16] + 16*f[17] - 4*f[18] + 8*f[19] + 24*f[1] - 4*f[20] + 8*f[21] - 16*f[22] + 8*f[23] - 4*f[24] + 8*f[25] - 4*f[26] - 12*f[2] + 24*f[3] - 48*f[4] + 24*f[5] - 12*f[6] + 24*f[7] - 12*f[8] + 16*f[9] + z*(8*f[0] + 32*f[10] - 16*f[11] + 32*f[12] - 64*f[13] + 32*f[14] - 16*f[15] + 32*f[16] - 16*f[17] + 8*f[18] - 16*f[19] - 16*f[1] + 8*f[20] - 16*f[21] + 32*f[22] - 16*f[23] + 8*f[24] - 16*f[25] + 8*f[26] + 8*f[2] - 16*f[3] + 32*f[4] - 16*f[5] + 8*f[6] - 16*f[7] + 8*f[8] - 16*f[9]))) + z*(18*f[0] + 48*f[10] - 24*f[11] + 32*f[12] - 64*f[13] + 32*f[14] - 8*f[15] + 16*f[16] - 8*f[17] + 6*f[18] - 12*f[19] - 36*f[1] + 6*f[20] - 8*f[21] + 16*f[22] - 8*f[23] + 2*f[24] - 4*f[25] + 2*f[26] + 18*f[2] - 24*f[3] + 48*f[4] - 24*f[5] + 6*f[6] - 12*f[7] + 6*f[8] - 24*f[9] + z*(-12*f[0] - 48*f[10] + 24*f[11] - 32*f[12] + 64*f[13] - 32*f[14] + 8*f[15] - 16*f[16] + 8*f[17] - 12*f[18] + 24*f[19] + 24*f[1] - 12*f[20] + 16*f[21] - 32*f[22] + 16*f[23] - 4*f[24] + 8*f[25] - 4*f[26] - 12*f[2] + 16*f[3] - 32*f[4] + 16*f[5] - 4*f[6] + 8*f[7] - 4*f[8] + 24*f[9]))) + z*(-6*f[0] - 16*f[10] + 8*f[11] - 2*f[18] + 4*f[19] + 12*f[1] - 2*f[20] - 6*f[2] + 8*f[9] + z*(4*f[0] + 16*f[10] - 8*f[11] + 4*f[18] - 8*f[19] - 8*f[1] + 4*f[20] + 4*f[2] - 8*f[9]))) + y*(9*f[0] - 12*f[1] + 3*f[2] - 12*f[3] + 16*f[4] - 4*f[5] + 3*f[6] - 4*f[7] + f[8] + y*(-6*f[0] + 8*f[1] - 2*f[2] + 12*f[3] - 16*f[4] + 4*f[5] - 6*f[6] + 8*f[7] - 2*f[8] + z*(18*f[0] + 32*f[10] - 8*f[11] + 48*f[12] - 64*f[13] + 16*f[14] - 24*f[15] + 32*f[16] - 8*f[17] + 6*f[18] - 8*f[19] - 24*f[1] + 2*f[20] - 12*f[21] + 16*f[22] - 4*f[23] + 6*f[24] - 8*f[25] + 2*f[26] + 6*f[2] - 36*f[3] + 48*f[4] - 12*f[5] + 18*f[6] - 24*f[7] + 6*f[8] - 24*f[9] + z*(-12*f[0] - 32*f[10] + 8*f[11] - 48*f[12] + 64*f[13] - 16*f[14] + 24*f[15] - 32*f[16] + 8*f[17] - 12*f[18] + 16*f[19] + 16*f[1] - 4*f[20] + 24*f[21] - 32*f[22] + 8*f[23] - 12*f[24] + 16*f[25] - 4*f[26] - 4*f[2] + 24*f[3] - 32*f[4] + 8*f[5] - 12*f[6] + 16*f[7] - 4*f[8] + 24*f[9]))) + z*(-27*f[0] - 48*f[10] + 12*f[11] - 48*f[12] + 64*f[13] - 16*f[14] + 12*f[15] - 16*f[16] + 4*f[17] - 9*f[18] + 12*f[19] + 36*f[1] - 3*f[20] + 12*f[21] - 16*f[22] + 4*f[23] - 3*f[24] + 4*f[25] - f[26] - 9*f[2] + 36*f[3] - 48*f[4] + 12*f[5] - 9*f[6] + 12*f[7] - 3*f[8] + 36*f[9] + z*(18*f[0] + 48*f[10] - 12*f[11] + 48*f[12] - 64*f[13] + 16*f[14] - 12*f[15] + 16*f[16] - 4*f[17] + 18*f[18] - 24*f[19] - 24*f[1] + 6*f[20] - 24*f[21] + 32*f[22] - 8*f[23] + 6*f[24] - 8*f[25] + 2*f[26] + 6*f[2] - 24*f[3] + 32*f[4] - 8*f[5] + 6*f[6] - 8*f[7] + 2*f[8] - 36*f[9]))) + z*(9*f[0] + 16*f[10] - 4*f[11] + 3*f[18] - 4*f[19] - 12*f[1] + f[20] + 3*f[2] - 12*f[9] + z*(-6*f[0] - 16*f[10] + 4*f[11] - 6*f[18] + 8*f[19] + 8*f[1] - 2*f[20] - 2*f[2] + 12*f[9]))) + y*(-3*f[0] + 4*f[3] - f[6] + y*(2*f[0] - 4*f[3] + 2*f[6] + z*(-6*f[0] - 16*f[12] + 8*f[15] - 2*f[18] + 4*f[21] - 2*f[24] + 12*f[3] - 6*f[6] + 8*f[9] + z*(4*f[0] + 16*f[12] - 8*f[15] + 4*f[18] - 8*f[21] + 4*f[24] - 8*f[3] + 4*f[6] - 8*f[9]))) + z*(9*f[0] + 16*f[12] - 4*f[15] + 3*f[18] - 4*f[21] + f[24] - 12*f[3] + 3*f[6] - 12*f[9] + z*(-6*f[0] - 16*f[12] + 4*f[15] - 6*f[18] + 8*f[21] - 2*f[24] + 8*f[3] - 2*f[6] + 12*f[9]))) + z*(-3*f[0] - f[18] + 4*f[9] + z*(2*f[0] + 2*f[18] - 4*f[9]));
}
#endif
#endif

#ifdef ET_HEX
#if ORDER==2
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2] + y*(-6*f[0] + 12*f[1] - 6*f[2] + 8*f[3] - 16*f[4] + 8*f[5] - 2*f[6] + 4*f[7] - 2*f[8] + y*(4*f[0] - 8*f[1] + 4*f[2] - 8*f[3] + 16*f[4] - 8*f[5] + 4*f[6] - 8*f[7] + 4*f[8] + z*(-12*f[0] - 32*f[10] + 16*f[11] - 32*f[12] + 64*f[13] - 32*f[14] + 16*f[15] - 32*f[16] + 16*f[17] - 4*f[18] + 8*f[19] + 24*f[1] - 4*f[20] + 8*f[21] - 16*f[22] + 8*f[23] - 4*f[24] + 8*f[25] - 4*f[26] - 12*f[2] + 24*f[3] - 48*f[4] + 24*f[5] - 12*f[6] + 24*f[7] - 12*f[8] + 16*f[9] + z*(8*f[0] + 32*f[10] - 16*f[11] + 32*f[12] - 64*f[13] + 32*f[14] - 16*f[15] + 32*f[16] - 16*f[17] + 8*f[18] - 16*f[19] - 16*f[1] + 8*f[20] - 16*f[21] + 32*f[22] - 16*f[23] + 8*f[24] - 16*f[25] + 8*f[26] + 8*f[2] - 16*f[3] + 32*f[4] - 16*f[5] + 8*f[6] - 16*f[7] + 8*f[8] - 16*f[9]))) + z*(18*f[0] + 48*f[10] - 24*f[11] + 32*f[12] - 64*f[13] + 32*f[14] - 8*f[15] + 16*f[16] - 8*f[17] + 6*f[18] - 12*f[19] - 36*f[1] + 6*f[20] - 8*f[21] + 16*f[22] - 8*f[23] + 2*f[24] - 4*f[25] + 2*f[26] + 18*f[2] - 24*f[3] + 48*f[4] - 24*f[5] + 6*f[6] - 12*f[7] + 6*f[8] - 24*f[9] + z*(-12*f[0] - 48*f[10] + 24*f[11] - 32*f[12] + 64*f[13] - 32*f[14] + 8*f[15] - 16*f[16] + 8*f[17] - 12*f[18] + 24*f[19] + 24*f[1] - 12*f[20] + 16*f[21] - 32*f[22] + 16*f[23] - 4*f[24] + 8*f[25] - 4*f[26] - 12*f[2] + 16*f[3] - 32*f[4] + 16*f[5] - 4*f[6] + 8*f[7] - 4*f[8] + 24*f[9]))) + z*(-6*f[0] - 16*f[10] + 8*f[11] - 2*f[18] + 4*f[19] + 12*f[1] - 2*f[20] - 6*f[2] + 8*f[9] + z*(4*f[0] + 16*f[10] - 8*f[11] + 4*f[18] - 8*f[19] - 8*f[1] + 4*f[20] + 4*f[2] - 8*f[9]))) + y*(9*f[0] - 12*f[1] + 3*f[2] - 12*f[3] + 16*f[4] - 4*f[5] + 3*f[6] - 4*f[7] + f[8] + y*(-6*f[0] + 8*f[1] - 2*f[2] + 12*f[3] - 16*f[4] + 4*f[5] - 6*f[6] + 8*f[7] - 2*f[8] + z*(18*f[0] + 32*f[10] - 8*f[11] + 48*f[12] - 64*f[13] + 16*f[14] - 24*f[15] + 32*f[16] - 8*f[17] + 6*f[18] - 8*f[19] - 24*f[1] + 2*f[20] - 12*f[21] + 16*f[22] - 4*f[23] + 6*f[24] - 8*f[25] + 2*f[26] + 6*f[2] - 36*f[3] + 48*f[4] - 12*f[5] + 18*f[6] - 24*f[7] + 6*f[8] - 24*f[9] + z*(-12*f[0] - 32*f[10] + 8*f[11] - 48*f[12] + 64*f[13] - 16*f[14] + 24*f[15] - 32*f[16] + 8*f[17] - 12*f[18] + 16*f[19] + 16*f[1] - 4*f[20] + 24*f[21] - 32*f[22] + 8*f[23] - 12*f[24] + 16*f[25] - 4*f[26] - 4*f[2] + 24*f[3] - 32*f[4] + 8*f[5] - 12*f[6] + 16*f[7] - 4*f[8] + 24*f[9]))) + z*(-27*f[0] - 48*f[10] + 12*f[11] - 48*f[12] + 64*f[13] - 16*f[14] + 12*f[15] - 16*f[16] + 4*f[17] - 9*f[18] + 12*f[19] + 36*f[1] - 3*f[20] + 12*f[21] - 16*f[22] + 4*f[23] - 3*f[24] + 4*f[25] - f[26] - 9*f[2] + 36*f[3] - 48*f[4] + 12*f[5] - 9*f[6] + 12*f[7] - 3*f[8] + 36*f[9] + z*(18*f[0] + 48*f[10] - 12*f[11] + 48*f[12] - 64*f[13] + 16*f[14] - 12*f[15] + 16*f[16] - 4*f[17] + 18*f[18] - 24*f[19] - 24*f[1] + 6*f[20] - 24*f[21] + 32*f[22] - 8*f[23] + 6*f[24] - 8*f[25] + 2*f[26] + 6*f[2] - 24*f[3] + 32*f[4] - 8*f[5] + 6*f[6] - 8*f[7] + 2*f[8] - 36*f[9]))) + z*(9*f[0] + 16*f[10] - 4*f[11] + 3*f[18] - 4*f[19] - 12*f[1] + f[20] + 3*f[2] - 12*f[9] + z*(-6*f[0] - 16*f[10] + 4*f[11] - 6*f[18] + 8*f[19] + 8*f[1] - 2*f[20] - 2*f[2] + 12*f[9]))) + y*(-3*f[0] + 4*f[3] - f[6] + y*(2*f[0] - 4*f[3] + 2*f[6] + z*(-6*f[0] - 16*f[12] + 8*f[15] - 2*f[18] + 4*f[21] - 2*f[24] + 12*f[3] - 6*f[6] + 8*f[9] + z*(4*f[0] + 16*f[12] - 8*f[15] + 4*f[18] - 8*f[21] + 4*f[24] - 8*f[3] + 4*f[6] - 8*f[9]))) + z*(9*f[0] + 16*f[12] - 4*f[15] + 3*f[18] - 4*f[21] + f[24] - 12*f[3] + 3*f[6] - 12*f[9] + z*(-6*f[0] - 16*f[12] + 4*f[15] - 6*f[18] + 8*f[21] - 2*f[24] + 8*f[3] - 2*f[6] + 12*f[9]))) + z*(-3*f[0] - f[18] + 4*f[9] + z*(2*f[0] + 2*f[18] - 4*f[9]));
}
#endif
#endif

#ifdef ET_HEX
#if ORDER==3
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[64];
  int ii=0;
  for (int i=0; i<=3; i++) {
    for (int j=0; j<=3; j++) {
      for (int k=0; k<=3; k++) {
        f[ii] = getValue(coefficients, first+k+j*dy+i*dz)[component];
        ii++;
      }
    }
  }
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3] + y*((99.0/4.0)*f[0] + (243.0/4.0)*f[10] - 81.0/4.0*f[11] - 9.0/2.0*f[12] + (27.0/2.0)*f[13] - 27.0/2.0*f[14] + (9.0/2.0)*f[15] - 297.0/4.0*f[1] + (297.0/4.0)*f[2] - 99.0/4.0*f[3] - 81.0/2.0*f[4] + (243.0/2.0)*f[5] - 243.0/2.0*f[6] + (81.0/2.0)*f[7] + (81.0/4.0)*f[8] - 243.0/4.0*f[9] + y*(-81.0/2.0*f[0] - 243*f[10] + 81*f[11] + (81.0/4.0)*f[12] - 243.0/4.0*f[13] + (243.0/4.0)*f[14] - 81.0/4.0*f[15] + (243.0/2.0)*f[1] - 243.0/2.0*f[2] + (81.0/2.0)*f[3] + (405.0/4.0)*f[4] - 1215.0/4.0*f[5] + (1215.0/4.0)*f[6] - 405.0/4.0*f[7] - 81*f[8] + 243*f[9] + y*((81.0/4.0)*f[0] + (729.0/4.0)*f[10] - 243.0/4.0*f[11] - 81.0/4.0*f[12] + (243.0/4.0)*f[13] - 243.0/4.0*f[14] + (81.0/4.0)*f[15] - 243.0/4.0*f[1] + (243.0/4.0)*f[2] - 81.0/4.0*f[3] - 243.0/4.0*f[4] + (729.0/4.0)*f[5] - 729.0/4.0*f[6] + (243.0/4.0)*f[7] + (243.0/4.0)*f[8] - 729.0/4.0*f[9] + z*(-891.0/8.0*f[0] - 8019.0/8.0*f[10] + (2673.0/8.0)*f[11] + (891.0/8.0)*f[12] - 2673.0/8.0*f[13] + (2673.0/8.0)*f[14] - 891.0/8.0*f[15] + (729.0/4.0)*f[16] - 2187.0/4.0*f[17] + (2187.0/4.0)*f[18] - 729.0/4.0*f[19] + (2673.0/8.0)*f[1] - 2187.0/4.0*f[20] + (6561.0/4.0)*f[21] - 6561.0/4.0*f[22] + (2187.0/4.0)*f[23] + (2187.0/4.0)*f[24] - 6561.0/4.0*f[25] + (6561.0/4.0)*f[26] - 2187.0/4.0*f[27] - 729.0/4.0*f[28] + (2187.0/4.0)*f[29] - 2673.0/8.0*f[2] - 2187.0/4.0*f[30] + (729.0/4.0)*f[31] - 729.0/8.0*f[32] + (2187.0/8.0)*f[33] - 2187.0/8.0*f[34] + (729.0/8.0)*f[35] + (2187.0/8.0)*f[36] - 6561.0/8.0*f[37] + (6561.0/8.0)*f[38] - 2187.0/8.0*f[39] + (891.0/8.0)*f[3] - 2187.0/8.0*f[40] + (6561.0/8.0)*f[41] - 6561.0/8.0*f[42] + (2187.0/8.0)*f[43] + (729.0/8.0)*f[44] - 2187.0/8.0*f[45] + (2187.0/8.0)*f[46] - 729.0/8.0*f[47] + (81.0/4.0)*f[48] - 243.0/4.0*f[49] + (2673.0/8.0)*f[4] + (243.0/4.0)*f[50] - 81.0/4.0*f[51] - 243.0/4.0*f[52] + (729.0/4.0)*f[53] - 729.0/4.0*f[54] + (243.0/4.0)*f[55] + (243.0/4.0)*f[56] - 729.0/4.0*f[57] + (729.0/4.0)*f[58] - 243.0/4.0*f[59] - 8019.0/8.0*f[5] - 81.0/4.0*f[60] + (243.0/4.0)*f[61] - 243.0/4.0*f[62] + (81.0/4.0)*f[63] + (8019.0/8.0)*f[6] - 2673.0/8.0*f[7] - 2673.0/8.0*f[8] + (8019.0/8.0)*f[9] + z*((729.0/4.0)*f[0] + (6561.0/4.0)*f[10] - 2187.0/4.0*f[11] - 729.0/4.0*f[12] + (2187.0/4.0)*f[13] - 2187.0/4.0*f[14] + (729.0/4.0)*f[15] - 3645.0/8.0*f[16] + (10935.0/8.0)*f[17] - 10935.0/8.0*f[18] + (3645.0/8.0)*f[19] - 2187.0/4.0*f[1] + (10935.0/8.0)*f[20] - 32805.0/8.0*f[21] + (32805.0/8.0)*f[22] - 10935.0/8.0*f[23] - 10935.0/8.0*f[24] + (32805.0/8.0)*f[25] - 32805.0/8.0*f[26] + (10935.0/8.0)*f[27] + (3645.0/8.0)*f[28] - 10935.0/8.0*f[29] + (2187.0/4.0)*f[2] + (10935.0/8.0)*f[30] - 3645.0/8.0*f[31] + (729.0/2.0)*f[32] - 2187.0/2.0*f[33] + (2187.0/2.0)*f[34] - 729.0/2.0*f[35] - 2187.0/2.0*f[36] + (6561.0/2.0)*f[37] - 6561.0/2.0*f[38] + (2187.0/2.0)*f[39] - 729.0/4.0*f[3] + (2187.0/2.0)*f[40] - 6561.0/2.0*f[41] + (6561.0/2.0)*f[42] - 2187.0/2.0*f[43] - 729.0/2.0*f[44] + (2187.0/2.0)*f[45] - 2187.0/2.0*f[46] + (729.0/2.0)*f[47] - 729.0/8.0*f[48] + (2187.0/8.0)*f[49] - 2187.0/4.0*f[4] - 2187.0/8.0*f[50] + (729.0/8.0)*f[51] + (2187.0/8.0)*f[52] - 6561.0/8.0*f[53] + (6561.0/8.0)*f[54] - 2187.0/8.0*f[55] - 2187.0/8.0*f[56] + (6561.0/8.0)*f[57] - 6561.0/8.0*f[58] + (2187.0/8.0)*f[59] + (6561.0/4.0)*f[5] + (729.0/8.0)*f[60] - 2187.0/8.0*f[61] + (2187.0/8.0)*f[62] - 729.0/8.0*f[63] - 6561.0/4.0*f[6] + (2187.0/4.0)*f[7] + (2187.0/4.0)*f[8] - 6561.0/4.0*f[9] + z*(-729.0/8.0*f[0] - 6561.0/8.0*f[10] + (2187.0/8.0)*f[11] + (729.0/8.0)*f[12] - 2187.0/8.0*f[13] + (2187.0/8.0)*f[14] - 729.0/8.0*f[15] + (2187.0/8.0)*f[16] - 6561.0/8.0*f[17] + (6561.0/8.0)*f[18] - 2187.0/8.0*f[19] + (2187.0/8.0)*f[1] - 6561.0/8.0*f[20] + (19683.0/8.0)*f[21] - 19683.0/8.0*f[22] + (6561.0/8.0)*f[23] + (6561.0/8.0)*f[24] - 19683.0/8.0*f[25] + (19683.0/8.0)*f[26] - 6561.0/8.0*f[27] - 2187.0/8.0*f[28] + (6561.0/8.0)*f[29] - 2187.0/8.0*f[2] - 6561.0/8.0*f[30] + (2187.0/8.0)*f[31] - 2187.0/8.0*f[32] + (6561.0/8.0)*f[33] - 6561.0/8.0*f[34] + (2187.0/8.0)*f[35] + (6561.0/8.0)*f[36] - 19683.0/8.0*f[37] + (19683.0/8.0)*f[38] - 6561.0/8.0*f[39] + (729.0/8.0)*f[3] - 6561.0/8.0*f[40] + (19683.0/8.0)*f[41] - 19683.0/8.0*f[42] + (6561.0/8.0)*f[43] + (2187.0/8.0)*f[44] - 6561.0/8.0*f[45] + (6561.0/8.0)*f[46] - 2187.0/8.0*f[47] + (729.0/8.0)*f[48] - 2187.0/8.0*f[49] + (2187.0/8.0)*f[4] + (2187.0/8.0)*f[50] - 729.0/8.0*f[51] - 2187.0/8.0*f[52] + (6561.0/8.0)*f[53] - 6561.0/8.0*f[54] + (2187.0/8.0)*f[55] + (2187.0/8.0)*f[56] - 6561.0/8.0*f[57] + (6561.0/8.0)*f[58] - 2187.0/8.0*f[59] - 6561.0/8.0*f[5] - 729.0/8.0*f[60] + (2187.0/8.0)*f[61] - 2187.0/8.0*f[62] + (729.0/8.0)*f[63] + (6561.0/8.0)*f[6] - 2187.0/8.0*f[7] - 2187.0/8.0*f[8] + (6561.0/8.0)*f[9])))) + z*((891.0/4.0)*f[0] + (2673.0/2.0)*f[10] - 891.0/2.0*f[11] - 891.0/8.0*f[12] + (2673.0/8.0)*f[13] - 2673.0/8.0*f[14] + (891.0/8.0)*f[15] - 729.0/2.0*f[16] + (2187.0/2.0)*f[17] - 2187.0/2.0*f[18] + (729.0/2.0)*f[19] - 2673.0/4.0*f[1] + (3645.0/4.0)*f[20] - 10935.0/4.0*f[21] + (10935.0/4.0)*f[22] - 3645.0/4.0*f[23] - 729*f[24] + 2187*f[25] - 2187*f[26] + 729*f[27] + (729.0/4.0)*f[28] - 2187.0/4.0*f[29] + (2673.0/4.0)*f[2] + (2187.0/4.0)*f[30] - 729.0/4.0*f[31] + (729.0/4.0)*f[32] - 2187.0/4.0*f[33] + (2187.0/4.0)*f[34] - 729.0/4.0*f[35] - 3645.0/8.0*f[36] + (10935.0/8.0)*f[37] - 10935.0/8.0*f[38] + (3645.0/8.0)*f[39] - 891.0/4.0*f[3] + (729.0/2.0)*f[40] - 2187.0/2.0*f[41] + (2187.0/2.0)*f[42] - 729.0/2.0*f[43] - 729.0/8.0*f[44] + (2187.0/8.0)*f[45] - 2187.0/8.0*f[46] + (729.0/8.0)*f[47] - 81.0/2.0*f[48] + (243.0/2.0)*f[49] - 4455.0/8.0*f[4] - 243.0/2.0*f[50] + (81.0/2.0)*f[51] + (405.0/4.0)*f[52] - 1215.0/4.0*f[53] + (1215.0/4.0)*f[54] - 405.0/4.0*f[55] - 81*f[56] + 243*f[57] - 243*f[58] + 81*f[59] + (13365.0/8.0)*f[5] + (81.0/4.0)*f[60] - 243.0/4.0*f[61] + (243.0/4.0)*f[62] - 81.0/4.0*f[63] - 13365.0/8.0*f[6] + (4455.0/8.0)*f[7] + (891.0/2.0)*f[8] - 2673.0/2.0*f[9] + z*(-729.0/2.0*f[0] - 2187*f[10] + 729*f[11] + (729.0/4.0)*f[12] - 2187.0/4.0*f[13] + (2187.0/4.0)*f[14] - 729.0/4.0*f[15] + (3645.0/4.0)*f[16] - 10935.0/4.0*f[17] + (10935.0/4.0)*f[18] - 3645.0/4.0*f[19] + (2187.0/2.0)*f[1] - 18225.0/8.0*f[20] + (54675.0/8.0)*f[21] - 54675.0/8.0*f[22] + (18225.0/8.0)*f[23] + (3645.0/2.0)*f[24] - 10935.0/2.0*f[25] + (10935.0/2.0)*f[26] - 3645.0/2.0*f[27] - 3645.0/8.0*f[28] + (10935.0/8.0)*f[29] - 2187.0/2.0*f[2] - 10935.0/8.0*f[30] + (3645.0/8.0)*f[31] - 729*f[32] + 2187*f[33] - 2187*f[34] + 729*f[35] + (3645.0/2.0)*f[36] - 10935.0/2.0*f[37] + (10935.0/2.0)*f[38] - 3645.0/2.0*f[39] + (729.0/2.0)*f[3] - 1458*f[40] + 4374*f[41] - 4374*f[42] + 1458*f[43] + (729.0/2.0)*f[44] - 2187.0/2.0*f[45] + (2187.0/2.0)*f[46] - 729.0/2.0*f[47] + (729.0/4.0)*f[48] - 2187.0/4.0*f[49] + (3645.0/4.0)*f[4] + (2187.0/4.0)*f[50] - 729.0/4.0*f[51] - 3645.0/8.0*f[52] + (10935.0/8.0)*f[53] - 10935.0/8.0*f[54] + (3645.0/8.0)*f[55] + (729.0/2.0)*f[56] - 2187.0/2.0*f[57] + (2187.0/2.0)*f[58] - 729.0/2.0*f[59] - 10935.0/4.0*f[5] - 729.0/8.0*f[60] + (2187.0/8.0)*f[61] - 2187.0/8.0*f[62] + (729.0/8.0)*f[63] + (10935.0/4.0)*f[6] - 3645.0/4.0*f[7] - 729*f[8] + 2187*f[9] + z*((729.0/4.0)*f[0] + (2187.0/2.0)*f[10] - 729.0/2.0*f[11] - 729.0/8.0*f[12] + (2187.0/8.0)*f[13] - 2187.0/8.0*f[14] + (729.0/8.0)*f[15] - 2187.0/4.0*f[16] + (6561.0/4.0)*f[17] - 6561.0/4.0*f[18] + (2187.0/4.0)*f[19] - 2187.0/4.0*f[1] + (10935.0/8.0)*f[20] - 32805.0/8.0*f[21] + (32805.0/8.0)*f[22] - 10935.0/8.0*f[23] - 2187.0/2.0*f[24] + (6561.0/2.0)*f[25] - 6561.0/2.0*f[26] + (2187.0/2.0)*f[27] + (2187.0/8.0)*f[28] - 6561.0/8.0*f[29] + (2187.0/4.0)*f[2] + (6561.0/8.0)*f[30] - 2187.0/8.0*f[31] + (2187.0/4.0)*f[32] - 6561.0/4.0*f[33] + (6561.0/4.0)*f[34] - 2187.0/4.0*f[35] - 10935.0/8.0*f[36] + (32805.0/8.0)*f[37] - 32805.0/8.0*f[38] + (10935.0/8.0)*f[39] - 729.0/4.0*f[3] + (2187.0/2.0)*f[40] - 6561.0/2.0*f[41] + (6561.0/2.0)*f[42] - 2187.0/2.0*f[43] - 2187.0/8.0*f[44] + (6561.0/8.0)*f[45] - 6561.0/8.0*f[46] + (2187.0/8.0)*f[47] - 729.0/4.0*f[48] + (2187.0/4.0)*f[49] - 3645.0/8.0*f[4] - 2187.0/4.0*f[50] + (729.0/4.0)*f[51] + (3645.0/8.0)*f[52] - 10935.0/8.0*f[53] + (10935.0/8.0)*f[54] - 3645.0/8.0*f[55] - 729.0/2.0*f[56] + (2187.0/2.0)*f[57] - 2187.0/2.0*f[58] + (729.0/2.0)*f[59] + (10935.0/8.0)*f[5] + (729.0/8.0)*f[60] - 2187.0/8.0*f[61] + (2187.0/8.0)*f[62] - 729.0/8.0*f[63] - 10935.0/8.0*f[6] + (3645.0/8.0)*f[7] + (729.0/2.0)*f[8] - 2187.0/2.0*f[9])))) + z*(-1089.0/8.0*f[0] - 2673.0/8.0*f[10] + (891.0/8.0)*f[11] + (99.0/4.0)*f[12] - 297.0/4.0*f[13] + (297.0/4.0)*f[14] - 99.0/4.0*f[15] + (891.0/4.0)*f[16] - 2673.0/4.0*f[17] + (2673.0/4.0)*f[18] - 891.0/4.0*f[19] + (3267.0/8.0)*f[1] - 729.0/2.0*f[20] + (2187.0/2.0)*f[21] - 2187.0/2.0*f[22] + (729.0/2.0)*f[23] + (729.0/4.0)*f[24] - 2187.0/4.0*f[25] + (2187.0/4.0)*f[26] - 729.0/4.0*f[27] - 81.0/2.0*f[28] + (243.0/2.0)*f[29] - 3267.0/8.0*f[2] - 243.0/2.0*f[30] + (81.0/2.0)*f[31] - 891.0/8.0*f[32] + (2673.0/8.0)*f[33] - 2673.0/8.0*f[34] + (891.0/8.0)*f[35] + (729.0/4.0)*f[36] - 2187.0/4.0*f[37] + (2187.0/4.0)*f[38] - 729.0/4.0*f[39] + (1089.0/8.0)*f[3] - 729.0/8.0*f[40] + (2187.0/8.0)*f[41] - 2187.0/8.0*f[42] + (729.0/8.0)*f[43] + (81.0/4.0)*f[44] - 243.0/4.0*f[45] + (243.0/4.0)*f[46] - 81.0/4.0*f[47] + (99.0/4.0)*f[48] - 297.0/4.0*f[49] + (891.0/4.0)*f[4] + (297.0/4.0)*f[50] - 99.0/4.0*f[51] - 81.0/2.0*f[52] + (243.0/2.0)*f[53] - 243.0/2.0*f[54] + (81.0/2.0)*f[55] + (81.0/4.0)*f[56] - 243.0/4.0*f[57] + (243.0/4.0)*f[58] - 81.0/4.0*f[59] - 2673.0/4.0*f[5] - 9.0/2.0*f[60] + (27.0/2.0)*f[61] - 27.0/2.0*f[62] + (9.0/2.0)*f[63] + (2673.0/4.0)*f[6] - 891.0/4.0*f[7] - 891.0/8.0*f[8] + (2673.0/8.0)*f[9] + z*((891.0/4.0)*f[0] + (2187.0/4.0)*f[10] - 729.0/4.0*f[11] - 81.0/2.0*f[12] + (243.0/2.0)*f[13] - 243.0/2.0*f[14] + (81.0/2.0)*f[15] - 4455.0/8.0*f[16] + (13365.0/8.0)*f[17] - 13365.0/8.0*f[18] + (4455.0/8.0)*f[19] - 2673.0/4.0*f[1] + (3645.0/4.0)*f[20] - 10935.0/4.0*f[21] + (10935.0/4.0)*f[22] - 3645.0/4.0*f[23] - 3645.0/8.0*f[24] + (10935.0/8.0)*f[25] - 10935.0/8.0*f[26] + (3645.0/8.0)*f[27] + (405.0/4.0)*f[28] - 1215.0/4.0*f[29] + (2673.0/4.0)*f[2] + (1215.0/4.0)*f[30] - 405.0/4.0*f[31] + (891.0/2.0)*f[32] - 2673.0/2.0*f[33] + (2673.0/2.0)*f[34] - 891.0/2.0*f[35] - 729*f[36] + 2187*f[37] - 2187*f[38] + 729*f[39] - 891.0/4.0*f[3] + (729.0/2.0)*f[40] - 2187.0/2.0*f[41] + (2187.0/2.0)*f[42] - 729.0/2.0*f[43] - 81*f[44] + 243*f[45] - 243*f[46] + 81*f[47] - 891.0/8.0*f[48] + (2673.0/8.0)*f[49] - 729.0/2.0*f[4] - 2673.0/8.0*f[50] + (891.0/8.0)*f[51] + (729.0/4.0)*f[52] - 2187.0/4.0*f[53] + (2187.0/4.0)*f[54] - 729.0/4.0*f[55] - 729.0/8.0*f[56] + (2187.0/8.0)*f[57] - 2187.0/8.0*f[58] + (729.0/8.0)*f[59] + (2187.0/2.0)*f[5] + (81.0/4.0)*f[60] - 243.0/4.0*f[61] + (243.0/4.0)*f[62] - 81.0/4.0*f[63] - 2187.0/2.0*f[6] + (729.0/2.0)*f[7] + (729.0/4.0)*f[8] - 2187.0/4.0*f[9] + z*(-891.0/8.0*f[0] - 2187.0/8.0*f[10] + (729.0/8.0)*f[11] + (81.0/4.0)*f[12] - 243.0/4.0*f[13] + (243.0/4.0)*f[14] - 81.0/4.0*f[15] + (2673.0/8.0)*f[16] - 8019.0/8.0*f[17] + (8019.0/8.0)*f[18] - 2673.0/8.0*f[19] + (2673.0/8.0)*f[1] - 2187.0/4.0*f[20] + (6561.0/4.0)*f[21] - 6561.0/4.0*f[22] + (2187.0/4.0)*f[23] + (2187.0/8.0)*f[24] - 6561.0/8.0*f[25] + (6561.0/8.0)*f[26] - 2187.0/8.0*f[27] - 243.0/4.0*f[28] + (729.0/4.0)*f[29] - 2673.0/8.0*f[2] - 729.0/4.0*f[30] + (243.0/4.0)*f[31] - 2673.0/8.0*f[32] + (8019.0/8.0)*f[33] - 8019.0/8.0*f[34] + (2673.0/8.0)*f[35] + (2187.0/4.0)*f[36] - 6561.0/4.0*f[37] + (6561.0/4.0)*f[38] - 2187.0/4.0*f[39] + (891.0/8.0)*f[3] - 2187.0/8.0*f[40] + (6561.0/8.0)*f[41] - 6561.0/8.0*f[42] + (2187.0/8.0)*f[43] + (243.0/4.0)*f[44] - 729.0/4.0*f[45] + (729.0/4.0)*f[46] - 243.0/4.0*f[47] + (891.0/8.0)*f[48] - 2673.0/8.0*f[49] + (729.0/4.0)*f[4] + (2673.0/8.0)*f[50] - 891.0/8.0*f[51] - 729.0/4.0*f[52] + (2187.0/4.0)*f[53] - 2187.0/4.0*f[54] + (729.0/4.0)*f[55] + (729.0/8.0)*f[56] - 2187.0/8.0*f[57] + (2187.0/8.0)*f[58] - 729.0/8.0*f[59] - 2187.0/4.0*f[5] - 81.0/4.0*f[60] + (243.0/4.0)*f[61] - 243.0/4.0*f[62] + (81.0/4.0)*f[63] + (2187.0/4.0)*f[6] - 729.0/4.0*f[7] - 729.0/8.0*f[8] + (2187.0/8.0)*f[9])))) + z*((99.0/4.0)*f[0] - 81.0/2.0*f[16] + (243.0/2.0)*f[17] - 243.0/2.0*f[18] + (81.0/2.0)*f[19] - 297.0/4.0*f[1] + (297.0/4.0)*f[2] + (81.0/4.0)*f[32] - 243.0/4.0*f[33] + (243.0/4.0)*f[34] - 81.0/4.0*f[35] - 99.0/4.0*f[3] - 9.0/2.0*f[48] + (27.0/2.0)*f[49] - 27.0/2.0*f[50] + (9.0/2.0)*f[51] + z*(-81.0/2.0*f[0] + (405.0/4.0)*f[16] - 1215.0/4.0*f[17] + (1215.0/4.0)*f[18] - 405.0/4.0*f[19] + (243.0/2.0)*f[1] - 243.0/2.0*f[2] - 81*f[32] + 243*f[33] - 243*f[34] + 81*f[35] + (81.0/2.0)*f[3] + (81.0/4.0)*f[48] - 243.0/4.0*f[49] + (243.0/4.0)*f[50] - 81.0/4.0*f[51] + z*((81.0/4.0)*f[0] - 243.0/4.0*f[16] + (729.0/4.0)*f[17] - 729.0/4.0*f[18] + (243.0/4.0)*f[19] - 243.0/4.0*f[1] + (243.0/4.0)*f[2] + (243.0/4.0)*f[32] - 729.0/4.0*f[33] + (729.0/4.0)*f[34] - 243.0/4.0*f[35] - 81.0/4.0*f[3] - 81.0/4.0*f[48] + (243.0/4.0)*f[49] - 243.0/4.0*f[50] + (81.0/4.0)*f[51])))) + y*(-99.0/2.0*f[0] - 81*f[10] + (81.0/4.0)*f[11] + 9*f[12] - 45.0/2.0*f[13] + 18*f[14] - 9.0/2.0*f[15] + (495.0/4.0)*f[1] - 99*f[2] + (99.0/4.0)*f[3] + 81*f[4] - 405.0/2.0*f[5] + 162*f[6] - 81.0/2.0*f[7] - 81.0/2.0*f[8] + (405.0/4.0)*f[9] + y*(81*f[0] + 324*f[10] - 81*f[11] - 81.0/2.0*f[12] + (405.0/4.0)*f[13] - 81*f[14] + (81.0/4.0)*f[15] - 405.0/2.0*f[1] + 162*f[2] - 81.0/2.0*f[3] - 405.0/2.0*f[4] + (2025.0/4.0)*f[5] - 405*f[6] + (405.0/4.0)*f[7] + 162*f[8] - 405*f[9] + y*(-81.0/2.0*f[0] - 243*f[10] + (243.0/4.0)*f[11] + (81.0/2.0)*f[12] - 405.0/4.0*f[13] + 81*f[14] - 81.0/4.0*f[15] + (405.0/4.0)*f[1] - 81*f[2] + (81.0/4.0)*f[3] + (243.0/2.0)*f[4] - 1215.0/4.0*f[5] + 243*f[6] - 243.0/4.0*f[7] - 243.0/2.0*f[8] + (1215.0/4.0)*f[9] + z*((891.0/4.0)*f[0] + (2673.0/2.0)*f[10] - 2673.0/8.0*f[11] - 891.0/4.0*f[12] + (4455.0/8.0)*f[13] - 891.0/2.0*f[14] + (891.0/8.0)*f[15] - 729.0/2.0*f[16] + (3645.0/4.0)*f[17] - 729*f[18] + (729.0/4.0)*f[19] - 4455.0/8.0*f[1] + (2187.0/2.0)*f[20] - 10935.0/4.0*f[21] + 2187*f[22] - 2187.0/4.0*f[23] - 2187.0/2.0*f[24] + (10935.0/4.0)*f[25] - 2187*f[26] + (2187.0/4.0)*f[27] + (729.0/2.0)*f[28] - 3645.0/4.0*f[29] + (891.0/2.0)*f[2] + 729*f[30] - 729.0/4.0*f[31] + (729.0/4.0)*f[32] - 3645.0/8.0*f[33] + (729.0/2.0)*f[34] - 729.0/8.0*f[35] - 2187.0/4.0*f[36] + (10935.0/8.0)*f[37] - 2187.0/2.0*f[38] + (2187.0/8.0)*f[39] - 891.0/8.0*f[3] + (2187.0/4.0)*f[40] - 10935.0/8.0*f[41] + (2187.0/2.0)*f[42] - 2187.0/8.0*f[43] - 729.0/4.0*f[44] + (3645.0/8.0)*f[45] - 729.0/2.0*f[46] + (729.0/8.0)*f[47] - 81.0/2.0*f[48] + (405.0/4.0)*f[49] - 2673.0/4.0*f[4] - 81*f[50] + (81.0/4.0)*f[51] + (243.0/2.0)*f[52] - 1215.0/4.0*f[53] + 243*f[54] - 243.0/4.0*f[55] - 243.0/2.0*f[56] + (1215.0/4.0)*f[57] - 243*f[58] + (243.0/4.0)*f[59] + (13365.0/8.0)*f[5] + (81.0/2.0)*f[60] - 405.0/4.0*f[61] + 81*f[62] - 81.0/4.0*f[63] - 2673.0/2.0*f[6] + (2673.0/8.0)*f[7] + (2673.0/4.0)*f[8] - 13365.0/8.0*f[9] + z*(-729.0/2.0*f[0] - 2187*f[10] + (2187.0/4.0)*f[11] + (729.0/2.0)*f[12] - 3645.0/4.0*f[13] + 729*f[14] - 729.0/4.0*f[15] + (3645.0/4.0)*f[16] - 18225.0/8.0*f[17] + (3645.0/2.0)*f[18] - 3645.0/8.0*f[19] + (3645.0/4.0)*f[1] - 10935.0/4.0*f[20] + (54675.0/8.0)*f[21] - 10935.0/2.0*f[22] + (10935.0/8.0)*f[23] + (10935.0/4.0)*f[24] - 54675.0/8.0*f[25] + (10935.0/2.0)*f[26] - 10935.0/8.0*f[27] - 3645.0/4.0*f[28] + (18225.0/8.0)*f[29] - 729*f[2] - 3645.0/2.0*f[30] + (3645.0/8.0)*f[31] - 729*f[32] + (3645.0/2.0)*f[33] - 1458*f[34] + (729.0/2.0)*f[35] + 2187*f[36] - 10935.0/2.0*f[37] + 4374*f[38] - 2187.0/2.0*f[39] + (729.0/4.0)*f[3] - 2187*f[40] + (10935.0/2.0)*f[41] - 4374*f[42] + (2187.0/2.0)*f[43] + 729*f[44] - 3645.0/2.0*f[45] + 1458*f[46] - 729.0/2.0*f[47] + (729.0/4.0)*f[48] - 3645.0/8.0*f[49] + (2187.0/2.0)*f[4] + (729.0/2.0)*f[50] - 729.0/8.0*f[51] - 2187.0/4.0*f[52] + (10935.0/8.0)*f[53] - 2187.0/2.0*f[54] + (2187.0/8.0)*f[55] + (2187.0/4.0)*f[56] - 10935.0/8.0*f[57] + (2187.0/2.0)*f[58] - 2187.0/8.0*f[59] - 10935.0/4.0*f[5] - 729.0/4.0*f[60] + (3645.0/8.0)*f[61] - 729.0/2.0*f[62] + (729.0/8.0)*f[63] + 2187*f[6] - 2187.0/4.0*f[7] - 2187.0/2.0*f[8] + (10935.0/4.0)*f[9] + z*((729.0/4.0)*f[0] + (2187.0/2.0)*f[10] - 2187.0/8.0*f[11] - 729.0/4.0*f[12] + (3645.0/8.0)*f[13] - 729.0/2.0*f[14] + (729.0/8.0)*f[15] - 2187.0/4.0*f[16] + (10935.0/8.0)*f[17] - 2187.0/2.0*f[18] + (2187.0/8.0)*f[19] - 3645.0/8.0*f[1] + (6561.0/4.0)*f[20] - 32805.0/8.0*f[21] + (6561.0/2.0)*f[22] - 6561.0/8.0*f[23] - 6561.0/4.0*f[24] + (32805.0/8.0)*f[25] - 6561.0/2.0*f[26] + (6561.0/8.0)*f[27] + (2187.0/4.0)*f[28] - 10935.0/8.0*f[29] + (729.0/2.0)*f[2] + (2187.0/2.0)*f[30] - 2187.0/8.0*f[31] + (2187.0/4.0)*f[32] - 10935.0/8.0*f[33] + (2187.0/2.0)*f[34] - 2187.0/8.0*f[35] - 6561.0/4.0*f[36] + (32805.0/8.0)*f[37] - 6561.0/2.0*f[38] + (6561.0/8.0)*f[39] - 729.0/8.0*f[3] + (6561.0/4.0)*f[40] - 32805.0/8.0*f[41] + (6561.0/2.0)*f[42] - 6561.0/8.0*f[43] - 2187.0/4.0*f[44] + (10935.0/8.0)*f[45] - 2187.0/2.0*f[46] + (2187.0/8.0)*f[47] - 729.0/4.0*f[48] + (3645.0/8.0)*f[49] - 2187.0/4.0*f[4] - 729.0/2.0*f[50] + (729.0/8.0)*f[51] + (2187.0/4.0)*f[52] - 10935.0/8.0*f[53] + (2187.0/2.0)*f[54] - 2187.0/8.0*f[55] - 2187.0/4.0*f[56] + (10935.0/8.0)*f[57] - 2187.0/2.0*f[58] + (2187.0/8.0)*f[59] + (10935.0/8.0)*f[5] + (729.0/4.0)*f[60] - 3645.0/8.0*f[61] + (729.0/2.0)*f[62] - 729.0/8.0*f[63] - 2187.0/2.0*f[6] + (2187.0/8.0)*f[7] + (2187.0/4.0)*f[8] - 10935.0/8.0*f[9])))) + z*(-891.0/2.0*f[0] - 1782*f[10] + (891.0/2.0)*f[11] + (891.0/4.0)*f[12] - 4455.0/8.0*f[13] + (891.0/2.0)*f[14] - 891.0/8.0*f[15] + 729*f[16] - 3645.0/2.0*f[17] + 1458*f[18] - 729.0/2.0*f[19] + (4455.0/4.0)*f[1] - 3645.0/2.0*f[20] + (18225.0/4.0)*f[21] - 3645*f[22] + (3645.0/4.0)*f[23] + 1458*f[24] - 3645*f[25] + 2916*f[26] - 729*f[27] - 729.0/2.0*f[28] + (3645.0/4.0)*f[29] - 891*f[2] - 729*f[30] + (729.0/4.0)*f[31] - 729.0/2.0*f[32] + (3645.0/4.0)*f[33] - 729*f[34] + (729.0/4.0)*f[35] + (3645.0/4.0)*f[36] - 18225.0/8.0*f[37] + (3645.0/2.0)*f[38] - 3645.0/8.0*f[39] + (891.0/4.0)*f[3] - 729*f[40] + (3645.0/2.0)*f[41] - 1458*f[42] + (729.0/2.0)*f[43] + (729.0/4.0)*f[44] - 3645.0/8.0*f[45] + (729.0/2.0)*f[46] - 729.0/8.0*f[47] + 81*f[48] - 405.0/2.0*f[49] + (4455.0/4.0)*f[4] + 162*f[50] - 81.0/2.0*f[51] - 405.0/2.0*f[52] + (2025.0/4.0)*f[53] - 405*f[54] + (405.0/4.0)*f[55] + 162*f[56] - 405*f[57] + 324*f[58] - 81*f[59] - 22275.0/8.0*f[5] - 81.0/2.0*f[60] + (405.0/4.0)*f[61] - 81*f[62] + (81.0/4.0)*f[63] + (4455.0/2.0)*f[6] - 4455.0/8.0*f[7] - 891*f[8] + (4455.0/2.0)*f[9] + z*(729*f[0] + 2916*f[10] - 729*f[11] - 729.0/2.0*f[12] + (3645.0/4.0)*f[13] - 729*f[14] + (729.0/4.0)*f[15] - 3645.0/2.0*f[16] + (18225.0/4.0)*f[17] - 3645*f[18] + (3645.0/4.0)*f[19] - 3645.0/2.0*f[1] + (18225.0/4.0)*f[20] - 91125.0/8.0*f[21] + (18225.0/2.0)*f[22] - 18225.0/8.0*f[23] - 3645*f[24] + (18225.0/2.0)*f[25] - 7290*f[26] + (3645.0/2.0)*f[27] + (3645.0/4.0)*f[28] - 18225.0/8.0*f[29] + 1458*f[2] + (3645.0/2.0)*f[30] - 3645.0/8.0*f[31] + 1458*f[32] - 3645*f[33] + 2916*f[34] - 729*f[35] - 3645*f[36] + (18225.0/2.0)*f[37] - 7290*f[38] + (3645.0/2.0)*f[39] - 729.0/2.0*f[3] + 2916*f[40] - 7290*f[41] + 5832*f[42] - 1458*f[43] - 729*f[44] + (3645.0/2.0)*f[45] - 1458*f[46] + (729.0/2.0)*f[47] - 729.0/2.0*f[48] + (3645.0/4.0)*f[49] - 3645.0/2.0*f[4] - 729*f[50] + (729.0/4.0)*f[51] + (3645.0/4.0)*f[52] - 18225.0/8.0*f[53] + (3645.0/2.0)*f[54] - 3645.0/8.0*f[55] - 729*f[56] + (3645.0/2.0)*f[57] - 1458*f[58] + (729.0/2.0)*f[59] + (18225.0/4.0)*f[5] + (729.0/4.0)*f[60] - 3645.0/8.0*f[61] + (729.0/2.0)*f[62] - 729.0/8.0*f[63] - 3645*f[6] + (3645.0/4.0)*f[7] + 1458*f[8] - 3645*f[9] + z*(-729.0/2.0*f[0] - 1458*f[10] + (729.0/2.0)*f[11] + (729.0/4.0)*f[12] - 3645.0/8.0*f[13] + (729.0/2.0)*f[14] - 729.0/8.0*f[15] + (2187.0/2.0)*f[16] - 10935.0/4.0*f[17] + 2187*f[18] - 2187.0/4.0*f[19] + (3645.0/4.0)*f[1] - 10935.0/4.0*f[20] + (54675.0/8.0)*f[21] - 10935.0/2.0*f[22] + (10935.0/8.0)*f[23] + 2187*f[24] - 10935.0/2.0*f[25] + 4374*f[26] - 2187.0/2.0*f[27] - 2187.0/4.0*f[28] + (10935.0/8.0)*f[29] - 729*f[2] - 2187.0/2.0*f[30] + (2187.0/8.0)*f[31] - 2187.0/2.0*f[32] + (10935.0/4.0)*f[33] - 2187*f[34] + (2187.0/4.0)*f[35] + (10935.0/4.0)*f[36] - 54675.0/8.0*f[37] + (10935.0/2.0)*f[38] - 10935.0/8.0*f[39] + (729.0/4.0)*f[3] - 2187*f[40] + (10935.0/2.0)*f[41] - 4374*f[42] + (2187.0/2.0)*f[43] + (2187.0/4.0)*f[44] - 10935.0/8.0*f[45] + (2187.0/2.0)*f[46] - 2187.0/8.0*f[47] + (729.0/2.0)*f[48] - 3645.0/4.0*f[49] + (3645.0/4.0)*f[4] + 729*f[50] - 729.0/4.0*f[51] - 3645.0/4.0*f[52] + (18225.0/8.0)*f[53] - 3645.0/2.0*f[54] + (3645.0/8.0)*f[55] + 729*f[56] - 3645.0/2.0*f[57] + 1458*f[58] - 729.0/2.0*f[59] - 18225.0/8.0*f[5] - 729.0/4.0*f[60] + (3645.0/8.0)*f[61] - 729.0/2.0*f[62] + (729.0/8.0)*f[63] + (3645.0/2.0)*f[6] - 3645.0/8.0*f[7] - 729*f[8] + (3645.0/2.0)*f[9])))) + z*((1089.0/4.0)*f[0] + (891.0/2.0)*f[10] - 891.0/8.0*f[11] - 99.0/2.0*f[12] + (495.0/4.0)*f[13] - 99*f[14] + (99.0/4.0)*f[15] - 891.0/2.0*f[16] + (4455.0/4.0)*f[17] - 891*f[18] + (891.0/4.0)*f[19] - 5445.0/8.0*f[1] + 729*f[20] - 3645.0/2.0*f[21] + 1458*f[22] - 729.0/2.0*f[23] - 729.0/2.0*f[24] + (3645.0/4.0)*f[25] - 729*f[26] + (729.0/4.0)*f[27] + 81*f[28] - 405.0/2.0*f[29] + (1089.0/2.0)*f[2] + 162*f[30] - 81.0/2.0*f[31] + (891.0/4.0)*f[32] - 4455.0/8.0*f[33] + (891.0/2.0)*f[34] - 891.0/8.0*f[35] - 729.0/2.0*f[36] + (3645.0/4.0)*f[37] - 729*f[38] + (729.0/4.0)*f[39] - 1089.0/8.0*f[3] + (729.0/4.0)*f[40] - 3645.0/8.0*f[41] + (729.0/2.0)*f[42] - 729.0/8.0*f[43] - 81.0/2.0*f[44] + (405.0/4.0)*f[45] - 81*f[46] + (81.0/4.0)*f[47] - 99.0/2.0*f[48] + (495.0/4.0)*f[49] - 891.0/2.0*f[4] - 99*f[50] + (99.0/4.0)*f[51] + 81*f[52] - 405.0/2.0*f[53] + 162*f[54] - 81.0/2.0*f[55] - 81.0/2.0*f[56] + (405.0/4.0)*f[57] - 81*f[58] + (81.0/4.0)*f[59] + (4455.0/4.0)*f[5] + 9*f[60] - 45.0/2.0*f[61] + 18*f[62] - 9.0/2.0*f[63] - 891*f[6] + (891.0/4.0)*f[7] + (891.0/4.0)*f[8] - 4455.0/8.0*f[9] + z*(-891.0/2.0*f[0] - 729*f[10] + (729.0/4.0)*f[11] + 81*f[12] - 405.0/2.0*f[13] + 162*f[14] - 81.0/2.0*f[15] + (4455.0/4.0)*f[16] - 22275.0/8.0*f[17] + (4455.0/2.0)*f[18] - 4455.0/8.0*f[19] + (4455.0/4.0)*f[1] - 3645.0/2.0*f[20] + (18225.0/4.0)*f[21] - 3645*f[22] + (3645.0/4.0)*f[23] + (3645.0/4.0)*f[24] - 18225.0/8.0*f[25] + (3645.0/2.0)*f[26] - 3645.0/8.0*f[27] - 405.0/2.0*f[28] + (2025.0/4.0)*f[29] - 891*f[2] - 405*f[30] + (405.0/4.0)*f[31] - 891*f[32] + (4455.0/2.0)*f[33] - 1782*f[34] + (891.0/2.0)*f[35] + 1458*f[36] - 3645*f[37] + 2916*f[38] - 729*f[39] + (891.0/4.0)*f[3] - 729*f[40] + (3645.0/2.0)*f[41] - 1458*f[42] + (729.0/2.0)*f[43] + 162*f[44] - 405*f[45] + 324*f[46] - 81*f[47] + (891.0/4.0)*f[48] - 4455.0/8.0*f[49] + 729*f[4] + (891.0/2.0)*f[50] - 891.0/8.0*f[51] - 729.0/2.0*f[52] + (3645.0/4.0)*f[53] - 729*f[54] + (729.0/4.0)*f[55] + (729.0/4.0)*f[56] - 3645.0/8.0*f[57] + (729.0/2.0)*f[58] - 729.0/8.0*f[59] - 3645.0/2.0*f[5] - 81.0/2.0*f[60] + (405.0/4.0)*f[61] - 81*f[62] + (81.0/4.0)*f[63] + 1458*f[6] - 729.0/2.0*f[7] - 729.0/2.0*f[8] + (3645.0/4.0)*f[9] + z*((891.0/4.0)*f[0] + (729.0/2.0)*f[10] - 729.0/8.0*f[11] - 81.0/2.0*f[12] + (405.0/4.0)*f[13] - 81*f[14] + (81.0/4.0)*f[15] - 2673.0/4.0*f[16] + (13365.0/8.0)*f[17] - 2673.0/2.0*f[18] + (2673.0/8.0)*f[19] - 4455.0/8.0*f[1] + (2187.0/2.0)*f[20] - 10935.0/4.0*f[21] + 2187*f[22] - 2187.0/4.0*f[23] - 2187.0/4.0*f[24] + (10935.0/8.0)*f[25] - 2187.0/2.0*f[26] + (2187.0/8.0)*f[27] + (243.0/2.0)*f[28] - 1215.0/4.0*f[29] + (891.0/2.0)*f[2] + 243*f[30] - 243.0/4.0*f[31] + (2673.0/4.0)*f[32] - 13365.0/8.0*f[33] + (2673.0/2.0)*f[34] - 2673.0/8.0*f[35] - 2187.0/2.0*f[36] + (10935.0/4.0)*f[37] - 2187*f[38] + (2187.0/4.0)*f[39] - 891.0/8.0*f[3] + (2187.0/4.0)*f[40] - 10935.0/8.0*f[41] + (2187.0/2.0)*f[42] - 2187.0/8.0*f[43] - 243.0/2.0*f[44] + (1215.0/4.0)*f[45] - 243*f[46] + (243.0/4.0)*f[47] - 891.0/4.0*f[48] + (4455.0/8.0)*f[49] - 729.0/2.0*f[4] - 891.0/2.0*f[50] + (891.0/8.0)*f[51] + (729.0/2.0)*f[52] - 3645.0/4.0*f[53] + 729*f[54] - 729.0/4.0*f[55] - 729.0/4.0*f[56] + (3645.0/8.0)*f[57] - 729.0/2.0*f[58] + (729.0/8.0)*f[59] + (3645.0/4.0)*f[5] + (81.0/2.0)*f[60] - 405.0/4.0*f[61] + 81*f[62] - 81.0/4.0*f[63] - 729*f[6] + (729.0/4.0)*f[7] + (729.0/4.0)*f[8] - 3645.0/8.0*f[9])))) + z*(-99.0/2.0*f[0] + 81*f[16] - 405.0/2.0*f[17] + 162*f[18] - 81.0/2.0*f[19] + (495.0/4.0)*f[1] - 99*f[2] - 81.0/2.0*f[32] + (405.0/4.0)*f[33] - 81*f[34] + (81.0/4.0)*f[35] + (99.0/4.0)*f[3] + 9*f[48] - 45.0/2.0*f[49] + 18*f[50] - 9.0/2.0*f[51] + z*(81*f[0] - 405.0/2.0*f[16] + (2025.0/4.0)*f[17] - 405*f[18] + (405.0/4.0)*f[19] - 405.0/2.0*f[1] + 162*f[2] + 162*f[32] - 405*f[33] + 324*f[34] - 81*f[35] - 81.0/2.0*f[3] - 81.0/2.0*f[48] + (405.0/4.0)*f[49] - 81*f[50] + (81.0/4.0)*f[51] + z*(-81.0/2.0*f[0] + (243.0/2.0)*f[16] - 1215.0/4.0*f[17] + 243*f[18] - 243.0/4.0*f[19] + (405.0/4.0)*f[1] - 81*f[2] - 243.0/2.0*f[32] + (1215.0/4.0)*f[33] - 243*f[34] + (243.0/4.0)*f[35] + (81.0/4.0)*f[3] + (81.0/2.0)*f[48] - 405.0/4.0*f[49] + 81*f[50] - 81.0/4.0*f[51])))) + y*((121.0/4.0)*f[0] + (81.0/4.0)*f[10] - 9.0/2.0*f[11] - 11.0/2.0*f[12] + 9*f[13] - 9.0/2.0*f[14] + f[15] - 99.0/2.0*f[1] + (99.0/4.0)*f[2] - 11.0/2.0*f[3] - 99.0/2.0*f[4] + 81*f[5] - 81.0/2.0*f[6] + 9*f[7] + (99.0/4.0)*f[8] - 81.0/2.0*f[9] + y*(-99.0/2.0*f[0] - 81*f[10] + 18*f[11] + (99.0/4.0)*f[12] - 81.0/2.0*f[13] + (81.0/4.0)*f[14] - 9.0/2.0*f[15] + 81*f[1] - 81.0/2.0*f[2] + 9*f[3] + (495.0/4.0)*f[4] - 405.0/2.0*f[5] + (405.0/4.0)*f[6] - 45.0/2.0*f[7] - 99*f[8] + 162*f[9] + y*((99.0/4.0)*f[0] + (243.0/4.0)*f[10] - 27.0/2.0*f[11] - 99.0/4.0*f[12] + (81.0/2.0)*f[13] - 81.0/4.0*f[14] + (9.0/2.0)*f[15] - 81.0/2.0*f[1] + (81.0/4.0)*f[2] - 9.0/2.0*f[3] - 297.0/4.0*f[4] + (243.0/2.0)*f[5] - 243.0/4.0*f[6] + (27.0/2.0)*f[7] + (297.0/4.0)*f[8] - 243.0/2.0*f[9] + z*(-1089.0/8.0*f[0] - 2673.0/8.0*f[10] + (297.0/4.0)*f[11] + (1089.0/8.0)*f[12] - 891.0/4.0*f[13] + (891.0/8.0)*f[14] - 99.0/4.0*f[15] + (891.0/4.0)*f[16] - 729.0/2.0*f[17] + (729.0/4.0)*f[18] - 81.0/2.0*f[19] + (891.0/4.0)*f[1] - 2673.0/4.0*f[20] + (2187.0/2.0)*f[21] - 2187.0/4.0*f[22] + (243.0/2.0)*f[23] + (2673.0/4.0)*f[24] - 2187.0/2.0*f[25] + (2187.0/4.0)*f[26] - 243.0/2.0*f[27] - 891.0/4.0*f[28] + (729.0/2.0)*f[29] - 891.0/8.0*f[2] - 729.0/4.0*f[30] + (81.0/2.0)*f[31] - 891.0/8.0*f[32] + (729.0/4.0)*f[33] - 729.0/8.0*f[34] + (81.0/4.0)*f[35] + (2673.0/8.0)*f[36] - 2187.0/4.0*f[37] + (2187.0/8.0)*f[38] - 243.0/4.0*f[39] + (99.0/4.0)*f[3] - 2673.0/8.0*f[40] + (2187.0/4.0)*f[41] - 2187.0/8.0*f[42] + (243.0/4.0)*f[43] + (891.0/8.0)*f[44] - 729.0/4.0*f[45] + (729.0/8.0)*f[46] - 81.0/4.0*f[47] + (99.0/4.0)*f[48] - 81.0/2.0*f[49] + (3267.0/8.0)*f[4] + (81.0/4.0)*f[50] - 9.0/2.0*f[51] - 297.0/4.0*f[52] + (243.0/2.0)*f[53] - 243.0/4.0*f[54] + (27.0/2.0)*f[55] + (297.0/4.0)*f[56] - 243.0/2.0*f[57] + (243.0/4.0)*f[58] - 27.0/2.0*f[59] - 2673.0/4.0*f[5] - 99.0/4.0*f[60] + (81.0/2.0)*f[61] - 81.0/4.0*f[62] + (9.0/2.0)*f[63] + (2673.0/8.0)*f[6] - 297.0/4.0*f[7] - 3267.0/8.0*f[8] + (2673.0/4.0)*f[9] + z*((891.0/4.0)*f[0] + (2187.0/4.0)*f[10] - 243.0/2.0*f[11] - 891.0/4.0*f[12] + (729.0/2.0)*f[13] - 729.0/4.0*f[14] + (81.0/2.0)*f[15] - 4455.0/8.0*f[16] + (3645.0/4.0)*f[17] - 3645.0/8.0*f[18] + (405.0/4.0)*f[19] - 729.0/2.0*f[1] + (13365.0/8.0)*f[20] - 10935.0/4.0*f[21] + (10935.0/8.0)*f[22] - 1215.0/4.0*f[23] - 13365.0/8.0*f[24] + (10935.0/4.0)*f[25] - 10935.0/8.0*f[26] + (1215.0/4.0)*f[27] + (4455.0/8.0)*f[28] - 3645.0/4.0*f[29] + (729.0/4.0)*f[2] + (3645.0/8.0)*f[30] - 405.0/4.0*f[31] + (891.0/2.0)*f[32] - 729*f[33] + (729.0/2.0)*f[34] - 81*f[35] - 2673.0/2.0*f[36] + 2187*f[37] - 2187.0/2.0*f[38] + 243*f[39] - 81.0/2.0*f[3] + (2673.0/2.0)*f[40] - 2187*f[41] + (2187.0/2.0)*f[42] - 243*f[43] - 891.0/2.0*f[44] + 729*f[45] - 729.0/2.0*f[46] + 81*f[47] - 891.0/8.0*f[48] + (729.0/4.0)*f[49] - 2673.0/4.0*f[4] - 729.0/8.0*f[50] + (81.0/4.0)*f[51] + (2673.0/8.0)*f[52] - 2187.0/4.0*f[53] + (2187.0/8.0)*f[54] - 243.0/4.0*f[55] - 2673.0/8.0*f[56] + (2187.0/4.0)*f[57] - 2187.0/8.0*f[58] + (243.0/4.0)*f[59] + (2187.0/2.0)*f[5] + (891.0/8.0)*f[60] - 729.0/4.0*f[61] + (729.0/8.0)*f[62] - 81.0/4.0*f[63] - 2187.0/4.0*f[6] + (243.0/2.0)*f[7] + (2673.0/4.0)*f[8] - 2187.0/2.0*f[9] + z*(-891.0/8.0*f[0] - 2187.0/8.0*f[10] + (243.0/4.0)*f[11] + (891.0/8.0)*f[12] - 729.0/4.0*f[13] + (729.0/8.0)*f[14] - 81.0/4.0*f[15] + (2673.0/8.0)*f[16] - 2187.0/4.0*f[17] + (2187.0/8.0)*f[18] - 243.0/4.0*f[19] + (729.0/4.0)*f[1] - 8019.0/8.0*f[20] + (6561.0/4.0)*f[21] - 6561.0/8.0*f[22] + (729.0/4.0)*f[23] + (8019.0/8.0)*f[24] - 6561.0/4.0*f[25] + (6561.0/8.0)*f[26] - 729.0/4.0*f[27] - 2673.0/8.0*f[28] + (2187.0/4.0)*f[29] - 729.0/8.0*f[2] - 2187.0/8.0*f[30] + (243.0/4.0)*f[31] - 2673.0/8.0*f[32] + (2187.0/4.0)*f[33] - 2187.0/8.0*f[34] + (243.0/4.0)*f[35] + (8019.0/8.0)*f[36] - 6561.0/4.0*f[37] + (6561.0/8.0)*f[38] - 729.0/4.0*f[39] + (81.0/4.0)*f[3] - 8019.0/8.0*f[40] + (6561.0/4.0)*f[41] - 6561.0/8.0*f[42] + (729.0/4.0)*f[43] + (2673.0/8.0)*f[44] - 2187.0/4.0*f[45] + (2187.0/8.0)*f[46] - 243.0/4.0*f[47] + (891.0/8.0)*f[48] - 729.0/4.0*f[49] + (2673.0/8.0)*f[4] + (729.0/8.0)*f[50] - 81.0/4.0*f[51] - 2673.0/8.0*f[52] + (2187.0/4.0)*f[53] - 2187.0/8.0*f[54] + (243.0/4.0)*f[55] + (2673.0/8.0)*f[56] - 2187.0/4.0*f[57] + (2187.0/8.0)*f[58] - 243.0/4.0*f[59] - 2187.0/4.0*f[5] - 891.0/8.0*f[60] + (729.0/4.0)*f[61] - 729.0/8.0*f[62] + (81.0/4.0)*f[63] + (2187.0/8.0)*f[6] - 243.0/4.0*f[7] - 2673.0/8.0*f[8] + (2187.0/4.0)*f[9])))) + z*((1089.0/4.0)*f[0] + (891.0/2.0)*f[10] - 99*f[11] - 1089.0/8.0*f[12] + (891.0/4.0)*f[13] - 891.0/8.0*f[14] + (99.0/4.0)*f[15] - 891.0/2.0*f[16] + 729*f[17] - 729.0/2.0*f[18] + 81*f[19] - 891.0/2.0*f[1] + (4455.0/4.0)*f[20] - 3645.0/2.0*f[21] + (3645.0/4.0)*f[22] - 405.0/2.0*f[23] - 891*f[24] + 1458*f[25] - 729*f[26] + 162*f[27] + (891.0/4.0)*f[28] - 729.0/2.0*f[29] + (891.0/4.0)*f[2] + (729.0/4.0)*f[30] - 81.0/2.0*f[31] + (891.0/4.0)*f[32] - 729.0/2.0*f[33] + (729.0/4.0)*f[34] - 81.0/2.0*f[35] - 4455.0/8.0*f[36] + (3645.0/4.0)*f[37] - 3645.0/8.0*f[38] + (405.0/4.0)*f[39] - 99.0/2.0*f[3] + (891.0/2.0)*f[40] - 729*f[41] + (729.0/2.0)*f[42] - 81*f[43] - 891.0/8.0*f[44] + (729.0/4.0)*f[45] - 729.0/8.0*f[46] + (81.0/4.0)*f[47] - 99.0/2.0*f[48] + 81*f[49] - 5445.0/8.0*f[4] - 81.0/2.0*f[50] + 9*f[51] + (495.0/4.0)*f[52] - 405.0/2.0*f[53] + (405.0/4.0)*f[54] - 45.0/2.0*f[55] - 99*f[56] + 162*f[57] - 81*f[58] + 18*f[59] + (4455.0/4.0)*f[5] + (99.0/4.0)*f[60] - 81.0/2.0*f[61] + (81.0/4.0)*f[62] - 9.0/2.0*f[63] - 4455.0/8.0*f[6] + (495.0/4.0)*f[7] + (1089.0/2.0)*f[8] - 891*f[9] + z*(-891.0/2.0*f[0] - 729*f[10] + 162*f[11] + (891.0/4.0)*f[12] - 729.0/2.0*f[13] + (729.0/4.0)*f[14] - 81.0/2.0*f[15] + (4455.0/4.0)*f[16] - 3645.0/2.0*f[17] + (3645.0/4.0)*f[18] - 405.0/2.0*f[19] + 729*f[1] - 22275.0/8.0*f[20] + (18225.0/4.0)*f[21] - 18225.0/8.0*f[22] + (2025.0/4.0)*f[23] + (4455.0/2.0)*f[24] - 3645*f[25] + (3645.0/2.0)*f[26] - 405*f[27] - 4455.0/8.0*f[28] + (3645.0/4.0)*f[29] - 729.0/2.0*f[2] - 3645.0/8.0*f[30] + (405.0/4.0)*f[31] - 891*f[32] + 1458*f[33] - 729*f[34] + 162*f[35] + (4455.0/2.0)*f[36] - 3645*f[37] + (3645.0/2.0)*f[38] - 405*f[39] + 81*f[3] - 1782*f[40] + 2916*f[41] - 1458*f[42] + 324*f[43] + (891.0/2.0)*f[44] - 729*f[45] + (729.0/2.0)*f[46] - 81*f[47] + (891.0/4.0)*f[48] - 729.0/2.0*f[49] + (4455.0/4.0)*f[4] + (729.0/4.0)*f[50] - 81.0/2.0*f[51] - 4455.0/8.0*f[52] + (3645.0/4.0)*f[53] - 3645.0/8.0*f[54] + (405.0/4.0)*f[55] + (891.0/2.0)*f[56] - 729*f[57] + (729.0/2.0)*f[58] - 81*f[59] - 3645.0/2.0*f[5] - 891.0/8.0*f[60] + (729.0/4.0)*f[61] - 729.0/8.0*f[62] + (81.0/4.0)*f[63] + (3645.0/4.0)*f[6] - 405.0/2.0*f[7] - 891*f[8] + 1458*f[9] + z*((891.0/4.0)*f[0] + (729.0/2.0)*f[10] - 81*f[11] - 891.0/8.0*f[12] + (729.0/4.0)*f[13] - 729.0/8.0*f[14] + (81.0/4.0)*f[15] - 2673.0/4.0*f[16] + (2187.0/2.0)*f[17] - 2187.0/4.0*f[18] + (243.0/2.0)*f[19] - 729.0/2.0*f[1] + (13365.0/8.0)*f[20] - 10935.0/4.0*f[21] + (10935.0/8.0)*f[22] - 1215.0/4.0*f[23] - 2673.0/2.0*f[24] + 2187*f[25] - 2187.0/2.0*f[26] + 243*f[27] + (2673.0/8.0)*f[28] - 2187.0/4.0*f[29] + (729.0/4.0)*f[2] + (2187.0/8.0)*f[30] - 243.0/4.0*f[31] + (2673.0/4.0)*f[32] - 2187.0/2.0*f[33] + (2187.0/4.0)*f[34] - 243.0/2.0*f[35] - 13365.0/8.0*f[36] + (10935.0/4.0)*f[37] - 10935.0/8.0*f[38] + (1215.0/4.0)*f[39] - 81.0/2.0*f[3] + (2673.0/2.0)*f[40] - 2187*f[41] + (2187.0/2.0)*f[42] - 243*f[43] - 2673.0/8.0*f[44] + (2187.0/4.0)*f[45] - 2187.0/8.0*f[46] + (243.0/4.0)*f[47] - 891.0/4.0*f[48] + (729.0/2.0)*f[49] - 4455.0/8.0*f[4] - 729.0/4.0*f[50] + (81.0/2.0)*f[51] + (4455.0/8.0)*f[52] - 3645.0/4.0*f[53] + (3645.0/8.0)*f[54] - 405.0/4.0*f[55] - 891.0/2.0*f[56] + 729*f[57] - 729.0/2.0*f[58] + 81*f[59] + (3645.0/4.0)*f[5] + (891.0/8.0)*f[60] - 729.0/4.0*f[61] + (729.0/8.0)*f[62] - 81.0/4.0*f[63] - 3645.0/8.0*f[6] + (405.0/4.0)*f[7] + (891.0/2.0)*f[8] - 729*f[9])))) + z*(-1331.0/8.0*f[0] - 891.0/8.0*f[10] + (99.0/4.0)*f[11] + (121.0/4.0)*f[12] - 99.0/2.0*f[13] + (99.0/4.0)*f[14] - 11.0/2.0*f[15] + (1089.0/4.0)*f[16] - 891.0/2.0*f[17] + (891.0/4.0)*f[18] - 99.0/2.0*f[19] + (1089.0/4.0)*f[1] - 891.0/2.0*f[20] + 729*f[21] - 729.0/2.0*f[22] + 81*f[23] + (891.0/4.0)*f[24] - 729.0/2.0*f[25] + (729.0/4.0)*f[26] - 81.0/2.0*f[27] - 99.0/2.0*f[28] + 81*f[29] - 1089.0/8.0*f[2] - 81.0/2.0*f[30] + 9*f[31] - 1089.0/8.0*f[32] + (891.0/4.0)*f[33] - 891.0/8.0*f[34] + (99.0/4.0)*f[35] + (891.0/4.0)*f[36] - 729.0/2.0*f[37] + (729.0/4.0)*f[38] - 81.0/2.0*f[39] + (121.0/4.0)*f[3] - 891.0/8.0*f[40] + (729.0/4.0)*f[41] - 729.0/8.0*f[42] + (81.0/4.0)*f[43] + (99.0/4.0)*f[44] - 81.0/2.0*f[45] + (81.0/4.0)*f[46] - 9.0/2.0*f[47] + (121.0/4.0)*f[48] - 99.0/2.0*f[49] + (1089.0/4.0)*f[4] + (99.0/4.0)*f[50] - 11.0/2.0*f[51] - 99.0/2.0*f[52] + 81*f[53] - 81.0/2.0*f[54] + 9*f[55] + (99.0/4.0)*f[56] - 81.0/2.0*f[57] + (81.0/4.0)*f[58] - 9.0/2.0*f[59] - 891.0/2.0*f[5] - 11.0/2.0*f[60] + 9*f[61] - 9.0/2.0*f[62] + f[63] + (891.0/4.0)*f[6] - 99.0/2.0*f[7] - 1089.0/8.0*f[8] + (891.0/4.0)*f[9] + z*((1089.0/4.0)*f[0] + (729.0/4.0)*f[10] - 81.0/2.0*f[11] - 99.0/2.0*f[12] + 81*f[13] - 81.0/2.0*f[14] + 9*f[15] - 5445.0/8.0*f[16] + (4455.0/4.0)*f[17] - 4455.0/8.0*f[18] + (495.0/4.0)*f[19] - 891.0/2.0*f[1] + (4455.0/4.0)*f[20] - 3645.0/2.0*f[21] + (3645.0/4.0)*f[22] - 405.0/2.0*f[23] - 4455.0/8.0*f[24] + (3645.0/4.0)*f[25] - 3645.0/8.0*f[26] + (405.0/4.0)*f[27] + (495.0/4.0)*f[28] - 405.0/2.0*f[29] + (891.0/4.0)*f[2] + (405.0/4.0)*f[30] - 45.0/2.0*f[31] + (1089.0/2.0)*f[32] - 891*f[33] + (891.0/2.0)*f[34] - 99*f[35] - 891*f[36] + 1458*f[37] - 729*f[38] + 162*f[39] - 99.0/2.0*f[3] + (891.0/2.0)*f[40] - 729*f[41] + (729.0/2.0)*f[42] - 81*f[43] - 99*f[44] + 162*f[45] - 81*f[46] + 18*f[47] - 1089.0/8.0*f[48] + (891.0/4.0)*f[49] - 891.0/2.0*f[4] - 891.0/8.0*f[50] + (99.0/4.0)*f[51] + (891.0/4.0)*f[52] - 729.0/2.0*f[53] + (729.0/4.0)*f[54] - 81.0/2.0*f[55] - 891.0/8.0*f[56] + (729.0/4.0)*f[57] - 729.0/8.0*f[58] + (81.0/4.0)*f[59] + 729*f[5] + (99.0/4.0)*f[60] - 81.0/2.0*f[61] + (81.0/4.0)*f[62] - 9.0/2.0*f[63] - 729.0/2.0*f[6] + 81*f[7] + (891.0/4.0)*f[8] - 729.0/2.0*f[9] + z*(-1089.0/8.0*f[0] - 729.0/8.0*f[10] + (81.0/4.0)*f[11] + (99.0/4.0)*f[12] - 81.0/2.0*f[13] + (81.0/4.0)*f[14] - 9.0/2.0*f[15] + (3267.0/8.0)*f[16] - 2673.0/4.0*f[17] + (2673.0/8.0)*f[18] - 297.0/4.0*f[19] + (891.0/4.0)*f[1] - 2673.0/4.0*f[20] + (2187.0/2.0)*f[21] - 2187.0/4.0*f[22] + (243.0/2.0)*f[23] + (2673.0/8.0)*f[24] - 2187.0/4.0*f[25] + (2187.0/8.0)*f[26] - 243.0/4.0*f[27] - 297.0/4.0*f[28] + (243.0/2.0)*f[29] - 891.0/8.0*f[2] - 243.0/4.0*f[30] + (27.0/2.0)*f[31] - 3267.0/8.0*f[32] + (2673.0/4.0)*f[33] - 2673.0/8.0*f[34] + (297.0/4.0)*f[35] + (2673.0/4.0)*f[36] - 2187.0/2.0*f[37] + (2187.0/4.0)*f[38] - 243.0/2.0*f[39] + (99.0/4.0)*f[3] - 2673.0/8.0*f[40] + (2187.0/4.0)*f[41] - 2187.0/8.0*f[42] + (243.0/4.0)*f[43] + (297.0/4.0)*f[44] - 243.0/2.0*f[45] + (243.0/4.0)*f[46] - 27.0/2.0*f[47] + (1089.0/8.0)*f[48] - 891.0/4.0*f[49] + (891.0/4.0)*f[4] + (891.0/8.0)*f[50] - 99.0/4.0*f[51] - 891.0/4.0*f[52] + (729.0/2.0)*f[53] - 729.0/4.0*f[54] + (81.0/2.0)*f[55] + (891.0/8.0)*f[56] - 729.0/4.0*f[57] + (729.0/8.0)*f[58] - 81.0/4.0*f[59] - 729.0/2.0*f[5] - 99.0/4.0*f[60] + (81.0/2.0)*f[61] - 81.0/4.0*f[62] + (9.0/2.0)*f[63] + (729.0/4.0)*f[6] - 81.0/2.0*f[7] - 891.0/8.0*f[8] + (729.0/4.0)*f[9])))) + z*((121.0/4.0)*f[0] - 99.0/2.0*f[16] + 81*f[17] - 81.0/2.0*f[18] + 9*f[19] - 99.0/2.0*f[1] + (99.0/4.0)*f[2] + (99.0/4.0)*f[32] - 81.0/2.0*f[33] + (81.0/4.0)*f[34] - 9.0/2.0*f[35] - 11.0/2.0*f[3] - 11.0/2.0*f[48] + 9*f[49] - 9.0/2.0*f[50] + f[51] + z*(-99.0/2.0*f[0] + (495.0/4.0)*f[16] - 405.0/2.0*f[17] + (405.0/4.0)*f[18] - 45.0/2.0*f[19] + 81*f[1] - 81.0/2.0*f[2] - 99*f[32] + 162*f[33] - 81*f[34] + 18*f[35] + 9*f[3] + (99.0/4.0)*f[48] - 81.0/2.0*f[49] + (81.0/4.0)*f[50] - 9.0/2.0*f[51] + z*((99.0/4.0)*f[0] - 297.0/4.0*f[16] + (243.0/2.0)*f[17] - 243.0/4.0*f[18] + (27.0/2.0)*f[19] - 81.0/2.0*f[1] + (81.0/4.0)*f[2] + (297.0/4.0)*f[32] - 243.0/2.0*f[33] + (243.0/4.0)*f[34] - 27.0/2.0*f[35] - 9.0/2.0*f[3] - 99.0/4.0*f[48] + (81.0/2.0)*f[49] - 81.0/4.0*f[50] + (9.0/2.0)*f[51])))) + y*(-11.0/2.0*f[0] + f[12] + 9*f[4] - 9.0/2.0*f[8] + y*(9*f[0] - 9.0/2.0*f[12] - 45.0/2.0*f[4] + 18*f[8] + y*(-9.0/2.0*f[0] + (9.0/2.0)*f[12] + (27.0/2.0)*f[4] - 27.0/2.0*f[8] + z*((99.0/4.0)*f[0] - 99.0/4.0*f[12] - 81.0/2.0*f[16] + (243.0/2.0)*f[20] - 243.0/2.0*f[24] + (81.0/2.0)*f[28] + (81.0/4.0)*f[32] - 243.0/4.0*f[36] + (243.0/4.0)*f[40] - 81.0/4.0*f[44] - 9.0/2.0*f[48] - 297.0/4.0*f[4] + (27.0/2.0)*f[52] - 27.0/2.0*f[56] + (9.0/2.0)*f[60] + (297.0/4.0)*f[8] + z*(-81.0/2.0*f[0] + (81.0/2.0)*f[12] + (405.0/4.0)*f[16] - 1215.0/4.0*f[20] + (1215.0/4.0)*f[24] - 405.0/4.0*f[28] - 81*f[32] + 243*f[36] - 243*f[40] + 81*f[44] + (81.0/4.0)*f[48] + (243.0/2.0)*f[4] - 243.0/4.0*f[52] + (243.0/4.0)*f[56] - 81.0/4.0*f[60] - 243.0/2.0*f[8] + z*((81.0/4.0)*f[0] - 81.0/4.0*f[12] - 243.0/4.0*f[16] + (729.0/4.0)*f[20] - 729.0/4.0*f[24] + (243.0/4.0)*f[28] + (243.0/4.0)*f[32] - 729.0/4.0*f[36] + (729.0/4.0)*f[40] - 243.0/4.0*f[44] - 81.0/4.0*f[48] - 243.0/4.0*f[4] + (243.0/4.0)*f[52] - 243.0/4.0*f[56] + (81.0/4.0)*f[60] + (243.0/4.0)*f[8])))) + z*(-99.0/2.0*f[0] + (99.0/4.0)*f[12] + 81*f[16] - 405.0/2.0*f[20] + 162*f[24] - 81.0/2.0*f[28] - 81.0/2.0*f[32] + (405.0/4.0)*f[36] - 81*f[40] + (81.0/4.0)*f[44] + 9*f[48] + (495.0/4.0)*f[4] - 45.0/2.0*f[52] + 18*f[56] - 9.0/2.0*f[60] - 99*f[8] + z*(81*f[0] - 81.0/2.0*f[12] - 405.0/2.0*f[16] + (2025.0/4.0)*f[20] - 405*f[24] + (405.0/4.0)*f[28] + 162*f[32] - 405*f[36] + 324*f[40] - 81*f[44] - 81.0/2.0*f[48] - 405.0/2.0*f[4] + (405.0/4.0)*f[52] - 81*f[56] + (81.0/4.0)*f[60] + 162*f[8] + z*(-81.0/2.0*f[0] + (81.0/4.0)*f[12] + (243.0/2.0)*f[16] - 1215.0/4.0*f[20] + 243*f[24] - 243.0/4.0*f[28] - 243.0/2.0*f[32] + (1215.0/4.0)*f[36] - 243*f[40] + (243.0/4.0)*f[44] + (81.0/2.0)*f[48] + (405.0/4.0)*f[4] - 405.0/4.0*f[52] + 81*f[56] - 81.0/4.0*f[60] - 81*f[8])))) + z*((121.0/4.0)*f[0] - 11.0/2.0*f[12] - 99.0/2.0*f[16] + 81*f[20] - 81.0/2.0*f[24] + 9*f[28] + (99.0/4.0)*f[32] - 81.0/2.0*f[36] + (81.0/4.0)*f[40] - 9.0/2.0*f[44] - 11.0/2.0*f[48] - 99.0/2.0*f[4] + 9*f[52] - 9.0/2.0*f[56] + f[60] + (99.0/4.0)*f[8] + z*(-99.0/2.0*f[0] + 9*f[12] + (495.0/4.0)*f[16] - 405.0/2.0*f[20] + (405.0/4.0)*f[24] - 45.0/2.0*f[28] - 99*f[32] + 162*f[36] - 81*f[40] + 18*f[44] + (99.0/4.0)*f[48] + 81*f[4] - 81.0/2.0*f[52] + (81.0/4.0)*f[56] - 9.0/2.0*f[60] - 81.0/2.0*f[8] + z*((99.0/4.0)*f[0] - 9.0/2.0*f[12] - 297.0/4.0*f[16] + (243.0/2.0)*f[20] - 243.0/4.0*f[24] + (27.0/2.0)*f[28] + (297.0/4.0)*f[32] - 243.0/2.0*f[36] + (243.0/4.0)*f[40] - 27.0/2.0*f[44] - 99.0/4.0*f[48] - 81.0/2.0*f[4] + (81.0/2.0)*f[52] - 81.0/4.0*f[56] + (9.0/2.0)*f[60] + (81.0/4.0)*f[8])))) + z*(-11.0/2.0*f[0] + 9*f[16] - 9.0/2.0*f[32] + f[48] + z*(9*f[0] - 45.0/2.0*f[16] + 18*f[32] - 9.0/2.0*f[48] + z*(-9.0/2.0*f[0] + (27.0/2.0)*f[16] - 27.0/2.0*f[32] + (9.0/2.0)*f[48])));
}
#endif
#endif

#ifdef ET_HEX
#if ORDER==3
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[64];
  int ii=0;
  for (int i=0; i<=3; i++) {
    for (int j=0; j<=3; j++) {
      for (int k=0; k<=3; k++) {
        f[ii] = getValue(coefficients, first+k+j*dy+i*dz).xyz;
        ii++;
      }
    }
  }
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3] + y*((99.0/4.0)*f[0] + (243.0/4.0)*f[10] - 81.0/4.0*f[11] - 9.0/2.0*f[12] + (27.0/2.0)*f[13] - 27.0/2.0*f[14] + (9.0/2.0)*f[15] - 297.0/4.0*f[1] + (297.0/4.0)*f[2] - 99.0/4.0*f[3] - 81.0/2.0*f[4] + (243.0/2.0)*f[5] - 243.0/2.0*f[6] + (81.0/2.0)*f[7] + (81.0/4.0)*f[8] - 243.0/4.0*f[9] + y*(-81.0/2.0*f[0] - 243*f[10] + 81*f[11] + (81.0/4.0)*f[12] - 243.0/4.0*f[13] + (243.0/4.0)*f[14] - 81.0/4.0*f[15] + (243.0/2.0)*f[1] - 243.0/2.0*f[2] + (81.0/2.0)*f[3] + (405.0/4.0)*f[4] - 1215.0/4.0*f[5] + (1215.0/4.0)*f[6] - 405.0/4.0*f[7] - 81*f[8] + 243*f[9] + y*((81.0/4.0)*f[0] + (729.0/4.0)*f[10] - 243.0/4.0*f[11] - 81.0/4.0*f[12] + (243.0/4.0)*f[13] - 243.0/4.0*f[14] + (81.0/4.0)*f[15] - 243.0/4.0*f[1] + (243.0/4.0)*f[2] - 81.0/4.0*f[3] - 243.0/4.0*f[4] + (729.0/4.0)*f[5] - 729.0/4.0*f[6] + (243.0/4.0)*f[7] + (243.0/4.0)*f[8] - 729.0/4.0*f[9] + z*(-891.0/8.0*f[0] - 8019.0/8.0*f[10] + (2673.0/8.0)*f[11] + (891.0/8.0)*f[12] - 2673.0/8.0*f[13] + (2673.0/8.0)*f[14] - 891.0/8.0*f[15] + (729.0/4.0)*f[16] - 2187.0/4.0*f[17] + (2187.0/4.0)*f[18] - 729.0/4.0*f[19] + (2673.0/8.0)*f[1] - 2187.0/4.0*f[20] + (6561.0/4.0)*f[21] - 6561.0/4.0*f[22] + (2187.0/4.0)*f[23] + (2187.0/4.0)*f[24] - 6561.0/4.0*f[25] + (6561.0/4.0)*f[26] - 2187.0/4.0*f[27] - 729.0/4.0*f[28] + (2187.0/4.0)*f[29] - 2673.0/8.0*f[2] - 2187.0/4.0*f[30] + (729.0/4.0)*f[31] - 729.0/8.0*f[32] + (2187.0/8.0)*f[33] - 2187.0/8.0*f[34] + (729.0/8.0)*f[35] + (2187.0/8.0)*f[36] - 6561.0/8.0*f[37] + (6561.0/8.0)*f[38] - 2187.0/8.0*f[39] + (891.0/8.0)*f[3] - 2187.0/8.0*f[40] + (6561.0/8.0)*f[41] - 6561.0/8.0*f[42] + (2187.0/8.0)*f[43] + (729.0/8.0)*f[44] - 2187.0/8.0*f[45] + (2187.0/8.0)*f[46] - 729.0/8.0*f[47] + (81.0/4.0)*f[48] - 243.0/4.0*f[49] + (2673.0/8.0)*f[4] + (243.0/4.0)*f[50] - 81.0/4.0*f[51] - 243.0/4.0*f[52] + (729.0/4.0)*f[53] - 729.0/4.0*f[54] + (243.0/4.0)*f[55] + (243.0/4.0)*f[56] - 729.0/4.0*f[57] + (729.0/4.0)*f[58] - 243.0/4.0*f[59] - 8019.0/8.0*f[5] - 81.0/4.0*f[60] + (243.0/4.0)*f[61] - 243.0/4.0*f[62] + (81.0/4.0)*f[63] + (8019.0/8.0)*f[6] - 2673.0/8.0*f[7] - 2673.0/8.0*f[8] + (8019.0/8.0)*f[9] + z*((729.0/4.0)*f[0] + (6561.0/4.0)*f[10] - 2187.0/4.0*f[11] - 729.0/4.0*f[12] + (2187.0/4.0)*f[13] - 2187.0/4.0*f[14] + (729.0/4.0)*f[15] - 3645.0/8.0*f[16] + (10935.0/8.0)*f[17] - 10935.0/8.0*f[18] + (3645.0/8.0)*f[19] - 2187.0/4.0*f[1] + (10935.0/8.0)*f[20] - 32805.0/8.0*f[21] + (32805.0/8.0)*f[22] - 10935.0/8.0*f[23] - 10935.0/8.0*f[24] + (32805.0/8.0)*f[25] - 32805.0/8.0*f[26] + (10935.0/8.0)*f[27] + (3645.0/8.0)*f[28] - 10935.0/8.0*f[29] + (2187.0/4.0)*f[2] + (10935.0/8.0)*f[30] - 3645.0/8.0*f[31] + (729.0/2.0)*f[32] - 2187.0/2.0*f[33] + (2187.0/2.0)*f[34] - 729.0/2.0*f[35] - 2187.0/2.0*f[36] + (6561.0/2.0)*f[37] - 6561.0/2.0*f[38] + (2187.0/2.0)*f[39] - 729.0/4.0*f[3] + (2187.0/2.0)*f[40] - 6561.0/2.0*f[41] + (6561.0/2.0)*f[42] - 2187.0/2.0*f[43] - 729.0/2.0*f[44] + (2187.0/2.0)*f[45] - 2187.0/2.0*f[46] + (729.0/2.0)*f[47] - 729.0/8.0*f[48] + (2187.0/8.0)*f[49] - 2187.0/4.0*f[4] - 2187.0/8.0*f[50] + (729.0/8.0)*f[51] + (2187.0/8.0)*f[52] - 6561.0/8.0*f[53] + (6561.0/8.0)*f[54] - 2187.0/8.0*f[55] - 2187.0/8.0*f[56] + (6561.0/8.0)*f[57] - 6561.0/8.0*f[58] + (2187.0/8.0)*f[59] + (6561.0/4.0)*f[5] + (729.0/8.0)*f[60] - 2187.0/8.0*f[61] + (2187.0/8.0)*f[62] - 729.0/8.0*f[63] - 6561.0/4.0*f[6] + (2187.0/4.0)*f[7] + (2187.0/4.0)*f[8] - 6561.0/4.0*f[9] + z*(-729.0/8.0*f[0] - 6561.0/8.0*f[10] + (2187.0/8.0)*f[11] + (729.0/8.0)*f[12] - 2187.0/8.0*f[13] + (2187.0/8.0)*f[14] - 729.0/8.0*f[15] + (2187.0/8.0)*f[16] - 6561.0/8.0*f[17] + (6561.0/8.0)*f[18] - 2187.0/8.0*f[19] + (2187.0/8.0)*f[1] - 6561.0/8.0*f[20] + (19683.0/8.0)*f[21] - 19683.0/8.0*f[22] + (6561.0/8.0)*f[23] + (6561.0/8.0)*f[24] - 19683.0/8.0*f[25] + (19683.0/8.0)*f[26] - 6561.0/8.0*f[27] - 2187.0/8.0*f[28] + (6561.0/8.0)*f[29] - 2187.0/8.0*f[2] - 6561.0/8.0*f[30] + (2187.0/8.0)*f[31] - 2187.0/8.0*f[32] + (6561.0/8.0)*f[33] - 6561.0/8.0*f[34] + (2187.0/8.0)*f[35] + (6561.0/8.0)*f[36] - 19683.0/8.0*f[37] + (19683.0/8.0)*f[38] - 6561.0/8.0*f[39] + (729.0/8.0)*f[3] - 6561.0/8.0*f[40] + (19683.0/8.0)*f[41] - 19683.0/8.0*f[42] + (6561.0/8.0)*f[43] + (2187.0/8.0)*f[44] - 6561.0/8.0*f[45] + (6561.0/8.0)*f[46] - 2187.0/8.0*f[47] + (729.0/8.0)*f[48] - 2187.0/8.0*f[49] + (2187.0/8.0)*f[4] + (2187.0/8.0)*f[50] - 729.0/8.0*f[51] - 2187.0/8.0*f[52] + (6561.0/8.0)*f[53] - 6561.0/8.0*f[54] + (2187.0/8.0)*f[55] + (2187.0/8.0)*f[56] - 6561.0/8.0*f[57] + (6561.0/8.0)*f[58] - 2187.0/8.0*f[59] - 6561.0/8.0*f[5] - 729.0/8.0*f[60] + (2187.0/8.0)*f[61] - 2187.0/8.0*f[62] + (729.0/8.0)*f[63] + (6561.0/8.0)*f[6] - 2187.0/8.0*f[7] - 2187.0/8.0*f[8] + (6561.0/8.0)*f[9])))) + z*((891.0/4.0)*f[0] + (2673.0/2.0)*f[10] - 891.0/2.0*f[11] - 891.0/8.0*f[12] + (2673.0/8.0)*f[13] - 2673.0/8.0*f[14] + (891.0/8.0)*f[15] - 729.0/2.0*f[16] + (2187.0/2.0)*f[17] - 2187.0/2.0*f[18] + (729.0/2.0)*f[19] - 2673.0/4.0*f[1] + (3645.0/4.0)*f[20] - 10935.0/4.0*f[21] + (10935.0/4.0)*f[22] - 3645.0/4.0*f[23] - 729*f[24] + 2187*f[25] - 2187*f[26] + 729*f[27] + (729.0/4.0)*f[28] - 2187.0/4.0*f[29] + (2673.0/4.0)*f[2] + (2187.0/4.0)*f[30] - 729.0/4.0*f[31] + (729.0/4.0)*f[32] - 2187.0/4.0*f[33] + (2187.0/4.0)*f[34] - 729.0/4.0*f[35] - 3645.0/8.0*f[36] + (10935.0/8.0)*f[37] - 10935.0/8.0*f[38] + (3645.0/8.0)*f[39] - 891.0/4.0*f[3] + (729.0/2.0)*f[40] - 2187.0/2.0*f[41] + (2187.0/2.0)*f[42] - 729.0/2.0*f[43] - 729.0/8.0*f[44] + (2187.0/8.0)*f[45] - 2187.0/8.0*f[46] + (729.0/8.0)*f[47] - 81.0/2.0*f[48] + (243.0/2.0)*f[49] - 4455.0/8.0*f[4] - 243.0/2.0*f[50] + (81.0/2.0)*f[51] + (405.0/4.0)*f[52] - 1215.0/4.0*f[53] + (1215.0/4.0)*f[54] - 405.0/4.0*f[55] - 81*f[56] + 243*f[57] - 243*f[58] + 81*f[59] + (13365.0/8.0)*f[5] + (81.0/4.0)*f[60] - 243.0/4.0*f[61] + (243.0/4.0)*f[62] - 81.0/4.0*f[63] - 13365.0/8.0*f[6] + (4455.0/8.0)*f[7] + (891.0/2.0)*f[8] - 2673.0/2.0*f[9] + z*(-729.0/2.0*f[0] - 2187*f[10] + 729*f[11] + (729.0/4.0)*f[12] - 2187.0/4.0*f[13] + (2187.0/4.0)*f[14] - 729.0/4.0*f[15] + (3645.0/4.0)*f[16] - 10935.0/4.0*f[17] + (10935.0/4.0)*f[18] - 3645.0/4.0*f[19] + (2187.0/2.0)*f[1] - 18225.0/8.0*f[20] + (54675.0/8.0)*f[21] - 54675.0/8.0*f[22] + (18225.0/8.0)*f[23] + (3645.0/2.0)*f[24] - 10935.0/2.0*f[25] + (10935.0/2.0)*f[26] - 3645.0/2.0*f[27] - 3645.0/8.0*f[28] + (10935.0/8.0)*f[29] - 2187.0/2.0*f[2] - 10935.0/8.0*f[30] + (3645.0/8.0)*f[31] - 729*f[32] + 2187*f[33] - 2187*f[34] + 729*f[35] + (3645.0/2.0)*f[36] - 10935.0/2.0*f[37] + (10935.0/2.0)*f[38] - 3645.0/2.0*f[39] + (729.0/2.0)*f[3] - 1458*f[40] + 4374*f[41] - 4374*f[42] + 1458*f[43] + (729.0/2.0)*f[44] - 2187.0/2.0*f[45] + (2187.0/2.0)*f[46] - 729.0/2.0*f[47] + (729.0/4.0)*f[48] - 2187.0/4.0*f[49] + (3645.0/4.0)*f[4] + (2187.0/4.0)*f[50] - 729.0/4.0*f[51] - 3645.0/8.0*f[52] + (10935.0/8.0)*f[53] - 10935.0/8.0*f[54] + (3645.0/8.0)*f[55] + (729.0/2.0)*f[56] - 2187.0/2.0*f[57] + (2187.0/2.0)*f[58] - 729.0/2.0*f[59] - 10935.0/4.0*f[5] - 729.0/8.0*f[60] + (2187.0/8.0)*f[61] - 2187.0/8.0*f[62] + (729.0/8.0)*f[63] + (10935.0/4.0)*f[6] - 3645.0/4.0*f[7] - 729*f[8] + 2187*f[9] + z*((729.0/4.0)*f[0] + (2187.0/2.0)*f[10] - 729.0/2.0*f[11] - 729.0/8.0*f[12] + (2187.0/8.0)*f[13] - 2187.0/8.0*f[14] + (729.0/8.0)*f[15] - 2187.0/4.0*f[16] + (6561.0/4.0)*f[17] - 6561.0/4.0*f[18] + (2187.0/4.0)*f[19] - 2187.0/4.0*f[1] + (10935.0/8.0)*f[20] - 32805.0/8.0*f[21] + (32805.0/8.0)*f[22] - 10935.0/8.0*f[23] - 2187.0/2.0*f[24] + (6561.0/2.0)*f[25] - 6561.0/2.0*f[26] + (2187.0/2.0)*f[27] + (2187.0/8.0)*f[28] - 6561.0/8.0*f[29] + (2187.0/4.0)*f[2] + (6561.0/8.0)*f[30] - 2187.0/8.0*f[31] + (2187.0/4.0)*f[32] - 6561.0/4.0*f[33] + (6561.0/4.0)*f[34] - 2187.0/4.0*f[35] - 10935.0/8.0*f[36] + (32805.0/8.0)*f[37] - 32805.0/8.0*f[38] + (10935.0/8.0)*f[39] - 729.0/4.0*f[3] + (2187.0/2.0)*f[40] - 6561.0/2.0*f[41] + (6561.0/2.0)*f[42] - 2187.0/2.0*f[43] - 2187.0/8.0*f[44] + (6561.0/8.0)*f[45] - 6561.0/8.0*f[46] + (2187.0/8.0)*f[47] - 729.0/4.0*f[48] + (2187.0/4.0)*f[49] - 3645.0/8.0*f[4] - 2187.0/4.0*f[50] + (729.0/4.0)*f[51] + (3645.0/8.0)*f[52] - 10935.0/8.0*f[53] + (10935.0/8.0)*f[54] - 3645.0/8.0*f[55] - 729.0/2.0*f[56] + (2187.0/2.0)*f[57] - 2187.0/2.0*f[58] + (729.0/2.0)*f[59] + (10935.0/8.0)*f[5] + (729.0/8.0)*f[60] - 2187.0/8.0*f[61] + (2187.0/8.0)*f[62] - 729.0/8.0*f[63] - 10935.0/8.0*f[6] + (3645.0/8.0)*f[7] + (729.0/2.0)*f[8] - 2187.0/2.0*f[9])))) + z*(-1089.0/8.0*f[0] - 2673.0/8.0*f[10] + (891.0/8.0)*f[11] + (99.0/4.0)*f[12] - 297.0/4.0*f[13] + (297.0/4.0)*f[14] - 99.0/4.0*f[15] + (891.0/4.0)*f[16] - 2673.0/4.0*f[17] + (2673.0/4.0)*f[18] - 891.0/4.0*f[19] + (3267.0/8.0)*f[1] - 729.0/2.0*f[20] + (2187.0/2.0)*f[21] - 2187.0/2.0*f[22] + (729.0/2.0)*f[23] + (729.0/4.0)*f[24] - 2187.0/4.0*f[25] + (2187.0/4.0)*f[26] - 729.0/4.0*f[27] - 81.0/2.0*f[28] + (243.0/2.0)*f[29] - 3267.0/8.0*f[2] - 243.0/2.0*f[30] + (81.0/2.0)*f[31] - 891.0/8.0*f[32] + (2673.0/8.0)*f[33] - 2673.0/8.0*f[34] + (891.0/8.0)*f[35] + (729.0/4.0)*f[36] - 2187.0/4.0*f[37] + (2187.0/4.0)*f[38] - 729.0/4.0*f[39] + (1089.0/8.0)*f[3] - 729.0/8.0*f[40] + (2187.0/8.0)*f[41] - 2187.0/8.0*f[42] + (729.0/8.0)*f[43] + (81.0/4.0)*f[44] - 243.0/4.0*f[45] + (243.0/4.0)*f[46] - 81.0/4.0*f[47] + (99.0/4.0)*f[48] - 297.0/4.0*f[49] + (891.0/4.0)*f[4] + (297.0/4.0)*f[50] - 99.0/4.0*f[51] - 81.0/2.0*f[52] + (243.0/2.0)*f[53] - 243.0/2.0*f[54] + (81.0/2.0)*f[55] + (81.0/4.0)*f[56] - 243.0/4.0*f[57] + (243.0/4.0)*f[58] - 81.0/4.0*f[59] - 2673.0/4.0*f[5] - 9.0/2.0*f[60] + (27.0/2.0)*f[61] - 27.0/2.0*f[62] + (9.0/2.0)*f[63] + (2673.0/4.0)*f[6] - 891.0/4.0*f[7] - 891.0/8.0*f[8] + (2673.0/8.0)*f[9] + z*((891.0/4.0)*f[0] + (2187.0/4.0)*f[10] - 729.0/4.0*f[11] - 81.0/2.0*f[12] + (243.0/2.0)*f[13] - 243.0/2.0*f[14] + (81.0/2.0)*f[15] - 4455.0/8.0*f[16] + (13365.0/8.0)*f[17] - 13365.0/8.0*f[18] + (4455.0/8.0)*f[19] - 2673.0/4.0*f[1] + (3645.0/4.0)*f[20] - 10935.0/4.0*f[21] + (10935.0/4.0)*f[22] - 3645.0/4.0*f[23] - 3645.0/8.0*f[24] + (10935.0/8.0)*f[25] - 10935.0/8.0*f[26] + (3645.0/8.0)*f[27] + (405.0/4.0)*f[28] - 1215.0/4.0*f[29] + (2673.0/4.0)*f[2] + (1215.0/4.0)*f[30] - 405.0/4.0*f[31] + (891.0/2.0)*f[32] - 2673.0/2.0*f[33] + (2673.0/2.0)*f[34] - 891.0/2.0*f[35] - 729*f[36] + 2187*f[37] - 2187*f[38] + 729*f[39] - 891.0/4.0*f[3] + (729.0/2.0)*f[40] - 2187.0/2.0*f[41] + (2187.0/2.0)*f[42] - 729.0/2.0*f[43] - 81*f[44] + 243*f[45] - 243*f[46] + 81*f[47] - 891.0/8.0*f[48] + (2673.0/8.0)*f[49] - 729.0/2.0*f[4] - 2673.0/8.0*f[50] + (891.0/8.0)*f[51] + (729.0/4.0)*f[52] - 2187.0/4.0*f[53] + (2187.0/4.0)*f[54] - 729.0/4.0*f[55] - 729.0/8.0*f[56] + (2187.0/8.0)*f[57] - 2187.0/8.0*f[58] + (729.0/8.0)*f[59] + (2187.0/2.0)*f[5] + (81.0/4.0)*f[60] - 243.0/4.0*f[61] + (243.0/4.0)*f[62] - 81.0/4.0*f[63] - 2187.0/2.0*f[6] + (729.0/2.0)*f[7] + (729.0/4.0)*f[8] - 2187.0/4.0*f[9] + z*(-891.0/8.0*f[0] - 2187.0/8.0*f[10] + (729.0/8.0)*f[11] + (81.0/4.0)*f[12] - 243.0/4.0*f[13] + (243.0/4.0)*f[14] - 81.0/4.0*f[15] + (2673.0/8.0)*f[16] - 8019.0/8.0*f[17] + (8019.0/8.0)*f[18] - 2673.0/8.0*f[19] + (2673.0/8.0)*f[1] - 2187.0/4.0*f[20] + (6561.0/4.0)*f[21] - 6561.0/4.0*f[22] + (2187.0/4.0)*f[23] + (2187.0/8.0)*f[24] - 6561.0/8.0*f[25] + (6561.0/8.0)*f[26] - 2187.0/8.0*f[27] - 243.0/4.0*f[28] + (729.0/4.0)*f[29] - 2673.0/8.0*f[2] - 729.0/4.0*f[30] + (243.0/4.0)*f[31] - 2673.0/8.0*f[32] + (8019.0/8.0)*f[33] - 8019.0/8.0*f[34] + (2673.0/8.0)*f[35] + (2187.0/4.0)*f[36] - 6561.0/4.0*f[37] + (6561.0/4.0)*f[38] - 2187.0/4.0*f[39] + (891.0/8.0)*f[3] - 2187.0/8.0*f[40] + (6561.0/8.0)*f[41] - 6561.0/8.0*f[42] + (2187.0/8.0)*f[43] + (243.0/4.0)*f[44] - 729.0/4.0*f[45] + (729.0/4.0)*f[46] - 243.0/4.0*f[47] + (891.0/8.0)*f[48] - 2673.0/8.0*f[49] + (729.0/4.0)*f[4] + (2673.0/8.0)*f[50] - 891.0/8.0*f[51] - 729.0/4.0*f[52] + (2187.0/4.0)*f[53] - 2187.0/4.0*f[54] + (729.0/4.0)*f[55] + (729.0/8.0)*f[56] - 2187.0/8.0*f[57] + (2187.0/8.0)*f[58] - 729.0/8.0*f[59] - 2187.0/4.0*f[5] - 81.0/4.0*f[60] + (243.0/4.0)*f[61] - 243.0/4.0*f[62] + (81.0/4.0)*f[63] + (2187.0/4.0)*f[6] - 729.0/4.0*f[7] - 729.0/8.0*f[8] + (2187.0/8.0)*f[9])))) + z*((99.0/4.0)*f[0] - 81.0/2.0*f[16] + (243.0/2.0)*f[17] - 243.0/2.0*f[18] + (81.0/2.0)*f[19] - 297.0/4.0*f[1] + (297.0/4.0)*f[2] + (81.0/4.0)*f[32] - 243.0/4.0*f[33] + (243.0/4.0)*f[34] - 81.0/4.0*f[35] - 99.0/4.0*f[3] - 9.0/2.0*f[48] + (27.0/2.0)*f[49] - 27.0/2.0*f[50] + (9.0/2.0)*f[51] + z*(-81.0/2.0*f[0] + (405.0/4.0)*f[16] - 1215.0/4.0*f[17] + (1215.0/4.0)*f[18] - 405.0/4.0*f[19] + (243.0/2.0)*f[1] - 243.0/2.0*f[2] - 81*f[32] + 243*f[33] - 243*f[34] + 81*f[35] + (81.0/2.0)*f[3] + (81.0/4.0)*f[48] - 243.0/4.0*f[49] + (243.0/4.0)*f[50] - 81.0/4.0*f[51] + z*((81.0/4.0)*f[0] - 243.0/4.0*f[16] + (729.0/4.0)*f[17] - 729.0/4.0*f[18] + (243.0/4.0)*f[19] - 243.0/4.0*f[1] + (243.0/4.0)*f[2] + (243.0/4.0)*f[32] - 729.0/4.0*f[33] + (729.0/4.0)*f[34] - 243.0/4.0*f[35] - 81.0/4.0*f[3] - 81.0/4.0*f[48] + (243.0/4.0)*f[49] - 243.0/4.0*f[50] + (81.0/4.0)*f[51])))) + y*(-99.0/2.0*f[0] - 81*f[10] + (81.0/4.0)*f[11] + 9*f[12] - 45.0/2.0*f[13] + 18*f[14] - 9.0/2.0*f[15] + (495.0/4.0)*f[1] - 99*f[2] + (99.0/4.0)*f[3] + 81*f[4] - 405.0/2.0*f[5] + 162*f[6] - 81.0/2.0*f[7] - 81.0/2.0*f[8] + (405.0/4.0)*f[9] + y*(81*f[0] + 324*f[10] - 81*f[11] - 81.0/2.0*f[12] + (405.0/4.0)*f[13] - 81*f[14] + (81.0/4.0)*f[15] - 405.0/2.0*f[1] + 162*f[2] - 81.0/2.0*f[3] - 405.0/2.0*f[4] + (2025.0/4.0)*f[5] - 405*f[6] + (405.0/4.0)*f[7] + 162*f[8] - 405*f[9] + y*(-81.0/2.0*f[0] - 243*f[10] + (243.0/4.0)*f[11] + (81.0/2.0)*f[12] - 405.0/4.0*f[13] + 81*f[14] - 81.0/4.0*f[15] + (405.0/4.0)*f[1] - 81*f[2] + (81.0/4.0)*f[3] + (243.0/2.0)*f[4] - 1215.0/4.0*f[5] + 243*f[6] - 243.0/4.0*f[7] - 243.0/2.0*f[8] + (1215.0/4.0)*f[9] + z*((891.0/4.0)*f[0] + (2673.0/2.0)*f[10] - 2673.0/8.0*f[11] - 891.0/4.0*f[12] + (4455.0/8.0)*f[13] - 891.0/2.0*f[14] + (891.0/8.0)*f[15] - 729.0/2.0*f[16] + (3645.0/4.0)*f[17] - 729*f[18] + (729.0/4.0)*f[19] - 4455.0/8.0*f[1] + (2187.0/2.0)*f[20] - 10935.0/4.0*f[21] + 2187*f[22] - 2187.0/4.0*f[23] - 2187.0/2.0*f[24] + (10935.0/4.0)*f[25] - 2187*f[26] + (2187.0/4.0)*f[27] + (729.0/2.0)*f[28] - 3645.0/4.0*f[29] + (891.0/2.0)*f[2] + 729*f[30] - 729.0/4.0*f[31] + (729.0/4.0)*f[32] - 3645.0/8.0*f[33] + (729.0/2.0)*f[34] - 729.0/8.0*f[35] - 2187.0/4.0*f[36] + (10935.0/8.0)*f[37] - 2187.0/2.0*f[38] + (2187.0/8.0)*f[39] - 891.0/8.0*f[3] + (2187.0/4.0)*f[40] - 10935.0/8.0*f[41] + (2187.0/2.0)*f[42] - 2187.0/8.0*f[43] - 729.0/4.0*f[44] + (3645.0/8.0)*f[45] - 729.0/2.0*f[46] + (729.0/8.0)*f[47] - 81.0/2.0*f[48] + (405.0/4.0)*f[49] - 2673.0/4.0*f[4] - 81*f[50] + (81.0/4.0)*f[51] + (243.0/2.0)*f[52] - 1215.0/4.0*f[53] + 243*f[54] - 243.0/4.0*f[55] - 243.0/2.0*f[56] + (1215.0/4.0)*f[57] - 243*f[58] + (243.0/4.0)*f[59] + (13365.0/8.0)*f[5] + (81.0/2.0)*f[60] - 405.0/4.0*f[61] + 81*f[62] - 81.0/4.0*f[63] - 2673.0/2.0*f[6] + (2673.0/8.0)*f[7] + (2673.0/4.0)*f[8] - 13365.0/8.0*f[9] + z*(-729.0/2.0*f[0] - 2187*f[10] + (2187.0/4.0)*f[11] + (729.0/2.0)*f[12] - 3645.0/4.0*f[13] + 729*f[14] - 729.0/4.0*f[15] + (3645.0/4.0)*f[16] - 18225.0/8.0*f[17] + (3645.0/2.0)*f[18] - 3645.0/8.0*f[19] + (3645.0/4.0)*f[1] - 10935.0/4.0*f[20] + (54675.0/8.0)*f[21] - 10935.0/2.0*f[22] + (10935.0/8.0)*f[23] + (10935.0/4.0)*f[24] - 54675.0/8.0*f[25] + (10935.0/2.0)*f[26] - 10935.0/8.0*f[27] - 3645.0/4.0*f[28] + (18225.0/8.0)*f[29] - 729*f[2] - 3645.0/2.0*f[30] + (3645.0/8.0)*f[31] - 729*f[32] + (3645.0/2.0)*f[33] - 1458*f[34] + (729.0/2.0)*f[35] + 2187*f[36] - 10935.0/2.0*f[37] + 4374*f[38] - 2187.0/2.0*f[39] + (729.0/4.0)*f[3] - 2187*f[40] + (10935.0/2.0)*f[41] - 4374*f[42] + (2187.0/2.0)*f[43] + 729*f[44] - 3645.0/2.0*f[45] + 1458*f[46] - 729.0/2.0*f[47] + (729.0/4.0)*f[48] - 3645.0/8.0*f[49] + (2187.0/2.0)*f[4] + (729.0/2.0)*f[50] - 729.0/8.0*f[51] - 2187.0/4.0*f[52] + (10935.0/8.0)*f[53] - 2187.0/2.0*f[54] + (2187.0/8.0)*f[55] + (2187.0/4.0)*f[56] - 10935.0/8.0*f[57] + (2187.0/2.0)*f[58] - 2187.0/8.0*f[59] - 10935.0/4.0*f[5] - 729.0/4.0*f[60] + (3645.0/8.0)*f[61] - 729.0/2.0*f[62] + (729.0/8.0)*f[63] + 2187*f[6] - 2187.0/4.0*f[7] - 2187.0/2.0*f[8] + (10935.0/4.0)*f[9] + z*((729.0/4.0)*f[0] + (2187.0/2.0)*f[10] - 2187.0/8.0*f[11] - 729.0/4.0*f[12] + (3645.0/8.0)*f[13] - 729.0/2.0*f[14] + (729.0/8.0)*f[15] - 2187.0/4.0*f[16] + (10935.0/8.0)*f[17] - 2187.0/2.0*f[18] + (2187.0/8.0)*f[19] - 3645.0/8.0*f[1] + (6561.0/4.0)*f[20] - 32805.0/8.0*f[21] + (6561.0/2.0)*f[22] - 6561.0/8.0*f[23] - 6561.0/4.0*f[24] + (32805.0/8.0)*f[25] - 6561.0/2.0*f[26] + (6561.0/8.0)*f[27] + (2187.0/4.0)*f[28] - 10935.0/8.0*f[29] + (729.0/2.0)*f[2] + (2187.0/2.0)*f[30] - 2187.0/8.0*f[31] + (2187.0/4.0)*f[32] - 10935.0/8.0*f[33] + (2187.0/2.0)*f[34] - 2187.0/8.0*f[35] - 6561.0/4.0*f[36] + (32805.0/8.0)*f[37] - 6561.0/2.0*f[38] + (6561.0/8.0)*f[39] - 729.0/8.0*f[3] + (6561.0/4.0)*f[40] - 32805.0/8.0*f[41] + (6561.0/2.0)*f[42] - 6561.0/8.0*f[43] - 2187.0/4.0*f[44] + (10935.0/8.0)*f[45] - 2187.0/2.0*f[46] + (2187.0/8.0)*f[47] - 729.0/4.0*f[48] + (3645.0/8.0)*f[49] - 2187.0/4.0*f[4] - 729.0/2.0*f[50] + (729.0/8.0)*f[51] + (2187.0/4.0)*f[52] - 10935.0/8.0*f[53] + (2187.0/2.0)*f[54] - 2187.0/8.0*f[55] - 2187.0/4.0*f[56] + (10935.0/8.0)*f[57] - 2187.0/2.0*f[58] + (2187.0/8.0)*f[59] + (10935.0/8.0)*f[5] + (729.0/4.0)*f[60] - 3645.0/8.0*f[61] + (729.0/2.0)*f[62] - 729.0/8.0*f[63] - 2187.0/2.0*f[6] + (2187.0/8.0)*f[7] + (2187.0/4.0)*f[8] - 10935.0/8.0*f[9])))) + z*(-891.0/2.0*f[0] - 1782*f[10] + (891.0/2.0)*f[11] + (891.0/4.0)*f[12] - 4455.0/8.0*f[13] + (891.0/2.0)*f[14] - 891.0/8.0*f[15] + 729*f[16] - 3645.0/2.0*f[17] + 1458*f[18] - 729.0/2.0*f[19] + (4455.0/4.0)*f[1] - 3645.0/2.0*f[20] + (18225.0/4.0)*f[21] - 3645*f[22] + (3645.0/4.0)*f[23] + 1458*f[24] - 3645*f[25] + 2916*f[26] - 729*f[27] - 729.0/2.0*f[28] + (3645.0/4.0)*f[29] - 891*f[2] - 729*f[30] + (729.0/4.0)*f[31] - 729.0/2.0*f[32] + (3645.0/4.0)*f[33] - 729*f[34] + (729.0/4.0)*f[35] + (3645.0/4.0)*f[36] - 18225.0/8.0*f[37] + (3645.0/2.0)*f[38] - 3645.0/8.0*f[39] + (891.0/4.0)*f[3] - 729*f[40] + (3645.0/2.0)*f[41] - 1458*f[42] + (729.0/2.0)*f[43] + (729.0/4.0)*f[44] - 3645.0/8.0*f[45] + (729.0/2.0)*f[46] - 729.0/8.0*f[47] + 81*f[48] - 405.0/2.0*f[49] + (4455.0/4.0)*f[4] + 162*f[50] - 81.0/2.0*f[51] - 405.0/2.0*f[52] + (2025.0/4.0)*f[53] - 405*f[54] + (405.0/4.0)*f[55] + 162*f[56] - 405*f[57] + 324*f[58] - 81*f[59] - 22275.0/8.0*f[5] - 81.0/2.0*f[60] + (405.0/4.0)*f[61] - 81*f[62] + (81.0/4.0)*f[63] + (4455.0/2.0)*f[6] - 4455.0/8.0*f[7] - 891*f[8] + (4455.0/2.0)*f[9] + z*(729*f[0] + 2916*f[10] - 729*f[11] - 729.0/2.0*f[12] + (3645.0/4.0)*f[13] - 729*f[14] + (729.0/4.0)*f[15] - 3645.0/2.0*f[16] + (18225.0/4.0)*f[17] - 3645*f[18] + (3645.0/4.0)*f[19] - 3645.0/2.0*f[1] + (18225.0/4.0)*f[20] - 91125.0/8.0*f[21] + (18225.0/2.0)*f[22] - 18225.0/8.0*f[23] - 3645*f[24] + (18225.0/2.0)*f[25] - 7290*f[26] + (3645.0/2.0)*f[27] + (3645.0/4.0)*f[28] - 18225.0/8.0*f[29] + 1458*f[2] + (3645.0/2.0)*f[30] - 3645.0/8.0*f[31] + 1458*f[32] - 3645*f[33] + 2916*f[34] - 729*f[35] - 3645*f[36] + (18225.0/2.0)*f[37] - 7290*f[38] + (3645.0/2.0)*f[39] - 729.0/2.0*f[3] + 2916*f[40] - 7290*f[41] + 5832*f[42] - 1458*f[43] - 729*f[44] + (3645.0/2.0)*f[45] - 1458*f[46] + (729.0/2.0)*f[47] - 729.0/2.0*f[48] + (3645.0/4.0)*f[49] - 3645.0/2.0*f[4] - 729*f[50] + (729.0/4.0)*f[51] + (3645.0/4.0)*f[52] - 18225.0/8.0*f[53] + (3645.0/2.0)*f[54] - 3645.0/8.0*f[55] - 729*f[56] + (3645.0/2.0)*f[57] - 1458*f[58] + (729.0/2.0)*f[59] + (18225.0/4.0)*f[5] + (729.0/4.0)*f[60] - 3645.0/8.0*f[61] + (729.0/2.0)*f[62] - 729.0/8.0*f[63] - 3645*f[6] + (3645.0/4.0)*f[7] + 1458*f[8] - 3645*f[9] + z*(-729.0/2.0*f[0] - 1458*f[10] + (729.0/2.0)*f[11] + (729.0/4.0)*f[12] - 3645.0/8.0*f[13] + (729.0/2.0)*f[14] - 729.0/8.0*f[15] + (2187.0/2.0)*f[16] - 10935.0/4.0*f[17] + 2187*f[18] - 2187.0/4.0*f[19] + (3645.0/4.0)*f[1] - 10935.0/4.0*f[20] + (54675.0/8.0)*f[21] - 10935.0/2.0*f[22] + (10935.0/8.0)*f[23] + 2187*f[24] - 10935.0/2.0*f[25] + 4374*f[26] - 2187.0/2.0*f[27] - 2187.0/4.0*f[28] + (10935.0/8.0)*f[29] - 729*f[2] - 2187.0/2.0*f[30] + (2187.0/8.0)*f[31] - 2187.0/2.0*f[32] + (10935.0/4.0)*f[33] - 2187*f[34] + (2187.0/4.0)*f[35] + (10935.0/4.0)*f[36] - 54675.0/8.0*f[37] + (10935.0/2.0)*f[38] - 10935.0/8.0*f[39] + (729.0/4.0)*f[3] - 2187*f[40] + (10935.0/2.0)*f[41] - 4374*f[42] + (2187.0/2.0)*f[43] + (2187.0/4.0)*f[44] - 10935.0/8.0*f[45] + (2187.0/2.0)*f[46] - 2187.0/8.0*f[47] + (729.0/2.0)*f[48] - 3645.0/4.0*f[49] + (3645.0/4.0)*f[4] + 729*f[50] - 729.0/4.0*f[51] - 3645.0/4.0*f[52] + (18225.0/8.0)*f[53] - 3645.0/2.0*f[54] + (3645.0/8.0)*f[55] + 729*f[56] - 3645.0/2.0*f[57] + 1458*f[58] - 729.0/2.0*f[59] - 18225.0/8.0*f[5] - 729.0/4.0*f[60] + (3645.0/8.0)*f[61] - 729.0/2.0*f[62] + (729.0/8.0)*f[63] + (3645.0/2.0)*f[6] - 3645.0/8.0*f[7] - 729*f[8] + (3645.0/2.0)*f[9])))) + z*((1089.0/4.0)*f[0] + (891.0/2.0)*f[10] - 891.0/8.0*f[11] - 99.0/2.0*f[12] + (495.0/4.0)*f[13] - 99*f[14] + (99.0/4.0)*f[15] - 891.0/2.0*f[16] + (4455.0/4.0)*f[17] - 891*f[18] + (891.0/4.0)*f[19] - 5445.0/8.0*f[1] + 729*f[20] - 3645.0/2.0*f[21] + 1458*f[22] - 729.0/2.0*f[23] - 729.0/2.0*f[24] + (3645.0/4.0)*f[25] - 729*f[26] + (729.0/4.0)*f[27] + 81*f[28] - 405.0/2.0*f[29] + (1089.0/2.0)*f[2] + 162*f[30] - 81.0/2.0*f[31] + (891.0/4.0)*f[32] - 4455.0/8.0*f[33] + (891.0/2.0)*f[34] - 891.0/8.0*f[35] - 729.0/2.0*f[36] + (3645.0/4.0)*f[37] - 729*f[38] + (729.0/4.0)*f[39] - 1089.0/8.0*f[3] + (729.0/4.0)*f[40] - 3645.0/8.0*f[41] + (729.0/2.0)*f[42] - 729.0/8.0*f[43] - 81.0/2.0*f[44] + (405.0/4.0)*f[45] - 81*f[46] + (81.0/4.0)*f[47] - 99.0/2.0*f[48] + (495.0/4.0)*f[49] - 891.0/2.0*f[4] - 99*f[50] + (99.0/4.0)*f[51] + 81*f[52] - 405.0/2.0*f[53] + 162*f[54] - 81.0/2.0*f[55] - 81.0/2.0*f[56] + (405.0/4.0)*f[57] - 81*f[58] + (81.0/4.0)*f[59] + (4455.0/4.0)*f[5] + 9*f[60] - 45.0/2.0*f[61] + 18*f[62] - 9.0/2.0*f[63] - 891*f[6] + (891.0/4.0)*f[7] + (891.0/4.0)*f[8] - 4455.0/8.0*f[9] + z*(-891.0/2.0*f[0] - 729*f[10] + (729.0/4.0)*f[11] + 81*f[12] - 405.0/2.0*f[13] + 162*f[14] - 81.0/2.0*f[15] + (4455.0/4.0)*f[16] - 22275.0/8.0*f[17] + (4455.0/2.0)*f[18] - 4455.0/8.0*f[19] + (4455.0/4.0)*f[1] - 3645.0/2.0*f[20] + (18225.0/4.0)*f[21] - 3645*f[22] + (3645.0/4.0)*f[23] + (3645.0/4.0)*f[24] - 18225.0/8.0*f[25] + (3645.0/2.0)*f[26] - 3645.0/8.0*f[27] - 405.0/2.0*f[28] + (2025.0/4.0)*f[29] - 891*f[2] - 405*f[30] + (405.0/4.0)*f[31] - 891*f[32] + (4455.0/2.0)*f[33] - 1782*f[34] + (891.0/2.0)*f[35] + 1458*f[36] - 3645*f[37] + 2916*f[38] - 729*f[39] + (891.0/4.0)*f[3] - 729*f[40] + (3645.0/2.0)*f[41] - 1458*f[42] + (729.0/2.0)*f[43] + 162*f[44] - 405*f[45] + 324*f[46] - 81*f[47] + (891.0/4.0)*f[48] - 4455.0/8.0*f[49] + 729*f[4] + (891.0/2.0)*f[50] - 891.0/8.0*f[51] - 729.0/2.0*f[52] + (3645.0/4.0)*f[53] - 729*f[54] + (729.0/4.0)*f[55] + (729.0/4.0)*f[56] - 3645.0/8.0*f[57] + (729.0/2.0)*f[58] - 729.0/8.0*f[59] - 3645.0/2.0*f[5] - 81.0/2.0*f[60] + (405.0/4.0)*f[61] - 81*f[62] + (81.0/4.0)*f[63] + 1458*f[6] - 729.0/2.0*f[7] - 729.0/2.0*f[8] + (3645.0/4.0)*f[9] + z*((891.0/4.0)*f[0] + (729.0/2.0)*f[10] - 729.0/8.0*f[11] - 81.0/2.0*f[12] + (405.0/4.0)*f[13] - 81*f[14] + (81.0/4.0)*f[15] - 2673.0/4.0*f[16] + (13365.0/8.0)*f[17] - 2673.0/2.0*f[18] + (2673.0/8.0)*f[19] - 4455.0/8.0*f[1] + (2187.0/2.0)*f[20] - 10935.0/4.0*f[21] + 2187*f[22] - 2187.0/4.0*f[23] - 2187.0/4.0*f[24] + (10935.0/8.0)*f[25] - 2187.0/2.0*f[26] + (2187.0/8.0)*f[27] + (243.0/2.0)*f[28] - 1215.0/4.0*f[29] + (891.0/2.0)*f[2] + 243*f[30] - 243.0/4.0*f[31] + (2673.0/4.0)*f[32] - 13365.0/8.0*f[33] + (2673.0/2.0)*f[34] - 2673.0/8.0*f[35] - 2187.0/2.0*f[36] + (10935.0/4.0)*f[37] - 2187*f[38] + (2187.0/4.0)*f[39] - 891.0/8.0*f[3] + (2187.0/4.0)*f[40] - 10935.0/8.0*f[41] + (2187.0/2.0)*f[42] - 2187.0/8.0*f[43] - 243.0/2.0*f[44] + (1215.0/4.0)*f[45] - 243*f[46] + (243.0/4.0)*f[47] - 891.0/4.0*f[48] + (4455.0/8.0)*f[49] - 729.0/2.0*f[4] - 891.0/2.0*f[50] + (891.0/8.0)*f[51] + (729.0/2.0)*f[52] - 3645.0/4.0*f[53] + 729*f[54] - 729.0/4.0*f[55] - 729.0/4.0*f[56] + (3645.0/8.0)*f[57] - 729.0/2.0*f[58] + (729.0/8.0)*f[59] + (3645.0/4.0)*f[5] + (81.0/2.0)*f[60] - 405.0/4.0*f[61] + 81*f[62] - 81.0/4.0*f[63] - 729*f[6] + (729.0/4.0)*f[7] + (729.0/4.0)*f[8] - 3645.0/8.0*f[9])))) + z*(-99.0/2.0*f[0] + 81*f[16] - 405.0/2.0*f[17] + 162*f[18] - 81.0/2.0*f[19] + (495.0/4.0)*f[1] - 99*f[2] - 81.0/2.0*f[32] + (405.0/4.0)*f[33] - 81*f[34] + (81.0/4.0)*f[35] + (99.0/4.0)*f[3] + 9*f[48] - 45.0/2.0*f[49] + 18*f[50] - 9.0/2.0*f[51] + z*(81*f[0] - 405.0/2.0*f[16] + (2025.0/4.0)*f[17] - 405*f[18] + (405.0/4.0)*f[19] - 405.0/2.0*f[1] + 162*f[2] + 162*f[32] - 405*f[33] + 324*f[34] - 81*f[35] - 81.0/2.0*f[3] - 81.0/2.0*f[48] + (405.0/4.0)*f[49] - 81*f[50] + (81.0/4.0)*f[51] + z*(-81.0/2.0*f[0] + (243.0/2.0)*f[16] - 1215.0/4.0*f[17] + 243*f[18] - 243.0/4.0*f[19] + (405.0/4.0)*f[1] - 81*f[2] - 243.0/2.0*f[32] + (1215.0/4.0)*f[33] - 243*f[34] + (243.0/4.0)*f[35] + (81.0/4.0)*f[3] + (81.0/2.0)*f[48] - 405.0/4.0*f[49] + 81*f[50] - 81.0/4.0*f[51])))) + y*((121.0/4.0)*f[0] + (81.0/4.0)*f[10] - 9.0/2.0*f[11] - 11.0/2.0*f[12] + 9*f[13] - 9.0/2.0*f[14] + f[15] - 99.0/2.0*f[1] + (99.0/4.0)*f[2] - 11.0/2.0*f[3] - 99.0/2.0*f[4] + 81*f[5] - 81.0/2.0*f[6] + 9*f[7] + (99.0/4.0)*f[8] - 81.0/2.0*f[9] + y*(-99.0/2.0*f[0] - 81*f[10] + 18*f[11] + (99.0/4.0)*f[12] - 81.0/2.0*f[13] + (81.0/4.0)*f[14] - 9.0/2.0*f[15] + 81*f[1] - 81.0/2.0*f[2] + 9*f[3] + (495.0/4.0)*f[4] - 405.0/2.0*f[5] + (405.0/4.0)*f[6] - 45.0/2.0*f[7] - 99*f[8] + 162*f[9] + y*((99.0/4.0)*f[0] + (243.0/4.0)*f[10] - 27.0/2.0*f[11] - 99.0/4.0*f[12] + (81.0/2.0)*f[13] - 81.0/4.0*f[14] + (9.0/2.0)*f[15] - 81.0/2.0*f[1] + (81.0/4.0)*f[2] - 9.0/2.0*f[3] - 297.0/4.0*f[4] + (243.0/2.0)*f[5] - 243.0/4.0*f[6] + (27.0/2.0)*f[7] + (297.0/4.0)*f[8] - 243.0/2.0*f[9] + z*(-1089.0/8.0*f[0] - 2673.0/8.0*f[10] + (297.0/4.0)*f[11] + (1089.0/8.0)*f[12] - 891.0/4.0*f[13] + (891.0/8.0)*f[14] - 99.0/4.0*f[15] + (891.0/4.0)*f[16] - 729.0/2.0*f[17] + (729.0/4.0)*f[18] - 81.0/2.0*f[19] + (891.0/4.0)*f[1] - 2673.0/4.0*f[20] + (2187.0/2.0)*f[21] - 2187.0/4.0*f[22] + (243.0/2.0)*f[23] + (2673.0/4.0)*f[24] - 2187.0/2.0*f[25] + (2187.0/4.0)*f[26] - 243.0/2.0*f[27] - 891.0/4.0*f[28] + (729.0/2.0)*f[29] - 891.0/8.0*f[2] - 729.0/4.0*f[30] + (81.0/2.0)*f[31] - 891.0/8.0*f[32] + (729.0/4.0)*f[33] - 729.0/8.0*f[34] + (81.0/4.0)*f[35] + (2673.0/8.0)*f[36] - 2187.0/4.0*f[37] + (2187.0/8.0)*f[38] - 243.0/4.0*f[39] + (99.0/4.0)*f[3] - 2673.0/8.0*f[40] + (2187.0/4.0)*f[41] - 2187.0/8.0*f[42] + (243.0/4.0)*f[43] + (891.0/8.0)*f[44] - 729.0/4.0*f[45] + (729.0/8.0)*f[46] - 81.0/4.0*f[47] + (99.0/4.0)*f[48] - 81.0/2.0*f[49] + (3267.0/8.0)*f[4] + (81.0/4.0)*f[50] - 9.0/2.0*f[51] - 297.0/4.0*f[52] + (243.0/2.0)*f[53] - 243.0/4.0*f[54] + (27.0/2.0)*f[55] + (297.0/4.0)*f[56] - 243.0/2.0*f[57] + (243.0/4.0)*f[58] - 27.0/2.0*f[59] - 2673.0/4.0*f[5] - 99.0/4.0*f[60] + (81.0/2.0)*f[61] - 81.0/4.0*f[62] + (9.0/2.0)*f[63] + (2673.0/8.0)*f[6] - 297.0/4.0*f[7] - 3267.0/8.0*f[8] + (2673.0/4.0)*f[9] + z*((891.0/4.0)*f[0] + (2187.0/4.0)*f[10] - 243.0/2.0*f[11] - 891.0/4.0*f[12] + (729.0/2.0)*f[13] - 729.0/4.0*f[14] + (81.0/2.0)*f[15] - 4455.0/8.0*f[16] + (3645.0/4.0)*f[17] - 3645.0/8.0*f[18] + (405.0/4.0)*f[19] - 729.0/2.0*f[1] + (13365.0/8.0)*f[20] - 10935.0/4.0*f[21] + (10935.0/8.0)*f[22] - 1215.0/4.0*f[23] - 13365.0/8.0*f[24] + (10935.0/4.0)*f[25] - 10935.0/8.0*f[26] + (1215.0/4.0)*f[27] + (4455.0/8.0)*f[28] - 3645.0/4.0*f[29] + (729.0/4.0)*f[2] + (3645.0/8.0)*f[30] - 405.0/4.0*f[31] + (891.0/2.0)*f[32] - 729*f[33] + (729.0/2.0)*f[34] - 81*f[35] - 2673.0/2.0*f[36] + 2187*f[37] - 2187.0/2.0*f[38] + 243*f[39] - 81.0/2.0*f[3] + (2673.0/2.0)*f[40] - 2187*f[41] + (2187.0/2.0)*f[42] - 243*f[43] - 891.0/2.0*f[44] + 729*f[45] - 729.0/2.0*f[46] + 81*f[47] - 891.0/8.0*f[48] + (729.0/4.0)*f[49] - 2673.0/4.0*f[4] - 729.0/8.0*f[50] + (81.0/4.0)*f[51] + (2673.0/8.0)*f[52] - 2187.0/4.0*f[53] + (2187.0/8.0)*f[54] - 243.0/4.0*f[55] - 2673.0/8.0*f[56] + (2187.0/4.0)*f[57] - 2187.0/8.0*f[58] + (243.0/4.0)*f[59] + (2187.0/2.0)*f[5] + (891.0/8.0)*f[60] - 729.0/4.0*f[61] + (729.0/8.0)*f[62] - 81.0/4.0*f[63] - 2187.0/4.0*f[6] + (243.0/2.0)*f[7] + (2673.0/4.0)*f[8] - 2187.0/2.0*f[9] + z*(-891.0/8.0*f[0] - 2187.0/8.0*f[10] + (243.0/4.0)*f[11] + (891.0/8.0)*f[12] - 729.0/4.0*f[13] + (729.0/8.0)*f[14] - 81.0/4.0*f[15] + (2673.0/8.0)*f[16] - 2187.0/4.0*f[17] + (2187.0/8.0)*f[18] - 243.0/4.0*f[19] + (729.0/4.0)*f[1] - 8019.0/8.0*f[20] + (6561.0/4.0)*f[21] - 6561.0/8.0*f[22] + (729.0/4.0)*f[23] + (8019.0/8.0)*f[24] - 6561.0/4.0*f[25] + (6561.0/8.0)*f[26] - 729.0/4.0*f[27] - 2673.0/8.0*f[28] + (2187.0/4.0)*f[29] - 729.0/8.0*f[2] - 2187.0/8.0*f[30] + (243.0/4.0)*f[31] - 2673.0/8.0*f[32] + (2187.0/4.0)*f[33] - 2187.0/8.0*f[34] + (243.0/4.0)*f[35] + (8019.0/8.0)*f[36] - 6561.0/4.0*f[37] + (6561.0/8.0)*f[38] - 729.0/4.0*f[39] + (81.0/4.0)*f[3] - 8019.0/8.0*f[40] + (6561.0/4.0)*f[41] - 6561.0/8.0*f[42] + (729.0/4.0)*f[43] + (2673.0/8.0)*f[44] - 2187.0/4.0*f[45] + (2187.0/8.0)*f[46] - 243.0/4.0*f[47] + (891.0/8.0)*f[48] - 729.0/4.0*f[49] + (2673.0/8.0)*f[4] + (729.0/8.0)*f[50] - 81.0/4.0*f[51] - 2673.0/8.0*f[52] + (2187.0/4.0)*f[53] - 2187.0/8.0*f[54] + (243.0/4.0)*f[55] + (2673.0/8.0)*f[56] - 2187.0/4.0*f[57] + (2187.0/8.0)*f[58] - 243.0/4.0*f[59] - 2187.0/4.0*f[5] - 891.0/8.0*f[60] + (729.0/4.0)*f[61] - 729.0/8.0*f[62] + (81.0/4.0)*f[63] + (2187.0/8.0)*f[6] - 243.0/4.0*f[7] - 2673.0/8.0*f[8] + (2187.0/4.0)*f[9])))) + z*((1089.0/4.0)*f[0] + (891.0/2.0)*f[10] - 99*f[11] - 1089.0/8.0*f[12] + (891.0/4.0)*f[13] - 891.0/8.0*f[14] + (99.0/4.0)*f[15] - 891.0/2.0*f[16] + 729*f[17] - 729.0/2.0*f[18] + 81*f[19] - 891.0/2.0*f[1] + (4455.0/4.0)*f[20] - 3645.0/2.0*f[21] + (3645.0/4.0)*f[22] - 405.0/2.0*f[23] - 891*f[24] + 1458*f[25] - 729*f[26] + 162*f[27] + (891.0/4.0)*f[28] - 729.0/2.0*f[29] + (891.0/4.0)*f[2] + (729.0/4.0)*f[30] - 81.0/2.0*f[31] + (891.0/4.0)*f[32] - 729.0/2.0*f[33] + (729.0/4.0)*f[34] - 81.0/2.0*f[35] - 4455.0/8.0*f[36] + (3645.0/4.0)*f[37] - 3645.0/8.0*f[38] + (405.0/4.0)*f[39] - 99.0/2.0*f[3] + (891.0/2.0)*f[40] - 729*f[41] + (729.0/2.0)*f[42] - 81*f[43] - 891.0/8.0*f[44] + (729.0/4.0)*f[45] - 729.0/8.0*f[46] + (81.0/4.0)*f[47] - 99.0/2.0*f[48] + 81*f[49] - 5445.0/8.0*f[4] - 81.0/2.0*f[50] + 9*f[51] + (495.0/4.0)*f[52] - 405.0/2.0*f[53] + (405.0/4.0)*f[54] - 45.0/2.0*f[55] - 99*f[56] + 162*f[57] - 81*f[58] + 18*f[59] + (4455.0/4.0)*f[5] + (99.0/4.0)*f[60] - 81.0/2.0*f[61] + (81.0/4.0)*f[62] - 9.0/2.0*f[63] - 4455.0/8.0*f[6] + (495.0/4.0)*f[7] + (1089.0/2.0)*f[8] - 891*f[9] + z*(-891.0/2.0*f[0] - 729*f[10] + 162*f[11] + (891.0/4.0)*f[12] - 729.0/2.0*f[13] + (729.0/4.0)*f[14] - 81.0/2.0*f[15] + (4455.0/4.0)*f[16] - 3645.0/2.0*f[17] + (3645.0/4.0)*f[18] - 405.0/2.0*f[19] + 729*f[1] - 22275.0/8.0*f[20] + (18225.0/4.0)*f[21] - 18225.0/8.0*f[22] + (2025.0/4.0)*f[23] + (4455.0/2.0)*f[24] - 3645*f[25] + (3645.0/2.0)*f[26] - 405*f[27] - 4455.0/8.0*f[28] + (3645.0/4.0)*f[29] - 729.0/2.0*f[2] - 3645.0/8.0*f[30] + (405.0/4.0)*f[31] - 891*f[32] + 1458*f[33] - 729*f[34] + 162*f[35] + (4455.0/2.0)*f[36] - 3645*f[37] + (3645.0/2.0)*f[38] - 405*f[39] + 81*f[3] - 1782*f[40] + 2916*f[41] - 1458*f[42] + 324*f[43] + (891.0/2.0)*f[44] - 729*f[45] + (729.0/2.0)*f[46] - 81*f[47] + (891.0/4.0)*f[48] - 729.0/2.0*f[49] + (4455.0/4.0)*f[4] + (729.0/4.0)*f[50] - 81.0/2.0*f[51] - 4455.0/8.0*f[52] + (3645.0/4.0)*f[53] - 3645.0/8.0*f[54] + (405.0/4.0)*f[55] + (891.0/2.0)*f[56] - 729*f[57] + (729.0/2.0)*f[58] - 81*f[59] - 3645.0/2.0*f[5] - 891.0/8.0*f[60] + (729.0/4.0)*f[61] - 729.0/8.0*f[62] + (81.0/4.0)*f[63] + (3645.0/4.0)*f[6] - 405.0/2.0*f[7] - 891*f[8] + 1458*f[9] + z*((891.0/4.0)*f[0] + (729.0/2.0)*f[10] - 81*f[11] - 891.0/8.0*f[12] + (729.0/4.0)*f[13] - 729.0/8.0*f[14] + (81.0/4.0)*f[15] - 2673.0/4.0*f[16] + (2187.0/2.0)*f[17] - 2187.0/4.0*f[18] + (243.0/2.0)*f[19] - 729.0/2.0*f[1] + (13365.0/8.0)*f[20] - 10935.0/4.0*f[21] + (10935.0/8.0)*f[22] - 1215.0/4.0*f[23] - 2673.0/2.0*f[24] + 2187*f[25] - 2187.0/2.0*f[26] + 243*f[27] + (2673.0/8.0)*f[28] - 2187.0/4.0*f[29] + (729.0/4.0)*f[2] + (2187.0/8.0)*f[30] - 243.0/4.0*f[31] + (2673.0/4.0)*f[32] - 2187.0/2.0*f[33] + (2187.0/4.0)*f[34] - 243.0/2.0*f[35] - 13365.0/8.0*f[36] + (10935.0/4.0)*f[37] - 10935.0/8.0*f[38] + (1215.0/4.0)*f[39] - 81.0/2.0*f[3] + (2673.0/2.0)*f[40] - 2187*f[41] + (2187.0/2.0)*f[42] - 243*f[43] - 2673.0/8.0*f[44] + (2187.0/4.0)*f[45] - 2187.0/8.0*f[46] + (243.0/4.0)*f[47] - 891.0/4.0*f[48] + (729.0/2.0)*f[49] - 4455.0/8.0*f[4] - 729.0/4.0*f[50] + (81.0/2.0)*f[51] + (4455.0/8.0)*f[52] - 3645.0/4.0*f[53] + (3645.0/8.0)*f[54] - 405.0/4.0*f[55] - 891.0/2.0*f[56] + 729*f[57] - 729.0/2.0*f[58] + 81*f[59] + (3645.0/4.0)*f[5] + (891.0/8.0)*f[60] - 729.0/4.0*f[61] + (729.0/8.0)*f[62] - 81.0/4.0*f[63] - 3645.0/8.0*f[6] + (405.0/4.0)*f[7] + (891.0/2.0)*f[8] - 729*f[9])))) + z*(-1331.0/8.0*f[0] - 891.0/8.0*f[10] + (99.0/4.0)*f[11] + (121.0/4.0)*f[12] - 99.0/2.0*f[13] + (99.0/4.0)*f[14] - 11.0/2.0*f[15] + (1089.0/4.0)*f[16] - 891.0/2.0*f[17] + (891.0/4.0)*f[18] - 99.0/2.0*f[19] + (1089.0/4.0)*f[1] - 891.0/2.0*f[20] + 729*f[21] - 729.0/2.0*f[22] + 81*f[23] + (891.0/4.0)*f[24] - 729.0/2.0*f[25] + (729.0/4.0)*f[26] - 81.0/2.0*f[27] - 99.0/2.0*f[28] + 81*f[29] - 1089.0/8.0*f[2] - 81.0/2.0*f[30] + 9*f[31] - 1089.0/8.0*f[32] + (891.0/4.0)*f[33] - 891.0/8.0*f[34] + (99.0/4.0)*f[35] + (891.0/4.0)*f[36] - 729.0/2.0*f[37] + (729.0/4.0)*f[38] - 81.0/2.0*f[39] + (121.0/4.0)*f[3] - 891.0/8.0*f[40] + (729.0/4.0)*f[41] - 729.0/8.0*f[42] + (81.0/4.0)*f[43] + (99.0/4.0)*f[44] - 81.0/2.0*f[45] + (81.0/4.0)*f[46] - 9.0/2.0*f[47] + (121.0/4.0)*f[48] - 99.0/2.0*f[49] + (1089.0/4.0)*f[4] + (99.0/4.0)*f[50] - 11.0/2.0*f[51] - 99.0/2.0*f[52] + 81*f[53] - 81.0/2.0*f[54] + 9*f[55] + (99.0/4.0)*f[56] - 81.0/2.0*f[57] + (81.0/4.0)*f[58] - 9.0/2.0*f[59] - 891.0/2.0*f[5] - 11.0/2.0*f[60] + 9*f[61] - 9.0/2.0*f[62] + f[63] + (891.0/4.0)*f[6] - 99.0/2.0*f[7] - 1089.0/8.0*f[8] + (891.0/4.0)*f[9] + z*((1089.0/4.0)*f[0] + (729.0/4.0)*f[10] - 81.0/2.0*f[11] - 99.0/2.0*f[12] + 81*f[13] - 81.0/2.0*f[14] + 9*f[15] - 5445.0/8.0*f[16] + (4455.0/4.0)*f[17] - 4455.0/8.0*f[18] + (495.0/4.0)*f[19] - 891.0/2.0*f[1] + (4455.0/4.0)*f[20] - 3645.0/2.0*f[21] + (3645.0/4.0)*f[22] - 405.0/2.0*f[23] - 4455.0/8.0*f[24] + (3645.0/4.0)*f[25] - 3645.0/8.0*f[26] + (405.0/4.0)*f[27] + (495.0/4.0)*f[28] - 405.0/2.0*f[29] + (891.0/4.0)*f[2] + (405.0/4.0)*f[30] - 45.0/2.0*f[31] + (1089.0/2.0)*f[32] - 891*f[33] + (891.0/2.0)*f[34] - 99*f[35] - 891*f[36] + 1458*f[37] - 729*f[38] + 162*f[39] - 99.0/2.0*f[3] + (891.0/2.0)*f[40] - 729*f[41] + (729.0/2.0)*f[42] - 81*f[43] - 99*f[44] + 162*f[45] - 81*f[46] + 18*f[47] - 1089.0/8.0*f[48] + (891.0/4.0)*f[49] - 891.0/2.0*f[4] - 891.0/8.0*f[50] + (99.0/4.0)*f[51] + (891.0/4.0)*f[52] - 729.0/2.0*f[53] + (729.0/4.0)*f[54] - 81.0/2.0*f[55] - 891.0/8.0*f[56] + (729.0/4.0)*f[57] - 729.0/8.0*f[58] + (81.0/4.0)*f[59] + 729*f[5] + (99.0/4.0)*f[60] - 81.0/2.0*f[61] + (81.0/4.0)*f[62] - 9.0/2.0*f[63] - 729.0/2.0*f[6] + 81*f[7] + (891.0/4.0)*f[8] - 729.0/2.0*f[9] + z*(-1089.0/8.0*f[0] - 729.0/8.0*f[10] + (81.0/4.0)*f[11] + (99.0/4.0)*f[12] - 81.0/2.0*f[13] + (81.0/4.0)*f[14] - 9.0/2.0*f[15] + (3267.0/8.0)*f[16] - 2673.0/4.0*f[17] + (2673.0/8.0)*f[18] - 297.0/4.0*f[19] + (891.0/4.0)*f[1] - 2673.0/4.0*f[20] + (2187.0/2.0)*f[21] - 2187.0/4.0*f[22] + (243.0/2.0)*f[23] + (2673.0/8.0)*f[24] - 2187.0/4.0*f[25] + (2187.0/8.0)*f[26] - 243.0/4.0*f[27] - 297.0/4.0*f[28] + (243.0/2.0)*f[29] - 891.0/8.0*f[2] - 243.0/4.0*f[30] + (27.0/2.0)*f[31] - 3267.0/8.0*f[32] + (2673.0/4.0)*f[33] - 2673.0/8.0*f[34] + (297.0/4.0)*f[35] + (2673.0/4.0)*f[36] - 2187.0/2.0*f[37] + (2187.0/4.0)*f[38] - 243.0/2.0*f[39] + (99.0/4.0)*f[3] - 2673.0/8.0*f[40] + (2187.0/4.0)*f[41] - 2187.0/8.0*f[42] + (243.0/4.0)*f[43] + (297.0/4.0)*f[44] - 243.0/2.0*f[45] + (243.0/4.0)*f[46] - 27.0/2.0*f[47] + (1089.0/8.0)*f[48] - 891.0/4.0*f[49] + (891.0/4.0)*f[4] + (891.0/8.0)*f[50] - 99.0/4.0*f[51] - 891.0/4.0*f[52] + (729.0/2.0)*f[53] - 729.0/4.0*f[54] + (81.0/2.0)*f[55] + (891.0/8.0)*f[56] - 729.0/4.0*f[57] + (729.0/8.0)*f[58] - 81.0/4.0*f[59] - 729.0/2.0*f[5] - 99.0/4.0*f[60] + (81.0/2.0)*f[61] - 81.0/4.0*f[62] + (9.0/2.0)*f[63] + (729.0/4.0)*f[6] - 81.0/2.0*f[7] - 891.0/8.0*f[8] + (729.0/4.0)*f[9])))) + z*((121.0/4.0)*f[0] - 99.0/2.0*f[16] + 81*f[17] - 81.0/2.0*f[18] + 9*f[19] - 99.0/2.0*f[1] + (99.0/4.0)*f[2] + (99.0/4.0)*f[32] - 81.0/2.0*f[33] + (81.0/4.0)*f[34] - 9.0/2.0*f[35] - 11.0/2.0*f[3] - 11.0/2.0*f[48] + 9*f[49] - 9.0/2.0*f[50] + f[51] + z*(-99.0/2.0*f[0] + (495.0/4.0)*f[16] - 405.0/2.0*f[17] + (405.0/4.0)*f[18] - 45.0/2.0*f[19] + 81*f[1] - 81.0/2.0*f[2] - 99*f[32] + 162*f[33] - 81*f[34] + 18*f[35] + 9*f[3] + (99.0/4.0)*f[48] - 81.0/2.0*f[49] + (81.0/4.0)*f[50] - 9.0/2.0*f[51] + z*((99.0/4.0)*f[0] - 297.0/4.0*f[16] + (243.0/2.0)*f[17] - 243.0/4.0*f[18] + (27.0/2.0)*f[19] - 81.0/2.0*f[1] + (81.0/4.0)*f[2] + (297.0/4.0)*f[32] - 243.0/2.0*f[33] + (243.0/4.0)*f[34] - 27.0/2.0*f[35] - 9.0/2.0*f[3] - 99.0/4.0*f[48] + (81.0/2.0)*f[49] - 81.0/4.0*f[50] + (9.0/2.0)*f[51])))) + y*(-11.0/2.0*f[0] + f[12] + 9*f[4] - 9.0/2.0*f[8] + y*(9*f[0] - 9.0/2.0*f[12] - 45.0/2.0*f[4] + 18*f[8] + y*(-9.0/2.0*f[0] + (9.0/2.0)*f[12] + (27.0/2.0)*f[4] - 27.0/2.0*f[8] + z*((99.0/4.0)*f[0] - 99.0/4.0*f[12] - 81.0/2.0*f[16] + (243.0/2.0)*f[20] - 243.0/2.0*f[24] + (81.0/2.0)*f[28] + (81.0/4.0)*f[32] - 243.0/4.0*f[36] + (243.0/4.0)*f[40] - 81.0/4.0*f[44] - 9.0/2.0*f[48] - 297.0/4.0*f[4] + (27.0/2.0)*f[52] - 27.0/2.0*f[56] + (9.0/2.0)*f[60] + (297.0/4.0)*f[8] + z*(-81.0/2.0*f[0] + (81.0/2.0)*f[12] + (405.0/4.0)*f[16] - 1215.0/4.0*f[20] + (1215.0/4.0)*f[24] - 405.0/4.0*f[28] - 81*f[32] + 243*f[36] - 243*f[40] + 81*f[44] + (81.0/4.0)*f[48] + (243.0/2.0)*f[4] - 243.0/4.0*f[52] + (243.0/4.0)*f[56] - 81.0/4.0*f[60] - 243.0/2.0*f[8] + z*((81.0/4.0)*f[0] - 81.0/4.0*f[12] - 243.0/4.0*f[16] + (729.0/4.0)*f[20] - 729.0/4.0*f[24] + (243.0/4.0)*f[28] + (243.0/4.0)*f[32] - 729.0/4.0*f[36] + (729.0/4.0)*f[40] - 243.0/4.0*f[44] - 81.0/4.0*f[48] - 243.0/4.0*f[4] + (243.0/4.0)*f[52] - 243.0/4.0*f[56] + (81.0/4.0)*f[60] + (243.0/4.0)*f[8])))) + z*(-99.0/2.0*f[0] + (99.0/4.0)*f[12] + 81*f[16] - 405.0/2.0*f[20] + 162*f[24] - 81.0/2.0*f[28] - 81.0/2.0*f[32] + (405.0/4.0)*f[36] - 81*f[40] + (81.0/4.0)*f[44] + 9*f[48] + (495.0/4.0)*f[4] - 45.0/2.0*f[52] + 18*f[56] - 9.0/2.0*f[60] - 99*f[8] + z*(81*f[0] - 81.0/2.0*f[12] - 405.0/2.0*f[16] + (2025.0/4.0)*f[20] - 405*f[24] + (405.0/4.0)*f[28] + 162*f[32] - 405*f[36] + 324*f[40] - 81*f[44] - 81.0/2.0*f[48] - 405.0/2.0*f[4] + (405.0/4.0)*f[52] - 81*f[56] + (81.0/4.0)*f[60] + 162*f[8] + z*(-81.0/2.0*f[0] + (81.0/4.0)*f[12] + (243.0/2.0)*f[16] - 1215.0/4.0*f[20] + 243*f[24] - 243.0/4.0*f[28] - 243.0/2.0*f[32] + (1215.0/4.0)*f[36] - 243*f[40] + (243.0/4.0)*f[44] + (81.0/2.0)*f[48] + (405.0/4.0)*f[4] - 405.0/4.0*f[52] + 81*f[56] - 81.0/4.0*f[60] - 81*f[8])))) + z*((121.0/4.0)*f[0] - 11.0/2.0*f[12] - 99.0/2.0*f[16] + 81*f[20] - 81.0/2.0*f[24] + 9*f[28] + (99.0/4.0)*f[32] - 81.0/2.0*f[36] + (81.0/4.0)*f[40] - 9.0/2.0*f[44] - 11.0/2.0*f[48] - 99.0/2.0*f[4] + 9*f[52] - 9.0/2.0*f[56] + f[60] + (99.0/4.0)*f[8] + z*(-99.0/2.0*f[0] + 9*f[12] + (495.0/4.0)*f[16] - 405.0/2.0*f[20] + (405.0/4.0)*f[24] - 45.0/2.0*f[28] - 99*f[32] + 162*f[36] - 81*f[40] + 18*f[44] + (99.0/4.0)*f[48] + 81*f[4] - 81.0/2.0*f[52] + (81.0/4.0)*f[56] - 9.0/2.0*f[60] - 81.0/2.0*f[8] + z*((99.0/4.0)*f[0] - 9.0/2.0*f[12] - 297.0/4.0*f[16] + (243.0/2.0)*f[20] - 243.0/4.0*f[24] + (27.0/2.0)*f[28] + (297.0/4.0)*f[32] - 243.0/2.0*f[36] + (243.0/4.0)*f[40] - 27.0/2.0*f[44] - 99.0/4.0*f[48] - 81.0/2.0*f[4] + (81.0/2.0)*f[52] - 81.0/4.0*f[56] + (9.0/2.0)*f[60] + (81.0/4.0)*f[8])))) + z*(-11.0/2.0*f[0] + 9*f[16] - 9.0/2.0*f[32] + f[48] + z*(9*f[0] - 45.0/2.0*f[16] + 18*f[32] - 9.0/2.0*f[48] + z*(-9.0/2.0*f[0] + (27.0/2.0)*f[16] - 27.0/2.0*f[32] + (9.0/2.0)*f[48])));
}
#endif
#endif

#ifdef ET_PRISM
#if ORDER==1
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
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
  return f[0] + x*(-f[0] + f[1] + z*(f[0] - f[1] - f[3] + f[4])) + y*(-f[0] + f[2] + z*(f[0] - f[2] - f[3] + f[5])) + z*(-f[0] + f[3]);
}
#endif
#endif

#ifdef ET_PRISM
#if ORDER==1
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
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
  return f[0] + x*(-f[0] + f[1] + z*(f[0] - f[1] - f[3] + f[4])) + y*(-f[0] + f[2] + z*(f[0] - f[2] - f[3] + f[5])) + z*(-f[0] + f[3]);
}
#endif
#endif

#ifdef ET_PRISM
#if ORDER==2
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2] + z*(-6*f[0] - 2*f[12] + 4*f[13] - 2*f[14] + 12*f[1] - 6*f[2] + 8*f[6] - 16*f[7] + 8*f[8] + z*(4*f[0] + 4*f[12] - 8*f[13] + 4*f[14] - 8*f[1] + 4*f[2] - 8*f[6] + 16*f[7] - 8*f[8]))) + y*(4*f[0] - 4*f[1] - 4*f[3] + 4*f[4] + z*(-12*f[0] + 16*f[10] - 4*f[12] + 4*f[13] + 4*f[15] - 4*f[16] + 12*f[1] + 12*f[3] - 12*f[4] + 16*f[6] - 16*f[7] - 16*f[9] + z*(8*f[0] - 16*f[10] + 8*f[12] - 8*f[13] - 8*f[15] + 8*f[16] - 8*f[1] - 8*f[3] + 8*f[4] - 16*f[6] + 16*f[7] + 16*f[9]))) + z*(9*f[0] + 3*f[12] - 4*f[13] + f[14] - 12*f[1] + 3*f[2] - 12*f[6] + 16*f[7] - 4*f[8] + z*(-6*f[0] - 6*f[12] + 8*f[13] - 2*f[14] + 8*f[1] - 2*f[2] + 12*f[6] - 16*f[7] + 4*f[8]))) + y*(-3*f[0] + 4*f[3] - f[5] + y*(2*f[0] - 4*f[3] + 2*f[5] + z*(-6*f[0] + 8*f[11] - 2*f[12] + 4*f[15] - 2*f[17] + 12*f[3] - 6*f[5] + 8*f[6] - 16*f[9] + z*(4*f[0] - 8*f[11] + 4*f[12] - 8*f[15] + 4*f[17] - 8*f[3] + 4*f[5] - 8*f[6] + 16*f[9]))) + z*(9*f[0] - 4*f[11] + 3*f[12] - 4*f[15] + f[17] - 12*f[3] + 3*f[5] - 12*f[6] + 16*f[9] + z*(-6*f[0] + 4*f[11] - 6*f[12] + 8*f[15] - 2*f[17] + 8*f[3] - 2*f[5] + 12*f[6] - 16*f[9]))) + z*(-3*f[0] - f[12] + 4*f[6] + z*(2*f[0] + 2*f[12] - 4*f[6]));
}
#endif
#endif

#ifdef ET_PRISM
#if ORDER==2
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
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
  return f[0] + x*(-3*f[0] + 4*f[1] - f[2] + x*(2*f[0] - 4*f[1] + 2*f[2] + z*(-6*f[0] - 2*f[12] + 4*f[13] - 2*f[14] + 12*f[1] - 6*f[2] + 8*f[6] - 16*f[7] + 8*f[8] + z*(4*f[0] + 4*f[12] - 8*f[13] + 4*f[14] - 8*f[1] + 4*f[2] - 8*f[6] + 16*f[7] - 8*f[8]))) + y*(4*f[0] - 4*f[1] - 4*f[3] + 4*f[4] + z*(-12*f[0] + 16*f[10] - 4*f[12] + 4*f[13] + 4*f[15] - 4*f[16] + 12*f[1] + 12*f[3] - 12*f[4] + 16*f[6] - 16*f[7] - 16*f[9] + z*(8*f[0] - 16*f[10] + 8*f[12] - 8*f[13] - 8*f[15] + 8*f[16] - 8*f[1] - 8*f[3] + 8*f[4] - 16*f[6] + 16*f[7] + 16*f[9]))) + z*(9*f[0] + 3*f[12] - 4*f[13] + f[14] - 12*f[1] + 3*f[2] - 12*f[6] + 16*f[7] - 4*f[8] + z*(-6*f[0] - 6*f[12] + 8*f[13] - 2*f[14] + 8*f[1] - 2*f[2] + 12*f[6] - 16*f[7] + 4*f[8]))) + y*(-3*f[0] + 4*f[3] - f[5] + y*(2*f[0] - 4*f[3] + 2*f[5] + z*(-6*f[0] + 8*f[11] - 2*f[12] + 4*f[15] - 2*f[17] + 12*f[3] - 6*f[5] + 8*f[6] - 16*f[9] + z*(4*f[0] - 8*f[11] + 4*f[12] - 8*f[15] + 4*f[17] - 8*f[3] + 4*f[5] - 8*f[6] + 16*f[9]))) + z*(9*f[0] - 4*f[11] + 3*f[12] - 4*f[15] + f[17] - 12*f[3] + 3*f[5] - 12*f[6] + 16*f[9] + z*(-6*f[0] + 4*f[11] - 6*f[12] + 8*f[15] - 2*f[17] + 8*f[3] - 2*f[5] + 12*f[6] - 16*f[9]))) + z*(-3*f[0] - f[12] + 4*f[6] + z*(2*f[0] + 2*f[12] - 4*f[6]));
}
#endif
#endif

#ifdef ET_PRISM
#if ORDER==3
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[40];
  int ii=0;
  for (int k=0; k<=3; k++) {
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
    first += dz;
  }
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3] + z*((99.0/4.0)*f[0] - 81.0/2.0*f[10] + (243.0/2.0)*f[11] - 243.0/2.0*f[12] + (81.0/2.0)*f[13] - 297.0/4.0*f[1] + (81.0/4.0)*f[20] - 243.0/4.0*f[21] + (243.0/4.0)*f[22] - 81.0/4.0*f[23] + (297.0/4.0)*f[2] - 9.0/2.0*f[30] + (27.0/2.0)*f[31] - 27.0/2.0*f[32] + (9.0/2.0)*f[33] - 99.0/4.0*f[3] + z*(-81.0/2.0*f[0] + (405.0/4.0)*f[10] - 1215.0/4.0*f[11] + (1215.0/4.0)*f[12] - 405.0/4.0*f[13] + (243.0/2.0)*f[1] - 81*f[20] + 243*f[21] - 243*f[22] + 81*f[23] - 243.0/2.0*f[2] + (81.0/4.0)*f[30] - 243.0/4.0*f[31] + (243.0/4.0)*f[32] - 81.0/4.0*f[33] + (81.0/2.0)*f[3] + z*((81.0/4.0)*f[0] - 243.0/4.0*f[10] + (729.0/4.0)*f[11] - 729.0/4.0*f[12] + (243.0/4.0)*f[13] - 243.0/4.0*f[1] + (243.0/4.0)*f[20] - 729.0/4.0*f[21] + (729.0/4.0)*f[22] - 243.0/4.0*f[23] + (243.0/4.0)*f[2] - 81.0/4.0*f[30] + (243.0/4.0)*f[31] - 243.0/4.0*f[32] + (81.0/4.0)*f[33] - 81.0/4.0*f[3])))) + y*(-27.0/2.0*f[0] + 27*f[1] - 27.0/2.0*f[2] + (27.0/2.0)*f[4] - 27*f[5] + (27.0/2.0)*f[6] + z*((297.0/4.0)*f[0] - 243.0/2.0*f[10] + 243*f[11] - 243.0/2.0*f[12] + (243.0/2.0)*f[14] - 243*f[15] + (243.0/2.0)*f[16] - 297.0/2.0*f[1] + (243.0/4.0)*f[20] - 243.0/2.0*f[21] + (243.0/4.0)*f[22] - 243.0/4.0*f[24] + (243.0/2.0)*f[25] - 243.0/4.0*f[26] + (297.0/4.0)*f[2] - 27.0/2.0*f[30] + 27*f[31] - 27.0/2.0*f[32] + (27.0/2.0)*f[34] - 27*f[35] + (27.0/2.0)*f[36] - 297.0/4.0*f[4] + (297.0/2.0)*f[5] - 297.0/4.0*f[6] + z*(-243.0/2.0*f[0] + (1215.0/4.0)*f[10] - 1215.0/2.0*f[11] + (1215.0/4.0)*f[12] - 1215.0/4.0*f[14] + (1215.0/2.0)*f[15] - 1215.0/4.0*f[16] + 243*f[1] - 243*f[20] + 486*f[21] - 243*f[22] + 243*f[24] - 486*f[25] + 243*f[26] - 243.0/2.0*f[2] + (243.0/4.0)*f[30] - 243.0/2.0*f[31] + (243.0/4.0)*f[32] - 243.0/4.0*f[34] + (243.0/2.0)*f[35] - 243.0/4.0*f[36] + (243.0/2.0)*f[4] - 243*f[5] + (243.0/2.0)*f[6] + z*((243.0/4.0)*f[0] - 729.0/4.0*f[10] + (729.0/2.0)*f[11] - 729.0/4.0*f[12] + (729.0/4.0)*f[14] - 729.0/2.0*f[15] + (729.0/4.0)*f[16] - 243.0/2.0*f[1] + (729.0/4.0)*f[20] - 729.0/2.0*f[21] + (729.0/4.0)*f[22] - 729.0/4.0*f[24] + (729.0/2.0)*f[25] - 729.0/4.0*f[26] + (243.0/4.0)*f[2] - 243.0/4.0*f[30] + (243.0/2.0)*f[31] - 243.0/4.0*f[32] + (243.0/4.0)*f[34] - 243.0/2.0*f[35] + (243.0/4.0)*f[36] - 243.0/4.0*f[4] + (243.0/2.0)*f[5] - 243.0/4.0*f[6])))) + z*(-99.0/2.0*f[0] + 81*f[10] - 405.0/2.0*f[11] + 162*f[12] - 81.0/2.0*f[13] + (495.0/4.0)*f[1] - 81.0/2.0*f[20] + (405.0/4.0)*f[21] - 81*f[22] + (81.0/4.0)*f[23] - 99*f[2] + 9*f[30] - 45.0/2.0*f[31] + 18*f[32] - 9.0/2.0*f[33] + (99.0/4.0)*f[3] + z*(81*f[0] - 405.0/2.0*f[10] + (2025.0/4.0)*f[11] - 405*f[12] + (405.0/4.0)*f[13] - 405.0/2.0*f[1] + 162*f[20] - 405*f[21] + 324*f[22] - 81*f[23] + 162*f[2] - 81.0/2.0*f[30] + (405.0/4.0)*f[31] - 81*f[32] + (81.0/4.0)*f[33] - 81.0/2.0*f[3] + z*(-81.0/2.0*f[0] + (243.0/2.0)*f[10] - 1215.0/4.0*f[11] + 243*f[12] - 243.0/4.0*f[13] + (405.0/4.0)*f[1] - 243.0/2.0*f[20] + (1215.0/4.0)*f[21] - 243*f[22] + (243.0/4.0)*f[23] - 81*f[2] + (81.0/2.0)*f[30] - 405.0/4.0*f[31] + 81*f[32] - 81.0/4.0*f[33] + (81.0/4.0)*f[3])))) + y*(18*f[0] - 45.0/2.0*f[1] + (9.0/2.0)*f[2] - 45.0/2.0*f[4] + 27*f[5] - 9.0/2.0*f[6] + (9.0/2.0)*f[7] - 9.0/2.0*f[8] + y*(-27.0/2.0*f[0] + (27.0/2.0)*f[1] + 27*f[4] - 27*f[5] - 27.0/2.0*f[7] + (27.0/2.0)*f[8] + z*((297.0/4.0)*f[0] - 243.0/2.0*f[10] + (243.0/2.0)*f[11] + 243*f[14] - 243*f[15] - 243.0/2.0*f[17] + (243.0/2.0)*f[18] - 297.0/4.0*f[1] + (243.0/4.0)*f[20] - 243.0/4.0*f[21] - 243.0/2.0*f[24] + (243.0/2.0)*f[25] + (243.0/4.0)*f[27] - 243.0/4.0*f[28] - 27.0/2.0*f[30] + (27.0/2.0)*f[31] + 27*f[34] - 27*f[35] - 27.0/2.0*f[37] + (27.0/2.0)*f[38] - 297.0/2.0*f[4] + (297.0/2.0)*f[5] + (297.0/4.0)*f[7] - 297.0/4.0*f[8] + z*(-243.0/2.0*f[0] + (1215.0/4.0)*f[10] - 1215.0/4.0*f[11] - 1215.0/2.0*f[14] + (1215.0/2.0)*f[15] + (1215.0/4.0)*f[17] - 1215.0/4.0*f[18] + (243.0/2.0)*f[1] - 243*f[20] + 243*f[21] + 486*f[24] - 486*f[25] - 243*f[27] + 243*f[28] + (243.0/4.0)*f[30] - 243.0/4.0*f[31] - 243.0/2.0*f[34] + (243.0/2.0)*f[35] + (243.0/4.0)*f[37] - 243.0/4.0*f[38] + 243*f[4] - 243*f[5] - 243.0/2.0*f[7] + (243.0/2.0)*f[8] + z*((243.0/4.0)*f[0] - 729.0/4.0*f[10] + (729.0/4.0)*f[11] + (729.0/2.0)*f[14] - 729.0/2.0*f[15] - 729.0/4.0*f[17] + (729.0/4.0)*f[18] - 243.0/4.0*f[1] + (729.0/4.0)*f[20] - 729.0/4.0*f[21] - 729.0/2.0*f[24] + (729.0/2.0)*f[25] + (729.0/4.0)*f[27] - 729.0/4.0*f[28] - 243.0/4.0*f[30] + (243.0/4.0)*f[31] + (243.0/2.0)*f[34] - 243.0/2.0*f[35] - 243.0/4.0*f[37] + (243.0/4.0)*f[38] - 243.0/2.0*f[4] + (243.0/2.0)*f[5] + (243.0/4.0)*f[7] - 243.0/4.0*f[8])))) + z*(-99*f[0] + 162*f[10] - 405.0/2.0*f[11] + (81.0/2.0)*f[12] - 405.0/2.0*f[14] + 243*f[15] - 81.0/2.0*f[16] + (81.0/2.0)*f[17] - 81.0/2.0*f[18] + (495.0/4.0)*f[1] - 81*f[20] + (405.0/4.0)*f[21] - 81.0/4.0*f[22] + (405.0/4.0)*f[24] - 243.0/2.0*f[25] + (81.0/4.0)*f[26] - 81.0/4.0*f[27] + (81.0/4.0)*f[28] - 99.0/4.0*f[2] + 18*f[30] - 45.0/2.0*f[31] + (9.0/2.0)*f[32] - 45.0/2.0*f[34] + 27*f[35] - 9.0/2.0*f[36] + (9.0/2.0)*f[37] - 9.0/2.0*f[38] + (495.0/4.0)*f[4] - 297.0/2.0*f[5] + (99.0/4.0)*f[6] - 99.0/4.0*f[7] + (99.0/4.0)*f[8] + z*(162*f[0] - 405*f[10] + (2025.0/4.0)*f[11] - 405.0/4.0*f[12] + (2025.0/4.0)*f[14] - 1215.0/2.0*f[15] + (405.0/4.0)*f[16] - 405.0/4.0*f[17] + (405.0/4.0)*f[18] - 405.0/2.0*f[1] + 324*f[20] - 405*f[21] + 81*f[22] - 405*f[24] + 486*f[25] - 81*f[26] + 81*f[27] - 81*f[28] + (81.0/2.0)*f[2] - 81*f[30] + (405.0/4.0)*f[31] - 81.0/4.0*f[32] + (405.0/4.0)*f[34] - 243.0/2.0*f[35] + (81.0/4.0)*f[36] - 81.0/4.0*f[37] + (81.0/4.0)*f[38] - 405.0/2.0*f[4] + 243*f[5] - 81.0/2.0*f[6] + (81.0/2.0)*f[7] - 81.0/2.0*f[8] + z*(-81*f[0] + 243*f[10] - 1215.0/4.0*f[11] + (243.0/4.0)*f[12] - 1215.0/4.0*f[14] + (729.0/2.0)*f[15] - 243.0/4.0*f[16] + (243.0/4.0)*f[17] - 243.0/4.0*f[18] + (405.0/4.0)*f[1] - 243*f[20] + (1215.0/4.0)*f[21] - 243.0/4.0*f[22] + (1215.0/4.0)*f[24] - 729.0/2.0*f[25] + (243.0/4.0)*f[26] - 243.0/4.0*f[27] + (243.0/4.0)*f[28] - 81.0/4.0*f[2] + 81*f[30] - 405.0/4.0*f[31] + (81.0/4.0)*f[32] - 405.0/4.0*f[34] + (243.0/2.0)*f[35] - 81.0/4.0*f[36] + (81.0/4.0)*f[37] - 81.0/4.0*f[38] + (405.0/4.0)*f[4] - 243.0/2.0*f[5] + (81.0/4.0)*f[6] - 81.0/4.0*f[7] + (81.0/4.0)*f[8])))) + z*((121.0/4.0)*f[0] - 99.0/2.0*f[10] + 81*f[11] - 81.0/2.0*f[12] + 9*f[13] - 99.0/2.0*f[1] + (99.0/4.0)*f[20] - 81.0/2.0*f[21] + (81.0/4.0)*f[22] - 9.0/2.0*f[23] + (99.0/4.0)*f[2] - 11.0/2.0*f[30] + 9*f[31] - 9.0/2.0*f[32] + f[33] - 11.0/2.0*f[3] + z*(-99.0/2.0*f[0] + (495.0/4.0)*f[10] - 405.0/2.0*f[11] + (405.0/4.0)*f[12] - 45.0/2.0*f[13] + 81*f[1] - 99*f[20] + 162*f[21] - 81*f[22] + 18*f[23] - 81.0/2.0*f[2] + (99.0/4.0)*f[30] - 81.0/2.0*f[31] + (81.0/4.0)*f[32] - 9.0/2.0*f[33] + 9*f[3] + z*((99.0/4.0)*f[0] - 297.0/4.0*f[10] + (243.0/2.0)*f[11] - 243.0/4.0*f[12] + (27.0/2.0)*f[13] - 81.0/2.0*f[1] + (297.0/4.0)*f[20] - 243.0/2.0*f[21] + (243.0/4.0)*f[22] - 27.0/2.0*f[23] + (81.0/4.0)*f[2] - 99.0/4.0*f[30] + (81.0/2.0)*f[31] - 81.0/4.0*f[32] + (9.0/2.0)*f[33] - 9.0/2.0*f[3])))) + y*(-11.0/2.0*f[0] + 9*f[4] - 9.0/2.0*f[7] + f[9] + y*(9*f[0] - 45.0/2.0*f[4] + 18*f[7] - 9.0/2.0*f[9] + y*(-9.0/2.0*f[0] + (27.0/2.0)*f[4] - 27.0/2.0*f[7] + (9.0/2.0)*f[9] + z*((99.0/4.0)*f[0] - 81.0/2.0*f[10] + (243.0/2.0)*f[14] - 243.0/2.0*f[17] + (81.0/2.0)*f[19] + (81.0/4.0)*f[20] - 243.0/4.0*f[24] + (243.0/4.0)*f[27] - 81.0/4.0*f[29] - 9.0/2.0*f[30] + (27.0/2.0)*f[34] - 27.0/2.0*f[37] + (9.0/2.0)*f[39] - 297.0/4.0*f[4] + (297.0/4.0)*f[7] - 99.0/4.0*f[9] + z*(-81.0/2.0*f[0] + (405.0/4.0)*f[10] - 1215.0/4.0*f[14] + (1215.0/4.0)*f[17] - 405.0/4.0*f[19] - 81*f[20] + 243*f[24] - 243*f[27] + 81*f[29] + (81.0/4.0)*f[30] - 243.0/4.0*f[34] + (243.0/4.0)*f[37] - 81.0/4.0*f[39] + (243.0/2.0)*f[4] - 243.0/2.0*f[7] + (81.0/2.0)*f[9] + z*((81.0/4.0)*f[0] - 243.0/4.0*f[10] + (729.0/4.0)*f[14] - 729.0/4.0*f[17] + (243.0/4.0)*f[19] + (243.0/4.0)*f[20] - 729.0/4.0*f[24] + (729.0/4.0)*f[27] - 243.0/4.0*f[29] - 81.0/4.0*f[30] + (243.0/4.0)*f[34] - 243.0/4.0*f[37] + (81.0/4.0)*f[39] - 243.0/4.0*f[4] + (243.0/4.0)*f[7] - 81.0/4.0*f[9])))) + z*(-99.0/2.0*f[0] + 81*f[10] - 405.0/2.0*f[14] + 162*f[17] - 81.0/2.0*f[19] - 81.0/2.0*f[20] + (405.0/4.0)*f[24] - 81*f[27] + (81.0/4.0)*f[29] + 9*f[30] - 45.0/2.0*f[34] + 18*f[37] - 9.0/2.0*f[39] + (495.0/4.0)*f[4] - 99*f[7] + (99.0/4.0)*f[9] + z*(81*f[0] - 405.0/2.0*f[10] + (2025.0/4.0)*f[14] - 405*f[17] + (405.0/4.0)*f[19] + 162*f[20] - 405*f[24] + 324*f[27] - 81*f[29] - 81.0/2.0*f[30] + (405.0/4.0)*f[34] - 81*f[37] + (81.0/4.0)*f[39] - 405.0/2.0*f[4] + 162*f[7] - 81.0/2.0*f[9] + z*(-81.0/2.0*f[0] + (243.0/2.0)*f[10] - 1215.0/4.0*f[14] + 243*f[17] - 243.0/4.0*f[19] - 243.0/2.0*f[20] + (1215.0/4.0)*f[24] - 243*f[27] + (243.0/4.0)*f[29] + (81.0/2.0)*f[30] - 405.0/4.0*f[34] + 81*f[37] - 81.0/4.0*f[39] + (405.0/4.0)*f[4] - 81*f[7] + (81.0/4.0)*f[9])))) + z*((121.0/4.0)*f[0] - 99.0/2.0*f[10] + 81*f[14] - 81.0/2.0*f[17] + 9*f[19] + (99.0/4.0)*f[20] - 81.0/2.0*f[24] + (81.0/4.0)*f[27] - 9.0/2.0*f[29] - 11.0/2.0*f[30] + 9*f[34] - 9.0/2.0*f[37] + f[39] - 99.0/2.0*f[4] + (99.0/4.0)*f[7] - 11.0/2.0*f[9] + z*(-99.0/2.0*f[0] + (495.0/4.0)*f[10] - 405.0/2.0*f[14] + (405.0/4.0)*f[17] - 45.0/2.0*f[19] - 99*f[20] + 162*f[24] - 81*f[27] + 18*f[29] + (99.0/4.0)*f[30] - 81.0/2.0*f[34] + (81.0/4.0)*f[37] - 9.0/2.0*f[39] + 81*f[4] - 81.0/2.0*f[7] + 9*f[9] + z*((99.0/4.0)*f[0] - 297.0/4.0*f[10] + (243.0/2.0)*f[14] - 243.0/4.0*f[17] + (27.0/2.0)*f[19] + (297.0/4.0)*f[20] - 243.0/2.0*f[24] + (243.0/4.0)*f[27] - 27.0/2.0*f[29] - 99.0/4.0*f[30] + (81.0/2.0)*f[34] - 81.0/4.0*f[37] + (9.0/2.0)*f[39] - 81.0/2.0*f[4] + (81.0/4.0)*f[7] - 9.0/2.0*f[9])))) + z*(-11.0/2.0*f[0] + 9*f[10] - 9.0/2.0*f[20] + f[30] + z*(9*f[0] - 45.0/2.0*f[10] + 18*f[20] - 9.0/2.0*f[30] + z*(-9.0/2.0*f[0] + (27.0/2.0)*f[10] - 27.0/2.0*f[20] + (9.0/2.0)*f[30])));
}
#endif
#endif

#ifdef ET_PRISM
#if ORDER==3
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int dx, int dy, int dz, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[40];
  int ii=0;
  for (int k=0; k<=3; k++) {
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
    first += dz;
  }
  return f[0] + x*(-11.0/2.0*f[0] + 9*f[1] - 9.0/2.0*f[2] + f[3] + x*(9*f[0] - 45.0/2.0*f[1] + 18*f[2] - 9.0/2.0*f[3] + x*(-9.0/2.0*f[0] + (27.0/2.0)*f[1] - 27.0/2.0*f[2] + (9.0/2.0)*f[3] + z*((99.0/4.0)*f[0] - 81.0/2.0*f[10] + (243.0/2.0)*f[11] - 243.0/2.0*f[12] + (81.0/2.0)*f[13] - 297.0/4.0*f[1] + (81.0/4.0)*f[20] - 243.0/4.0*f[21] + (243.0/4.0)*f[22] - 81.0/4.0*f[23] + (297.0/4.0)*f[2] - 9.0/2.0*f[30] + (27.0/2.0)*f[31] - 27.0/2.0*f[32] + (9.0/2.0)*f[33] - 99.0/4.0*f[3] + z*(-81.0/2.0*f[0] + (405.0/4.0)*f[10] - 1215.0/4.0*f[11] + (1215.0/4.0)*f[12] - 405.0/4.0*f[13] + (243.0/2.0)*f[1] - 81*f[20] + 243*f[21] - 243*f[22] + 81*f[23] - 243.0/2.0*f[2] + (81.0/4.0)*f[30] - 243.0/4.0*f[31] + (243.0/4.0)*f[32] - 81.0/4.0*f[33] + (81.0/2.0)*f[3] + z*((81.0/4.0)*f[0] - 243.0/4.0*f[10] + (729.0/4.0)*f[11] - 729.0/4.0*f[12] + (243.0/4.0)*f[13] - 243.0/4.0*f[1] + (243.0/4.0)*f[20] - 729.0/4.0*f[21] + (729.0/4.0)*f[22] - 243.0/4.0*f[23] + (243.0/4.0)*f[2] - 81.0/4.0*f[30] + (243.0/4.0)*f[31] - 243.0/4.0*f[32] + (81.0/4.0)*f[33] - 81.0/4.0*f[3])))) + y*(-27.0/2.0*f[0] + 27*f[1] - 27.0/2.0*f[2] + (27.0/2.0)*f[4] - 27*f[5] + (27.0/2.0)*f[6] + z*((297.0/4.0)*f[0] - 243.0/2.0*f[10] + 243*f[11] - 243.0/2.0*f[12] + (243.0/2.0)*f[14] - 243*f[15] + (243.0/2.0)*f[16] - 297.0/2.0*f[1] + (243.0/4.0)*f[20] - 243.0/2.0*f[21] + (243.0/4.0)*f[22] - 243.0/4.0*f[24] + (243.0/2.0)*f[25] - 243.0/4.0*f[26] + (297.0/4.0)*f[2] - 27.0/2.0*f[30] + 27*f[31] - 27.0/2.0*f[32] + (27.0/2.0)*f[34] - 27*f[35] + (27.0/2.0)*f[36] - 297.0/4.0*f[4] + (297.0/2.0)*f[5] - 297.0/4.0*f[6] + z*(-243.0/2.0*f[0] + (1215.0/4.0)*f[10] - 1215.0/2.0*f[11] + (1215.0/4.0)*f[12] - 1215.0/4.0*f[14] + (1215.0/2.0)*f[15] - 1215.0/4.0*f[16] + 243*f[1] - 243*f[20] + 486*f[21] - 243*f[22] + 243*f[24] - 486*f[25] + 243*f[26] - 243.0/2.0*f[2] + (243.0/4.0)*f[30] - 243.0/2.0*f[31] + (243.0/4.0)*f[32] - 243.0/4.0*f[34] + (243.0/2.0)*f[35] - 243.0/4.0*f[36] + (243.0/2.0)*f[4] - 243*f[5] + (243.0/2.0)*f[6] + z*((243.0/4.0)*f[0] - 729.0/4.0*f[10] + (729.0/2.0)*f[11] - 729.0/4.0*f[12] + (729.0/4.0)*f[14] - 729.0/2.0*f[15] + (729.0/4.0)*f[16] - 243.0/2.0*f[1] + (729.0/4.0)*f[20] - 729.0/2.0*f[21] + (729.0/4.0)*f[22] - 729.0/4.0*f[24] + (729.0/2.0)*f[25] - 729.0/4.0*f[26] + (243.0/4.0)*f[2] - 243.0/4.0*f[30] + (243.0/2.0)*f[31] - 243.0/4.0*f[32] + (243.0/4.0)*f[34] - 243.0/2.0*f[35] + (243.0/4.0)*f[36] - 243.0/4.0*f[4] + (243.0/2.0)*f[5] - 243.0/4.0*f[6])))) + z*(-99.0/2.0*f[0] + 81*f[10] - 405.0/2.0*f[11] + 162*f[12] - 81.0/2.0*f[13] + (495.0/4.0)*f[1] - 81.0/2.0*f[20] + (405.0/4.0)*f[21] - 81*f[22] + (81.0/4.0)*f[23] - 99*f[2] + 9*f[30] - 45.0/2.0*f[31] + 18*f[32] - 9.0/2.0*f[33] + (99.0/4.0)*f[3] + z*(81*f[0] - 405.0/2.0*f[10] + (2025.0/4.0)*f[11] - 405*f[12] + (405.0/4.0)*f[13] - 405.0/2.0*f[1] + 162*f[20] - 405*f[21] + 324*f[22] - 81*f[23] + 162*f[2] - 81.0/2.0*f[30] + (405.0/4.0)*f[31] - 81*f[32] + (81.0/4.0)*f[33] - 81.0/2.0*f[3] + z*(-81.0/2.0*f[0] + (243.0/2.0)*f[10] - 1215.0/4.0*f[11] + 243*f[12] - 243.0/4.0*f[13] + (405.0/4.0)*f[1] - 243.0/2.0*f[20] + (1215.0/4.0)*f[21] - 243*f[22] + (243.0/4.0)*f[23] - 81*f[2] + (81.0/2.0)*f[30] - 405.0/4.0*f[31] + 81*f[32] - 81.0/4.0*f[33] + (81.0/4.0)*f[3])))) + y*(18*f[0] - 45.0/2.0*f[1] + (9.0/2.0)*f[2] - 45.0/2.0*f[4] + 27*f[5] - 9.0/2.0*f[6] + (9.0/2.0)*f[7] - 9.0/2.0*f[8] + y*(-27.0/2.0*f[0] + (27.0/2.0)*f[1] + 27*f[4] - 27*f[5] - 27.0/2.0*f[7] + (27.0/2.0)*f[8] + z*((297.0/4.0)*f[0] - 243.0/2.0*f[10] + (243.0/2.0)*f[11] + 243*f[14] - 243*f[15] - 243.0/2.0*f[17] + (243.0/2.0)*f[18] - 297.0/4.0*f[1] + (243.0/4.0)*f[20] - 243.0/4.0*f[21] - 243.0/2.0*f[24] + (243.0/2.0)*f[25] + (243.0/4.0)*f[27] - 243.0/4.0*f[28] - 27.0/2.0*f[30] + (27.0/2.0)*f[31] + 27*f[34] - 27*f[35] - 27.0/2.0*f[37] + (27.0/2.0)*f[38] - 297.0/2.0*f[4] + (297.0/2.0)*f[5] + (297.0/4.0)*f[7] - 297.0/4.0*f[8] + z*(-243.0/2.0*f[0] + (1215.0/4.0)*f[10] - 1215.0/4.0*f[11] - 1215.0/2.0*f[14] + (1215.0/2.0)*f[15] + (1215.0/4.0)*f[17] - 1215.0/4.0*f[18] + (243.0/2.0)*f[1] - 243*f[20] + 243*f[21] + 486*f[24] - 486*f[25] - 243*f[27] + 243*f[28] + (243.0/4.0)*f[30] - 243.0/4.0*f[31] - 243.0/2.0*f[34] + (243.0/2.0)*f[35] + (243.0/4.0)*f[37] - 243.0/4.0*f[38] + 243*f[4] - 243*f[5] - 243.0/2.0*f[7] + (243.0/2.0)*f[8] + z*((243.0/4.0)*f[0] - 729.0/4.0*f[10] + (729.0/4.0)*f[11] + (729.0/2.0)*f[14] - 729.0/2.0*f[15] - 729.0/4.0*f[17] + (729.0/4.0)*f[18] - 243.0/4.0*f[1] + (729.0/4.0)*f[20] - 729.0/4.0*f[21] - 729.0/2.0*f[24] + (729.0/2.0)*f[25] + (729.0/4.0)*f[27] - 729.0/4.0*f[28] - 243.0/4.0*f[30] + (243.0/4.0)*f[31] + (243.0/2.0)*f[34] - 243.0/2.0*f[35] - 243.0/4.0*f[37] + (243.0/4.0)*f[38] - 243.0/2.0*f[4] + (243.0/2.0)*f[5] + (243.0/4.0)*f[7] - 243.0/4.0*f[8])))) + z*(-99*f[0] + 162*f[10] - 405.0/2.0*f[11] + (81.0/2.0)*f[12] - 405.0/2.0*f[14] + 243*f[15] - 81.0/2.0*f[16] + (81.0/2.0)*f[17] - 81.0/2.0*f[18] + (495.0/4.0)*f[1] - 81*f[20] + (405.0/4.0)*f[21] - 81.0/4.0*f[22] + (405.0/4.0)*f[24] - 243.0/2.0*f[25] + (81.0/4.0)*f[26] - 81.0/4.0*f[27] + (81.0/4.0)*f[28] - 99.0/4.0*f[2] + 18*f[30] - 45.0/2.0*f[31] + (9.0/2.0)*f[32] - 45.0/2.0*f[34] + 27*f[35] - 9.0/2.0*f[36] + (9.0/2.0)*f[37] - 9.0/2.0*f[38] + (495.0/4.0)*f[4] - 297.0/2.0*f[5] + (99.0/4.0)*f[6] - 99.0/4.0*f[7] + (99.0/4.0)*f[8] + z*(162*f[0] - 405*f[10] + (2025.0/4.0)*f[11] - 405.0/4.0*f[12] + (2025.0/4.0)*f[14] - 1215.0/2.0*f[15] + (405.0/4.0)*f[16] - 405.0/4.0*f[17] + (405.0/4.0)*f[18] - 405.0/2.0*f[1] + 324*f[20] - 405*f[21] + 81*f[22] - 405*f[24] + 486*f[25] - 81*f[26] + 81*f[27] - 81*f[28] + (81.0/2.0)*f[2] - 81*f[30] + (405.0/4.0)*f[31] - 81.0/4.0*f[32] + (405.0/4.0)*f[34] - 243.0/2.0*f[35] + (81.0/4.0)*f[36] - 81.0/4.0*f[37] + (81.0/4.0)*f[38] - 405.0/2.0*f[4] + 243*f[5] - 81.0/2.0*f[6] + (81.0/2.0)*f[7] - 81.0/2.0*f[8] + z*(-81*f[0] + 243*f[10] - 1215.0/4.0*f[11] + (243.0/4.0)*f[12] - 1215.0/4.0*f[14] + (729.0/2.0)*f[15] - 243.0/4.0*f[16] + (243.0/4.0)*f[17] - 243.0/4.0*f[18] + (405.0/4.0)*f[1] - 243*f[20] + (1215.0/4.0)*f[21] - 243.0/4.0*f[22] + (1215.0/4.0)*f[24] - 729.0/2.0*f[25] + (243.0/4.0)*f[26] - 243.0/4.0*f[27] + (243.0/4.0)*f[28] - 81.0/4.0*f[2] + 81*f[30] - 405.0/4.0*f[31] + (81.0/4.0)*f[32] - 405.0/4.0*f[34] + (243.0/2.0)*f[35] - 81.0/4.0*f[36] + (81.0/4.0)*f[37] - 81.0/4.0*f[38] + (405.0/4.0)*f[4] - 243.0/2.0*f[5] + (81.0/4.0)*f[6] - 81.0/4.0*f[7] + (81.0/4.0)*f[8])))) + z*((121.0/4.0)*f[0] - 99.0/2.0*f[10] + 81*f[11] - 81.0/2.0*f[12] + 9*f[13] - 99.0/2.0*f[1] + (99.0/4.0)*f[20] - 81.0/2.0*f[21] + (81.0/4.0)*f[22] - 9.0/2.0*f[23] + (99.0/4.0)*f[2] - 11.0/2.0*f[30] + 9*f[31] - 9.0/2.0*f[32] + f[33] - 11.0/2.0*f[3] + z*(-99.0/2.0*f[0] + (495.0/4.0)*f[10] - 405.0/2.0*f[11] + (405.0/4.0)*f[12] - 45.0/2.0*f[13] + 81*f[1] - 99*f[20] + 162*f[21] - 81*f[22] + 18*f[23] - 81.0/2.0*f[2] + (99.0/4.0)*f[30] - 81.0/2.0*f[31] + (81.0/4.0)*f[32] - 9.0/2.0*f[33] + 9*f[3] + z*((99.0/4.0)*f[0] - 297.0/4.0*f[10] + (243.0/2.0)*f[11] - 243.0/4.0*f[12] + (27.0/2.0)*f[13] - 81.0/2.0*f[1] + (297.0/4.0)*f[20] - 243.0/2.0*f[21] + (243.0/4.0)*f[22] - 27.0/2.0*f[23] + (81.0/4.0)*f[2] - 99.0/4.0*f[30] + (81.0/2.0)*f[31] - 81.0/4.0*f[32] + (9.0/2.0)*f[33] - 9.0/2.0*f[3])))) + y*(-11.0/2.0*f[0] + 9*f[4] - 9.0/2.0*f[7] + f[9] + y*(9*f[0] - 45.0/2.0*f[4] + 18*f[7] - 9.0/2.0*f[9] + y*(-9.0/2.0*f[0] + (27.0/2.0)*f[4] - 27.0/2.0*f[7] + (9.0/2.0)*f[9] + z*((99.0/4.0)*f[0] - 81.0/2.0*f[10] + (243.0/2.0)*f[14] - 243.0/2.0*f[17] + (81.0/2.0)*f[19] + (81.0/4.0)*f[20] - 243.0/4.0*f[24] + (243.0/4.0)*f[27] - 81.0/4.0*f[29] - 9.0/2.0*f[30] + (27.0/2.0)*f[34] - 27.0/2.0*f[37] + (9.0/2.0)*f[39] - 297.0/4.0*f[4] + (297.0/4.0)*f[7] - 99.0/4.0*f[9] + z*(-81.0/2.0*f[0] + (405.0/4.0)*f[10] - 1215.0/4.0*f[14] + (1215.0/4.0)*f[17] - 405.0/4.0*f[19] - 81*f[20] + 243*f[24] - 243*f[27] + 81*f[29] + (81.0/4.0)*f[30] - 243.0/4.0*f[34] + (243.0/4.0)*f[37] - 81.0/4.0*f[39] + (243.0/2.0)*f[4] - 243.0/2.0*f[7] + (81.0/2.0)*f[9] + z*((81.0/4.0)*f[0] - 243.0/4.0*f[10] + (729.0/4.0)*f[14] - 729.0/4.0*f[17] + (243.0/4.0)*f[19] + (243.0/4.0)*f[20] - 729.0/4.0*f[24] + (729.0/4.0)*f[27] - 243.0/4.0*f[29] - 81.0/4.0*f[30] + (243.0/4.0)*f[34] - 243.0/4.0*f[37] + (81.0/4.0)*f[39] - 243.0/4.0*f[4] + (243.0/4.0)*f[7] - 81.0/4.0*f[9])))) + z*(-99.0/2.0*f[0] + 81*f[10] - 405.0/2.0*f[14] + 162*f[17] - 81.0/2.0*f[19] - 81.0/2.0*f[20] + (405.0/4.0)*f[24] - 81*f[27] + (81.0/4.0)*f[29] + 9*f[30] - 45.0/2.0*f[34] + 18*f[37] - 9.0/2.0*f[39] + (495.0/4.0)*f[4] - 99*f[7] + (99.0/4.0)*f[9] + z*(81*f[0] - 405.0/2.0*f[10] + (2025.0/4.0)*f[14] - 405*f[17] + (405.0/4.0)*f[19] + 162*f[20] - 405*f[24] + 324*f[27] - 81*f[29] - 81.0/2.0*f[30] + (405.0/4.0)*f[34] - 81*f[37] + (81.0/4.0)*f[39] - 405.0/2.0*f[4] + 162*f[7] - 81.0/2.0*f[9] + z*(-81.0/2.0*f[0] + (243.0/2.0)*f[10] - 1215.0/4.0*f[14] + 243*f[17] - 243.0/4.0*f[19] - 243.0/2.0*f[20] + (1215.0/4.0)*f[24] - 243*f[27] + (243.0/4.0)*f[29] + (81.0/2.0)*f[30] - 405.0/4.0*f[34] + 81*f[37] - 81.0/4.0*f[39] + (405.0/4.0)*f[4] - 81*f[7] + (81.0/4.0)*f[9])))) + z*((121.0/4.0)*f[0] - 99.0/2.0*f[10] + 81*f[14] - 81.0/2.0*f[17] + 9*f[19] + (99.0/4.0)*f[20] - 81.0/2.0*f[24] + (81.0/4.0)*f[27] - 9.0/2.0*f[29] - 11.0/2.0*f[30] + 9*f[34] - 9.0/2.0*f[37] + f[39] - 99.0/2.0*f[4] + (99.0/4.0)*f[7] - 11.0/2.0*f[9] + z*(-99.0/2.0*f[0] + (495.0/4.0)*f[10] - 405.0/2.0*f[14] + (405.0/4.0)*f[17] - 45.0/2.0*f[19] - 99*f[20] + 162*f[24] - 81*f[27] + 18*f[29] + (99.0/4.0)*f[30] - 81.0/2.0*f[34] + (81.0/4.0)*f[37] - 9.0/2.0*f[39] + 81*f[4] - 81.0/2.0*f[7] + 9*f[9] + z*((99.0/4.0)*f[0] - 297.0/4.0*f[10] + (243.0/2.0)*f[14] - 243.0/4.0*f[17] + (27.0/2.0)*f[19] + (297.0/4.0)*f[20] - 243.0/2.0*f[24] + (243.0/4.0)*f[27] - 27.0/2.0*f[29] - 99.0/4.0*f[30] + (81.0/2.0)*f[34] - 81.0/4.0*f[37] + (9.0/2.0)*f[39] - 81.0/2.0*f[4] + (81.0/4.0)*f[7] - 9.0/2.0*f[9])))) + z*(-11.0/2.0*f[0] + 9*f[10] - 9.0/2.0*f[20] + f[30] + z*(9*f[0] - 45.0/2.0*f[10] + 18*f[20] - 9.0/2.0*f[30] + z*(-9.0/2.0*f[0] + (27.0/2.0)*f[10] - 27.0/2.0*f[20] + (9.0/2.0)*f[30])));
}
#endif
#endif

#ifdef ET_PYRAMID
#if ORDER==1
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[5];
  int ii=0;
  for (int k=0; k<=4; k++)
      f[k] = getValue(coefficients, first+k)[component];
  /*
  for (int k=0; k<=1; k++) {
      int offsety = 0;
      for (int i=0; i<=1-k; i++) {
        int offsetx = 0;
        for (int j=0; j<=1-k; j++) {
          f[ii] = getValue(coefficients, first+offsetx+offsety)[component];
          offsetx += 1;
          ii++;
        }
        offsety += 1+1;
      }
    first += (1-k+1)*(1-k+1);
  }
  */
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3])) + y*(-f[0] + f[2]) + z*(-f[0] + f[4]);
}
#endif
#endif

#ifdef ET_PYRAMID
#if ORDER==1
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[5];
  int ii=0;
  for (int k=0; k<=4; k++)
      f[k] = getValue(coefficients, first+k).xyz;
  /*
  for (int k=0; k<=1; k++) {
      int offsety = 0;
      for (int i=0; i<=1-k; i++) {
        int offsetx = 0;
        for (int j=0; j<=1-k; j++) {
          f[ii] = getValue(coefficients, first+offsetx+offsety).xyz;
          offsetx += 1;
          ii++;
        }
        offsety += 1+1;
      }
    first += (1-k+1)*(1-k+1);
  }
  */
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3])) + y*(-f[0] + f[2]) + z*(-f[0] + f[4]);
}
#endif
#endif

#ifdef ET_PYRAMID
#if ORDER==2
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[5];
  int ii=0;
  for (int k=0; k<=4; k++)
      f[k] = getValue(coefficients, first+k)[component];
  /*
  for (int k=0; k<=2; k++) {
      int offsety = 0;
      for (int i=0; i<=2-k; i++) {
        int offsetx = 0;
        for (int j=0; j<=2-k; j++) {
          f[ii] = getValue(coefficients, first+offsetx+offsety)[component];
          offsetx += 1;
          ii++;
        }
        offsety += 2+1;
      }
    first += (2-k+1)*(2-k+1);
  }
  */
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3])) + y*(-f[0] + f[2]) + z*(-f[0] + f[4]);
}
#endif
#endif

#ifdef ET_PYRAMID
#if ORDER==2
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[5];
  int ii=0;
  for (int k=0; k<=4; k++)
      f[k] = getValue(coefficients, first+k).xyz;
  /*
  for (int k=0; k<=2; k++) {
      int offsety = 0;
      for (int i=0; i<=2-k; i++) {
        int offsetx = 0;
        for (int j=0; j<=2-k; j++) {
          f[ii] = getValue(coefficients, first+offsetx+offsety).xyz;
          offsetx += 1;
          ii++;
        }
        offsety += 2+1;
      }
    first += (2-k+1)*(2-k+1);
  }
  */
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3])) + y*(-f[0] + f[2]) + z*(-f[0] + f[4]);
}
#endif
#endif

#ifdef ET_PYRAMID
#if ORDER==3
float EvaluateElementInternal(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float f[5];
  int ii=0;
  for (int k=0; k<=4; k++)
      f[k] = getValue(coefficients, first+k)[component];
  /*
  for (int k=0; k<=3; k++) {
      int offsety = 0;
      for (int i=0; i<=3-k; i++) {
        int offsetx = 0;
        for (int j=0; j<=3-k; j++) {
          f[ii] = getValue(coefficients, first+offsetx+offsety)[component];
          offsetx += 1;
          ii++;
        }
        offsety += 3+1;
      }
    first += (3-k+1)*(3-k+1);
  }
  */
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3])) + y*(-f[0] + f[2]) + z*(-f[0] + f[4]);
}
#endif
#endif

#ifdef ET_PYRAMID
#if ORDER==3
vec3 EvaluateElementInternalVec(samplerBuffer coefficients, vec3 lam, int first, int component) {
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  vec3 f[5];
  int ii=0;
  for (int k=0; k<=4; k++)
      f[k] = getValue(coefficients, first+k).xyz;
  /*
  for (int k=0; k<=3; k++) {
      int offsety = 0;
      for (int i=0; i<=3-k; i++) {
        int offsetx = 0;
        for (int j=0; j<=3-k; j++) {
          f[ii] = getValue(coefficients, first+offsetx+offsety).xyz;
          offsetx += 1;
          ii++;
        }
        offsety += 3+1;
      }
    first += (3-k+1)*(3-k+1);
  }
  */
  return f[0] + x*(-f[0] + f[1] + y*(f[0] - f[1] - f[2] + f[3])) + y*(-f[0] + f[2]) + z*(-f[0] + f[4]);
}
#endif
#endif

#ifdef ET_SEGM
float EvaluateElement(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
    int n = subdivision+1;
    int N = ORDER*n+1;
    int values_per_element = N;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);

    int X = ORDER*x;

    int first = element*values_per_element + X;
    return EvaluateElementInternal(values, lam, first, component);
}
#endif
#ifdef ET_SEGM
vec3 EvaluateElementVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
    int n = subdivision+1;
    int N = ORDER*n+1;
    int values_per_element = N;
    vec3 lamn = lam*(n);
    lam = lamn-floor(lamn);
    int x = int(lamn.x);

    int X = ORDER*x;

    int first = element*values_per_element + X;
    return EvaluateElementInternalVec(values, lam, first, component);
}
#endif
#ifdef ET_TRIG
float EvaluateElement(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return EvaluateElementInternal(values, lam, first, dx, dy, component);
}
#endif
#ifdef ET_TRIG
vec3 EvaluateElementVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return EvaluateElementInternalVec(values, lam, first, dx, dy, component);
}
#endif
#ifdef ET_TET
float EvaluateElement(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam, int component) {
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
  return EvaluateElementInternal( element, coefficients, N, d, s, special_order, vec3(x,y,z), component);
}
#endif
#ifdef ET_TET
vec3 EvaluateElementVec(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam, int component) {
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
  return EvaluateElementInternalVec( element, coefficients, N, d, s, special_order, vec3(x,y,z), component);
}
#endif
#ifdef ET_QUAD
float EvaluateElement(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return EvaluateElementInternal(values, lam, first, dy, component);
}
#endif
#ifdef ET_QUAD
vec3 EvaluateElementVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return EvaluateElementInternalVec(values, lam, first, dy, component);
}
#endif
#ifdef ET_HEX
float EvaluateElement(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return EvaluateElementInternal(values, lam, first, dy, dz, component);
}
#endif
#ifdef ET_HEX
vec3 EvaluateElementVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return EvaluateElementInternalVec(values, lam, first, dy, dz, component);
}
#endif
#ifdef ET_PRISM
float EvaluateElement(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return EvaluateElementInternal(values, lam, first, dx, dy, dz, component);
}
#endif
#ifdef ET_PRISM
vec3 EvaluateElementVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
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
    return EvaluateElementInternalVec(values, lam, first, dx, dy, dz, component);
}
#endif
#ifdef ET_PYRAMID
float EvaluateElement(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
    int N = 1+1;
    int values_per_element = N*(N+1)*(2*N+1)/6;
    int first = values_per_element*element;
    return EvaluateElementInternal(values, lam, first, component);
}
#endif
#ifdef ET_PYRAMID
vec3 EvaluateElementVec(int element, samplerBuffer values, int order, int subdivision, vec3 lam, int component) {
    int N = 1+1;
    int values_per_element = N*(N+1)*(2*N+1)/6;
    int first = values_per_element*element;
    return EvaluateElementInternalVec(values, lam, first, component);
}
#endif
