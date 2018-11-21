import os
os.environ['NGSGUI_HEADLESS'] = "1"
del os

import ngsolve, weakref, time
import ngsgui.gui as G

G._load_plugins()

_ngs_drawn_objects = []
def Draw(*args, **kwargs):
    scene = G._createScene(*args,**kwargs)
    vals = scene.objectsToUpdate()
    index = len(_ngs_drawn_objects)
    _ngs_drawn_objects.append([weakref.ref(val) if not (val is None) else lambda : None for val in vals])
    get_ipython().kernel.send_spyder_msg("ngsolve_draw", None, [index, args, kwargs])
    

_last_time = False
def Redraw(*args, **kwargs):
    global _last_time
    t = time.time()
    if t-_last_time > 0.02:
        # only send the signal, the spyder plugin will query all still drawn objects
        get_ipython().kernel.send_spyder_msg("ngsolve_redraw", None, [[val() for val in obj] for obj in _ngs_drawn_objects])
        _last_time = t

ngsolve.Draw = Draw
ngsolve.Redraw = Redraw
