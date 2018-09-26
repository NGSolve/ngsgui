#version 150

{include utilsnew.inc}
{include interpolation.inc}
#line 5

uniform samplerBuffer coefficients;
uniform bool clipping_plane_deformation;
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
out vec3 pos2;
out vec3 val;

void getM( ELEMENT_TYPE tet, out mat3 m, out mat3 minv ) {
    for (int i=0; i<ELEMENT_N_VERTICES-1; i++)
      m[i] = tet.pos[i]-tet.pos[3];
    minv = inverse(m);
}

void main() {
    int element = inData[0].element;
    if (element/2*2==element) return;
    ELEMENT_TYPE tet = getElement(mesh, element);

    vec4 lam = vec4(0.25,0.25,0.25, 0.25);
    vec4 lam_last;

    mat3 m, minv;
    getM(tet, m, minv);
    pos = m * lam.xyz + tet.pos[3];
    float h = length(pos-tet.pos[0])*0.2;
    pos2 = pos;

    for (int i=0; i<n_steps+1;i++) {
        val = EvaluateElementVec(element, coefficients, ORDER, subdivision, lam.xyz, component);
        if(i>0) {
            EmitVertex();
            EndPrimitive();
        }
        pos = pos2;
        pos2 = pos+h*normalize(val);
        lam_last = lam;
        lam.xyz = minv*(pos2-tet.pos[3]);
        lam.w = 1.0-lam.x-lam.y-lam.z;
        if(lam.x<0.0 || lam.y<0.0 || lam.z<0.0 || lam.w<0.0) {
            // jumping out of one element -> find adjacent element to continue
            // first cut the current trajectory with the element face
            int face = -1;
            vec4 x = lam_last/(lam_last-lam);
            float minx = 999.0;
            for (int i=0; i<4; i++) {
                if( x[i] < minx && lam[i]<0.0 && x[i]>0.0)
                {
                    minx = x[i];
                    face = i;
                }
            }
            if(face==-1) return;
            // find point on face with lam[face] = 0.0
            lam_last = mix(lam_last, lam, minx);
            pos2 = mix(pos, pos2, minx);
            
            int new_element = texelFetch(mesh.elements, ELEMENT_SIZE*element+6+face).r;
            if(new_element==-1) return;
            /*
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
            */

            element = new_element;
            tet = getElement(mesh, element);

            getM(tet, m, minv);
            lam.xyz = minv*(pos2-tet.pos[3]);
            lam.w = 1.0-lam.x-lam.y-lam.z;
            for (int i=0; i<4; i++)
                if(lam[i]<0.0) lam[i] = 1e-6;
        }
        pos2 = m*lam.xyz + tet.pos[3];
    }
}
