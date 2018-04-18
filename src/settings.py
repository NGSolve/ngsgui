
from . import widgets as wid
from . import scenes
from .thread import inthread, inmain_decorator

from .widgets import ArrangeH, ArrangeV

from PySide2 import QtWidgets, QtGui

import ngsolve as ngs

class Settings():
    def __init__(self, gui):
        self.name = "Settings"
        self.gui = gui


    def getQtWidget(self):
        self.widgets = wid.OptionWidgets()
        return self.widgets

    def __getstate__(self):
        return (self.name, self.meshes, self.active_mesh)

    def __setstate__(self, state):
        self.name, self.meshes, self.active_mesh = state
        # coefficient functions not yet picklable
        self.solutions = []
