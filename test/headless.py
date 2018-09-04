import os
os.environ['PYOPENGL_PLATFORM']='egl'

import numpy as np
import OpenGL.GL as GL
import OpenGL.EGL as EGL

from ngsgui.settings import BaseSettings
BaseSettings.__init__ = lambda self: self
import ngsgui, glob
import ngsgui.gl as gl
for shaderpath in ngsgui.shader.locations:
    for incfile in glob.glob(os.path.join(shaderpath, '*.inc')):
        gl.Shader.includes[os.path.basename(incfile)] = open(incfile,'r').read()

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
          EGL.EGL_BLUE_SIZE, 8,
          EGL.EGL_GREEN_SIZE, 8,
          EGL.EGL_RED_SIZE, 8,
          EGL.EGL_DEPTH_SIZE, 8,
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
        self.clear()

    def clear(self):
        GL.glClearColor(1.0, 1.0, 1.0, 0.0)
        GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)

    def check_image(self, name):
        import PIL.Image as im
        data = GL.glReadPixels(0, 0, self.width, self.height, GL.GL_RGB, GL.GL_UNSIGNED_BYTE, outputType=None)
        out_name = 'images/{}_out.png'.format(name)
        ref_name = 'images/{}_ref.png'.format(name)
        diff_name = 'images/{}_diff.png'.format(name)
        out_image = im.fromarray(data)
        out_image.save(out_name)
        if not os.path.exists(ref_name):
            print('warning, found no reference image, store current image as reference image')
            out_image.save(ref_name)
        ref = np.array(im.open(ref_name))
        out = np.array(im.open(out_name))
        diff = np.zeros(ref.shape, dtype=np.int16)
        diff [:] = ref
        diff -= out
        diff_image = im.fromarray(np.array(255-diff, dtype=np.uint8))
        diff_image.save(diff_name)
        assert diff.any() == False
