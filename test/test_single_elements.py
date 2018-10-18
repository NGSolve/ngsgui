from headless import HeadlessGUI as Gui
from ngsolve import *
ngsglobals.msg_level = 0
import meshes
import ngsgui.gl as gl
# from ngsgui.glwindow import RenderingParameters
from ngsgui.scenes import MeshScene, RenderingSettings, SolutionScene
import OpenGL.GL as GL
import pytest

gui = Gui()

def getParameters():
    settings = RenderingSettings()
    settings.initGL()
    settings.setColormapMin(-0.3)
    settings.setColormapMax(1.0)
    settings.min = Vector([0,0,0])
    settings.max = Vector([1,1,1])
    settings.rotateCamera(-30, 30)
    settings.zoom=-50
    settings.dx = -0.2
    settings.dy = 0.1
    settings.setLightAmbient(0.5)
    settings.setLightDiffuse(0.5)
    return settings

@pytest.mark.parametrize("name,mesh", meshes.meshes_3d)
def test_cf(name, mesh):

    settings = getParameters()
    settings.individualClippingPlane = True
    settings.setClippingNormal([0,0,1])
    settings.zoom=-100
    s = SolutionScene(z+x*x-0.3*y*y, mesh, iso_surface=x+2*y+z*z)
    s.setOrder(3)
    s.setSubdivision(3)
    s._global_rendering_parameters = settings
    s.setShowSurface(True)
    s.setShowClippingPlane(True)
    s.initGL()
    s.update()

    gui.clear()
    s.render(settings);
    GL.glFinish()
    gui.check_image('cf_'+name)

    if name == 'tet':
        s.setShowSurface(False)
        s.setShowClippingPlane(False)
        s.setShowIsoSurface(True)
        s.setIsoValue(0.7)
        gui.clear()
        s.render(settings);
        GL.glFinish()
        gui.check_image('iso_'+name)

@pytest.mark.parametrize("name,mesh", meshes.meshes_3d)
def test_mesh(name, mesh):
    gui.clear()
    settings = getParameters()
    s = MeshScene(mesh)
    s._global_rendering_parameters = settings
    colors = [0,255,0,255]*100
    s.setMaterialColors(colors)
    s.setSurfaceColors(colors)
    s.setEdgeColors(colors)
    s.setShowSurface(True)
    s.initGL()
    s.update()

    for els in s.mesh_data.elements[BND]:
        with gl.Query(GL.GL_PRIMITIVES_GENERATED) as q:
            s._render2DElements(settings, els, True);

    GL.glFinish()
    gui.check_image(name)

if __name__ == '__main__':
    for name,mesh in meshes.meshes_3d:
        test_mesh(name, mesh)
        test_cf(name, mesh)
