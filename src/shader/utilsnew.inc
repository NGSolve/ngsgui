{include utils.inc}
#line 20002

struct POINT {
  vec3 pos[1];
};

struct SEGM {
  vec3 pos[2];
  vec3 normal;
  int curved_vertices;
  int index;
};

struct TRIG {
  vec3 pos[3];
  vec3 normal;
  int curved_vertices;
  int index;
};

struct QUAD {
  vec3 pos[4];
  vec3 normal;
  int curved_vertices;
  int index;
};

struct TET {
  vec3 pos[4];
  int curved_vertices;
  int index;
};

struct PYRAMID {
  vec3 pos[5];
  int curved_vertices;
  int index;
};

struct PRISM {
  vec3 pos[6];
  int curved_vertices;
  int index;
};

struct HEX {
  vec3 pos[8];
  int curved_vertices;
  int index;
};

ivec4 getTetFromElement( PYRAMID el, int subtet, out TET tet, out vec3 lam[4]) {
  vec3 all_lams[5] = vec3[5]( vec3(0,0,0), vec3(1,0,0), vec3(1,1,0), vec3(0,1,0), vec3(0,0,1) );
  ivec4 vi;

  if(subtet==0) vi=ivec4( 0,1,2,4 );
  else vi=ivec4( 0,2,3,4 );

  for (int i=0; i<4; i++) {
    tet.pos[i] = el.pos[vi[i]];
    lam[i] = all_lams[vi[i]];
  }
  return vi;
}

ivec4 getTetFromElement( PRISM el, int subtet, out TET tet, out vec3 lam[4]) {
  vec3 all_lams[6] = vec3[6](
    vec3(1,0,0), vec3(0,1,0), vec3(0,0,0),
    vec3(1,0,1), vec3(0,1,1), vec3(0,0,1) );
  ivec4 vi;

  if(subtet==0) vi=ivec4( 0,1,2,3 );
  else if(subtet==1) vi=ivec4( 1,2,3,5 );
  else vi=ivec4( 1,3,4,5 );

  for (int i=0; i<4; i++) {
    tet.pos[i] = el.pos[vi[i]];
    lam[i] = all_lams[vi[i]];
  }
  return vi;
}

ivec4 getTetFromElement( HEX el, int subtet, out TET tet, out vec3 lam[4]) {
  vec3 all_lams[8] = vec3[8](
    vec3(0,0,0), vec3(1,0,0), vec3(1,1,0), vec3(0,1,0),
    vec3(0,0,1), vec3(1,0,1), vec3(1,1,1), vec3(0,1,1) );
  ivec4 vi;
  if(subtet==0) vi=ivec4( 0,1,3,4 );
  else if(subtet==1) vi=ivec4( 1,3,4,7 );
  else if(subtet==2) vi=ivec4( 1,4,5,7 );
  else if(subtet==3) vi=ivec4( 2,5,6,7 );
  else if(subtet==4) vi=ivec4( 1,2,3,7 );
  else vi=ivec4( 1,2,5,7 );

  for (int i=0; i<4; i++) {
    tet.pos[i] = el.pos[vi[i]];
    lam[i] = all_lams[vi[i]];
  }
  return vi;
}

struct PointData {
  vec3 pos;
  vec3 normal;
  vec3 edgedist;
};

PointData interpolatePoint(Mesh mesh, SEGM el, vec3 lam) {
  PointData p;
  p.pos =  mix(el.pos[0], el.pos[1], lam.x);
  p.normal =  el.normal;
  p.edgedist = vec3(0,0,0);
  return p;
}

PointData interpolatePoint(Mesh mesh, TRIG el, vec3 lam) {
  PointData p;
  p.pos =  mix(el.pos[0], el.pos[1], lam.x);
  p.normal =  el.normal;
  p.edgedist = vec3(0,0,0);
  return p;
}


#ifdef ELEMENT_TYPE
ELEMENT_TYPE getElement(Mesh mesh, int elnr ) {
    ELEMENT_TYPE el;
#ifdef ET_POINT
    el.pos[0] = texelFetch(mesh.vertices, elnr).xyz;
#else // ET_POINT
    int offset = ELEMENT_SIZE*elnr;
    el.index = texelFetch(mesh.elements, offset +1).r;
    for (int i=0; i<ELEMENT_N_VERTICES; i++) {
        int v = texelFetch(mesh.elements, offset+i+2).r;
        el.pos[i] = texelFetch(mesh.vertices, v).xyz;
    }
#ifdef CURVED
    el.curved_vertices = texelFetch(mesh.elements, offset + ELEMENT_SIZE-1).r;
#endif
#if defined(ET_TRIG) || defined(ET_QUAD)
    el.normal = cross(el.pos[1]-el.pos[0], el.pos[2]-el.pos[0]);
#endif
#endif // ET_POINT
    return el;
}
#endif // ELEMENT_TYPE

