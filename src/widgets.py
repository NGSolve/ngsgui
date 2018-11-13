

from qtpy import QtCore, QtGui, QtWidgets, QtOpenGL
from qtpy.QtCore import Qt

from .thread import inmain_decorator

import sys, re, math

class ObjectHolder():
    """Utility class to be used in situations like these:
for i in range(5):
  btn = Button("btnname")
  cb = CheckBox("cbname")
  cb.stateChanged.connect(lambda val: btn.clicked.emit())
  btns.append(btn)
  cbs.append(cb)
This would lead to all checkboxes being connected to the last button,
since the lambda sould only store a reference to the btn object of the function
which changes.
This can be circumvented by using
cb.stateChanged.connect(ObjectHolder(btn,btn.clicked.emit))
"""
    def __init__(self, obj, call_func):
        self.obj = obj
        self.call_func = call_func

    def __call__(self, *args, **kwargs):
        return self.call_func(self,*args, **kwargs)

def Arrange(layout_type, *args):
    layout = layout_type()
    for w in args:
        if isinstance(w, QtWidgets.QWidget):
            layout.addWidget(w)
        else:
            layout.addLayout(w)
    m = 0
    layout.setContentsMargins(0,0,0,0)
    return layout

def ArrangeV(*args):
    """Creates vertically arrange layout of input widgets/layouts"""
    return Arrange(QtWidgets.QVBoxLayout, *args)

def ArrangeH(*args):
    """Creates horizontally arrange layout of input widgets/layouts"""
    return Arrange(QtWidgets.QHBoxLayout, *args)

def CheckBox(name, *slots, checked=False):
    cb = QtWidgets.QCheckBox(name)
    if checked:
        cb.setCheckState(QtCore.Qt.Checked)
    else:
        cb.setCheckState(QtCore.Qt.Unchecked)
    for slot in slots:
        cb.stateChanged.connect(slot)
    return cb

def Button(name, *slots):
    button = QtWidgets.QPushButton(name)
    button.setMinimumWidth(1);
    for slot in slots:
        button.clicked.connect(slot)
    return button

def DoubleSpinBox(*slots, step=1, name=None):
    box = QtWidgets.QDoubleSpinBox()
    for slot in slots:
        box.valueChanged[float].connect(slot)
    box.setSingleStep(step)
    if(name):
        label = QtWidgets.QLabel(name)
        return ArrangeH(label, box)
    else:
        return box

class RangeGroup(QtWidgets.QWidget):
    valueChanged = QtCore.Signal(float)
    def __init__(self, name, min=-1, max=1, value=0, direction=Qt.Horizontal):
        super(RangeGroup, self).__init__()
        self.scalingFactor = 1000 # scaling between integer widgets (scrollslider) and float values to get more resoltion
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
        self.setValue(value)

    def setIntValue(self, int_value):
        float_value = int_value*1.0/self.scalingFactor
        self.valueBox.setValue(float_value)

    def setValue(self, float_value):
        int_value = round(self.scalingFactor*float_value)
        self.scroll.setValue(int_value)
        self.valueChanged.emit(float_value)

from .toolbox import ToolBoxItem

class OptionWidgets(QtCore.QObject):
    updateGLSignal = QtCore.Signal()

    def __init__(self, updateGL=None):
        super().__init__()
        self.groups = []
        if updateGL:
            self.updateGLSignal.connect(updateGL)

    def addGroup(self, name, *widgets, importance = 0):
        group = ToolBoxItem(name, killButton=False, endLine=False)
        minwidth = min([w.minimumWidth() for w in widgets])
        layout = QtWidgets.QHBoxLayout if minwidth>0 and minwidth<50 else QtWidgets.QVBoxLayout
        group.body.setLayout(Arrange(layout, *widgets))
        group.setMinimumWidth(1);
        group.body.layout().setContentsMargins(20,5,5,5)
        group.body.layout().setAlignment(Qt.AlignTop)
        group._importance = importance
        group._widgets = widgets
        if len(self.groups) and importance > self.groups[-1]._importance:
            for i in range(len(self.groups)):
                if self.groups[i]._importance < importance:
                    self.groups.insert(i,group)
                    break
        else:
            self.groups.append(group)
        self.update()

    @inmain_decorator(True)
    def update(self):
        if self.parent() != None:
            for group in self.groups:
                is_visible = lambda w: w._visible if hasattr(w,"_visible") else True
                group.setVisible(any((is_visible(w) for w in group._widgets)))

