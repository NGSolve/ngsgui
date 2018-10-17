
from .widgets import ArrangeH, ArrangeV
from PySide2 import QtWidgets, QtCore, QtGui
from ngsgui.icons import location as icon_path
import weakref

class ToolBoxItem(QtWidgets.QWidget):
    changeExpand = QtCore.Signal()
    class Header(QtWidgets.QWidget):
        class Icon(QtWidgets.QToolButton):
            """List of icons to be changed by toggling signal"""
            def __init__(self, icons, signal = None, tooltip = None, **kwargs):
                super().__init__(**kwargs)
                if not isinstance(icons, list):
                    assert isinstance(icons, str)
                    self._lst = [icons]
                else:
                    assert all((isinstance(icon, str) for icon in icons))
                    self._lst = icons
                self._active = 0
                if signal:
                    self.clicked.connect(signal.emit)
                    signal.connect(self.next)
                self.setIcon(QtGui.QIcon(self._lst[0]))
                self.setIconSize(QtCore.QSize(12,12))
                self.setAutoRaise(True)
                if tooltip:
                    self.setToolTip(tooltip)

            def next(self):
                self._active = (self._active + 1)%len(self._lst)
                self.setIcon(QtGui.QIcon(self._lst[self._active]))

        def __init__(self, text, *args, **kwargs):
            super().__init__(*args,**kwargs)
            self._icons = []
            self._text = QtWidgets.QLabel(text)

        def updateLayout(self):
            """Not very clean - this function must be called only once after all icons are added..."""
            self.setLayout(ArrangeH(*self._icons, self._text))
            self.layout().setContentsMargins(0,0,0,0)

        def addIcon(self, images, action=None, tooltip=None):
            """Adds icon to header, if icon is list of icons then each time action is clicked, the icon
changes to the next one in the list. For that, action must be a QtCore.Signal

Parameters
----------

images: str or list of str
  Path to icon or list of paths

action: QtCore.Signal=None
  Signal to be emitted when icon is clicked

tooltip: str
  Tooltip for mouse hover"""
            if action:
                assert isinstance(action, QtCore.Signal)
            self._icons.append(ToolBoxItem.Header.Icon(images, signal=action, tooltip=tooltip))

    class Body(QtWidgets.QWidget):
        def __init__(self, *args,**kwargs):
            super().__init__(*args,**kwargs)

    def __init__(self, name, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.header = ToolBoxItem.Header(name)
        self.header.addIcon([icon_path + "/next.png", icon_path + "/down-arrow.png"], self.changeExpand,
                            "Expand menu")
        self.changeExpand.connect(self._changeExpand)
        self.body = ToolBoxItem.Body()
        self.body.setVisible(False)
        line = QtWidgets.QFrame()
        line.setFrameShape(line.HLine)
        self.setLayout(ArrangeV(self.header, self.body, line))
        self.layout().setContentsMargins(0,0,0,0)
        self.layout().setAlignment(QtCore.Qt.AlignTop)

    def _changeExpand(self):
        self.body.setVisible(not self.body.isVisible())

class SceneToolBoxItem(ToolBoxItem):
    def __init__(self, scene, visibilityButton=True, colorButton=True, *args, **kwargs):
        super().__init__(scene.name, *args, **kwargs)
        if visibilityButton:
            self.header.addIcon([icon_path + "/visible.png", icon_path + "/hidden.png"], scene.activeChanged,
                                "Show/Hide scene")
        if colorButton:
            self.header.addIcon([icon_path + "/nocolor.png", icon_path + "/color.png"], scene.individualColormapChanged,
                                "Use individual colormap")
        self.header.updateLayout()
        self.body.setLayout(ArrangeV(*scene.widgets.groups))
        scene.widgets.setParent(self.body)
        self.body.layout().setContentsMargins(0,0,0,0)
        self.body.layout().setAlignment(QtCore.Qt.AlignTop)
        scene.widgets.update()
        self._scene = weakref.ref(scene)

class ToolBox(QtWidgets.QDockWidget):
    """Our own toolbox class, because the PySide one doesn't support the stuff we want"""
    def __init__(self, title="", **kwargs):
        super().__init__(title,**kwargs)
        self._widget = QtWidgets.QWidget()
        self._widget.setLayout(ArrangeV())
        self._widget.layout().setContentsMargins(0,0,0,0)
        self._widget.layout().setAlignment(QtCore.Qt.AlignTop)
        self.setWidget(self._widget)

    def addWidget(self, widget):
        self._widget.layout().addWidget(widget)
        self._widget.layout().setContentsMargins(0,0,0,0)

class SceneToolBox(ToolBox):
    def __init__(self, glWidget, **kwargs):
        super().__init__(title="Scene Toolbox")
        scrollarea = QtWidgets.QScrollArea()
        scrollarea.setWidgetResizable(True)
        sceneWidget = QtWidgets.QWidget()
        sceneWidget.setLayout(ArrangeV())
        sceneWidget.layout().setAlignment(QtCore.Qt.AlignTop)
        sceneWidget.layout().setContentsMargins(0,0,0,0)
        scrollarea.setWidget(sceneWidget)
        self.addWidget(scrollarea)
        btnZoomReset = QtWidgets.QPushButton("ZoomReset", self)
        btnZoomReset.clicked.connect(glWidget.ZoomReset)
        btnZoomReset.setMinimumWidth(1);
        self.addWidget(btnZoomReset)

    def addScene(self, scene, **kwargs):
        widget = self.widget().layout().itemAt(0).widget().takeWidget()
        widget.layout().addWidget(SceneToolBoxItem(scene,**kwargs))
        self.widget().layout().itemAt(0).widget().setWidget(widget)
