#version 150
uniform mat4 MV;
uniform vec4 clipping_plane;
uniform bool do_clipping;
uniform bool wireframe;
uniform float light_ambient;
uniform float light_diffuse;

in VertexData
{
  vec3 pos;
  vec3 normal;
  vec4 color;
  float edgedist;
} inData;

out vec4 FragColor;

vec3 TransformVec( vec3 x) {
    return normalize(inverse(mat3(MV))*x);
}

void main()
{
  if(wireframe && inData.edgedist>1e-5) discard;
  FragColor = inData.color;


  if(do_clipping && dot(vec4(inData.pos,1.0),clipping_plane)<0)
    discard;


  vec3 lightVector = TransformVec(vec3(1,3,3));
  FragColor.rgb *= light_ambient+light_diffuse*clamp(dot(normalize(inData.normal), lightVector), 0, 1.0);
}
