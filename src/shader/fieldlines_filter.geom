#version 150

{include utilsnew.inc}
{include interpolation.inc}
#line 5

uniform samplerBuffer coefficients;
uniform bool clipping_plane_deformation;
uniform float colormap_min, colormap_max;
uniform Mesh mesh;
uniform float grid_size;
uniform int subdivision;
uniform int order;
uniform int component;
uniform int n_steps;

layout(points) in;
layout(points, max_vertices=40) out;

in VertexData
{
  flat int element;
} inData[];

out vec3 pos;
out vec3 val;
out vec3 val2;

void getM( ELEMENT_TYPE tet, out mat3 m, out mat3 minv ) {
    for (int i=0; i<ELEMENT_N_VERTICES-1; i++)
      m[i] = tet.pos[i]-tet.pos[3];
    minv = inverse(m);
}

void main() {
    int element = inData[0].element;
    ELEMENT_TYPE tet = getElement(mesh, element);

    vec4 lam = vec4(0.25,0.25,0.25, 0.25);
    vec4 lam_last;

    mat3 m, minv;
    getM(tet, m, minv);
    pos = m * lam.xyz + tet.pos[3];
    float h = length(pos-tet.pos[0])*0.2;

    for (int i=0; i<n_steps;i++) {
        if(lam.x<=0 || lam.y<=0 || lam.z<=0 || lam.w<=0) {
            // jumping out of one element -> find adjacent element to continue
            // first cut the current trajectory with the element face
            int face = 3;
            float minlam = lam.w;
            for (int i=0; i<3; i++) {
                if(lam[i] < minlam )
                {
                    minlam = lam[i];
                    face = i;
                }
            }
            // find point on face with lam[face] = 0.0
            float x = lam_last[face]/(lam_last[face]-lam[face]);
            lam_last = mix(lam, lam_last, x);
            
            int new_element = texelFetch(mesh.elements, ELEMENT_SIZE*element+6+face).r;
            if(new_element==-1) return;
            ivec4 verts0;
            ivec4 verts1;
            lam = vec4(0.0, 0.0, 0.0, 0.0);
            for (int i=0; i<ELEMENT_N_VERTICES; i++) {
                verts0[i] = texelFetch(mesh.elements, ELEMENT_SIZE*element+i+2).r;
                verts1[i] = texelFetch(mesh.elements, ELEMENT_SIZE*new_element+i+2).r;
            }
            for (int i=0; i<ELEMENT_N_VERTICES-1; i++) {
                for (int j=0; j<ELEMENT_N_VERTICES; j++)
                    if(verts1[i] == verts0[j])
                        lam[i] = lam_last[j];

            }

            element = new_element;
            tet = getElement(mesh, element);

            getM(tet, m, minv);
            lam.xyz = minv*(pos-tet.pos[3]);
            lam.w = 1.0-lam.x-lam.y-lam.z;
        }
        vec3 v = h*normalize(EvaluateElementVec(element, coefficients, ORDER, subdivision, lam.xyz, component));
        if(i==0) val = v;
        val2 = val;
        val = v;
        EmitVertex();
        EndPrimitive();
        pos += v;
        lam_last = lam;
        lam.xyz = minv*(pos-tet.pos[3]);
        lam.w = 1.0-lam.x-lam.y-lam.z;
        pos = m*lam.xyz + tet.pos[3];
    }
}
