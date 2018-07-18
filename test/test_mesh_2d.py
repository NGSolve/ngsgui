from ngsolve import *
from PySide2 import QtGui, QtCore
import numpy as np

width = 1024
height = 768

from ngsgui.gui import gui

def TestImage(filename, width=width, height=height):
    gui.renderToImage(width, height,'out/'+filename)
    im = QtGui.QImage('out/'+filename)
    im_ref = QtGui.QImage('ref/'+filename)
    im_diff = QtGui.QImage(im)

    np_im = np.asarray(im.bits(), dtype=np.int16)
    np_im_ref = np.asarray(im_ref.bits(), dtype=np.int16)
    np_im_diff = np.asarray(im_diff.bits())
    if im == im_ref:
        print('working')
    else:
        print('difference')
        np_im_diff[:] = 255-abs(np_im-np_im_ref)
        im_diff.save('diff/'+filename)
    print('done')


mesh = Mesh('cube.vol.gz')
s = Draw(mesh)
TestImage('mesh_2d.png')
gui.app.quit()
