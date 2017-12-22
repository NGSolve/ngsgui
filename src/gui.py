#!/usr/bin/env python
import sys
import math
import OpenGL.GL as GL
from math import exp
import time
from ngsolve.bla import Vector

from . import glmath

try:
    from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
    from PySide2.QtCore import Qt
except:
    from PyQt5 import QtCore, QtGui, QtWidgets, QtOpenGL
    from PyQt5.QtCore import Qt

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

    def setIntValue(self, int_value):
        float_value = int_value*1.0/self.scalingFactor
        self.valueBox.setValue(float_value)

    def setValue(self, float_value):
        int_value = round(self.scalingFactor*float_value)
        self.scroll.setValue(int_value)
        self.valueChanged.emit(float_value)

class ColorMapSettings(QtWidgets.QWidget):
    linearChanged = QtCore.Signal(bool)

    def __init__(self, min=-1, max=1, min_value=0, max_value=1, direction=Qt.Horizontal):
        super(ColorMapSettings, self).__init__()

        self.rangeMin = RangeGroup("Min", min, max, min_value, direction)
        self.minChanged = self.rangeMin.valueChanged
        self.rangeMax = RangeGroup("Max", min, max, max_value, direction)
        self.maxChanged = self.rangeMax.valueChanged

        self.linear = QtWidgets.QCheckBox('Linear', self)
        self.linear.stateChanged.connect( lambda state: self.linearChanged.emit(state==Qt.Checked))

        self.setLayout( ArrangeV( self.rangeMin, self.rangeMax, self.linear ))

        self.rangeMin.setValue(min_value)
        self.rangeMax.setValue(max_value)

class RenderingParameters:
    view = glmath.Identity()
    rotmat = glmath.Identity()
    zoom = 0.0
    ratio = 1.0
    dx = 0.0
    dy = 0.0

    clipping_rotmat = glmath.Identity()
    clipping_dist = 0.0

    @property
    def model(self):
        mat = glmath.Identity();
        mat = self.rotmat*mat;
        mat = glmath.Translate(self.dx, -self.dy, -0 )*mat;
        mat = glmath.Scale(exp(-self.zoom/100))*mat;
        mat = glmath.Translate(0, -0, -5 )*mat;
        return mat

    @property
    def view(self):
        return glmath.LookAt()

    @property
    def projection(self):
        return glmath.Perspective(0.8, self.ratio, .1, 20.);

    def clipping_plane(self, center=None):
        x = Vector(4);
        x[:] = 0.0
        x[2] = 1
        x = self.clipping_rotmat * x
        if center:
            d = glmath.Dot(center,x[0:3])
            x[3] = -d
        x[3] = x[3]-self.clipping_dist
        return x


class MainWindow(QtWidgets.QMainWindow):

    def __init__(self):
        super(MainWindow, self).__init__()

        mainWidget = QtWidgets.QWidget()
        self.setCentralWidget(mainWidget)

        f = QtOpenGL.QGLFormat()
        f.setVersion(3,2)
        f.setProfile(QtOpenGL.QGLFormat.CoreProfile)
        QtOpenGL.QGLFormat.setDefaultFormat(f)


        self.glWidget = GLWidget()
        self.glWidget.context().setFormat(f)
        self.glWidget.context().create()
        print(self.glWidget.context().format())

        buttons = QtWidgets.QVBoxLayout()

        btnZoomReset = QtWidgets.QPushButton("ZoomReset", self)
        btnZoomReset.clicked.connect(self.glWidget.ZoomReset)
        btnQuit = QtWidgets.QPushButton("Quit", self)
        btnQuit.clicked.connect(self.close)
        
        self.colormapSettings = ColorMapSettings(min=-2, max=2, min_value=-1, max_value=1)
        self.colormapSettings.layout().setAlignment(Qt.AlignTop)

        buttons.addWidget(btnZoomReset)
        buttons.addWidget(btnQuit)

#         self.settings = QtWidgets.QVBoxLayout()
        self.settings = QtWidgets.QToolBox()
#         self.settings.addLayout(buttons)

        mainLayout = QtWidgets.QHBoxLayout()
        mainLayout.addLayout( ArrangeV(self.settings, buttons),1)
        mainLayout.addWidget(self.glWidget, 3)
        mainWidget.setLayout(mainLayout)

        self.setWindowTitle(self.tr("Pyside2 GL"))
        from . import shader
        shader.printLimits()

    def keyPressEvent(self, event):
        if event.key() == 16777216:
            self.close()

class GLWidget(QtOpenGL.QGLWidget):
    scenes = []
    do_rotate = False
    do_translate = False
    do_zoom = False
    do_move_clippingplane = False
    do_rotate_clippingplane = False
    old_time = time.time()
    rendering_parameters = RenderingParameters()

    redraw_signal = QtCore.Signal()
    redraw_update_done = QtCore.QWaitCondition()
    redraw_mutex = QtCore.QMutex()

    def ZoomReset(self):
        self.rendering_parameters.rotmat = glmath.Identity()
        self.rendering_parameters.zoom = 0.0
        self.rendering_parameters.dx = 0.0
        self.rendering_parameters.dy = 0.0
        self.updateGL()

    def __init__(self, parent=None):
        QtOpenGL.QGLWidget.__init__(self, parent)

        self.redraw_signal.connect(self.updateScenes)

        self.lastPos = QtCore.QPoint()