// Cut tet with plane and store 0-4 points (and barycentric coords), return the number of intersection points
int CutElement3d( TET tet, float values[4], out vec3 pos[4], inout vec3 lam[4] ) {
    int nvertices_behind = 0;
    int vertices_behind[3];
    int nvertices_front = 0;
    int vertices_front[3];
    for (int i=0; i<4; ++i) {
      // float dist = dot(plane, vec4(tet.pos[i],1.0));
      float dist = values[i];
      if(dist>0) {
          vertices_behind[nvertices_behind] = i;
          nvertices_behind++;
      }
      else {
          vertices_front[nvertices_front] = i;
          nvertices_front++;
      }
    }
    // vec3 lams[4] = vec3[4]( vec3(0,0,0), vec3(1,0,0), vec3(0,1,0), vec3(0,0,1)); // vec3(0,0,0));
    vec3 lams[4] = lam; // vec3[4]( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1), vec3(0,0,0));
    if( nvertices_behind==0 || nvertices_behind==4 ) return 0;
    if( nvertices_behind==3 ) {
        for (int i=0; i<3; ++i) {
          float vx = values[vertices_front[0]];
          float vy = values[vertices_behind[i]];
          float a = vx/(vx-vy);
          // float a = CutEdge(plane, tet.pos[vertices_front[0]] , tet.pos[vertices_behind[i]]);
          pos[i] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[i]], a);
          lam[i] =  mix(lams[vertices_front[0]], lams[vertices_behind[i]], a);
        }
        return 3;
    }
    if( nvertices_behind==1 ) {
        for (int i=0; i<3; ++i) {
          float vx = values[vertices_behind[0]];
          float vy = values[vertices_front[i]];
          float a = vx/(vx-vy);
          // float a = CutEdge(plane, tet.pos[vertices_behind[0]], tet.pos[vertices_front[i]]);
          pos[i] =  mix(tet.pos[vertices_behind[0]], tet.pos[vertices_front[i]], a);
          lam[i] =  mix(lams[vertices_behind[0]], lams[vertices_front[i]], a);
        }
        return 3;
    }

    if( nvertices_behind==2 ) {
        float a, vx, vy;
        vx = values[vertices_front[0]];
        vy = values[vertices_behind[1]];
        a = vx/(vx-vy);
        // a = CutEdge(plane, tet.pos[vertices_front[0]], tet.pos[vertices_behind[1]]);
        pos[0] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[1]], a);
        lam[0] =  mix(lams[vertices_front[0]], lams[vertices_behind[1]], a);

        vx = values[vertices_front[0]];
        vy = values[vertices_behind[0]];
        a = vx/(vx-vy);
        // a = CutEdge(plane, tet.pos[vertices_front[0]], tet.pos[vertices_behind[0]]);
        pos[1] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[0]], a);
        lam[1] =  mix(lams[vertices_front[0]], lams[vertices_behind[0]], a);

        vx = values[vertices_front[1]];
        vy = values[vertices_behind[1]];
        a = vx/(vx-vy);
        // a = CutEdge(plane, tet.pos[vertices_front[1]], tet.pos[vertices_behind[1]]);
        pos[2] =  mix(tet.pos[vertices_front[1]], tet.pos[vertices_behind[1]], a);
        lam[2] =  mix(lams[vertices_front[1]], lams[vertices_behind[1]], a);

        vx = values[vertices_front[1]];
        vy = values[vertices_behind[0]];
        a = vx/(vx-vy);
        // a = CutEdge(plane, tet.pos[vertices_front[1]], tet.pos[vertices_behind[0]]);
        pos[3] =  mix(tet.pos[vertices_front[1]], tet.pos[vertices_behind[0]], a);
        lam[3] =  mix(lams[vertices_front[1]], lams[vertices_behind[0]], a);
        return 4;
    }           
}

int CutElement3d( HEX el, int subtet, float values[8], out vec3 pos[4], inout vec3 lam[4] ) {
  TET tet;
  ivec4 vi = getTetFromElement( el, subtet, tet, lam);
  float tet_values[4];
  for (int i=0; i<4; i++)
    tet_values[i] = values[vi[i]];
  return CutElement3d(tet, tet_values, pos, lam);
}

