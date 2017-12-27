class clipping:
    fragment =  """
#version 150
uniform vec4 fColor;
uniform vec4 fColor_clipped;

uniform vec4 clipping_plane;

out vec4 FragColor;

in VertexData
{
  vec3 pos;
} inData;

void main()
{
  FragColor = fColor;
}
"""

    vertex = """
#version 150
uniform mat4 MV;
uniform mat4 P;
in vec3 vPos;

out VertexData
{
  vec3 pos;
} outData;

void main()
{
    gl_Position = P * MV * vec4(vPos, 1.0);
    outData.pos = vPos;
}
"""

    geometry_solution = """
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

{shader_functions}

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

"""
    geometry = """
#version 150

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices=6) out;

in VertexData
{
  vec3 pos;
} inData[];

out VertexData
{
  vec3 pos;
} outData;

uniform mat4 MV;
uniform mat4 P;
uniform vec4 clipping_plane;

vec3 cut(vec3 x, vec3 y) {
      float dx = dot(clipping_plane, vec4(x,1.0));
      float dy = dot(clipping_plane, vec4(y,1.0));
      float a = dx/(dx-dy);
      vec3 res =  mix(x,y,a);
      return res;
}

void emit(vec3 x) {
    outData.pos = x;
    gl_Position = P * MV *vec4(x,1);
    EmitVertex();
}


void main() {

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
    if( nvertices_behind==-1 ) {
        outData.pos = inData[0].pos;
        gl_Position = P * MV *vec4(inData[0].pos,1);
        EmitVertex();
        outData.pos = inData[1].pos;
        gl_Position = P * MV *vec4(inData[1].pos,1);
        EmitVertex();
        outData.pos = inData[2].pos;
        gl_Position = P * MV *vec4(inData[2].pos,1);
        EmitVertex();
        EndPrimitive();
    }
    if( nvertices_front==-1 ) {
        outData.pos = inData[vertices_behind[0]].pos;
        gl_Position = P * MV *vec4(inData[vertices_behind[0]].pos,1);
        EmitVertex();
        outData.pos = inData[1].pos;
        gl_Position = P * MV *vec4(inData[vertices_behind[1]].pos,1);
        EmitVertex();
        outData.pos = inData[2].pos;
        gl_Position = P * MV *vec4(inData[vertices_behind[2]].pos,1);
        EmitVertex();
        EndPrimitive();
    }
    if( nvertices_behind==3 ) {
        vec3 x = inData[vertices_front[0]].pos;
        for (int i=0; i<3; ++i) {
          vec3 y = inData[vertices_behind[i]].pos;
          vec3 res = cut(x,y);
          outData.pos = res;
          gl_Position = P * MV * vec4(res,1);
          EmitVertex();
        }
        EndPrimitive();
    }
    if( nvertices_behind==1 ) {
        vec3 x = inData[vertices_behind[0]].pos;
        for (int i=0; i<3; ++i) {
          vec3 y = inData[vertices_front[i]].pos;
          vec3 res = cut(x,y);
          outData.pos = res;
          gl_Position = P * MV * vec4(res,1);
          EmitVertex();
        }
        EndPrimitive();
    }

    if( nvertices_behind==2 ) {
        vec3 res;

        emit(cut(inData[vertices_front[0]].pos, inData[vertices_behind[0]].pos));
        emit(cut(inData[vertices_front[0]].pos, inData[vertices_behind[1]].pos));
        emit(cut(inData[vertices_front[1]].pos, inData[vertices_behind[1]].pos));
        EndPrimitive();

        emit(cut(inData[vertices_front[0]].pos, inData[vertices_behind[0]].pos));
        emit(cut(inData[vertices_front[1]].pos, inData[vertices_behind[1]].pos));
        emit(cut(inData[vertices_front[1]].pos, inData[vertices_behind[0]].pos));
        EndPrimitive();
    }
}

"""

