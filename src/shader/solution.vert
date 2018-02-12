#version 150
uniform mat4 MV;
uniform mat4 P;

{include utils.inc}
uniform Mesh mesh;

out VertexData
{
  vec3 lam;
  vec3 pos;
  flat int element;
} outData;

void main()
{
    outData.element = gl_VertexID/3;
    int vertex_in_element = gl_VertexID - 3*outData.element;

    Element2d el = getElement2d(mesh, outData.element );
    outData.pos = el.pos[vertex_in_element];

    outData.lam = vec3(0,0,0);
    if(vertex_in_element==0) outData.lam.x = 1.0;
    if(vertex_in_element==1) outData.lam.y = 1.0;
    outData.lam.z = 1.0 - outData.lam.x - outData.lam.y;

    gl_Position = P * MV * vec4(outData.pos, 1.0);
}
