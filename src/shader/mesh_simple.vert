#version 150

{include utilsnew.inc}
{include interpolation.inc}
#line 4

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform float shrink_elements;
uniform bool clip_whole_elements;
uniform vec4 clipping_plane;
uniform samplerBuffer coefficients;
uniform int subdivision;
uniform int component;

out VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} outData;


void main()
{
#if ELEMENT_DIM<=2
  int nverts = ELEMENT_N_VERTICES;
#else
  int nverts = 3;
#endif
  int eid = gl_VertexID/nverts;
  int vid = gl_VertexID - nverts*eid;
  ELEMENT_TYPE element = getElement(mesh, eid);
  outData.element = eid;
  outData.normal = vec3(0,0,0);
  outData.lam = vec3(0,0,0);
  outData.pos = element.pos[vid];

///////////////////////////////////////////////////////////////////////////////
#if   defined(ET_SEGM)
  outData.lam[1-vid] = 1.0;
///////////////////////////////////////////////////////////////////////////////
#elif defined(ET_TRIG)
  outData.lam[vid] = 1.0;
  #ifdef CURVED
    outData.normal = texelFetch(mesh.vertices, element.curved_vertices+vid).xyz;
  #else
    outData.normal = element.normal;
  #endif
///////////////////////////////////////////////////////////////////////////////
#elif defined(ET_QUAD)
  #ifdef CURVED
    if(vid>2) outData.lam.x = 1.0;
    if(vid==1 || vid==2) outData.lam.y = 1.0;
    outData.normal = texelFetch(mesh.vertices, element.curved_vertices+vid).xyz;
  #else
    outData.lam[vid] = 1.0;
    outData.normal = element.normal;
  #endif
///////////////////////////////////////////////////////////////////////////////
#elif defined(ET_TET)
    // draw faces of 3d elements using multiple instances (gl_InstanceID)
    // the 4 faces are using vertices [1,2,3],[0,2,3],[0,1,3],[0,1,2]
    int fid = gl_InstanceID;
    ivec3 verts = ivec3(0,1,2);
    for (int i=fid; i<3; i++)
        verts[i]++;
    vec3 center = 0.25*(element.pos[0]+element.pos[1]+element.pos[2]+element.pos[3]);
    outData.normal = cross(element.pos[verts[1]]-element.pos[verts[0]], element.pos[verts[2]]-element.pos[verts[0]]);
    outData.pos = element.pos[verts[vid]];
    if(dot(outData.normal, outData.pos-center)<0)
        outData.normal = -outData.normal;
    outData.pos = mix(center,outData.pos,  shrink_elements);
    if(clip_whole_elements) {
      float min_dist = 1.0;
      min_dist = min(min_dist, dot(vec4(element.pos[0],1.0),clipping_plane));
      min_dist = min(min_dist, dot(vec4(element.pos[1],1.0),clipping_plane));
      min_dist = min(min_dist, dot(vec4(element.pos[2],1.0),clipping_plane));
      min_dist = min(min_dist, dot(vec4(element.pos[3],1.0),clipping_plane));
      if(min_dist<0) {
          // discard
          gl_Position = vec4(0,0,0,0);
          return;
      } 
    }
#endif
///////////////////////////////////////////////////////////////////////////////
  gl_Position = P * MV * vec4(outData.pos, 1);
}
