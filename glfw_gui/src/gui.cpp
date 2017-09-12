#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gui.hpp>
#include <generate_shader.hpp>
#include <pybind11/pybind11.h>
#include <linmath.h>

// constexpr int order = 20;
// constexpr int ndof = (order+1)*(order+2)*(order+3)/6;
// constexpr int ndof = (order+1)*(order+2)/2;
constexpr float ticks_per_second = 5;

double mousex, mousey;
bool do_rotate;
bool do_translate;
bool do_zoom;


auto TRANSPOSE=GL_TRUE;

GLfloat alpha = 0.0f, beta = 0.f, zoom=0.0f, dx=0.0f, dy=0.0f;

GUI::GUI()
{
  glfwSetErrorCallback(
      [](int error, const char* description)
      {
        std::cerr << "Error: " << description << std::endl;
      });


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

  glfwSetKeyCallback(window,
      [](GLFWwindow* window, int key, int scancode, int action, int mods)
      {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
      });

  glfwSetMouseButtonCallback(window, 
      [](GLFWwindow* window, int button, int action, int mods)
      {
        if (action == GLFW_PRESS)
        {
        switch(button) {
          case GLFW_MOUSE_BUTTON_LEFT:
            do_rotate = true;
            glfwGetCursorPos(window, &mousex, &mousey);
            break;
          case GLFW_MOUSE_BUTTON_MIDDLE:
            do_translate = true;
            glfwGetCursorPos(window, &mousex, &mousey);
            break;
          case GLFW_MOUSE_BUTTON_RIGHT:
            do_zoom = true;
            glfwGetCursorPos(window, &mousex, &mousey);
            break;
            }
        }
        else {
            do_rotate = false;
            do_zoom = false;
            do_translate = false;
        }
      });
                             
  glfwSetCursorPosCallback(window, 
      [](GLFWwindow* window, double x, double y)
      {
        if(do_rotate)
        {
          alpha += (GLfloat) (x - mousex) / 10.f;
          beta += (GLfloat) (y - mousey) / 10.f;
        }

        if(do_translate)
        {
          float s = 0.45*exp(-zoom/100);
          dx += (GLfloat) (x - mousex) / (1000.f*s);
          dy += (GLfloat) (y - mousey) / (1000.f*s);
        }

        if(do_zoom)
          zoom += (GLfloat) (y-mousey);

        mousex = x;
        mousey = y;
      });

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);
}

void GUI::Update()
{
}

void GUI::Render()
{
        check_gl_error();
      glClearColor( 1.f, 1.f, 1.f, 0.f);
        check_gl_error();
      glClear(GL_COLOR_BUFFER_BIT);
        check_gl_error();
//       glViewport(0, 0, width, height);
        check_gl_error();
//       glUseProgram(program);
//       glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp(0,0));
//       glDrawArrays(GL_TRIANGLES, 0, 3);

}

void GUI::SwapBuffers()
{
      glfwSwapBuffers(window);
      glfwPollEvents();
      double t = glfwGetTime();
      std::cout << "\rframes per second: " << 1.0/(t-told) << std::flush;
      told = t;

}

bool GUI::ShouldCloseWindow() {
    return glfwWindowShouldClose(window);
}
GUI::~GUI() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

