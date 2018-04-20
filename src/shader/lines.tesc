#version 410 core

layout(vertices = 2) out;

{include utils.inc}

uniform float TessLevel;
uniform Mesh mesh;

in VertexData
{
  flat int element;
} inData[];

out VertexData
{
  flat int element;
} outData[];

void main()
{
    outData[gl_InvocationID].element = inData[0].element;
    float level=1;

    if(mesh.dim== 1) {
      Element1d el = getElement1d(mesh, inData[0].element);
      level = el.curved_index>=0 ? TessLevel : 1;
    }
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = level;
        gl_TessLevelOuter[0] = level;
        gl_TessLevelOuter[1] = level;
        gl_TessLevelOuter[2] = level;
    }
}
