from ngsolve import *
from PySide2 import QtGui, QtCore

width = 1024
height = 768

from ngsolve.gui import gui

def TestImage(filename, width=width, height=height):
    gui.renderToImage(width, height,'out/'+filename)
    im = QtGui.QImage('out/'+filename)
    im_ref = QtGui.QImage('ref/'+filename)
    im_diff = QtGui.QImage(im)
    im_diff.fill(QtCore.Qt.white)
    if im == im_ref:
        print('working')
    else:
        print('difference')
        for i in range(im.width()):
            for j in range(im.height()):
                p1 = im.pixelColor(i,j).getRgb()
                p2 = im_ref.pixelColor(i,j).getRgb()
                diff = [255-(1+abs(a-b))//2 for a,b in zip(p1,p2)]
                c = QtGui.QColor.fromRgb(*diff)
                im_diff.setPixelColor(i,j, c)
        im_diff.save('diff/'+filename)
        print('done')


mesh = Mesh('cube.vol.gz')
s = Draw(mesh)
TestImage('mesh_2d.png')
