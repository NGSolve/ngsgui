#version 150

{include utils.inc}
#line 5

uniform bool wireframe;
uniform sampler1D colors;

in VertexData
{
  vec3 normal;
  vec3 value;
} inData;

out vec4 FragColor;

vec3 TransformVec( vec3 x) {
    return normalize(inverse(mat3(MV))*x);
}

void main()
{
  FragColor.rgb = MapColor(length(inData.value));
  FragColor.a = 1.0;

  vec3 lightVector = TransformVec(vec3(1,3,3));
  FragColor.rgb *= light.ambient+light.diffuse*clamp(dot(normalize(inData.normal), lightVector), 0, 1.0);
}
