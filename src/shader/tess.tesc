#version 410 core

layout(vertices = 3) out;

{include utils.inc}

uniform float TessLevel;
uniform Mesh mesh;

in VertexData
{
  flat int el_id;
} inData[];

out VertexData
{
  flat int el_id;
} outData[];

void main()
{
    outData[gl_InvocationID].el_id = inData[0].el_id;
    float level;

    if(mesh.dim== 2) {
      Element2d el = getElement2d(mesh, inData[0].el_id);
      level = el.curved_index>=0 ? TessLevel : 1;
    }

    if(mesh.dim== 3) {
      Element3d el = getElement3d(mesh, inData[0].el_id);
      level = el.curved_index>=0 ? TessLevel : 1;
    }

    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = level;
        gl_TessLevelOuter[0] = level;
        gl_TessLevelOuter[1] = level;
        gl_TessLevelOuter[2] = level;
    }
}