// Cut tet with plane and store 0-4 points (and barycentric coords), return the number of intersection points
int CutElement3d( TET tet, float values[4], out vec3 pos[4], inout vec3 lam[4], inout vec3 normals[4] ) {
    int nvertices_behind = 0;
    int vertices_behind[3];
    int nvertices_front = 0;
    int vertices_front[3];
    vec3 normals_ori[4];
    for (int i=0; i<4; ++i) {
      // float dist = dot(plane, vec4(tet.pos[i],1.0));
      normals_ori[i] = normals[i];
      float dist = values[i];
      if(dist>0) {
          vertices_behind[nvertices_behind] = i;
          nvertices_behind++;
      }
      else {
          vertices_front[nvertices_front] = i;
          nvertices_front++;
      }
    }
    // vec3 lams[4] = vec3[4]( vec3(0,0,0), vec3(1,0,0), vec3(0,1,0), vec3(0,0,1)); // vec3(0,0,0));
    // vec3 lams[4] = vec3[4]( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1), vec3(0,0,0));
    vec3 lams[4] = lam;
    if( nvertices_behind==0 || nvertices_behind==4 ) return 0;
    if( nvertices_behind==3 ) {
        for (int i=0; i<3; ++i) {
          float vx = values[vertices_front[0]];
          float vy = values[vertices_behind[i]];
          float a = vx/(vx-vy);
          // float a = CutEdge(plane, tet.pos[vertices_front[0]] , tet.pos[vertices_behind[i]]);
          pos[i] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[i]], a);
          lam[i] =  mix(lams[vertices_front[0]], lams[vertices_behind[i]], a);
          normals[i] =  mix(normals_ori[vertices_front[0]], normals_ori[vertices_behind[i]], a);
        }
        return 3;
    }
    if( nvertices_behind==1 ) {
        for (int i=0; i<3; ++i) {
          float vx = values[vertices_behind[0]];
          float vy = values[vertices_front[i]];
          float a = vx/(vx-vy);
          // float a = CutEdge(plane, tet.pos[vertices_behind[0]], tet.pos[vertices_front[i]]);
          pos[i] =  mix(tet.pos[vertices_behind[0]], tet.pos[vertices_front[i]], a);
          lam[i] =  mix(lams[vertices_behind[0]], lams[vertices_front[i]], a);
          normals[i] =  mix(normals_ori[vertices_behind[0]], normals_ori[vertices_front[i]], a);
        }
        return 3;
    }

    if( nvertices_behind==2 ) {
        float a, vx, vy;
        vx = values[vertices_front[0]];
        vy = values[vertices_behind[1]];
        a = vx/(vx-vy);
        // a = CutEdge(plane, tet.pos[vertices_front[0]], tet.pos[vertices_behind[1]]);
        pos[0] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[1]], a);
        lam[0] =  mix(lams[vertices_front[0]], lams[vertices_behind[1]], a);
        normals[0] =  mix(normals_ori[vertices_front[0]], normals_ori[vertices_behind[1]], a);

        vx = values[vertices_front[0]];
        vy = values[vertices_behind[0]];
        a = vx/(vx-vy);
        // a = CutEdge(plane, tet.pos[vertices_front[0]], tet.pos[vertices_behind[0]]);
        pos[1] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[0]], a);
        lam[1] =  mix(lams[vertices_front[0]], lams[vertices_behind[0]], a);
        normals[1] =  mix(normals_ori[vertices_front[0]], normals_ori[vertices_behind[0]], a);

        vx = values[vertices_front[1]];
        vy = values[vertices_behind[1]];
        a = vx/(vx-vy);
        // a = CutEdge(plane, tet.pos[vertices_front[1]], tet.pos[vertices_behind[1]]);
        pos[2] =  mix(tet.pos[vertices_front[1]], tet.pos[vertices_behind[1]], a);
        lam[2] =  mix(lams[vertices_front[1]], lams[vertices_behind[1]], a);
        normals[2] =  mix(normals_ori[vertices_front[1]], normals_ori[vertices_behind[1]], a);

        vx = values[vertices_front[1]];
        vy = values[vertices_behind[0]];
        a = vx/(vx-vy);
        // a = CutEdge(plane, tet.pos[vertices_front[1]], tet.pos[vertices_behind[0]]);
        pos[3] =  mix(tet.pos[vertices_front[1]], tet.pos[vertices_behind[0]], a);
        lam[3] =  mix(lams[vertices_front[1]], lams[vertices_behind[0]], a);
        normals[3] =  mix(normals_ori[vertices_front[1]], normals_ori[vertices_behind[0]], a);
        return 4;
    }           
}
