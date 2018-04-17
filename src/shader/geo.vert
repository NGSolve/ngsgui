#version 150

uniform mat4 MV;
uniform mat4 P;
uniform samplerBuffer vertices;
uniform isamplerBuffer triangles;
uniform samplerBuffer normals;
uniform sampler1D colors;

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec4 color;
  vec3 edgedist;
} outData;

void main()
{
  int trignr = gl_VertexID/3;
  int vert_in_trig = gl_VertexID - 3*trignr;
  ivec4 trig = texelFetch(triangles, trignr);
  ivec3 verts = trig.rgb;
  int surfnr = trig.a;
  outData.pos = texelFetch(vertices, verts[vert_in_trig]).xyz;
  outData.normal = texelFetch(normals, verts[vert_in_trig]).xyz;
  gl_Position = P * MV * vec4(outData.pos,1);
  outData.edgedist = vec3(1,1,1);
  outData.color = vec4(texelFetch(colors,surfnr,0));
}
