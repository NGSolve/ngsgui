#version 150
uniform samplerBuffer coefficients;
uniform float colormap_min, colormap_max;
uniform bool colormap_linear;
uniform int element_type;
uniform vec4 clipping_plane;
uniform bool do_clipping;
uniform int subdivision;
uniform int order;

in VertexData
{
  vec3 pos;
  flat int element;
  vec3 lam;
} inData;

out vec4 FragColor;


int getOffset(int n, int xi, int yi) {
    yi = max(0,yi);
    yi = min(n-1,yi);
    xi = max(0,xi);
    xi = min(xi,n-1-yi);
    return n*(n+1)/2-( (n-yi)*(n-yi+1)/2) + xi;
}

int Round(float x) {
    return int(x+0.5);
}

float loadValue(int i) {
    int n = (order*(subdivision+1))+1;
    int values_per_element = n*(n+1)/2;
    return texelFetch(coefficients, inData.element*values_per_element+i).r;
}

// find containing subtriangle
void getSubTrigStart(inout vec3 lam, inout int x, inout int y, inout int dy)
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

float getSubTrigValue(int xtrig, int ytrig, int dy, int x, int y) {
    int t = (subdivision+1); // number of small edges in large edge
    int n = (order+1)*t; // points on edge of subtrigs
    int N = order*(subdivision+1)+1; // points on edge of trig
    int m = N-ytrig*order;
    int offset=0;
    if(dy==1) {
        offset  = (N)*(N+1)/2 - m*(m+1)/2 + xtrig*order;
        offset += m*(m+1)/2-(m-y)*(m-y+1)/2 + x;
    }
    else {
        offset  = (N)*(N+1)/2 - m*(m+1)/2 + xtrig*order;
        offset -= (m+y)*(m+y+1)/2 - m*(m+1)/2 + x;
    }
    return loadValue(offset);
}

