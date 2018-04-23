#version 400 // 400 for subdivision with multiple invocations

{include utils.inc}

uniform samplerBuffer coefficients;
uniform float colormap_min, colormap_max;
uniform Mesh mesh;
uniform bool have_gradient;
uniform int component;

layout(points) in;
layout(triangle_strip, max_vertices=24) out;

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
uniform int subdivision;
uniform int order;

void CutSubTet(float values[8], vec3 normals[8], vec3 pos[8], vec3 lams[8], int nodes[4]) {
        // subtet
        Element3d el;
        vec3 norm[4];
        float vals[4];
        vec3 lam[4];

        for (int i=0; i<4; i++) {
            el.pos[i] = pos[nodes[i]];
            norm[i] = normals[nodes[i]];
            vals[i] = values[nodes[i]];
            lam[i] = lams[nodes[i]];
        }

        vec3 outpos[4];

        int n_cutting_points = CutElement3d( el, vals, outpos, lam, norm );

        if(n_cutting_points >= 3) {
            if(!have_gradient) {
                outData.normal = cross(outpos[1]-outpos[0], outpos[2]-outpos[0]);
            }
            for (int i=0; i<n_cutting_points; i++) {
                outData.pos = outpos[i];
                outData.lam = lam[i];
                if(have_gradient)
                  outData.normal = norm[i];
                gl_Position = P * MV *vec4(outData.pos,1);
                EmitVertex();
            }
            EndPrimitive();
        }
}

void main() {
    int N = order*(subdivision+1)+1;
    int n = N-1;

    int index = inData[0].instance;
    ivec3 ind;
    ind.x = index - n*(index/n);
    index = index/n;
    ind.y = index - n*(index/n);
    ind.z = index/n;
    if(ind.x+ind.y+ind.z>=n) return;

    int values_per_element = N*(N+1)*(N+2)/6;

    outData.element = inData[0].element;
    // undivided large tet
    Element3d tet1 = getElement3d(mesh, inData[0].element);

    float h = 1.0/float(N-1);

    float values[8];
    vec3 pos[8];
    vec3 normals[8];
    vec3 lams[8];

    for (int dz=0; dz<=1; dz++)
        for (int dy=0; dy<=1; dy++)
            for (int dx=0; dx<=1; dx++) {
                ivec3 ii = ivec3(ind.x+dx, ind.y+dy, ind.z+dz);
                if(ii.x+ii.y+ii.z>n) continue;
                int i = 4*dz + 2*dy + dx;
                vec4 lam = vec4(1.0-h*(ii.x+ii.y+ii.z), ii.x*h, ii.y*h, ii.z*h);
                lams[i] = lam.xyz;
                pos[i] = lam.x * tet1.pos[0] + lam.y * tet1.pos[1] + lam.z * tet1.pos[2] + lam.w * tet1.pos[3];
                vec4 data = texelFetch(coefficients, values_per_element*inData[0].element + getIndex(N, ii.x, ii.y, ii.z)+0);
                values[i] = data[component] - colormap_max;
                normals[i] = data.yzw;
            }
    
    CutSubTet(values, normals, pos, lams, int[4](0,1,2,4));

    if(ind.x+ind.y+ind.z+1>=n) return;
    CutSubTet(values, normals, pos, lams, int[4](1,2,4,6));
    CutSubTet(values, normals, pos, lams, int[4](1,4,5,6));
    CutSubTet(values, normals, pos, lams, int[4](1,2,3,6));
    CutSubTet(values, normals, pos, lams, int[4](1,3,5,6));

    if(ind.x+ind.y+ind.z+2>=n) return;
    CutSubTet(values, normals, pos, lams, int[4](3,5,6,7));
}
