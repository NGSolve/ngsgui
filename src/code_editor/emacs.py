
from PySide2 import QtGui, QtWidgets, QtCore
from ngsgui.widgets import ArrangeH, ArrangeV
from .utils import PythonFileButtonArea
from ngsgui.thread import inmain_decorator, inthread
from epc.server import ThreadingEPCServer
import logging, threading, os, time, weakref

emacs_script = os.path.join(os.path.dirname(os.path.abspath(__file__)),"emacs-integration.el")

class EmacsProcess(QtCore.QProcess):
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)

    def start(self, winId, filename, port):
        super().start('emacs --eval "(setq portnumber ' + str(port) + ')" --load ' + emacs_script + ' --maximized --parent-id ' + str(winId) + ' ' + filename)


class MyEPCServer(ThreadingEPCServer):
    def __init__(self, editor):
        super().__init__(('localhost',0), log_traceback=True)
        self.editor = weakref.ref(editor)
        self.logger.setLevel(logging.WARNING)
        self.server_thread = threading.Thread(target=self.serve_forever)
        self.server_thread.allow_reuse_address = True
        self.server_thread.start()
        def run(buffer_filename):
            self.editor().run(buffer_filename)
        self.register_function(run)

class EmacsEditor(QtWidgets.QWidget):
    def __init__(self, filename=None, gui=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.filename = filename
        self.gui = gui
        self.setWindowTitle("emacs")
        self.buttonArea = PythonFileButtonArea(code_editor=self, parent=self, search_button=False)
        self.buttonArea.setFixedHeight(35)
        self.active_thread = None
        self._server = MyEPCServer(self)
        gui.app.aboutToQuit.connect(self._server.shutdown)
        self._emacs_window = QtGui.QWindow()
        self._emacs_widget = QtWidgets.QWidget.createWindowContainer(self._emacs_window)
        self.proc = EmacsProcess(self._emacs_window)
        self.proc.start(self._emacs_window.winId(), filename, self._server.server_address[1])
        gui._procs.append(self.proc)
        self.setLayout(ArrangeV(self.buttonArea, self._emacs_widget))
        inthread(self._resize_emacs)

    def _resize_emacs(self):
        while not self._server.clients:
            time.sleep(0.1)
        self._server.clients[0].call("set-width", [int(self.geometry().width()*0.97)])
        self._server.clients[0].call("set-height", [int(self.geometry().height()*0.92)])


    def resizeEvent(self, event):
        super().resizeEvent(event)
        inthread(self._resize_emacs)

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

    def run(self, filename=None, *args, **kwargs):
        filename = filename or self.filename
        with open(filename,"r") as f:
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
