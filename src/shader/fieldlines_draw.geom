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
} inData[];

out VertexData
{
  vec3 normal;
  vec3 color;
} outData;

void DrawQuad( float radius, vec3 start, vec3 end, vec3 v1, vec3 v2 ) {
    vec3 a = start+radius*v1;
    vec3 b = start+radius*v2;
    vec3 c = end+radius*v1;
    vec3 d = end+radius*v2;

    outData.normal = v1;
    DrawVertex(a);
    outData.normal = v2;
    DrawVertex(b);
    outData.normal = v1;
    DrawVertex(c);
    outData.normal = v2;
    DrawVertex(d);
    EndPrimitive();

}

void DrawPipe( vec3 start, vec3 end, float radius ) {
    int n = 8;

    vec3 v0 = end-start;
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

    DrawQuad(radius, start, end,  v1,  v3);
    DrawQuad(radius, start, end,  v3,  v2);
    DrawQuad(radius, start, end,  v2, -v4);
    DrawQuad(radius, start, end, -v4, -v1);
    DrawQuad(radius, start, end, -v1, -v3);
    DrawQuad(radius, start, end, -v3, -v2);
    DrawQuad(radius, start, end, -v2,  v4);
    DrawQuad(radius, start, end,  v4,  v1);
}

void main() {
    float s = 0.5*grid_size;
    vec3 val = inData[0].val;

    float value = (length(inData[0].val)-colormap_min)/(colormap_max-colormap_min);
    value = clamp(value, 0.0, 1.0);
    value = (1.0 - value);
    if(!colormap_linear)
        value = floor(8*value)/7.0;
    outData.color.r = MapColor(value).r;
    outData.color.g = MapColor(value).g;
    outData.color.b = MapColor(value).b;

    DrawPipe( inData[0].pos , inData[0].pos+inData[0].val, s/4);
}
