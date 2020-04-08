
from jupyter_client.multikernelmanager import MultiKernelManager
from qtconsole.inprocess import QtInProcessRichJupyterWidget, QtInProcessKernelManager, QtInProcessKernelClient, QtInProcessChannel
from traitlets import DottedObjectName, Type

from .thread import inmain_decorator

# workaround because inprocesskernelclient misses this static member
class FixQtInProcessKernelClient(QtInProcessKernelClient):
    control_channel_class = Type(QtInProcessChannel)

    @property
    def control_channel(self):
        if self._control_channel is None:
            self._control_channel  = self.control_channel_class(self)
        return self._control_channel

class FixQtInProcessKernelManager(QtInProcessKernelManager):
    client_class = __module__ + ".FixQtInProcessKernelClient"

class MultiQtKernelManager(MultiKernelManager):
    kernel_manager_class = DottedObjectName(__module__ + ".FixQtInProcessKernelManager",
                                            config = True,
                                            help = """kernel manager class""")


class NGSJupyterWidget(QtInProcessRichJupyterWidget):
    def __init__(self, gui,*args, **kwargs):
        import ngsolve
        super().__init__(*args,**kwargs)
        self.gui = gui
        self.banner = """NGSolve %s
Developed by Joachim Schoeberl at
2010-xxxx Vienna University of Technology
2006-2010 RWTH Aachen University
1996-2006 Johannes Kepler University Linz

""" % ngsolve.__version__
        multikernel_manager = gui.multikernel_manager
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
        self.exit_requested.connect(stop)

    @inmain_decorator(wait_for_return=True)
    def pushVariables(self, varDict):
        self.kernel_manager.kernel.shell.push(varDict)

    @inmain_decorator(wait_for_return=True)
    def clearTerminal(self):
        self._control.clear()
