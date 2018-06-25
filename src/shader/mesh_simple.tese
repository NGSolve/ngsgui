#version 410 core
{DEFINES}

#if defined(ET_SEGM)
layout(isolines) in;
#elif defined(ET_TRIG)
layout(triangles) in;
#elif defined(ET_QUAD)
layout(quads) in;
#endif

{include utils.inc}
#line 13

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;

in VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} inData[];

out VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} outData;

void main()
{
    outData.element = inData[0].element;

    float x = gl_TessCoord.x;
    float y = gl_TessCoord.y;
    float z = 1.0-x-y;

    int offset = texelFetch(mesh.elements, ELEMENT_SIZE*inData[0].element + ELEMENT_SIZE-1).r;

#if defined(ET_SEGM)
    vec3 a = inData[0].pos;
    vec3 b = texelFetch(mesh.vertices, offset+2).xyz;
    vec3 c = inData[1].pos;
    outData.pos = a + x*(-c-3*a+4*b) + x*x*2*(a-2*b+c);
    outData.normal = mix(inData[0].normal, inData[1].normal, x);
    outData.lam = vec3(x,0,0);
#elif defined(ET_TRIG)
    vec3 f[6];
    f[0] = inData[2].pos;
    f[2] = inData[0].pos;
    f[5] = inData[1].pos;
    f[1] = texelFetch(mesh.vertices, offset+3).xyz;
    f[3] = texelFetch(mesh.vertices, offset+4).xyz;
    f[4] = texelFetch(mesh.vertices, offset+5).xyz;
    outData.pos = 1.0*f[0] + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]);
    outData.normal = x*inData[0].normal + y*inData[1].normal + z*inData[2].normal;
    outData.lam = vec3(x,y,z);
#elif defined(ET_QUAD)
    vec3 f[9];
    f[0] = inData[0].pos;
    f[2] = inData[1].pos;
    f[8] = inData[2].pos;
    f[6] = inData[3].pos;
    f[1] = texelFetch(mesh.vertices, offset+4).xyz;
    f[3] = texelFetch(mesh.vertices, offset+5).xyz;
    f[4] = texelFetch(mesh.vertices, offset+6).xyz;
    f[5] = texelFetch(mesh.vertices, offset+7).xyz;
    f[7] = texelFetch(mesh.vertices, offset+8).xyz;
    outData.pos = 1.0*f[0] + x*x*y*y*(4.0*f[0] - 8.0*f[1] + 4.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 4.0*f[6] - 8.0*f[7] + 4.0*f[8]) - x*x*y*(6.0*f[0] - 12.0*f[1] + 6.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 2.0*f[6] - 4.0*f[7] + 2.0*f[8]) + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) - x*y*y*(6.0*f[0] - 8.0*f[1] + 2.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 6.0*f[6] - 8.0*f[7] + 2.0*f[8]) + x*y*(9.0*f[0] - 12.0*f[1] + 3.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 3.0*f[6] - 4.0*f[7] + 1.0*f[8]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[6]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[6]);
    vec3 n1 = mix(inData[0].normal, inData[1].normal, x);
    vec3 n2 = mix(inData[3].normal, inData[2].normal, x);
    outData.normal = mix(n1,n2, y);
//    outData.lam = vec3(min(x,1-y), min(1-x,y), 1);
    outData.lam = vec3(x,y,1);
#else
    unknown type
#endif
    gl_Position = P * MV * vec4(outData.pos, 1);
}
