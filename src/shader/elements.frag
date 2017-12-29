#version 150
uniform mat4 MV;
in VertexData
{
  vec3 pos;
  vec3 normal;
} inData;

out vec4 FragColor;

void main()
{
  vec3 lightVector = normalize(vec3(-1,-3,-3)).xyz;
  vec3 normal = normalize(inverse(transpose(mat3(MV)))*inData.normal).xyz;
  vec3 color = vec3(0,0,0.4);
  color.b += 0.7*clamp(dot(normal, lightVector), 0,1);
  FragColor = vec4(color, 1);
}
