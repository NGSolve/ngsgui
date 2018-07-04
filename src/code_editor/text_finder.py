
from PySide2 import QtWidgets, QtGui
from ngsgui.widgets import ArrangeH, ArrangeV
from ngsgui.thread import inmain_decorator

class TextFinder(QtWidgets.QDialog):
    def __init__(self,editor, *args,**kwargs):
        super().__init__(parent=editor, *args,**kwargs)
        self.editor = editor
        label = QtWidgets.QLabel("Find:")
        textedit = QtWidgets.QLineEdit()
        btn_next = QtWidgets.QPushButton("Next")
        btn_up = QtWidgets.QPushButton("Up")
        btn_close = QtWidgets.QPushButton("Close")
        btn_goto = QtWidgets.QPushButton("GoTo")
        def _go_to():
            searchString = textedit.text().lower()
            if not searchString or not self.editor.highlighter._nfound:
                return
            text = self.editor.document().toPlainText().lower()
            index = text.find(searchString,0)
            length = len(searchString)
            count = 0
            while index >= 0:
                if self.editor.highlighter.findRule[3]%self.editor.highlighter._nfound == count:
                    break
                count += 1
                index = text.find(searchString, index + length)
            cursor = self.editor.textCursor()
            cursor.setPosition(index)
            self.close()
            self.editor.setFocus()
            self.editor.setTextCursor(cursor)
        btn_goto.clicked.connect(_go_to)
        def _jump_next():
            searchString = textedit.text().lower()
            self.editor.highlighter.nextFindRulePosition()
        btn_next.clicked.connect(_jump_next)
        def _jump_back():
            searchString = textedit.text().lower()
            self.editor.highlighter.lastFindRulePosition()
        btn_up.clicked.connect(_jump_back)
        def _highlight():
            searchString = textedit.text()
            if not searchString:
                self.editor.highlighter.clearFindRule()
            else:
                self.editor.highlighter.setFindRule(searchString,'cyan','yellow')
        textedit.textChanged.connect(_highlight)
        btn_close.clicked.connect(self.close)
        btn_next.setShortcut(QtGui.QKeySequence("Ctrl+f"))
        btn_up.setShortcut(QtGui.QKeySequence("Ctrl+r"))
        self.setLayout(ArrangeH(label,textedit,btn_goto, btn_next,btn_up,btn_close))

    @inmain_decorator(True)
    def close(self):
        self.editor.highlighter.clearFindRule()
        super().close()
