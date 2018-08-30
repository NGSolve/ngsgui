
from PySide2 import QtGui, QtWidgets, QtCore
from ngsgui.widgets import ArrangeH, ArrangeV
from .utils import PythonFileButtonArea
from ngsgui.thread import inmain_decorator, inthread

class EmacsProcess(QtCore.QProcess):
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)

    def start(self, winId, filename):
        super().start("emacs --parent-id " + str(winId) + " " + filename)

class EmacsEditor(QtWidgets.QWidget):
    def __init__(self, filename=None, gui=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.filename = filename
        self.gui = gui
        self.setWindowTitle("emacs")
        self.buttonArea = PythonFileButtonArea(code_editor=self, parent=self, search_button=False)
        self.buttonArea.setFixedHeight(35)
        self.active_thread = None
        self._emacs_window = QtGui.QWindow()
        self._emacs_widget = QtWidgets.QWidget.createWindowContainer(self._emacs_window)
        self.proc = EmacsProcess(self._emacs_window)
        self.proc.start(self._emacs_window.winId(), filename)
        self.setLayout(ArrangeV(self.buttonArea, self._emacs_widget))

    @inmain_decorator(True)
    def show_exception(self, e, lineno):
        self.gui.window_tabber.setCurrentWidget(self)
        self.msgbox = QtWidgets.QMessageBox(text = type(e).__name__ + ": " + str(e))
        self.msgbox.setWindowTitle("Exception caught!")
        self.msgbox.show()
        if self.gui._dontCatchExceptions:
            raise e

    def save(self):
        pass

    def run(self, *args, **kwargs):
        with open(self.filename,"r") as f:
            code = f.read()
        self.exec_locals = { "__name__" : "__main__" }
        def _run():
            try:
                exec(code,self.exec_locals)
            except Exception as e:
                import sys
                count_frames = 0
                tbc = sys.exc_info()[2]
                while tbc is not None:
                    tb = tbc
                    tbc = tb.tb_next
                self.show_exception(e,tb.tb_frame.f_lineno)
            self.active_thread = None
            self.gui.console.pushVariables(self.exec_locals)
        if self.active_thread:
            self.msgbox = QtWidgets.QMessageBox(text="Already running, please stop the other computation before starting a new one!")
            self.msgbox.setWindowTitle("Multiple computations error")
            self.msgbox.show()
            return
        self.active_thread = inthread(_run)

    def isGLWindow(self):
        return False
