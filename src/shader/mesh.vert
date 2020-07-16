#version 150

{include utils.inc}
#line 4

uniform float shrink_elements;
uniform bool clip_whole_elements;
uniform samplerBuffer coefficients;
uniform int subdivision;
uniform int component;

uniform bool filter_elements;
uniform samplerBuffer tex_filter;
uniform float filter_min;
uniform float filter_max;

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

  if(filter_elements)
  {
    int nr = getElementNr(eid);
    float val = texelFetch(tex_filter, nr).r;
    if(val<filter_min || val>filter_max)
    {
      gl_Position = vec4(0,0,0,0);
      return;
    }
  }

  ELEMENT_TYPE element = getElement(eid);
  outData.element = eid;
  outData.normal = vec3(0,0,0);
  outData.lam = vec3(0,0,0);
  outData.pos = element.pos[vid];

  if(clip_whole_elements) {
      bool is_visible = true;
      for (int i=0; i<ELEMENT_N_VERTICES; i++)
          is_visible = is_visible && CalcClipping(element.pos[i]);
      if(!is_visible) {
          // discard
          gl_Position = vec4(0,0,0,0);
          return;
      } 
  }

#if ELEMENT_DIM==3
  vec3 center = vec3(0.0,0.0,0.0);
  for (int i=0; i<ELEMENT_N_VERTICES;i++)
      center += element.pos[i];

  center *= 1.0/ELEMENT_N_VERTICES;
#endif

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
#elif defined(ET_HEX)
    // draw faces of 3d elements using multiple instances (gl_InstanceID)
    // 6 quads -> 12 triangles in total
    int fid = gl_InstanceID;
    ivec3 verts;
    if (fid<4) {
        verts = ivec3(fid,(fid+1)%4,fid+4);
    }
    else if(fid<8) {
        verts.x = (fid-3)%4;
        verts.y = fid;
        verts.z = verts.x+4;
    }
    // quad 0123
    else if(fid== 8) verts = ivec3(0,1,2);
    else if(fid== 9) verts = ivec3(0,2,3);
    // quad 4567 
    else if(fid==10) verts = ivec3(4,5,6);
    else if(fid==11) verts = ivec3(4,6,7);
    else return;
#elif defined(ET_PRISM)
    // draw faces of 3d elements using multiple instances (gl_InstanceID)
    // 3 quads + 2 trigs -> 8 triangles in total
    int fid = gl_InstanceID;
    ivec3 verts;
    if (fid==0) verts = ivec3(0,1,2);
    else if (fid==1) verts = ivec3(0,1,3);
    else if (fid==2) verts = ivec3(1,3,4);
    else if (fid==3) verts = ivec3(1,2,4);
    else if (fid==4) verts = ivec3(2,4,5);
    else if (fid==5) verts = ivec3(2,0,3);
    else if (fid==6) verts = ivec3(2,3,5);
    else if (fid==7) verts = ivec3(3,4,5);
    else return;
#elif defined(ET_PYRAMID)
    // draw faces of 3d elements using multiple instances (gl_InstanceID)
    // 1 quads + 4 trigs -> 6 triangles in total
    int fid = gl_InstanceID;
    ivec3 verts;
    if (fid==0) verts = ivec3(0,1,2);
    else if (fid==1) verts = ivec3(2,3,0);
    else if (fid==2) verts = ivec3(0,1,4);
    else if (fid==3) verts = ivec3(1,2,4);
    else if (fid==4) verts = ivec3(2,3,4);
    else if (fid==5) verts = ivec3(3,0,4);
    else return;
#endif

#if ELEMENT_DIM==3
    outData.normal = normalize(cross(element.pos[verts[1]]-element.pos[verts[0]], element.pos[verts[2]]-element.pos[verts[0]]));
    outData.pos = element.pos[verts[vid]];
    if(dot(outData.normal, outData.pos-center)<0)
        outData.normal = -outData.normal;
    outData.pos = mix(center,outData.pos,  shrink_elements);
#endif
///////////////////////////////////////////////////////////////////////////////
  gl_Position = P * MV * vec4(outData.pos, 1);
}
