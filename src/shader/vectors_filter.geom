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

void main() {
    ELEMENT_TYPE tet = getElement(mesh, inData[0].element);
    vec3 pmin = tet.pos[0];
    vec3 pmax = tet.pos[0];

    for (int i=1; i<4; i++) {
        pmin.x = min(pmin.x, tet.pos[i].x);
        pmin.y = min(pmin.y, tet.pos[i].y);
        pmin.z = min(pmin.z, tet.pos[i].z);

        pmax.x = max(pmax.x, tet.pos[i].x);
        pmax.y = max(pmax.y, tet.pos[i].y);
        pmax.z = max(pmax.z, tet.pos[i].z);
    }
    pmin = pmin - fract( pmin/grid_size ) * grid_size;
    vec4 p = vec4(pmin,1);

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
    
    int counter = 0;
    float e = 0;
    while(p.x<pmax.x) {
        p.y = pmin.y;
        while(p.y<pmax.y) {
            p.z = pmin.z;
            while(p.z<pmax.z) {
                vec4 lam = transpose(normals) * p;
                if(dot(clipping_plane, p)>0 && 
                   lam.x>=0 && lam.y>=0 && lam.z >=0 && lam.w >=0 &&
                   lam.x<1 && lam.y<1 && lam.z <1 && lam.w <1 )
                {
                    counter++;
                    if(counter==40) return;
                    pos = p.xyz;
                    val = InterpolateTetVec(inData[0].element, coefficients, ORDER, subdivision, lam.xyz, component);
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
