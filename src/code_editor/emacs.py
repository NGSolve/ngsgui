
from PySide2 import QtGui, QtWidgets, QtCore
from ngsgui.widgets import ArrangeH, ArrangeV
from .utils import PythonFileButtonArea
from ngsgui.thread import inmain_decorator, inthread
from epc.server import ThreadingEPCServer
import logging, threading, os, time, weakref
from .baseEditor import BaseEditor

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
        @inmain_decorator(True)
        def switchTabWindow(direction):
            tabber = self.editor().gui.window_tabber
            tabber.setCurrentIndex((tabber.currentIndex() + direction)%tabber.count())
        def nextTab():
            switchTabWindow(1)
        def previousTab():
            switchTabWindow(-1)
        self.register_function(nextTab)
        self.register_function(previousTab)
        @inmain_decorator(False)
        def activateConsole():
            gui = self.editor().gui
            gui.output_tabber.setCurrentWidget(gui.console)
            gui.console._control.setFocus()
        self.register_function(activateConsole)

class EmacsEditor(QtWidgets.QWidget, BaseEditor):
    def __init__(self, filename=None, gui=None, *args, **kwargs):
        QtWidgets.QWidget.__init__(self, *args, **kwargs)
        BaseEditor.__init__(self, filename, gui)
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

    def _resize_emacs(self):
        while not self._server.clients:
            time.sleep(0.1)
        self._server.clients[0].call("set-width", [int(self.geometry().width()*0.97)])
        self._server.clients[0].call("set-height", [int(self.geometry().height()*0.92)])


    def resizeEvent(self, event):
        super().resizeEvent(event)
        inthread(self._resize_emacs)

    def save(self):
        pass

    def run(self, filename=None, *args, **kwargs):
        filename = filename or self.filename
        with open(filename,"r") as f:
            code = f.read()
        BaseEditor.run(self, code,True)

