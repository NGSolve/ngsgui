import os
os.environ['NGSGUI_HEADLESS'] = "1"
del os

import ngsolve, weakref, time
import ngsgui.gui as G

import logging
logger = logging.getLogger(__name__)

G._load_plugins()

class ProxyItem:
    def __init__(self, index, name):
        self.index = index
        self.name = name

    def __set__(self, obj, value):
        get_ipython().get_ipython().kernel.send_spyder_msg("ngsolve_set_scene_item", None, [self.index, self.name,
                                                                                            value])
    def __call__(self, *args, **kwargs):
        get_ipython().get_ipython().kernel.send_spyder_msg("ngsolve_call_scene_item", None, [self.index, self.name,
                                                                                             args, kwargs])

_ngs_drawn_objects = []
def Draw(*args, **kwargs):
    scene = G._createScene(*args,**kwargs)
    index = len(_ngs_drawn_objects)
    _ngs_drawn_objects.append(scene.objectsToUpdate())
    handler = get_ipython().kernel.frontend_comm.remote_call()
    handler.ngsolve_draw(index, *args, **kwargs)
    class ProxyScene:
        pass
    for attr in dir(scene):
        if not attr.startswith("_"):
            setattr(ProxyScene, attr, ProxyItem(index, attr))
    return ProxyScene()
    

_last_time_ngs_draw = 0
def Redraw(*args, fr=25, **kwargs):
    global _last_time_ngs_draw
    t = time.time()
    if (t-_last_time_ngs_draw) * fr > 1:
        # only send the signal, the spyder plugin will query all still drawn objects
        handler = get_ipython().kernel.frontend_comm.remote_call()
        handler.ngsolve_redraw(_ngs_drawn_objects, *args, **kwargs)
        _last_time_ngs_draw = t

ngsolve.Draw = Draw
ngsolve.Redraw = Redraw
