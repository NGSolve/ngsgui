#version 150 // 400 for subdivision with multiple invocations

{include utilsnew.inc}
#line 4

uniform samplerBuffer coefficients;
uniform Mesh mesh;
uniform int subtet;
uniform ClippingPlanes clipping_planes;

layout(points) in;
layout(triangle_strip, max_vertices=18) out;

in VertexData
{
  flat int element;
} inData[];

out VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} outData;

uniform mat4 MV;
uniform mat4 P;

void main() {
    outData.element = inData[0].element;
    ELEMENT_TYPE el = getElement(mesh, inData[0].element);

    vec3 lam_ori[4];
    TET tet;
#ifdef ET_TET
    tet = el;
    lam_ori = vec3[4]( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1), vec3(0,0,0));
#else
    ivec4 vi = getTetFromElement( el, subtet, tet, lam_ori);
#endif

    for (int ci=0; ci<N_CLIPPING_PLANES; ci++) {
      vec3 lam[4] = lam_ori;
      float values[4];
      for (int i=0; i<4; i++)
        values[i] = dot(clipping_planes.p[ci], vec4(tet.pos[i],1.0));

      vec3 pos[4];
      int n_cutting_points = CutElement3d( tet, values, pos, lam );

      if(n_cutting_points >= 3) {
          for (int i=0; i<n_cutting_points; i++) {
              outData.pos = pos[i];
              outData.lam = lam[i];
              outData.normal = clipping_planes.p[ci].xyz;
              gl_Position = P * MV *vec4(outData.pos,1);
              EmitVertex();
          }
          EndPrimitive();
      }
    }
}