class QColorButton(QtWidgets.QPushButton):
    '''
    Custom Qt Widget to show a chosen color.

    Left-clicking the button shows the color-chooser, while
    right-clicking resets the color to None (no-color).
    '''

    colorChanged = QtCore.Signal()

    def __init__(self, initial_color,*args, **kwargs):
        super(QColorButton, self).__init__(*args, **kwargs)

        self.setColor(QtGui.QColor(*initial_color))
        self.setMaximumWidth(32)
        self.pressed.connect(self.onColorPicker)

    def setColor(self, color):
        if color:
            self._color = color
            self.setStyleSheet("background-color: %s;" % self._color.name())

    def color(self):
        return self._color

    def onColorPicker(self):
        '''
        Show color-picker dialog to select color.

        Qt will use the native dialog by default.

        '''
        dlg = QtWidgets.QColorDialog()
        dlg.setStyleSheet("")
        if self._color:
            dlg.setCurrentColor(self._color)

        if dlg.exec_():
            self.setColor(dlg.currentColor())
            self.colorChanged.emit()

    def mousePressEvent(self, e):
        if e.button() == Qt.RightButton:
            self.setColor(None)
            self.colorChanged.emit()

        return super(QColorButton, self).mousePressEvent(e)

class FloatValidator(QtGui.QValidator):
    _float_re = re.compile(r'(([+-]?\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?)')
    def _valid_float_string(self,string):
        match = self._float_re.search(string)
        return match.groups()[0] == string if match else False

    def validate(self,string,position):
        if self._valid_float_string(string):
            return (self.Acceptable, string, position)
        if string == "" or string[position-1] in "e.-+":
            return (self.Intermediate, string, position)
        return (self.Invalid, string, position)

    def fixup(self,text):
        match = self._float_re.search(text)
        return match.groups()[0] if match else text

class ScienceSpinBox(QtWidgets.QDoubleSpinBox):
    changed = QtCore.Signal(float)
    def __init__(self, *args, **kwargs):
        super().__init__(*args,**kwargs)
        maxval = sys.float_info.max
        self.setRange(-maxval,maxval)
        self.validator = FloatValidator()
        self.setDecimals(1000)
        self.lastWheelStep = 1.
        self.changed.connect(self.setValue)

    def validate(self,text,position):
        return self.validator.validate(text,position)

    def fixup(self, text):
        return self.validator.fixup(text)

    def textFromValue(self, value):
        return self._format_float(value)

    def valueFromText(self,text):
        return float(text)

    def wheelEvent(self, event):
        s = 1.0 if event.angleDelta().y()>0 else -1.0
        val = self.value()
        step = self.lastWheelStep

        if event.modifiers() == QtCore.Qt.ControlModifier:
            self.changed.emit(val*(10**s))
            self.lastWheelStep *= (10**s)
            event.accept()
            return

        if event.modifiers() == QtCore.Qt.ShiftModifier:
            step /= 10

        newval = val + s*step
        oldval = 1 if self.value() == 0 else self.value()
        self.changed.emit(0 if abs(newval/oldval) < 1e-10 else newval)
        event.accept()

    def _format_float(self,value):
        """Modified form of the 'g' format specifier."""
        string = "{:.3g}".format(value).replace("e+", "e")
        string = re.sub("e(-?)0*(\d+)", r"e\1\2", string)
        return string


class ColorMapSettings(QtWidgets.QWidget):
    linearChanged = QtCore.Signal(bool)
    def __init__(self, min=-1, max=1, min_value=0, max_value=1, direction=Qt.Horizontal, linear=False):
        super(ColorMapSettings, self).__init__()

        self.rangeMin = ScienceSpinBox()
        self.rangeMax = ScienceSpinBox()

        self.linear = QtWidgets.QCheckBox('Linear', self)
        self.linear.setCheckState(Qt.Checked if linear else Qt.Unchecked)
        self.linear.stateChanged.connect( lambda state: self.linearChanged.emit(state==Qt.Checked))

        self.setLayout( ArrangeV( self.rangeMin, self.rangeMax, self.linear ))

        self.rangeMin.setValue(min_value)
        self.rangeMax.setValue(max_value)

