#version 410 core

layout(triangles, equal_spacing, cw) in;

{include utils.inc}

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;

in VertexData
{
  flat int element;
} inData[];

out VertexData
{
  flat int element;
  vec3 lam;
} outData;

void main()
{
    outData.element = inData[0].element;
    outData.lam = gl_TessCoord.xyz;
}
