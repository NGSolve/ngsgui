#include <gui.hpp>
#include <generate_shader.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;


auto TRANSPOSE=GL_TRUE;


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
  glEnable(GL_DEPTH_TEST);
  glPolygonOffset (-1, -1);
  glEnable(GL_POLYGON_OFFSET_LINE);
  glDepthFunc(GL_LEQUAL);
  do_zoom = false;
  do_rotate = false;
  do_translate = false;
}

void GUI::ZoomReset() {
    rotmat = Identity();
    zoom=0.0f;
    dx=0.0f;
    dy=0.0f;
}

void GUI::MouseMove(int dxm, int dym)
{
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
}

void GUI::MouseClick(int button, bool press)
{
  if(press) {
      switch(button) {
        case 1:
          do_rotate = true;
          break;
        case 4:
          do_translate = true;
          break;
        case 2:
          do_zoom = true;
          break;
      }
  }
  else {
      do_rotate = false;
      do_zoom = false;
      do_translate = false;
  }
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
  check_gl_error();

  for(auto & scene : scenes)
      scene->Render();
}

GUI::~GUI() {
}

void GUI::GetMatrices(Mat4 &model, Mat4 &view, Mat4 &projection) const {
    check_gl_error();
    float ratio = width / (float) height;
    glViewport(0,0,width, height);

    float s = 0.6f;

    view = Identity();
    model = Identity();
    projection = Identity();

    projection = Perspective(0.8f, ratio, .1f, 20.f);

    Vec3 eye = { 0.f, 0.f, 6.0f };
    Vec3 center = { 0.f, 0.f, 0.f };
    Vec3 up = { 0.f, 1.f, 0.f };
    view = LookAt( eye, center, up );

    model = rotmat*model;
    model = Translate(dx, -dy, -0 )*model;
    model = Scale(exp(-zoom/100))*model;
    model = Translate(0, -0, -5 )*model;
}

MeshScene::MeshScene(shared_ptr<ngcomp::MeshAccess> ma_)
     : ma(ma_)
{
  check_gl_error();
  Shader vshader{shaders::vertex_mesh, GL_VERTEX_SHADER};
  Shader fshader{shaders::fragment_mesh, GL_FRAGMENT_SHADER};
  //         Shader gshader{shaders::geometry_copy, GL_GEOMETRY_SHADER};
  check_gl_error();

  shaderProgram = Program{vshader, fshader/*, gshader*/};
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
          trig_indices.Append(sorted_vertices[0]);
          trig_indices.Append(sorted_vertices[1]);
          trig_indices.Append(sorted_vertices[2]);
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
              // Todo: put numbers for tetrahedra
              trig_indices.Append(sorted_vertices[0]);
              trig_indices.Append(sorted_vertices[1]);
              trig_indices.Append(sorted_vertices[2]);
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

  if(vpos_location>=0)
  {
      glEnableVertexAttribArray(vpos_location);
      glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
      check_gl_error();
  }

  if(trig_index_location>=0)
  {
      glEnableVertexAttribArray(trig_index_location);
      glVertexAttribIPointer(trig_index_location, 1, GL_BYTE, 0, (void*) 0);
      check_gl_error();
  }

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
     : gf(gf_)
{
  mesh_scene = make_shared<MeshScene>(gf_->GetMeshAccess());

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
  check_gl_error();
  vpos_location = glGetAttribLocation(solution_program.id, "vPos");
  check_gl_error();
  trig_index_location = glGetAttribLocation(solution_program.id, "vIndex");
  check_gl_error();
  mv_location = glGetUniformLocation(solution_program.id, "MV");
  check_gl_error();
  p_location = glGetUniformLocation(solution_program.id, "P");
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

  glEnableVertexAttribArray(trig_index_location);
  mesh_scene->trig_index_buffer.Bind();
  glVertexAttribIPointer(trig_index_location, 1, GL_BYTE, 0, (void*) 0);
  check_gl_error();
}

void SolutionScene::Update(const GUI & gui)
{
  Scene::Update(gui);
  mesh_scene->Update(gui);
  model = mesh_scene->model;
}

void SolutionScene::Render()
{
  check_gl_error();
  auto mv = view*model;
  auto p = projection;
  check_gl_error();
  glUseProgram(solution_program.id);
  check_gl_error();
  glUniformMatrix4fv(mv_location, 1, TRANSPOSE, (const GLfloat*) &mv);
  glUniformMatrix4fv(p_location, 1, TRANSPOSE, (const GLfloat*) &p);
  check_gl_error();

  // Set vPos input
  glEnableVertexAttribArray(vpos_location);
  check_gl_error();
  mesh_scene->coordinates_buffer.Bind();
  check_gl_error();
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  check_gl_error();

  glEnableVertexAttribArray(trig_index_location);
  mesh_scene->trig_index_buffer.Bind();
  glVertexAttribIPointer(trig_index_location, 1, GL_BYTE, 0, (void*) 0);
  check_gl_error();

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glDrawArrays(GL_TRIANGLES, 0, 3*mesh_scene->ntrigs);
  check_gl_error();
  mesh_scene->RenderWireframe();
}

SolutionScene::~SolutionScene()
{
}


typedef void (*__GLXextFuncPtr)(void);
extern "C" __GLXextFuncPtr glXGetProcAddressARB (const GLubyte *);
extern "C" void (*glXGetProcAddress(const GLubyte *procname))( void );


PYBIND11_MODULE(ngui, m) {
    py::class_<GUI>(m, "GUI")
      .def(py::init<>())
      .def("Update", &GUI::Update)
      .def("Render", &GUI::Render)
      .def("AddScene", &GUI::AddScene)
      .def("SetSize", &GUI::SetSize)
      .def("MouseClick", &GUI::MouseClick)
      .def("MouseMove", &GUI::MouseMove)
      .def("ZoomReset", &GUI::ZoomReset)
      ;

    m.def("Init", [] () {
          glewExperimental=GL_TRUE;
          GLenum err = glewInit();

          if (GLEW_OK != err)
          {
          /* Problem: glewInit failed, something is seriously wrong. */
          fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

          }

          fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
          });
    py::class_<Scene, shared_ptr<Scene>>(m, "Scene");

    py::class_<MeshScene, Scene, shared_ptr<MeshScene>>(m, "MeshScene")
      .def(py::init<shared_ptr<ngcomp::MeshAccess>>())
      .def("Update", &MeshScene::Update)
      .def("Render", &MeshScene::Render)
      ;

    py::class_<SolutionScene, Scene, shared_ptr<SolutionScene>>(m, "SolutionScene")
      .def(py::init<shared_ptr<ngcomp::GridFunction>>())
      ;
}
