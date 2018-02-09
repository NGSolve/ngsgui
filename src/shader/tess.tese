#version 410 core

layout(triangles, equal_spacing, cw) in;
// layout(triangles, fractional_even_spacing, cw) in;

uniform mat4 P;
uniform mat4 MV;

in VertexData
{
  vec3 pos;
  vec3 normal;
  vec3 other_pos;
  flat int index;
  flat int curved_index;
} inData[];

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec3 other_pos;
  float edgedist;
  flat int index;
  flat int curved_index;
} outData;


void InterpolatePos() {
    float x = gl_TessCoord.x;
    float y = gl_TessCoord.y;
    float z = gl_TessCoord.z;
    vec3 f[6];
    f[0] = inData[2].pos;         // #
    f[1] = inData[1].other_pos;
    f[2] = inData[0].pos;         // #
    f[3] = inData[0].other_pos;
    f[4] = inData[2].other_pos;
    f[5] = inData[1].pos;         // #

    vec3 pos;
    pos = 1.0*f[0] + pow(x, 2)*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + pow(y, 2)*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]);
    // pos = inData[0].pos*x + inData[1].pos*y+inData[2].pos*z;
    // pos = inData[0].other_pos*x + inData[1].other_pos*y+inData[2].other_pos*z;
    // pos = f[1]*x + f[3]*y + f[5]*z;
    outData.pos = pos;
    outData.other_pos = pos;
    gl_Position = P * MV * vec4(pos, 1);
}

void InterpolateNormal() {
    float x = gl_TessCoord.x;
    float y = gl_TessCoord.y;
    float z = gl_TessCoord.z;
    outData.normal = x*inData[0].normal + y*inData[1].normal + z*inData[2].normal;
}

void main()
{
    // vec3 p0 = gl_TessCoord.x * inData[0].pos;
    // vec3 p1 = gl_TessCoord.y * inData[1].pos;
    // vec3 p2 = gl_TessCoord.z * inData[2].pos;
    // vec3 pos = p0 + p1 + p2;
    // gl_Position = P * MV * vec4(pos, 1);
    // outData.pos = pos;
    outData.edgedist = min(min(gl_TessCoord.x, gl_TessCoord.y), gl_TessCoord.z);

    outData.index = inData[0].index;

    // outData.pos = gl_TessCoord.x*inData[0].pos+gl_TessCoord.y*inData[1].pos+gl_TessCoord.z*inData[2].pos;
    // outData.normal = gl_TessCoord.x*inData[0].normal+gl_TessCoord.y*inData[1].normal+gl_TessCoord.z*inData[2].normal;
    // outData.other_pos = gl_TessCoord.x*inData[0].other_pos+gl_TessCoord.y*inData[1].other_pos+gl_TessCoord.z*inData[2].other_pos;
    // outData.other_normal = gl_TessCoord.x*inData[0].other_normal+gl_TessCoord.y*inData[1].other_normal+gl_TessCoord.z*inData[2].other_normal;

    outData.index = inData[0].index;
    outData.curved_index = inData[0].curved_index;
    InterpolatePos();
    InterpolateNormal();
}