class ColorPickerWidget(QtWidgets.QWidget):
    colors_changed = QtCore.Signal()
    def __init__(self, names, initial_color=(0,255,0,255)):
        super().__init__()
        self._initial_color = initial_color
        self._colorbtns = {}
        self._checkboxes = []
        self.names = names

        layouts = []

        def call_func(self,state):
            color = self.obj._color
            if state:
                color.setAlpha(255)
            else:
                color.setAlpha(0)
            self.obj.setColor(color)
            self.obj.colorChanged.emit()

        for name in names:
            if name not in self._colorbtns:
                btn = QColorButton(initial_color=initial_color)
                btn.colorChanged.connect(self.colors_changed.emit)
                cb_visible = QtWidgets.QCheckBox('visible')
                self._checkboxes.append(cb_visible)
                cb_visible.setChecked(True)
                cb_visible.stateChanged.connect(ObjectHolder(btn,call_func))
                self._colorbtns[name] = btn
                layouts.append(ArrangeH(btn,QtWidgets.QLabel(name),cb_visible))

        colors = ArrangeV(*layouts)
        colors.layout().setAlignment(Qt.AlignTop)

        btn_random = QtWidgets.QPushButton("Random")
        btn_random.clicked.connect(self.setRandom)
        btn_reset = QtWidgets.QPushButton("Reset")
        btn_reset.clicked.connect(self.reset)
        btn_checkall = QtWidgets.QPushButton("Check All")
        btn_checkall.clicked.connect(self.checkAll)
        btn_uncheckall = QtWidgets.QPushButton("Uncheck All")
        btn_uncheckall.clicked.connect(self.uncheckAll)
        self.setLayout(ArrangeV(colors,ArrangeV(ArrangeH(btn_random,btn_reset),
                                                ArrangeH(btn_checkall, btn_uncheckall))))

    def checkAll(self):
        old_state = self.blockSignals(True)
        for cb in self._checkboxes:
            cb.setChecked(True)
        self.blockSignals(old_state)
        self.colors_changed.emit()

    def uncheckAll(self):
        old_state = self.blockSignals(True)
        for cb in self._checkboxes:
            cb.setChecked(False)
        self.blockSignals(old_state)
        self.colors_changed.emit()

    def reset(self):
        for name, btn in self._colorbtns.items():
            col = QtGui.QColor(*self._initial_color)
            col.setAlpha( btn.color().alpha() )
            btn.setColor(col)
        self.colors_changed.emit()

    def setRandom(self):
        import itertools
        import random
        n = 2
        while n**3+1 < len(self._colorbtns):
            n += 1
        vals = [int(255*i/(n-1)) for i in range(n)]
        colors = [(vals[colr],vals[colg],vals[colb]) for colr, colg, colb in
                  itertools.product(range(n),range(n),range(n))][:-1]
        random.shuffle(colors)


        gr = 0.618033988749895
        h = random.uniform(0,1)
        for i,(name,btn) in enumerate(self._colorbtns.items()):
            h += gr
            h %= 1
            s = [1.0,0.4][i%2]
            v = [1.0,0.8][i%2]
            color = QtGui.QColor()
            color.setHsvF(h,s,v)
            color.setAlpha(btn.color().alpha())
            btn.setColor(color)
        self.colors_changed.emit()
        

    def setColors(self, colors):
        old_state = self.blockSignals(True)
        for i, (btn,cb) in enumerate(zip(self._colorbtns.values(), self._checkboxes)):
            btn.setColor(QtGui.QColor(*colors[i*4:(i+1)*4]))
            cb.setChecked(colors[i*4+3])
        self.blockSignals(old_state)
        self.colors_changed.emit()

    def getColors(self):
        return [QtGui.QColor(self._colorbtns[name]._color) for name in self.names]

class WidgetWithLabel(QtWidgets.QWidget):
    def __init__(self, widget, label=None):
        super().__init__()
        self._value_widget = widget

        if label==None:
            l = ArrangeV(widget)
            l.setMargin(0)
            self.setLayout(l)
        else:
            l= QtWidgets.QLabel(label)
            lay = ArrangeH( l, widget )
            lay.setMargin(0)
            self.setLayout(lay)

    def setValue(self, value):
        if isinstance(self._value_widget, QtWidgets.QCheckBox):
            self._value_widget.setCheckState(QtCore.Qt.Checked if value else QtCore.Qt.Unchecked)
        elif isinstance(self._value_widget, QtWidgets.QComboBox):
            self._value_widget.setCurrentIndex(value)
        else:
            self._value_widget.setValue(value)

class ButtonArea(QtWidgets.QWidget):
    """Widget containing buttons"""
    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.setLayout(ArrangeH())
        # pal = QtGui.QPalette()
        # pal.setColor(QtGui.QPalette.Background, QtCore.Qt.grey)
        # self.setPalette(pal)

    def addButton(self, function, name=None, icon=None, icon_size = (20,20), description=None,
                  shortcut = None, checkable = False):
        """Create a new button in the area.

Parameters
----------

function: function
  Function that is called when the button is clicked.

name: str = None
  If no icon is set, the button will have this text

icon: str = None
  Filename of the icon installed into the icon path

icon_size: tuple = (20,20)
  Size of the icon in the button

description: str = None
  Optional, tooltip for the button

shortcut: str = None
  Optional, set shortcut for the button
"""
        assert (name or icon), "Button needs name or icon"
        btn = QtWidgets.QPushButton()
        if not icon:
            btn.setText(name)
        else:
            from ngsgui.icons import location as icon_path
            btn.setIcon(QtGui.QIcon(icon_path + "/" + icon))
            btn.setIconSize(QtCore.QSize(*icon_size))
        if description:
            btn.setToolTip(description)
        btn.clicked.connect(function)
        if shortcut:
            btn.setShortcut(shortcut)
        if checkable:
            btn.setCheckable(True)
        self.layout().addWidget(btn)

def addShortcut(widget, name, key, func):
    """Helper function to create shortcuts and register them in QSettings, so that
user can change them."""
    settings = QtCore.QSettings()
    action = QtWidgets.QAction(name)
    action.triggered.connect(func)
    if not settings.value("shortcuts/" + name):
            settings.setValue("shortcuts/" + name, key)
    action.setShortcut(QtGui.QKeySequence(settings.value("shortcuts/"+ name)))
    widget.addAction(action)
    if not hasattr(widget, "_qactions"):
        widget._qactions = []
    widget._qactions.append(action)

