
from . import glmath
from . import widgets as wid
from . import scenes
from . import gl as mygl
from .widgets import ArrangeV, ArrangeH
from qtconsole.inprocess import QtInProcessRichJupyterWidget

import time
from ngsolve.bla import Vector
from math import exp

from PySide2 import QtWidgets, QtOpenGL, QtCore
from OpenGL import GL


class SceneToolBox(QtWidgets.QToolBox):
    def __init__(self, window):
        super().__init__()
        self.window = window
        self.scenes = []

    def addScene(self, scene):
        self.scenes.append(scene)
        widget = QtWidgets.QWidget()
        layout = QtWidgets.QVBoxLayout()
        layout.setAlignment(QtCore.Qt.AlignTop)
        for item in scene.getQtWidget(self.window.glWidget.updateGL,
                                      self.window.glWidget.rendering_parameters).groups:
            layout.addWidget(item)
        widget.setLayout(layout)
        self.addItem(widget,scene.name)
        self.setCurrentIndex(len(self.scenes)-1)

class RenderingParameters:
    def __init__(self):
        self.rotmat = glmath.Identity()
        self.zoom = 0.0
        self.ratio = 1.0
        self.dx = 0.0
        self.dy = 0.0
        self.min = Vector(3)
        self.min[:] = 0.0
        self.max = Vector(3)
        self.max[:] = 0.0

        self.clipping_rotmat = glmath.Identity()
        self.clipping_normal = Vector(4)
        self.clipping_normal[0] = 1.0
        self.clipping_point = Vector(3)
        self.clipping_dist = 0.0

    @property
    def center(self):
        return 0.5*(self.min+self.max)

    @property
    def model(self):
        mat = glmath.Identity();
        mat = self.rotmat*mat;
        mat = glmath.Translate(self.dx, -self.dy, -0 )*mat;
        mat = glmath.Scale(exp(-self.zoom/100))*mat;
        mat = glmath.Translate(0, -0, -5 )*mat;
        mat = mat*glmath.Translate(-self.center[0], -self.center[1], -self.center[2]) #move to center
        return mat

    @property
    def view(self):
        return glmath.LookAt()

    @property
    def projection(self):
        return glmath.Perspective(0.8, self.ratio, .1, 20.);

    @property
    def clipping_plane(self):
        x = self.clipping_rotmat * self.clipping_normal
        d = glmath.Dot(self.clipping_point,x[0:3])
        x[3] = -d
        x[3] = x[3]-self.clipping_dist
        return x

    def getClippingPlaneNormal(self):
        x = self.clipping_rotmat * self.clipping_normal
        return x[0:3]

    def getClippingPlanePoint(self):
        return self.clipping_point

    def setClippingPlaneNormal(self, normal):
        for i in range(3):
            self.clipping_normal[i] = normal[i]
        self.clipping_rotmat = glmath.Identity()

    def setClippingPlanePoint(self, point):
        for i in range(3):
            self.clipping_point[i] = point[i]

class MainWindow(QtWidgets.QMainWindow):

    def __init__(self,multikernel_manager,console, shared):
        super(MainWindow, self).__init__()
        self.multikernel_manager = multikernel_manager

        self.scenes = []

        f = QtOpenGL.QGLFormat()
        f.setVersion(3,2)
        f.setProfile(QtOpenGL.QGLFormat.CoreProfile)
        QtOpenGL.QGLFormat.setDefaultFormat(f)

        self.glWidget = GLWidget(shared=shared)
        if shared is None:
            self.glWidget.context().setFormat(f)
            self.glWidget.context().create()

        buttons = QtWidgets.QVBoxLayout()

        btnZoomReset = QtWidgets.QPushButton("ZoomReset", self)
        btnZoomReset.clicked.connect(self.glWidget.ZoomReset)
        btnQuit = QtWidgets.QPushButton("Quit", self)
        btnQuit.clicked.connect(self.close)

        self.colormapSettings = wid.ColorMapSettings(min=-2, max=2, min_value=-1, max_value=1)
        self.colormapSettings.layout().setAlignment(QtCore.Qt.AlignTop)

        buttons.addWidget(btnZoomReset)
        buttons.addWidget(btnQuit)

        self.toolbox = SceneToolBox(self)

        mainWidget = QtWidgets.QSplitter()
        settings = QtWidgets.QWidget()
        settings.setLayout( ArrangeV(self.toolbox, buttons))
        mainWidget.addWidget(settings)
        if console:
            self.kernel_id = self.multikernel_manager.start_kernel()
            kernel_manager = self.multikernel_manager.get_kernel(self.kernel_id)
            class dummyioloop():
                def call_later(self,a,b):
                    return
                def stop(self):
                    return
            kernel_manager.kernel.io_loop = dummyioloop()
            console_and_gl = QtWidgets.QSplitter()
            console_and_gl.setOrientation(QtCore.Qt.Vertical)
            console_and_gl.addWidget(self.glWidget)
            kernel_client = kernel_manager.client()
            kernel_client.start_channels()
            console = QtInProcessRichJupyterWidget()
            console.kernel_manager = kernel_manager
            console.kernel_client = kernel_client
            console.exit_requested.connect(self.close)
            console_and_gl.addWidget(console)
            console_and_gl.setStretchFactor(0,3)
            console_and_gl.setStretchFactor(1,1)
            mainWidget.addWidget(console_and_gl)
        else:
            mainWidget.addWidget(self.glWidget)
        self.setCentralWidget(mainWidget)

        self.setWindowTitle(self.tr("Pyside2 GL"))
        self.last = time.time()
        self.overlay = scenes.OverlayScene(name="Global options")
        self.draw(self.overlay)

    def draw(self, scene):
        self.scenes.append(scene)
        scene.setWindow(self)
        self.glWidget.makeCurrent()
        scene.update()
        self.glWidget.addScene(scene)
        self.toolbox.addScene(scene)
        self.overlay.addScene(scene)

    def deleteScene(self, scene):
        # TODO
        pass

    def redraw(self, blocking=True):
        if time.time() - self.last < 0.02:
            return
        if blocking:
            self.glWidget.redraw_mutex.lock()
            self.glWidget.redraw_signal.emit()
            self.glWidget.redraw_update_done.wait(self.glWidget.redraw_mutex)
            self.glWidget.redraw_mutex.unlock()
        else:
            self.glWidget.redraw_signal.emit()
        self.last = time.time()

    def keyPressEvent(self, event):
        if event.key() == 16777216:
            self.close()

