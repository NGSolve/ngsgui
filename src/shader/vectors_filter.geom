#version 150
#define VOLUME_GRID 0
#define CLIPPING_PLANE_GRID 1
#ifndef FILTER_MODE
#error FILTER_MODE not set (either VOLUME_GRID or CLIPPING_PLANE_GRID)
#endif


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


#if FILTER_MODE==VOLUME_GRID
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
