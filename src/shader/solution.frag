#version 150

{include utils.inc}
{include interpolation.inc}
#line 5

uniform sampler1D colors;
uniform ClippingPlanes clipping_planes;
uniform mat4 MV;
uniform Colormap colormap;
uniform Light light;
uniform Function function;

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
#ifdef SKIP_FRAGMENT_CLIPPING
  if(true)
#else
  if(CalcClipping(clipping_planes, inData.pos))
#endif
  {
      float value;
      vec3 lam = inData.lam;
#if defined(ET_QUAD)
      lam = inData.lam.yxz;
#endif
      value = Evaluate(function, inData.element, lam);

      FragColor.rgb = MapColor(colormap, value);
      FragColor.a = 1.0;
      FragColor.rgb = CalcLight(light, FragColor.rgb, MV, inData.pos, inData.normal);
  }
  else
    discard;
}