class mesh:
    fragment =  """
#version 150
uniform vec4 fColor;
uniform vec4 fColor_clipped;
uniform vec4 clipping_plane;

out vec4 FragColor;

in VertexData
{
  vec3 pos;
} inData;

void main()
{
  if(dot(vec4(inData.pos,1.0),clipping_plane)<0)
    FragColor = fColor;
  else
    discard;
    // FragColor = fColor_clipped;
}
"""

    vertex = """
#version 150
uniform mat4 MV;
uniform mat4 P;
in vec3 vPos;

out VertexData
{
  vec3 pos;
} outData;

void main()
{
    gl_Position = P * MV * vec4(vPos, 1.0);
    outData.pos = vPos;
}
"""

class solution:
    fragment_header = """
#version 150
uniform samplerBuffer coefficients;
uniform float colormap_min, colormap_max;
uniform bool colormap_linear;
uniform int element_type;
uniform vec4 clipping_plane;
uniform bool do_clipping;

in VertexData
{
  vec3 pos;
  flat int element;
  vec3 lam;
} inData;

out vec4 FragColor;

vec3 MapColor(float value)
{
    value = (value-colormap_min)/(colormap_max-colormap_min);
    value = clamp(value, 0.0, 1.0);
    value = (1.0 - value);
    if(!colormap_linear)
      value = floor(8*value)/7.0;
    value = clamp(value, 0.0, 1.0);
    vec3 res;
    res.r = clamp(2.0-4.0*value, 0.0, 1.0);
    res.g = clamp(2.0-4.0*abs(0.5-value), 0.0, 1.0);
    res.b = clamp(4.0*value - 2.0, 0.0, 1.0);
    return res;
}

float zahn(float x, float y) {
  return atan(1000*x*y*y - floor(1000*x*y*y));
}

{shader_functions}

void main()
{
  if(!do_clipping || dot(vec4(inData.pos,1.0),clipping_plane)<0)
  {
      float x = inData.lam.x;
      float y = inData.lam.y;
      float z = inData.lam.z;
      //  { ET_POINT = 0, ET_SEGM = 1,
      //    ET_TRIG = 10, ET_QUAD = 11, 
      //    ET_TET = 20, ET_PYRAMID = 21, ET_PRISM = 22, ET_HEX = 24 };
      float value;
      if(element_type == 10) value = EvalTRIG(inData.element, x,y,z);
      if(element_type == 20) value = EvalTET(inData.element, x,y,z);
      if(element_type == 21) value = EvalPYRAMID(inData.element, x,y,z);
      if(element_type == 22) value = EvalPRISM(inData.element, x,y,z);
      if(element_type == 24) value = EvalHEX(inData.element, x,y,z);
      FragColor.r = MapColor(value).r;
      FragColor.g = MapColor(value).g;
      FragColor.b = MapColor(value).b;
      FragColor.a = 1.0;
  }
  else
    discard;
}
"""

    fragment_main = """
"""

    vertex = """
#version 150
uniform mat4 MV;
uniform mat4 P;

in vec3 vPos;
in vec3 vLam;
in int vElementNumber;

out VertexData
{
  vec3 pos;
  flat int element;
  vec3 lam;
} outData;

void main()
{
    gl_Position = P * MV * vec4(vPos, 1.0);
//    outData.lam = vec3(0.0, 0.0, 0.0);
    outData.pos = vPos; //0.5*vPos +0.5;
    outData.element = vElementNumber; //gl_VertexID/3; //vIndex/3;
    outData.lam = vLam;
}
"""

geometry_copy = """
#version 420

layout(triangles) in;
layout(triangle_strip, max_vertices=6) out;

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

void main() {
    // vec3 normal = cross(inData[1].pos-inData[0].pos, inData[2].pos-inData[0].pos);
    // normal = normal/sqrt(dot(normal,normal));

    // fBrightness = 0.3+0.7*clamp(dot(normal,vec3(1,1,1)/sqrt(3)), 0.0, 1.0);

    outData.element = inData[0].element;

    for (int i=0; i<3; ++i) {
      gl_Position = P * MV * vec4(inData[i].pos,1);
      outData.pos = inData[i].pos;
      outData.lam = inData[i].lam;
      EmitVertex();
    }
    EndPrimitive();
}

"""
