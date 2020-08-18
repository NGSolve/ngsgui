
# Spyder imports
from spyder.config.base import _
from spyder.utils.qthelpers import (create_action, create_toolbutton,
                                    add_actions, MENU_SEPARATOR)
from spyder.utils import icon_manager as ima
from spyder.api.plugins import SpyderPluginWidget
from spyder.plugins.ipythonconsole.utils.kernelspec import SpyderKernelSpec
import spyder.plugins.ipythonconsole.plugin as ipyplugin
import spyder.plugins.ipythonconsole.widgets.namespacebrowser as spyder_namespacebrowser

# NGSolve imports
import ngsgui.gui as G
from ngsgui.widgets import ArrangeH
import weakref

import logging
logger = logging.getLogger(__name__)

# qt imports
from qtpy import PYQT4, PYSIDE, QtCore

import ngsolve, cloudpickle, queue, threading
import ngsgui.thread as thread
ngsolve.ngsglobals.msg_level = 0

class Drawer:
    def __init__(self, ngs_plugin):
        self.index_table = {}
        self._ngs_plugin = ngs_plugin
        self.to_draw = queue.Queue()
        def run():
            while True:
                val = self.to_draw.get()
                if val is None:
                    break
                what, values = val
                while what == 'redraw':
                    try:
                        val = self.to_draw.get(False)
                        logger.debug("throw away redraw signal")
                    except:
                        break
                    what, values = val
                if what == "draw":
                    index, args, kwargs = values
                    scene = ngsolve.Draw(*args, **kwargs)
                    self._ngs_plugin.switch_to_plugin()
                    scene._redraw_index = index
                    self.index_table[index] = weakref.ref(scene)
                elif what == "set_scene_item":
                    index, name, val = values
                    logger.debug("Receive set {} of scene {} to {}".format(name, index, val))
                    scene = self.getScene(index)
                    if scene:
                        setattr(scene, name, val)
                elif what == "call_scene_item":
                    index, name, args, kwargs = values
                    logger.debug("Receive call {} of scene {} with args {} and kwargs {}".format(name, index,
                                                                                                 args, kwargs))
                    scene = self.getScene(index)
                    if scene:
                        getattr(scene, name)(*args, **kwargs)
                else:
                    assert what == "redraw"
                    widget = G.gui.window_tabber.activeGLWindow.glWidget
                    state = widget.blockSignals(True)
                    for scene in widget.scenes:
                        if hasattr(scene, "_redraw_index"):
                            scene.update(*(values[scene._redraw_index]))
                    widget.blockSignals(state)
                    widget.updateGL()
                self.to_draw.task_done()
        self.worker = threading.Thread(target=run)
        self.worker.start()

    def getScene(self, index):
        if index in self.index_table:
            scene = self.index_table[index]()
            if not scene:
                logger.debug("Scene {} already dead".format(index))
        else:
            scene = None
            logger.error("Scene {} does not exist".format(index))
        return scene

class NgsSpyderKernelSpec(SpyderKernelSpec):
    @property
    def env(self):
        env_vars = super().env
        start_code = "import os; os.environ['NGSGUI_HEADLESS'] = '1'; del os; import spyder_ngsgui.startup as s;"
        env_vars['SPY_RUN_LINES_O'] = start_code + env_vars['SPY_RUN_LINES_O']
        return env_vars

class NGSolvePlugin(SpyderPluginWidget):
    CONF_SECTION = "ngsolve"
    LOCATION = QtCore.Qt.BottomDockWidgetArea

    def __init__(self, parent):
        SpyderPluginWidget.__init__(self, parent)
        self.gui = G.GUI(flags=["--noOutputpipe", "--noConsole"],startApplication=False,
                         createMenu=False)
        G.gui = self.gui
        self.setLayout(ArrangeH(self.gui.mainWidget))
        self.shellwidgets = {}

    # SpyderPluginMixin API
    def on_first_registration(self):
        """Action to be performed on first plugin registration."""
        self.tabify(self.main.variableexplorer)

    def update_font(self):
        """Update font from Preferences."""
        pass

    # ------ SpyderPluginWidget API -------------------------------------------

    def add_shellwidget(self, shellwidget):
        shellwidget_id = id(shellwidget)
        if shellwidget_id not in self.shellwidgets:
            self.shellwidgets[shellwidget_id] = shellwidget
            def redraw(drawn_objects):
                self.drawer.to_draw.put(["redraw", drawn_objects])
            def draw(index, *args, **kwargs):
                self.drawer.to_draw.put(["draw", (index, args, kwargs)])
            def set_scene_item(index, name, value):
                self.drawer.to_draw.put(["set_scene_item", (index, name, value)])
            def call_scene_item(index, name, *args, **kwargs):
                self.drawer.to_draw.put(["call_scene_item", (index, name, args, kwargs)])
            shellwidget.spyder_kernel_comm.register_call_handler("ngsolve_redraw", redraw)
            shellwidget.spyder_kernel_comm.register_call_handler("ngsolve_draw", draw)
            shellwidget.spyder_kernel_comm.register_call_handler("ngsolve_set_scene_item", set_scene_item)
            shellwidget.spyder_kernel_comm.register_call_handler("ngsolve_call_scene_item", call_scene_item)
            def loadNGS():
                shellwidget.silent_execute("import os")
                shellwidget.silent_execute("os.environ['NGSGUI_HEADLESS'] = '1'")
                shellwidget.silent_execute("import spyder_ngsgui.startup as s")
                shellwidget.silent_execute("del os")
                shellwidget.silent_execute("del s")
            shellwidget.sig_prompt_ready.connect(loadNGS)

    def remove_shellwidget(self, shellwidget_id):
        if shellwidget_id in self.shellwidgets:
            sw = self.shellwidgets.pop(shellwidget_id)

    def get_plugin_title(self):
        """Return widget title."""
        title = _('NGSolve')
        return title

    def get_plugin_icon(self):
        """Return widget icon."""
        return ima.icon('ipython_console')

    def get_focus_widget(self):
        """Return the widget to give focus to."""
        return self.gui.mainWidget

    def register_plugin(self):
        """Register plugin in Spyder's main window."""
        super().register_plugin()
        self.ipyconsole = self.main.ipyconsole
        # call add_shellwidget everywhere
        self.drawer = Drawer(self)
        old_connect_external_kernel = self.ipyconsole.connect_external_kernel
        def new_connect_external_kernel(_self, sw):
            old_connect_external_kernel(_self, sw)
            self.add_shellwidget(sw)
            kc = sw.kernel_client
            kc.stopped_channels.connect(lambda : self.remove_shellwidget(id(sw)))
        self.ipyconsole.connect_external_kernel = new_connect_external_kernel
        old_process_started = self.ipyconsole.process_started
        def new_process_started(client):
            old_process_started(client)
            self.add_shellwidget(client.shellwidget)
        self.ipyconsole.process_started = new_process_started

        old_process_finished = self.ipyconsole.process_finished
        def new_process_finished(client):
            old_process_finished(client)
            self.remove_shellwidget(id(client.shellwidget))
        self.ipyconsole.process_finished = new_process_finished

    def check_compatibility(self):
        """Check compatibility for PyQt and sWebEngine."""
        value = True
        message = ''
        if PYQT4 or PYSIDE:
            message = _("You are working with Qt4 and in order to use this "
                        "plugin you need to have Qt5.<br><br>"
                        "Please update your Qt and/or PyQt packages to "
                        "meet this requirement.")
            value = False
        return value, message
