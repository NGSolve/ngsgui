#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using std::string;

#include <glerror.hpp>
#include <generate_shader.hpp>

string readFile(string f) {
    auto ifs = std::ifstream(f);
    return string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

constexpr int order = 10;
constexpr int ndof = (order+1)*(order+2)*(order+3)/6;
constexpr float ticks_per_second = 10;


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
      ofstream file(string("shader.") + ((type==GL_VERTEX_SHADER) ? "vert" : "frag"));
      file << p << endl;

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

    Program (std::initializer_list<Shader> list) 
      : shaders(list)
      {
        id = glCreateProgram();
        for( auto &shader : shaders)
          glAttachShader(id, shader.id);
        glLinkProgram(id);
      }


};

static const float coordinates[] = {
    -1.0f,  -1.0f, 0.0f, 
    1.0f,  -1.0f,  0.0f, 
    -1.0f,   1.0f, 0.0f, 

    0.0f,  0.0f,   0.0f,
    0.0f,  1.0f,   0.0f,
    1.0f,  0.0f,   0.0f,
                        
    -0.6f, -0.4f,  0.0f,
    0.6f, -0.4f,   0.0f,
    0.f,  0.6f,    0.0f,
                        
    0.6f, -0.4f,   0.0f,
    0.f,  0.6f,    0.0f,
    0.9f,  0.9f,   0.0f,
};                      

static float colors[ndof];

static float coefs[3*4*5/2];

static const struct
{
  float x, y;
  float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

double mousex, mousey;
bool do_rotate;
GLfloat alpha = 210.f, beta = -70.f;

int main()
{
  GLFWwindow* window;
  GLuint vertex_buffer, program;
  GLuint coordinates_buffer, colors_buffer, coefs_buffer;
  GLint mvp_location, vpos_location, vcol_location, coefs_location;

  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

  window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
  if (!window)
    {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    if (action == GLFW_PRESS)
    {
        do_rotate = true;
        glfwGetCursorPos(window, &mousex, &mousey);
    }
    else
        do_rotate = false;
  });
                             
  glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
    if(!do_rotate) return;
    alpha += (GLfloat) (x - mousex) / 10.f;
    beta += (GLfloat) (y - mousey) / 10.f;

    mousex = x;
    mousey = y;
  });

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);

  // NOTE: OpenGL error checks have been omitted for brevity
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  Shader vertex_shader(shaders::vertex::simple, GL_VERTEX_SHADER);
  string fragment_shader_string = readFile("shader/header.frag") + genshader::GenerateCode(order) + readFile("shader/main.frag");
  ofstream fsfile("shader/trig.frag");
  fsfile << fragment_shader_string << endl;
  Shader fragment_shader(fragment_shader_string, GL_FRAGMENT_SHADER);

  Program program1({vertex_shader, fragment_shader});
  program = program1.id;
  check_gl_error() ;

  mvp_location = glGetUniformLocation(program, "MVP");
  vpos_location = glGetAttribLocation(program, "vPos");
  GLuint u_tbo_tex = glGetUniformLocation(program, "u_tbo_tex");
  check_gl_error() ;
  //     vcol_location = glGetAttribLocation(program, "vCol");
  //     coefs_location = glGetAttribLocation(program, "vCoefs");

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &coordinates_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, coordinates_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(coordinates), coordinates, GL_STATIC_DRAW);

  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

  ////////////////////////////
  //// Create one OpenGL texture
  GLuint textureID;
  glGenTextures(1, &textureID);

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_1D, textureID);
  glActiveTexture(GL_TEXTURE0);
  std::cout << "u_tbo_tex " << u_tbo_tex << std::endl;
  //     glUniform1i(u_tbo_tex, 0);

  // Generate and fill buffer object
  GLuint buffer;
  glGenBuffers   ( 1, &buffer );
  glBindBuffer   ( GL_TEXTURE_BUFFER, buffer );
  glBufferData   ( GL_TEXTURE_BUFFER, sizeof(colors), NULL, GL_DYNAMIC_DRAW );  // Alloc
  glBufferSubData( GL_TEXTURE_BUFFER, 0, sizeof(colors), colors );              // Fill
  // Generate texture "wrapper" around buffer object
  GLuint tex;
  glGenTextures  ( 1, &tex );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture  ( GL_TEXTURE_BUFFER, tex );
  glTexBuffer    ( GL_TEXTURE_BUFFER, GL_R32F, buffer );

  int dof = 0;
  double told = glfwGetTime();
  while (!glfwWindowShouldClose(window))
    {
      for(float & c : colors) {
          c = 0.0;
      }
      int dof = ((int)(glfwGetTime()*ticks_per_second))%ndof;
      colors[dof] = 1.0;

      glBindBuffer   ( GL_TEXTURE_BUFFER, buffer );
      glBufferSubData( GL_TEXTURE_BUFFER, 0, sizeof(colors), colors );

      float ratio;
      int width, height;
      mat4x4 m, p, mvp;

      glfwGetFramebufferSize(window, &width, &height);
      ratio = width / (float) height;

      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT);

      mat4x4_identity(m);
      mat4x4_rotate_X(m, m, beta );
      mat4x4_rotate_Z(m, m, alpha );
      mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      mat4x4_mul(mvp, p, m);

      glUseProgram(program);
      glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
      glDrawArrays(GL_TRIANGLES, 0, 3);

      glfwSwapBuffers(window);
      glfwPollEvents();
      double t = glfwGetTime();
      cout << "\rframes per second: " << 1.0/(t-told) << std::flush;
      told = t;
    }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}

