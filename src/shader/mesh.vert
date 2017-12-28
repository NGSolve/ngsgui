#version 150
uniform mat4 MV;
uniform mat4 P;
in vec3 vPos;

out VertexData
{
  vec3 pos;
} outData;

void main()
{
    gl_Position = P * MV * vec4(vPos, 1.0);
    outData.pos = vPos;
}
