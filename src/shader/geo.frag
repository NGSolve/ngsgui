#version 150

{include utils.inc}

uniform mat4 MV;
uniform vec4 clipping_plane;
uniform bool clip_whole_elements;
uniform bool do_clipping;
uniform Light light;

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

  if(!clip_whole_elements && do_clipping && dot(vec4(inData.pos,1.0),clipping_plane)<0)
    discard;

  if (FragColor.a == 0.0)
    discard;

  FragColor.rgb = CalcLight(light, FragColor.rgb, MV, inData.pos, inData.normal);
}
