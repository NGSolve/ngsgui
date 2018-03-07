
from . import syntax
from .widgets import ArrangeH, ArrangeV

from PySide2 import QtWidgets, QtGui, QtCore
from qtutils import inmain_decorator

class LineNumberArea(QtWidgets.QWidget):
    def __init__(self, editor, *args, **kwargs):
        super().__init__(parent=editor,*args,**kwargs)
        self.editor = editor

    def paintEvent(self,event):
        self.editor.lineNumberAreaPaintEvent(event)

class ButtonArea(QtWidgets.QWidget):
    def __init__(self, editor, *args, **kwargs):
        super().__init__(parent=editor, *args,**kwargs)
        self.editor = editor
        pal = QtGui.QPalette()
        pal.setColor(QtGui.QPalette.Background,QtCore.Qt.black)
        self.setPalette(pal)
        savebtn = QtWidgets.QPushButton("Save")
        savebtn.setShortcut(QtGui.QKeySequence("Ctrl+s"))
        savebtn.clicked.connect(self.editor.save)
        runbtn = QtWidgets.QPushButton("Run")
        def _run():
            self.editor.settings.computation_started_at = 0
            self.editor.settings.run(self.editor.toPlainText())
        runbtn.setShortcut(QtGui.QKeySequence("Ctrl+x"))
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
        runbtn_cursor.setShortcut(QtGui.QKeySequence("Ctrl+Shift+x"))
        runbtn_cursor.clicked.connect(_run_cursor)
        run_line = QtWidgets.QPushButton("Run line")
        run_line.setShortcut("Ctrl+l")
        def _run_line():
            self.editor.settings.computation_started_at = self.editor.textCursor().position()
            self.editor.settings.run(self.editor.textCursor().block().text())
            self.editor.moveCursor(QtGui.QTextCursor.Down)
        run_line.clicked.connect(_run_line)
        layout = ArrangeH(savebtn, runbtn, runbtn_cursor,run_line)
        self.setContentsMargins(-5,-5,15,-5)
        self.setLayout(layout)

class CodeEditor(QtWidgets.QPlainTextEdit):
    def __init__(self, filename, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.filename = filename
        self.setWindowTitle(filename)
        self.buttonArea = ButtonArea(self)
        self.lineNumberArea = LineNumberArea(self)
        self.blockCountChanged.connect(self.updateLineNumberAreaWidth)
        self.updateRequest.connect(self.updateLineNumberArea)
        self.cursorPositionChanged.connect(self.highlightCurrentLine)
        with open(filename,"r") as f:
            txt = f.read()
        highlight = syntax.PythonHighlighter(self.document())
        self.setPlainText(txt)
        def setTitleAsterix():
            if self.windowTitle()[0] != "*":
                self.setWindowTitle("* " + self.windowTitle())
        self.textChanged.connect(setTitleAsterix)
        self.highlightCurrentLine()

    def isGLWindow(self):
        return False

    def contextMenuEvent(self, event):
        menu = self.createStandardContextMenu()
        run_section = menu.addAction("Run selection")
        def _run():
            self.settings.computation_started_at = self.textCursor().selectionStart()
            self.settings.run(self.textCursor().selection().toPlainText())
        run_section.triggered.connect(_run)
        menu.exec_(event.globalPos())

    def save(self):
        if self.windowTitle()[0] == "*":
            with open(self.filename,"w") as f:
                f.write(self.toPlainText())
            self.setWindowTitle(self.windowTitle()[2:])

    @inmain_decorator(wait_for_return=True)
    def toPlainText(self):
        return super().toPlainText()

    def run(self, code, exec_locals):
        exec(code, exec_locals)

    def lineNumberAreaPaintEvent(self, event):
        painter =  QtGui.QPainter(self.lineNumberArea)
        painter.fillRect(event.rect(),QtCore.Qt.lightGray)
        block = self.firstVisibleBlock()
        blockNumber = block.blockNumber()
        top = int(self.blockBoundingGeometry(block).translated(self.contentOffset()).top())
        bottom = top + int(self.blockBoundingRect(block).height())
        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                painter.setPen(QtCore.Qt.black)
                painter.drawText(0,top, self.lineNumberArea.width(), self.fontMetrics().height(),
                                 QtCore.Qt.AlignRight, str(blockNumber + 1))
            block = block.next()
            top = bottom
            bottom = top + int(self.blockBoundingRect(block).height())
            blockNumber += 1

    def lineNumberAreaWidth(self):
        import math
        digits = 1+max(0,int(math.log10(self.blockCount())))
        space = 3 + self.fontMetrics().width("9") * digits
        return space

    def resizeEvent(self, event):
        super().resizeEvent(event)
        cr = self.contentsRect()
        self.lineNumberArea.setGeometry(QtCore.QRect(cr.left(), cr.top() + self.buttonAreaHeight(), self.lineNumberAreaWidth(), cr.height()))
        self.buttonArea.setGeometry(QtCore.QRect(cr.left(), cr.top(), cr.right(), self.buttonAreaHeight()))

    def buttonAreaHeight(self):
        return 30

    def updateLineNumberAreaWidth(self, newBlockCount):
        self.setViewportMargins(self.lineNumberAreaWidth(),self.buttonAreaHeight(),0,0)

    def highlightCurrentLine(self):
        selection = QtWidgets.QTextEdit.ExtraSelection()
        lineColor = QtGui.QColor(QtCore.Qt.yellow).lighter(160)
        selection.format.setBackground(lineColor)
        selection.format.setProperty(QtGui.QTextFormat.FullWidthSelection, True)
        selection.cursor = self.textCursor()
        selection.cursor.clearSelection()
        self.setExtraSelections([selection])

    def updateLineNumberArea(self, rect, dy):
        if dy:
            self.lineNumberArea.scroll(0,dy)
        else:
            self.lineNumberArea.update(0,rect.y(), self.lineNumberArea.width(), rect.height())
        if rect.contains(self.viewport().rect()):
            self.updateLineNumberAreaWidth(0)
