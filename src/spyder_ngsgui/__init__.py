""""Spyder NGSolve Plugin"""

from .ngsgui_plugin import NGSolvePlugin as PLUGIN_CLASS
from ngsgui.__main__ import Draw, Redraw
import ngsolve
ngsolve.Draw = Draw
ngsolve.Redraw = Redraw

PLUGIN_CLASS
