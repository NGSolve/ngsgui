#line 10001
struct Colormap {
  float min;
  float max;
  sampler1D colors;
  int n;
  bool linear;
};

struct Light {
  vec3 dir;
  float ambient;
  float diffuse;
  float spec;
  float shininess;
};

#if !defined(NO_CLIPPING) && defined(CLIPPING)
struct ClippingPlanes
{
  vec4 p[N_CLIPPING_PLANES];
};

bool CalcClipping(ClippingPlanes cp, vec3 pos)
{
  bool p[N_CLIPPING_PLANES];
  vec4 pos4 = vec4(pos, 1.0);
  for (int i=0; i<N_CLIPPING_PLANES; i++)
      p[i] = dot(cp.p[i], pos4) < 0;
  return CLIPPING_EXPRESSION;
}
#else // clipping disabled
struct ClippingPlanes {bool dummy_var;};
bool CalcClipping(ClippingPlanes cp, vec3 pos) { return true; }
#endif

struct Mesh {
    int dim;
    isamplerBuffer elements;
    samplerBuffer vertices;
    int surface_curved_offset;
    int volume_elements_offset;
    int surface_elements_offset;
};

int elements1d_size = 4;
int elements1d_index_offset = 2;
int elements1d_curved_offset = 3;

int elements2d_size = 5;
int elements2d_index_offset = 3;
int elements2d_curved_offset = 4;

int elements3d_size = 6;
int elements3d_index_offset = 4;

int elements2d_curved_size = 6;
int elements2d_curved_normals_offset = 0;
int elements2d_curved_points_offset = 3;

struct Element1d {
    int index;
    int curved_index;
    int vertices[2];
    vec3 pos[3];
    vec3 normals[2];
};

struct Element2d {
    int index;
    int nverts;
    int curved_index;
    int vertices[4];
    vec3 pos[4];
    vec3 normals[4];
};

struct Element3d {
    int index;
    int nverts;
    int curved_index;
    int vertices[4];
    vec3 pos[4];
};

vec3 interpolatePoint(Mesh mesh, Element1d el, float x0) {
  float x = 1-x0;
  if(el.curved_index<0) {
    return x*el.pos[0] + (1-x)*el.pos[1];
  }
  else {
    return el.pos[0] + x*(-el.pos[1]-3*el.pos[0]+4*el.pos[2]) + x*x*2*(el.pos[0]-2*el.pos[2]+el.pos[1]);
  }
}

vec3 interpolatePoint(Mesh mesh, Element2d el, vec2 lam) {
    float x = lam.x;
    float y = lam.y;
    if(el.curved_index<0) {
      return x*el.pos[0] + y*el.pos[1]+(1-x-y)*el.pos[2];
    }
    else {
      if(el.nverts == 3) {
        vec3 f[6];
        f[0] = el.pos[2];
        f[2] = el.pos[0];
        f[5] = el.pos[1];
        int curved_offset = texelFetch(mesh.elements, el.curved_index+1).r;
        int offset = curved_offset + elements2d_curved_points_offset;
        f[1] = texelFetch(mesh.vertices, offset+0).xyz;
        f[3] = texelFetch(mesh.vertices, offset+1).xyz;
        f[4] = texelFetch(mesh.vertices, offset+2).xyz;
        return 1.0*f[0] + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]);
      }
      if(el.nverts == 4) {
        vec3 f[9];
        f[0] = el.pos[0];
        f[2] = el.pos[1];
        f[8] = el.pos[2];
        f[6] = el.pos[3];
        int curved_offset = texelFetch(mesh.elements, el.curved_index+1).r;
        int offset = curved_offset + el.nverts+1;
        f[1] = texelFetch(mesh.vertices, offset+0).xyz;
        f[3] = texelFetch(mesh.vertices, offset+1).xyz;
        f[4] = texelFetch(mesh.vertices, offset+2).xyz;
        f[5] = texelFetch(mesh.vertices, offset+3).xyz;
        f[7] = texelFetch(mesh.vertices, offset+4).xyz;
        return 1.0*f[0] + x*x*y*y*(4.0*f[0] - 8.0*f[1] + 4.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 4.0*f[6] - 8.0*f[7] + 4.0*f[8]) - x*x*y*(6.0*f[0] - 12.0*f[1] + 6.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 2.0*f[6] - 4.0*f[7] + 2.0*f[8]) + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) - x*y*y*(6.0*f[0] - 8.0*f[1] + 2.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 6.0*f[6] - 8.0*f[7] + 2.0*f[8]) + x*y*(9.0*f[0] - 12.0*f[1] + 3.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 3.0*f[6] - 4.0*f[7] + 1.0*f[8]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[6]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[6]);

    }
  }
}

