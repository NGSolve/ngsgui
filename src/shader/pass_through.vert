#version 150

#ifndef USE_GL_VERTEX_ID
in int element;
#endif

out VertexData
{
  flat int element;
} outData;

void main()
{
#ifdef USE_GL_VERTEX_ID
  outData.element = gl_VertexID;
#else
  outData.element = element;
#endif
}
