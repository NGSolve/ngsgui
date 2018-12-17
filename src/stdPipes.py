
from qtpy import QtCore, QtWidgets, QtGui
from .thread import inmain_decorator
import re, os, time, sys

class Receiver(QtCore.QObject):
    """Class responsible for piping the stdout to the internal output. Removes ansi escape characters.
"""
    received = QtCore.Signal(str)

    def __init__(self,pipe, *args,**kwargs):
        super().__init__(*args,**kwargs)
        self.pipe = pipe
        self.ansi_escape = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')
        self.kill = False

    def SetKill(self):
        self.kill = True
        print("killme")

    def run(self):
        while not self.kill:
            self.received.emit(self.ansi_escape.sub("",os.read(self.pipe,1024).decode("ascii")))
        self.kill = False

class OutputBuffer(QtWidgets.QTextEdit):
    """Textview where the stdoutput is piped into. Is not writable, so stdin is not piped (yet).
"""
    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.setReadOnly(True)
        self._waitForEnter = False

    def start(self):
        sys.stdin = self
        stdout_fileno = sys.stdout.fileno()
        stderr_fileno = sys.stderr.fileno()
        stderr_save = os.dup(stderr_fileno)
        stdout_save = os.dup(stdout_fileno)
        stdout_pipe = os.pipe()
        os.dup2(stdout_pipe[1], stdout_fileno)
        os.dup2(stdout_pipe[1], stderr_fileno)
        os.close(stdout_pipe[1])
        self.receiver = Receiver(stdout_pipe[0])
        self.receiver.received.connect(self.append_text)
        self.stdoutThread = QtCore.QThread()
        self.receiver.moveToThread(self.stdoutThread)
        self.stdoutThread.started.connect(self.receiver.run)
        self.stdoutThread.start()

    def append_text(self, text):
        self.moveCursor(QtGui.QTextCursor.End)
        self.insertPlainText(text)

    def keyPressEvent(self, event):
        if self._waitForEnter:
            if event.key() == 16777220: # enter key
                self._waitForEnter = False
                self.setReadOnly(True)
        super().keyPressEvent(event)

    def onQuit(self):
        self.receiver.SetKill()
        self.stdoutThread.exit()

    @inmain_decorator(True)
    def _setupReadline(self):
        self.setReadOnly(False)
        self.moveCursor(QtGui.QTextCursor.End)
        self.insertPlainText("\n")
        self._waitForEnter = True

    @inmain_decorator(True)
    def _getLastLine(self):
        return self.toPlainText().split("\n")[-1] + "\n"

    def readline(self):
        time.sleep(0.1)
        self._setupReadline()
        while self._waitForEnter:
            time.sleep(0.1)
        return self._getLastLine()
