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
  float edgedist;
  flat int index;
  flat int curved_index;
} inData;

vec3 TransformPos( vec3 x) {
  vec4 X = MV*vec4(x, 1);
  X*= 1.0/X.w;
  return X.xyz;
}

vec3 TransformVec( vec3 x) {
    return normalize(inverse(mat3(MV))*x);
}

void main()
{

  if(draw_edges && inData.edgedist>1e-5) discard;
  // FragColor.rgba = fColor.rgba;
  if(!do_clipping || dot(vec4(inData.pos,1.0),clipping_plane)<0) {
    FragColor = vec4(texelFetch(index_color, inData.index, 0));
    if(!use_index_color) {
      FragColor.rgb = fColor.rgb;
    }
    vec3 normal = inData.normal;
    vec3 lightVector = TransformVec(vec3(1,3,3));
    // vec3 viewVector = TransformVec(vec3(0,0,1));
    // if(dot(normal,viewVector)<0)
    //   normal = -normal;
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
