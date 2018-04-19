#version 150 

{include utils.inc}

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;
uniform float shrink_elements;

layout(triangles) in;
layout(triangle_strip, max_vertices=12) out;

in VertexData
{
  flat int element;
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

void AddPoint( int face_index, vec3 lam, Element3d tet, Element2d trig, vec3 center) {

    outData.edgedist = lam;
    outData.pos = interpolatePoint(mesh, tet, trig, face_index, lam.xy);
    outData.pos = mix(center,outData.pos,  shrink_elements);
    outData.normal = lam.x*trig.normals[0] + lam.y*trig.normals[1] + lam.z*trig.normals[2];

    outData.color = vec4(texelFetch(colors, tet.index, 0));
    gl_Position = P * MV * vec4(outData.pos, 1);
    EmitVertex();
}


void main() {

    if(mesh.dim==2) {
        Element2d el = getElement2d(mesh, inData[0].element);
        AddTrig(el, 0,1,2, 0);
        if(el.nverts==4)
            AddTrig(el, 2,1,0, 1);
    }

    if(mesh.dim==3) {
        Element3d el = getElement3d(mesh, inData[0].element);
        vec3 center = 0.25*(el.pos[0]+el.pos[1]+el.pos[2]+el.pos[3]);
        for (int face =0; face<4; face++) {
            Element2d trig = getElement2d(mesh, el, face);
            AddPoint( face, inData[0].lam, el, trig, center);
            AddPoint( face, inData[1].lam, el, trig, center);
            AddPoint( face, inData[2].lam, el, trig, center);
            EndPrimitive();
        }
    }

}
