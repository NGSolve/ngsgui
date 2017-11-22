#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <glmath.hpp>
#include <comp.hpp>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <glerror.hpp>

using std::string;


static std::map<decltype(GL_VERTEX_SHADER), string>
shader_extension{ {GL_VERTEX_SHADER, "vert"}, {GL_FRAGMENT_SHADER, "frag"}, {GL_GEOMETRY_SHADER, "geom"} };

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
        std::ofstream file(string("shader.") + shader_extension[type]);
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
    struct Attributes {
        GLuint id;
        auto operator [](const char *name) {
            return glGetAttribLocation(id, name);
        }
    };

    std::vector<Shader> shaders;
    GLuint id;
    Attributes attributes;

    Program ()  {}

    Program (std::initializer_list<Shader> list) 
      : shaders(list)
    {
        Compile();
        attributes.id = id;
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

public:
  Mat4 model;
  Mat4 view;
  Mat4 projection;

  Scene();
  virtual void Update(const GUI& gui);
  virtual void Render() = 0;
  virtual ~Scene();
};

class GUI
{
  float ratio;
  int width, height;
  double told;

  bool do_rotate;
  bool do_translate;
  bool do_zoom;
  GLfloat zoom=0.0f, dx=0.0f, dy=0.0f;

  Mat4 rotmat;


  Array<shared_ptr<Scene>> scenes;

public:
  GUI();

  void Update();
  void Render();
  void GetMatrices(Mat4 & model, Mat4 &view, Mat4 &projection) const;
  bool ShouldCloseWindow();
  void SetSize(int width_, int height_) {
      width = width_;
      height = height_;
  }
  void MouseMove(int dx, int dy);
  void MouseClick(int button, bool press);

  void ZoomReset();

  void AddScene( shared_ptr<Scene> scene )
  {
    scenes.Append(scene);
  }

  virtual ~GUI();
};


class MeshScene : public Scene
{
public:
  Program shaderProgram;
  GLint vpos_location, mv_location, p_location, fcolor_location, trig_index_location;

  shared_ptr<ngcomp::MeshAccess> ma;

  GLuint vao;
  ArrayBuffer<GLfloat> coordinates_buffer;
  ArrayBuffer<GLbyte> trig_index_buffer;
  size_t nvertices, nlines, ntrigs;


  MeshScene(shared_ptr<ngcomp::MeshAccess> ma_); 

  virtual void Update(const GUI &gui) override;
  virtual void Render() override;
  virtual void RenderWireframe();
  virtual void RenderSurface();
  virtual void SetupRender();

  virtual ~MeshScene();
};

class SolutionScene : public Scene
{
  shared_ptr<ngcomp::GridFunction> gf;
  shared_ptr<MeshScene> mesh_scene;

  Program solution_program;
  GLint vpos_location, mv_location, p_location, fcolor_location, trig_index_location;

  GLint tbo_tex_location, colormap_min_location, colormap_max_location, colormap_linear_location;
  GLuint textureID;

  GLuint buffer;
  GLuint tex;


public:
  float colormap_min = -1.0;
  float colormap_max =  1.0;
  bool colormap_linear = true;

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
    extern string geometry_copy;
}
