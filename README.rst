
ngsgui - A new graphical user interface for NGSolve
===================================================

This project aims to eliminate some weaknesses of the 20 year old GUI interface of Netgen/NGSolve, which was created primarily for mesh generation and not for the complexities of the visualization of multiphysic FEM computation. The code is written in Python using the Qt5 bindings of PySide2 and shader code for OpenGL.

Some of the features it offers are:

- Faster rendering using modern OpenGL features
- Drawing of functions on different meshes at the same time
- Visualization of multiple functions in same/different windows and from different perspectives
- Easy verification of boundary conditiones/ material properties
- Saving state as is - including camera perspective,...
- Easy extendability using ngsgui.plugin entry points
- And many more...

Getting started
----------------
- Install dependencies:

  `pip3 install --index-url=http://download.qt.io/snapshots/ci/pyside/5.11/latest/ pyside2 --trusted-host download.qt.io`

- Install via pip:

  `pip3 install ngsgui`

- Install current master branch (may contain some fixes or some new bugs...)

  `pip3 install git+https://github.com/NGSolve/ngsgui.git@master`

- Run `ngsolve` or try to load some python file: `ngsolve example.py`

- On some systems you have to set `PATH` in order to find `ngsolve`, an alternative way to start the GUI is

  `python3 -m ngsgui`

Troubleshooting
---------------

This project is in a very eary stage, so problems will arise very likely. If you run into any, report an Issue on GitHub, explaining the problem as precisely as possible, send us some reproducible code or run it with 

`ngsolve yourfile.py -noOutputpipe -dontCatchExceptions`

and attach the stdout output if possible.
