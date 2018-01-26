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
  vec3 corners;
  flat int index;
} inData[];

out VertexData
{
  vec3 pos;
  vec3 normal;
  flat int index;
} outData;

void main() {
    vec3 center = vec3(0.0,0.0,0.0);
    int nvertices_behind = 0;
    // vec3[4] corners;
    // mat4 bmat = mat4(vec4(inData[0].bary_pos,1.0),
    //                  vec4(inData[1].bary_pos,1.0),
    //                  vec4(inData[2].bary_pos,1.0),
    //                  vec4(inData[3].bary_pos,1.0));
    // mat4 inv = inverse(bmat);

    // for(int i = 3; i>=0; --i)
    //   {
    //     corners[i] = inv[i].x * inData[0].pos +
    //       inv[i].y * inData[1].pos +
    //       inv[i].z * inData[2].pos +
    //       inv[i].w * inData[3].pos;
    //     if(i<3)
    //       corners[i] += corners[3];
    //   }
    for (int i=0; i<4; ++i) {
      center += inData[i].corners;
      float dist = dot(clipping_plane, vec4(inData[i].corners,1.0));
      if(dist>0)
          nvertices_behind++;
    }
    center *= 0.25;

    if(nvertices_behind<4) {
        for (int i=0; i<4; i++) {
             int ids[3];
          for (int v=0; v<3; v++) {
              int id = v+i;
              if(id>3) id = id-4;
              ids[v] = id;
          }

          outData.normal = normalize(cross(inData[ids[1]].pos-inData[ids[0]].pos, inData[ids[2]].pos-inData[ids[0]].pos));
          outData.index = inData[0].index;
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

