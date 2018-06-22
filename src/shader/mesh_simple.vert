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
  flat int element;
  flat int index;
} outData;

// ivec2 I2toI1(int n) {
//   int N = {N_INSTANCES};
//   if (n<N) return ivec2(n, 0);
//   if (n<2*N-1) return ivec2(n-N, 1);
//   if (n<3*N-3) return ivec2(n-2*N-1, 2);
//   if (n<4*N-6) return ivec2(n-3*N-3, 3);
//   if (n<5*N-10) return ivec2(n-4*N-6, 4);
//   return ivec2(0,5);
// }

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

#ifndef CURVED
  outData.pos = element.pos[vid];
  #if defined(ET_TRIG) || defined(ET_QUAD)
    outData.normal = element.normal;
  #endif
#else
  outData.index = element.curved_vertices;
  // Interpolate position for curved element
  // Drawing curved lines/triangles is not possible in OpenGL, so we subdivide
  // it into linear segments using multiple 'instances' of the same element.
  #if defined(ET_SEGM)
    float x = (gl_InstanceID+vid)/(1.0*{N_INSTANCES});
    vec3 a = element.pos[0];
    vec3 b = texelFetch(mesh.vertices, element.curved_vertices+2).xyz;
    vec3 c = element.pos[1];
    outData.pos = a + x*(-c-3*a+4*b) + x*x*2*(a-2*b+c);
  #elif defined(ET_TRIG)
    int offset = element.curved_vertices;
    outData.normal = texelFetch(mesh.vertices, offset+vid).xyz;
    outData.pos = element.pos[vid];
  #elif defined(ET_QUAD)
    int offset = element.curved_vertices;
    outData.normal = texelFetch(mesh.vertices, offset+vid).xyz;
    outData.pos = element.pos[vid];
    outData.pos = vec3(vid, vid*vid, -5);
  #else
      unknown element type
  #endif
#endif
  gl_Position = P * MV * vec4(outData.pos, 1);
}
// #endif


