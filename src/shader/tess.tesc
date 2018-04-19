#version 410 core

layout(vertices = 3) out;

{include utils.inc}

uniform float TessLevel;
uniform Mesh mesh;
uniform bool clip_whole_elements;
uniform vec4 clipping_plane;

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
    float level=0;

    if(mesh.dim== 2) {
      Element2d el = getElement2d(mesh, inData[0].element);
      level = el.curved_index>=0 ? TessLevel : 1;
      if(clip_whole_elements) {
          float clip_dist = dot(clipping_plane, vec4(el.pos[0], 1.0));
          clip_dist = max(clip_dist, dot(clipping_plane, vec4(el.pos[1], 1.0)));
          clip_dist = max(clip_dist, dot(clipping_plane, vec4(el.pos[2], 1.0)));
          if(clip_dist<0)
              level = 0;
      }
    }

    if(mesh.dim== 3) {
      Element3d el = getElement3d(mesh, inData[0].element);
      level = el.curved_index>=0 ? TessLevel : 1;
      if(clip_whole_elements) {
          float clip_dist = dot(clipping_plane, vec4(el.pos[0], 1.0));
          clip_dist = max(clip_dist, dot(clipping_plane, vec4(el.pos[1], 1.0)));
          clip_dist = max(clip_dist, dot(clipping_plane, vec4(el.pos[2], 1.0)));
          clip_dist = max(clip_dist, dot(clipping_plane, vec4(el.pos[3], 1.0)));
          if(clip_dist<0)
              level = 0;
      }
    }


    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = level;
        gl_TessLevelOuter[0] = level;
        gl_TessLevelOuter[1] = level;
        gl_TessLevelOuter[2] = level;
    }
}
