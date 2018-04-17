
from . import glwindow
from . import code_editor
from . widgets import ArrangeV
from . settings import PythonFileSettings
from .thread import inthread, inmain_decorator
import ngui

import inspect
import time

from PySide2 import QtWidgets, QtCore, QtGui

from jupyter_client.multikernelmanager import MultiKernelManager
from qtconsole.inprocess import QtInProcessRichJupyterWidget
from traitlets import DottedObjectName

import os
os.environ['Qt_API'] = 'pyside2'
from IPython.lib import guisupport

class MultiQtKernelManager(MultiKernelManager):
    kernel_manager_class = DottedObjectName("qtconsole.inprocess.QtInProcessKernelManager",
                                            config = True,
                                            help = """kernel manager class""")

class MenuBarWithDict(QtWidgets.QMenuBar):
    def __init__(self,*args, **kwargs):
        super().__init__(*args,**kwargs)
        self._dict = {}

    @inmain_decorator(wait_for_return=True)
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

    @inmain_decorator(wait_for_return=True)
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

    @inmain_decorator(wait_for_return=True)
    def redraw(self, blocking = True):
        if time.time() - self.last < 0.02:
            return
        for window in (self.window_tabber.widget(index) for index in range(self.window_tabber.count())):
            if window.isGLWindow():
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

    @inmain_decorator(wait_for_return=False)
    def addSettings(self, sett):
        self.settings.append(sett)
        widget = QtWidgets.QWidget()
        widget.setLayout(ArrangeV(*sett.getQtWidget().groups))
        widget.layout().setAlignment(QtCore.Qt.AlignTop)
        self.addItem(widget, sett.name)
        self.setCurrentIndex(len(self.settings)-1)

class NGSJupyterWidget(QtInProcessRichJupyterWidget):
    def __init__(self, multikernel_manager,*args, **kwargs):
        super().__init__(*args,**kwargs)
        self.banner = """NGSolve %s
Developed by Joachim Schoeberl at
2010-xxxx Vienna University of Technology
2006-2010 RWTH Aachen University
1996-2006 Johannes Kepler University Linz

""" % ngsolve.__version__
        if multikernel_manager is not None:
            self.kernel_id = multikernel_manager.start_kernel()
            self.kernel_manager = multikernel_manager.get_kernel(self.kernel_id)
        else:
            self.kernel_manager = QtInProcessKernelManager()
            self.kernel_manager.start_kernel()
        self.kernel_manager.kernel.gui = 'qt'
        self.kernel_client = self.kernel_manager.client()
        self.kernel_client.start_channels()
        class dummyioloop():
            def call_later(self,a,b):
                return
            def stop(self):
                return
        self.kernel_manager.kernel.io_loop = dummyioloop()

        def stop():
            self.kernel_client.stop_channels()
            self.kernel_manager.shutdown_kernel()
            # this function is qt5 compatible as well
            guisupport.get_app_qt4().exit()
        self.exit_requested.connect(stop)

    @inmain_decorator(wait_for_return=True)
    def pushVariables(self, varDict):
        self.kernel_manager.kernel.shell.push(varDict)

    @inmain_decorator(wait_for_return=True)
    def clearTerminal(self):
        self._control.clear()

