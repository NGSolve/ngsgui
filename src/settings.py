
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
            return obj._name
        except AttributeError:
            return prefix + str(index)

class PythonFileSettings(Settings):

    def __init__(self, name, editTab, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.editTab = editTab
        editTab.settings = self
        self.name = "Python File Settings: " + name
        self.active_mesh = None
        self.exec_locals = {}
        self.active_thread = None

    def __getstate__(self):
        return (super().__getstate__(),)

    def __setstate__(self, state):
        super().__setstate__(state[0])

    def getQtWidget(self):
        super().getQtWidget()
        btn_save = QtWidgets.QPushButton("Save")
        btn_save.clicked.connect(self.save)
        btn_run = QtWidgets.QPushButton("Run")
        def _run():
            self.computation_started_at = 0
            self.run(self.editTab.toPlainText())
        btn_run.clicked.connect(_run)
        btn_stop = QtWidgets.QPushButton("Stop")
        btn_stop.clicked.connect(self.stop)
        btn_clear = QtWidgets.QPushButton("Clear")
        btn_clear.clicked.connect(self.clear)
        self.widgets.addGroup("Executing", ArrangeV(ArrangeH(btn_save,btn_run),
                                                    ArrangeH(btn_stop,btn_clear)),
                              importance=5)
        return self.widgets

    @inmain_decorator(wait_for_return=False)
    def updateWidget(self):
        self.comb_sol.clear()
        self.comb_mesh.clear()
        self.comb_active_mesh.clear()
        self.solutions = []
        self.meshes = []
        for name, item in self.exec_locals.items():
            if isinstance(item, ngs.CoefficientFunction):
                item._name = name
                self.solutions.append((item,None))
            if isinstance(item,ngs.Mesh):
                item._name =  name
                self.meshes.append((item,None))
        self.comb_sol.addItems([self._tryGetName(sol[0], "solution", i+1) for i,sol in enumerate(self.solutions)])
        meshnames = [self._tryGetName(msh[0], "mesh", i+1) for i,msh in enumerate(self.meshes)]
        self.comb_mesh.addItems(meshnames)
        self.comb_active_mesh.addItems(meshnames)
        if self.meshes:
            self.active_mesh = self.meshes[-1][0]

    @inmain_decorator(wait_for_return=True)
    def save(self):
        self.editTab.save()

    @inmain_decorator(wait_for_return=False)
    def show_exception(self,e, lineno):
        self.editTab.setTextCursor(QtGui.QTextCursor(self.editTab.document().findBlock(self.computation_started_at)))
        for i in range(lineno-1):
            self.editTab.moveCursor(QtGui.QTextCursor.Down)
        self.msgbox = QtWidgets.QMessageBox(text = type(e).__name__ + ": " + str(e))
        self.msgbox.setWindowTitle("Exception caught!")
        self.msgbox.show()

    def run(self, code):
        def _run(_code):
            try:
                self.editTab.run(_code,self.exec_locals)
            except Exception as e:
                import sys
                tb = sys.exc_info()[2].tb_next.tb_next
                self.show_exception(e, tb.tb_frame.f_lineno)
            self.gui.console.pushVariables(self.exec_locals)
            self.updateWidget()
        def run_and_reset():
            _run(code)
            self.active_thread = None
        if self.active_thread:
                self.msgbox = QtWidgets.QMessageBox(text="Already running, please stop the other computation before starting a new one!")
                self.msgbox.setWindowTitle("Multiple computations error")
                self.msgbox.show()
                return
        self.active_thread = inthread(run_and_reset)

    def clear(self):
        self.exec_locals = {}
        self.updateWidget()

    def stop(self):
        if self.active_thread:
            self.active_thread.raiseExc(KeyboardInterrupt)
            self.gui.console.pushVariables(self.exec_locals)
            self.updateWidget()
