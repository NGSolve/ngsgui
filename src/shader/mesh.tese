#version 410 core

#if defined(ET_SEGM)
layout(isolines) in;
#elif defined(ET_TRIG)
layout(triangles) in;
#elif defined(ET_QUAD)
layout(quads) in;
#endif

{include utils.inc}
#line 12

uniform mat4 P;
uniform mat4 MV;
uniform Mesh mesh;
uniform sampler1D colors;

#if DEFORMATION
#undef ORDER
#define ORDER DEFORMATION_ORDER
#line 0
{include interpolation.inc}
#line 23
uniform int subdivision;
uniform int component;
uniform samplerBuffer deformation_coefficients;
uniform int deformation_subdivision;
uniform float deformation_scale;
// for complex-valued functions
uniform bool is_complex;
uniform samplerBuffer coefficients;
uniform samplerBuffer coefficients_imag;
uniform int complex_vis_function; // 0=real, 1=imag, 2=abs, 3=arg
uniform vec2 complex_factor; // factor to multiply with values before visualizing
#endif // DEFORMATION

in VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} inData[];

out VertexData
{
  vec3 lam;
  vec3 pos;
  vec3 normal;
  flat int element;
} outData;

void main()
{
    outData.element = inData[0].element;

    float x = gl_TessCoord.x;
    float y = gl_TessCoord.y;
    float z = 1.0-x-y;

    int offset = texelFetch(mesh.elements, ELEMENT_SIZE*inData[0].element + ELEMENT_SIZE-1).r;

#if defined(CURVED)
#if defined(ET_SEGM)
    vec3 a = inData[0].pos;
    vec3 b = texelFetch(mesh.vertices, offset+2).xyz;
    vec3 c = inData[1].pos;
    outData.pos = a + x*(-c-3*a+4*b) + x*x*2*(a-2*b+c);
    outData.normal = mix(inData[0].normal, inData[1].normal, x);
    outData.lam = vec3(x,0,0);
#elif defined(ET_TRIG)
    vec3 f[6];
    f[0] = inData[2].pos;
    f[2] = inData[0].pos;
    f[5] = inData[1].pos;
    f[1] = texelFetch(mesh.vertices, offset+3).xyz;
    f[3] = texelFetch(mesh.vertices, offset+4).xyz;
    f[4] = texelFetch(mesh.vertices, offset+5).xyz;
    outData.pos = 1.0*f[0] + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[5]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]);
    outData.normal = x*inData[0].normal + y*inData[1].normal + z*inData[2].normal;
    outData.lam = vec3(x,y,z);
#elif defined(ET_QUAD)
    vec3 f[9];
    f[0] = inData[0].pos;
    f[2] = inData[1].pos;
    f[8] = inData[2].pos;
    f[6] = inData[3].pos;
    f[1] = texelFetch(mesh.vertices, offset+4).xyz;
    f[3] = texelFetch(mesh.vertices, offset+5).xyz;
    f[4] = texelFetch(mesh.vertices, offset+6).xyz;
    f[5] = texelFetch(mesh.vertices, offset+7).xyz;
    f[7] = texelFetch(mesh.vertices, offset+8).xyz;
    outData.pos = 1.0*f[0] + x*x*y*y*(4.0*f[0] - 8.0*f[1] + 4.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 4.0*f[6] - 8.0*f[7] + 4.0*f[8]) - x*x*y*(6.0*f[0] - 12.0*f[1] + 6.0*f[2] - 8.0*f[3] + 16.0*f[4] - 8.0*f[5] + 2.0*f[6] - 4.0*f[7] + 2.0*f[8]) + x*x*(2.0*f[0] - 4.0*f[1] + 2.0*f[2]) - x*y*y*(6.0*f[0] - 8.0*f[1] + 2.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 6.0*f[6] - 8.0*f[7] + 2.0*f[8]) + x*y*(9.0*f[0] - 12.0*f[1] + 3.0*f[2] - 12.0*f[3] + 16.0*f[4] - 4.0*f[5] + 3.0*f[6] - 4.0*f[7] + 1.0*f[8]) - x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) + y*y*(2.0*f[0] - 4.0*f[3] + 2.0*f[6]) - y*(3.0*f[0] - 4.0*f[3] + 1.0*f[6]);
    vec3 n1 = mix(inData[0].normal, inData[1].normal, x);
    vec3 n2 = mix(inData[3].normal, inData[2].normal, x);
    outData.normal = mix(n1,n2, y);
    outData.lam = vec3(y,x,0);
#else
    unknown type
#endif
#else // CURVED
    outData.normal = inData[0].normal;
    outData.lam = vec3(x,y,z);
#if defined(ET_SEGM)
    outData.pos = mix(inData[0].pos, inData[1].pos, x);
#elif defined(ET_TRIG)
    outData.pos = x*inData[0].pos+y*inData[1].pos+z*inData[2].pos;
#elif defined(ET_QUAD)
    vec3 p0 = mix(inData[0].pos, inData[1].pos, x);
    vec3 p1 = mix(inData[3].pos, inData[2].pos, x);
    outData.normal = mix(p0,p1, y);
#else
    unknown type
#endif
#endif // CURVED

#if DEFORMATION
      if(is_complex) {
        float value, value_imag;
        value = EvaluateElement(inData[0].element, coefficients, ORDER, subdivision, outData.lam, component);
        value_imag = EvaluateElement(inData[0].element, coefficients_imag, ORDER, subdivision, outData.lam, component);
        float r = value*complex_factor.x - value_imag*complex_factor.y;
        value_imag = value*complex_factor.y + value_imag*complex_factor.x;
        value = r;
        switch(complex_vis_function){
          case 0:
            break;
          case 1:
            value = value_imag;
            break;
          case 2:
            value = length(vec2(value, value_imag));
            break;
          case 3:
            value = atan(value, value_imag);
            break;
        }
        outData.pos.z += deformation_scale*value;
      }
      else {
        vec3 value = vec3(deformation_scale,deformation_scale,deformation_scale);
        value *= EvaluateElementVec(inData[0].element, deformation_coefficients, ORDER, deformation_subdivision, outData.lam, 0);
        outData.pos += value;
      }
#endif // DEFORMATION

    gl_Position = P * MV * vec4(outData.pos, 1);
}