vec3 interpolatePoint(Mesh mesh, Element3d tet, Element2d trig, int face, vec2 lam) {
    float x = lam.x;
    float y = lam.y;
    if(tet.curved_index<0) {
      return x*trig.pos[0] + y*trig.pos[1]+(1-x-y)*trig.pos[2];
    }
    vec3 f[6];
    f[0] = trig.pos[2];
    f[2] = trig.pos[0];
    f[5] = trig.pos[1];
    int curved_offset = texelFetch(mesh.elements, tet.curved_index+1).r;
    int offset = curved_offset + 0;
    // fall-back code to ignore curving
    // f[1] = 0.5*(trig.pos[2]+trig.pos[0]);
    // f[3] = 0.5*(trig.pos[1]+trig.pos[2]);
    // f[4] = 0.5*(trig.pos[0]+trig.pos[1]);

    if(face==0) {
      f[1] = texelFetch(mesh.vertices, offset+1).xyz;
      f[3] = texelFetch(mesh.vertices, offset+5).xyz;
      f[4] = texelFetch(mesh.vertices, offset+4).xyz;
    }
    if(face==1) {
      f[1] = texelFetch(mesh.vertices, offset+0).xyz;
      f[3] = texelFetch(mesh.vertices, offset+5).xyz;
      f[4] = texelFetch(mesh.vertices, offset+3).xyz;
    }
    if(face==2) {
      f[1] = texelFetch(mesh.vertices, offset+0).xyz;
      f[3] = texelFetch(mesh.vertices, offset+1).xyz;
      f[4] = texelFetch(mesh.vertices, offset+2).xyz;
    }
    if(face==3) {
      f[1] = texelFetch(mesh.vertices, offset+3).xyz;
      f[3] = texelFetch(mesh.vertices, offset+4).xyz;
      f[4] = texelFetch(mesh.vertices, offset+2).xyz;
    }
    return 1.0*f[0] + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]);
}

void calcNormals(inout Element2d el) {
  // flat element, normal is constant
  vec3 n = cross(el.pos[1]-el.pos[0], el.pos[2]-el.pos[0]);
  el.normals[0] = n;
  el.normals[1] = n;
  el.normals[2] = n;
}

Element2d getElement2d(Mesh mesh, Element3d tet, int face ) {
  Element2d trig;
  trig.nverts = 3;
  trig.index = tet.index;
  trig.curved_index = -1;
  int counter = 0;
  for (int i=0; i<4; i++) {
    if(i==face) continue;
    trig.vertices[counter] = tet.vertices[i];
    trig.pos[counter] = tet.pos[i];
    counter++;
  }
  calcNormals(trig);
  vec3 center = 0.25*(tet.pos[0]+tet.pos[1]+tet.pos[2]+tet.pos[3]);
  vec3 center_trig = 0.3333*(trig.pos[0]+trig.pos[1]+trig.pos[2]);
  if(dot(center_trig-center, trig.normals[0])<0) {
    trig.normals[0] = -trig.normals[0];
    trig.normals[1] = -trig.normals[1];
    trig.normals[2] = -trig.normals[2];
  }
//   }
//   else {
//     int counter = 0;
//     int curved_offset = texelFetch(mesh.elements, tet.curved_index+1).r;
//     for (int i=0; i<4; i++) {
//       if(i==face) continue;
//       trig.normals[counter] = -texelFetch(mesh.vertices, curved_offset + i).xyz;
//       counter++;
//     }
// 
//   }

  return trig;
}

