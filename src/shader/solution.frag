#version 150
uniform samplerBuffer coefficients;
uniform float colormap_min, colormap_max;
uniform bool colormap_linear;
uniform int element_type;
uniform vec4 clipping_plane;
uniform bool do_clipping;
uniform int subdivision;
uniform int order;
uniform mat4 MV;
uniform int component;

// for complex-valued functions
uniform bool is_complex;
uniform samplerBuffer coefficients_imag;
uniform int complex_vis_function; // 0=real, 1=imag, 2=abs, 3=arg
uniform vec2 complex_factor; // factor to multiply with values before visualizing

in VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} inData;

out vec4 FragColor;

{include utils.inc}
{include interpolation.inc}

void main()
{
  FragColor = vec4(0,1,0,1);
  if(!do_clipping || dot(vec4(inData.pos,1.0),clipping_plane)>0)
  {
      float x = inData.lam.x;
      float y = inData.lam.y;
      float z = inData.lam.z;
      //  { ET_POINT = 0, ET_SEGM = 1,
      //    ET_TRIG = 10, ET_QUAD = 11, 
      //    ET_TET = 20, ET_PYRAMID = 21, ET_PRISM = 22, ET_HEX = 24 };
      float value;
      vec3 lam = inData.lam.yzx;
      lam.z = 1.0 - inData.lam.x - inData.lam.y - inData.lam.z;
      if(element_type == 10) value = InterpolateTrig(inData.element, coefficients, order, subdivision, inData.lam, component);
      if(element_type == 20) value = InterpolateTet(inData.element, coefficients, order, subdivision, lam, component);
      // if(element_type == 21) value = EvalPYRAMID(inData.element, x,y,z);
      // if(element_type == 22) value = EvalPRISM(inData.element, x,y,z);
      // if(element_type == 24) value = EvalHEX(inData.element, x,y,z);

      // value = evalTrig(inData.lam, subdivision);
      // value = evalTrigLinear1(inData.lam, subdivision);

      if(is_complex) {
          float value_imag;
          if(element_type == 10) value_imag = InterpolateTrig(inData.element, coefficients_imag, order, subdivision, inData.lam);
          if(element_type == 20) value_imag = InterpolateTet(inData.element, coefficients_imag, order, subdivision, lam);
          float r = value*complex_factor.x - value_imag*complex_factor.y;
          value_imag = value*complex_factor.y + value_imag*complex_factor.x;
          value = r;
          switch(complex_vis_function){
            case 0:
              break;
            case 1:
              value = value_imag;
              break;
            case 2:
              value = length(vec2(value, value_imag));
              break;
            case 3:
              value = atan(value, value_imag);
              break;
          }
      }

      value = (value-colormap_min)/(colormap_max-colormap_min);
      value = clamp(value, 0.0, 1.0);
      value = (1.0 - value);
      if(!colormap_linear)
        value = floor(8*value)/7.0;
      FragColor.r = MapColor(value).r;
      FragColor.g = MapColor(value).g;
      FragColor.b = MapColor(value).b;
      FragColor.a = 1.0;
      // vec3 lightVector = TransformVec(vec3(1,3,3));
      // FragColor.rgb *= 0.3+0.7*clamp(dot(normalize(inData.normal), lightVector), 0, 1.0);
      FragColor.rgb = light(FragColor.rgb, MV, inData.pos, inData.normal);
      // FragColor.rgb = 0.5+0.5*normalize(inData.normal);
      // float l = length(inData.normal)*0.5;
      // FragColor.rgb = vec3(l,l,l);
  }
  else
    discard;
}
