from os import environ as _environ
# _environ["QT_API"] = "pyside2"
# _environ["QT_API"] = "pyqt5"
# needed when pyside2 is used, order in which qt is loaded seems to be important
# otherwise crash in setting.py:856 when loading the matplotlib colormaps
try:
    import matplotlib.pyplot as plt
except:
    pass

if 'NGS_DEBUG' in _environ:
    import OpenGL
    OpenGL.FULL_LOGGING = True
    _debug=True
else:
    _debug=False

from .scenes import *
from .gui import GUI 

