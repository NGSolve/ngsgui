
from PySide2 import QtGui

class Lines:
    """ List like object giving linewise access to editor """
    def __init__(self, editor):
        self.editor = editor

    def __getitem__(self, item):
        """ Get line by line number"""
        if isinstance(item, slice):
            return SlicedLines(self,item)
        else:
            return self._getblock(item).text()

    def __setitem__(self, item, value):
        cur = QtGui.QTextCursor(self._getblock(item))
        cur.select(cur.BlockUnderCursor)
        cur.removeSelectedText()
        cur.insertText("\n" + value)

    def __iter__(self):
        block = self.editor.document().firstBlock()
        while block.isValid():
            yield block.text()
            block = block.next()

    def _getblock(self, index):
        return self.editor.document().findBlockByLineNumber(index)

    def __len__(self):
        return self.editor.blockCount()

    def __str__(self):
        string = ""
        for line in self:
            string += line + "\n"
        return string


class SlicedLines(Lines):
    """ Sliced Lines object """
    def __init__(self, lines, _slice):
        super().__init__(lines.editor)
        self._lines = lines
        self._slice = _slice
        self.stop = len(lines) if _slice.stop is None else _slice.stop
        self.start = 0 if _slice.start is None else _slice.start
        self.step = 1 if _slice.step is None else _slice.step

    def _getblock(self, index):
        return self._lines._getblock(self.start + self.step * index)

    def __iter__(self):
        block = self._lines._getblock(self.start)
        index = self.start
        while block.isValid() and index < self.stop:
            yield block.text()
            for i in range(self.step):
                block = block.next()
            index += self.step

    def __len__(self):
        return int((self.stop-self.start)/self.step)

class Selection:
    """ Helper class for cursor selection. Implement all methods that manipulate selected text here """
    def __init__(self, editor):
        self.editor = editor
        self.cursor = self.editor.textCursor()
        if not self.cursor.hasSelection():
            raise ValueError("Nothing selected")
        self.start = self.cursor.selectionStart()
        self.end = self.cursor.selectionEnd()
        self.cursor.setPosition(self.start)
        self.startline = self.cursor.blockNumber()
        self.cursor.setPosition(self.end)
        self.endline = self.cursor.blockNumber()
        self.lines = self.editor.lines[self.startline:self.endline+1]

    def _hasUncommented(self):
        uncommented = False
        for line in self.lines:
            if line and line[0] != "#":
                uncommented = True
        return uncommented

    def commentOrUncomment(self):
        self.cursor.beginEditBlock()
        if self._hasUncommented():
            self.comment()
        else:
            self.uncomment()
        self._markSelection()
        self.cursor.endEditBlock()

    def comment(self):
        for i in range(len(self.lines)):
            if self.lines[i]:
                self.lines[i] = "# " + self.lines[i]

    def uncomment(self):
        for i in range(len(self.lines)):
            txt = self.lines[i]
            if txt and txt[0] == "#":
                if len(txt)>1 and txt[1] == " ":
                    self.lines[i] = self.lines[i][2:]
                else:
                    self.lines[i] = self.lines[i][1:]

    def __str__(self):
        return str(self.lines)

    def _markSelection(self):
        cursor = QtGui.QTextCursor(self.editor.document().findBlockByLineNumber(self.startline))
        self.cursor = cursor
        cursor.movePosition(cursor.Down,cursor.KeepAnchor, self.endline-self.startline)
        cursor.movePosition(cursor.EndOfLine, cursor.KeepAnchor)
        self.editor.setTextCursor(cursor)
