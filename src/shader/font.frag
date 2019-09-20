#version 150
uniform sampler2D font;

uniform vec4 font_color;

out vec4 FragColor;

in VertexData_out
{
  vec2 tex_coordinate;
} inData;


void main()
{
  float x = texture(font, inData.tex_coordinate).r;
  if(x==0) discard;
  FragColor = vec4(0,0,0,x);
}
