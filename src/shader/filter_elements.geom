#version 150 

layout(points) in;
layout(points, max_vertices=1) out;

{include utils.inc}

uniform Mesh mesh;
uniform vec4 clipping_plane;

uniform samplerBuffer coefficients;
uniform float colormap_max;
uniform int subdivision;
uniform int order;
uniform int component;
uniform int filter_type; // 0...clipping plane, 1...iso-surface

in VertexData
{
  flat int element;
} inData[];

flat out int element;

bool isCuttingClippingPlane() {
    float max_dist;
    float min_dist;

    Element3d el = getElement3d(mesh, inData[0].element);
    max_dist = dot(clipping_plane, vec4(el.pos[0], 1.0));
    min_dist = max_dist;
    for (int i=1; i<4; i++) {
        float dist = dot(clipping_plane, vec4(el.pos[i], 1.0));
        min_dist = min(dist, min_dist);
        max_dist = max(dist, max_dist);
    }

    return min_dist<=0 && max_dist>0;
}

bool isCuttingIsoSurface() {
    float min_value;
    float max_value;

    int n = subdivision+1;
    int N = order*n+1;
    int values_per_element = N*(N+1)*(N+2)/6;
    int first = inData[0].element*values_per_element;
    float value = texelFetch(coefficients, first)[component];
    min_value = value;
    max_value = value;
    for (int i=1; i<values_per_element; i++) {
        float value = texelFetch(coefficients, first+i)[component];
        min_value = min(min_value, value);
        max_value = max(max_value, value);
    }
    min_value -= colormap_max;
    max_value -= colormap_max;

    return min_value<=0 && max_value>0;
}

void main() {
    bool emit = false;
    if(filter_type==0)
      emit = isCuttingClippingPlane();
    if(filter_type==1)
      emit = isCuttingIsoSurface();

    if(emit) {
      element = inData[0].element;
      EmitVertex();
      EndPrimitive();
    }
}
