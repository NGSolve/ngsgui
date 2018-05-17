

from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
from PySide2.QtCore import Qt

import sys, re, math

class ObjectHolder():
    def __init__(self, obj, call_func):
        self.obj = obj
        self.call_func = call_func

    def __call__(self, *args, **kwargs):
        self.call_func(self,*args, **kwargs)

def Arrange(layout_type, *args):
    layout = layout_type()
    for w in args:
        if isinstance(w, QtWidgets.QWidget):
            layout.addWidget(w)
        else:
            layout.addLayout(w)
    return layout

def ArrangeV(*args):
    return Arrange(QtWidgets.QVBoxLayout, *args)

def ArrangeH(*args):
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
    valueChanged = QtCore.Signal(float) # TODO: this shouldn't be static
    def __init__(self, name, min=-1, max=1, value=0, direction=Qt.Horizontal):
        super(RangeGroup, self).__init__()
        # self.valueChanged.connect(onValueChanged)

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

class OptionWidgets(QtCore.QObject):
    updateGLSignal = QtCore.Signal()

    def __init__(self, updateGL=None):
        super().__init__()
        self.visibilityOptions = {}
        self.groups = []
        if updateGL:
            self.updateGLSignal.connect(updateGL)

    def addGroup(self, name, *widgets, connectedVisibility=None, importance = 0):
        group = QtWidgets.QGroupBox(name)
        minwidth = min([w.minimumWidth() for w in widgets])
        layout = QtWidgets.QHBoxLayout if minwidth>0 and minwidth<50 else QtWidgets.QVBoxLayout
        group.setLayout(Arrange(layout, *widgets))
        group.setMinimumWidth(1);
        group.layout().setAlignment(Qt.AlignTop)
        group._importance = importance
        if connectedVisibility is not None:
            self.visibilityOptions[connectedVisibility] = group
        if len(self.groups) and importance > self.groups[-1]._importance:
            for i in range(len(self.groups)):
                if self.groups[i]._importance < importance:
                    self.groups.insert(i,group)
                    break
        else:
            self.groups.append(group)
        self.update()

    def update(self):
        for evaluator, group in self.visibilityOptions.items():
            group.setVisible(evaluator())

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
            return self.State.Acceptable
        if string == "" or string[position-1] in "e.-+":
            return self.State.Intermediate
        return self.State.Invalid

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

class CollColors(QtWidgets.QWidget):
    colors_changed = QtCore.Signal()

    def __init__(self,coll,initial_color=(0,255,0,255)):
        super().__init__()

        self.initial_color = initial_color

        self.colorbtns = {}
        self.checkboxes = []
        layouts = []
        self.coll = coll

        def call_func(self,state):
            color = self.obj._color
            if state:
                color.setAlpha(255)
            else:
                color.setAlpha(0)
            self.obj.setColor(color)
            self.obj.colorChanged.emit()

        for item in coll:
            if not item in self.colorbtns:
                btn = QColorButton(initial_color=initial_color)
                btn.colorChanged.connect(self.colors_changed.emit)
                cb_visible = QtWidgets.QCheckBox('visible',self)
                self.checkboxes.append(cb_visible)
                cb_visible.setCheckState(QtCore.Qt.Checked)
                cb_visible.stateChanged.connect(ObjectHolder(btn,call_func))
                self.colorbtns[item] = btn
                layouts.append(ArrangeH(btn,QtWidgets.QLabel(item),cb_visible))

        colors = ArrangeV(*layouts)
        colors.layout().setAlignment(Qt.AlignTop)
        btn_random = QtWidgets.QPushButton("Random",self)
        def SetRandom():
            import itertools
            import random
            n = 2
            while n**3+1 < len(self.colorbtns):
                n += 1
            vals = [int(255*i/(n-1)) for i in range(n)]
            colors = [(vals[colr],vals[colg],vals[colb]) for colr, colg, colb in
                      itertools.product(range(n),range(n),range(n))][:-1]
            random.shuffle(colors)


            gr = 0.618033988749895
            h = random.uniform(0,1)
            for i,(name,btn) in enumerate(self.colorbtns.items()):
                h += gr
                h %= 1
                s = [1.0,0.4][i%2]
                v = [1.0,0.8][i%2]
                color = QtGui.QColor()
                color.setHsvF(h,s,v)
                color.setAlpha(btn.color().alpha())
                btn.setColor(color)
            self.colors_changed.emit()

        btn_random.clicked.connect(SetRandom)
        btn_reset = QtWidgets.QPushButton("Reset", self)
        def Reset():
            for name, btn in self.colorbtns.items():
                col = QtGui.QColor(*self.initial_color)
                col.setAlpha( btn.color().alpha() )
                btn.setColor(col)
            self.colors_changed.emit()
        btn_reset.clicked.connect(Reset)
        layout = ArrangeV(colors,ArrangeH(btn_random,btn_reset))
        self.setLayout(layout)

    def getColors(self):
        return [QtGui.QColor(self.colorbtns[item]._color) for item in self.coll]
