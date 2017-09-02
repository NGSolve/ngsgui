#include<string>
#include<iostream>
using std::string;

void _check_gl_error(const char *file, int line);

///
/// Usage
/// [... some opengl calls]
/// glCheckError();
///
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
// void check_gl_error() { _check_gl_error("lskdfls", 234); }

// #ifdef WIN32
// #  include <GL/glew.h>
// #elif __APPLE__
// #  include <OpenGL/gl3.h>
// #else
// #  include <GL3/gl3.h>
// #endif


void _check_gl_error(const char *file, int line) {
    GLenum err (glGetError());

    while(err!=GL_NO_ERROR) {
        string error;

        switch(err) {
          case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
          case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
          case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
          case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
          case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }

        std::cerr << "GL_" << error.c_str() <<" - "<<file<<":"<<line<<std::endl;
        err=glGetError();
    }
}

void _get_int_val(const char *s, GLenum val) {
    GLint v;
    glGetIntegerv(val, &v);
    std::cout << s << ": " << v << std::endl;
//     glGetIntegerv with GL_MAX_VERTEX_UNIFORM_BLOCKS, GL_MAX_GEOMETRY_UNIFORM_BLOCKS, or GL_MAX_FRAGMENT_UNIFORM_BLOCKS.

}

#define GET_INT_VAL(VAL) _get_int_val(#VAL, VAL);
