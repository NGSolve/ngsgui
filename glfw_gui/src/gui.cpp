#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gui.hpp>
#include <generate_shader.hpp>
#include <pybind11/pybind11.h>
#include <linmath.h>

namespace py = pybind11;
// constexpr int order = 20;
// constexpr int ndof = (order+1)*(order+2)*(order+3)/6;
// constexpr int ndof = (order+1)*(order+2)/2;
constexpr float ticks_per_second = 5;

double mousex, mousey;
bool do_rotate;
bool do_translate;
bool do_zoom;


Mat4 rotmat;
auto TRANSPOSE=GL_TRUE;

GLfloat zoom=0.0f, dx=0.0f, dy=0.0f;

void Scene::Update(const GUI& gui)
{
  gui.GetMatrices(model, view, projection);
}

Scene::Scene()
{
}

Scene::~Scene()
{
}



GUI::GUI()
{
  rotmat = Identity();
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
                             
  glfwSetScrollCallback(window, 
      [](GLFWwindow* window, double x, double y)
      {
          zoom -= 5.0f*y;
      }
  );


  glfwSetCursorPosCallback(window, 
      [](GLFWwindow* window, double x, double y)
      {
        float dxm = x-mousex;
        float dym = y-mousey;
        if(do_rotate)
        {
          rotmat = RotateY(-dxm/50.0f)*rotmat;
          rotmat = RotateX(-dym/50.0f)*rotmat;
        }

        if(do_translate)
        {
          float s = 0.20*exp(-zoom/100);
          dx += dxm / (1000.f*s);
          dy += dym / (1000.f*s);
        }

        if(do_zoom)
          zoom += dym;

        mousex = x;
        mousey = y;
      });

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);
  glEnable(GL_DEPTH_TEST);
  glPolygonOffset (-1, -1);
  glEnable(GL_POLYGON_OFFSET_LINE);
  glDepthFunc(GL_LEQUAL);
}

void GUI::Update()
{
  for(auto & scene : scenes)
    scene->Update(*this);
}

void GUI::Render()
{
        check_gl_error();
      glClearColor( 1.f, 1.f, 1.f, 0.f);
        check_gl_error();
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        check_gl_error();
//       glViewport(0, 0, width, height);
        check_gl_error();
//       glUseProgram(program);
//       glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp(0,0));
//       glDrawArrays(GL_TRIANGLES, 0, 3);

  for(auto & scene : scenes)
    scene->Render();
  SwapBuffers();
}

void GUI::SwapBuffers()
{
      glfwSwapBuffers(window);
      glfwPollEvents();
      double t = glfwGetTime();
//       std::cout << "\rframes per second: " << 1.0/(t-told) << std::flush;
      told = t;

}

bool GUI::ShouldCloseWindow() {
    return glfwWindowShouldClose(window);
}
GUI::~GUI() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void GUI::GetMatrices(Mat4 &model, Mat4 &view, Mat4 &projection) const {
    check_gl_error();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float ratio = width / (float) height;
    glViewport(0,0,width, height);

    float s = 0.6f;

    view = Identity();
    model = Identity();
    projection = Identity();

    projection = Perspective(0.8f, ratio, .1f, 20.f);

    mat4x4 Mview;
    vec3 eye = { 0.f, 0.f, 6.0f };
    vec3 center = { 0.f, 0.f, 0.f };
    vec3 up = { 0.f, 1.f, 0.f };
    mat4x4_look_at( Mview, eye, center, up );

    for(int i : Range(4))
      for(int j : Range(4)) {
          view(i,j) = Mview[i][j];
      }

    model = rotmat*model;
    // move unit square to center
    model = Translate(dx, -dy, -0 )*model;
    model = Scale(exp(-zoom/100))*model;
    model = Translate(0, -0, -5 )*model;
//     return projection*view*model;
}

