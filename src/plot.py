
from .thread import inmain_decorator
from . import widgets as wid
from matplotlib.backends.backend_qt5agg import (FigureCanvas, NavigationToolbar2QT as NavigationToolbar)
from matplotlib.figure import Figure

from PySide2 import QtCore, QtWidgets

class PlotTab(QtWidgets.QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def isGLWindow(self):
        False

    @inmain_decorator(True)
    def plot(self, figure, **kwargs):
        canvas = FigureCanvas(figure)
        self.setLayout(wid.ArrangeV(canvas, NavigationToolbar(canvas, self)))

    # @inmain_decorator(True)
    # def plot(self, x, y):
    #     canvas = FigureCanvas(Figure(figsize = (5,3)))
    #     self.setLayout(wid.ArrangeV(canvas, NavigationToolbar(canvas, self)))
    #     self._axes = canvas.figure.subplots()
    #     self._axes.plot(x,y)
