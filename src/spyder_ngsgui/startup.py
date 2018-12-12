import os
os.environ['NGSGUI_HEADLESS'] = "1"
del os

import ngsolve, weakref, time
import ngsgui.gui as G

G._load_plugins()

_ngs_drawn_objects = []
def Draw(*args, **kwargs):
    scene = G._createScene(*args,**kwargs)
    index = len(_ngs_drawn_objects)
    _ngs_drawn_objects.append(scene.objectsToUpdate())
    get_ipython().get_ipython().kernel.send_spyder_msg("ngsolve_draw", None, [index, args, kwargs])
    

_last_time_ngs_draw = False
def Redraw(*args, **kwargs):
    global _last_time_ngs_draw
    t = time.time()
    if t-_last_time_ngs_draw > 0.2:
        # only send the signal, the spyder plugin will query all still drawn objects
        get_ipython().get_ipython().kernel.send_spyder_msg("ngsolve_redraw", None, _ngs_drawn_objects)
        _last_time_ngs_draw = t

ngsolve.Draw = Draw
ngsolve.Redraw = Redraw
