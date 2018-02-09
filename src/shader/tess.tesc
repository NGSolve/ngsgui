#version 410 core

layout(vertices = 3) out;

uniform float TessLevel;

in VertexData
{
  vec3 pos;
  vec3 normal;
  vec3 other_pos;
  flat int index;
  flat int curved_index;
} inData[];

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec3 other_pos;
  flat int index;
  flat int curved_index;
} outData[];

void main()
{
    outData[gl_InvocationID].pos = inData[gl_InvocationID].pos;
    outData[gl_InvocationID].normal = inData[gl_InvocationID].normal;
    outData[gl_InvocationID].other_pos = inData[gl_InvocationID].other_pos;
    outData[gl_InvocationID].index = inData[gl_InvocationID].index;
    outData[gl_InvocationID].curved_index = inData[gl_InvocationID].curved_index;
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = TessLevel;
        gl_TessLevelOuter[0] = TessLevel;
        gl_TessLevelOuter[1] = TessLevel;
        gl_TessLevelOuter[2] = TessLevel;
    }
}
