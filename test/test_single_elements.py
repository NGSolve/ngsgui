import ngsolve as ngs
ngs.ngsglobals.msg_level = 0
from headless import *
import meshes
import ngsgui.gl as gl
# from ngsgui.glwindow import RenderingParameters
from ngsgui.scenes import MeshScene, RenderingSettings, SolutionScene
import OpenGL.GL as GL
import pytest
import ngsgui

gui = HeadlessGUI(width=400, height=400)

def getParameters():
    from headless import _headless
    if _headless:
        settings = RenderingSettings()
        settings.initGL()
    else:
        from ngsgui.gui import gui
        settings = ngsgui.gui.gui.getCurrentGLWindow().glWidget._settings
    settings.setColormapMin(-0.3)
    settings.setColormapMax(1.0)
    settings.min = ngs.Vector([0,0,0])
    settings.max = ngs.Vector([1,1,1])
    settings.rotateCamera(-30, 30)
    settings.zoom=-50
    settings.dx = -0.2
    settings.dy = 0.1
    settings.setLightAmbient(0.5)
    settings.setLightDiffuse(0.5)
    settings.setShowCross(False)
    settings.setShowVersion(False)
    settings.setShowColorbar(False)
    return settings

@pytest.mark.parametrize("name,mesh", meshes.meshes_3d)
def test_cf(name, mesh):
    from ngsolve import x,y,z

    settings = getParameters()
    settings.individualClippingPlane = True
    settings.setClippingNormal([0,0,1])
    settings.zoom=-100

    s = SolutionScene(z+x*x-0.3*y*y, mesh, iso_surface=x+2*y+z*z)
    s._global_rendering_parameters = settings
    Draw(s, name=name, tab=name+'_cf')
    # appy custom rendering settings (also because there are not global settings in headless mode)

    s.setOrder(3)
    s.setSubdivision(3)
    s.setShowSurface(True)
    s.setShowClippingPlane(True)

    gui.checkImage(s, 'cf_'+name)

    if name == 'tet':
        s.setShowSurface(False)
        s.setShowClippingPlane(False)
        s.setShowIsoSurface(True)
        s.setIsoValue(0.7)
        gui.checkImage(s, 'iso_'+name)



@pytest.mark.parametrize("name,mesh", meshes.meshes_3d)
def test_mesh(name, mesh):
    s = MeshScene(mesh)

    # need to add color settings for non-gui testing
    colors = [255,0,0,255, 0,255,0,255, 0,0,255,255, 255,255,0,255, 255,0,255,255, 0,255,255,255]
    s.setMaterialColors(colors)
    s.setSurfaceColors(colors)
    s.setEdgeColors(colors)

    settings = getParameters()

    Draw(s, name=name, tab=name)
    s._global_rendering_parameters = settings
    # appy custom rendering settings (also because there are not global settings in headless mode)

    # set up scene settings
    s.setShowSurface(False)
    s.setShowWireframe(True)
    gui.checkImage(s, name+'_wireframe')

    s.setShowSurface(True)
    s.setShowWireframe(False)
    gui.checkImage(s, name+'_surface')

if __name__ == '__main__':
    for name,mesh in meshes.meshes_3d:
        #test_mesh(name, mesh)
        test_cf(name, mesh)
