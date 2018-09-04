
from PySide2 import QtWidgets, QtCore
from .widgets import ArrangeH, ArrangeV

class SettingDialog(QtWidgets.QDialog):
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.setWindowTitle("Settings")
        self._editorCB = QtWidgets.QComboBox()
        self._editorCB.addItems(["default","emacs"])
        settings = QtCore.QSettings()
        self._editorCB.setCurrentText(settings.value("editor/type", "default"))
        self._editorCB.currentTextChanged.connect(self._checkValidSelection)
        self._sysmonCB = QtWidgets.QCheckBox("System monitor")
        self._sysmonCB.setChecked(settings.value("sysmon/active", "false") == "true")
        self._sysmonCB.stateChanged.connect(self._checkSysmonRequirement)
        self._saveBtn = QtWidgets.QPushButton("Save")
        self._saveBtn.clicked.connect(self._save)
        self._cancelBtn = QtWidgets.QPushButton("Cancel")
        self._cancelBtn.clicked.connect(self.close)
        self.setLayout(ArrangeV(QtWidgets.QLabel("Settings will get active after restarting the GUI"),
                                ArrangeH(QtWidgets.QLabel("Editor:"), self._editorCB),
                                self._sysmonCB,
                                ArrangeH(self._cancelBtn, self._saveBtn)))

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
        self.close()
