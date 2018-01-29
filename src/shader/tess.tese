#version 410 core

layout(triangles, equal_spacing, cw) in;
// layout(triangles, fractional_even_spacing, cw) in;

uniform mat4 P;
uniform mat4 MV;

in VertexData
{
  vec3 pos;
  flat int index;
} inData[];

out VertexData
{
  vec3 pos;
  float edgedist;
  flat int index;
} outData;



void main()
{
    vec3 p0 = gl_TessCoord.x * inData[0].pos;
    vec3 p1 = gl_TessCoord.y * inData[1].pos;
    vec3 p2 = gl_TessCoord.z * inData[2].pos;
    outData.edgedist = min(min(gl_TessCoord.x, gl_TessCoord.y), gl_TessCoord.z);
    vec3 pos = p0 + p1 + p2;
    gl_Position = P * MV * vec4(pos, 1);
    outData.pos = pos;
    outData.index = inData[0].index;
}
