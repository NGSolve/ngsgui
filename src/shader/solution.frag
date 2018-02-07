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
      if(element_type == 10) value = InterpolateTrig(inData.element, coefficients, order, subdivision, inData.lam);
      if(element_type == 20) value = InterpolateTet(inData.element, coefficients, order, subdivision, inData.lam);
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