float Interpolate() {
  vec3 lam = inData.lam;
  int trigx=0;
  int trigy=0;
  int dy=1;
  getSubTrigStart(lam, trigx, trigy, dy);
  float f[6];
  float x = lam.x;
  float y = lam.y;
  if(order==1) {
    f[0] = getSubTrigValue(trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(trigx, trigy, dy, 0,1);
    return lam.x*f[1] + lam.y*f[2] + (1-lam.x-lam.y)*f[0];
  }
  if(order == 2) {
    f[0] = getSubTrigValue(trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(trigx, trigy, dy, 2,0);
    f[3] = getSubTrigValue(trigx, trigy, dy, 0,1);
    f[4] = getSubTrigValue(trigx, trigy, dy, 1,1);
    f[5] = getSubTrigValue(trigx, trigy, dy, 0,2);
    return 1.0*f[0] + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + x*y*(4.0*f[0] - 4.0*f[1] - 4.0*f[3] + 4.0*f[4]) + x*(-3.0*f[0] + 4.0*f[1] - 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) + y*(-3.0*f[0] + 4.0*f[3] - 1.0*f[5]);
  }
  return 0.0;
}

// find nearest three grid points with values in triangle
void getNearestGridPoints(vec3 lam, out int xi[3], out int yi[3], out vec3 weights)
{
    int n = subdivision;
    vec3 lamn = lam*(n-1);
    vec3 floor_lamn = floor(lamn);
    vec3 dif = lamn-floor_lamn;
    int x=int(lamn.x);
    int y=int(lamn.y);
    int z=int(lamn.z);
    int missing = n-1-x-y-z;

    for (int i=0; i<3; i++) {
        xi[i] = x;
        yi[i] = y;
    }
    float sumdif = dif.x+dif.y+dif.z;

    if(missing==1) {
        xi[0] = x+1;
        yi[1] = y+1;
        weights = dif; //sumdif;
    }
    if(missing==2) {
        sumdif = 3-sumdif;
        xi[0] = x+1;
        yi[0] = y+1;
        xi[1] = x+1;
        yi[2] = y+1;
        weights.x = (1-dif.z)/sumdif;
        weights.y = (1-dif.y)/sumdif;
        weights.z = (1-dif.x)/sumdif;
    }
}


void getNearestGridPoint(vec3 lam, out int xi, out int yi) {
    int n = subdivision;
    vec3 lamn = lam*(n-1);
    vec3 floor_lamn = ceil(lamn);
    vec3 dif = lamn-floor_lamn;
    xi=int(lamn.x);
    yi=int(lamn.y);
    int zi=int(lamn.z);

    int missing = n-1-xi-yi-zi;
    if(missing==1) {
        if(dif.x >= max(dif.y, dif.z)) {
            xi += 1;
            return;
        }
        if(dif.y >= max(dif.x, dif.z)) {
            yi += 1;
            return;
        }
    }
    if(missing==2) {
        if(dif.z <= min(dif.y, dif.x)) {
            xi += 1;
            yi += 1;
            return;
        }
        if(dif.y <= min(dif.x, dif.z)) {
            xi += 1;
            return;
        }
        if(dif.x <= min(dif.y, dif.z)) {
            yi += 1;
            return;
        }
    }
    /*
    vec3 dist = abs(lam-floor(lam+0.5));
    int n = subdivision;
    xi = Round(lam.x);
    yi = 0;
    return dist.x;
    if(dist.x<= min(dist.y, dist.z)) {
        xi = Round(lam.x);
        if(dist.y <= dist.z)
          yi = Round(lam.y);
        else
          yi = n-1 - xi - Round(lam.z);
        return 0.0;
    }
    xi = 0;
    yi = 2;
    if(dist.y<= min(dist.x, dist.z)) {
        yi = Round(lam.y);
        if(dist.x <= dist.z)
          xi = Round(lam.x);
        else
          xi = n-1 - yi - Round(lam.z);
        return 0.0;
    }
    if(dist.z<= min(dist.x, dist.y)) {
        int zi = Round(lam.z);
        if(dist.x <= dist.y) {
          xi = Round(lam.x);
          yi = 1 - zi - xi;
        }
        else {
          yi = Round(lam.y);
          xi = 1 - zi - yi;
        }
        return 0.0;
    }
    */
}
float evalTrig(vec3 lam, int n) {
    // int yi = Round(lam.y*(n-1));
    // int xi = Round(lam.x*(n-1-yi));
    // return loadValue(getOffset(n, xi, yi));

    vec3 lamn = lam*(n-1);
    vec3 floor_lamn = ceil(lamn);
    vec3 dif = lamn-floor_lamn;

    int xi=int(lamn.x+1);
    int yi=int(lamn.y+1);
    int zi=int(lamn.z+1);

    float h = 1.0/n;
    vec3 lam1;
    // getNearestGridPoint(lam, xi, yi);
    // return loadValue(getOffset(n, xi, yi));

    int xii[3];
    int yii[3];
    int zii[3];
    vec3 weights;
    getNearestGridPoints(lam, xii, yii, weights);
    float value = 0.0;
    for (int i=0; i<3; i++)
      value += weights[i]*loadValue(getOffset(n, xii[i], yii[i]));
    return value;

    // if((dif.x <= dif.y) && (dif.x <= dif.z) ) xi-=1; TODO: eventuel zwei erhoehen!!!
    // else if((dif.y < dif.x) && (dif.y < dif.z) ) yi-=1;
    // else zi-=1;
    // return float(-1+xi+yi+zi);
    // return float(int(lamn.x)+abs(xi));
    // return float(xi);
    // return float(xi==yi);
}

float evalTrigLinear(vec3 lam, int n) {
    float x = lam.y*(n-1);
    float y = lam.z*(n-1);

    float val = 0.0;
    float x0 = x-0.5;
    int yi = Round(y);
    for (int i=0; i<2; i++) {
        int xi = Round(x0+i);
        val += (1-abs(xi-x))*loadValue(getOffset(n, xi, yi));
    }
    return val;
}
float evalTrigLinear1(vec3 lam, int n) {
    float y = lam.z*(n-1);

    float val = 0.0;
    float y0 = y-0.5;
    for (int j=0; j<2; j++) {
      int yi = min(n-1,max(0,Round(y0+j)));
      float valx = 0.0;
      float x = lam.y*(n-1-yi);
      float x0 = x-0.5;
      for (int i=0; i<2; i++) {
          int xi = Round(x0+i);
          valx += (1-abs(x-xi))*loadValue(getOffset(n, xi, yi));
      }
      val += (1-abs(y-yi))*valx;
    }
    return val;
}

float evalTrigLinear2(vec3 lam, int n) {
    float x = lam.y*(n-1);
    float y = lam.z*(n-1);

    float val = 0.0;
    float y0 = y-0.5;
    for (int j=0; j<2; j++) {
      int yi = Round(y0+j);
      float valx = 0.0;
      int xi = Round(x);
      valx += loadValue(getOffset(n, xi, yi));
      val += (1-abs(y-yi))*valx;
    }
    return val;
}


{include utils.inc}
void main()
{
  if(!do_clipping || dot(vec4(inData.pos,1.0),clipping_plane)<0)
  {
      float x = inData.lam.x;
      float y = inData.lam.y;
      float z = inData.lam.z;
      //  { ET_POINT = 0, ET_SEGM = 1,
      //    ET_TRIG = 10, ET_QUAD = 11, 
      //    ET_TET = 20, ET_PYRAMID = 21, ET_PRISM = 22, ET_HEX = 24 };
      float value;
      // if(element_type == 10) value = EvalTRIG(inData.element, x,y,z);
      // if(element_type == 20) value = EvalTET(inData.element, x,y,z);
      // if(element_type == 21) value = EvalPYRAMID(inData.element, x,y,z);
      // if(element_type == 22) value = EvalPRISM(inData.element, x,y,z);
      // if(element_type == 24) value = EvalHEX(inData.element, x,y,z);

      // value = evalTrig(inData.lam, subdivision);
      value = Interpolate();
      // value = evalTrigLinear1(inData.lam, subdivision);

      value = (value-colormap_min)/(colormap_max-colormap_min);
      value = clamp(value, 0.0, 1.0);
      value = (1.0 - value);
      if(!colormap_linear)
        value = floor(8*value)/7.0;
      FragColor.r = MapColor(value).r;
      FragColor.g = MapColor(value).g;
      FragColor.b = MapColor(value).b;
      FragColor.a = 1.0;
  }
  else
    discard;
}
