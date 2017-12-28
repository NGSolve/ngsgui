#version 150 // 400 for subdivision with multiple invocations

uniform samplerBuffer coefficients;
uniform bool clipping_plane_deformation;

// layout(lines_adjacency, invocations=4) in; // for subdivision
layout(lines_adjacency) in;
layout(triangle_strip, max_vertices=122) out;

in VertexData
{
  vec3 pos;
  flat int element;
  vec3 lam;
} inData[];

out VertexData
{
  vec3 pos;
  flat int element;
  vec3 lam;
} outData;

uniform mat4 MV;
uniform mat4 P;
uniform vec4 clipping_plane;

{include shader_functions}

float cut(vec3 x, vec3 y) {
      float dx = dot(clipping_plane, vec4(x,1.0));
      float dy = dot(clipping_plane, vec4(y,1.0));
      float a = dx/(dx-dy);
      return a;
}

void emit(vec3 x, vec3 lam) {
    outData.pos = x;
    outData.lam = lam;
    gl_Position = P * MV *vec4(x,1);
    EmitVertex();
}

void emitTrig(vec3 pos[3], vec3 lam[3]) {
    float scale = 0.1;
    for (int i=0; i<3; i++) {

      outData.pos = pos[i];
      outData.lam = lam[i];
      if(clipping_plane_deformation)
          outData.pos += scale*vec3(clipping_plane.x, clipping_plane.y, clipping_plane.z)*EvalTET(inData[0].element, outData.lam.x, outData.lam.y, outData.lam.z);
      gl_Position = P * MV *vec4(outData.pos,1);
      EmitVertex();
    }
    EndPrimitive();
}

void emitSubdividedTrigs1(vec3 pos[3], vec3 lam[3]) {
    float scale = 0.1;

        for (int first=0; first<3; first++) {
          outData.pos = pos[first];
          outData.lam = lam[first];
          if(clipping_plane_deformation)
              outData.pos += scale*vec3(clipping_plane.x, clipping_plane.y, clipping_plane.z)*EvalTET(inData[0].element, outData.lam.x, outData.lam.y, outData.lam.z);
          gl_Position = P * MV *vec4(outData.pos,1);
          EmitVertex();

          for (int i=1; i<3; i++) {
            int other = first+i;
            if(other>=3)
                other -= 3;
            outData.pos = 0.5*(pos[first]+pos[other]);
            outData.lam = 0.5*(lam[first]+lam[other]);
            if(clipping_plane_deformation)
                outData.pos += scale*vec3(clipping_plane.x, clipping_plane.y, clipping_plane.z)*EvalTET(inData[0].element, outData.lam.x, outData.lam.y, outData.lam.z);
            gl_Position = P * MV *vec4(outData.pos,1);
            EmitVertex();

            }

            EndPrimitive();
        }

        for (int first=0; first<3; first++) {
          int next = first+1;
          if(next==3)
            next = 0;

            outData.pos = 0.5*(pos[first]+pos[next]);
            outData.lam = 0.5*(lam[first]+lam[next]);
            if(clipping_plane_deformation)
                outData.pos += scale*vec3(clipping_plane.x, clipping_plane.y, clipping_plane.z)*EvalTET(inData[0].element, outData.lam.x, outData.lam.y, outData.lam.z);
            gl_Position = P * MV *vec4(outData.pos,1);
            EmitVertex();
        }
        EndPrimitive();
}

/* for subdivision (needs OpenGL 4.0)
void emitSubdividedTrigs2(vec3 pos[3], vec3 lam[3]) {
        vec3 pos1[3];
        vec3 lam1[3];

        int id1 = gl_InvocationID/4;
        int id = gl_InvocationID-4*id1;
        if(id<3) {
              int first = id;
              pos1[0] = pos[first];
              lam1[0] = lam[first];
              for (int i=1; i<3; i++) {
                int other = first+i;
                if(other>=3)
                    other -= 3;
                pos1[i] = 0.5*(pos[first]+pos[other]);
                lam1[i] = 0.5*(lam[first]+lam[other]);
              }
              emitSubdividedTrigs1(pos1, lam1);
        }

        else {

        for (int first=0; first<3; first++) {
          int next = first+1;
          if(next==3)
            next = 0;

            pos1[first] = 0.5*(pos[first]+pos[next]);
            lam1[first] = 0.5*(lam[first]+lam[next]);
        }
        emitSubdividedTrigs1(pos1, lam1);
        }
}

void emitSubdividedTrigs3(vec3 pos[3], vec3 lam[3]) {
        vec3 pos1[3];
        vec3 lam1[3];

        int id = gl_InvocationID/4;
        if(id<3) {
              int first = id;
              pos1[0] = pos[first];
              lam1[0] = lam[first];
              for (int i=1; i<3; i++) {
                int other = first+i;
                if(other>=3)
                    other -= 3;
                pos1[i] = 0.5*(pos[first]+pos[other]);
                lam1[i] = 0.5*(lam[first]+lam[other]);
              }
              emitSubdividedTrigs2(pos1, lam1);
        }

        else {

        for (int first=0; first<3; first++) {
          int next = first+1;
          if(next==3)
            next = 0;

            pos1[first] = 0.5*(pos[first]+pos[next]);
            lam1[first] = 0.5*(lam[first]+lam[next]);
        }
        emitSubdividedTrigs2(pos1, lam1);
        }
}
// for subdivision (needs OpenGL 4.0) */


