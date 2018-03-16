#version 400

{include utils.inc}

uniform samplerBuffer coefficients;
uniform Mesh mesh;

layout(points) in;
layout(triangle_strip, max_vertices=48) out;

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
uniform int subdivision;
uniform int order;

void DrawVertex( vec3 pos ) {
    outData.pos = pos;
    gl_Position = P * MV *vec4(outData.pos,1);
    EmitVertex();
}

void DrawTrig( float radius, vec3 base, vec3 v1, vec3 v2, vec3 c ) {
    vec3 a = base+radius*v1;
    vec3 b = base+radius*v2;

    outData.normal = v1;
    DrawVertex(a);
    outData.normal = v2;
    DrawVertex(b);
    outData.normal = normalize(v1+v2);
    DrawVertex(c);
    EndPrimitive();

    outData.normal = base-c;
    DrawVertex(b);
    DrawVertex(a);
    DrawVertex(base);
    EndPrimitive();
}

void DrawCone( vec3 base, vec3 top, float radius ) {
    int n = 8;

    vec3 v0 = top-base;
    vec3 v1 = abs(v0);

    float maxval = max(max(v1.x, v1.y), v1.z);

    if(v1.x == maxval)
        v1 = vec3(-v0.y/v0.x, 1, 0);
    else if(v1.y == maxval)
        v1 = vec3(0,-v0.z/v0.y, 1);
    else
        v1 = vec3(1,0,-v0.x/v0.z);

    v1 = normalize(v1);
    vec3 v2 = normalize(cross(v0, v1));

    outData.normal = vec3(1,0,0);

    vec3 v3 = normalize(v1+v2);
    vec3 v4 = normalize(v1-v2);

    DrawTrig(radius, base,  v1,  v3, top);
    DrawTrig(radius, base,  v3,  v2, top);
    DrawTrig(radius, base,  v2, -v4, top);
    DrawTrig(radius, base, -v4, -v1, top);
    DrawTrig(radius, base, -v1, -v3, top);
    DrawTrig(radius, base, -v3, -v2, top);
    DrawTrig(radius, base, -v2,  v4, top);
    DrawTrig(radius, base,  v4,  v1, top);
}

void main() {
    outData.element = inData[0].element;
    outData.lam = vec3(0,0,0);

    int N = order*(subdivision+1)+1;
    int n = N-1;
    int values_per_element = N*(N+1)*(N+2)/6;
    vec4 data = texelFetch(coefficients, 2+values_per_element*inData[0].element);

    Element3d tet = getElement3d(mesh, inData[0].element);
    vec3 pmin = min(min(tet.pos[0], tet.pos[1]), min(tet.pos[2], tet.pos[3]));
    vec3 pmax = max(max(tet.pos[0], tet.pos[1]), max(tet.pos[2], tet.pos[3]));
    vec3 base = 0.25*(tet.pos[0]+tet.pos[1]+tet.pos[2]+tet.pos[3]);
    float radius = 0.06;
    vec3 val = normalize(data.xyz);
    DrawCone( base-0.5*radius*val, base+0.5*radius*val, radius/20);
}