import ngsolve
class GUI():
    def __init__(self):
        self.app = QtWidgets.QApplication([])
        ngui.SetLocale()
        self.common_context = None
        self.multikernel_manager = MultiQtKernelManager()
        self.mainWidget = MainWindow()
        self.menuBar = MenuBarWithDict()
        self.activeGLWindow = None
        fileMenu = self.menuBar.addMenu("&File")
        loadMenu = fileMenu.addMenu("&Load")
        saveMenu = fileMenu.addMenu("&Save")
        saveSolution = saveMenu.addAction("&Solution")
        loadSolution = loadMenu.addAction("&Solution")
        loadSolution.triggered.connect(self.loadSolution)
        saveSolution.triggered.connect(self.saveSolution)
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
        self.window_tabber.setTabsClosable(True)
        def _remove_tab(index):
            if self.window_tabber.widget(index).isGLWindow():
                if self.common_context == self.window_tabber.widget(index).glWidget:
                    # cannot delete window with openGL context
                    return
                if self.activeGLWindow == self.window_tabber.widget(index):
                    self.activeGLWindow = None
            self.window_tabber.removeTab(index)
        self.window_tabber.tabCloseRequested.connect(_remove_tab)
        window_splitter.addWidget(self.window_tabber)
        self.console = NGSJupyterWidget(multikernel_manager = self.multikernel_manager)
        window_splitter.addWidget(self.console)
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

    @inmain_decorator(wait_for_return=True)
    def make_window(self):
        self.activeGLWindow = window = glwindow.WindowTab(shared=self.common_context)
        if self.common_context is None:
            self.common_context = window.glWidget
        self.window_tabber.addTab(window,"window" + str(self.window_tabber.count() + 1))
        self.window_tabber.setCurrentWidget(window)
        return window

    def saveSolution(self):
        import pickle
        filename, filt = QtWidgets.QFileDialog.getSaveFileName(caption="Save Solution",
                                                               filter = "Solution Files (*.sol)")
        if not filename[-4:] == ".sol":
            filename += ".sol"
        tabs = []
        for i in range(self.window_tabber.count()):
            tabs.append(self.window_tabber.widget(i))
        settings = self.settings_toolbox.settings
        with open(filename,"wb") as f:
            pickle.dump((tabs,settings), f)

    def loadSolution(self):
        import pickle
        filename, filt = QtWidgets.QFileDialog.getOpenFileName(caption="Load Solution",
                                                               filter = "Solution Files (*.sol)")
        if not filename[-4:] == ".sol":
            filename += ".sol"
        with open(filename, "rb") as f:
            tabs, settings = pickle.load(f)
        for tab in tabs:
            self.window_tabber.addTab(tab, "window" + str(self.window_tabber.count()))
            tab.show()
            self.window_tabber.setCurrentWidget(tab)
        for setting in settings:
            setting.gui = self
            self.settings_toolbox.addSettings(setting)

    # def getActiveWindow(self):
    #     if not self.window_tabber.count():
    #         self.make_window()
    #     return self.window_tabber.currentWidget()

    def getActiveGLWindow(self):
        if self.activeGLWindow is None:
            self.make_window()
        return self.activeGLWindow

    @inmain_decorator(wait_for_return=True)
    def draw(self, *args, **kwargs):
        self.getActiveGLWindow().draw(*args,**kwargs)

    @inmain_decorator(wait_for_return=True)
    def redraw(self, blocking=True):
        for index in range(self.window_tabber.count()):
            win = self.window_tabber.widget(index)
            if win.isGLWindow():
                win.redraw(blocking=blocking)

    @inmain_decorator(wait_for_return=True)
    def _loadFile(self, filename):
        txt = ""
        with open(filename,"r") as f:
            for line in f.readlines():
                txt += line
        return txt

    def loadPythonFile(self, filename, execute = False):
        editTab = code_editor.CodeEditor(filename=filename,parent=self.window_tabber)
        pos = self.window_tabber.addTab(editTab,filename)
        editTab.windowTitleChanged.connect(lambda txt: self.window_tabber.setTabText(pos, txt))
        setting = PythonFileSettings(gui=self, name = filename, editTab = editTab)
        self.settings_toolbox.addSettings(setting)
        if execute:
            setting.computation_started_at = 0
            setting.run(editTab.text)

    def run(self,do_after_run=lambda : None):
        import os, threading
        self.mainWidget.show()
        globs = inspect.stack()[1][0].f_globals
        self.console.pushVariables(globs)
        do_after_run()
        self.app.exec_()

class DummyObject:
    def __init__(self,*arg,**kwargs):
        pass
    def __getattr__(self,name):
        return DummyObject()
    def __call__(self,*args,**kwargs):
        pass

gui = DummyObject()
