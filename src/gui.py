
import os
os.environ['Qt_API'] = 'pyside2'

from . import glwindow
from . import code_editor
from . widgets import ArrangeV
from .thread import inthread, inmain_decorator
from .menu import MenuBarWithDict
from .console import NGSJupyterWidget, MultiQtKernelManager

import sys, textwrap, inspect, re, pkgutil, ngsolve, ngui, pickle

from PySide2 import QtWidgets, QtCore, QtGui

class Receiver(QtCore.QObject):
    received = QtCore.Signal(str)

    def __init__(self,pipe, *args,**kwargs):
        super().__init__(*args,**kwargs)
        self.pipe = pipe
        self.ansi_escape = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')
        self.kill = False

    def SetKill(self):
        self.kill = True
        print("killme")

    def run(self):
        while not self.kill:
            self.received.emit(self.ansi_escape.sub("",os.read(self.pipe,1024).decode("ascii")))
        self.kill = False

class OutputBuffer(QtWidgets.QTextEdit):
    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.setReadOnly(True)

    def append_text(self, text):
        self.moveCursor(QtGui.QTextCursor.End)
        self.insertPlainText(text)

class SettingsToolBox(QtWidgets.QToolBox):
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.settings = []

    @inmain_decorator(wait_for_return=False)
    def addSettings(self, sett):
        self.settings.append(sett)
        widget = QtWidgets.QWidget()
        widget.setLayout(ArrangeV(*sett.widgets.groups))
        widget.layout().setAlignment(QtCore.Qt.AlignTop)
        self.addItem(widget, sett.name)
        self.setCurrentIndex(len(self.settings)-1)


def _noexec(gui, val):
    gui.executeFileOnStartup = not val
def _fastmode(gui,val):
    gui.window_tabber._fastmode = val
def _noOutputpipe(gui,val):
    gui.pipeOutput = not val

def _showHelp(gui, val):
    if val:
        print("Available flags:")
        for flag, tup in gui.flags.items():
            print(flag)
            print(textwrap.indent(tup[1],"  "))
        quit()

def _dontCatchExceptions(gui, val):
    gui._dontCatchExceptions = val

