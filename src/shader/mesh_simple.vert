#version 150

{DEFINES}
{include utils.inc}
#line 5

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;

ELEMENT_TYPE getElement(Mesh mesh, int elnr ) {
    ELEMENT_TYPE el;
    int offset = ELEMENT_SIZE*elnr;
    el.index = texelFetch(mesh.elements, offset +1).r;
    for (int i=0; i<ELEMENT_N_VERTICES; i++) {
        int v = texelFetch(mesh.elements, offset+i+2).r;
        el.pos[i] = texelFetch(mesh.vertices, v).xyz;
    }
#ifdef CURVED
    el.curved_vertices = texelFetch(mesh.elements, offset + ELEMENT_SIZE-1).r;
#endif
#if defined(ET_TRIG) || defined(ET_QUAD)
    el.normal = cross(el.pos[1]-el.pos[0], el.pos[2]-el.pos[0]);
#endif
    return el;
}

out VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} outData;

void main()
{
  int eid = gl_VertexID/ELEMENT_N_VERTICES;
  int vid = gl_VertexID - ELEMENT_N_VERTICES*eid;
  ELEMENT_TYPE element = getElement(mesh, eid);
  outData.element = eid;
  outData.normal = vec3(0,0,0);
  outData.lam = vec3(0,0,0);
  outData.pos = element.pos[vid];

#ifndef CURVED
  #if defined(ET_TRIG) || defined(ET_QUAD)
    outData.normal = element.normal;
  #endif
  outData.lam[vid] = 1.0;
#else
  outData.normal = texelFetch(mesh.vertices, element.curved_vertices+vid).xyz;
  #if defined(ET_QUAD)
  if(vid>2) outData.lam.y = 1.0;
  if(vid==1 || vid==2) outData.lam.x = 1.0;
  #endif

#endif
  gl_Position = P * MV * vec4(outData.pos, 1);
}
