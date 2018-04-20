#version 410 core

layout(isolines, equal_spacing) in;

{include utils.inc}

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;

in VertexData
{
  flat int element;
} inData[];

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec4 color;
  vec3 edgedist;
} outData;

void main()
{
    float lam = gl_TessCoord.x;

    if(mesh.dim==1) {
        Element1d el = getElement1d(mesh, inData[0].element);
        vec3 p = interpolatePoint(mesh, el, lam);
        outData.pos = el.pos[0]*(1-lam) + el.pos[1]*lam;
        outData.pos = p;
        outData.normal = vec3(1,0,0);
        outData.color = vec4(texelFetch(colors, el.index, 0));
        outData.edgedist = vec3(0,0,0);
        gl_Position = P * MV * vec4(outData.pos, 1);
    }
}
