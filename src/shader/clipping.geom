#version 150 // 400 for subdivision with multiple invocations

{include utilsnew.inc}

uniform samplerBuffer coefficients;
uniform bool clipping_plane_deformation;
uniform float colormap_min, colormap_max;
uniform Mesh mesh;

layout(points) in;
layout(triangle_strip, max_vertices=6) out;

in VertexData
{
  flat int element;
  flat int instance;
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
    ELEMENT_TYPE tet = getElement(mesh, inData[0].element);
    
    float values[4]; // = float[4](-colormap_max, -colormap_max, -colormap_max, -colormap_max);
    for (int i=0; i<4; i++) {
      values[i] = dot(clipping_plane, vec4(tet.pos[i],1.0));
    }
    vec4 plane = -inverse(transpose(mat4(vec4(tet.pos[0],1), vec4(tet.pos[1],1), vec4(tet.pos[2],1), vec4(tet.pos[3],1))))*vec4(values[0], values[1], values[2], values[3]);

    vec3 pos[4];

    vec3 lam[4] = vec3[4]( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1), vec3(0,0,0));
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
