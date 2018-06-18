#version 150 

{include utils.inc}
#line 4

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;
uniform float shrink_elements;

#define ELEMENT_TYPE {ELEMENT_TYPE}
#define {ELEMENT_TYPE_NAME}
#define ELEMENT_SIZE {ELEMENT_SIZE}

layout(triangles) in;
#ifdef ET_SEGM
layout(line_strip, max_vertices=12) out;
#else
layout(triangle_strip, max_vertices=12) out;
#endif

const int dim = {DIM};
// const int et = {ELEMENT_TYPE};
const int order = {ORDER};
const bool curved = {CURVED};

in VertexData
{
  flat int element;
// #ifdef CURVED
  vec3 lam;
// #endif // CURVED
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

void AddPoint(vec3 p ) {
    outData.pos = p;
    gl_Position = P * MV * vec4(p, 1);
    EmitVertex();
}

ELEMENT_TYPE getElement(Mesh mesh, int elnr ) {
    ELEMENT_TYPE el;
    int offset = ELEMENT_SIZE*elnr;
    el.index = texelFetch(mesh.elements, offset +1).r;
    for (int i=0; i<{ELEMENT_N_VERTICES}; i++) {
        int v = texelFetch(mesh.elements, offset+i+2).r;
        el.pos[i] = texelFetch(mesh.vertices, v).xyz;
    }
#if defined(ET_TRIG) || defined(ET_QUAD)
    el.normal = cross(el.pos[1]-el.pos[0], el.pos[2]-el.pos[0]);
#endif
    return el;
}

void Draw(SEGM el) {
    outData.edgedist = vec3(0,0,0);
    outData.color = vec4(texelFetch(colors, el.index, 0));
    outData.normal = vec3(0,0,1);
    AddPoint(el.pos[0]);
    AddPoint(el.pos[1]);
    EndPrimitive();
}

void Draw(TRIG el) {
    outData.edgedist = vec3(0,0,0);
    outData.color = vec4(texelFetch(colors, el.index, 0));
    outData.normal = el.normal;
    AddPoint(el.pos[0]);
    AddPoint(el.pos[1]);
    AddPoint(el.pos[2]);
    EndPrimitive();
}

void Draw(QUAD el) {
    outData.normal = el.normal;
    outData.color = vec4(texelFetch(colors, el.index, 0));

    outData.edgedist = vec3(1,0,1);
    AddPoint(el.pos[0]);
    outData.edgedist = vec3(0,0,1);
    AddPoint(el.pos[1]);
    outData.edgedist = vec3(0,1,1);
    AddPoint(el.pos[2]);
    EndPrimitive();

    outData.edgedist = vec3(1,0,1);
    AddPoint(el.pos[0]);
    outData.edgedist = vec3(0,1,1);
    AddPoint(el.pos[2]);
    outData.edgedist = vec3(0,0,1);
    AddPoint(el.pos[3]);
    EndPrimitive();
}


void main() {
    ELEMENT_TYPE element = getElement(mesh, inData[0].element);
    Draw(element);
    /*
#ifdef ET_TRIG
        Element2d el;
        int size = 5;
        int offset = inData[0].element*size;

        el.index = texelFetch(mesh.elements, offset +1).r;
        outData.color = vec4(texelFetch(colors, el.index, 0));
        for (int i=0; i<3; i++) {
            el.vertices[i] = texelFetch(mesh.elements, size*inData[0].element+i+2).r;
            el.pos[i] = texelFetch(mesh.vertices, el.vertices[i]).xyz;
        }
        el.nverts = 3;
        calcNormals(el);
        outData.normal = el.normals[0];
        AddPoint(el.pos[0]);
        AddPoint(el.pos[1]);
        AddPoint(el.pos[2]);
        EndPrimitive();

//         AddTrig(el, 0,1,2, 0);
#endif // ET_TRIG

#ifdef ET_QUAD
        Element2d el;
        int size = 7;
        int offset = inData[0].element*size;

        el.index = texelFetch(mesh.elements, offset +1).r;
        outData.color = vec4(texelFetch(colors, el.index, 0));

        for (int i=0; i<4; i++) {
            el.vertices[i] = texelFetch(mesh.elements, size*inData[0].element+i+2).r;
            el.pos[i] = texelFetch(mesh.vertices, el.vertices[i]).xyz;
        }

        calcNormals(el);
        outData.normal = el.normals[0];

        outData.edgedist = vec3(1,0,1);
        AddPoint(el.pos[0]);
        outData.edgedist = vec3(0,0,1);
        AddPoint(el.pos[1]);
        outData.edgedist = vec3(0,1,1);
        AddPoint(el.pos[2]);
        EndPrimitive();

        outData.edgedist = vec3(1,0,1);
        AddPoint(el.pos[0]);
        outData.edgedist = vec3(0,1,1);
        AddPoint(el.pos[2]);
        outData.edgedist = vec3(0,0,1);
        AddPoint(el.pos[3]);
        EndPrimitive();

//         AddTrig(el, 0,1,2, 0);
//         AddTrig(el, 2,1,0, 1);
#endif // ET_QUAD

*/

    /*
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
    */

}
