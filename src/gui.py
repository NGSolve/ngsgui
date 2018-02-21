
from . import glwindow

import inspect

from PySide2 import QtWidgets

from jupyter_client.multikernelmanager import MultiKernelManager
from traitlets import DottedObjectName

class MultiQtKernelManager(MultiKernelManager):
    kernel_manager_class = DottedObjectName("qtconsole.inprocess.QtInProcessKernelManager",
                                            config = True,
                                            help = """kernel manager class""")

class GUI():
    def __init__(self):
        self.windows = []
        self.app = QtWidgets.QApplication([])
        self.kernel_manager = MultiQtKernelManager()

    def make_window(self, console=True):
        if len(self.windows):
            shared = self.windows[0].glWidget
        else:
            shared = None
        window = glwindow.MainWindow(multikernel_manager=self.kernel_manager,
                                     console = console,shared=shared)
        window.show()
        window.raise_()
        self.windows.append(window)
        return window

    def getWindow(self,index=-1):
        return self.windows[index]

    def draw(self, *args, **kwargs):
        if not len(self.windows):
            self.make_window()
        self.windows[0].draw(*args, **kwargs)

    def redraw(self, blocking=True):
        for win in self.windows:
            win.redraw(blocking=blocking)


    def run(self):
        for win in self.windows:
            globs = inspect.stack()[1][0].f_globals
            self.kernel_manager.get_kernel(win.kernel_id).kernel.shell.push(globs)
        res = self.app.exec_()
        for window in self.windows:
            window.glWidget.freeResources()
