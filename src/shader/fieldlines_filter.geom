#version 150

{include utils.inc}
{include interpolation.inc}
#line 5

uniform int n_steps;
uniform float step_size;

layout(points) in;
layout(points, max_vertices=40) out;

in VertexData
{
  flat int element;
} inData[];

out vec3 pos;
out vec3 pos2;
out vec3 val;

// global variables
mat3 m;
mat3 minv;
ELEMENT_TYPE tet;
vec4 lam0;
vec4 lam1;
int element;

void getM( ELEMENT_TYPE tet) {
    for (int i=0; i<ELEMENT_N_VERTICES-1; i++)
      m[i] = tet.pos[i]-tet.pos[3];
    minv = inverse(m);
}

vec3 getPos(vec4 l) {
    return m*l.xyz + tet.pos[3];
}

void writePoint() {
    pos = getPos(lam0);
    pos2 = getPos(lam1);
    val = EvaluateVec(FUNCTION, element, lam1.xyz);
    EmitVertex();
    EndPrimitive();
}

void main() {
    element = inData[0].element;
    tet = getElement(element);
    lam0 = vec4(0.25,0.25,0.25,0.25);
    lam1 = lam0;

    getM(tet);
    pos = getPos(lam0);
    float h = step_size;

    for (int i=0; i<n_steps+1;i++)
    {
        lam0 = lam1;
        vec4 v;
        v.xyz = EvaluateVec(1, element, lam0.xyz);
        v.w = -(v.x+v.y+v.z);
        lam1 += h*normalize(v);
        
        float eps = 0.0;
        if((lam0.x>0 && lam1.x<0) || (lam0.y>0&&lam1.y<0) || (lam0.z>0&&lam1.z<eps) || (lam0.w>0&&lam1.w<0)) {
            // jumping out of one element -> find adjacent element to continue
            // first cut the current trajectory with the element face
            int face = -1;
            vec4 x = lam0/(lam0-lam1);
            float minx = 1e8;

            // find the face which is cutting the trajectory first
            for (int i=0; i<4; i++) {
                if( x[i] < minx && x[i]>0.0 && lam0[i]>0.0 && lam1[i]<0.0 )
                {
                    minx = x[i];
                    face = i;
                }
            }
            if(face==-1) return; // out of mesh

            // calc point on face with lam[face] = 0.0
            lam1 = mix(lam0, lam1, minx);

            // emit point on face
            writePoint();

            int new_element = texelFetch(mesh.elements, mesh.offset+ELEMENT_SIZE*element+6+face).r;
            if(new_element==-1) return;

            // find barycentric coordinates of lam1 of adjacent element
            ivec4 verts0;
            ivec4 verts1;
            for (int i=0; i<ELEMENT_N_VERTICES; i++) {
                verts0[i] = texelFetch(mesh.elements, mesh.offset+ELEMENT_SIZE*element+i+2).r;
                verts1[i] = texelFetch(mesh.elements, mesh.offset+ELEMENT_SIZE*new_element+i+2).r;
            }
            element = new_element;
            tet = getElement(element);
            getM(tet);

            lam0 = lam1;
            lam1 = vec4(-1,-1,-1,-1);
            for (int i=0; i<ELEMENT_N_VERTICES; i++) {
                for (int j=0; j<ELEMENT_N_VERTICES; j++)
                    if(verts1[i] == verts0[j])
                        lam1[i] = lam0[j];
            }

            for (int i=0; i<ELEMENT_N_VERTICES; i++) {
                if(lam1[i]==-1) {
                    lam1[i] = 0.0;
                    lam1[i] = 1.0-lam1.x-lam1.y-lam1.z-lam1.w;
                }
            }
            lam0 = lam1;
        }
        else
          writePoint();

        float lmin = 1.0;
        lmin = min(lam1.x, lmin);
        lmin = min(lam1.y, lmin);
        lmin = min(lam1.z, lmin);
        lmin = min(lam1.w, lmin);

        float lmax = 0.0;
        lmax = max(lam1.x, lmax);
        lmax = max(lam1.y, lmax);
        lmax = max(lam1.z, lmax);
        lmax = max(lam1.w, lmax);

        // something went wrong
        if(lmin < -1e-6 || lmax > 1.0+1e-6)
            return;

        float s = lam1.x+lam1.y+lam1.z+lam1.w;
        if(abs(s-1.0)>1e-6)
            return;
    }
}

