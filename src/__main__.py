#!/usr/bin/python3
# -*- coding: utf-8 -*-

def Draw(obj, *args, tab=None, **kwargs):
    """Draw a Mesh or a CoefficientFunction, this function is overridden by
    the new gui and returns the drawn scene."""
    import ngsgui.gui as G
    for t in type(obj).__mro__:
        if t in G.GUI.sceneCreators:
            scene = G.GUI.sceneCreators[t](obj,*args,**kwargs)
            break
    else:
        print("Cannot draw object of type ",type(obj))
        return
    G.gui.draw(scene, tab=tab)
    return scene

_last_time = 0
def Redraw(blocking=False,**kwargs):
    from time import time
    import ngsgui.gui as G
    global _last_time
    if blocking:
        G.gui.redraw_blocking()
        G.gui.app.processEvents()
    else:
        t = time()
        if t-_last_time > 0.02:
            G.gui.app.processEvents()
            G.gui.redraw()
            _last_time = t

def main():
    import sys
    import ngsolve as ngs
    import ngsgui.gui as G
    ngs.Draw = Draw
    ngs.Redraw = Redraw
    G.gui = G.GUI()
    G.gui._run()

if __name__ == "__main__":
    main()
