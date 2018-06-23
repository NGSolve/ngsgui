#version 150

{DEFINES}
{include utils.inc}
#line 5

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;

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
  vec3 pos;
  vec3 normal;
  vec4 color;
  vec3 edge_dist;
  flat int element;
  flat int index;
} outData;

void main()
{
  int eid = gl_VertexID/ELEMENT_N_VERTICES;
  int vid = gl_VertexID - ELEMENT_N_VERTICES*eid;
  ELEMENT_TYPE element = getElement(mesh, eid);
  if(element.index==-1)
    outData.color = vec4(0,0,0,1);
  else
    outData.color = vec4(texelFetch(colors, element.index, 0));
  outData.element = eid;
  outData.normal = vec3(0,0,0);
  outData.edge_dist = vec3(0,0,0);
  outData.pos = element.pos[vid];

#ifndef CURVED
  #if defined(ET_TRIG) || defined(ET_QUAD)
    outData.normal = element.normal;
  #endif
#else
  outData.index = element.curved_vertices;
  outData.normal = texelFetch(mesh.vertices, outData.index+vid).xyz;
#endif
  gl_Position = P * MV * vec4(outData.pos, 1);
}
// #endif


