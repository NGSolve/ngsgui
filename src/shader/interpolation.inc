#line 40001
#ifndef INTERPOLATION_INC
#define INTERPOLATION_INC

vec4 getValue(samplerBuffer coefficients, int offset) {
  return texelFetch(coefficients, offset);
}

uniform struct Function {
  samplerBuffer coefficients;
  samplerBuffer coefficients_imag;
  int subdivision;
  int order;
  int component;

  bool is_complex;
  int complex_vis_function; // 0=real, 1=imag, 2=abs, 3=arg
  vec2 complex_factor; // factor to multiply with values before visualizing
} functions[3];

#define FUNCTION 0
#define DEFORMATION_FUNCTION 1
#define ISO_FUNCTION 2

#line 30000
{include generated_interpolation.inc}
#line 40020

float Evaluate(int f, int element, vec3 lam)
{
  float value = EvaluateElement(element, functions[f].coefficients, ORDER, functions[f].subdivision, lam, functions[f].component);

  if(functions[f].is_complex) {
    float value_imag = EvaluateElement(element, functions[f].coefficients_imag, ORDER, functions[f].subdivision, lam, functions[f].component);
    float r = value*functions[f].complex_factor.x - value_imag*functions[f].complex_factor.y;
    value_imag = value*functions[f].complex_factor.y + value_imag*functions[f].complex_factor.x;
    value = r;
    switch(functions[f].complex_vis_function){
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
  }
  return value;
}

vec3 EvaluateVec(int f, int element, vec3 lam)
{
  vec3 value = EvaluateElementVec(element, functions[f].coefficients, ORDER, functions[f].subdivision, lam, 0);

  if(functions[f].is_complex) {
    vec3 value_imag = EvaluateElementVec(element, functions[f].coefficients_imag, ORDER, functions[f].subdivision, lam, 0);
    vec3 r = value*functions[f].complex_factor.x - value_imag*functions[f].complex_factor.y;
    value_imag = value*functions[f].complex_factor.y + value_imag*functions[f].complex_factor.x;
    value = r;
    switch(functions[f].complex_vis_function){
      case 0:
      break;
      case 1:
      value = value_imag;
      break;
      case 2:
      value = sqrt(value*value+value_imag*value_imag);
      break;
      case 3:
      value = vec3(0.0);
      break;
    }
  }
  return value;
}
#endif // INTERPOLATION_INC
