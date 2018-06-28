#version 150

uniform mat4 P;
uniform mat4 MV;
uniform sampler1D colors;

in vec3 pos;
in ivec3 domain;

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
  outData.color = texelFetch(colors, domain.x-1, 0);
  gl_Position = P*MV * vec4(pos, 1);
}
