struct Mesh {
    isamplerBuffer elements;
    samplerBuffer curved_elements;
    samplerBuffer vertices;
};

int elements2d_stride = 5;
int elements2d_index_offset = 3;
int elements2d_curved_offset = 4;

int elements3d_stride = 6;
int elements3d_index_offset = 4;

int elements2d_curved_stride = 6;
int elements2d_curved_normals_offset = 0;
int elements2d_curved_points_offset = 3;

struct Element2d {
    int index;
    int curved_index;
    int vertices[3];
    vec3 pos[3];
    vec3 normals[3];
};

struct Element3d {
    int index;
    int vertices[4];
    vec3 pos[4];
};

vec3 interpolatePoint(Mesh mesh, Element2d el, vec2 lam) {
    float x = lam.x;
    float y = lam.y;
    if(el.curved_index<0) {
      return x*el.pos[0] + y*el.pos[1]+(1-x-y)*el.pos[2];
    }
    else {
      vec3 f[6];
      f[0] = el.pos[2]; // getSubTrigValue(0,0);
      f[2] = el.pos[0]; // getSubTrigValue(2,0);
      f[5] = el.pos[1]; // getSubTrigValue(0,2);
      f[1] = texelFetch(mesh.curved_elements, elements2d_curved_stride*el.curved_index + elements2d_curved_points_offset+0).xyz; // getSubTrigValue(1,0);
      f[3] = texelFetch(mesh.curved_elements, elements2d_curved_stride*el.curved_index + elements2d_curved_points_offset+1).xyz; // getSubTrigValue(0,1);
      f[4] = texelFetch(mesh.curved_elements, elements2d_curved_stride*el.curved_index + elements2d_curved_points_offset+2).xyz; // getSubTrigValue(1,1);
      return 1.0*f[0] + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]);
    }
}

void calcNormals(inout Element2d el) {
  // flat element, normal is constant
  vec3 n = cross(el.pos[1]-el.pos[0], el.pos[2]-el.pos[0]);
  el.normals[0] = n;
  el.normals[1] = n;
  el.normals[2] = n;
}

Element2d getElement2d(Element3d tet, int face ) {
  Element2d trig;
  trig.index = tet.index;
  trig.curved_index = -1;
  int counter = 0;
  for (int i=0; i<4; i++) {
    if(i==face) continue;
    trig.vertices[counter] = tet.vertices[i];
    trig.pos[counter] = tet.pos[i];
    counter++;
  }
  if(face==2) {
    vec3 t=trig.pos[2];
    trig.pos[2] = trig.pos[1];
    trig.pos[1] = t;
    int i=trig.vertices[2];
    trig.vertices[2] = trig.vertices[1];
    trig.vertices[1] = i;
  }

  calcNormals(trig);
  return trig;
}

Element2d getElement2d(Mesh mesh, int ei ) {
    Element2d el;
    int stride = elements2d_stride;

    for (int i=0; i<3; i++) {
      el.vertices[i] = texelFetch(mesh.elements, stride*ei+i).r;
      el.pos[i] = texelFetch(mesh.vertices, el.vertices[i]).xyz;
    }
    el.index = texelFetch(mesh.elements, stride*ei+elements2d_index_offset).r;
    el.curved_index = texelFetch(mesh.elements, stride*ei+elements2d_curved_offset).r;
    if(el.curved_index>=0) {
        // have curved element
        for (int i=0; i<3; i++)
          el.normals[i] = texelFetch(mesh.curved_elements, elements2d_curved_stride*el.curved_index + elements2d_curved_normals_offset+i).xyz;
    }
    else {
      calcNormals(el);
    }

    return el;
}

Element3d getElement3d(Mesh mesh, int ei ) {
    Element3d el;
    int stride = elements3d_stride;

    for (int i=0; i<4; i++) {
      el.vertices[i] = texelFetch(mesh.elements, stride*ei+i).r;
      el.pos[i] = texelFetch(mesh.vertices, el.vertices[i]).xyz;
    }
    el.index = texelFetch(mesh.elements, stride*ei+elements3d_index_offset).r;
    return el;
}


