#!/usr/bin/python3

# -*- coding: utf-8 -*-
import sys, os
import ngsgui.gui as G
import ngsolve as ngs
import netgen.meshing



def main():
    G.gui = G.GUI()
    G.gui.parseFlags(sys.argv[1:])
    G.gui.run()

if __name__ == "__main__":
    main()
