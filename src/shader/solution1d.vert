#version 150
uniform mat4 MV;
uniform mat4 P;
uniform samplerBuffer coefficients;
uniform int subdivision;
uniform int order;

{include utils.inc}
uniform Mesh mesh;

out VertexData
{
  flat int element;
} outData;

void main()
{
    int n = subdivision+1;
    int N = order*n+1;

    int ei = gl_VertexID/(2*N-2);
    int id = (gl_VertexID+1)/2;
    int vertex_in_element = id - (N-1)*ei;
    float lam = vertex_in_element;
    lam = lam/(N-1);

    Element1d el = getElement1d(mesh, ei );
    vec3 pos = vec3(0,0,0);
    pos.x = mix(el.pos[0].x, el.pos[1].x, lam);
    pos.y = texelFetch(coefficients, 2+N*ei+vertex_in_element).r;
    pos.z = 0;

    gl_Position = P * MV * vec4(pos, 1.0);
}