Mat4 GUI::GetMVP() {
    check_gl_error();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float ratio = width / (float) height;
    glViewport(0,0,width, height);

    float s = 0.6f;
    Mat4 mvp = Identity();

    Mat4 view = Identity();
    Mat4 model = Identity();
    Mat4 projection = Identity();

//     if(ratio >= 1.0f)
//         projection = Ortho(-ratio*s, ratio*s, -s, s, 1.f, -1.f);
//     else
//         projection = Ortho(-s, s, -s/ratio, s/ratio, 1.f, -1.f);
// 
//     projection = Ortho(-s, s, -s/ratio, s/ratio, 10.f, -10.f);
    projection = Perspective(1.0f, ratio, .1f, 20.f);

    mat4x4 M;
    mat4x4_identity(M);
    mat4x4_perspective(M, 1.0f, ratio, 0.1f, 20.f);


    mat4x4 Mview;
    vec3 eye = { 0.f, 0.f, 2.0f };
    vec3 center = { 0.f, 0.f, 0.f };
    vec3 up = { 0.f, 1.f, 0.f };
    mat4x4_look_at( Mview, eye, center, up );

    for(int i : Range(4))
      for(int j : Range(4)) {
          view(i,j) = Mview[i][j];
      }

    // move unit square to center
    model = Translate(-0.5, -0.5, 0 )*model;

    model = RotateY(-alpha/5)*model;
    model = RotateX(-beta/5)*model;
    model = Translate(dx, -dy, -0 )*model;
    model = Scale(exp(-zoom/100))*model;
    model = Translate(0, -0, -2 )*model;
    return projection*view*model;
}

MeshScene::MeshScene(shared_ptr<ngcomp::MeshAccess> ma_, shared_ptr<GUI> gui_)
      : ma(ma_),
      gui(gui_)
    {
        check_gl_error();
        Shader vshader{shaders::vertex_mesh, GL_VERTEX_SHADER};
        Shader fshader{shaders::fragment_mesh, GL_FRAGMENT_SHADER};
        check_gl_error();

        shaderProgram = Program{vshader, fshader};
        check_gl_error();
        ngstd::Array<GLfloat> coordinates;

        check_gl_error();

        auto vertices = ma->Nodes(ngcomp::NT_VERTEX);
        nvertices = ma->GetNV();
        ntrigs = ma->GetNFaces();

        for (auto i : ngcomp::Range(ntrigs)) {
            auto verts = ma->GetElement(i).Vertices();

            ArrayMem<int,3> sorted_vertices{0,1,2};
            ArrayMem<int,3> unsorted_vertices{verts[0], verts[1], verts[2]};

            BubbleSort (unsorted_vertices, sorted_vertices);
            for (auto j : ngcomp::Range(3)) {
                auto v = ma->GetPoint<3>(unsorted_vertices[j]);
                coordinates.Append(v[0]);
                coordinates.Append(v[1]);
                coordinates.Append(v[2]);
            }
        }

        glGenVertexArrays(1, &vao);
        check_gl_error();
        glBindVertexArray(vao);
        check_gl_error();

        coordinates_buffer.Store(coordinates);

        vpos_location = glGetAttribLocation(shaderProgram.id, "vPos");
        check_gl_error();
        mvp_location = glGetUniformLocation(shaderProgram.id, "MVP");
        check_gl_error();
        fcolor_location = glGetUniformLocation(shaderProgram.id, "fColor");
        check_gl_error();

        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        check_gl_error();
    }

void MeshScene::Render()
{
  RenderSurface();
  RenderWireframe();
}

void MeshScene::RenderWireframe()
{
  check_gl_error();
  auto mvp = gui->GetMVP();
  glUseProgram(shaderProgram.id);
  glUniformMatrix4fv(mvp_location, 1, TRANSPOSE, (const GLfloat*) &mvp);
  glUniform4f(fcolor_location, 0.0f, 0.0f ,0.0f, 1.0f);
  glEnableVertexAttribArray(vpos_location);
  coordinates_buffer.Bind();
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glDrawArrays(GL_TRIANGLES, 0, 3*ntrigs);
  check_gl_error();
}

void MeshScene::RenderSurface()
{
  check_gl_error();
  auto mvp = gui->GetMVP();
  glUseProgram(shaderProgram.id);
  glUniformMatrix4fv(mvp_location, 1, TRANSPOSE, (const GLfloat*) &mvp);
  glUniform4f(fcolor_location, 0.0f, 1.0f ,0.0f, 1.0f);
  glEnableVertexAttribArray(vpos_location);
  coordinates_buffer.Bind();
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glDrawArrays(GL_TRIANGLES, 0, 3*ntrigs);
  check_gl_error();
}

