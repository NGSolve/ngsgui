#!/usr/bin/env python
import sys
import math
import OpenGL.GL as GL
import glmath
from math import exp

try:
    from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
    from PySide2.QtCore import Qt
except:
    from PyQt5 import QtCore, QtGui, QtWidgets, QtOpenGL
    from PyQt5.QtCore import Qt
# import ngui

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

def ArrangeV(*args):
    layout = QtWidgets.QVBoxLayout()
    for w in args:
        if isinstance(w, QtWidgets.QWidget):
            layout.addWidget(w)
        else:
            layout.addLayout(w)
    return layout

def ArrangeH(*args):
    layout = QtWidgets.QHBoxLayout()
    for w in args:
        if isinstance(w, QtWidgets.QWidget):
            layout.addWidget(w)
        else:
            layout.addLayout(w)
    return layout

class RangeGroup(QtWidgets.QWidget):
    valueChanged = QtCore.Signal(float)
    intValueChanged = QtCore.Signal(int)

    scalingFactor = 1000 # scaling between integer widgets (scrollslider) and float values to get more resolution

    def __init__(self, name, min=-1, max=1, value=0, direction=Qt.Horizontal):
        super(RangeGroup, self).__init__()
#         self.valueChanged.connect(onValueChanged)

        self.scroll = QtWidgets.QScrollBar(direction)
        self.scroll.setFocusPolicy(Qt.StrongFocus)
        self.scroll.valueChanged[int].connect(self.setIntValue)
        self.scroll.setRange(self.scalingFactor*min,self.scalingFactor*max)

        self.valueBox = QtWidgets.QDoubleSpinBox()
        self.valueBox.setRange(min,max)
        self.valueBox.valueChanged[float].connect(self.setValue)
        self.valueBox.setSingleStep(0.01*(max-min))

        self.label = QtWidgets.QLabel(name)
        
        self.setLayout(ArrangeV(ArrangeH(self.label, self.valueBox), self.scroll))
        self.scroll.setValue(100)

    def setIntValue(self, int_value, emit=True):
        float_value = int_value*1.0/self.scalingFactor
        self.valueBox.setValue(float_value)
        if emit:
            self.setValue(float_value, False)

    def setValue(self, float_value, emit=True):
        int_value = round(self.scalingFactor*float_value)
#         print('setvalue', float_value, int_value)
        self.scroll.setValue(int_value)
        if emit:
            self.setIntValue(int_value, False)

class MainWindow(QtWidgets.QMainWindow):

    cmMinChanged = QtCore.Signal(int)
    cmMaxChanged = QtCore.Signal(int)

    def onMinChanged(self,value):
        try:
            self.glWidget.scene.colormap_min = value
            self.glWidget.updateGL()
        except:
            pass

    def onMaxChanged(self,value):
        try:
            self.glWidget.scene.colormap_max = value
            self.glWidget.updateGL()
        except:
            pass

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
        
        colormapSettings = ArrangeV(
            RangeGroup("Min", min=-2, max=2, value=-1),
            RangeGroup("Max", min=-2, max=2, value= 1)
        )
        colormapSettings.setAlignment(Qt.AlignTop)

        buttons.addWidget(btnZoomReset)
        buttons.addWidget(btnQuit)

        mainLayout = QtWidgets.QHBoxLayout()
        mainLayout.addLayout(ArrangeV(colormapSettings, buttons),1)
        mainLayout.addWidget(self.glWidget, 3)
        mainWidget.setLayout(mainLayout)

        self.setWindowTitle(self.tr("Pyside2 GL"))

    def keyPressEvent(self, event):
        if event.key() == 16777216:
            self.close()

class GLWidget(QtOpenGL.QGLWidget):
    scenes = []
    view = glmath.Identity()
    rotmat = glmath.Identity()
    do_rotate = False
    do_translate = False
    do_zoom = False
    zoom = 0.0
    translate_x=0.0
    translate_y=0.0
    dx = 0.0
    dy = 0.0
    ratio = 1.0

    def ZoomReset(self):
        self.rotmat = glmath.Identity()
        self.zoom = 0.0
        self.dx = 0.0
        self.dy = 0.0
        self.updateGL()

    def __init__(self, parent=None):
        QtOpenGL.QGLWidget.__init__(self, parent)

#         self.setFocusPolicy(Qt.StrongFocus)
        self.lastPos = QtCore.QPoint()

    def minimumSizeHint(self):
        return QtCore.QSize(50, 50)

    def sizeHint(self):
        return QtCore.QSize(400, 400)

    def initializeGL(self):
        pass

    def paintGL(self):
        model = glmath.Identity();
        model = self.rotmat*model;
        model = glmath.Translate(self.dx, -self.dy, -0 )*model;
        model = glmath.Scale(exp(-self.zoom/100))*model;
        model = glmath.Translate(0, -0, -5 )*model;

        view = glmath.LookAt()
        projection = glmath.Perspective(0.8, self.ratio, .1, 20.);

        GL.glClearColor( 1, 1, 1, 0)
        GL.glClear(GL.GL_COLOR_BUFFER_BIT|GL.GL_DEPTH_BUFFER_BIT)
        GL.glEnable(GL.GL_DEPTH_TEST)
        GL.glDepthFunc(GL.GL_LEQUAL)
        GL.glPolygonOffset (-1, -1)
        GL.glEnable(GL.GL_POLYGON_OFFSET_LINE)
        for scene in self.scenes:
            scene.render(model, view, projection)

    def resizeGL(self, width, height):
        GL.glViewport(0, 0, width, height)
        self.ratio = width/height

    def mousePressEvent(self, event):
        self.lastPos = QtCore.QPoint(event.pos())
        if event.button() == Qt.MouseButton.LeftButton:
            self.do_rotate = True
        if event.button() == Qt.MouseButton.MidButton:
            self.do_translate = True
        if event.button() == Qt.MouseButton.RightButton:
            self.do_zoom = True

    def mouseReleaseEvent(self, event):
        self.do_rotate = False
        self.do_translate = False
        self.do_zoom = False

    def mouseMoveEvent(self, event):
        dx = event.x() - self.lastPos.x()
        dy = event.y() - self.lastPos.y()
        if self.do_rotate:
            self.rotmat = glmath.RotateY(-dx/50.0)*self.rotmat
            self.rotmat = glmath.RotateX(-dy/50.0)*self.rotmat
        if self.do_translate:
            s = 200.0*exp(-self.zoom/100)
            self.dx += dx/s
            self.dy += dy/s
        if self.do_zoom:
            self.zoom += dy
        self.lastPos = QtCore.QPoint(event.pos())
        self.updateGL()

    def wheelEvent(self, event):
        self.zoom -= event.angleDelta().y()/10
        self.updateGL()

    def freeResources(self):
        self.makeCurrent()

class GUI():
    def __init__(self):
        self.app = QtWidgets.QApplication(sys.argv)
        self.window = MainWindow()
        self.window.show()
        self.window.raise_()

    def draw(self, scene):
        scene.update()
        self.window.glWidget.scenes.append(scene)

    def run(self):
        self.window.show()
        res = self.app.exec_()
        self.window.glWidget.freeResources()
        sys.exit(res)
