#version 150
#define VOLUME_GRID 0
#define CLIPPING_PLANE_GRID 1
#define FIELDLINES 2
#ifndef FILTER_MODE
#error FILTER_MODE not set (either VOLUME_GRID or CLIPPING_PLANE_GRID or FIELDLINES)
#endif


{include utilsnew.inc}
{include interpolation.inc}
#line 12

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

uniform mat4 MV;
uniform mat4 P;
uniform vec4 clipping_plane;

void getMinMax(ELEMENT_TYPE el, out vec3 pmin, out vec3 pmax)
{
    pmin = el.pos[0];
    pmax = el.pos[0];

    for (int i=1; i<4; i++) {
        pmin.x = min(pmin.x, el.pos[i].x);
        pmin.y = min(pmin.y, el.pos[i].y);
        pmin.z = min(pmin.z, el.pos[i].z);

        pmax.x = max(pmax.x, el.pos[i].x);
        pmax.y = max(pmax.y, el.pos[i].y);
        pmax.z = max(pmax.z, el.pos[i].z);
    }
    pmin = pmin - fract( pmin/grid_size ) * grid_size;
    vec4 p = vec4(pmin,1);
}

mat4 getBaryMatrix( ELEMENT_TYPE tet )
{
    // define 4 planes of tet
    mat4 normals;
    normals[0].xyz = cross(tet.pos[1]-tet.pos[2], tet.pos[1]-tet.pos[3]);
    normals[1].xyz = cross(tet.pos[0]-tet.pos[2], tet.pos[0]-tet.pos[3]);
    normals[2].xyz = cross(tet.pos[0]-tet.pos[1], tet.pos[0]-tet.pos[3]);
    normals[3].xyz = cross(tet.pos[0]-tet.pos[1], tet.pos[0]-tet.pos[2]);

    for (int i=0; i<4; i++) { 
        normals[i].w = -dot(normals[i].xyz, tet.pos[3-i]);
        normals[i] /= dot(normals[i], vec4(tet.pos[i],1));
    }
    return transpose(normals);
}


#if FILTER_MODE==FIELDLINES
// void findNextElement(out ELEMENT_TYPE tet, out vec4 lam, out int element) {
//     int other;
//     int offset = ELEMENT_SIZE*element;
//     float minlam = 1.0;
//     int minind = 0;
//     int nneg = 0;
//     for (int i=0; i<4; i++) {
//         if(lam[i] <= min(minlam, 0.0) )
//         {
//             minlam = lam[i];
//             minind = i;
//             nneg++;
//         }
//     }
// 
//     other = texelFetch(mesh.elements, offset+6+minind).r;
// //     other = element+1;
//     lam[minind] = 0.0;
// 
//     //if(nneg>1) return -1;
//     if(other==-1) {
//         element = -1;
//         return;
//     }
// 
//     lam = normalize(lam);
//     vec4 new_lam;
//     ivec4 verts0;
//     ivec4 verts1;
//     for (int i=0; i<ELEMENT_N_VERTICES; i++) {
//         verts0[i] = texelFetch(mesh.elements, ELEMENT_SIZE*element+i+2).r;
//         verts1[i] = texelFetch(mesh.elements, ELEMENT_SIZE*other+i+2).r;
//     }
//     for (int i=0; i<ELEMENT_N_VERTICES-1; i++) {
//         for (int j=0; j<ELEMENT_N_VERTICES; j++)
//             if(verts1[i] == verts0[j])
//                 new_lam[i] = lam[j];
// 
//     }
//     lam = new_lam;
//     element = other;
//     tet = getElement(mesh, element);
// //     lam = vec3(0.25,0.25,0.25);
// }

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
//         v = vec3(h,0,0);
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
#elif FILTER_MODE==VOLUME
void main() {
    ELEMENT_TYPE tet = getElement(mesh, inData[0].element);

    vec3 pmin,pmax;
    getMinMax(tet, pmin, pmax);
    pmin = pmin - fract( pmin/grid_size ) * grid_size;

    mat4 bary_mat = getBaryMatrix(tet);

    vec4 p = vec4(pmin,1);
    int counter = 0;
    while(p.x<pmax.x) {
        p.y = pmin.y;
        while(p.y<pmax.y) {
            p.z = pmin.z;
            while(p.z<pmax.z) {
                vec4 lam = bary_mat * p;
                if(dot(clipping_plane, p)>0 && 
                   lam.x>=0 && lam.y>=0 && lam.z >=0 && lam.w >=0 &&
                   lam.x<1 && lam.y<1 && lam.z <1 && lam.w <1 )
                {
                    counter++;
                    if(counter==40) return;
                    pos = p.xyz;
                    val = EvaluateElementVec(inData[0].element, coefficients, ORDER, subdivision, lam.xyz, component);
                    EmitVertex();
                    EndPrimitive();
                }
                p.z += grid_size;
            }
            p.y += grid_size;
        }
        p.x += grid_size;
    }
}
#elif FILTER_MODE==CLIPPING_PLANE_GRID
void main() {
    ELEMENT_TYPE tet = getElement(mesh, inData[0].element);
    float dmin = dot(vec4(tet.pos[0],1), clipping_plane);
    float dmax = dot(vec4(tet.pos[0],1), clipping_plane);
    for (int i=1; i<4; i++) {
        dmin = min(dmin, dot(vec4(tet.pos[i],1), clipping_plane));
        dmax = max(dmax, dot(vec4(tet.pos[i],1), clipping_plane));
    }

    if(dmin>0 || dmax <0) return;

    mat3 base; // Orthogonal system with first two base vectors inside clipping plane, third vector clipping plane normal vector
    base[2] = normalize(clipping_plane.xyz);
    vec3 n = abs(clipping_plane.xyz);

    if(n.z <= min(n.x, n.y))
        base[0] = normalize(vec3(-base[2].y, base[2].x, 0));
    else
        base[0] = normalize(vec3(0, -base[2].z, base[2].y));

    base[1] = normalize(cross(base[0], base[2]));
    mat3 invbase = inverse(base);

    mat4 bary_mat = getBaryMatrix(tet);

    /////////////////////////////////////////////////////
    // Careful, We change the coordinate system from here!

    for (int i=0; i<4; i++)
        tet.pos[i] = invbase*tet.pos[i];

    vec3 pmin,pmax;
    getMinMax(tet, pmin, pmax);
    pmin = pmin - fract( pmin/grid_size ) * grid_size;
    pmin.z = -clipping_plane.w;

    vec3 p = pmin;
    int counter = 0;
    while(p.x<pmax.x) {
        p.y = pmin.y;
        while(p.y<pmax.y) {
            p.z = -clipping_plane.w;
            vec4 lam = bary_mat * vec4(base*p,1);
            if(lam.x>=0 && lam.y>=0 && lam.z >=0 && lam.w >=0 &&
               lam.x<1 && lam.y<1 && lam.z <1 && lam.w <1 )
            {
                counter++;
                if(counter==40) return;
                pos = base*p.xyz;
                val = EvaluateElementVec(inData[0].element, coefficients, ORDER, subdivision, lam.xyz, component);
                EmitVertex();
                EndPrimitive();
            }
            p.y += grid_size;
        }
        p.x += grid_size;
    }
}
#endif
