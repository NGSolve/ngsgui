import sys, os
if sys.platform=='linux':
    # headless operation possible
    os.environ['PYOPENGL_PLATFORM']='egl'
    os.environ['NGSGUI_HEADLESS']='1'
    _headless = True
    import OpenGL.GL as GL
    import OpenGL.EGL as EGL
else:
    # no headless operation possible on other systems, mimic same interface with usual gui
    # this way we can run tests 'manually' on Windows/MacOS
    _headless = False
    import OpenGL.GL as GL

import numpy as np

from ngsgui.settings import Parameter, BaseSettings
from ngsgui.thread import inmain_decorator

import ngsgui, glob
import ngsgui.gl as gl
for shaderpath in ngsgui.shader.locations:
    for incfile in glob.glob(os.path.join(shaderpath, '*.inc')):
        gl.Shader.includes[os.path.basename(incfile)] = open(incfile,'r').read()

if _headless:
    class HeadlessGUI:
        def __init__(self, width=400, height=400):
            self.width = width
            self.height = height
            from OpenGL import EGL
            self.EGL = EGL
            self.display = EGL.eglGetDisplay(EGL.EGL_NO_DISPLAY)
            major = np.zeros(1, "i4")
            minor = np.zeros(1, "i4")
            EGL.eglInitialize(self.display, major, minor)
            num_configs = np.zeros(1, "i4")
            configs = (EGL.EGLConfig*1)()
            # Now we create our necessary bits.
            config_attribs = np.array([
              EGL.EGL_SURFACE_TYPE, EGL.EGL_PBUFFER_BIT,
              EGL.EGL_ALPHA_SIZE, 8,
              EGL.EGL_BLUE_SIZE, 8,
              EGL.EGL_GREEN_SIZE, 8,
              EGL.EGL_RED_SIZE, 8,
              EGL.EGL_DEPTH_SIZE, 24,
              EGL.EGL_RENDERABLE_TYPE,
              EGL.EGL_OPENGL_BIT,
              EGL.EGL_NONE,
            ], dtype="i4")
            EGL.eglChooseConfig(self.display, config_attribs, configs, 1, num_configs)
            self.config = configs[0]

            pbuffer_attribs = np.array([
              EGL.EGL_WIDTH, width,
              EGL.EGL_HEIGHT, height,
              EGL.EGL_NONE
            ], dtype="i4")
            self.surface = EGL.eglCreatePbufferSurface(self.display, self.config, pbuffer_attribs)

            EGL.eglBindAPI(EGL.EGL_OPENGL_API)
            
            self.context = EGL.eglCreateContext(self.display, self.config, EGL.EGL_NO_CONTEXT, None)

            EGL.eglMakeCurrent(self.display, self.surface, self.surface, self.context)
            GL.glEnable(GL.GL_DEPTH_TEST)
            self.clear()

        def clear(self):
            GL.glClearColor(1.0, 1.0, 1.0, 0.0)
            GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)

        def renderToImage(self, width, height, filename=None):
            # ignore width and height, use numbers specified in ctor
            GL.glFinish()
            import PIL.Image as im
            data = GL.glReadPixels(0, 0, self.width, self.height, GL.GL_RGBA, GL.GL_UNSIGNED_BYTE, outputType=None)
            image = im.fromarray(data[::-1,:,:])
            if filename:
                image.save(filename)
            return image

else:
    # no headless gui
    class HeadlessGUI:
        def __init__(self, width=400, height=400):
            self.width = width
            self.height = height

        @inmain_decorator(wait_for_return=True)
        def clear(self):
            return
            GL.glClearColor(1.0, 1.0, 1.0, 0.0)
            GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)

        def renderToImage(self, width, height, filename=None):
            from ngsgui.gui import gui
            return gui.renderToImage( self.width, self.height, filename)


@inmain_decorator(wait_for_return=True)
def _checkImage(self,scene, name):
    import PIL.Image as im
    path = os.path.split('images/{}_out.png'.format(name))[0]
    if not os.path.exists(path):
        os.makedirs(path)
    out_name = 'images/{}_out.png'.format(name)
    ref_name = 'images/{}_ref.png'.format(name)
    diff_name = 'images/{}_diff.png'.format(name)
    if _headless:
        self.clear()
        scene.render(scene._global_rendering_parameters)

    out_image = self.renderToImage(0,0, out_name)
    if not os.path.exists(ref_name):
        print('warning, found no reference image, store current image as reference image')
        out_image.save(ref_name)
    if os.path.exists(diff_name):
        os.remove(diff_name)
    ref = np.array(im.open(ref_name))
    out = np.array(im.open(out_name))
    diff = np.zeros(ref.shape, dtype=np.int16)
    diff [:] = ref
    diff -= out
    error = diff.any()
    w,h,d = diff.shape
    errsum = sum(sum(sum(abs(diff))))/(w*h*d)
    if error:
        diff = np.array(255-abs(diff), dtype=np.uint8)
        diff[:,:,3] = 255
        diff_image = im.fromarray(diff)
        diff_image.save(diff_name)
    assert errsum<0.001
    if error:
        print("warning: small error ({}) discovered in: ".format(errsum)+diff_name)

HeadlessGUI.checkImage = _checkImage

def Draw(scene, *args, **kwargs):
    if _headless:
        scene.initGL()
        scene.update()
        return scene
    else:
        import ngsolve
        ngsolve.Draw(scene, *args, **kwargs)
        ngsolve.Redraw(blocking=True)


