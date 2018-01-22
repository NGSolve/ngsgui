#version 150
uniform mat4 MV;
uniform sampler1D mat_color;

in VertexData
{
  vec3 pos;
  vec3 normal;
  flat int index;
} inData;

out vec4 FragColor;

void main()
{
  vec3 lightVector = normalize(vec3(-1,-3,-3)).xyz;
  vec3 normal = normalize(inverse(transpose(mat3(MV)))*inData.normal).xyz;
  FragColor = vec4(texelFetch(mat_color,inData.index,0));
  if(FragColor.a==0.0)
    discard;
  float ambient = 0.3;
  float diffuse = 0.7;
  FragColor.rgb *= ambient+diffuse*clamp(dot(normal, lightVector), 0, 1.0);
}
