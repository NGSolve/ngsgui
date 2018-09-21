from IPython import get_ipython
ipython = get_ipython()
ipython.magic('gui qt5')
import time

import ngsgui.gui as G
import ngsolve as ngs
from ngsgui.__main__ import Draw,Redraw

ngs.Draw = Draw
ngs.Redraw = Redraw
G.gui = G.GUI(flags=['--noOutputpipe', '--noConsole','--noEditor'])
G.gui._run(run_event_loop=False)
