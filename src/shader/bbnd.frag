#version 400
uniform mat4 MV;
uniform float light_ambient;
uniform float light_diffuse;

in VertexData
{
  vec3 pos;
  vec4 color;
} inData;

out vec4 FragColor;

vec3 TransformVec( vec3 x) {
    return normalize(inverse(mat3(MV))*x);
}
void main()
{
  FragColor = inData.color;
  if(FragColor.a == 0.0) discard;
}
