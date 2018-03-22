
from PySide2 import QtWidgets, QtGui

class TextFinder(QtWidgets.QDialog):
    def __init__(self,editor, *args,**kwargs):
        super().__init__(parent=editor, *args,**kwargs)
        self.editor = editor
        label = QtWidgets.QLabel("Find:")
        textedit = QtWidgets.QLineEdit()
        btn_next = QtWidgets.QPushButton("Next")
        btn_up = QtWidgets.QPushButton("Up")
        btn_close = QtWidgets.QPushButton("Close")
        def _jump_next():
            searchString = textedit.text().lower()
            self.editor.find(searchString)
        btn_next.clicked.connect(_jump_next)
        def _jump_back():
            searchString = textedit.text().lower()
            print(searchString)
            self.editor.find(searchString, QtGui.QTextDocument.FindBackward)
        btn_up.clicked.connect(_jump_back)
        def _highlight():
            searchString = textedit.text()
            if not searchString:
                self.editor.highlighter.clearFindRule()
            else:
                self.editor.highlighter.setFindRule(searchString,'cyan')
        textedit.textChanged.connect(_highlight)
        btn_close.clicked.connect(self.close)
        btn_next.setShortcut(QtGui.QKeySequence("Ctrl+f"))
        btn_up.setShortcut(QtGui.QKeySequence("Ctrl+r"))
        self.setLayout(ArrangeH(label,textedit,btn_next,btn_up,btn_close))

    def close(self):
        self.editor.highlighter.clearFindRule()
        super().close()