class GLWidget(QtOpenGL.QGLWidget):
    redraw_signal = QtCore.Signal()

    def ZoomReset(self):
        self.rendering_parameters.rotmat = glmath.Identity()
        self.rendering_parameters.zoom = 0.0
        self.rendering_parameters.dx = 0.0
        self.rendering_parameters.dy = 0.0
        self.updateGL()

    def __init__(self, parent=None,shared=None):
        QtOpenGL.QGLWidget.__init__(self, parent=parent,shareWidget=shared)
        self.scenes = []
        self.do_rotate = False
        self.do_translate = False
        self.do_zoom = False
        self.do_move_clippingplane = False
        self.do_rotate_clippingplane = False
        self.old_time = time.time()
        self.rendering_parameters = RenderingParameters()

        self.redraw_update_done = QtCore.QWaitCondition()
        self.redraw_mutex = QtCore.QMutex()

        self.redraw_signal.connect(self.updateScenes)

        self.lastPos = QtCore.QPoint()

    def minimumSizeHint(self):
        return QtCore.QSize(50, 50)

    def sizeHint(self):
        return QtCore.QSize(400, 400)

    def initializeGL(self):
        self.updateScenes()

    def updateScenes(self):
        self.redraw_mutex.lock()
        self.makeCurrent()
        for scene in self.scenes:
            scene.update()
        self.redraw_update_done.wakeAll()
        self.redraw_mutex.unlock()
        self.update()

    def paintGL(self):
        t = time.time() - self.old_time
#         print("frames per second: ", 1.0/t, end='\r')
        self.old_time = time.time()


        GL.glClearColor( 1, 1, 1, 0)
        GL.glClear(GL.GL_COLOR_BUFFER_BIT|GL.GL_DEPTH_BUFFER_BIT)
        GL.glEnable(GL.GL_DEPTH_TEST)
        GL.glDepthFunc(GL.GL_LEQUAL)
        GL.glEnable(GL.GL_BLEND);
        GL.glBlendFunc(GL.GL_SRC_ALPHA, GL.GL_ONE_MINUS_SRC_ALPHA);
        with mygl.Query(GL.GL_PRIMITIVES_GENERATED) as q:
            for scene in self.scenes:
                scene.render(self.rendering_parameters) #model, view, projection)
        # print('\rtotal trigs drawn ' + str(q.value)+' '*10, end='')

    def addScene(self, scene):
        self.scenes.append(scene)
        self.scenes.sort(key=lambda x: x.deferRendering())
        box_min = Vector(3)
        box_max = Vector(3)
        box_min[:] = 1e99
        box_max[:] = -1e99
        for scene in self.scenes:
            s_min, s_max = scene.getBoundingBox()
            for i in range(3):
                box_min[i] = min(s_min[i], box_min[i])
                box_max[i] = max(s_max[i], box_max[i])
        self.rendering_parameters.min = box_min
        self.rendering_parameters.max = box_max

    def mouseDoubleClickEvent(self, event):
        import OpenGL.GLU
        viewport = GL.glGetIntegerv( GL.GL_VIEWPORT )
        x = event.pos().x()
        y = viewport[3]-event.pos().y()
        GL.glReadBuffer(GL.GL_FRONT);
        z = GL.glReadPixels(x, y, 1, 1, GL.GL_DEPTH_COMPONENT, GL.GL_FLOAT)
        params = self.rendering_parameters
        p = OpenGL.GLU.gluUnProject(
                x,y,z,
                (params.view*params.model).T.NumPy(),
                params.projection.T.NumPy(),
                viewport,
                )
        for scene in self.scenes:
            scene.doubleClickAction(p)


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
            if event.button() == QtCore.Qt.MouseButton.RightButton:
                self.do_move_clippingplane = True
            if event.button() == QtCore.Qt.MouseButton.LeftButton:
                self.do_rotate_clippingplane = True
        else:
            if event.button() == QtCore.Qt.MouseButton.LeftButton:
                self.do_rotate = True
            if event.button() == QtCore.Qt.MouseButton.MidButton:
                self.do_translate = True
            if event.button() == QtCore.Qt.MouseButton.RightButton:
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
            shift = -dy/s*param.getClippingPlaneNormal()
            p = param.getClippingPlanePoint()
            param.setClippingPlanePoint(p+shift)
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
