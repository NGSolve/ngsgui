#version 150
uniform mat4 MV;
uniform mat4 P;

in vec3 pos;
in vec3 normal;
in vec3 other_normal;
in vec3 other_pos;
in int index;
in int curved_index;

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec3 other_pos;
  vec3 other_normal;
  flat int index;
  flat int curved_index;
} outData;

void main()
{
    gl_Position = P * MV * vec4(pos, 1.0);
    if(curved_index==-1)
        gl_Position = vec4(0,0,0,1);

    outData.pos = pos;
    outData.index = index;
    outData.curved_index = curved_index;
    outData.normal = normal;
    outData.other_normal = other_normal;
    outData.other_pos = other_pos;
}
