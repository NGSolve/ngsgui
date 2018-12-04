#version 400

{include utils.inc}
#line 4

layout(points) in;
layout(triangle_strip, max_vertices=32) out;

uniform float grid_size;

void DrawVertex( vec3 pos ) {
    gl_Position = P * MV *vec4(pos,1);
    EmitVertex();
}

in VertexData
{
  vec3 pos;
  vec3 pos2;
  vec3 val;
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

void CalcNormals( vec3 v, vec3 v2, out vec3 n1, out vec3 n2, out vec3 n3, out vec3 n4) {
    vec3 vabs = abs(v);
    float maxval = max(max(vabs.x, vabs.y), vabs.z);

    if(vabs.x == maxval) {
        n1 = vec3(-v.y/v.x, 1, 0);
        n3 = vec3(-v2.y/v2.x, 1, 0);
    }
    else if(vabs.y == maxval) {
        n1 = vec3(0,-v.z/v.y, 1);
        n3 = vec3(0,-v2.z/v2.y, 1);
    }
    else {
        n1 = vec3(1,0,-v.x/v.z);
        n3 = vec3(1,0,-v2.x/v2.z);
    }

    n1 = normalize(n1);
    n3 = normalize(n3);
    n2 = normalize(cross(v, n1));
    n4 = normalize(cross(v2, n3));
}

void DrawPipe( float radius ) {
    vec3 pos = inData[0].pos;
    vec3 end = inData[0].pos2;

    if(!CalcClipping(pos)) return;

    vec3 v0,v1,v2,v3;
    //CalcNormals(end-pos, end-pos , v0, v1, v2, v3);
    CalcNormals(end-pos, inData[0].val, v0, v1, v2, v3);

    vec3 v01 = normalize(v0+v1);
    vec3 v10 = normalize(v0-v1);
    vec3 v23 = normalize(v2+v3);
    vec3 v32 = normalize(v2-v3);

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

    float value = length(inData[0].val);
    outData.color.rgb = MapColor(value);

    DrawPipe( s/4);
}
