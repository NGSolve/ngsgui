#pragma once

namespace shaders {

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

    namespace fragment {
            static string header = R"shader_string(
#version 150
uniform samplerBuffer tex;
in vec3 posx;

vec3 hsv2rgb(vec3 c)
{
    // c.x = 0.5*c.x + 0.5;
    c.x = 2.5*c.x + 0.5;
    c.x = clamp(c.x, 0.0, 1.0);
    c.x = (1.0 - c.x);
    c.x = floor(8*c.x)/7.0;
    c.x = clamp(c.x, 0.0, 1.0);
    c.x = c.x*240.0/360.0;
//    c.x = c.x*0.75;
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
)shader_string";

static string main = R"shader_string(
void main()
{
  float x = 1.0-posx.x-posx.y;
  float y = posx.y;
  float z = posx.z;
//   gl_FragColor = vec4(texelFetch( tex, 0).r, texelFetch(tex, 0).g, texelFetch(tex, 0).b, 1.0);
  // gl_FragColor = vec4(Eval(x,y, z), 0.0, 0.0, 1.0);
  gl_FragColor = vec4(hsv2rgb(vec3(Eval(x,y, z), 1.0, 1.0)), 1.0);
}
)shader_string";
}

namespace vertex {

    static string simple = R"shader_string(
#version 150
uniform mat4 MVP;

// in vec3 vCol;
in vec3 vPos;

// out vec3 vCoefs;

// out vec3 color;
out vec3 posx;
out vec3 coefs;
void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
//    color = vCol;
    posx = 0.5*vPos +0.5;
//    coefs = vec3(0.0, 0.0, 0.0);
//    coefs = vCoefs;
}
)shader_string";
    }
}
