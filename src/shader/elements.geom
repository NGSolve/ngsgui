#version 150 // 400 for subdivision with multiple invocations

uniform float shrink_elements;

uniform mat4 MV;
uniform mat4 P;
uniform vec4 clipping_plane;

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices=12) out;

in VertexData
{
  vec3 pos;
} inData[];

out VertexData
{
  vec3 pos;
  vec3 normal;
} outData;

void main() {
    vec3 center = vec3(0.0,0.0,0.0);
    int nvertices_behind = 0;
    for (int i=0; i<4; ++i) {
      center += inData[i].pos;
      float dist = dot(clipping_plane, vec4(inData[i].pos,1.0));
      if(dist>0)
          nvertices_behind++;
    }
    // TODO: use barycentric coordinates of vertices to calculate the center (needed for prisms etc.)
    center *= 0.25;

    if( nvertices_behind<4) {
        for (int i=0; i<4; i++) {
             int ids[3];
          for (int v=0; v<3; v++) {
              int id = v+i;
              if(id>3) id = id-4;
              ids[v] = id;
          }

          outData.normal = normalize(cross(inData[ids[1]].pos-inData[ids[0]].pos, inData[ids[2]].pos-inData[ids[0]].pos));
          if(dot(center - inData[ids[0]].pos, outData.normal)<0.0)
              outData.normal = -outData.normal;

          for (int v=0; v<3; v++) {
              outData.pos = mix(center, inData[ids[v]].pos, shrink_elements);
              //  outData.pos = inData[ids[v]].pos;
              gl_Position = P * MV *vec4(outData.pos,1);
              EmitVertex();
          }
          EndPrimitive();
        }
    }
}

