#version 410 core

layout(vertices = 3) out;

uniform float TessLevelInner;
uniform float TessLevelOuter;

in VertexData
{
  vec3 pos;
  flat int index;
} inData[];

out VertexData
{
  vec3 pos;
  flat int index;
} outData[];

void main()
{
    outData[gl_InvocationID].pos = inData[gl_InvocationID].pos;
    outData[gl_InvocationID].index = inData[gl_InvocationID].index;
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = TessLevelInner;
        gl_TessLevelOuter[0] = TessLevelOuter;
        gl_TessLevelOuter[1] = TessLevelOuter;
        gl_TessLevelOuter[2] = TessLevelOuter;
    }
}
