
from PySide2 import QtWidgets, QtCore, QtGui
from ngsolve.gui.widgets import ArrangeH, ArrangeV

class ButtonArea(QtWidgets.QWidget):
    def __init__(self, editor, *args, **kwargs):
        super().__init__(parent=editor, *args,**kwargs)
        self.editor = editor
        pal = QtGui.QPalette()
        pal.setColor(QtGui.QPalette.Background,QtCore.Qt.black)
        self.setPalette(pal)
        savebtn = QtWidgets.QPushButton("Save")
        savebtn.clicked.connect(self.editor.save)
        runbtn = QtWidgets.QPushButton("Run")
        def _run():
            self.editor.settings.computation_started_at = 0
            self.editor.settings.run(self.editor.text)
        runbtn.clicked.connect(_run)
        def _run_cursor():
            self.editor.settings.computation_started_at = self.editor.textCursor().position()
            txt = ""
            block = self.editor.textCursor().block()
            while block != self.editor.document().end():
                txt += block.text() + "\n"
                block = block.next()
            self.editor.settings.run(txt)
        runbtn_cursor = QtWidgets.QPushButton("Run from Cursor")
        runbtn_cursor.clicked.connect(_run_cursor)
        run_line = QtWidgets.QPushButton("Run line")
        def _run_line():
            self.editor.settings.computation_started_at = self.editor.textCursor().position()
            self.editor.settings.run(self.editor.textCursor().block().text())
            self.editor.moveCursor(QtGui.QTextCursor.Down)
        run_line.clicked.connect(_run_line)
        find_btn = QtWidgets.QPushButton("Find")
        find_btn.clicked.connect(lambda : TextFinder(self.editor).show())
        savebtn.setShortcut(QtGui.QKeySequence("Ctrl+s"))
        runbtn.setShortcut(QtGui.QKeySequence("Ctrl+r"))
        runbtn_cursor.setShortcut(QtGui.QKeySequence("Ctrl+Shift+r"))
        run_line.setShortcut("Ctrl+l")
        find_btn.setShortcut(QtGui.QKeySequence("Ctrl+f"))
        layout = ArrangeH(savebtn, runbtn, runbtn_cursor,run_line,find_btn)
        self.setContentsMargins(-5,-5,15,-5)
        self.setLayout(layout)
