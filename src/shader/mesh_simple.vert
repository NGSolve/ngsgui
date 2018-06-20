#version 150

{include utils.inc}
#line 4
uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;

#define ELEMENT_TYPE {ELEMENT_TYPE}
#define {ELEMENT_TYPE_NAME}
#define ELEMENT_SIZE {ELEMENT_SIZE}
#define ELEMENT_N_VERTICES {ELEMENT_N_VERTICES}

ELEMENT_TYPE getElement(Mesh mesh, int elnr ) {
    ELEMENT_TYPE el;
    int offset = ELEMENT_SIZE*elnr;
    el.index = texelFetch(mesh.elements, offset +1).r;
    for (int i=0; i<ELEMENT_N_VERTICES; i++) {
        int v = texelFetch(mesh.elements, offset+i+2).r;
        el.pos[i] = texelFetch(mesh.vertices, v).xyz;
    }
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
  flat int element;
  flat int index;
} outData;

void main()
{
  int eid = gl_VertexID/ELEMENT_N_VERTICES;
  int vid = gl_VertexID - ELEMENT_N_VERTICES*eid;
  ELEMENT_TYPE element = getElement(mesh, eid);
  outData.color = vec4(texelFetch(colors, element.index, 0));
  outData.element = eid;
  outData.pos = element.pos[vid];
#if defined(ET_TRIG) || defined(ET_QUAD)
  outData.normal = element.normal;
#endif
  gl_Position = P * MV * vec4(outData.pos, 1);
}