vec3 MapColor(float value)
{
    value = clamp(value, 0.0, 1.0);
    vec3 res;
    res.r = clamp(2.0-4.0*value, 0.0, 1.0);
    res.g = clamp(2.0-4.0*abs(0.5-value), 0.0, 1.0);
    res.b = clamp(4.0*value - 2.0, 0.0, 1.0);
    return res;
}

// find containing subtriangle
void getSubTrigStart(int order, int subdivision, inout vec3 lam, inout int x, inout int y, inout int dy)
{
    int n = subdivision+1;
    vec3 lamn = lam*(n);
    vec3 floor_lamn = floor(lamn);
    lam = lamn-floor_lamn;
    x=int(lamn.x);
    y=int(lamn.y);
    int z=int(lamn.z);
    int missing = n-x-y-z;

    if(missing==1) {
        dy=1;
    }
    if(missing==2) {
        dy=-1;
        x += 1;
        y += 1;
        lam.x = 1-lam.x;
        lam.y = 1-lam.y;
    }
}

float getSubTrigValue(int element, samplerBuffer coefficients, int order, int subdivision, int xtrig, int ytrig, int dy, int x, int y) {
    int t = (subdivision+1); // number of small edges in large edge
    int n = (order+1)*t; // points on edge of subtrigs
    int N = order*(subdivision+1)+1; // points on edge of trig
    int m = N-ytrig*order;
    int values_per_element = N*(N+1)/2;
    int offset=0;
    if(dy==1) {
        offset  = values_per_element - m*(m+1)/2 + xtrig*order;
        offset += m*(m+1)/2-(m-y)*(m-y+1)/2 + x;
    }
    else {
        offset  = values_per_element - m*(m+1)/2 + xtrig*order;
        offset -= (m+y)*(m+y+1)/2 - m*(m+1)/2 + x;
    }
    return texelFetch(coefficients, element*values_per_element+offset).r;
}

float getSubTetValue(int element, samplerBuffer coefficients, int order, int subdivision, int xtrig, int ytrig, int dy, int x, int y, int z) {
    // todo: implement subdivision
    int n = order+1;
    int values_per_element = n*(n+1)*(n+2)/6;
    int offset = x;

    offset += n*(n+1)*(n+2)/6 - (n-z)*(n-z+1)*(n-z+2)/6;
    n = n-z;
    offset += n*(n+1)/2 - (n-y)*(n-y+1)/2;

    return texelFetch(coefficients, element*values_per_element+offset).r;
}

