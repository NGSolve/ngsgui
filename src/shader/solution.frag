#version 150

{include utils.inc}
{include interpolation.inc}
#line 5

uniform sampler1D colors;
uniform samplerBuffer coefficients;
uniform int element_type;
uniform vec4 clipping_plane;
uniform bool do_clipping;
uniform int subdivision;
uniform int order;
uniform mat4 MV;
uniform int component;
uniform Colormap colormap;
uniform Light light;

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

void main()
{
  FragColor = vec4(0,1,0,1);
  if(!do_clipping || dot(vec4(inData.pos,1.0),clipping_plane)>0)
  {
      float value;
      vec3 lam = inData.lam;
#if defined(ET_QUAD)
       lam = inData.lam.yxz;
#endif
       value = EvaluateElement(inData.element, coefficients, ORDER, subdivision, lam, component);

      if(is_complex) {
          float value_imag;
          value = EvaluateElement(inData.element, coefficients, ORDER, subdivision, lam, component);
          value_imag = EvaluateElement(inData.element, coefficients_imag, ORDER, subdivision, lam, component);
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

      FragColor.rgb = MapColor(colormap, value);
      FragColor.a = 1.0;
#ifndef NOLIGHT
      FragColor.rgb = CalcLight(light, FragColor.rgb, MV, inData.pos, inData.normal);
#endif
  }
  else
    discard;
}
