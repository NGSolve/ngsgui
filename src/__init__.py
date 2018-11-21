from os import environ as _environ
# _environ["QT_API"] = "pyside2"
# _environ["QT_API"] = "pyqt5"
if 'NGS_DEBUG' in _environ:
    import OpenGL
    OpenGL.FULL_LOGGING = True
    _debug=True
else:
    _debug=False

from .scenes import *
from .gui import GUI 

