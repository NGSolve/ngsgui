
# Spyder imports
from spyder.config.base import _
from spyder.utils.qthelpers import (create_action, create_toolbutton,
                                    add_actions, MENU_SEPARATOR)
from spyder.utils import icon_manager as ima
try:
    # Spyder 4
    from spyder.api.plugins import SpyderPluginWidget
    from spyder.plugins.ipythonconsole.utils.kernelspec import SpyderKernelSpec
    import spyder.plugins.ipythonconsole.plugin as ipyplugin
    import spyder.plugins.ipythonconsole.widgets.namespacebrowser as spyder_namespacebrowser
except ImportError:
    # Spyder 3
    from spyder.plugins import SpyderPluginWidget
    from spyder.utils.ipython.kernelspec import SpyderKernelSpec
    import spyder.plugins.ipythonconsole as ipyplugin
    import spyder.widgets.ipythonconsole.namespacebrowser as spyder_namespacebrowser

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
    def __init__(self):
        self.index_table = {}
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
        super().__init__(parent)
        self.main = parent
        self.gui = G.GUI(flags=["--noOutputpipe", "--noConsole"],startApplication=False,
                         createMenu=False)
        G.gui = self.gui
        self.initialize_plugin()
        self.setLayout(ArrangeH(self.gui.mainWidget))

    # SpyderPluginMixin API
    def on_first_registration(self):
        """Action to be performed on first plugin registration."""
        pass

    def update_font(self):
        """Update font from Preferences."""
        pass

    # ------ SpyderPluginWidget API -------------------------------------------
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

    def closing_plugin(self, cancelable=False):
        """Perform actions before parent main window is closed."""
        return True

    def refresh_plugin(self):
        """Refresh tabwidget."""
        pass

    def create_dockwidget(self):
        doc,loc = super().create_dockwidget()
        return doc, loc

    def get_plugin_actions(self):
        """Return a list of actions related to plugin."""
        return []

    def register_plugin(self):
        """Register plugin in Spyder's main window."""
        self.main.add_dockwidget(self)
        self.ipyconsole = self.main.ipyconsole
        _drawer = self.ipyconsole._ngs_drawer = Drawer()
        # patch NameSpaceBrowser._handle_spyder_msg to get our ngsolve stuff...
        old_handle_spyder_msg = spyder_namespacebrowser.NamepaceBrowserWidget._handle_spyder_msg
        def new_handle_spyder_msg(_self, msg):
            spyder_msg_type = msg['content'].get('spyder_msg_type')
            if spyder_msg_type == 'ngsolve_redraw':
                try:
                    _drawer.to_draw.put(["redraw",cloudpickle.loads(bytes(msg['buffers'][0]))])
                except Exception as e:
                    # TODO: sometimes this crashes with a bad_weak_ptr exception...
                    pass
            elif spyder_msg_type.startswith("ngsolve_"):
                ngs_type, vals = spyder_msg_type[8:], cloudpickle.loads(bytes(msg['buffers'][0]))
                logger.debug("Receive ngsolve msg {} with values {}".format(ngs_type, vals))
                _drawer.to_draw.put([ngs_type, vals])
            else:
                old_handle_spyder_msg(_self, msg)
        spyder_namespacebrowser.NamepaceBrowserWidget._handle_spyder_msg = new_handle_spyder_msg

        ipyplugin.SpyderKernelSpec = NgsSpyderKernelSpec
        # monkeypatch notebookplugin if available
        try:
            import spyder_notebook.utils.nbopen as nbo
            nbo.KERNELSPEC = ('spyder_ngsgui.ngsgui_plugin.NgsSpyderKernelSpec')
        except ImportError:
            pass



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