void emitSubdividedTrigs(vec3 pos[3], vec3 lam[3]) {
   emitTrig(pos,lam);
   // emitSubdividedTrigs1(pos, lam);
   // emitSubdividedTrigs2(pos, lam);
   // emitSubdividedTrigs3(pos, lam);
}

void main() {
    outData.element = inData[0].element;

    int nvertices_behind = 0;
    int vertices_behind[3];
    int nvertices_front = 0;
    int vertices_front[3];
    for (int i=0; i<4; ++i) {
      float dist = dot(clipping_plane, vec4(inData[i].pos,1.0));
      if(dist>0) {
          vertices_behind[nvertices_behind] = i;
          nvertices_behind++;
      }
      else {
          vertices_front[nvertices_front] = i;
          nvertices_front++;
      }
    }
    vec3 pos[3];
    vec3 lam[3];
    if( nvertices_behind==3 ) {
        for (int i=0; i<3; ++i) {
          float a = cut(inData[vertices_front[0]].pos, inData[vertices_behind[i]].pos);
          pos[i] =  mix(inData[vertices_front[0]].pos, inData[vertices_behind[i]].pos, a);
          lam[i] =  mix(inData[vertices_front[0]].lam, inData[vertices_behind[i]].lam, a);
        }
        emitSubdividedTrigs(pos, lam);
    }
    if( nvertices_behind==1 ) {
        for (int i=0; i<3; ++i) {
          float a = cut(inData[vertices_behind[0]].pos, inData[vertices_front[i]].pos);
          pos[i] =  mix(inData[vertices_behind[0]].pos, inData[vertices_front[i]].pos, a);
          lam[i] =  mix(inData[vertices_behind[0]].lam, inData[vertices_front[i]].lam, a);
        }
        emitSubdividedTrigs(pos, lam);
    }

    if( nvertices_behind==2 ) {
        float a;
        a = cut(inData[vertices_front[0]].pos, inData[vertices_behind[0]].pos);
        pos[0] =  mix(inData[vertices_front[0]].pos, inData[vertices_behind[0]].pos, a);
        lam[0] =  mix(inData[vertices_front[0]].lam, inData[vertices_behind[0]].lam, a);

        a = cut(inData[vertices_front[0]].pos, inData[vertices_behind[1]].pos);
        pos[1] =  mix(inData[vertices_front[0]].pos, inData[vertices_behind[1]].pos, a);
        lam[1] =  mix(inData[vertices_front[0]].lam, inData[vertices_behind[1]].lam, a);

        a = cut(inData[vertices_front[1]].pos, inData[vertices_behind[1]].pos);
        pos[2] =  mix(inData[vertices_front[1]].pos, inData[vertices_behind[1]].pos, a);
        lam[2] =  mix(inData[vertices_front[1]].lam, inData[vertices_behind[1]].lam, a);
        emitSubdividedTrigs(pos, lam);

        a = cut(inData[vertices_front[0]].pos, inData[vertices_behind[0]].pos);
        pos[0] =  mix(inData[vertices_front[0]].pos, inData[vertices_behind[0]].pos, a);
        lam[0] =  mix(inData[vertices_front[0]].lam, inData[vertices_behind[0]].lam, a);
        a = cut(inData[vertices_front[1]].pos, inData[vertices_behind[1]].pos);
        pos[1] =  mix(inData[vertices_front[1]].pos, inData[vertices_behind[1]].pos, a);
        lam[1] =  mix(inData[vertices_front[1]].lam, inData[vertices_behind[1]].lam, a);
        a = cut(inData[vertices_front[1]].pos, inData[vertices_behind[0]].pos);
        pos[2] =  mix(inData[vertices_front[1]].pos, inData[vertices_behind[0]].pos, a);
        lam[2] =  mix(inData[vertices_front[1]].lam, inData[vertices_behind[0]].lam, a);
        emitSubdividedTrigs(pos, lam);
    }
}

