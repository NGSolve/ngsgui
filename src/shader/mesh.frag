#version 150

{include utils.inc}
#line 5

uniform mat4 MV;
uniform Mesh mesh;
uniform vec4 clipping_plane;
uniform bool do_clipping;
uniform bool wireframe;
uniform float light_ambient;
uniform float light_diffuse;
uniform sampler1D colors;

in VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} inData;

out vec4 FragColor;

vec3 TransformVec( vec3 x) {
    return normalize(inverse(mat3(MV))*x);
}

void main()
{
  if(wireframe) {
      vec3 l = inData.lam;
#ifndef ET_QUAD
      float d = min(min(l.x, l.y), l.z);
#else
      float d =  min(min(l.x,l.y), min(1-l.x,1.0-l.y));
#endif
      if(d>1e-5) discard;
  }
  int index = texelFetch(mesh.elements, ELEMENT_SIZE*inData.element + 1).r;
  if(index==-1)
    FragColor = vec4(0,0,0,1);
  else
    FragColor = vec4(texelFetch(colors, index, 0));

  if(do_clipping && dot(vec4(inData.pos,1.0),clipping_plane)<0)
    discard;

  if (FragColor.a == 0.0)
    discard;


  vec3 lightVector = TransformVec(vec3(1,3,3));
  FragColor.rgb *= light_ambient+light_diffuse*clamp(dot(normalize(inData.normal), lightVector), 0, 1.0);
}
