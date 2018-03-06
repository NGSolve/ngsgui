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
