#version 150

uniform mat4 P;
uniform mat4 MV;

in vec3 pos;

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec4 color;
  vec3 edgedist;
} outData;

void main()
{
  outData.pos = pos;
  outData.color = vec4(0,0,0,1);
  gl_Position = P*MV * vec4(pos, 1);
}
