
from . import widgets as wid

from PySide2 import QtWidgets

class Settings():
    def __init__(self, gui):
        self.name = "Settings"
        self.gui = gui
        self.toolboxupdate = lambda me: None

    def getQtWidget(self):
        self.widgets = wid.OptionWidgets()
        self.comb_mesh = QtWidgets.QComboBox()
        btn_draw_mesh = QtWidgets.QPushButton("Draw Mesh")
        btn_draw_mesh.clicked.connect(lambda : self.drawMesh(self.comb_mesh.currentIndex(), self.gui.getActiveWindow()))
        self.comb_sol = QtWidgets.QComboBox()
        btn_draw_sol = QtWidgets.QPushButton("Draw Solution")
        btn_draw_sol.clicked.connect(lambda : self.drawSolution(self.comb_sol.currentIndex(), self.gui.getActiveWindow()))
        self.widgets.addGroup("Drawing", ArrangeV(ArrangeH(self.comb_mesh, btn_draw_mesh),
                                                  ArrangeH(self.comb_sol, btn_draw_sol)))
        return self.widgets

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

    def __init__(self, namespace, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.name = "Python File Settings"
        self.solutions = []
        self.meshes = []
        for name, item in namespace:
            if isinstance(item, ngs.CoefficientFunction):
                item.name = name
                self.solutions.append((item,None))
            if isinstance(item,ngs.Mesh):
                item.name =  name
                self.meshes.append((item,None))

    def getQtWidget(self):
        super().getQtWidget()
        self.comb_sol.addItems([self._tryGetName(sol[0], "solution", i+1) for i,sol in enumerate(self.solutions)])
        self.comb_mesh.addItems([self._tryGetName(msh[0], "mesh", i+1) for i,msh in enumerate(self.meshes)])
        return self.widgets
