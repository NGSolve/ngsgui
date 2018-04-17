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
  flat Element2d el;
  vec3 lam;
} inData[];

out VertexData
{
  vec3 pos;
  vec3 normal;
  vec4 color;
  vec3 edgedist;
} outData;


void AddPoint( bool flip, vec3 lam, Element2d el ) {
    if(el.nverts==3) {
      outData.edgedist = lam;
      outData.pos = interpolatePoint(mesh, el, lam.xy);
      outData.normal = lam.x*el.normals[0] + lam.y*el.normals[1] + lam.z*el.normals[2];
    }
    else {
      outData.edgedist = vec3(lam.xz, 1.0);
//       if(lam.y<0.99)
//         lam.x /= 1.0-lam.y;
//       lam.y /= 1.0-lam.z;
      // TODO: curving for quads
      if(flip)
          lam = 1.0-lam;
      outData.pos = interpolatePoint(mesh, el, lam.xz);
//       outData.normal = lam.x*el.normals[i0] + lam.y*el.normals[i1] + lam.z*el.normals[i2];
      outData.normal = mix(mix(el.normals[0],el.normals[1], lam.x), mix(el.normals[3], el.normals[2],lam.x),lam.z);
//      outData.pos = lam.x*el.pos[i0] + lam.y*el.pos[i1] + lam.z*el.pos[i2];
    }

    outData.color = vec4(texelFetch(colors, el.index, 0));
    gl_Position = P * MV * vec4(outData.pos, 1);
    EmitVertex();
}

void main() {

    Element2d el = inData[0].el;
//     calcNormals(el);
    AddPoint(false, inData[0].lam, el);
    AddPoint(false, inData[1].lam, el);
    AddPoint(false, inData[2].lam, el);
    EndPrimitive();
    if(el.nverts==4) {
        AddPoint(true, inData[2].lam, el);
        AddPoint(true, inData[1].lam, el);
        AddPoint(true, inData[0].lam, el);
    }
    EndPrimitive();

}
