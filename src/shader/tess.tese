#version 410 core

layout(triangles, equal_spacing, cw) in;

{include utils.inc}

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;

in VertexData
{
  flat int el_id;
} inData[];

out VertexData
{
  flat int el_id;
  vec3 lam;
} outData;

void main()
{
    outData.el_id = inData[0].el_id;
    outData.lam = gl_TessCoord.xyz;
}
