from .thread import inmain_decorator
from qtpy import QtCore, QtWidgets

class PlotTab(QtWidgets.QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def isGLWindow(self):
        False

    @inmain_decorator(True)
    def plot(self, figure, **kwargs):
        from . import widgets as wid
        from matplotlib.backends.backend_qt5agg import (FigureCanvas, NavigationToolbar2QT as NavigationToolbar)
        canvas = FigureCanvas(figure)
        self.setLayout(wid.ArrangeV(canvas, NavigationToolbar(canvas, self)))
