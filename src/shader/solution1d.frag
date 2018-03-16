#version 150

in VertexData
{
  flat int element;
} inData;

out vec4 FragColor;

{include utils.inc}
{include interpolation.inc}

void main()
{
  FragColor = vec4(0,0,0,1);
}
