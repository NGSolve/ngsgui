#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <glmath.hpp>
#include <comp.hpp>
#include <glerror.hpp>

using std::string;



struct Shader {
    string code;
    GLuint type;
    GLuint id;

    Shader(string code_, GLuint type_) 
      : code(code_),
      type(type_)
    {
        id = glCreateShader(type);
        auto p = code.c_str();
        glShaderSource(id, 1, &p, NULL);
        glCompileShader(id);
        std::ofstream file(string("shader.") + ((type==GL_VERTEX_SHADER) ? "vert" : "frag"));
        file << p << std::endl;

        GLint shader_ok;
        glGetShaderiv(id, GL_COMPILE_STATUS, &shader_ok);
        if (shader_ok != GL_TRUE)
        {
            GLsizei log_length;
            char info_log[8192];
            std::cerr << "ERROR: Failed to compile ";
            if(type==GL_VERTEX_SHADER)
                std::cerr << "vertex ";
            if(type==GL_FRAGMENT_SHADER)
                std::cerr << "fragment ";
            std::cerr << "shader " << std::endl;
            glGetShaderInfoLog(id, 8192, &log_length,info_log);
            std::cerr << "ERROR: " << info_log << std::endl;;
        }
    }
};

struct Program {
    std::vector<Shader> shaders;
    GLuint id;


    Program ()  {}

    Program (std::initializer_list<Shader> list) 
      : shaders(list)
    {
        Compile();
    }

    void Compile() {
        id = glCreateProgram();
        for( auto &shader : shaders)
            glAttachShader(id, shader.id);
        glLinkProgram(id);

        GLint result;
        glGetProgramiv(id, GL_LINK_STATUS,&result);
        if(result == GL_TRUE){
        } else {
            int maxLength;
            int length;
            glGetProgramiv(id,GL_INFO_LOG_LENGTH,&maxLength);
            char log[8192];
            glGetProgramInfoLog(id,8192,&length,log);
            std::cerr << "ERROR: Failed to link" << endl;
            std::cerr << "ERROR: " << log << std::endl;;
        }
    }
};

template<typename T>
class ArrayBuffer
{
  GLuint id;
  GLuint type;
  GLenum usage;
  size_t size;

public:
  ArrayBuffer(GLuint type_=GL_ARRAY_BUFFER, GLenum usage_=GL_STATIC_DRAW)
    : type(type_),
      usage(usage_)
  {
      check_gl_error();
      glGenBuffers(1, &id);
      check_gl_error();
  }

  void Bind() {
      check_gl_error();
      glBindBuffer(type, id);
      check_gl_error();
  }

  void Store( T *p, GLsizei size_ ) {
      size = size_;
      Bind();
      glBufferData(type, sizeof(T)*size, p, usage);
      check_gl_error();
  }

  void Store( FlatArray<T> a) {
      size = a.Size();
      Store(&a[0], sizeof(T)*size);
  }

  auto Size() { return size; }
};

class GUI;

class Scene
{
protected:
  Mat4 model;
  Mat4 view;
  Mat4 projection;

public:

  Scene();
  virtual void Update(const GUI& gui);
  virtual void Render() = 0;
  virtual ~Scene();
};

class GUI
{
  GLuint vertex_buffer, program;
  GLuint coordinates_buffer, colors_buffer, coefs_buffer;
  GLint mvp_location, vpos_location, vcol_location, coefs_location;
  GLuint u_tbo_tex;
  GLuint vao;
  GLuint textureID;
  GLuint buffer;
  GLuint tex;

  float ratio;
  int width, height;
  double told;


  Array<shared_ptr<Scene>> scenes;

public:
  GLFWwindow* window;

  GUI();
  void Update();
  void Render();
  void GetMatrices(Mat4 & model, Mat4 &view, Mat4 &projection) const;
  void SwapBuffers();
  bool ShouldCloseWindow();
  virtual ~GUI();
};


class MeshScene : public Scene
{
protected:
  shared_ptr<ngcomp::MeshAccess> ma;
  Program shaderProgram;

  GLuint vao;
  GLint vpos_location, mvp_location, fcolor_location;
  ArrayBuffer<GLfloat> coordinates_buffer;
  ArrayBuffer<GLuint> trig_index_buffer, line_index_buffer;
  size_t nvertices, nlines, ntrigs;

public:

  MeshScene(shared_ptr<ngcomp::MeshAccess> ma_); 

  virtual void Update(const GUI &gui) override;
  virtual void Render() override;
  virtual void RenderWireframe();
  virtual void RenderSurface();

  virtual ~MeshScene();
};

class SolutionScene : public MeshScene
{
  shared_ptr<ngcomp::GridFunction> gf;
  shared_ptr<MeshScene> mesh_scene;

  Program solution_program;
  GLint tbo_tex_location;
  GLuint textureID;

  GLuint buffer;
  GLuint tex;

public:

  SolutionScene(shared_ptr<ngcomp::GridFunction> gf_); 

  virtual void Update(const GUI &gui) override;

  virtual void Render() override;

  virtual ~SolutionScene();
};

namespace shaders {
    extern string vertex_mesh;
    extern string fragment_mesh;
    extern string fragment_header;
    extern string fragment_main;
    extern string vertex_simple;
}
