#!/usr/bin/env python
import sys
import math
try:
    from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
except:
    from PyQt5 import QtCore, QtGui, QtWidgets, QtOpenGL
import ngui

try:
    from OpenGL import GL
except ImportError:
    app = QtWidgets.QApplication(sys.argv)
    messageBox = QtWidgets.QMessageBox(QtWidgets.QMessageBox.Critical, "OpenGL hellogl",
                                       "PyOpenGL must be installed to run this example.",
                                       QtWidgets.QMessageBox.Close)
    messageBox.setDetailedText("Run:\npip install PyOpenGL PyOpenGL_accelerate")
    messageBox.exec_()
    sys.exit(1)


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()

        mainWidget = QtWidgets.QWidget()
        self.setCentralWidget(mainWidget)

        f = QtOpenGL.QGLFormat.defaultFormat()
        f.setVersion(4,2)
        QtOpenGL.QGLFormat.setDefaultFormat(f)


        self.glWidget = GLWidget()

        buttons = QtWidgets.QVBoxLayout()

        btnZoomReset = QtWidgets.QPushButton("ZoomReset", self)
        btnZoomReset.clicked.connect(self.glWidget.ZoomReset)
        btnQuit = QtWidgets.QPushButton("Quit", self)
        btnQuit.clicked.connect(self.close)

        buttons.addWidget(btnZoomReset)
        buttons.addWidget(btnQuit)

        mainLayout = QtWidgets.QHBoxLayout()
        mainLayout.addLayout(buttons,0)
        mainLayout.addWidget(self.glWidget, 1)
        mainWidget.setLayout(mainLayout)

        self.setWindowTitle(self.tr("Pyside2 GL"))

    def keyPressEvent(self, event):
        if event.key() == 16777216:
            self.close()

class GLWidget(QtOpenGL.QGLWidget):
    def ZoomReset(self):
        self.ngui.ZoomReset()
        self.updateGL()

    def __init__(self, parent=None):
        QtOpenGL.QGLWidget.__init__(self, parent)


        self.object = 0
        self.xRot = 0
        self.yRot = 0
        self.zRot = 0

        self.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.lastPos = QtCore.QPoint()

        self.trolltechGreen = QtGui.QColor.fromCmykF(0.40, 0.0, 1.0, 0.0)
        self.trolltechPurple = QtGui.QColor.fromCmykF(0.39, 0.39, 0.0, 0.0)

    def minimumSizeHint(self):
        return QtCore.QSize(50, 50)

    def sizeHint(self):
        return QtCore.QSize(400, 400)

    def initializeGL(self):
        ngui.Init()
        self.ngui = ngui.GUI()
        self.ngui.AddScene(ngui.SolutionScene(self.gf))

    def paintGL(self):
        self.ngui.Update()
        self.ngui.Render()

    def resizeGL(self, width, height):
        side = min(width, height)
        self.ngui.SetSize(width, height)

    def mousePressEvent(self, event):
        self.lastPos = QtCore.QPoint(event.pos())
        self.ngui.MouseClick(int(event.button()), True)

    def mouseReleaseEvent(self, event):
        self.ngui.MouseClick(int(event.button()), False)

    def mouseMoveEvent(self, event):
        dx = event.x() - self.lastPos.x()
        dy = event.y() - self.lastPos.y()
        self.ngui.MouseMove(dx, dy)
        self.lastPos = QtCore.QPoint(event.pos())
        self.updateGL()

    def wheelEvent(self, event):
        d = event.angleDelta()
        dx = d.x()
        dy = d.y()
        self.ngui.MouseClick(2, True)
        self.ngui.MouseMove(dx, -dy//10)
        self.ngui.MouseClick(2, False)
        self.updateGL()


    def freeResources(self):
        self.makeCurrent()

class GUI():
    def __init__(self):
        self.app = QtWidgets.QApplication(sys.argv)
        self.window = MainWindow()

    def draw(self, gf):
        self.window.glWidget.gf = gf

    def run(self):
        self.window.show()
        res = self.app.exec_()
        self.window.glWidget.freeResources()
        sys.exit(res)
