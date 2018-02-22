
from . import widgets as wid

class Settings():
    def __init__(self, gui):
        self.name = "Settings"
        self.gui = gui
        self.toolboxupdate = lambda me: None

    def getQtWidgets(self):
        self.widgets = wid.OptionWidgets()

