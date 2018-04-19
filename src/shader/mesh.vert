#version 150

{include utils.inc}

uniform mat4 MV;
uniform mat4 P;
uniform Mesh mesh;
uniform sampler1D colors;

out VertexData
{
  flat int element;
} outData;

void main()
{
  outData.element = gl_VertexID;
}
