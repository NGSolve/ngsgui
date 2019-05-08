
from . import glmath, scenes
from . import widgets as wid
from .gl import TextRenderer, ArrayBuffer, VertexArray, getProgram, Texture
from .widgets import ArrangeV, ArrangeH, addShortcut
from .thread import inmain_decorator
from .toolbox import SceneToolBox
from ngsgui import _debug
import numpy as np

import time, ngsolve, weakref
from ngsolve.bla import Vector
from math import exp, sqrt

import logging
logger = logging.getLogger(__name__)

from qtpy import QtWidgets, QtOpenGL, QtCore, QtGui
from OpenGL import GL
import pickle

class WindowTab(QtWidgets.QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._startup_scenes = []
        self._actions = []
        settings = QtCore.QSettings()

    def create(self,sharedContext):
        self.glWidget = GLWidget(shared=sharedContext)
        self.glWidget.makeCurrent()

        self.toolbox = SceneToolBox(self.glWidget)
        self.glWidget._settings.window = weakref.ref(self)
        self.glWidget.addScene(self.glWidget._settings)
        self.toolbox.addScene(self.glWidget._settings, visibilityButton=False, colorButton=False, killButton=False)

        splitter = QtWidgets.QSplitter()
        splitter.addWidget(self.glWidget)
        splitter.addWidget(self.toolbox)
        splitter.setOrientation(QtCore.Qt.Horizontal)
        splitter.setSizes([75000, 25000])
        self.setLayout(ArrangeH(splitter))
        for scene in self._startup_scenes:
            self.draw(scene)

    def __getstate__(self):
        return (self.glWidget.scenes)

    def __setstate__(self,state):
        self.__init__()
        self._startup_scenes = state[0]

    def isGLWindow(self):
        return True

    @inmain_decorator(True)
    def draw(self, scene):
        self.glWidget.makeCurrent()
        scene.window = weakref.ref(self)
        scene._global_rendering_parameters = self.glWidget._settings
        scene.update()
        self.glWidget.addScene(scene)
        self.toolbox.addScene(scene)

    @inmain_decorator(True)
    def remove(self, scene):
        """Remove scene from window"""
        logger.debug("Remove scene {}".format(scene.name))
        self.glWidget.makeCurrent()
        scene.window = None
        self.glWidget.scenes.remove(scene)
        self.toolbox.removeScene(scene)
        self.glWidget.updateGL()

class GLWidget(QtOpenGL.QGLWidget):
    _trace_paintGL_calls = False
    def __init__(self,shared=None, rendering_parameters=None, *args, **kwargs):
        f = QtOpenGL.QGLFormat()
        f.setVersion(3,2)
        f.setSamples(8)
        # f.setProfile(QtOpenGL.QGLFormat.CompatibilityProfile)
        f.setProfile(QtOpenGL.QGLFormat.CoreProfile)
        if _debug:
            f.setOption(QtGui.QSurfaceFormat.DebugContext)
        QtOpenGL.QGLFormat.setDefaultFormat(f)
        super().__init__(shareWidget=shared, *args, **kwargs)
        if shared is None:
            self.context().setFormat(f)
            self.context().create()
        self.scenes = []
        self._rotation_enabled = True
        self.do_rotate = False
        self.do_translate = False
        self.do_zoom = False
        self.do_move_clippingplane = False
        self.do_rotate_clippingplane = False
        self.old_time = time.time()

        self.redraw_update_done = QtCore.QWaitCondition()
        self.redraw_mutex = QtCore.QMutex()

        self._settings = scenes.RenderingSettings(name="Global settings") 
        self._settings.interpolationChanged.connect(self.updateScenes)

        self.lastPos = QtCore.QPoint()
        self.lastFastmode = self._settings.fastmode


    @inmain_decorator(True)
    def updateGL(self,*args,**kwargs):
        super().updateGL(*args,**kwargs)

    def ZoomReset(self):
        self._settings.rotmat = glmath.Identity()
        self._settings.zoom = 0.0
        self._settings.dx = 0.0
        self._settings.dy = 0.0
        box_min = Vector(3)
        box_max = Vector(3)
        box_min[:] = 1e99
        box_max[:] = -1e99
        for scene in self.scenes:
            if not scene.active:
                continue
            s_min, s_max = scene.getBoundingBox()
            for i in range(3):
                box_min[i] = min(s_min[i], box_min[i])
                box_max[i] = max(s_max[i], box_max[i])
        self._settings.min = box_min
        self._settings.max = box_max
        self.updateGL()

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
            if scene.active:
                scene.update()
        self.redraw_update_done.wakeAll()
        self.redraw_mutex.unlock()
        self.update()

    def paintGL(self):
        t = time.time() - self.old_time
#         print("frames per second: ", 1.0/t, end='\r')
        self.old_time = time.time()

        if self._trace_paintGL_calls:
            import traceback
            logger.debug("\n".join(["************************ paintGL stack *****************************"]
                                   + traceback.format_stack()))

        GL.glClearColor( 1, 1, 1, 0)
        GL.glClear(GL.GL_COLOR_BUFFER_BIT|GL.GL_DEPTH_BUFFER_BIT)
        GL.glEnable(GL.GL_DEPTH_TEST)
        GL.glDepthFunc(GL.GL_LEQUAL)
        GL.glEnable(GL.GL_BLEND);
        GL.glBlendFunc(GL.GL_SRC_ALPHA, GL.GL_ONE_MINUS_SRC_ALPHA);

        viewport = GL.glGetIntegerv( GL.GL_VIEWPORT )
        screen_width = viewport[2]-viewport[0]
        screen_height = viewport[3]-viewport[1]
        rp = self._settings
        rp.ratio = screen_width/max(screen_height,1)

        for scene in self.scenes:
            if scene.active and hasattr(scene, "individualColormap") and scene.individualColormap and scene.getColormapAutoscale():
                smin, smax = scene.getAutoscaleRange(rp)
                state = self.blockSignals(True)
                scene.setColormapMin(smin)
                scene.setColormapMax(smax)
                self.blockSignals(state)

        if rp.getColormapAutoscale():
            colormap_min = 0
            colormap_max = 1
            # use the autoscale of the last drawn scene
            for scene in self.scenes:
                if scene.active and hasattr(scene, "individualColormap") and not scene.individualColormap:
                    colormap_min, colormap_max = scene.getAutoscaleRange(rp)
            state = self.blockSignals(True)
            rp.setColormapMin(colormap_min)
            rp.setColormapMax(colormap_max)
            self.blockSignals(state)
        for scene in self.scenes[1:]:
            scene.render(rp)
        rp.render(rp)

    def addScene(self, scene):
        self.scenes.append(scene)
        self.scenes.sort(key=lambda x: x.deferRendering())
        box_min = Vector(3)
        box_max = Vector(3)
        box_min[:] = 1e99
        box_max[:] = -1e99
        for scene in self.scenes:
            if not scene.active:
                continue
            s_min, s_max = scene.getBoundingBox()
            for i in range(3):
                box_min[i] = min(s_min[i], box_min[i])
                box_max[i] = max(s_max[i], box_max[i])
        self._settings.min = box_min
        self._settings.max = box_max
        self.updateGL()

    def mouseDoubleClickEvent(self, event):
        import OpenGL.GLU
        viewport = GL.glGetIntegerv( GL.GL_VIEWPORT )
        x = event.pos().x()
        y = viewport[3]-event.pos().y()
        GL.glReadBuffer(GL.GL_FRONT);
        z = GL.glReadPixels(x, y, 1, 1, GL.GL_DEPTH_COMPONENT, GL.GL_FLOAT)
        params = self._settings
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
        self._settings.ratio = width/max(1,height)

    def mousePressEvent(self, event):
        self.lastPos = QtCore.QPoint(event.pos())
        self.lastFastmode = self._settings.fastmode
        self._settings.fastmode = True
        if event.modifiers() == QtCore.Qt.ControlModifier:
            if event.button() == QtCore.Qt.RightButton:
                self.do_move_clippingplane = True
            if event.button() == QtCore.Qt.LeftButton:
                self.do_rotate_clippingplane = True
        else:
            if event.button() == QtCore.Qt.LeftButton:
                if self._rotation_enabled:
                    self.do_rotate = True
            if event.button() == QtCore.Qt.MidButton:
                self.do_translate = True
            if event.button() == QtCore.Qt.RightButton:
                self.do_zoom = True

    def mouseReleaseEvent(self, event):
        param = self._settings
        self.do_rotate_clippingplane = False
        self.do_move_clippingplane = False
        self.do_rotate = False
        self.do_translate = False
        self.do_zoom = False
        if param.fastmode and not self.lastFastmode:
            param.fastmode = False
            self.updateGL()

    def mouseMoveEvent(self, event):
        dx = event.x() - self.lastPos.x()
        dy = event.y() - self.lastPos.y()
        if self.do_rotate:
            self._settings.rotateCamera(dx, dy)
        if self.do_translate:
            self._settings.moveCamera(dx, dy)
        if self.do_zoom:
            self._settings.zoom += dy
        if self.do_move_clippingplane:
            s = 200.0*exp(-self._settings.zoom/100)
            self._settings.moveClippingPoint(-dy/s)
        if self.do_rotate_clippingplane:
            self._settings.rotateClippingNormal(dx, dy, self._settings.rotmat)
        self.lastPos = QtCore.QPoint(event.pos())
        self.updateGL()

    def wheelEvent(self, event):
        self._settings.zoom -= event.angleDelta().y()/10
        self.updateGL()

    def freeResources(self):
        self.makeCurrent()

# attaching / detaching from https://stackoverflow.com/questions/48901854/is-it-possible-to-drag-a-qtabwidget-and-open-a-new-window-containing-whats-in-t


class WindowTabber(QtWidgets.QTabWidget):
    def __init__(self,commonContext, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self._commonContext = commonContext
        self._tabBar = self.TabBar(self)
        self.setTabBar(self._tabBar)
        self.setTabsClosable(True)
        self._tabBar.onDetachTabSignal.connect(self.detachTab)
        self._tabBar.onMoveTabSignal.connect(self.moveTab)
        self._activeGLWindow = None
        self.tabCloseRequested.connect(self._remove_tab)
        self._fastmode = False
        self.setTabBarAutoHide(True)

    ##
    #  The default movable functionality of QTabWidget must remain disabled
    #  so as not to conflict with the added features
    def setMovable(self, movable):
        pass

    #  Move a tab from one position (index) to another
    def moveTab(self, fromIndex, toIndex):
        widget = self.widget(fromIndex)
        icon = self.tabIcon(fromIndex)
        text = self.tabText(fromIndex)

        self.removeTab(fromIndex)
        self.insertTab(toIndex, widget, icon, text)
        self.setCurrentIndex(toIndex)

    def detachTab(self, index, point):
        name = self.tabText(index)
        icon = self.tabIcon(index)
        if icon.isNull():
            icon = self.window().windowIcon()
        contentWidget = self.widget(index)
        contentWidgetRect = contentWidget.frameGeometry()

        # create a new detached tab window
        detachedTab = self.DetachedTab(contentWidget, self.parentWidget())
        # detachedTab = setWindowModality(Qt.NonModal)
        detachedTab.setWindowTitle(name)
        detachedTab.setWindowIcon(icon)
        detachedTab.setObjectName(name)
        detachedTab.setGeometry(contentWidgetRect)
        detachedTab.onCloseSignal.connect(self.attachTab)
        detachedTab.move(point)
        detachedTab.show()

    def attachTab(self, contentWidget, name, icon):
        # Make the content widget a child of this widget
        contentWidget.setParent(self)


        # Create an image from the given icon
        if not icon.isNull():
            tabIconPixmap = icon.pixmap(icon.availableSizes()[0])
            tabIconImage = tabIconPixmap.toImage()
        else:
            tabIconImage = None


        # Create an image of the main window icon
        if not icon.isNull():
            windowIconPixmap = self.window().windowIcon().pixmap(icon.availableSizes()[0])
            windowIconImage = windowIconPixmap.toImage()
        else:
            windowIconImage = None


        # Determine if the given image and the main window icon are the same.
        # If they are, then do not add the icon to the tab
        if tabIconImage == windowIconImage:
            index = self.addTab(contentWidget, name)
        else:
            index = self.addTab(contentWidget, icon, name)


        # Make this tab the current tab
        if index > -1:
            self.setCurrentIndex(index)

    ##
    #  When a tab is detached, the contents are placed into this QDialog.  The tab
    #  can be re-attached by closing the dialog or by double clicking on its
    #  window frame.
    class DetachedTab(QtWidgets.QDialog):
        onCloseSignal = QtCore.Signal(object,object,object)

        def __init__(self, contentWidget, parent=None):
            super().__init__(parent)
            self.setLayout(ArrangeV(contentWidget))
            self.contentWidget = contentWidget
            self.contentWidget.show()
            self.setWindowFlags(QtCore.Qt.Window)

        def event(self, event):

            # If the event type is QEvent.NonClientAreaMouseButtonDblClick then
            # close the dialog
            if event.type() == QtCore.QEvent.NonClientAreaMouseButtonDblClick:
                event.accept()
                self.close()

            return super().event(event)

        def closeEvent(self, event):
            self.onCloseSignal.emit(self.contentWidget, self.objectName(), self.windowIcon())

            ##
    #  The TabBar class re-implements some of the functionality of the QTabBar widget
    class TabBar(QtWidgets.QTabBar):
        onDetachTabSignal = QtCore.Signal(int, object)
        onMoveTabSignal = QtCore.Signal(int, int)

        def __init__(self, parent=None):
            super().__init__(parent)

            self.setAcceptDrops(True)
            self.setElideMode(QtCore.Qt.ElideRight)
            self.setSelectionBehaviorOnRemove(QtWidgets.QTabBar.SelectLeftTab)

            self.dragStartPos = QtCore.QPoint()
            self.dragDropedPos = QtCore.QPoint()
            self.mouseCursor = QtGui.QCursor()
            self.dragInitiated = False

        #  Send the onDetachTabSignal when a tab is double clicked
        def mouseDoubleClickEvent(self, event):
            event.accept()
            self.onDetachTabSignal.emit(self.tabAt(event.pos()), self.mouseCursor.pos())

        #  Set the starting position for a drag event when the mouse button is pressed
        def mousePressEvent(self, event):
            if event.button() == QtCore.Qt.LeftButton:
                self.dragStartPos = event.pos()

            self.dragDropedPos.setX(0)
            self.dragDropedPos.setY(0)

            self.dragInitiated = False

            super().mousePressEvent(event)

        #  Determine if the current movement is a drag.  If it is, convert it into a QDrag.  If the
        #  drag ends inside the tab bar, emit an onMoveTabSignal.  If the drag ends outside the tab
        #  bar, emit an onDetachTabSignal.
        def mouseMoveEvent(self, event):

            # Determine if the current movement is detected as a drag
            if not self.dragStartPos.isNull() and ((event.pos() - self.dragStartPos).manhattanLength() > QtWidgets.QApplication.startDragDistance()):
                self.dragInitiated = True

            # If the current movement is a drag initiated by the left button
            if (((event.buttons() & QtCore.Qt.LeftButton)) and self.dragInitiated):

                # Stop the move event
                finishMoveEvent = QtGui.QMouseEvent(QtCore.QEvent.MouseMove, event.pos(), QtCore.Qt.NoButton, QtCore.Qt.NoButton, QtCore.Qt.NoModifier)
                super().mouseMoveEvent(finishMoveEvent)

                # Convert the move event into a drag
                drag = QtGui.QDrag(self)
                mimeData = QtCore.QMimeData()
                mimeData.setData('action', b'application/tab-detach')
                drag.setMimeData(mimeData)

                #Create the appearance of dragging the tab content
                pixmap = self.parentWidget().grab()
                targetPixmap = QtGui.QPixmap(pixmap.size())
                targetPixmap.fill(QtCore.Qt.transparent)
                painter = QtGui.QPainter(targetPixmap)
                painter.setOpacity(0.85)
                painter.drawPixmap(0, 0, pixmap)
                painter.end()
                drag.setPixmap(targetPixmap)

                # Initiate the drag
                dropAction = drag.exec_(QtCore.Qt.MoveAction | QtCore.Qt.CopyAction)

                # If the drag completed outside of the tab bar, detach the tab and move
                # the content to the current cursor position
                if dropAction == QtCore.Qt.IgnoreAction:
                    event.accept()
                    self.onDetachTabSignal.emit(self.tabAt(self.dragStartPos), self.mouseCursor.pos())

                # Else if the drag completed inside the tab bar, move the selected tab to the new position
                elif dropAction == QtCore.Qt.MoveAction:
                    if not self.dragDropedPos.isNull():
                        event.accept()
                        self.onMoveTabSignal.emit(self.tabAt(self.dragStartPos), self.tabAt(self.dragDropedPos))
            else:
                super().mouseMoveEvent(event)

        #  Determine if the drag has entered a tab position from another tab position
        def dragEnterEvent(self, event):
            if event.mimeData().hasFormat("scene"):
                event.accept()
                return
            mimeData = event.mimeData()
            formats = mimeData.formats()

            if 'action' in formats and mimeData.data('action') == 'application/tab-detach':
                event.acceptProposedAction()

            super().dragMoveEvent(event)

        def dragMoveEvent(self, event):
            if event.mimeData().hasFormat("scene"):
                index = self.tabAt(event.pos())
                self.parent().setCurrentIndex(index)
                self.parent().activeGLWindow = self.parent().currentWidget()
                event.accept()

        #  Get the position of the end of the drag
        def dropEvent(self, event):
            if event.mimeData().hasFormat("scene"):
                scene = self.parent().draw(pickle.loads(event.mimeData().data("scene").data()))
                event.accept()
                return
            self.dragDropedPos = event.pos()
            super().dropEvent(event)

        
    def _getActiveGLWindow(self):
        if not self._activeGLWindow:
            self.make_window()
        return self._activeGLWindow
    def _setActiveGLWindow(self, win):
        self.setCurrentWidget(win)
        self._activeGLWindow = win
    activeGLWindow = property(_getActiveGLWindow, _setActiveGLWindow)

    @inmain_decorator(True)
    def _remove_tab(self, index):
        if self.widget(index).isGLWindow():
            if self.activeGLWindow == self.widget(index):
                self.activeGLWindow = None
                for i in range(self.count()):
                    if instance(self.widget(self.count()-i-1), WindowTab):
                        self.activeGLWindow = self.widget(self.count()-i-1)
                        break
        self.removeTab(index)

    def draw(self, *args, tab=None, **kwargs):
        if tab is not None:
            tab_found = False
            for i in range(self.count()):
                if self.tabText(i) == tab:
                    # tab already exists -> activate it
                    tab_found = True
                    self.activeGLWindow = self.widget(i)
            if not tab_found:
                # create new tab with given name
                self.make_window(name=tab)
        self.activeGLWindow.draw(*args,**kwargs)

    @inmain_decorator(True)
    def plot(self, *args, **kwargs):
        from .plot import PlotTab
        window = PlotTab()
        window.plot(*args, **kwargs)
        self.addTab(window, kwargs["label"] if "label" in kwargs else "plot")
        self.setCurrentWidget(window)

    def setCurrentIndex(self, index):
        super().setCurrentIndex(index)
        if isinstance(self.currentWidget(), WindowTab):
            self.activeGLWindow = self.currentWidget()

    @inmain_decorator(True)
    def make_window(self, name=None):
        window = WindowTab()
        window.create(sharedContext=self._commonContext)
        if self._fastmode:
            window.glWidget._settings.fastmode = True
        name = name or "window" + str(self.count() + 1)
        self.addTab(window, name)
        self.activeGLWindow = window
