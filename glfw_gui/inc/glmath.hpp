#pragma once
#include <bla.hpp>

namespace ngbla { 
  typedef Mat<4,4,float> Mat4;

  inline Mat4 Identity()
  {
    Mat4 m;
    m = 0.0f;
    m(0,0) = 1.0f;
    m(1,1) = 1.0f;
    m(2,2) = 1.0f;
    m(3,3) = 1.0f;
    return m;
  }

  inline Mat4 RotateX(float angle)
  {
    float s = sinf(angle);
    float c = cosf(angle);
    Mat4 res = Identity();
    res(1,1) = c;
    res(2,2) = c;
    res(1,2) = s;
    res(2,1) =-s;
    return res;
  }

  inline Mat4 RotateY(float angle)
  {
    float s = sinf(angle);
    float c = cosf(angle);
    Mat4 res = Identity();
    res(0,0) = c;
    res(2,2) = c;
    res(0,2) = s;
    res(2,0) =-s;
    return Trans(res);
  }

  inline Mat4 RotateZ(float angle)
  {
    float s = sinf(angle);
    float c = cosf(angle);
    Mat4 res = Identity();
    res(0,0) = c;
    res(1,1) = c;
    res(0,1) = s;
    res(1,0) =-s;
    return Trans(res);
  }

  inline Mat4 Ortho(float l, float r, float b, float t, float n, float f)
  {
    Mat4 m;
    m = 0.0f;
    m(0,0) = 2.f/(r-l);
    m(1,1) = 2.f/(t-b);
    m(2,2) = -2.f/(f-n);
    m(3,0) = -(r+l)/(r-l);
    m(3,1) = -(t+b)/(t-b);
    m(3,2) = -(f+n)/(f-n);
    m(3,3) = 1.f;
    return m;
  }

  inline Mat4 Perspective(float y_fov, float aspect, float n, float f)
  {
    float const a = 1.f / (float) tan(y_fov / 2.f);

    Mat4 m;
    m = 0.0f;
    m(0,0) = a / aspect;
    m(1,1) = a;
    m(2,2) = -((f + n) / (f - n));
    m(2,3) = -1.f;
    m(3,2) = -((2.f * f * n) / (f - n));
    return Trans(m);
  }

  inline Mat4 Translate(float x, float y, float z=0.f)
  {
      Mat4 res = Identity();
      res(3,0) = x;
      res(3,1) = y;
      res(3,2) = z;
      return Trans(res);
  }

  inline Mat4 Scale(float s)
  {
      Mat4 res = Identity();
      res(0,0) = s;
      res(1,1) = s;
      res(2,2) = s;
      return res;
  }
// inline Mat4 LookAt(Vec<3,double> eye, Vec<3,double> center, Vec<3,double> up)
// {
// 	vec3 f;
// 	vec3 s;
// 	vec3 t;
// 
// 	vec3_sub(f, center, eye);
// 	vec3_norm(f, f);
// 
// 	vec3_mul_cross(s, f, up);
// 	vec3_norm(s, s);
// 
// 	vec3_mul_cross(t, s, f);
// 
// 	m[0][0] =  s[0];
// 	m[0][1] =  t[0];
// 	m[0][2] = -f[0];
// 	m[0][3] =   0.f;
// 
// 	m[1][0] =  s[1];
// 	m[1][1] =  t[1];
// 	m[1][2] = -f[1];
// 	m[1][3] =   0.f;
// 
// 	m[2][0] =  s[2];
// 	m[2][1] =  t[2];
// 	m[2][2] = -f[2];
// 	m[2][3] =   0.f;
// 
// 	m[3][0] =  0.f;
// 	m[3][1] =  0.f;
// 	m[3][2] =  0.f;
// 	m[3][3] =  1.f;
// 
// 	mat4x4_translate_in_place(m, -eye[0], -eye[1], -eye[2]);
// }
}