class GUI():
    # functions to modify the gui with flags. If the flag is not set, the function is called with False as argument
    flags = { "-noexec" : (_noexec, "Do not execute loaded Python file on startup"),
              "-fastmode" : (_fastmode, "Use fastmode for drawing large scenes faster"),
              "-noOutputpipe" : (_noOutputpipe, "Do not pipe the std output to the output window in the gui"),
              "-help" : (_showHelp, "Show this help function"),
              "-dontCatchExceptions" : (_dontCatchExceptions, "Do not catch exceptions")}
    # use a list of tuples instead of a dict to be able to sort it
    sceneCreators = []
    def __init__(self):
        self.app = QtWidgets.QApplication([])
        ngui.SetLocale()
        self.multikernel_manager = MultiQtKernelManager()
        self._commonContext = glwindow.GLWidget()
        self.createMenu()
        self.createLayout()
        self.mainWidget.setWindowTitle("NGSolve")
        self.crawlPlugins()

    def createMenu(self):
        self.menuBar = MenuBarWithDict()
        filemenu = self.menuBar.addMenu("&File")
        saveSolution = filemenu["&Save"].addAction("&Solution")
        loadSolution = filemenu["&Load"].addAction("&Solution")
        loadSolution.triggered.connect(self.loadSolution)
        saveSolution.triggered.connect(self.saveSolution)
        def selectPythonFile():
            filename, filt = QtWidgets.QFileDialog.getOpenFileName(caption = "Load Python File",
                                                                   filter = "Python files (*.py)")
            if filename:
                self.loadPythonFile(filename)
        loadPython = filemenu["&Load"].addAction("&Python File", shortcut = "l+y")
        loadPython.triggered.connect(selectPythonFile)
        newWindowAction = self.menuBar["&Create"].addAction("New &Window")
        newWindowAction.triggered.connect(lambda :self.window_tabber.make_window())

    def createLayout(self):
        self.mainWidget = QtWidgets.QWidget()
        self.activeGLWindow = None
        menu_splitter = QtWidgets.QSplitter(parent=self.mainWidget)
        menu_splitter.setOrientation(QtCore.Qt.Vertical)
        menu_splitter.addWidget(self.menuBar)
        self.toolbox_splitter = toolbox_splitter = QtWidgets.QSplitter(parent=menu_splitter)
        menu_splitter.addWidget(toolbox_splitter)
        toolbox_splitter.setOrientation(QtCore.Qt.Horizontal)
        self.settings_toolbox = SettingsToolBox(parent=toolbox_splitter)
        toolbox_splitter.addWidget(self.settings_toolbox)
        window_splitter = QtWidgets.QSplitter(parent=toolbox_splitter)
        toolbox_splitter.addWidget(window_splitter)
        window_splitter.setOrientation(QtCore.Qt.Vertical)
        self.window_tabber = glwindow.WindowTabber(commonContext = self._commonContext,
                                                   parent=window_splitter)
        window_splitter.addWidget(self.window_tabber)
        self.console = NGSJupyterWidget(multikernel_manager = self.multikernel_manager)
        self.outputBuffer = OutputBuffer()
        self.output_tabber = QtWidgets.QTabWidget()
        self.output_tabber.addTab(self.console,"Console")
        self.output_tabber.addTab(self.outputBuffer, "Output")
        self.output_tabber.setCurrentIndex(1)
        window_splitter.addWidget(self.output_tabber)
        menu_splitter.setSizes([100, 10000])
        toolbox_splitter.setSizes([0, 85000])
        window_splitter.setSizes([70000, 30000])
        self.mainWidget.setLayout(ArrangeV(menu_splitter))

        # global shortkeys:
        def activateConsole():
            self.output_tabber.setCurrentWidget(self.console)
            self.console._control.setFocus()
        self.mainWidget._console_action = console_action = QtWidgets.QAction("Activate Console")
        console_action.triggered.connect(activateConsole)
        console_action.setShortcut(QtGui.QKeySequence("Ctrl+j"))
        self.mainWidget.addAction(console_action)

        def switchTabWindow(direction):
            self.window_tabber.setCurrentIndex((self.window_tabber.currentIndex() + direction)%self.window_tabber.count())
        self.mainWidget._next_tab_action = next_tab = QtWidgets.QAction("Next Tab")
        next_tab.triggered.connect(lambda : switchTabWindow(1))
        next_tab.setShortcut(QtGui.QKeySequence("Ctrl+w"))
        self.mainWidget._last_tab_action = last_tab = QtWidgets.QAction("Last Tab")
        last_tab.triggered.connect(lambda : switchTabWindow(-1))
        last_tab.setShortcut(QtGui.QKeySequence("Ctrl+q"))
        self.mainWidget.addAction(next_tab)
        self.mainWidget.addAction(last_tab)

    def crawlPlugins(self):
        try:
            from . import plugins as plu
            plugins_exist = True
        except ImportError:
            plugins_exist = False
        if plugins_exist:
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

    def parseFlags(self, flags):
        flag = {val.split("=")[0] : (val.split("=")[1] if len(val.split("="))>1 else True) for val in flags}
        for key, tup in self.flags.items():
            if key in flag:
                tup[0](self,flag[key])
            else:
                tup[0](self, False)

    @inmain_decorator(wait_for_return=False)
    def update_setting_area(self):
        if len(self.settings_toolbox.settings) == 0:
            self.toolbox_splitter.setSizes([0,85000])
        else:
            self.toolbox_splitter.setSizes([15000, 85000])

    def saveSolution(self):
        filename, filt = QtWidgets.QFileDialog.getSaveFileName(caption="Save Solution",
                                                               filter = "Solution Files (*.sol)")
        if not filename[-4:] == ".sol":
            filename += ".sol"
        tabs = []
        for i in range(self.window_tabber.count()):
            tabs.append((self.window_tabber.widget(i),self.window_tabber.tabBar().tabText(i)))
        settings = self.settings_toolbox.settings
        with open(filename,"wb") as f:
            pickle.dump((tabs,settings), f)

    def loadSolution(self):
        filename, filt = QtWidgets.QFileDialog.getOpenFileName(caption="Load Solution",
                                                               filter = "Solution Files (*.sol)")
        if not filename[-4:] == ".sol":
            filename += ".sol"
        with open(filename, "rb") as f:
            tabs,settings = pickle.load(f)
        for tab,name in tabs:
            if isinstance(tab, glwindow.WindowTab):
                tab.create(self._commonContext)
            if isinstance(tab, code_editor.CodeEditor):
                tab.gui = self
            self.window_tabber.addTab(tab, name)
            tab.show()
            self.window_tabber.setCurrentWidget(tab)
        for setting in settings:
            setting.gui = self
            self.settings_toolbox.addSettings(setting)

    @inmain_decorator(wait_for_return=True)
    def draw(self, *args, **kwargs):
        self.window_tabber.draw(*args,**kwargs)

    @inmain_decorator(wait_for_return=False)
    def redraw(self):
        self.window_tabber.activeGLWindow.glWidget.updateScenes()

    @inmain_decorator(wait_for_return=True)
    def redraw_blocking(self):
        self.window_tabber.activeGLWindow.glWidget.updateScenes()

    def plot(self, x,y):
        self.window_tabber.plot(x,y)

    @inmain_decorator(wait_for_return=True)
    def _loadFile(self, filename):
        txt = ""
        with open(filename,"r") as f:
            for line in f.readlines():
                txt += line
        return txt

    def loadPythonFile(self, filename):
        editTab = code_editor.CodeEditor(filename=filename,gui=self,parent=self.window_tabber)
        pos = self.window_tabber.addTab(editTab,filename)
        editTab.windowTitleChanged.connect(lambda txt: self.window_tabber.setTabText(pos, txt))
        if self.executeFileOnStartup:
            editTab.computation_started_at = 0
            editTab.run()

    def run(self,do_after_run=lambda : None):
        self.mainWidget.show()
        globs = inspect.stack()[1][0].f_globals
        self.console.pushVariables(globs)
        if self.pipeOutput:
            stdout_fileno = sys.stdout.fileno()
            stderr_fileno = sys.stderr.fileno()
            stderr_save = os.dup(stderr_fileno)
            stdout_save = os.dup(stdout_fileno)
            stdout_pipe = os.pipe()
            os.dup2(stdout_pipe[1], stdout_fileno)
            os.dup2(stdout_pipe[1], stderr_fileno)
            os.close(stdout_pipe[1])
            receiver = Receiver(stdout_pipe[0])
            receiver.received.connect(self.outputBuffer.append_text)
            self.stdoutThread = QtCore.QThread()
            receiver.moveToThread(self.stdoutThread)
            self.stdoutThread.started.connect(receiver.run)
            self.stdoutThread.start()
        do_after_run()
        def onQuit():
            if self.pipeOutput:
                receiver.SetKill()
                self.stdoutThread.exit()
        self.app.aboutToQuit.connect(onQuit)
        sys.exit(self.app.exec_())

    def setFastMode(self, fastmode):
        self.fastmode = fastmode

class DummyObject:
    def __init__(self,*arg,**kwargs):
        pass
    def __getattr__(self,name):
        return DummyObject()
    def __call__(self,*args,**kwargs):
        pass

    def plot(self, x,y):
        import matplotlib.pyplot as plt
        plt.plot(x,y)
        plt.show()

gui = DummyObject()
