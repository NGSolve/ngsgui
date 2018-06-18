
from PySide2 import QtWidgets, QtCore, QtGui
from ngsolve.gui.widgets import ArrangeH, ArrangeV
from ngsolve.gui.config import icon_path
from .text_finder import TextFinder

class ButtonArea(QtWidgets.QWidget):
    def __init__(self, editor, *args, **kwargs):
        super().__init__(parent=editor, *args,**kwargs)
        self.editor = editor
        pal = QtGui.QPalette()
        pal.setColor(QtGui.QPalette.Background,QtCore.Qt.black)
        self.setPalette(pal)
        savebtn = QtWidgets.QPushButton()
        savebtn.setIcon(QtGui.QIcon(icon_path + "/save.png"))
        savebtn.setIconSize(QtCore.QSize(20,20))
        savebtn.clicked.connect(self.editor.save)
        savebtn.setToolTip("Save")
        runbtn = QtWidgets.QPushButton()
        runbtn.setIcon(QtGui.QIcon(icon_path + "/run.png"))
        runbtn.setIconSize(QtCore.QSize(40,40))
        runbtn.setToolTip("Run")
        runbtn.clicked.connect(self.editor.run)
        def _run_cursor():
            txt = ""
            block = self.editor.textCursor().block()
            while block != self.editor.document().end():
                txt += block.text() + "\n"
                block = block.next()
            self.editor.run(txt,reset_exec_locals=False,computation_started_at=self.editor.textCursor().position())
        runbtn_cursor = QtWidgets.QPushButton("Run from Cursor")
        runbtn_cursor.clicked.connect(_run_cursor)
        run_line = QtWidgets.QPushButton("Run line")
        def _run_line():
            self.editor.run(self.editor.textCursor().block().text(), reset_exec_locals=False,
                            computation_started_at = self.editor.textCursor().position())
            self.editor.moveCursor(QtGui.QTextCursor.Down)
        run_line.clicked.connect(_run_line)
        find_btn = QtWidgets.QPushButton()
        find_btn.setIcon(QtGui.QIcon(icon_path + "/search.png"))
        find_btn.setIconSize(QtCore.QSize(17,17))
        find_btn.clicked.connect(lambda : TextFinder(self.editor).show())
        savebtn.setShortcut(QtGui.QKeySequence("Ctrl+s"))
        runbtn.setShortcut(QtGui.QKeySequence("Ctrl+r"))
        runbtn_cursor.setShortcut(QtGui.QKeySequence("Ctrl+Shift+r"))
        run_line.setShortcut("Ctrl+l")
        find_btn.setShortcut(QtGui.QKeySequence("Ctrl+f"))
        layout = ArrangeH(savebtn, runbtn, runbtn_cursor,run_line,find_btn)
        self.setContentsMargins(-20,-20,15,-20)
        self.setLayout(layout)
