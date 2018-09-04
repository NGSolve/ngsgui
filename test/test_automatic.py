
from headless import HeadlessGUI as Gui
from ngsgui.settings import BaseSettings
from ngsolve import *
ngsglobals.msg_level = 0
import OpenGL.GL as GL

import pickle, os
# BaseSettings.__init__ = lambda self: self
BaseSettings.__setstate__ = lambda self, state: None

def loadFromFile(filename):
    with open(filename, "rb") as f:
        scene, renderingParameters, parameters = pickle.load(f)
    print("parameters = ", parameters)
    def newgetattr(self, name):
        funcname = name.replace("get","")
        if funcname in parameters:
            parameter = parameters[funcname]
            return lambda: parameter
        raise AttributeError
    BaseSettings.__getattr__ = newgetattr
    return scene, renderingParameters


def test_autotests():
    files = os.listdir("automatic_tests")
    gui = Gui()
    for filename in files:
        name = filename.replace(".test","")
        gui.clear()
        scene, settings = loadFromFile("automatic_tests/" + filename)
        scene.initGL()
        scene.update()
        scene.render(settings)
        GL.glFinish()
        gui.check_image(name)
