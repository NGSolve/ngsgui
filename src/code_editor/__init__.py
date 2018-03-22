
from . import syntax
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
            self.editor.settings.run(self.editor.toPlainText())
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
        self.highlighter = syntax.PythonHighlighter(self.document())
        self.setPlainText(txt)
        def setTitleAsterix():
            if self.windowTitle()[0] != "*":
                self.setWindowTitle("* " + self.windowTitle())
        self.textChanged.connect(setTitleAsterix)
        self.highlightCurrentLine()

        self.comment_action = QtWidgets.QAction("Comment/Uncomment")
        def _comment():
            cursor = self.textCursor()
            if not cursor.hasSelection():
                cursor.movePosition(cursor.StartOfLine)
                cursor.movePosition(cursor.EndOfLine,cursor.KeepAnchor)
            start = cursor.selectionStart()
            end = cursor.selectionEnd()
            cursor.setPosition(start)
            firstline = cursor.blockNumber()
            cursor.setPosition(end,cursor.KeepAnchor)
            lastline = cursor.blockNumber()
            hasUncommented = False
            for line in range(firstline,lastline+1):
                block = self.document().findBlockByLineNumber(line)
                if block.text() and block.text()[0] != "#":
                    hasUncommented = True
            cursor.beginEditBlock()
            block = self.document().findBlockByLineNumber(firstline)
            linenr = firstline
            while block.isValid() and linenr <= lastline:
                linenr += 1
                cur = QtGui.QTextCursor(block)
                txt = block.text()
                block = block.next()
                cur.select(cur.BlockUnderCursor)
                cur.removeSelectedText()
                if txt:
                    if hasUncommented:
                        cur.insertText("\n# " + txt)
                    else:
                        if txt and txt[0] == "#":
                            if len(txt)>1 and txt[1] == " ":
                                cur.insertText("\n" + txt[2:])
                            else:
                                cur.insertText("\n" + txt[1:])
            cursor.endEditBlock()
            cur = QtGui.QTextCursor(self.document().findBlockByLineNumber(firstline))
            cur.movePosition(cur.Down, cur.KeepAnchor, lastline-firstline)
            cur.movePosition(cur.EndOfLine, cur.KeepAnchor)
            self.setTextCursor(cur)
        self.comment_action.triggered.connect(_comment)
        self.comment_action.setShortcut(QtGui.QKeySequence("Ctrl+c"))
        # somehow this doesn't work...
        self.addAction(self.comment_action)

    @property
    @inmain_decorator(wait_for_return=True)
    def text(self):
        return self.toPlainText()
    @text.setter
    @inmain_decorator(wait_for_return=False)
    def text(self, text):
        self.setPlainText(text)

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
