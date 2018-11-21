
from headless import HeadlessGUI as Gui
from ngsgui.settings import BaseSettings
from ngsgui.widgets import ObjectHolder
from ngsolve import *
ngsglobals.msg_level = 0
import OpenGL.GL as GL
import glob
from qtpy import QtCore
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
        QtCore.QObject.__init__(self)
        parameters[self] = state[0]
    BaseSettings.__setstate__ = newSetstate
    with open(filename, "rb") as f:
        tabs = pickle.load(f)
    def call_func(self):
        return self.obj
    for scenes, settings, name in tabs:
        for scene in scenes:
            for par,val in parameters[scene].items():
                holder = ObjectHolder(val, call_func)
                setattr(scene,"get" + par, holder)
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
