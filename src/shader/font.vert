#version 150

in int char_;

out VertexData
{
  flat int character;
  flat int index;
} outData;

void main()
{
  outData.character = char_;
  outData.index = gl_VertexID;
  gl_Position = vec4(0,0,0,1);
  // gl_Position = P * MV * vec4(start_pos, 1.0);
}
