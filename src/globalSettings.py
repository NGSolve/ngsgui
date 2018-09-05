
from PySide2 import QtWidgets, QtCore
from .widgets import ArrangeH, ArrangeV


class BaseSettings(QtWidgets.QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self._editorCB = QtWidgets.QComboBox()
        self._editorCB.addItems(["default","emacs"])
        settings = QtCore.QSettings()
        self._editorCB.setCurrentText(settings.value("editor/type", "default"))
        self._editorCB.currentTextChanged.connect(self._checkValidSelection)
        self._sysmonCB = QtWidgets.QCheckBox("System monitor")
        self._sysmonCB.setChecked(settings.value("sysmon/active", "false") == "true")
        self._sysmonCB.stateChanged.connect(self._checkSysmonRequirement)
        self.setLayout(ArrangeV(QtWidgets.QLabel("Settings will get active after restarting the GUI"),
                                ArrangeH(QtWidgets.QLabel("Editor:"), self._editorCB),
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
                self._editorCB.setCurrentText("default")
                self.msgbox = QtWidgets.QMessageBox(text="""Cannot embed emacs without epc, please install it with
pip3 install --user epc""")
                self.msgbox.show()

    def _save(self):
        settings = QtCore.QSettings()
        settings.setValue("editor/type", self._editorCB.currentText())
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
