#version 150
out VertexData
{
  flat int element;
  flat int instance;
} outData;

void main()
{
    outData.element = gl_VertexID;
    outData.instance = gl_InstanceID;
}
