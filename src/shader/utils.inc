vec3 MapColor(float value)
{
    value = clamp(value, 0.0, 1.0);
    vec3 res;
    res.r = clamp(2.0-4.0*value, 0.0, 1.0);
    res.g = clamp(2.0-4.0*abs(0.5-value), 0.0, 1.0);
    res.b = clamp(4.0*value - 2.0, 0.0, 1.0);
    return res;
}

// find containing subtriangle
void getSubTrigStart(int order, int subdivision, inout vec3 lam, inout int x, inout int y, inout int dy)
{
    int n = subdivision+1;
    vec3 lamn = lam*(n);
    vec3 floor_lamn = floor(lamn);
    lam = lamn-floor_lamn;
    x=int(lamn.x);
    y=int(lamn.y);
    int z=int(lamn.z);
    int missing = n-x-y-z;

    if(missing==1) {
        dy=1;
    }
    if(missing==2) {
        dy=-1;
        x += 1;
        y += 1;
        lam.x = 1-lam.x;
        lam.y = 1-lam.y;
    }
}

float getSubTrigValue(int element, samplerBuffer coefficients, int order, int subdivision, int xtrig, int ytrig, int dy, int x, int y) {
    int t = (subdivision+1); // number of small edges in large edge
    int n = (order+1)*t; // points on edge of subtrigs
    int N = order*(subdivision+1)+1; // points on edge of trig
    int m = N-ytrig*order;
    int values_per_element = N*(N+1)/2;
    int offset=0;
    if(dy==1) {
        offset  = values_per_element - m*(m+1)/2 + xtrig*order;
        offset += m*(m+1)/2-(m-y)*(m-y+1)/2 + x;
    }
    else {
        offset  = values_per_element - m*(m+1)/2 + xtrig*order;
        offset -= (m+y)*(m+y+1)/2 - m*(m+1)/2 + x;
    }
    return texelFetch(coefficients, element*values_per_element+offset).r;
}

float InterpolateTrig(int element, samplerBuffer coefficients, int order, int subdivision, vec3 lam) {
  int trigx=0;
  int trigy=0;
  int dy=1;
  getSubTrigStart(order, subdivision, lam, trigx, trigy, dy);
  float x = lam.x;
  float y = lam.y;
  float value = 0.0;
  if (order==1) {
    float f[3];
    f[0] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1);
    value = 1.0*f[0] - 1.0*x*(f[0] - f[1]) - 1.0*y*(f[0] - f[2]);
  }
  if (order==2) {
    float f[6];
    f[0] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0);
    f[3] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1);
    f[4] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1);
    f[5] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2);
    value = 2.33333333333333*f[0] - 1.33333333333333*f[1] + 0.666666666666667*f[2] - 1.33333333333333*f[3] + 0.666666666666667*f[5] + 4.0*x*y*(f[0] - f[1] - f[3] + f[4]) - 1.0*x*(3.0*f[0] - 4.0*f[1] + 1.0*f[2]) - 1.0*y*(3.0*f[0] - 4.0*f[3] + 1.0*f[5]) + (1.5*pow(x, 2) - 0.5)*(1.33333333333333*f[0] - 2.66666666666667*f[1] + 1.33333333333333*f[2]) + (1.5*pow(y, 2) - 0.5)*(1.33333333333333*f[0] - 2.66666666666667*f[3] + 1.33333333333333*f[5]);
  }
  if (order==3) {
    float f[10];
    f[0] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,0);
    f[1] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,0);
    f[2] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,0);
    f[3] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 3,0);
    f[4] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,1);
    f[5] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,1);
    f[6] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 2,1);
    f[7] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,2);
    f[8] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 1,2);
    f[9] = getSubTrigValue(element, coefficients, order, subdivision, trigx, trigy, dy, 0,3);
    value = 7.0*f[0] - 7.5*f[1] + 6.0*f[2] - 1.5*f[3] - 7.5*f[4] + 6.0*f[7] - 1.5*f[9] + 1.0*x*y*(18.0*f[0] - 22.5*f[1] + 4.5*f[2] - 22.5*f[4] + 27.0*f[5] - 4.5*f[6] + 4.5*f[7] - 4.5*f[8]) - x*(2.5*pow(x, 2) - 1.5)*(1.8*f[0] - 5.4*f[1] + 5.4*f[2] - 1.8*f[3]) - 1.0*x*(1.5*pow(y, 2) - 0.5)*(9.0*f[0] - 9.0*f[1] - 18.0*f[4] + 18.0*f[5] + 9.0*f[7] - 9.0*f[8]) - 1.0*x*(12.7*f[0] - 21.6*f[1] + 12.6*f[2] - 3.7*f[3] - 9.0*f[4] + 9.0*f[5] + 4.5*f[7] - 4.5*f[8]) - 1.0*y*(1.5*pow(x, 2) - 0.5)*(9.0*f[0] - 18.0*f[1] + 9.0*f[2] - 9.0*f[4] + 18.0*f[5] - 9.0*f[6]) - y*(2.5*pow(y, 2) - 1.5)*(1.8*f[0] - 5.4*f[4] + 5.4*f[7] - 1.8*f[9]) - 1.0*y*(12.7*f[0] - 9.0*f[1] + 4.5*f[2] - 21.6*f[4] + 9.0*f[5] - 4.5*f[6] + 12.6*f[7] - 3.7*f[9]) + (1.5*pow(x, 2) - 0.5)*(6.0*f[0] - 15.0*f[1] + 12.0*f[2] - 3.0*f[3]) + (1.5*pow(y, 2) - 0.5)*(6.0*f[0] - 15.0*f[4] + 12.0*f[7] - 3.0*f[9]);
  }
  return value;
}

