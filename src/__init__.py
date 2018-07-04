from os import environ as _environ
if 'NGS_DEBUG' in _environ:
    import OpenGL
    OpenGL.FULL_LOGGING = True
    _debug=True
else:
    _debug=False

from . import config
from .scenes import *
from .gui import GUI 

