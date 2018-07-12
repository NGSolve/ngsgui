#!/usr/bin/python3

# -*- coding: utf-8 -*-
import sys, os
import ngsgui.gui as G
import ngsolve as ngs
import netgen.meshing


def Draw(obj, *args, tab=None, **kwargs):
    """Draw a Mesh or a CoefficientFunction, this function is overridden by
    the new gui and returns the drawn scene."""
    for t in type(obj).__mro__:
        if t in G.GUI.sceneCreators:
            scene = creator(obj,*args,**kwargs)
    else:
        print("Cannot draw object of type ",type(obj))
        return
    G.gui.draw(scene, tab=tab)
    return scene

def Redraw(blocking=True,**kwargs):
    if blocking:
        G.gui.redraw_blocking()
    else:
        G.gui.redraw()



def main():
    ngs.Draw = Draw
    ngs.Redraw = Redraw
    G.gui = G.GUI()
    G.gui._parseFlags(sys.argv[1:])
    G.gui._run()

if __name__ == "__main__":
    main()
