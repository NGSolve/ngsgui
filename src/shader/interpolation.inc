vec4 getValue(samplerBuffer coefficients, int offset) {
  return texelFetch(coefficients, offset);
}

{include generated_interpolation.inc}
