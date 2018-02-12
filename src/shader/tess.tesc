#version 410 core

layout(vertices = 3) out;

{include utils.inc}

uniform float TessLevel;
uniform Mesh mesh;

in VertexData
{
  // flat int element;
  flat Element2d el;
} inData[];

out VertexData
{
  // flat int element;
  flat Element2d el;
} outData[];

void main()
{
    outData[gl_InvocationID].el = inData[0].el;
    // outData[gl_InvocationID].element = inData[0].element;
    // Element2d el = getElement2d(mesh, inData[0].element); 
    float level = inData[0].el.curved_index>=0 ? TessLevel : 1;

    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = level;
        gl_TessLevelOuter[0] = level;
        gl_TessLevelOuter[1] = level;
        gl_TessLevelOuter[2] = level;
    }
}