MeshScene::MeshScene(shared_ptr<ngcomp::MeshAccess> ma_)
      : ma(ma_)
    {
        check_gl_error();
        Shader vshader{shaders::vertex_mesh, GL_VERTEX_SHADER};
        Shader fshader{shaders::fragment_mesh, GL_FRAGMENT_SHADER};
        Shader gshader{shaders::geometry_copy, GL_GEOMETRY_SHADER};
        check_gl_error();

        shaderProgram = Program{vshader, fshader, gshader};
        check_gl_error();
        ngstd::Array<GLfloat> coordinates;
        ngstd::Array<GLbyte> trig_indices;

        check_gl_error();

        if(ma->GetDimension()==2)
        {
            ntrigs = ma->GetNE();
            for (auto i : ngcomp::Range(ntrigs)) {
                auto verts = ma->GetElement(ElementId( VOL, i)).Vertices();

                ArrayMem<int,3> sorted_vertices{0,1,2};
                ArrayMem<int,3> unsorted_vertices{verts[0], verts[1], verts[2]};

                BubbleSort (unsorted_vertices, sorted_vertices);
                for (auto j : ngcomp::Range(3)) {
                    auto v = ma->GetPoint<3>(verts[j]);
                    coordinates.Append(v[0]);
                    coordinates.Append(v[1]);
                    coordinates.Append(v[2]);
                }
                trig_indices.Append(unsorted_vertices[0]);
                trig_indices.Append(unsorted_vertices[1]);
                trig_indices.Append(unsorted_vertices[2]);
            }
        }
        else
        {
            ntrigs = ma->GetNSE();

            for (auto i : ngcomp::Range(ntrigs)) {
                auto verts = ma->GetElement(ElementId( BND, i)).Vertices();

                ArrayMem<int,3> sorted_vertices{0,1,2};
                ArrayMem<int,3> unsorted_vertices{verts[0], verts[1], verts[2]};

                BubbleSort (unsorted_vertices, sorted_vertices);
                for (auto j : ngcomp::Range(3)) {
                    auto v = ma->GetPoint<3>(unsorted_vertices[j]);
                    coordinates.Append(v[0]);
                    coordinates.Append(v[1]);
                    coordinates.Append(v[2]);
                    trig_indices.Append(0);
                    trig_indices.Append(1);
                    trig_indices.Append(2);
                }
            }
        }

        glGenVertexArrays(1, &vao);
        check_gl_error();
        glBindVertexArray(vao);
        check_gl_error();

        coordinates_buffer.Store(coordinates);
        trig_index_buffer.Store(trig_indices);

        trig_index_location = glGetAttribLocation(shaderProgram.id, "vIndex");
        check_gl_error();
        vpos_location = glGetAttribLocation(shaderProgram.id, "vPos");
        check_gl_error();
        mv_location = glGetUniformLocation(shaderProgram.id, "MV");
        check_gl_error();
        p_location = glGetUniformLocation(shaderProgram.id, "P");
        check_gl_error();
        fcolor_location = glGetUniformLocation(shaderProgram.id, "fColor");
        check_gl_error();

        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        check_gl_error();

//         glEnableVertexAttribArray(trig_index_location);
//         glVertexAttribIPointer(trig_index_location, 1, GL_BYTE, 0, (void*) 0);
//         check_gl_error();

    }

void MeshScene::Update(const GUI &gui)
{
  Scene::Update(gui);
  netgen::Point3d pmin, pmax;
  if(ma->GetDimension()==2)
      ma->GetNetgenMesh()->GetBox(pmin, pmax);
  else
      ma->GetNetgenMesh()->GetBox(pmin, pmax, netgen::SURFACEPOINT);
  auto c = 0.5*(pmin+pmax);

  model = model*Translate(-c.X(), -c.Y(), -c.Z());
};

void MeshScene::Render()
{
  RenderSurface();
  RenderWireframe();
}

void MeshScene::RenderWireframe()
{
  SetupRender();
  glUniform4f(fcolor_location, 0.0f, 0.0f ,0.0f, 1.0f);
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glDrawArrays(GL_TRIANGLES, 0, 3*ntrigs);
  check_gl_error();
}

