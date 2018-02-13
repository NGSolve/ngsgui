#version 150
out VertexData
{
  flat int element;
} outData;

void main()
{
    outData.element = gl_VertexID;
}
