#version 150
uniform mat4 MV;
uniform mat4 P;

in vec3 pos;
in int index;

out VertexData
{
  vec3 pos;
  flat int index;
} outData;

void main()
{
    gl_Position = P * MV * vec4(pos, 1.0);
    outData.pos = pos;
    outData.index = index;
}
