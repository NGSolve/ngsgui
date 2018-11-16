#line 40001
vec4 getValue(samplerBuffer coefficients, int offset) {
  return texelFetch(coefficients, offset);
}

struct Function {
  samplerBuffer coefficients;
  samplerBuffer coefficients_imag;
  int subdivision;
  int order;
  int component;

  bool is_complex;
  int complex_vis_function; // 0=real, 1=imag, 2=abs, 3=arg
  vec2 complex_factor; // factor to multiply with values before visualizing
};

#line 30000
{include generated_interpolation.inc}
#line 40020

float Evaluate(in Function f, int element, vec3 lam)
{
  float value = EvaluateElement(element, f.coefficients, ORDER, f.subdivision, lam, f.component);

  if(f.is_complex) {
    float value_imag = EvaluateElement(element, f.coefficients_imag, ORDER, f.subdivision, lam, f.component);
    float r = value*f.complex_factor.x - value_imag*f.complex_factor.y;
    value_imag = value*f.complex_factor.y + value_imag*f.complex_factor.x;
    value = r;
    switch(f.complex_vis_function){
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

vec3 EvaluateVec(in Function f, int element, vec3 lam)
{
  vec3 value = EvaluateElementVec(element, f.coefficients, ORDER, f.subdivision, lam, 0);

  if(f.is_complex) {
    vec3 value_imag = EvaluateElementVec(element, f.coefficients_imag, ORDER, f.subdivision, lam, 0);
    vec3 r = value*f.complex_factor.x - value_imag*f.complex_factor.y;
    value_imag = value*f.complex_factor.y + value_imag*f.complex_factor.x;
    value = r;
    switch(f.complex_vis_function){
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
