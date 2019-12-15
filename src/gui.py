
import os

from . import glwindow
from . widgets import ArrangeV
from .thread import inmain_decorator, inthread
from .globalSettings import SettingDialog
from .stdPipes import OutputBuffer

import logging
logger = logging.getLogger(__name__)

import ngsolve

class DictWithHook(dict):
    def __setitem__(self, name, val):
        super().__setitem__(name,val)
        self.hook(name,val)

    def hook(self, name, val):
        """This function can be patched to hook into setitem"""
        pass

def _load_plugins():
    import pkg_resources
    for entry_point in pkg_resources.iter_entry_points(group="ngsgui.plugin",name=None):
        plugin = entry_point.load()
        plugin(None)

def _createScene(obj, *args, tab=None, **kwargs):
    for t in type(obj).__mro__:
        if t in GUI.sceneCreators:
            return GUI.sceneCreators[t](obj, *args, **kwargs)
    print("Cannot create scene from object of type", type(obj))

from qtpy import QtWidgets, QtCore, QtGui

class GUI():
    """Graphical user interface for NGSolve. This object is created when ngsolve is started and
the ngsgui.gui.gui object is set to it. You can import it and manipulate it on the fly with:
from ngsgui.gui import gui
"""
    sceneCreators = {}
    file_loaders = {}
    def __init__(self,flags=None, startApplication=True, createMenu=True):
        self._parseFlags(flags)
        self._hasApplication = startApplication
        self._hasMenuBar = createMenu
        if startApplication:
            self.app = QtWidgets.QApplication([])
            self.app.setOrganizationName("NGSolve")
            self.app.setApplicationName("NGSolve")
        ngsolve.solve._SetLocale()
        self._commonContext = glwindow.GLWidget()
        if createMenu:
            self._createMenu()
        self._createLayout()
        self.mainWidget.setWindowTitle("NGSolve")
        self._crawlPlugins()
        from .gl import Shader
        Shader.preloadShaderIncludes()
        self._procs = []
        if startApplication:
            self.app.aboutToQuit.connect(self._killProcs)

    def _killProcs(self):
        """If external processes are spawned somewhere register them in self._procs to be killed when
the gui is closed"""
        for proc in self._procs:
            proc.kill()
            proc.waitForFinished()

    def _createMenu(self):
        """Creates menu bar. It can afterwards be modified by plugins"""
        logger.debug("Create menu bar")
        from .menu import MenuBarWithDict
        self.menuBar = MenuBarWithDict()
        filemenu = self.menuBar["&File"]

        def selectMeshFile():
            filename, filt = QtWidgets.QFileDialog.getOpenFileName(caption = "Load Mesh",
                                                                   filter = "Netgen mesh file (*.vol, *.vol.gz);; Neutral format (*.mesh, *.emt);; Surface format (*.surf);; Universal format (*.unv);; Olaf format (*.emt);; TET format (*.tet);; STL format (*.stl, *.stlb);; Pro/ENGINEER neutral format (*.fnf)")
            if filename:
                if filename.endswith(".vol") or filename.endswith(".vol.gz"):
                    mesh = ngsolve.Mesh(filename)
                else:
                    from netgen.meshing import ImportMesh
                    mesh = ngsolve.Mesh(ImportMesh(filename))
                ngsolve.Draw(mesh)
                if not self._flags.noConsole:
                    self.console.pushVariables({"mesh" : mesh})

        def saveNetgenMesh():
            from .scenes import BaseMeshScene
            activeWindow = self.getCurrentGLWindow().glWidget
            meshes = set()
            for scene in activeWindow.scenes:
                if scene.active and isinstance(scene, BaseMeshScene):
                    meshes.add(scene.mesh)
            if len(meshes) != 1:
                self.showErrorMessageBox("Failure mesh saving",
                                         "Failed to save mesh, hide all scenes with meshes except the one to save")
                return
            mesh = meshes.pop()

            filename, filt = QtWidgets.QFileDialog.getSaveFileName(caption="Save Mesh",
                                                                   filter="Netgen mesh file (*.vol, *.vol.gz)")
            if filename:
                mesh.ngmesh.Save(filename)

        loadMesh = filemenu["&Load"].addAction("&Mesh")
        loadMesh.triggered.connect(selectMeshFile)
        saveMesh = filemenu["&Save"].addAction("&Mesh")
        saveMesh.triggered.connect(saveNetgenMesh)

        saveSolution = filemenu["&Save"].addAction("&Solution")
        loadSolution = filemenu["&Load"].addAction("&Solution")
        loadSolution.triggered.connect(self.loadSolution)
        saveSolution.triggered.connect(self.saveSolution)
        def selectPythonFile():
            filename, filt = QtWidgets.QFileDialog.getOpenFileName(caption = "Load Python File",
                                                                   filter = "Python files (*.py)")
            if filename:
                self.loadPythonFile(filename)
        filemenu["&Load"].addAction("&Python File", selectPythonFile, shortcut = "l+y")
        newWindowAction = self.menuBar["&Create"].addAction("New &Window")
        newWindowAction.triggered.connect(lambda :self.window_tabber.make_window())
        settings = self.menuBar["&Settings"].addAction("&Settings")
        settings.triggered.connect(lambda : setattr(self, "settings", SettingDialog()) or self.settings.show())
        if not self._flags.noConsole:
            mem_profiler = self.menuBar["&Tools"].addAction("&Show Memory Profile")
            def showMemProfile():
                from .systemmonitor import MemoryUsageProfiler
                self._memUsage = MemoryUsageProfiler(self.console)
                self.window_tabber.addTab(self._memUsage, "Memory Profiler")
            mem_profiler.triggered.connect(showMemProfile)
        if os.getenv("NGSGUI_TEST_CREATION"):
            self._addTestMenu()

    def _loadTest(self, filename):
        """Load a .test file, these files are created using the save test menu item, that is available if the
environment variable NGSGUI_TEST_CREATION is set. It uses some monkeypatching to pickle the gui
state and being able to reload it without a graphical interface."""
        import ngsolve
        import pickle
        from .glwindow import WindowTab
        from .settings import BaseSettings
        logger.debug("Load testfile {}".format(filename))
        save_setstate = BaseSettings.__setstate__
        def newSetstate(scene, state):
            BaseSettings.__init__(scene)
            for key, value in state[0].items():
                scene.__getattribute__("set" + key)(value)
        BaseSettings.__setstate__ = newSetstate
        with open(filename, "rb") as f:
            tabs = pickle.load(f)
        for scenes, parameters, name in tabs:
            tab = WindowTab(rendering_parameters=parameters)
            tab._startup_scenes = scenes
            tab.create(self._commonContext)
            self.window_tabber.addTab(tab, name)
        BaseSettings.__setstate__ = save_setstate

    def getScenesFromCurrentWindow(self):
        """Get the list of the scenes of the currently active GLWindow"""
        return self.window_tabber.activeGLWindow.glWidget.scenes

    def getCurrentGLWindow(self):
        return self.window_tabber.activeGLWindow

    def _createLayout(self):
        """Creates the main layout of the gui"""
        from .globalSettings import SettingsToolBox
        self.mainWidget = QtWidgets.QWidget()
        if self._hasMenuBar:
            menu_splitter = QtWidgets.QSplitter()
            menu_splitter.setOrientation(QtCore.Qt.Vertical)
            menu_splitter.addWidget(self.menuBar)
        self.toolbox_splitter = toolbox_splitter = QtWidgets.QSplitter()
        if self._hasMenuBar:
            menu_splitter.addWidget(toolbox_splitter)
        toolbox_splitter.setOrientation(QtCore.Qt.Horizontal)
        self.settings_toolbox = SettingsToolBox(parent=toolbox_splitter)
        window_splitter = QtWidgets.QSplitter(parent=toolbox_splitter)
        toolbox_splitter.addWidget(window_splitter)
        window_splitter.setOrientation(QtCore.Qt.Vertical)
        self.window_tabber = glwindow.WindowTabber(commonContext = self._commonContext,
                                                   parent=window_splitter)
        self.window_tabber._fastmode = self._flags.fastmode
        window_splitter.addWidget(self.window_tabber)
        if not (self._flags.noConsole and not self._flags.outputpipe):
            self.output_tabber = glwindow.WindowTabber(commonContext=self._commonContext,
                                                   parent=window_splitter)
        if not self._flags.noConsole:
            from .console import MultiQtKernelManager, NGSJupyterWidget
            self.multikernel_manager = MultiQtKernelManager()
            self.console = NGSJupyterWidget(gui=self,multikernel_manager = self.multikernel_manager)
            if self._hasApplication:
                self.console.exit_requested.connect(self.app.quit)
            self.output_tabber.addTab(self.console,"Console")
        if self._flags.outputpipe:
            self.outputBuffer = OutputBuffer()
            self.output_tabber.addTab(self.outputBuffer, "Output")
            self.output_tabber.setCurrentWidget(self.outputBuffer)
        settings = QtCore.QSettings()
        if settings.value("sysmon/active", "false") == "true":
            from .systemmonitor import SystemMonitor
            self._SysMonitor = SystemMonitor()
            self._SysMonitor.setFixedHeight(30)
            sysmon_splitter = QtWidgets.QSplitter(parent=window_splitter)
            sysmon_splitter.setOrientation(QtCore.Qt.Vertical)
            if self._flags.outputpipe or not self._flags.noConsole:
                sysmon_splitter.addWidget(self.output_tabber)
            sysmon_splitter.addWidget(self._SysMonitor)
            sysmon_splitter.setSizes([10000,2000])
            window_splitter.addWidget(sysmon_splitter)
        else:
            if self._flags.outputpipe or not self._flags.noConsole:
                window_splitter.addWidget(self.output_tabber)
        if self._hasMenuBar:
            menu_splitter.setSizes([100, 10000])
        window_splitter.setSizes([70000, 30000])
        self.mainWidget.setLayout(ArrangeV(menu_splitter if self._hasMenuBar else toolbox_splitter))
        self._addShortcuts()

    def _crawlPlugins(self):
        """Crawls registered plugins, plugins can be added by using the entry point ngsgui.plugin"""
        import pkg_resources
        for entry_point in pkg_resources.iter_entry_points(group="ngsgui.plugin",name=None):
            logger.debug("Load plugin {}".format(str(entry_point)))
            plugin = entry_point.load()
            plugin(self)

    def _tryLoadFile(self, filename):
        """Tries to load the given file, if the file doesn't exist it does nothing. There must
exist a load function for the file extension type registered in GUI.file_loaders"""
        if os.path.isfile(filename):
            name, ext = os.path.splitext(filename)
            if not ext in GUI.file_loaders:
                self.showErrorMessageBox("File loading error", "Cannot load file type: " + ext)
                return
            try:
                GUI.file_loaders[ext](self, filename)
            except Exception as e:
                import traceback
                self.showErrorMessageBox("Exception in load file:", str(e))
                traceback.print_exc()
        else:
            logger.error("Couldn't find file {}".format(filename))    

    def _parseFlags(self, flags):
        """Parses command line arguments and calls functions registered in GUI.flags. If argument is
not none, argument is parsed instead of command line args"""
        import argparse
        parser = argparse.ArgumentParser()
        parser.add_argument("file",type=str,nargs="?",
                            help="load file, readable file types: py,ngs,stl,step,geo,in2d,vol.gz,vol")
        parser.add_argument("-fm","--fastmode", action="store_false",
                            help="Activate fastmode, some large scenes are drawn less accurate but faster")
        parser.add_argument("-nc","--noConsole", action="store_true",
                            help="Start without jupyter console")
        parser.add_argument("-o", "--outputpipe", action="store_true",
                            help="Pipe output to buffer in gui")
        parser.add_argument("-dc","--dontCatchExceptions", action="store_true",
                            help="Don't catch exceptions up to user input, but show internal gui traceback")
        parser.add_argument("--logfile", nargs=1, type=str, action="store",
                            help="Write debug log to file")
        parser.add_argument("--logformat", nargs=1, type=str, action="store",
                            help="Set format for logging, only active if logfile is set")
        parser.add_argument("--keep_matplotlib_debug_log", action="store_true",
                            help="Matplotlib writes a lot of debug log output, which we remove from the log files, set this to keep it.")
        parser.add_argument("--trace_paintGL_calls", action="store_true",
                            help="Write trace of paintGL calls to logfile")
        if not flags is None:
            self._flags = parser.parse_args(flags)
        else:
            self._flags = parser.parse_args()
        if self._flags.logfile is not None:
            fmt = self._flags.logformat if self._flags.logformat else '%(asctime)s [%(levelname)s] {%(name)s} - %(message)s'
            logging.basicConfig(level=logging.DEBUG,
                                format=fmt,
                                filename=os.path.join(os.getcwd(), self._flags.logfile[0]),
                                filemode="w")
            if not self._flags.keep_matplotlib_debug_log:
                logging.getLogger("matplotlib").setLevel(logging.INFO)
            if self._flags.trace_paintGL_calls:
                glwindow.GLWidget._trace_paintGL_calls = True
        logger.debug("Parsed flags: {}".format(self._flags))


    def showErrorMessageBox(self, title, text):
        logger.error(text)
        self._msgbox = QtWidgets.QMessageBox(text=text)
        self._msgbox.setWindowTitle(title)
        self._msgbox.show()

    def saveSolution(self):
        """Opens a file dialog to save the current state of the GUI, including all drawn objects."""
        import pickle
        filename, filt = QtWidgets.QFileDialog.getSaveFileName(caption="Save Solution",
                                                               filter = "Solution Files (*.ngs)")
        if not filename[-4:] == ".ngs":
            filename += ".ngs"
        tabs = []
        for i in range(self.window_tabber.count()):
            tabs.append((self.window_tabber.widget(i),self.window_tabber.tabBar().tabText(i)))
        settings = self.settings_toolbox.settings
        currentIndex = self.window_tabber.currentIndex()
        with open(filename,"wb") as f:
            pickle.dump((tabs,settings, currentIndex), f)

    def _loadSolutionFile(self, filename):
        """Loads a .ngs solutions file, which contains a pickled gui state"""
        import pickle
        if not filename[-4:] == ".ngs":
            filename += ".ngs"
        with open(filename, "rb") as f:
            tabs,settings,currentIndex = pickle.load(f)
        for tab,name in tabs:
            if isinstance(tab, glwindow.WindowTab):
                tab.create(self._commonContext)
            self.window_tabber.addTab(tab, name)
        for setting in settings:
            setting.gui = self
            self.settings_toolbox.addSettings(setting)
        self.window_tabber.activeGLWindow = self.window_tabber.widget(currentIndex)

    def loadSolution(self):
        """Opens a file dialog to load a solution (*.ngs) file"""
        filename, filt = QtWidgets.QFileDialog.getOpenFileName(caption="Load Solution",
                                                               filter = "Solution Files (*.ngs)")
        self._loadSolutionFile(filename)

    def draw(self, *args, **kwargs):
        """Draw an object in the active GLWindow. The objects class must have a registered
 function/constructor (in GUI.sceneCreators) to create a scene from. Scenes,
 Meshes, (most) CoefficientFunctions, (most) GridFunctions and geometries can
 be drawn by default."""
        self.window_tabber.draw(*args,**kwargs)

    @inmain_decorator(wait_for_return=False)
    def redraw(self):
        """Redraw non-blocking. Redraw signals with a framerate higher than 50 fps are discarded, so
another Redraw after a time loop may be needed to see the final solutions."""
        logger.debug("Call redraw")
        self.window_tabber.activeGLWindow.glWidget.updateScenes()

    @inmain_decorator(wait_for_return=True)
    def redraw_blocking(self):
        """Draw blocking, no Redraw signals are discarded but it is a lot slower than non blocking"""
        logger.debug("Blocking redraw")
        self.window_tabber.activeGLWindow.glWidget.updateScenes()

    @inmain_decorator(wait_for_return=True)
    def renderToImage(self, width, height, filename=None, num_samples=16):
        """Render the current active GLWindow into a file"""
        import copy
        import OpenGL.GL as GL
        from qtpy import QtOpenGL
        viewport = GL.glGetIntegerv( GL.GL_VIEWPORT )
        GL.glViewport(0, 0, width, height)
        GL.glEnable(GL.GL_MULTISAMPLE)

        # =======================================
        # create framebuffer without multisampling
        simple_fb = GL.glGenFramebuffers(1)
        GL.glBindFramebuffer(GL.GL_FRAMEBUFFER, simple_fb)

        # create renderbuffer for image without multisampling
        simple_rb = GL.glGenRenderbuffers(1)
        GL.glBindRenderbuffer(GL.GL_RENDERBUFFER, simple_rb)
        GL.glRenderbufferStorage(GL.GL_RENDERBUFFER, GL.GL_RGBA8, width, height)
        GL.glFramebufferRenderbuffer(GL.GL_FRAMEBUFFER, GL.GL_COLOR_ATTACHMENT0, GL.GL_RENDERBUFFER, simple_rb)

        # =======================================
        # create framebuffer with multisampling
        fb = GL.glGenFramebuffers(1)
        GL.glBindFramebuffer(GL.GL_FRAMEBUFFER, fb)

        # create renderbuffer for image
        rb = GL.glGenRenderbuffers(1)
        GL.glBindRenderbuffer(GL.GL_RENDERBUFFER, rb)
        GL.glRenderbufferStorageMultisample(GL.GL_RENDERBUFFER, num_samples, GL.GL_RGBA8, width, height)
        GL.glFramebufferRenderbuffer(GL.GL_FRAMEBUFFER, GL.GL_COLOR_ATTACHMENT0, GL.GL_RENDERBUFFER, rb)

        # renderbuffer for depth
        depth_rb = GL.glGenRenderbuffers(1)
        GL.glBindRenderbuffer(GL.GL_RENDERBUFFER, depth_rb)
        GL.glRenderbufferStorageMultisample(GL.GL_RENDERBUFFER, num_samples, GL.GL_DEPTH_COMPONENT32, width, height)
        GL.glFramebufferRenderbuffer(GL.GL_FRAMEBUFFER, GL.GL_DEPTH_ATTACHMENT, GL.GL_RENDERBUFFER, depth_rb)

        # render image
        self.window_tabber.activeGLWindow.glWidget.paintGL()
        GL.glFinish()

        # blit image from multisampled framebuffer to simple frambuffer
        GL.glBindFramebuffer(GL.GL_READ_FRAMEBUFFER, fb)
        GL.glBindFramebuffer(GL.GL_DRAW_FRAMEBUFFER, simple_fb)
        GL.glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL.GL_COLOR_BUFFER_BIT, GL.GL_NEAREST)
        GL.glBindFramebuffer(GL.GL_FRAMEBUFFER, simple_fb)

        data = GL.glReadPixels(0, 0, width, height, GL.GL_RGBA, GL.GL_UNSIGNED_BYTE, outputType=None)
        import PIL.Image as im
        image = im.frombytes('RGBA', (width,height), data).transpose(im.FLIP_TOP_BOTTOM)

        if filename!=None:
            image.save(filename)

        # cleanup
        GL.glBindFramebuffer(GL.GL_FRAMEBUFFER, 0)
        GL.glDeleteFramebuffers(2, [fb, simple_fb])
        GL.glDeleteRenderbuffers(2, [rb, simple_rb])
        GL.glDeleteRenderbuffers(1, [depth_rb])

        GL.glViewport(*viewport)
        return image

    def plot(self, *args, **kwargs):
        """ Plot a matplotlib figure into a new Window"""
        self.window_tabber.plot(*args, **kwargs)

    @inmain_decorator(wait_for_return=True)
    def _loadFile(self, filename):
        txt = ""
        with open(filename,"r") as f:
            for line in f.readlines():
                txt += line
        return txt

    def _runCode(self, code_string, filename=None):
        locs = DictWithHook()
        locs["__name__"] = "__main__"
        if filename:
            locs["__file__"] = filename
        if not self._flags.noConsole:
            locs.hook = lambda name, val: self.console.pushVariables({ name: val})
        try:
            code = compile(code_string, filename, 'exec')
            exec(code, locs)
        except Exception as e:
            import traceback
            self.showErrorMessageBox("Exception in run code:", str(e))
            traceback.print_exc()

    def loadPythonFile(self, filename):
        """Load a Python file and execute it if gui.executeFileOnStartup is True"""
        with open(filename, "r") as f:
            code = f.read()
            try:
                inthread(lambda : self._runCode(code, filename))
            except Exception as ex:
                self.msgbox = QtWidgets.QMessageBox(text = type(ex).__name__ + ": " + str(ex))
                self.msgbox.setWindowTitle("Exception caught!")
                self.msgbox.show()

    def _run(self,do_after_run=lambda : None, run_event_loop=True):
        import sys, inspect
        self.mainWidget.show()
        globs = inspect.stack()[1][0].f_globals
        if not self._flags.noConsole:
            self.console.pushVariables(globs)
        settings = QtCore.QSettings()
        if self._flags.outputpipe:
            self.outputBuffer.start()
        if settings.value("sysmon/active", "false") == "true":
            self._SysMonitor.start()
        do_after_run()
        if self._flags.file:
            self._tryLoadFile(self._flags.file)
        def onQuit():
            if self._flags.outputpipe:
                self.outputBuffer.onQuit()
        self.app.aboutToQuit.connect(onQuit)
        if run_event_loop:
            sys.exit(self.app.exec_())

    def _addShortcuts(self):
        """Adds shortcuts to the main widget"""
        from .widgets import addShortcut
        def switchTabWindow(direction):
            self.window_tabber.setCurrentIndex((self.window_tabber.currentIndex() + direction)
                                               %self.window_tabber.count())
        if not self._flags.noConsole:
            def activateConsole():
                self.output_tabber.setCurrentWidget(self.console)
                self.console._control.setFocus()
            addShortcut(self.mainWidget, "Gui-Activate Console", "Ctrl+j", activateConsole)
        if self._hasApplication:
            addShortcut(self.mainWidget, "Gui-Quit", "Ctrl+q", lambda: self.app.quit())
        addShortcut(self.mainWidget, "Gui-Close Tab", "Ctrl+w",
                    lambda: self.window_tabber._remove_tab(self.window_tabber.currentIndex()))
        addShortcut(self.mainWidget, "Gui-Next Tab", "Ctrl+LeftArrow", lambda: switchTabWindow(-1))
        addShortcut(self.mainWidget, "Gui-Previous Tab", "Ctrl+RightArrow", lambda: switchTabWindow(1))

    def _addTestMenu(self):
        """Adds menu options to create tests"""
        from .settings import BaseSettings
        save_test =  self.menuBar["&File"]["&Save"].addAction("&Test")
        def saveTest():
            import pickle
            filename, filt = QtWidgets.QFileDialog.getSaveFileName(caption="Save Test",
                                                                   filter = "Test files (*.test)")
            if not filename.endswith(".test"):
                filename += ".test"
            save_getstate = BaseSettings.__getstate__
            BaseSettings.__getstate__ = lambda self: ({key : par.getValue() for key, par in self._par_name_dict.items() if hasattr(par, "getValue")},)
            tabs = []
            for i in range(self.window_tabber.count()):
                if self.window_tabber.widget(i).isGLWindow():
                    tabs.append((self.window_tabber.widget(i).glWidget.scenes,
                                 self.window_tabber.tabBar().tabText(i)))
            with open(filename, "wb") as f:
                pickle.dump(tabs,f)
            # BaseSettings.__getstate__ = save_getstate
        save_test.triggered.connect(saveTest)
        load_test = self.menuBar["&File"]["&Load"].addAction("&Test")
        def loadTest():
            filename, filt = QtWidgets.QFileDialog.getOpenFileName(caption="Load Test",
                                                                   filter = "Test files (*.test)")
            if filename:
                self._loadTest(filename)
        load_test.triggered.connect(loadTest)


