#version 150 // 400 for subdivision with multiple invocations
{include utils.inc}

uniform mat4 MV;
uniform mat4 P;
uniform vec4 clipping_plane;
uniform Mesh mesh;
uniform sampler1D colors;
uniform float shrink_elements;

layout(points) in;
layout(triangle_strip, max_vertices=12) out;

in VertexData
{
  flat int element;
} inData[];

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec4 color;
} outData;

void main() {
    Element3d tet = getElement3d(mesh, inData[0].element);
    float mindist = 1e19;
    for (int i=0; i<4; i++)
        mindist = min(mindist, dot(clipping_plane, vec4(tet.pos[i],1)));
    if(mindist > 0 )
    {
    vec3 center = 0.25*(tet.pos[0]+tet.pos[1]+tet.pos[2]+tet.pos[3]);

    for (int face=0; face<4; face++) {
        Element2d trig = getElement2d(tet, face);
        for (int j=0; j<3; j++) {
            outData.pos = mix(center, trig.pos[j], shrink_elements);
            outData.normal = trig.normals[j];
            outData.color = texelFetch(colors, trig.index, 0);

            gl_Position = P * MV *vec4(outData.pos,1);
            EmitVertex();
        }
        EndPrimitive();
    }
    }
}