void MeshScene::RenderSurface()
{
  SetupRender();
  glUniform4f(fcolor_location, 0.0f, 1.0f ,0.0f, 1.0f);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glDrawArrays(GL_TRIANGLES, 0, 3*ntrigs);
  check_gl_error();
}

void MeshScene::SetupRender()
{
  check_gl_error();
  auto mv = view*model;
  auto p = projection;
  glUseProgram(shaderProgram.id);
  glUniformMatrix4fv(mv_location, 1, TRANSPOSE, (const GLfloat*) &mv);
  glUniformMatrix4fv(p_location, 1, TRANSPOSE, (const GLfloat*) &p);

  glEnableVertexAttribArray(vpos_location);
  coordinates_buffer.Bind();
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

//   glEnableVertexAttribArray(trig_index_location);
//   trig_index_buffer.Bind();
//   glVertexAttribIPointer(trig_index_location, 1, GL_BYTE, 0, (void*) 0);
}

MeshScene::~MeshScene() {
}

SolutionScene::SolutionScene(shared_ptr<ngcomp::GridFunction> gf_)
      : MeshScene(gf_->GetMeshAccess()), gf(gf_)
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
  trig_index_location = glGetAttribLocation(solution_program.id, "vIndex");
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

//   glEnableVertexAttribArray(trig_index_location);
//   trig_index_buffer.Bind();
//   glVertexAttribIPointer(trig_index_location, 1, GL_BYTE, 0, (void*) 0);
//   check_gl_error();
}

void SolutionScene::Update(const GUI & gui)
{
  MeshScene::Update(gui);
}

void SolutionScene::Render()
{
  check_gl_error();
  auto mv = view*model;
  auto p = projection;
  glUseProgram(shaderProgram.id);
  glUniformMatrix4fv(mv_location, 1, TRANSPOSE, (const GLfloat*) &mv);
  glUniformMatrix4fv(p_location, 1, TRANSPOSE, (const GLfloat*) &p);
  check_gl_error();

  // Set vPos input
  glEnableVertexAttribArray(vpos_location);
  check_gl_error();
  coordinates_buffer.Bind();
  check_gl_error();
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  check_gl_error();

  glEnableVertexAttribArray(trig_index_location);
  trig_index_buffer.Bind();
  glVertexAttribIPointer(trig_index_location, 1, GL_BYTE, 0, (void*) 0);
  check_gl_error();

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glDrawArrays(GL_TRIANGLES, 0, 3*ntrigs);
  check_gl_error();
  RenderWireframe();
}

SolutionScene::~SolutionScene()
{
}


PYBIND11_MODULE(ngui, m) {
    py::class_<GUI>(m, "GUI")
      .def(py::init<>())
      .def("Update", &GUI::Update)
      .def("Render", &GUI::Render)
      .def("AddScene", &GUI::AddScene)
      ;

    py::class_<Scene, shared_ptr<Scene>>(m, "Scene");

    py::class_<MeshScene, Scene, shared_ptr<MeshScene>>(m, "MeshScene")
      .def(py::init<shared_ptr<ngcomp::MeshAccess>>())
      ;

    py::class_<SolutionScene, MeshScene, shared_ptr<SolutionScene>>(m, "SolutionScene")
      .def(py::init<shared_ptr<ngcomp::GridFunction>>())
      ;

//     m.def("Draw", [] (shared_ptr<ngcomp::GridFunction> gf) {
// //       auto gui = make_shared<GUI>();
//       auto scene = make_shared<SolutionScene>(gf);
// //       while (!gui->ShouldCloseWindow())
// //       {
// //           scene.Update(*gui);
// //           gui->Render();
// //           scene.Render();
// // //           scene.RenderWireframe();
// //           gui->SwapBuffers();
// //       }
//       return scene;
//    });
//     m.def("Draw", [] (shared_ptr<ngcomp::MeshAccess> ma) {
//       auto gui = make_shared<GUI>();
//       MeshScene scene(ma);
//       while (!gui->ShouldCloseWindow())
//       {
//           scene.Update(*gui);
//           gui->Render();
//           scene.Render();
//           gui->SwapBuffers();
//       }
//    });
}
