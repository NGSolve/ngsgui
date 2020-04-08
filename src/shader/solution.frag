#version 150

{include utils.inc}
{include interpolation.inc}
#line 5

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
  FragColor.w = clipping_plane_opacity;
  if(true)
#else
  if(CalcClipping(inData.pos))
#endif
  {
      float value;
      vec3 lam = inData.lam;
#if defined(ET_QUAD)
      lam = inData.lam.yxz;
#endif
      lam = clamp(lam,0,1);
      value = Evaluate(FUNCTION, inData.element, lam);

      FragColor.rgb = MapColor(value);
      FragColor.rgb = CalcLight(FragColor.rgb, MV, inData.pos, inData.normal);
  }
  else
    discard;
}
