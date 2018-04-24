#version 150

out VertexData
{
  flat int element;
  flat int instance;
} outData;

in int element;

void main()
{
  outData.element = element;
  outData.instance = gl_InstanceID;
}
