#version 150 // 400 for subdivision with multiple invocations

{include utils.inc}

uniform samplerBuffer coefficients;
uniform bool clipping_plane_deformation;
uniform Mesh mesh;

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
  flat int element;
} outData;

uniform mat4 MV;
uniform mat4 P;
uniform vec4 clipping_plane;

void main() {
    outData.element = inData[0].element;
    Element3d tet = getElement3d(mesh, inData[0].element);
    
    vec3 pos[4];
    vec3 lam[4];

    int n_cutting_points = CutElement3d( tet, clipping_plane, pos, lam );

    if(n_cutting_points >= 3) {
        for (int i=0; i<n_cutting_points; i++) {
            outData.pos = pos[i];
            outData.lam = lam[i];
            gl_Position = P * MV *vec4(outData.pos,1);
            EmitVertex();
        }
        EndPrimitive();
    }
}