MeshScene::~MeshScene() {
}

SolutionScene::SolutionScene(shared_ptr<ngcomp::GridFunction> gf_, shared_ptr<GUI> gui_)
      : MeshScene(gf_->GetMeshAccess(), gui_), gf(gf_)
{
  auto &fes = *gf->GetFESpace();
  int order = fes.GetOrder();
  int ndof = fes.GetOrder();
  check_gl_error();
  Shader vertex_shader(shaders::vertex_simple, GL_VERTEX_SHADER);
  check_gl_error();
  string fragment_shader_string = genshader::GenerateCode(order);
  Shader fragment_shader(fragment_shader_string, GL_FRAGMENT_SHADER);
  check_gl_error();
  solution_program = Program{ vertex_shader, fragment_shader};
  check_gl_error();

  tbo_tex_location = glGetUniformLocation(solution_program.id, "coefficients");
  vpos_location = glGetAttribLocation(solution_program.id, "vPos");
  check_gl_error();

  ////////////////////////////
  //// Create one OpenGL texture
  glGenTextures(1, &textureID);
  check_gl_error();

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_1D, textureID);
  glActiveTexture(GL_TEXTURE0);
  //     glUniform1i(tbo_tex_location, 0);


  auto coefficients = gf->GetVector().FVDouble();
  Array<GLfloat> coefficients_float(coefficients.Size());

  for( auto i : ngcomp::Range(coefficients.Size()))
      coefficients_float[i] = coefficients[i];

  check_gl_error();
  // Generate and fill buffer object
  glGenBuffers   ( 1, &buffer );
  glBindBuffer   ( GL_TEXTURE_BUFFER, buffer );
  glBufferData   ( GL_TEXTURE_BUFFER, sizeof(GLfloat)*coefficients_float.Size(), NULL, GL_STATIC_DRAW );  // Alloc
  glBufferSubData( GL_TEXTURE_BUFFER, 0, sizeof(GLfloat)*coefficients_float.Size(), &coefficients_float[0]); // Fill

  // Generate texture "wrapper" around buffer object
  glGenTextures  ( 1, &tex );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture  ( GL_TEXTURE_BUFFER, tex );
  glTexBuffer    ( GL_TEXTURE_BUFFER, GL_R32F, buffer );
  check_gl_error();

}

void SolutionScene::Update()
{
}

void SolutionScene::Render()
{
  check_gl_error();
  auto mvp = gui->GetMVP();
  glUseProgram(solution_program.id);
  check_gl_error();
  glUniformMatrix4fv(mvp_location, 1, TRANSPOSE, (const GLfloat*) &mvp);
  check_gl_error();

  // Set vPos input
  glEnableVertexAttribArray(vpos_location);
  check_gl_error();
  coordinates_buffer.Bind();
  check_gl_error();
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  check_gl_error();

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glDrawArrays(GL_TRIANGLES, 0, 3*ntrigs);
  check_gl_error();
}

SolutionScene::~SolutionScene()
{
}


PYBIND11_MODULE(ngui, m) {
    m.def("Draw", [] (shared_ptr<ngcomp::GridFunction> gf) {
      auto gui = make_shared<GUI>();
      SolutionScene scene(gf, gui);
      while (!gui->ShouldCloseWindow())
      {
          scene.Update();
          gui->Render();
          scene.Render();
          scene.RenderWireframe();
          gui->SwapBuffers();
      }
   });
    m.def("Draw", [] (shared_ptr<ngcomp::MeshAccess> ma) {
      auto gui = make_shared<GUI>();
      MeshScene scene(ma, gui);
      while (!gui->ShouldCloseWindow())
      {
          scene.Update();
          gui->Render();
          scene.Render();
          gui->SwapBuffers();
      }
   });
}
