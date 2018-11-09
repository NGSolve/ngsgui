
from qtpy import QtWidgets, QtGui, QtCore
from ngsgui.widgets import ArrangeH, ArrangeV, ButtonArea


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

class PythonFileButtonArea(ButtonArea):
    def __init__(self, code_editor, search_button=True, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.code_editor = code_editor
        self.addButton(code_editor.save, icon="save.png", description="Save file", shortcut="Ctrl+s")
        self.addButton(code_editor.run, icon="run.png", description="Run file", icon_size=(40,40),
                       shortcut="Ctrl+r")
        if search_button:
            self.addButton(code_editor.search, icon="search.png", icon_size=(17,17), shortcut="Ctrl+f")
