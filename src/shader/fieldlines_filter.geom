#version 150

{include utils.inc}
{include interpolation.inc}
#line 5

// uniform samplerBuffer coefficients;
// uniform int subdivision;
// uniform int order;
// uniform int component;
uniform int n_steps;

layout(points) in;
layout(points, max_vertices=100) out;

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
    ELEMENT_TYPE tet = getElement(element);
    vec3 lam = vec3(0.25,0.25,0.25);
    vec3 lam_last;

    mat3 m, minv;
    getM(tet, m, minv);
    pos = lam;
    float h = 0.1;
    float len;
    vec3 v;

    for (int i=0; i<n_steps+1;i++) {
        pos2 = m*lam + tet.pos[3];
        v = EvaluateVec(FUNCTION, element, lam);
        if(i>0) {
            vec3 p3 = m*(lam+v)+ tet.pos[3];
            val = p3-pos2;
            EmitVertex();
            EndPrimitive();
        }
        len = length(v);
        lam_last = lam;
        pos = m*lam_last + tet.pos[3];
        lam = lam + h*normalize(v);
        float w = 1.0-lam.x-lam.y-lam.z;
        if(lam.x<0.0 || lam.y<0.0 || lam.z<0.0 || w<0.0) {
            // jumping out of one element -> find adjacent element to continue
            // first cut the current trajectory with the element face
            int face = -1;
            vec4 x;
            float wlam = 1.0-lam.x-lam.y-lam.z;
            float wlaml = 1.0-lam_last.x-lam_last.y-lam_last.z;
            x.xyz = lam_last/(lam_last-lam);
            x.w = wlaml/(wlaml-wlam);
            float minx = 99999.0;
            vec4 lam4 = vec4(lam, 1.0-lam.x-lam.y-lam.z);
            for (int i=0; i<4; i++) {
                if( x[i] < minx && x[i]>0.0)
                {
                    minx = x[i];
                    face = i;
                }
            }
            if(face==-1) return;
            // find point on face with lam[face] = 0.0
            lam = mix(lam_last, lam, minx);
//             pos2 = m*lam + tet.pos[3];

//             for (int i=0; i<3; i++)
//                 if(lam[i]<0.0) lam[i] = 1e-6;

            int new_element = texelFetch(mesh.elements, mesh.offset+ELEMENT_SIZE*element+6+face).r;
            if(new_element==-1) return;

            ivec4 verts0;
            ivec4 verts1;
            lam_last = lam;
            vec4 llast = vec4(lam, 1.0-lam.x-lam.y-lam.z);
            lam = vec3(0.0, 0.0, 0.0);
            for (int i=0; i<ELEMENT_N_VERTICES; i++) {
                verts0[i] = texelFetch(mesh.elements, mesh.offset+ELEMENT_SIZE*element+i+2).r;
                verts1[i] = texelFetch(mesh.elements, mesh.offset+ELEMENT_SIZE*new_element+i+2).r;
            }
            for (int i=0; i<ELEMENT_N_VERTICES-1; i++) {
                for (int j=0; j<ELEMENT_N_VERTICES; j++)
                    if(verts1[i] == verts0[j])
                        lam[i] = llast[j];
            }


            element = new_element;
            tet = getElement(element);
            getM(tet, m, minv);

        }
    }
}
