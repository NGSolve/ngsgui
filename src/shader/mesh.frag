#version 150
uniform vec4 fColor;
uniform vec4 fColor_clipped;
uniform vec4 clipping_plane;
uniform bool use_index_color;
uniform int max_index;

out vec4 FragColor;

in VertexData
{
  vec3 pos;
  flat int index;
} inData;

{include utils.inc}

void main()
{
  if(dot(vec4(inData.pos,1.0),clipping_plane)<0) {
    if(use_index_color) {
      float val = float(inData.index)/float(max_index);
      FragColor = vec4(MapColor(val), 1.0);
    } else {
      FragColor = fColor;
    }
  } else {
    discard;
    // FragColor = fColor_clipped;
  }
}
