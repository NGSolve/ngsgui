
from . import syntax
from .text_finder import TextFinder
from .button_area import ButtonArea
from .text_partition import Lines, Selection
from ngsolve.gui.widgets import ArrangeH, ArrangeV
from ngsolve.gui.thread import inmain_decorator

from PySide2 import QtWidgets, QtGui, QtCore


class LineNumberArea(QtWidgets.QWidget):
    def __init__(self, editor, *args, **kwargs):
        super().__init__(parent=editor,*args,**kwargs)
        self.editor = editor

    def paintEvent(self,event):
        painter = QtGui.QPainter(self)
        painter.fillRect(event.rect(),QtCore.Qt.lightGray)
        block = self.editor.firstVisibleBlock()
        blockNumber = block.blockNumber()
        top = int(self.editor.blockBoundingGeometry(block).translated(self.editor.contentOffset()).top())
        bottom = top + int(self.editor.blockBoundingRect(block).height())
        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                painter.setPen(QtCore.Qt.black)
                painter.drawText(0,top,self.width(), self.editor.fontMetrics().height(), QtCore.Qt.AlignRight,
                                 str(blockNumber + 1))
            block = block.next()
            top = bottom
            bottom = top + int(self.editor.blockBoundingRect(block).height())
            blockNumber += 1

    def width(self):
        import math
        digits = 1+max(0,int(math.log10(self.editor.blockCount())))
        space = 3 + self.editor.fontMetrics().width("9") * digits
        return space

    def updateWidth(self, newBlockCount):
        self.editor.setViewportMargins(self.width(),self.editor.buttonAreaHeight(),0,0)

    def update(self, rect, dy):
        if dy:
            self.scroll(0,dy)
        else:
            self.editor.update(0,rect.y(), self.width(), rect.height())
        if rect.contains(self.editor.viewport().rect()):
            self.updateWidth(0)



class CodeEditor(QtWidgets.QPlainTextEdit):
    def __init__(self, filename, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.filename = filename
        self.setWindowTitle(filename)
        self.buttonArea = ButtonArea(self)
        self.lineNumberArea = LineNumberArea(self)
        self.blockCountChanged.connect(self.lineNumberArea.updateWidth)
        self.updateRequest.connect(self.lineNumberArea.update)
        self.cursorPositionChanged.connect(self.highlightCurrentLine)
        self._lines = Lines(self)
        with open(filename,"r") as f:
            txt = f.read()
        self.highlighter = syntax.PythonHighlighter(self.document())
        self.text = txt
        self.highlightCurrentLine()
        def setTitleAsterix():
            if self.windowTitle()[0] != "*":
                self.setWindowTitle("* " + self.windowTitle())
        self.textChanged.connect(setTitleAsterix)

        self.comment_action = QtWidgets.QAction("Comment/Uncomment")
        def _comment():
            try:
                Selection(self).commentOrUncomment()
            except ValueError:
                self.selectCurrentLine()
                Selection(self).commentOrUncomment()
        self.comment_action.triggered.connect(_comment)
        self.comment_action.setShortcut(QtGui.QKeySequence("Ctrl+c"))
        # somehow this doesn't work...
        self.addAction(self.comment_action)

    @property
    @inmain_decorator(wait_for_return=True)
    def text(self):
        return self.toPlainText()
    @text.setter
    @inmain_decorator(wait_for_return=True)
    def text(self, text):
        self.setPlainText(text)

    @property
    def lines(self):
        return self._lines

    def selectCurrentLine(self):
        cursor = self.textCursor()
        cursor.movePosition(cursor.StartOfLine)
        cursor.movePosition(cursor.EndOfLine, cursor.KeepAnchor)
        self.setTextCursor(cursor)

    def isGLWindow(self):
        return False

    def contextMenuEvent(self, event):
        # is there a selection
        menu = self.createStandardContextMenu()
        run_section = menu.addAction("Run selection")
        if not self.textCursor().hasSelection():
            run_section.setDisabled(True)
        def _run():
            self.settings.computation_started_at = self.textCursor().selectionStart()
            self.settings.run(self.textCursor().selection().toPlainText())
        run_section.triggered.connect(_run)
        menu.addAction(self.comment_action)
        menu.exec_(event.globalPos())

    def save(self):
        if self.windowTitle()[0] == "*":
            with open(self.filename,"w") as f:
                f.write(self.text)
            self.setWindowTitle(self.windowTitle()[2:])

    def run(self, code, exec_locals):
        exec(code, exec_locals)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        cr = self.contentsRect()
        self.lineNumberArea.setGeometry(QtCore.QRect(cr.left(), cr.top() + self.buttonAreaHeight(), self.lineNumberArea.width(), cr.height()))
        self.buttonArea.setGeometry(QtCore.QRect(cr.left(), cr.top(), cr.right(), self.buttonAreaHeight()))

    def buttonAreaHeight(self):
        return 30

    def highlightCurrentLine(self):
        selection = QtWidgets.QTextEdit.ExtraSelection()
        lineColor = QtGui.QColor(QtCore.Qt.yellow).lighter(160)
        selection.format.setBackground(lineColor)
        selection.format.setProperty(QtGui.QTextFormat.FullWidthSelection, True)
        selection.cursor = self.textCursor()
        selection.cursor.clearSelection()
        self.setExtraSelections([selection])

