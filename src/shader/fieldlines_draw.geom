#version 400

{include utils.inc}
#line 4

layout(points) in;
layout(triangle_strip, max_vertices=32) out;

uniform mat4 MV;
uniform mat4 P;
uniform float grid_size;
uniform float colormap_min, colormap_max;
uniform bool colormap_linear;

void DrawVertex( vec3 pos ) {
    gl_Position = P * MV *vec4(pos,1);
    EmitVertex();
}

in VertexData
{
  vec3 pos;
  vec3 val;
  vec3 val2;
} inData[];

out VertexData
{
  vec3 normal;
  vec3 color;
} outData;

void DrawQuad( float radius, vec3 start, vec3 end, vec3 v1, vec3 v2, vec3 v3, vec3 v4 ) {
    vec3 a = start+radius*v1;
    vec3 b = start+radius*v2;
    vec3 c = end+radius*v3;
    vec3 d = end+radius*v4;

    outData.normal = v1;
    DrawVertex(a);
    outData.normal = v2;
    DrawVertex(b);
    outData.normal = v3;
    DrawVertex(c);
    outData.normal = v4;
    DrawVertex(d);
    EndPrimitive();

}

void CalcNormals( vec3 vin, out vec3 n1, out vec3 n2 ) {
    vec3 v = vin;
    float maxval = max(max(v.x, v.y), v.z);

    if(v.x == maxval)
        n1 = vec3(-v.y/v.x, 1, 0);
    else if(v.y == maxval)
        n1 = vec3(0,-v.z/v.y, 1);
    else
        n1 = vec3(1,0,-v.x/v.z);

    n1 = normalize(n1);
    n2 = normalize(cross(v, n1));
}

void DrawPipe( float radius ) {
    int n = 8;

    vec3 v0,v1,v2,v3;
    CalcNormals(inData[0].val2, v0, v1);
    CalcNormals(inData[0].val, v2, v3);

    vec3 v01 = normalize(v0+v1);
    vec3 v10 = normalize(v0-v1);
    vec3 v23 = normalize(v2+v3);
    vec3 v32 = normalize(v2-v3);
    vec3 pos = inData[0].pos;
    vec3 end = inData[0].pos+inData[0].val;

    DrawQuad(radius, pos, end,  v0,  v01,  v2,  v23);
    DrawQuad(radius, pos, end,  v01,  v1,  v23,  v3);
    DrawQuad(radius, pos, end,  v1, -v10,  v3, -v32);
    DrawQuad(radius, pos, end, -v10, -v0, -v32, -v2);
    DrawQuad(radius, pos, end, -v0, -v01, -v2, -v23);
    DrawQuad(radius, pos, end, -v01, -v1, -v23, -v3);
    DrawQuad(radius, pos, end, -v1,  v10, -v3,  v32);
    DrawQuad(radius, pos, end,  v10,  v0,  v32,  v2);
}

void main() {
    float s = 0.5*grid_size;

    float value = (length(inData[0].val)-colormap_min)/(colormap_max-colormap_min);
    value = clamp(value, 0.0, 1.0);
    value = (1.0 - value);
    if(!colormap_linear)
        value = floor(8*value)/7.0;
    outData.color.r = MapColor(value).r;
    outData.color.g = MapColor(value).g;
    outData.color.b = MapColor(value).b;

    DrawPipe( s/4);
}
