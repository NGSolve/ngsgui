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
    return texelFetch(coefficients, inData.element*values_per_element+offset).r;
}

float InterpolateTrig() {
  vec3 lam = inData.lam;
  int trigx=0;
  int trigy=0;
  int dy=1;
  getSubTrigStart(lam, trigx, trigy, dy);
  float x = lam.x;
  float y = lam.y;
  if(order==1) {
    float f[3];
    f[0] = getSubTrigValue(trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(trigx, trigy, dy, 0,1);
    return lam.x*f[1] + lam.y*f[2] + (1-lam.x-lam.y)*f[0];
  }
  if(order == 2) {
    float f[6];
    f[0] = getSubTrigValue(trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(trigx, trigy, dy, 2,0);
    f[3] = getSubTrigValue(trigx, trigy, dy, 0,1);
    f[4] = getSubTrigValue(trigx, trigy, dy, 1,1);
    f[5] = getSubTrigValue(trigx, trigy, dy, 0,2);
    return 1.0*f[0] + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + x*y*(4.0*f[0] - 4.0*f[1] - 4.0*f[3] + 4.0*f[4]) + x*(-3.0*f[0] + 4.0*f[1] - 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) + y*(-3.0*f[0] + 4.0*f[3] - 1.0*f[5]);
  }
  if(order == 3) {
    float f[10];
    f[0] = getSubTrigValue(trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(trigx, trigy, dy, 2,0);
    f[3] = getSubTrigValue(trigx, trigy, dy, 3,0);
    f[4] = getSubTrigValue(trigx, trigy, dy, 0,1);
    f[5] = getSubTrigValue(trigx, trigy, dy, 1,1);
    f[6] = getSubTrigValue(trigx, trigy, dy, 2,1);
    f[7] = getSubTrigValue(trigx, trigy, dy, 0,2);
    f[8] = getSubTrigValue(trigx, trigy, dy, 1,2);
    f[9] = getSubTrigValue(trigx, trigy, dy, 0,3);
    return 1.0*f[0] + x*x*x*(-4.5*f[0] + 13.5*f[1] - 13.5*f[2] + 4.5*f[3]) + x*x*y*(-13.5*f[0] + 27.0*f[1] - 13.5*f[2] + 13.5*f[4] - 27.0*f[5] + 13.5*f[6]) + x*x*(9.0*f[0] - 22.5*f[1] + 18.0*f[2] - 4.49999999999999*f[3]) + x*y*y*(-13.5*f[0] + 13.5*f[1] + 27.0*f[4] - 27.0*f[5] - 13.5*f[7] + 13.5*f[8]) + x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8] + 4.9960036108132e-16*f[9]) + x*(-5.5*f[0] + 9.0*f[1] - 4.5*f[2] + 0.999999999999998*f[3]) + y*y*y*(-4.5*f[0] + 13.5*f[4] - 13.5*f[7] + 4.5*f[9]) + y*y*(9.0*f[0] - 22.5*f[4] + 18.0*f[7] - 4.49999999999999*f[9]) + y*(-5.5*f[0] + 9.0*f[4] - 4.5*f[7] + 0.999999999999998*f[9]);
  }
  return 0.0;
}

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
      if(element_type == 10) value = InterpolateTrig();
      // if(element_type == 20) value = EvalTET(inData.element, x,y,z);
      // if(element_type == 21) value = EvalPYRAMID(inData.element, x,y,z);
      // if(element_type == 22) value = EvalPRISM(inData.element, x,y,z);
      // if(element_type == 24) value = EvalHEX(inData.element, x,y,z);

      // value = evalTrig(inData.lam, subdivision);
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
