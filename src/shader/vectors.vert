#version 150

uniform float grid_size;

out VertexData
{
  vec3 pos;
  vec3 val;
} outData;


in vec3 pos;
in vec3 val;

void main()
{
  outData.pos = pos;
  outData.val = val;
}
