
from . import widgets as wid
from . import scenes
from .thread import inthread, inmain_decorator

from .widgets import ArrangeH, ArrangeV

from PySide2 import QtWidgets, QtGui, QtCore

import ngsolve as ngs

class Parameter(QtCore.QObject):
    changed = QtCore.Signal(object)
    def __init__(self, group, name, *args, **kwargs):
        super().__init__()
        self.group = group
        self.name = name
        self._options = kwargs if kwargs else {}

    def getOption(self, name):
        if not name in self._options:
            return False
        else:
            return self._options[name]

    def setValue(self, val):
        self.changed.emit(val)

    def _attachTo(self, cls):
        """Creates setter and getter functions in attached class"""
        def _getValue():
            return self.getValue()
        def _setValue(val):
            self.setValue(_val)
        if hasattr(self, "getValue"):
            setattr(cls, "get" + self.name, self.getValue)
        setattr(cls, "set" + self.name, self.setValue)

    def getWidget(self):
        return self._widget

    def __getstate__(self):
        return (self.group, self.name,self._options)

    def __setstate__(self, state):
        Parameter.__init__(self,state[0], state[1])
        self._options = state[2]

class ParameterWithLabel(Parameter):
    def __init__(self, group, name, *args, label=None, **kwargs):
        super().__init__(group,name, *args, **kwargs)
        widget = self._createWidget()
        self.label = label if label else name
        self._widget = QtWidgets.QWidget()
        self._widget.setLayout(ArrangeH(QtWidgets.QLabel(self.label), widget))

    def __getstate__(self):
        return (super().__getstate__(),
                self.label)

    def __setstate__(self, state):
        super().__setstate__(state[0])
        widget = self._createWidget()
        self._widget = QtWidgets.QWidget()
        self._widget.setLayout(ArrangeH(QtWidgets.QLabel(state[1]), widget))

class ValueParameter(ParameterWithLabel):
    def __init__(self, default_value, *args, **kwargs):
        self._initial_value = default_value
        super().__init__(*args,**kwargs)

    def _createWidget(self):
        if isinstance(self._initial_value, float):
            self._spinbox = wid.ScienceSpinBox()
        elif isinstance(self._initial_value, int):
            self._spinbox = QtWidgets.QSpinBox()
        else:
            raise Exception("Cannot create ValueParameter for type ", type(default_value))
        self._spinbox.setValue(self._initial_value)
        self._spinbox.valueChanged.connect(self.changed.emit)
        return self._spinbox

    def getValue(self):
        return self._spinbox.value()

    def __getstate__(self):
        return (super().__getstate__(),
                self.getValue())

    def __setstate__(self, state):
        self._initial_value = state[1]
        super().__setstate__(state[0])

class SingleOptionParameter(ParameterWithLabel):
    def __init__(self, group, name, *args, values = None, default_value = None, **kwargs):
        self._values = values if values else []
        self._initial_value = default_value
        super().__init__(group, name, *args, **kwargs)

    def _createWidget(self):
        self._combobox = QtWidgets.QComboBox()
        self._combobox.addItems(self._values)
        if self._initial_value:
            self._combobox.setCurrentText(self._initial_value)
        self._combobox.currentIndexChanged.connect(lambda index: self.changed.emit(self.getValue()))
        return self._combobox

    def setValue(self, val):
        self._combobox.setCurrentText(val)
        super().setValue()

    def __setitem__(self,attr, value):
        self._combobox.addItem(attr)
        self._values.append(value)

    def getValue(self):
        return self._values[self._combobox.currentIndex()]

    def __getstate__(self):
        return (super().__getstate__(),
                self._values,
                self.getValue())

    def __setstate__(self, state):
        self._initial_value = state[2]
        self._values = state[1]
        super().__setstate__(state[0])

class FileParameter(Parameter):
    changed = QtCore.Signal(str)
    def __init__(self, name, filt, *args,**kwargs):
        super().__init__(*args,**kwargs)
        self.filt = filt
        self.name = name
        self.filename = ""
        self.txt = ""
        self._createWidget()

    def _createWidget(self):
        button = QtWidgets.QPushButton("Select File")
        def selectFile():
            self.filename,filt = QtWidgets.QFileDialog.getOpenFileName(caption = self.name,
                                                                       filter = self.filt)
            if self.filename:
                self.changed.emit(self.filename)
        button.clicked.connect(selectFile)
        label = QtWidgets.QLabel(self.filename if self.filename else "...")
        changed.connect(label.setText)
        self.widget = QtWidgets.QWidget()
        self.widget.setLayout(ArrangeH(button, label))

    def __getstate__(self):
        if self.txt:
            txt = self.txt
        elif self.filename:
            with open(self.filename, "r") as f:
                txt = f.read()
        else:
            txt = ""
        return (self.name, self.filt, self.filename, txt)

    def __setstate__(self, state):
        super().__setstate__(state[0],state[1])
        self.filename = state[2]
        self.txt = state[3]
        self.createWidget()

