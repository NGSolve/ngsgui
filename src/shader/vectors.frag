#version 150

{include utils.inc}
#line 5

uniform mat4 MV;
uniform Mesh mesh;
uniform vec4 clipping_plane;
uniform bool do_clipping;
uniform bool wireframe;
uniform float light_ambient;
uniform float light_diffuse;
uniform sampler1D colors;

in VertexData
{
  vec3 normal;
  vec3 color;
} inData;

out vec4 FragColor;

vec3 TransformVec( vec3 x) {
    return normalize(inverse(mat3(MV))*x);
}

void main()
{
  FragColor = vec4(inData.color, 1);

  vec3 lightVector = TransformVec(vec3(1,3,3));
  FragColor.rgb *= light_ambient+light_diffuse*clamp(dot(normalize(inData.normal), lightVector), 0, 1.0);
}
