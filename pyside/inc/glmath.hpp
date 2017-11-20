#pragma once
#include <bla.hpp>

namespace ngbla { 
    typedef Mat<4,4,float> Mat4;
    typedef Vec<4,float> Vec4;
    typedef Vec<3,float> Vec3;

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

    inline Mat4 LookAt(Vec3 eye, Vec3 center, Vec3 up)
    {
        Vec3 f = center - eye;
        f /= L2Norm(f);

        Vec3 s = Cross(f,up);
        s /= L2Norm(s);

        Vec3 t = Cross(s,f);

        Mat4 m = Identity();
        m.Col(0).Range(0,3) = s;
        m.Col(1).Range(0,3) = t;
        m.Col(2).Range(0,3) = -f;

        Vec4 tmp = {-eye[0], -eye[1], -eye[2], 0};
        m.Row(3) += m*tmp;

        return m;
    }

}
