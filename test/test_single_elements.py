from headless import HeadlessGUI as Gui
from ngsolve import *
ngsglobals.msg_level = 0
import meshes
import ngsgui.gl as gl
from ngsgui.glwindow import RenderingParameters
from ngsgui.scenes import MeshScene
import OpenGL.GL as GL

gui = Gui()

def mesh_test(mesh, name):
    gui.clear()

    settings = RenderingParameters()
    s = MeshScene(mesh)
    colors = [0,255,0,255]*100
    s.getMaterialColors = lambda : colors
    s.getSurfaceColors = lambda : colors
    s.getEdgeColors = lambda : colors
    s.initGL()
    s.update()

    for els in s.mesh_data.elements[BND]:
        with gl.Query(GL.GL_PRIMITIVES_GENERATED) as q:
            s._render2DElements(settings, els, True);
#         assert q.value == els.nelements*els.n_instances_2d (currently failing for pyramid due to 'curved' quad

    GL.glFinish()
    gui.check_image(name)

def test_tet():
    mesh_test(meshes.Tet(), 'tet')

def test_pyramid():
    mesh_test(meshes.Pyramid(), 'pyramid')

if __name__ == '__main__':
    test_tet()
    test_pyramid()
