#version 150

{include utils.inc}

uniform mat4 MV;
uniform mat4 P;
uniform Mesh mesh;
uniform sampler1D colors;

out VertexData
{
  // flat int element;
  flat Element2d el;
  /*
  vec3 pos;
  vec3 normal;
  vec4 color;
  */
} outData;

void main()
{
  // outData.element = gl_VertexID;
  outData.el = getElement2d(mesh, gl_VertexID );
  /*

    int vert_in_element = gl_VertexID-3*element;
    Element2d el = getElement2d(mesh, element );

    outData.normal = el.normals[vert_in_element];
    outData.pos = el.pos[vert_in_element];
    gl_Position = P * MV * vec4(outData.pos, 1.0);

    outData.color = vec4(texelFetch(colors, el.index, 0));

    // Discard whole element if color is completely transparent
    if(outData.color.a==0.0)
        gl_Position = vec4(0,0,0,0);
        */
}