Element1d getElement1d(Mesh mesh, int ei ) {
    Element1d el;
    int stride = elements1d_size;

    for (int i=0; i<2; i++) {
      el.vertices[i] = texelFetch(mesh.elements, stride*ei+i).r;
      el.pos[i] = texelFetch(mesh.vertices, el.vertices[i]).xyz;
    }
    el.index = texelFetch(mesh.elements, stride*ei+elements1d_index_offset).r;
    el.curved_index = texelFetch(mesh.elements, stride*ei+elements1d_curved_offset).r;
    int curved_offset = texelFetch(mesh.elements, el.curved_index).r;
    if(el.curved_index>=0) {
      el.normals[0] = texelFetch(mesh.vertices, curved_offset+0).xyz;
      el.normals[1] = texelFetch(mesh.vertices, curved_offset+1).xyz;
      el.pos[2] = texelFetch(mesh.vertices, curved_offset+2).xyz;
    }
    else {
      el.normals[0] = el.pos[0];
      el.normals[1] = el.pos[1];
    }
    return el;
}

Element2d getElement2d(Mesh mesh, int ei ) {
    Element2d el;
    int offset = mesh.surface_elements_offset;
    int stride = elements2d_size;

    for (int i=0; i<3; i++) {
      el.vertices[i] = texelFetch(mesh.elements, offset + stride*ei+i).r;
      el.pos[i] = texelFetch(mesh.vertices, el.vertices[i]).xyz;
    }
    el.index = texelFetch(mesh.elements, offset + stride*ei+elements2d_index_offset).r;
    el.curved_index = texelFetch(mesh.elements, offset + stride*ei+elements2d_curved_offset).r;
    if(el.curved_index>=0) {
        // have curved element
        el.nverts = texelFetch(mesh.elements, el.curved_index).r;
        int curved_offset = texelFetch(mesh.elements, el.curved_index+1).r;
        for (int i=0; i<el.nverts; i++)
          el.normals[i] = normalize(texelFetch(mesh.vertices, curved_offset + i).xyz);

        for (int i=3; i<el.nverts; i++) {
          el.vertices[i] = texelFetch(mesh.elements, el.curved_index+2-3+i).r;
          el.pos[i] = texelFetch(mesh.vertices, el.vertices[i]).xyz;
        }
    }
    else {
      el.nverts = 3;
      calcNormals(el);
    }

    return el;
}

Element3d getElement3d(Mesh mesh, int ei ) {
    Element3d el;
    int offset = mesh.volume_elements_offset + elements3d_size*ei;

    for (int i=0; i<4; i++) {
      el.vertices[i] = texelFetch(mesh.elements, offset+i).r;
      el.pos[i] = texelFetch(mesh.vertices, el.vertices[i]).xyz;
    }
    el.index = texelFetch(mesh.elements, offset+elements3d_index_offset).r;
    el.curved_index = texelFetch(mesh.elements, offset+elements3d_index_offset+1).r;
    return el;
}


float CutEdge(vec4 plane, vec3 x, vec3 y) {
      float dx = dot(plane, vec4(x,1.0));
      float dy = dot(plane, vec4(y,1.0));
      return dx/(dx-dy);
}

// Cut tet with plane and store 0-4 points (and barycentric coords), return the number of intersection points
int CutElement3d( Element3d tet, float values[4], out vec3 pos[4], inout vec3 lam[4], inout vec3 normals[4] ) {
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

// Cut tet with plane and store 0-4 points (and barycentric coords), return the number of intersection points
int CutElement3d( Element3d tet, float values[4], out vec3 pos[4], inout vec3 lam[4] ) {
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

vec3 TransformVec(mat4 MV, vec3 x) {
    return normalize(transpose(mat3(MV))*x);
}

vec3 CalcLight(Light light, vec3 color, mat4 MV, vec3 position, vec3 norm)
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

vec3 MapColor(Colormap cm, float value)
{
  value = (value-cm.min)/(cm.max-cm.min);
  value = clamp(value, 0.0, 1.0-1e-7);
  if(cm.linear)
      return texture(cm.colors, value).rgb;

  int n = int(cm.n*value);
  return texelFetch(cm.colors, n, 0).rgb;
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

