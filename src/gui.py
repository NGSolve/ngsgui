
from . import glwindow
from . widgets import ArrangeV

import inspect
import time

from PySide2 import QtWidgets, QtCore, QtGui

from jupyter_client.multikernelmanager import MultiKernelManager
from qtconsole.inprocess import QtInProcessRichJupyterWidget
from traitlets import DottedObjectName

class MultiQtKernelManager(MultiKernelManager):
    kernel_manager_class = DottedObjectName("qtconsole.inprocess.QtInProcessKernelManager",
                                            config = True,
                                            help = """kernel manager class""")

class MenuBarWithDict(QtWidgets.QMenuBar):
    def __init__(self,*args, **kwargs):
        super().__init__(*args,**kwargs)
        self._dict = {}

    def addMenu(self, name, *args, **kwargs):
        menu = MenuWithDict(super().addMenu(name,*args,**kwargs))
        self._dict[name] = menu
        return menu

    def __getitem__(self, index):
        return self._dict[index]

class MenuWithDict(QtWidgets.QMenu):
    def __new__(self, menu):
        return menu

    def __init__(self,menu):
        self._dict = {}

    def addMenu(self, name, *args, **kwargs):
        menu = MenuWithDict(super().addMenu(name,*args,**kwargs))
        self._dict["name"] = menu
        return menu

    def __getitem__(self, index):
        return self._dict[index]