float InterpolateTrig(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam) {
  int trigx=0;
  int trigy=0;
  int dy=1;
  getSubTrigStart(order, subdivision, lam, trigx, trigy, dy);
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float value = 0.0;
  if (order==1) {
    float f[3];
    f[0] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1);
    value = 1.0*f[0] - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]);
  }
  if (order==2) {
    float f[6];
    f[0] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0);
    f[3] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1);
    f[4] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1);
    f[5] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2);
    value = 1.0*f[0] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]);
  }
  if (order==3) {
    float f[10];
    f[0] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0);
    f[3] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 3,0);
    f[4] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1);
    f[5] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1);
    f[6] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,1);
    f[7] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2);
    f[8] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,2);
    f[9] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,3);
    value = 1.0*f[0] + pow(x, 3)*(-4.5*f[0] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3]) - pow(x, 2)*y*(13.5*f[0] - 27.0*f[1] + 13.5*f[2] - 13.5*f[4] + 27.0*f[5] - 13.5*f[6]) + pow(x, 2)*(9.0*f[0] - 22.5*f[1] + 18.0*f[2] - 4.49999999999999*f[3]) - x*pow(y, 2)*(13.5*f[0] - 13.5*f[1] - 27.0*f[4] + 27.0*f[5] + 13.5*f[7] - 13.5*f[8]) + x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8] + 4.9960036108132e-16*f[9]) - x*(5.5*f[0] - 9.0*f[1] + 4.5*f[2] - 0.999999999999998*f[3]) + pow(y, 3)*(-4.5*f[0] + 13.5*f[4] - 13.5*f[7] + 4.5*f[9]) + pow(y, 2)*(9.0*f[0] - 22.5*f[4] + 18.0*f[7] - 4.49999999999999*f[9]) - y*(5.5*f[0] - 9.0*f[4] + 4.5*f[7] - 0.999999999999998*f[9]);
  }
  if (order==4) {
    float f[15];
    f[0] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0);
    f[3] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 3,0);
    f[4] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 4,0);
    f[5] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1);
    f[6] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1);
    f[7] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,1);
    f[8] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 3,1);
    f[9] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2);
    f[10] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,2);
    f[11] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,2);
    f[12] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,3);
    f[13] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,3);
    f[14] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,4);
    value = 1.0*f[0] + pow(x, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[1] + 64.0*f[2] - 42.6666666666667*f[3] + 10.6666666666667*f[4]) - pow(x, 3)*y*(-42.6666666666667*f[0] + 3.5527136788005e-15*f[14] + 128.0*f[1] - 128.0*f[2] + 42.6666666666667*f[3] + 42.6666666666667*f[5] - 128.0*f[6] + 128.0*f[7] - 42.6666666666667*f[8]) + pow(x, 3)*(-26.6666666666667*f[0] + 96.0*f[1] - 128.0*f[2] + 74.6666666666667*f[3] - 16.0*f[4]) + pow(x, 2)*pow(y, 2)*(64.0*f[0] - 128.0*f[10] + 64.0*f[11] - 128.0*f[1] + 64.0*f[2] - 128.0*f[5] + 256.0*f[6] - 128.0*f[7] + 64.0*f[9]) + pow(x, 2)*y*(-80.0*f[0] + 32.0*f[10] - 16.0*f[11] + 9.473903143468e-15*f[12] + 4.736951571734e-15*f[13] + 3.25665420556713e-15*f[14] + 192.0*f[1] - 144.0*f[2] + 32.0*f[3] + 96.0*f[5] - 224.0*f[6] + 160.0*f[7] - 32.0*f[8] - 16.0*f[9]) + pow(x, 2)*(23.3333333333333*f[0] - 69.3333333333333*f[1] + 76.0*f[2] - 37.3333333333333*f[3] + 7.33333333333334*f[4]) + x*pow(y, 3)*(42.6666666666667*f[0] - 128.0*f[10] - 42.6666666666667*f[12] + 42.6666666666667*f[13] + 3.5527136788005e-15*f[14] - 42.6666666666667*f[1] - 128.0*f[5] + 128.0*f[6] + 128.0*f[9]) - x*pow(y, 2)*(80.0*f[0] - 160.0*f[10] + 16.0*f[11] - 32.0*f[12] + 32.0*f[13] + 3.5527136788005e-15*f[14] - 96.0*f[1] + 16.0*f[2] - 192.0*f[5] + 224.0*f[6] - 32.0*f[7] + 144.0*f[9]) + x*y*(46.6666666666667*f[0] - 32.0*f[10] + 4.0*f[11] - 5.33333333333334*f[12] + 5.33333333333333*f[13] + 1.03620815631681e-15*f[14] - 69.3333333333333*f[1] + 28.0*f[2] - 5.33333333333333*f[3] - 69.3333333333333*f[5] + 96.0*f[6] - 32.0*f[7] + 5.33333333333333*f[8] + 28.0*f[9]) - x*(8.33333333333333*f[0] - 16.0*f[1] + 12.0*f[2] - 5.33333333333334*f[3] + 1.0*f[4]) + pow(y, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[12] + 10.6666666666667*f[14] - 42.6666666666667*f[5] + 64.0*f[9]) + pow(y, 3)*(-26.6666666666667*f[0] + 74.6666666666667*f[12] - 16.0*f[14] + 96.0*f[5] - 128.0*f[9]) + pow(y, 2)*(23.3333333333333*f[0] - 37.3333333333333*f[12] + 7.33333333333334*f[14] - 69.3333333333333*f[5] + 76.0*f[9]) - y*(8.33333333333333*f[0] - 5.33333333333334*f[12] + 1.0*f[14] - 16.0*f[5] + 12.0*f[9]);
  }
  return value;
}

