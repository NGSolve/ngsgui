
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
        self._saveBtn = QtWidgets.QPushButton("Save")
        self._saveBtn.clicked.connect(self._save)
        self._cancelBtn = QtWidgets.QPushButton("Cancel")
        self._cancelBtn.clicked.connect(self.close)
        self.setLayout(ArrangeV(ArrangeH(QtWidgets.QLabel("Editor:"), self._editorCB),
                                ArrangeH(self._cancelBtn, self._saveBtn)))

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
        self.close()
