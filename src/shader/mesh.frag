#version 150
uniform mat4 MV;
uniform vec4 fColor;
uniform vec4 fColor_clipped;
uniform vec4 clipping_plane;
uniform bool use_index_color;
uniform sampler1D index_color;
uniform bool do_clipping;
uniform bool draw_edges;

out vec4 FragColor;

in VertexData
{
  vec3 pos;
  vec3 normal;
  vec3 other_pos;
  vec3 other_normal;
  flat int index;
  flat int curved_index;
} inData;


void main()
{
  if(!do_clipping || dot(vec4(inData.pos,1.0),clipping_plane)<0) {
    FragColor = vec4(texelFetch(index_color, inData.index, 0));
    if(!use_index_color) {
      FragColor.rgb = fColor.rgb;
    }
    vec3 lightVector = normalize(vec3(-1,-3,-3)).xyz;
    vec3 normal = -normalize(inverse(transpose(mat3(MV)))*inData.normal).xyz;
    // vec3 normal = normalize(inData.normal);
    float ambient = 0.3;
    float diffuse = 0.7;
    FragColor.rgb *= ambient+diffuse*clamp(dot(normal, lightVector), 0, 1.0);
    if(FragColor.a==0.0)
      discard;
  } else {
    discard;
    // FragColor = fColor_clipped;
  }
}
