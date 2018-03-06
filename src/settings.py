
from . import widgets as wid
from . import scenes

from .widgets import ArrangeH, ArrangeV

from PySide2 import QtWidgets
from qtutils import inmain_decorator, inthread

import ngsolve as ngs

class Settings():
    def __init__(self, gui):
        self.name = "Settings"
        self.gui = gui
        self.toolboxupdate = lambda me: None
        self.active_mesh = None
        self.solutions = []
        self.meshes = []


    def getQtWidget(self):
        self.widgets = wid.OptionWidgets()
        self.comb_mesh = QtWidgets.QComboBox()
        btn_draw_mesh = QtWidgets.QPushButton("Draw Mesh")
        btn_draw_mesh.clicked.connect(lambda : self.drawMesh(self.comb_mesh.currentIndex(), self.gui.getActiveGLWindow()))
        self.comb_sol = QtWidgets.QComboBox()
        btn_draw_sol = QtWidgets.QPushButton("Draw Solution")
        btn_draw_sol.clicked.connect(lambda : self.drawSolution(self.comb_sol.currentIndex(), self.gui.getActiveGLWindow()))
        self.comb_active_mesh = QtWidgets.QComboBox()
        self.comb_active_mesh.activated.connect(lambda index: self.setActiveMesh(self.meshes[index][0]))
        self.widgets.addGroup("Drawing", ArrangeV(ArrangeH(self.comb_mesh, btn_draw_mesh),
                                                  ArrangeH(self.comb_sol, btn_draw_sol),
                                                  ArrangeH(QtWidgets.QLabel("On mesh:"), self.comb_active_mesh)))
        return self.widgets

    def __getstate__(self):
        return (self.name, self.meshes, self.active_mesh)

    def __setstate__(self, state):
        self.name, self.meshes, self.active_mesh = state
        # coefficient functions not yet picklable
        self.solutions = []

    def setActiveMesh(self,mesh):
        self.active_mesh = mesh

    def drawSolution(self, index, window):
        if self.solutions[index][1] is None:
            self.solutions[index] = (self.solutions[index][0],
                                     scenes.SolutionScene(self.solutions[index][0],self.active_mesh))
        window.draw(self.solutions[index][1])

    def drawMesh(self, index, window):
        if self.meshes[index][1] is None:
            self.meshes[index] = (self.meshes[index][0], scenes.MeshScene(self.meshes[index][0]))
        window.draw(self.meshes[index][1])

    def _tryGetName(self,obj, prefix, index):
        try:
            return obj.name
        except AttributeError:
            return prefix + str(index)

class PythonFileSettings(Settings):

    def __init__(self, name, editTab, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.editTab = editTab
        self.name = "Python File Settings: " + name
        self.active_mesh = None
        self.exec_locals = {}

    def __getstate__(self):
        return (super().__getstate__(),)

    def __setstate__(self, state):
        super().__setstate__(state[0])

    def getQtWidget(self):
        super().getQtWidget()
        btn_save = QtWidgets.QPushButton("Save")
        btn_save.clicked.connect(self.save)
        btn_run = QtWidgets.QPushButton("Run")
        btn_run.clicked.connect(self.run)
        btn_clear = QtWidgets.QPushButton("Clear")
        btn_clear.clicked.connect(self.clear)
        self.widgets.addGroup("Executing", ArrangeH(btn_save,btn_run,btn_clear),importance=5)
        return self.widgets

    def updateWidget(self):
        self.comb_sol.clear()
        self.comb_mesh.clear()
        self.solutions = []
        self.meshes = []
        for name, item in self.exec_locals.items():
            if isinstance(item, ngs.CoefficientFunction):
                item.name = name
                self.solutions.append((item,None))
            if isinstance(item,ngs.Mesh):
                item.name =  name
                self.meshes.append((item,None))
        self.comb_sol.addItems([self._tryGetName(sol[0], "solution", i+1) for i,sol in enumerate(self.solutions)])
        meshnames = [self._tryGetName(msh[0], "mesh", i+1) for i,msh in enumerate(self.meshes)]
        self.comb_mesh.addItems(meshnames)
        self.comb_active_mesh.addItems(meshnames)
        if self.meshes:
            self.active_mesh = self.meshes[-1][0]

    def save(self):
        self.editTab.save()

    def run(self):
        self.save()
        inthread(self.editTab.run(self.exec_locals))
        self.gui.console.pushVariables(self.exec_locals)
        self.updateWidget()

    def clear(self):
        self.exec_locals = {}
