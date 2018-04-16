#version 150

uniform mat4 MV;
uniform mat4 P;
uniform samplerBuffer vertices;
uniform isamplerBuffer triangles;

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec4 color;
  float edgedist;
} outData;

void main()
{
  int trignr = gl_VertexID/3;
  int vert_in_trig = gl_VertexID - 3*trignr;
  int other1 = (vert_in_trig + 1)%3;
  int other2 = (vert_in_trig + 2)%3;
  ivec3 verts = texelFetch(triangles, trignr).rgb;
  outData.pos = texelFetch(vertices, verts[vert_in_trig]).rgb;
  vec3 posother1 = texelFetch(vertices, verts[other1]).rgb;
  vec3 posother2 = texelFetch(vertices, verts[other2]).rgb;
  gl_Position = P * MV * vec4(outData.pos,1);
  outData.normal = normalize(cross(posother1-outData.pos, posother2-outData.pos));
  outData.edgedist = 1.;
  outData.color = vec4(0,0,1,1);
}
