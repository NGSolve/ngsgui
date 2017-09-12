#include <gui.hpp>
#include <comp.hpp>

int main()
{
  {
      auto ma = make_shared<ngcomp::MeshAccess>(ngcomp::MeshAccess("square.vol"));
      GUI gui;
      MeshScene ms(ma, gui);
      while (!gui.ShouldCloseWindow())
      {
          ms.Update();
          gui.Render();
          ms.Render();
          gui.SwapBuffers();
      }
  }

  exit(EXIT_SUCCESS);
}

