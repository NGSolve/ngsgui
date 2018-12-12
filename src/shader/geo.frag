#version 150

{include utils.inc}

uniform bool clip_whole_elements;

in VertexData
{
  vec3 pos;
  vec3 normal;
  vec4 color;
  vec3 edgedist;
} inData;

out vec4 FragColor;

vec3 TransformVec( vec3 x) {
    return normalize(inverse(mat3(MV))*x);
}

void main()
{
  FragColor = inData.color;

  if(!clip_whole_elements && !CalcClipping(inData.pos))
    discard;

  if (FragColor.a == 0.0)
    discard;

  FragColor.rgb = CalcLight(FragColor.rgb, MV, inData.pos, inData.normal);
}
