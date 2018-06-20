
from jupyter_client.multikernelmanager import MultiKernelManager
from qtconsole.inprocess import QtInProcessRichJupyterWidget
from traitlets import DottedObjectName

import ngsolve
from .thread import inmain_decorator

from IPython.lib import guisupport

class MultiQtKernelManager(MultiKernelManager):
    kernel_manager_class = DottedObjectName("qtconsole.inprocess.QtInProcessKernelManager",
                                            config = True,
                                            help = """kernel manager class""")


class NGSJupyterWidget(QtInProcessRichJupyterWidget):
    def __init__(self, gui,multikernel_manager,*args, **kwargs):
        super().__init__(*args,**kwargs)
        self.gui = gui
        self.banner = """NGSolve %s
Developed by Joachim Schoeberl at
2010-xxxx Vienna University of Technology
2006-2010 RWTH Aachen University
1996-2006 Johannes Kepler University Linz

""" % ngsolve.__version__
        if multikernel_manager is not None:
            self.kernel_id = multikernel_manager.start_kernel()
            self.kernel_manager = multikernel_manager.get_kernel(self.kernel_id)
        else:
            self.kernel_manager = QtInProcessKernelManager()
            self.kernel_manager.start_kernel()
        self.kernel_manager.kernel.gui = 'qt'
        self.kernel_client = self.kernel_manager.client()
        self.kernel_client.start_channels()
        class dummyioloop():
            def call_later(self,a,b):
                return
            def stop(self):
                return
        self.kernel_manager.kernel.io_loop = dummyioloop()

        def stop():
            self.kernel_client.stop_channels()
            self.kernel_manager.shutdown_kernel()
            self.gui.app.quit()
        self.exit_requested.connect(stop)

    @inmain_decorator(wait_for_return=True)
    def pushVariables(self, varDict):
        self.kernel_manager.kernel.shell.push(varDict)

    @inmain_decorator(wait_for_return=True)
    def clearTerminal(self):
        self._control.clear()
