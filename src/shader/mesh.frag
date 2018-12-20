#version 150

{include utils.inc}
#line 5

uniform bool wireframe;
uniform sampler1D colors;

in VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} inData;

out vec4 FragColor;

vec3 TransformVec( vec3 x) {
    return normalize(inverse(mat3(MV))*x);
}

void main()
{
  if(wireframe) {
      vec3 l = inData.lam;
#ifndef ET_QUAD
      float d = min(min(l.x, l.y), l.z);
#else
      float d =  min(min(l.x,l.y), min(1-l.x,1.0-l.y));
#endif
      if(d>1e-5) discard;
  }
  int index = getElementIndex(inData.element);
  if(index==-1)
    FragColor = vec4(0,0,0,1);
  else
    FragColor = vec4(texelFetch(colors, inData.element, 0));

  if(!CalcClipping(inData.pos))
    discard;

  if (FragColor.a == 0.0)
    discard;


  FragColor.rgb = CalcLight(FragColor.rgb, MV, inData.pos, inData.normal);
}
