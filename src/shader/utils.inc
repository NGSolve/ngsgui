#line 10001

uniform mat4 MV;
uniform mat4 P;

#ifdef CURVED
#endif


uniform struct Colormap {
  float min;
  float max;
  sampler1D colors;
  int n;
  bool linear;
} colormap;

uniform struct Light {
  vec3 dir;
  float ambient;
  float diffuse;
  float spec;
  float shininess;
} light;

#if !defined(NO_CLIPPING) && defined(CLIPPING)
uniform vec4 clipping_planes[N_CLIPPING_PLANES];

bool CalcClipping(vec3 pos)
{
  bool p[N_CLIPPING_PLANES];
  vec4 pos4 = vec4(pos, 1.0);
  for (int i=0; i<N_CLIPPING_PLANES; i++)
      p[i] = dot(clipping_planes[i], pos4) < 0;
  return CLIPPING_EXPRESSION;
}
#else // clipping disabled
bool CalcClipping(vec3 pos) { return true; }
#endif

uniform struct Mesh {
    int dim;
    int offset;
    isamplerBuffer elements;
    samplerBuffer vertices;
} mesh;

vec3 TransformVec(mat4 MV, vec3 x) {
    return normalize(transpose(mat3(MV))*x);
}

vec3 CalcLight(vec3 color, mat4 MV, vec3 position, vec3 norm)
{
  #ifdef NOLIGHT
    return color;
  #else
        mat3 mvt = transpose(inverse(mat3(MV)));
	vec3 n = normalize( mvt*norm );
	vec3 s = normalize( light.dir);
        vec4 p = MV*vec4(position,1);
	vec3 v = normalize( -p.xyz/p.w );
	vec3 r = reflect( -s, n );

	float ambient = light.ambient;

	// float sDotN = max( dot( s, n ), 0.0 );
	float sDotN = abs( dot( s, n ));
	float diffuse = light.diffuse * sDotN;

	// spec = Light[lightIndex].Ls * Material.Ks * pow( max( dot(r,v) , 0.0 ), Material.Shininess );
	float spec = pow( max( dot(r,v) , 0.0 ), light.shininess );
        if(diffuse==0.0) spec = 0.0;
        return color*(ambient+diffuse) + spec*light.spec*vec3(1,1,1);
  #endif
}

vec3 MapColor(float value)
{
  value = (value-colormap.min)/(colormap.max-colormap.min);
  value = clamp(value, 0.0, 1.0-1e-7);
  if(colormap.linear)
      return texture(colormap.colors, value).rgb;

  int n = int(colormap.n*value);
  return texelFetch(colormap.colors, n, 0).rgb;
}

vec3 MapColorIso(float value)
{
  value = (value-colormap.min)/(colormap.max-colormap.min);
  value = clamp(value, 0.0, 1.0-1e-7);
  int n = int(colormap.n*value);

  vec3 col;
  if(colormap.linear)
    col = texture(colormap.colors, value).rgb;
  else
    col = texelFetch(colormap.colors, n, 0).rgb;

  float rest = colormap.n*value - n;
  rest = max(rest, 1.0-rest);
  rest = rest*rest;
  rest = rest*rest;
  rest = rest*rest;
  rest = rest*rest;
  rest = rest*rest;
  rest = rest*rest;
  rest = 1.0-rest;
  return rest*col;
}

int getIndex(int N, int x, int y) {
  int res = N*(N+1)/2 - (N-y)*(N-y+1)/2;
  res += x;
  return res;
}

int getIndex(int N, int x, int y, int z) {
  int res = N*(N+1)*(N+2)/6 - (N-z)*(N-z+1)*(N-z+2)/6;
  res += getIndex(N-z, x, y);
  return res;
}

/////////////////////////////////////////////////////////////////////
// Code for elements
/////////////////////////////////////////////////////////////////////

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

#ifdef ELEMENT_TYPE

// material index of element
int getElementIndex(int elnr) {
#ifdef ET_POINT
  return 0;
#endif
    int offset = mesh.offset+ELEMENT_SIZE*elnr;
    return texelFetch(mesh.elements, offset +1).r;
}

ELEMENT_TYPE getElement(int elnr ) {
    ELEMENT_TYPE el;
#ifdef ET_POINT
    el.pos[0] = texelFetch(mesh.vertices, elnr).xyz;
#else // ET_POINT
    int offset = mesh.offset+ELEMENT_SIZE*elnr;
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
        pos[0] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[1]], a);
        lam[0] =  mix(lams[vertices_front[0]], lams[vertices_behind[1]], a);

        vx = values[vertices_front[0]];
        vy = values[vertices_behind[0]];
        a = vx/(vx-vy);
        pos[1] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[0]], a);
        lam[1] =  mix(lams[vertices_front[0]], lams[vertices_behind[0]], a);

        vx = values[vertices_front[1]];
        vy = values[vertices_behind[1]];
        a = vx/(vx-vy);
        pos[2] =  mix(tet.pos[vertices_front[1]], tet.pos[vertices_behind[1]], a);
        lam[2] =  mix(lams[vertices_front[1]], lams[vertices_behind[1]], a);

        vx = values[vertices_front[1]];
        vy = values[vertices_behind[0]];
        a = vx/(vx-vy);
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
        pos[0] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[1]], a);
        lam[0] =  mix(lams[vertices_front[0]], lams[vertices_behind[1]], a);
        normals[0] =  mix(normals_ori[vertices_front[0]], normals_ori[vertices_behind[1]], a);

        vx = values[vertices_front[0]];
        vy = values[vertices_behind[0]];
        a = vx/(vx-vy);
        pos[1] =  mix(tet.pos[vertices_front[0]], tet.pos[vertices_behind[0]], a);
        lam[1] =  mix(lams[vertices_front[0]], lams[vertices_behind[0]], a);
        normals[1] =  mix(normals_ori[vertices_front[0]], normals_ori[vertices_behind[0]], a);

        vx = values[vertices_front[1]];
        vy = values[vertices_behind[1]];
        a = vx/(vx-vy);
        pos[2] =  mix(tet.pos[vertices_front[1]], tet.pos[vertices_behind[1]], a);
        lam[2] =  mix(lams[vertices_front[1]], lams[vertices_behind[1]], a);
        normals[2] =  mix(normals_ori[vertices_front[1]], normals_ori[vertices_behind[1]], a);

        vx = values[vertices_front[1]];
        vy = values[vertices_behind[0]];
        a = vx/(vx-vy);
        pos[3] =  mix(tet.pos[vertices_front[1]], tet.pos[vertices_behind[0]], a);
        lam[3] =  mix(lams[vertices_front[1]], lams[vertices_behind[0]], a);
        normals[3] =  mix(normals_ori[vertices_front[1]], normals_ori[vertices_behind[0]], a);
        return 4;
    }           
}
