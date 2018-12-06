#version 150

uniform float grid_size;

out VertexData
{
  vec3 pos;
  vec3 pos2;
  vec3 val;
} outData;

in vec3 pos;
in vec3 pos2;
in vec3 val;

void main()
{
  outData.pos = pos;
  outData.pos2 = pos2;
  outData.val = val;
}
