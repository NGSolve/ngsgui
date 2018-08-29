#version 150 // 400 for subdivision with multiple invocations

{include utilsnew.inc}

uniform samplerBuffer coefficients;
uniform bool clipping_plane_deformation;
uniform float colormap_min, colormap_max;
uniform Mesh mesh;
uniform int subtet;

layout(points) in;
layout(triangle_strip, max_vertices=6) out;

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
uniform vec4 clipping_plane;

void main() {
    outData.element = inData[0].element;
    ELEMENT_TYPE el = getElement(mesh, inData[0].element);

    vec3 lam[4];
    TET tet;
#ifdef ET_TET
    tet = el;
    lam = vec3[4]( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1), vec3(0,0,0));
#else
    ivec4 vi = getTetFromElement( el, subtet, tet, lam);
#endif

    float values[4];
    for (int i=0; i<4; i++)
      values[i] = dot(clipping_plane, vec4(tet.pos[i],1.0));

    vec3 pos[4];
    int n_cutting_points = CutElement3d( tet, values, pos, lam );

    if(n_cutting_points >= 3) {
        for (int i=0; i<n_cutting_points; i++) {
            outData.pos = pos[i];
            outData.lam = lam[i];
            outData.normal = clipping_plane.xyz;
            gl_Position = P * MV *vec4(outData.pos,1);
            EmitVertex();
        }
        EndPrimitive();
    }
}
