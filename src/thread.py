import threading
import inspect
import ctypes
import psutil

def _async_raise(tid, exctype):
    '''Raises an exception in the threads with id tid'''
    if not inspect.isclass(exctype):
        raise TypeError("Only types can be raised (not instances)")
    res = ctypes.pythonapi.PyThreadState_SetAsyncExc(ctypes.c_long(tid),ctypes.py_object(exctype))
    if res == 0:
        raise ValueError("invalid thread id")
    elif res != 1:
        # "if it returns a number greater than one, you're in trouble,
        # and you should call it again with exc=NULL to revert the effect"
        ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, 0)
        raise SystemError("PyThreadState_SetAsyncExc failed")

class ThreadWithExc(threading.Thread):
    '''A thread class that supports raising exception in the thread from
       another thread.
    '''

    def raiseExc(self, exctype):
        """Raises the given exception type in the context of this thread.

        If the thread is busy in a system call (time.sleep(),
        socket.accept(), ...), the exception is simply ignored.

        If you are sure that your exception should terminate the thread,
        one way to ensure that it works is:

            t = ThreadWithExc( ... )
            ...
            t.raiseExc( SomeException )
            while t.isAlive():
                time.sleep( 0.1 )
                t.raiseExc( SomeException )

        If the exception is to be caught by the thread, you need a way to
        check that your thread has caught it.

        CAREFUL : this function is executed in the context of the
        caller thread, to raise an excpetion in the context of the
        thread represented by this instance.
        """
        _async_raise( self._id, exctype )

def inthread(func, *args, **kwargs):
    """A convenience function for starting a killable Python thread.

    This function launches a Python thread in Daemon mode, and returns a
    reference to the running thread object.

    Arguments:
        f: A reference to the target function to be executed in the Python thread.

        *args: Any arguments to pass to :code:`f` when it is executed in the
               new thread.

        **kwargs: Any keyword arguments to pass to :code:`f` when it is executed
                  in the new thread.

    Returns:
        A reference to the (already running) Python thread object
    """
    def set_id_and_run():
        thread._id = threading.get_ident()
        func(*args,**kwargs)
    thread = ThreadWithExc(target=set_id_and_run)
    thread.daemon = True
    thread.start()
    return thread


# the following code is copied from qtutils, because it doesn't support PySide2 yet and throws if it doesn't find
# any other qt binding library. For documentation have a look at qtutils

from queue import Queue
from qtutils.qt.QtCore import QEvent, QObject, QCoreApplication, QTimer, QThread
import functools

def _reraise(exc_info):
    type, value, traceback = exc_info
    raise value.with_traceback(traceback)

def get_inmain_result(queue):
    result, exception = queue.get()
    if exception is not None:
        _reraise(exception)
    return result

class CallEvent(QEvent):
    """An event containing a request for a function call."""
    EVENT_TYPE = QEvent.Type(QEvent.registerEventType())

    def __init__(self, queue, exceptions_in_main, fn, *args, **kwargs):
        QEvent.__init__(self, self.EVENT_TYPE)
        self.fn = fn
        self.args = args
        self.kwargs = kwargs
        self._returnval = queue
        # Whether to raise exceptions in the main thread or store them
        # for raising in the calling thread:
        self._exceptions_in_main = exceptions_in_main

class Caller(QObject):
    """An event handler which calls the function held within a CallEvent."""

    def event(self, event):
        event.accept()
        exception = None
        try:
            result = event.fn(*event.args, **event.kwargs)
        except Exception:
            # Store for re-raising the exception in the calling thread:
            exception = sys.exc_info()
            result = None
            if event._exceptions_in_main:
                # Or, if nobody is listening for this exception,
                # better raise it here so it doesn't pass
                # silently:
                raise
        finally:
            event._returnval.put([result, exception])
        return True

caller = Caller()

def _in_main_later(fn, exceptions_in_main, *args, **kwargs):
    queue = Queue()
    QCoreApplication.postEvent(caller, CallEvent(queue, exceptions_in_main, fn, *args, **kwargs))
    return queue

def inmain(fn, *args, **kwargs):
    if threading.current_thread().name == 'MainThread':
        return fn(*args, **kwargs)
    return get_inmain_result(_in_main_later(fn, False, *args, **kwargs))


def inmain_decorator(wait_for_return=True, exceptions_in_main=True):
    def wrap(fn):
        """A decorator which sets any function to always run in the main thread."""
        @functools.wraps(fn)
        def f(*args, **kwargs):
            if wait_for_return:
                return inmain(fn, *args, **kwargs)
            return _in_main_later(fn, exceptions_in_main, *args, **kwargs)
        return f
    return wrap
