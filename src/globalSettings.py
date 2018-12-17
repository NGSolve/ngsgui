
from qtpy import QtWidgets, QtCore
from .widgets import ArrangeH, ArrangeV
from .thread import inmain_decorator


class BaseSettings(QtWidgets.QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)
        settings = QtCore.QSettings()
        self._sysmonCB = QtWidgets.QCheckBox("System monitor")
        self._sysmonCB.setChecked(settings.value("sysmon/active", "false") == "true")
        self._sysmonCB.stateChanged.connect(self._checkSysmonRequirement)
        self.setLayout(ArrangeV(QtWidgets.QLabel("Settings will get active after restarting the GUI"),
                                self._sysmonCB))

    def _checkSysmonRequirement(self):
        if self._sysmonCB.isChecked():
            try:
                import matplotlib
            except ModuleNotFoundError:
                self._sysmonCB.setChecked(False)
                self.msgbox = QtWidgets.QMessageBox(text="""Cannot show system monitor without matplotlib, please install it with
    pip3 install --user matplotlib""")
                self.msgbox.show()

    def _checkValidSelection(self, text):
        if text == "emacs":
            try:
                import epc
            except ModuleNotFoundError:
                self.msgbox = QtWidgets.QMessageBox(text="""Cannot embed emacs without epc, please install it with
pip3 install --user epc""")
                self.msgbox.show()

    def _save(self):
        settings = QtCore.QSettings()
        settings.setValue("sysmon/active", self._sysmonCB.isChecked())

class ShortcutSettings(QtWidgets.QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        settings = QtCore.QSettings()
        shortcuts = [name.replace("shortcuts/","") for name in filter(lambda name: name.startswith("shortcuts/"), settings.allKeys())]
        widgets = []
        self.lineedits = {}
        for shortcut in shortcuts:
            lineedit = QtWidgets.QLineEdit()
            lineedit.setText(settings.value("shortcuts/" + shortcut))
            self.lineedits["shortcuts/" + shortcut] = lineedit
            widgets.append(ArrangeH(QtWidgets.QLabel(shortcut),lineedit))
        self._scrollArea = QtWidgets.QScrollArea()
        self._innerWidget = QtWidgets.QWidget()
        self._innerWidget.setLayout(ArrangeV(*widgets))
        self._scrollArea.setWidget(self._innerWidget)
        self.setLayout(ArrangeV(self._scrollArea))

    def _save(self):
        settings = QtCore.QSettings()
        for name, edit in self.lineedits.items():
            settings.setValue(name, edit.text())

class SettingDialog(QtWidgets.QDialog):
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.setWindowTitle("Settings")
        self._saveBtn = QtWidgets.QPushButton("Save")
        self._saveBtn.clicked.connect(self._save)
        self._cancelBtn = QtWidgets.QPushButton("Cancel")
        self._cancelBtn.clicked.connect(self.close)
        self.tabber = QtWidgets.QTabWidget(parent=self)
        self._baseSettings = BaseSettings(parent=self.tabber)
        self.tabber.addTab(self._baseSettings, "Base Settings")
        self._shortcutSettings = ShortcutSettings(parent=self.tabber)
        self.tabber.addTab(self._shortcutSettings, "Shortcuts")
        self.setLayout(ArrangeV(self.tabber, ArrangeH(self._saveBtn, self._cancelBtn)))

    def _save(self):
        self._baseSettings._save()
        self._shortcutSettings._save()
        self.close()

class SettingsToolBox(QtWidgets.QToolBox):
    """Global Toolbox on the left hand side, independent of windows. This ToolBox can be used by plugins to
to create Settings which are global to all windows.
"""
    def __init__(self, parent, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.settings = []
        self._splitter=parent

    @inmain_decorator(wait_for_return=False)
    def addSettings(self, sett):
        if not self.settings:
            self._splitter.addWidget(self)
        self.settings.append(sett)
        widget = QtWidgets.QWidget()
        widget.setLayout(ArrangeV(*sett.widgets.groups))
        widget.layout().setAlignment(QtCore.Qt.AlignTop)
        self.addItem(widget, sett.name)
        self.setCurrentIndex(len(self.settings)-1)