#         self.image = QtGui.QImage(200, 50, QtGui.QImage.Format_ARGB32_Premultiplied)
#         self.image.fill(0)
#         self.painter = QtGui.QPainter(self.image)
#         painter.setRenderHints(QtGui.QPainter.TextAntialiasing | QtGui.QPainter.Antialiasing | QtGui.QPainter.SmoothPixmapTransform)
#         painter.setPen(QtCore.Qt.NoPen)
# 
#         painter.setBrush(QtGui.QColor(0, 0, 0))
#         painter.setFont(Colors.tickerFont())
#         painter.setPen(QtGui.QColor(255, 255, 255))
#         self.painter.drawText(0, 0, "h")
#         self.painter.end()
#         print(self.image)

    def minimumSizeHint(self):
        return QtCore.QSize(50, 50)

    def sizeHint(self):
        return QtCore.QSize(400, 400)

    def initializeGL(self):
        pass

    def updateScenes(self):
        self.redraw_mutex.lock()
        for scene in self.scenes:
            scene.update()
        self.redraw_update_done.wakeAll()
        self.redraw_mutex.unlock()
        self.update()

    def paintGL(self):
        t = time.time() - self.old_time
        print("frames per second: ", 1.0/t, end='\r')
        self.old_time = time.time()


        GL.glClearColor( 1, 1, 1, 0)
        GL.glClear(GL.GL_COLOR_BUFFER_BIT|GL.GL_DEPTH_BUFFER_BIT)
        GL.glEnable(GL.GL_DEPTH_TEST)
        GL.glDepthFunc(GL.GL_LEQUAL)
        GL.glPolygonOffset (-1, -1)
        GL.glEnable(GL.GL_POLYGON_OFFSET_LINE)
        GL.glEnable(GL.GL_BLEND);
        GL.glBlendFunc(GL.GL_SRC_ALPHA, GL.GL_ONE_MINUS_SRC_ALPHA);
        for scene in self.scenes:
            scene.render(self.rendering_parameters) #model, view, projection)


########################
# font 
#         painter = QtGui.QPainter(self)
#         painter.drawLine(0, 0, 1, 1);
#         painter.end()

# ########################
#         GL.glUseProgram(0)
#         GL.glDisable(GL.GL_DEPTH_TEST)
#         GL.glMatrixMode(GL.GL_PROJECTION)
#         GL.glOrtho( -.5, .5, .5, -.5, -1000, 1000)
#         GL.glMatrixMode(GL.GL_MODELVIEW)
#         GL.glLoadIdentity()
# #         GL.glClearColor(1.0, 1.0, 1.0, 1.0)
# 
# 
#         GL.glClear(GL.GL_COLOR_BUFFER_BIT)
#       
#         self.qglColor(QtCore.Qt.black)
#         self.renderText(0.0, 0.0, 0.0, "Multisampling enabled")
# #         self.renderText(0.15, 0.4, 0.0, "Multisampling disabled")
########################

    def resizeGL(self, width, height):
        GL.glViewport(0, 0, width, height)
        self.rendering_parameters.ratio = width/height

    def mousePressEvent(self, event):
        self.lastPos = QtCore.QPoint(event.pos())
        if event.modifiers() == QtCore.Qt.ControlModifier:
            if event.button() == Qt.MouseButton.RightButton:
                self.do_move_clippingplane = True
            if event.button() == Qt.MouseButton.LeftButton:
                self.do_rotate_clippingplane = True
        else:
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
        self.do_move_clippingplane = False
        self.do_rotate_clippingplane = False

    def mouseMoveEvent(self, event):
        dx = event.x() - self.lastPos.x()
        dy = event.y() - self.lastPos.y()
        param = self.rendering_parameters
        if self.do_rotate:
            param.rotmat = glmath.RotateY(-dx/50.0)*param.rotmat
            param.rotmat = glmath.RotateX(-dy/50.0)*param.rotmat
        if self.do_translate:
            s = 200.0*exp(-param.zoom/100)
            param.dx += dx/s
            param.dy += dy/s
        if self.do_zoom:
            param.zoom += dy
        if self.do_move_clippingplane:
            s = 200.0*exp(-param.zoom/100)
            param.clipping_dist += dy/s
        if self.do_rotate_clippingplane:
            # rotation of clipping plane is view-dependent
            r = param.rotmat
            param.clipping_rotmat = r.T*glmath.RotateY(-dx/50.0)*r*param.clipping_rotmat
            param.clipping_rotmat = r.T*glmath.RotateX(-dy/50.0)*r*param.clipping_rotmat
        self.lastPos = QtCore.QPoint(event.pos())
        self.updateGL()

    def wheelEvent(self, event):
        self.rendering_parameters.zoom -= event.angleDelta().y()/10
        self.updateGL()

    def freeResources(self):
        self.makeCurrent()

class GUI():
    def __init__(self):
        self.app = QtWidgets.QApplication(sys.argv)
        self.window = MainWindow()
        self.window.show()
        self.window.raise_()
        self.last = time.time()


    def draw(self, scene):
        scene.update()
        self.window.glWidget.scenes.append(scene)
        self.window.settings.addItem(scene.getQtWidget(self.window.glWidget.updateGL),"Colormap")

    def redraw(self, blocking=True):
        if time.time() - self.last < 0.02:
            return
        if blocking:
            self.window.glWidget.redraw_mutex.lock()
            self.window.glWidget.redraw_signal.emit()
            self.window.glWidget.redraw_update_done.wait(self.window.glWidget.redraw_mutex)
            self.window.glWidget.redraw_mutex.unlock()
        else:
            self.window.glWidget.redraw_signal.emit()
        self.last = time.time()


    def run(self):
        self.window.show()
        res = self.app.exec_()
        self.window.glWidget.freeResources()
        sys.exit(res)
