#version 410 core

layout(triangles, equal_spacing, cw) in;

{include utils.inc}

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;

in VertexData
{
  flat Element2d el;
} inData[];

out VertexData
{
  flat Element2d el;
  vec3 lam;
} outData;

void main()
{
    outData.el = inData[0].el;
    outData.lam = gl_TessCoord.xyz;
}