float InterpolateTet(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam) {
  int trigx=0;
  int trigy=0;
  int dy=1;
  getSubTrigStart(order, subdivision, lam, trigx, trigy, dy);
  float x = lam.x;
  float y = lam.y;
  float z = lam.z;
  float value = 0.0;
  if (order==1) {
    float f[4];
    f[0] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,0);
    f[1] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,0);
    f[2] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,0);
    f[3] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,1);
    value = 1.0*f[0] - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]) - 1.0*z*(f[0] - f[3]);
  }
  if (order==2) {
    float f[10];
    f[0] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,0);
    f[1] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,0);
    f[2] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0,0);
    f[3] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,0);
    f[4] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1,0);
    f[5] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2,0);
    f[6] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,1);
    f[7] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,1);
    f[8] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,1);
    f[9] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,2);
    value = 1.0*f[0] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) + 4.0*x*z*(f[0] - f[1] - f[6] + f[7]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) + 4.0*y*z*(f[0] - f[3] - f[6] + f[8]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]) + pow(z, 2)*(2.0*f[0] - 4.0*f[6] + 2.0*f[9]) - z*(3.0*f[0] - 4.0*f[6] + 1.0*f[9]);
  }
  if (order==3) {
    float f[20];
    f[0] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,0);
    f[1] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,0);
    f[2] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0,0);
    f[3] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 3,0,0);
    f[4] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,0);
    f[5] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1,0);
    f[6] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,1,0);
    f[7] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2,0);
    f[8] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,2,0);
    f[9] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,3,0);
    f[10] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,1);
    f[11] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,1);
    f[12] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0,1);
    f[13] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,1);
    f[14] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1,1);
    f[15] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2,1);
    f[16] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,2);
    f[17] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,2);
    f[18] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,2);
    f[19] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,3);
    value = 1.0*f[0] + pow(x, 3)*(-4.5*f[0] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3]) - pow(x, 2)*y*(13.5*f[0] - 27.0*f[1] + 13.5*f[2] - 13.5*f[4] + 27.0*f[5] - 13.5*f[6]) - pow(x, 2)*z*(13.5*f[0] - 13.5*f[10] + 27.0*f[11] - 13.5*f[12] - 27.0*f[1] + 13.5*f[2]) + pow(x, 2)*(9.0*f[0] - 22.5*f[1] + 18.0*f[2] - 4.49999999999999*f[3]) - x*pow(y, 2)*(13.5*f[0] - 13.5*f[1] - 27.0*f[4] + 27.0*f[5] + 13.5*f[7] - 13.5*f[8]) - 27.0*x*y*z*(f[0] - f[10] + f[11] + f[13] - f[14] - f[1] - f[4] + f[5]) + x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8] + 4.9960036108132e-16*f[9]) - x*pow(z, 2)*(13.5*f[0] - 27.0*f[10] + 27.0*f[11] + 13.5*f[16] - 13.5*f[17] - 13.5*f[1]) + x*z*(18.0*f[0] - 22.5*f[10] + 27.0*f[11] - 4.5*f[12] + 4.5*f[16] - 4.5*f[17] + 4.9960036108132e-16*f[19] - 22.5*f[1] + 4.5*f[2]) - x*(5.5*f[0] - 9.0*f[1] + 4.5*f[2] - 0.999999999999998*f[3]) + pow(y, 3)*(-4.5*f[0] + 13.5*f[4] - 13.5*f[7] + 4.5*f[9]) - pow(y, 2)*z*(13.5*f[0] - 13.5*f[10] + 27.0*f[13] - 13.5*f[15] - 27.0*f[4] + 13.5*f[7]) + pow(y, 2)*(9.0*f[0] - 22.5*f[4] + 18.0*f[7] - 4.49999999999999*f[9]) - y*pow(z, 2)*(13.5*f[0] - 27.0*f[10] + 27.0*f[13] + 13.5*f[16] - 13.5*f[18] - 13.5*f[4]) + y*z*(18.0*f[0] - 22.5*f[10] + 27.0*f[13] - 4.5*f[15] + 4.5*f[16] - 4.5*f[18] + 4.9960036108132e-16*f[19] - 22.5*f[4] + 4.5*f[7]) - y*(5.5*f[0] - 9.0*f[4] + 4.5*f[7] - 0.999999999999998*f[9]) + pow(z, 3)*(-4.5*f[0] + 13.5*f[10] - 13.5*f[16] + 4.5*f[19]) + pow(z, 2)*(9.0*f[0] - 22.5*f[10] + 18.0*f[16] - 4.49999999999999*f[19]) - z*(5.5*f[0] - 9.0*f[10] + 4.5*f[16] - 0.999999999999998*f[19]);
  }
  if (order==4) {
    float f[35];
    f[0] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,0);
    f[1] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,0);
    f[2] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0,0);
    f[3] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 3,0,0);
    f[4] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 4,0,0);
    f[5] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,0);
    f[6] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1,0);
    f[7] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,1,0);
    f[8] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 3,1,0);
    f[9] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2,0);
    f[10] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,2,0);
    f[11] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,2,0);
    f[12] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,3,0);
    f[13] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,3,0);
    f[14] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,4,0);
    f[15] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,1);
    f[16] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,1);
    f[17] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0,1);
    f[18] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 3,0,1);
    f[19] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,1);
    f[20] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1,1);
    f[21] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,1,1);
    f[22] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2,1);
    f[23] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,2,1);
    f[24] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,3,1);
    f[25] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,2);
    f[26] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,2);
    f[27] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0,2);
    f[28] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,2);
    f[29] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1,2);
    f[30] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2,2);
    f[31] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,3);
    f[32] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0,3);
    f[33] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1,3);
    f[34] = getSubTetValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0,4);
    value = 1.0*f[0] + pow(x, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[1] + 64.0*f[2] - 42.6666666666667*f[3] + 10.6666666666667*f[4]) - pow(x, 3)*y*(-42.6666666666667*f[0] + 3.5527136788005e-15*f[14] + 128.0*f[1] - 128.0*f[2] + 42.6666666666667*f[3] + 42.6666666666667*f[5] - 128.0*f[6] + 128.0*f[7] - 42.6666666666667*f[8]) - pow(x, 3)*z*(-42.6666666666667*f[0] + 42.6666666666667*f[15] - 128.0*f[16] + 128.0*f[17] - 42.6666666666667*f[18] + 128.0*f[1] - 128.0*f[2] + 3.5527136788005e-15*f[34] + 42.6666666666667*f[3]) + pow(x, 3)*(-26.6666666666667*f[0] + 96.0*f[1] - 128.0*f[2] + 74.6666666666667*f[3] - 16.0*f[4]) + pow(x, 2)*pow(y, 2)*(64.0*f[0] - 128.0*f[10] + 64.0*f[11] - 128.0*f[1] + 64.0*f[2] - 128.0*f[5] + 256.0*f[6] - 128.0*f[7] + 64.0*f[9]) + pow(x, 2)*y*z*(128.0*f[0] - 128.0*f[15] + 256.0*f[16] - 128.0*f[17] + 128.0*f[19] - 256.0*f[1] - 256.0*f[20] + 128.0*f[21] + 128.0*f[2] + 1.1842378929335e-14*f[31] + 2.368475785867e-15*f[33] - 9.473903143468e-15*f[34] - 128.0*f[5] + 256.0*f[6] - 128.0*f[7]) + pow(x, 2)*y*(-80.0*f[0] + 32.0*f[10] - 16.0*f[11] + 9.473903143468e-15*f[12] + 4.736951571734e-15*f[13] + 3.25665420556713e-15*f[14] + 192.0*f[1] - 144.0*f[2] + 32.0*f[3] + 96.0*f[5] - 224.0*f[6] + 160.0*f[7] - 32.0*f[8] - 16.0*f[9]) + pow(x, 2)*pow(z, 2)*(64.0*f[0] - 128.0*f[15] + 256.0*f[16] - 128.0*f[17] - 128.0*f[1] + 64.0*f[25] - 128.0*f[26] + 64.0*f[27] + 64.0*f[2]) + pow(x, 2)*z*(-80.0*f[0] + 96.0*f[15] - 224.0*f[16] + 160.0*f[17] - 32.0*f[18] + 192.0*f[1] - 16.0*f[25] + 32.0*f[26] - 16.0*f[27] - 144.0*f[2] + 9.473903143468e-15*f[31] + 4.736951571734e-15*f[32] + 3.25665420556713e-15*f[34] + 32.0*f[3]) + pow(x, 2)*(23.3333333333333*f[0] - 69.3333333333333*f[1] + 76.0*f[2] - 37.3333333333333*f[3] + 7.33333333333334*f[4]) + x*pow(y, 3)*(42.6666666666667*f[0] - 128.0*f[10] - 42.6666666666667*f[12] + 42.6666666666667*f[13] + 3.5527136788005e-15*f[14] - 42.6666666666667*f[1] - 128.0*f[5] + 128.0*f[6] + 128.0*f[9]) + x*pow(y, 2)*z*(128.0*f[0] - 128.0*f[10] - 128.0*f[15] + 128.0*f[16] + 256.0*f[19] - 128.0*f[1] - 256.0*f[20] - 128.0*f[22] + 128.0*f[23] + 7.105427357601e-15*f[31] - 7.105427357601e-15*f[34] - 256.0*f[5] + 256.0*f[6] + 128.0*f[9]) - x*pow(y, 2)*(80.0*f[0] - 160.0*f[10] + 16.0*f[11] - 32.0*f[12] + 32.0*f[13] + 3.5527136788005e-15*f[14] - 96.0*f[1] + 16.0*f[2] - 192.0*f[5] + 224.0*f[6] - 32.0*f[7] + 144.0*f[9]) + x*y*pow(z, 2)*(128.0*f[0] - 256.0*f[15] + 256.0*f[16] + 256.0*f[19] - 128.0*f[1] - 256.0*f[20] + 128.0*f[25] - 128.0*f[26] - 128.0*f[28] + 128.0*f[29] - 128.0*f[5] + 128.0*f[6]) + x*y*z*(-160.0*f[0] + 32.0*f[10] + 192.0*f[15] - 224.0*f[16] + 32.0*f[17] - 224.0*f[19] + 192.0*f[1] + 256.0*f[20] - 32.0*f[21] + 32.0*f[22] - 32.0*f[23] - 32.0*f[25] + 32.0*f[26] + 32.0*f[28] - 32.0*f[29] - 32.0*f[2] + 1.83556873404693e-14*f[31] + 3.5527136788005e-15*f[32] + 2.96059473233375e-15*f[33] + 5.03301104496738e-15*f[34] + 192.0*f[5] - 224.0*f[6] + 32.0*f[7] - 32.0*f[9]) + x*y*(46.6666666666667*f[0] - 32.0*f[10] + 4.0*f[11] - 5.33333333333334*f[12] + 5.33333333333333*f[13] + 1.03620815631681e-15*f[14] - 69.3333333333333*f[1] + 28.0*f[2] - 5.33333333333333*f[3] - 69.3333333333333*f[5] + 96.0*f[6] - 32.0*f[7] + 5.33333333333333*f[8] + 28.0*f[9]) + x*pow(z, 3)*(42.6666666666667*f[0] - 128.0*f[15] + 128.0*f[16] - 42.6666666666667*f[1] + 128.0*f[25] - 128.0*f[26] - 42.6666666666667*f[31] + 42.6666666666667*f[32] + 3.5527136788005e-15*f[34]) - x*pow(z, 2)*(80.0*f[0] - 192.0*f[15] + 224.0*f[16] - 32.0*f[17] - 96.0*f[1] + 144.0*f[25] - 160.0*f[26] + 16.0*f[27] + 16.0*f[2] - 32.0*f[31] + 32.0*f[32] + 3.5527136788005e-15*f[34]) + x*z*(46.6666666666667*f[0] - 69.3333333333333*f[15] + 96.0*f[16] - 32.0*f[17] + 5.33333333333333*f[18] - 69.3333333333333*f[1] + 28.0*f[25] - 32.0*f[26] + 4.0*f[27] + 28.0*f[2] - 5.33333333333334*f[31] + 5.33333333333333*f[32] + 1.03620815631681e-15*f[34] - 5.33333333333333*f[3]) - x*(8.33333333333333*f[0] - 16.0*f[1] + 12.0*f[2] - 5.33333333333334*f[3] + 1.0*f[4]) + pow(y, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[12] + 10.6666666666667*f[14] - 42.6666666666667*f[5] + 64.0*f[9]) - pow(y, 3)*z*(-42.6666666666667*f[0] + 42.6666666666667*f[12] + 42.6666666666667*f[15] - 128.0*f[19] + 128.0*f[22] - 42.6666666666667*f[24] + 3.5527136788005e-15*f[34] + 128.0*f[5] - 128.0*f[9]) + pow(y, 3)*(-26.6666666666667*f[0] + 74.6666666666667*f[12] - 16.0*f[14] + 96.0*f[5] - 128.0*f[9]) + pow(y, 2)*pow(z, 2)*(64.0*f[0] - 128.0*f[15] + 256.0*f[19] - 128.0*f[22] + 64.0*f[25] - 128.0*f[28] + 64.0*f[30] - 128.0*f[5] + 64.0*f[9]) + pow(y, 2)*z*(-80.0*f[0] + 32.0*f[12] + 96.0*f[15] - 224.0*f[19] + 160.0*f[22] - 32.0*f[24] - 16.0*f[25] + 32.0*f[28] - 16.0*f[30] + 9.473903143468e-15*f[31] + 4.736951571734e-15*f[33] + 3.25665420556713e-15*f[34] + 192.0*f[5] - 144.0*f[9]) + pow(y, 2)*(23.3333333333333*f[0] - 37.3333333333333*f[12] + 7.33333333333334*f[14] - 69.3333333333333*f[5] + 76.0*f[9]) + y*pow(z, 3)*(42.6666666666667*f[0] - 128.0*f[15] + 128.0*f[19] + 128.0*f[25] - 128.0*f[28] - 42.6666666666667*f[31] + 42.6666666666667*f[33] + 3.5527136788005e-15*f[34] - 42.6666666666667*f[5]) - y*pow(z, 2)*(80.0*f[0] - 192.0*f[15] + 224.0*f[19] - 32.0*f[22] + 144.0*f[25] - 160.0*f[28] + 16.0*f[30] - 32.0*f[31] + 32.0*f[33] + 3.5527136788005e-15*f[34] - 96.0*f[5] + 16.0*f[9]) + y*z*(46.6666666666667*f[0] - 5.33333333333333*f[12] - 69.3333333333333*f[15] + 96.0*f[19] - 32.0*f[22] + 5.33333333333333*f[24] + 28.0*f[25] - 32.0*f[28] + 4.0*f[30] - 5.33333333333334*f[31] + 5.33333333333333*f[33] + 1.03620815631681e-15*f[34] - 69.3333333333333*f[5] + 28.0*f[9]) - y*(8.33333333333333*f[0] - 5.33333333333334*f[12] + 1.0*f[14] - 16.0*f[5] + 12.0*f[9]) + pow(z, 4)*(10.6666666666667*f[0] - 42.6666666666667*f[15] + 64.0*f[25] - 42.6666666666667*f[31] + 10.6666666666667*f[34]) + pow(z, 3)*(-26.6666666666667*f[0] + 96.0*f[15] - 128.0*f[25] + 74.6666666666667*f[31] - 16.0*f[34]) + pow(z, 2)*(23.3333333333333*f[0] - 69.3333333333333*f[15] + 76.0*f[25] - 37.3333333333333*f[31] + 7.33333333333334*f[34]) - z*(8.33333333333333*f[0] - 16.0*f[15] + 12.0*f[25] - 5.33333333333334*f[31] + 1.0*f[34]);
  }
  return value;
}