class MainWindow(QtWidgets.QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.last = time.time()

    def redraw(self, blocking = True):
        if time.time() - self.last < 0.02:
            return
        for window in self.windows:
            if blocking:
                self.glWidget.redraw_mutex.lock()
                self.glWidget.redraw_signal.emit()
                self.glWidget.redraw_update_done.wait(self.glWidget.redraw_mutex)
                self.glWidget.redraw_mutex.unlock()
            else:
                self.glWidget.redraw_signal.emit()
        self.last = time.time()

    def keyPressEvent(self, event):
        if event.key() == 16777216:
            self.close()

class SettingsToolBox(QtWidgets.QToolBox):
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.settings = []

    def addSettings(self, sett):
        self.settings.append(sett)
        widget = QtWidgets.QWidget()
        widget.setLayout(ArrangeV(*sett.getQtWidget().groups))
        widget.layout().setAlignment(QtCore.Qt.AlignTop)
        self.addItem(widget, sett.name)
        self.setCurrentIndex(len(self.settings)-1)


import ngsolve
class GUI():
    def __init__(self):
        self.windows = []
        self.app = QtWidgets.QApplication([])
        self.multikernel_manager = MultiQtKernelManager()
        self.mainWidget = MainWindow()
        self.menuBar = MenuBarWithDict()
        fileMenu = self.menuBar.addMenu("&File")
        loadMenu = fileMenu.addMenu("&Load")
        def selectPythonFile():
            filename, filt = QtWidgets.QFileDialog.getOpenFileName(caption = "Load Python File",
                                                                   filter = "Python files (*.py)")
            if filename:
                self.loadPythonFile(filename)
        loadPython = loadMenu.addAction("&Python File", shortcut = "l+y")
        loadPython.triggered.connect(selectPythonFile)
        createMenu = self.menuBar.addMenu("&Create")
        newWindowAction = createMenu.addAction("New &Window")
        newWindowAction.triggered.connect(self.make_window)

        self.kernel_id = self.multikernel_manager.start_kernel()
        kernel_manager = self.multikernel_manager.get_kernel(self.kernel_id)
        class dummyioloop():
            def call_later(self,a,b):
                return
            def stop(self):
                return
        kernel_manager.kernel.io_loop = dummyioloop()
        kernel_client = kernel_manager.client()
        kernel_client.start_channels()
        menu_splitter = QtWidgets.QSplitter(parent=self.mainWidget)
        menu_splitter.setOrientation(QtCore.Qt.Vertical)
        menu_splitter.addWidget(self.menuBar)
        toolbox_splitter = QtWidgets.QSplitter(parent=menu_splitter)
        menu_splitter.addWidget(toolbox_splitter)
        toolbox_splitter.setOrientation(QtCore.Qt.Horizontal)
        self.settings_toolbox = SettingsToolBox(parent=toolbox_splitter)
        toolbox_splitter.addWidget(self.settings_toolbox)
        window_splitter = QtWidgets.QSplitter(parent=toolbox_splitter)
        toolbox_splitter.addWidget(window_splitter)
        window_splitter.setOrientation(QtCore.Qt.Vertical)
        self.window_tabber = QtWidgets.QTabWidget(parent=window_splitter)
        window_splitter.addWidget(self.window_tabber)
        self.console = QtInProcessRichJupyterWidget()
        self.console.banner = """NGSolve %s
Developed by Joachim Schoeberl at
2010-xxxx Vienna University of Technology
2006-2010 RWTH Aachen University
1996-2006 Johannes Kepler University Linz

""" % ngsolve.__version__
        window_splitter.addWidget(self.console)
        self.console.kernel_manager = kernel_manager
        self.console.kernel_client = kernel_client
        self.console.exit_requested.connect(lambda c: self.app.exit())
        menu_splitter.setSizes([100, 10000])
        toolbox_splitter.setSizes([15000, 85000])
        window_splitter.setSizes([70000, 30000])
        self.mainWidget.setLayout(ArrangeV(menu_splitter))
        menu_splitter.show()
        self.console.show()
        self.mainWidget.setWindowTitle("NGSolve")

        # crawl for plugins
        try:
            from . import plugins as plu
            plugins_exist = True
        except ImportError:
            plugins_exist = False
        if plugins_exist:
            import pkgutil
            prefix = plu.__name__ + "."
            plugins = []
            for importer, modname, ispkg in pkgutil.iter_modules(plu.__path__,prefix):
                plugins.append(__import__(modname, fromlist="dummy"))
            from .plugin import GuiPlugin
            for plugin in plugins:
                for val in plugin.__dict__.values():
                    if inspect.isclass(val):
                        if issubclass(val, GuiPlugin):
                            val.loadPlugin(self)

    def make_window(self):
        if len(self.windows):
            shared = self.windows[0].glWidget
        else:
            shared = None
        window = glwindow.WindowTab(multikernel_manager=self.multikernel_manager,
                                    shared=shared)
        self.window_tabber.addTab(window,"window" + str(len(self.windows)+1))
        self.window_tabber.setCurrentWidget(window)
        window.show()
        self.windows.append(window)
        return window

    def getActiveWindow(self):
        if not self.window_tabber.count():
            self.make_window()
        return self.window_tabber.currentWidget()

    def getWindow(self,index=-1):
        return self.windows[index]

    def draw(self, *args, **kwargs):
        if not len(self.windows):
            self.make_window()
        self.windows[0].draw(*args, **kwargs)

    def redraw(self, blocking=True):
        for win in self.windows:
            win.redraw(blocking=blocking)

    def loadPythonFile(self, filename):
        txt = ""
        with open(filename, "r") as f:
            for line in f.readlines():
                txt += line
                if line[-1] != "\\":
                    line += "\n"

        self.console.execute(source=txt, hidden=True, interactive=True)

    def run(self,filename = None):
        import os, threading
        self.mainWidget.show()
        globs = inspect.stack()[1][0].f_globals
        self.multikernel_manager.get_kernel(self.kernel_id).kernel.shell.push(globs)
        if filename:
            name, ext = os.path.splitext(filename)
            if ext == ".py":
                self.loadPythonFile(filename)
            else:
                print("Cannot load file type: ", ext)
        res = self.app.exec_()
        for window in self.windows:
            window.glWidget.freeResources()
        self.multikernel_manager.shutdown_kernel(self.kernel_id)

