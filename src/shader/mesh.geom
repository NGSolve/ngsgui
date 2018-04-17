#version 150 

{include utils.inc}

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;

layout(triangles) in;
layout(triangle_strip, max_vertices=6) out;

in VertexData
{
  flat int el_id;
  vec3 lam;
} inData[];

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec4 color;
  vec3 edgedist;
} outData;


void AddPoint( int face_index, vec3 lam, Element2d el ) {
    if(el.nverts==3) {
      outData.edgedist = lam;
      outData.pos = interpolatePoint(mesh, el, lam.xy);
      outData.normal = lam.x*el.normals[0] + lam.y*el.normals[1] + lam.z*el.normals[2];
    }
    else {
      outData.edgedist = vec3(lam.xz, 1.0);
      if(face_index==1)
          lam = 1.0-lam;
      outData.pos = interpolatePoint(mesh, el, lam.xz);
      outData.normal = mix(mix(el.normals[0],el.normals[1], lam.x), mix(el.normals[3], el.normals[2],lam.x),lam.z);
    }

    outData.color = vec4(texelFetch(colors, el.index, 0));
    gl_Position = P * MV * vec4(outData.pos, 1);
    EmitVertex();
}

void AddTrig( Element2d el, int v0, int v1, int v2, int face_index ) {
    AddPoint(face_index, inData[v0].lam, el);
    AddPoint(face_index, inData[v1].lam, el);
    AddPoint(face_index, inData[v2].lam, el);
    EndPrimitive();
}

void main() {

    Element2d el = getElement2d(mesh, inData[0].el_id);
    AddTrig(el, 0,1,2, 0);
    if(el.nverts==4)
        AddTrig(el, 2,1,0, 1);

}
