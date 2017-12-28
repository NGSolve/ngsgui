#version 150
uniform vec4 fColor;
uniform vec4 fColor_clipped;
uniform vec4 clipping_plane;

out vec4 FragColor;

in VertexData
{
  vec3 pos;
} inData;

void main()
{
  if(dot(vec4(inData.pos,1.0),clipping_plane)<0)
    FragColor = fColor;
  else
    discard;
    // FragColor = fColor_clipped;
}
