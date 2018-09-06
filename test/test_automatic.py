
from headless import HeadlessGUI as Gui
from ngsgui.settings import BaseSettings
from ngsolve import *
ngsglobals.msg_level = 0
import OpenGL.GL as GL
import glob

import pickle, os

def runSceneTest(gui, name, scene, settings):
    print("*******************************************************")
    print("test scene ", scene.name)
    gui.clear()
    scene.update()
    scene.render(settings)
    GL.glFinish()
    gui.check_image(name + "_" + scene.name)

def runFileTest(gui, filename):
    parameters = {}
    def newSetstate(self, state):
        parameters[self] = state[0]
    def newgetattr(self, name):
        funcname = name.replace("get","")
        if funcname in parameters[self]:
            return lambda: parameters[self][funcname]
        raise AttributeError
    BaseSettings.__setstate__ = newSetstate
    BaseSettings.__getattr__ = newgetattr
    with open("automatic_tests/" + filename, "rb") as f:
        tabs = pickle.load(f)
    for (scenes, settings), name in tabs:
        for scene in scenes:
            runSceneTest(gui, filename.replace(".test",""), scene, settings)


def test_autotests():
    files = glob.glob('automatic_tests/*.test')
    gui = Gui()
    for filename in files:
        print("################################################################")
        print("test file ", filename)
        runFileTest(gui, filename)

if __name__ == "__main__":
    test_autotests()