class BaseSettings():
    def __init__(self):
        self.createParameters()
        self.createOptions()
        self.createQtWidget()

    def __getstate__(self):
        values = {}
        for key, group in self._widgets.items():
            for name in group:
                if hasattr(self, "get" + name):
                    values[name] = getattr(self, "get" + name)()
        return (values,self._parameters)

    def __setstate__(self, state):
        self._initial_values = state[0]
        self._parameters = []
        for param in state[1]:
            self.addParameter(param)
        self.createOptions()
        self.createQtWidget()

    def createOptions(self):
        self._widgets = {}

    def createParameters(self):
        self._parameters = []

    @inmain_decorator(True)
    def createQtWidget(self):
        self.widgets = wid.OptionWidgets()
        for group in self._widgets:
            self.widgets.addGroup(group,*self._widgets[group].values())
        param_groups = {}
        for param in self._parameters:
            if param.group not in param_groups:
                param_groups[param.group] = []
            param_groups[param.group].append(param.getWidget())
        for group in param_groups:
            self.widgets.addGroup(group, *param_groups[group])

    @inmain_decorator(True)
    def updateWidgets(self):
        """Updates setting widgets"""
        self.widgets.update()

    def addParameter(self, parameter):
        self._parameters.append(parameter)
        parameter._attachTo(self)

    def addOption(self, group, name, typ=None, update_on_change=False, update_widget_on_change=False, widget_type=None, label=None, values=None, on_change=None, *args, **kwargs):
        if not group in self._widgets:
            self._widgets[group] = {}
        default_value = self._initial_values[name]
        label = label or name
        propname = "_"+name
        widgetname = "_"+name+"Widget"
        setter_name = "set"+name

        setattr(self, propname, default_value)

        if typ==None and widget_type==None:
            typ = type(default_value)

        if typ is list:
            w = QtWidgets.QComboBox()
            assert type(values) is list
            w.addItems(values)
            w.currentIndexChanged[int].connect(lambda index: getattr(self,setter_name)(index))
            self._widgets[group][name] = wid.WidgetWithLabel(w,label)

        elif widget_type:
            w = widget_type(*args, **kwargs)
            w.setValue(default_value)
            self._widgets[group][name] = w

        elif typ==bool:
            w = QtWidgets.QCheckBox(label)
            w.setCheckState(QtCore.Qt.Checked if default_value else QtCore.Qt.Unchecked)
            if on_change:
                w.stateChanged.connect(on_change)
            w.stateChanged.connect(lambda value: getattr(self, setter_name)(bool(value)))
            self._widgets[group][name] = wid.WidgetWithLabel(w)

        elif typ==int:
            w = QtWidgets.QSpinBox()
            w.setValue(default_value)
            w.valueChanged[int].connect(lambda value: getattr(self, setter_name)(value))
            if "min" in kwargs:
                w.setMinimum(kwargs["min"])
            if "max" in kwargs:
                w.setMaximum(kwargs["max"])
            self._widgets[group][name] = wid.WidgetWithLabel(w,label)

        elif typ==float:
            w = wid.ScienceSpinBox()
            w.setRange(-1e99, 1e99)
            w.setValue(default_value)
            w.valueChanged[float].connect(lambda value: getattr(self, setter_name)(value))
            if "min" in kwargs:
                w.setMinimum(kwargs["min"])
            if "max" in kwargs:
                w.setMaximum(kwargs["max"])
            if "step" in kwargs:
                w.setSingleStep(kwargs["step"])
                w.lastWheelStep = kwargs["step"]
            self._widgets[group][name] = wid.WidgetWithLabel(w, label)
        else:
            raise RuntimeError("unknown type: ", typ)

        def getValue(self):
            return getattr(self, propname)

        def setValue(self, value, redraw=True, update_gui=True):
            if getattr(self, propname) == value:
                return

            setattr(self, propname, value)

            if update_widget_on_change:
                self.updateWidgets()
            if redraw:
                if update_on_change:
                    self.update()
                self.widgets.updateGLSignal.emit()

            if update_gui:
                widget = self._widgets[group][name]
                widget.setValue(value)

        cls = type(self)

        if not hasattr(cls, setter_name):
            setattr(cls, setter_name, setValue)
        if not hasattr(cls, 'get'+name):
            setattr(cls, 'get'+name, getValue)
        return self._widgets[group][name]

    def addButton(self, group, name, function, update_on_change=False, label = None,*args,**kwargs):
        if not group in self._widgets:
            self._widgets[group] = {}
        if not label:
            label = name
        def doAction(self, redraw=True):
            function(*args, **kwargs)
            if update_on_change:
                self.update()
            if redraw:
                self.widgets.updateGLSignal.emit()

        cls = type(self)

        if not hasattr(cls, name):
            setattr(cls, name, doAction)

        w = wid.Button(label, getattr(self, name))
        self._widgets[group][name] = w
