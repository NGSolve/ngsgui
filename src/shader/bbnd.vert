#version 400

{include utils.inc}

uniform mat4 MV;
uniform mat4 P;
uniform Mesh mesh;
uniform sampler1D colors;

out VertexData
{
  vec3 pos;
  vec4 color;
} outData;

void main()
{
  int elnr = gl_VertexID/2;
  int vnr = gl_VertexID - 2*elnr;
  Element1d el = getElement1d(mesh, elnr );
  outData.pos = el.pos[vnr];
  outData.color = texelFetch(colors,el.index,0);
  gl_Position = P * MV * vec4(outData.pos,1);
}