class DummyObject:
    """If code is not executed using ngsolve, then this dummy object allows to use the same code
with a netgen or python3 call as well"""
    def __init__(self,*arg,**kwargs):
        pass
    def __getattr__(self,name):
        return DummyObject()
    def __call__(self,*args,**kwargs):
        pass

    def plot(self, *args, **kwargs):
        import matplotlib.pyplot as plt
        plt.plot(*args, **kwargs)
        plt.show()

GUI.file_loaders[".py"] = GUI.loadPythonFile
GUI.file_loaders[".ngs"] = GUI._loadSolutionFile
def _loadSTL(gui, filename):
    import netgen.stl as stl
    geo = stl.LoadSTLGeometry(filename)
    ngsolve.Draw(geo)

def _loadOCC(gui, filename):
    try:
        gui.console.execute("import ngsolve")
        gui.console.execute("import netgen.NgOCC")
        gui.console.execute("geo = netgen.NgOCC.LoadOCCGeometry('" + filename + "')")
        gui.console.execute("ngsolve.Draw(geo)")
    except ImportError:
        gui.showErrorMessageBox("Netgen is not built with OCC support!")
def _loadGeo(gui, filename):
    import netgen.csg as csg
    geo = csg.CSGeometry(filename)
    ngsolve.Draw(geo)

def _loadin2d(gui, filename):
    import netgen.geom2d as geom2d
    geo = geom2d.SplineGeometry(filename)
    ngsolve.Draw(geo)

GUI.file_loaders[".stl"] = _loadSTL
GUI.file_loaders[".step"] = _loadOCC
GUI.file_loaders[".stp"] = _loadOCC
GUI.file_loaders[".geo"] = _loadGeo
GUI.file_loaders[".in2d"] = _loadin2d
if os.getenv("NGSGUI_TEST_CREATION"):
    GUI.file_loaders[".test"] = GUI._loadTest

gui = DummyObject()
