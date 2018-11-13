
# Spyder imports
from spyder.config.base import _
from spyder.utils.qthelpers import (create_action, create_toolbutton,
                                    add_actions, MENU_SEPARATOR)
try:
    # Spyder 4
    from spyder.api.plugins import SpyderPluginWidget
except ImportError:
    # Spyder 3
    from spyder.plugins import SpyderPluginWidget

# NGSolve imports
import ngsgui.gui as G
from ngsgui.widgets import ArrangeH

# qt imports
from qtpy import PYQT4, PYSIDE

class NGSolvePlugin(SpyderPluginWidget):
    CONF_SECTION = "ngsolve"
    def __init__(self, parent):
        super().__init__(parent)
        self.main = parent
        self.gui = G.GUI(flags=["--noOutputpipe", "--noConsole", "--noEditor"],startApplication=False)
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

    def get_plugin_actions(self):
        """Return a list of actions related to plugin."""
        return []

    def register_plugin(self):
        """Register plugin in Spyder's main window."""
        with open("debug.out","a") as f:
            f.write("register ngs plugin\n")
        self.main.add_dockwidget(self)
        self.ipyconsole = self.main.ipyconsole
        print("plugin loaded")

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
