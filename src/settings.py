
from . import widgets as wid
from . import scenes
from .thread import inthread, inmain_decorator

from .widgets import ArrangeH, ArrangeV

from PySide2 import QtWidgets, QtGui, QtCore

import ngsolve as ngs

class Parameter(QtCore.QObject):
    changed = QtCore.Signal(object)
    def __init__(self, name=None, label=None, **kwargs):
        super().__init__()
        self.name = name
        self.label = label
        self._options = kwargs if kwargs else {}
        self._createWithLabel()

    def _attachTo(self, obj):
        pass

    def _createWithLabel(self):
        if self.label:
            self._widget = QtWidgets.QWidget()
            self._widget.setLayout(ArrangeH(QtWidgets.QLabel(self.label), self._createWidget()))
        else:
            self._widget = self._createWidget()

    def _createWidget(self):
        raise NotImplementedError("Parameter class must overload _createWidget!")

    def getOption(self, name):
        if not name in self._options:
            return False
        else:
            return self._options[name]

    def setValue(self, val):
        self.changed.emit(val)

    def getWidget(self):
        return self._widget

    def __getstate__(self):
        return (self.name, self.label, self._options)

    def __setstate__(self, state):
        Parameter.__init__(self, name=state[1], label=state[2])
        self._options = state[3]

class ColorParameter(Parameter):
    def __init__(self, *args, default_value=(0,0,255,255), values, **kwargs):
        self._values = values
        self._default_value = default_value
        super().__init__(*args, **kwargs)

    def _createWidget(self):
        self._colorWidget = wid.CollColors(self._values, initial_color = self._default_value)
        self._colorWidget.colors_changed.connect(lambda : self.changed.emit(None))
        return self._colorWidget

    def getValue(self):
        return [f() for c in self._colorWidget.getColors() for f in [c.red, c.green, c.blue, c.alpha] ]

    def __getstate__(self):
        superstate = super().__getstate__()
        return (superstate,self._values, self.getValue())

    def __setstate__(self, state):
        self._values = state[1]
        self._default_value = (0,0,255,255)
        super().__setstate__(state[0])
        for i, (btn, cb) in enumerate(zip(self._colorWidget.colorbtns.values(), self._colorWidget.checkboxes)):
            btn.setColor(QtGui.QColor(*(state[2][i*4:(i+1)*4])))
            cb.setCheckState(QtCore.Qt.Checked if state[2][i*4+3] else QtCore.Qt.Unchecked)

class CheckboxParameter(Parameter):
    def __init__(self, *args, name, default_value=False, label=None, **kwargs):
        self._initial_value = default_value
        self._label = label if label else name
        super().__init__(*args, name=name,**kwargs)

    def _createWidget(self):
        self._cb = QtWidgets.QCheckBox(self._label)
        self._cb.setCheckState(QtCore.Qt.Checked if self._initial_value else QtCore.Qt.Unchecked)
        self._cb.stateChanged.connect(self.changed.emit)
        return self._cb

    def getValue(self):
        return self._cb.isChecked()

    def setValue(self,val):
        self._widget.setCheckState(QtCore.Qt.Checked if val else QtCore.Qt.Unchecked)

    def __getstate__(self):
        return (super().__getstate__(),
                self.getValue(),
                self._label)

    def __setstate__(self, state):
        self._initial_value = state[1]
        self._label = state[2]
        super().__setstate__(state[0])

class CheckboxParameterCluster(CheckboxParameter):
    def __init__(self, *args, sub_parameters, **kwargs):
        self._sub_parameters = sub_parameters
        super().__init__(*args, **kwargs)

    def _attachTo(self, obj):
        for par in self._sub_parameters:
            obj._attachParameter(par)

    def _createWidget(self):
        checkbox = super()._createWidget()
        widget = QtWidgets.QWidget()
        self._vbox = QtWidgets.QWidget()
        subwidgets = [par.getWidget() for par in self._sub_parameters]
        self._vbox.setLayout(ArrangeV(*subwidgets))
        self._vbox.setVisible(self.getValue())
        self.changed.connect(lambda : self._vbox.setVisible(self.getValue()))
        widget.setLayout(ArrangeV(checkbox, self._vbox))
        return widget

    def __getstate__(self):
        return (super().__getstate__(), self._sub_parameters)

    def __setstate__(self,state):
        self._sub_parameters = state[1]
        super().__setstate__(state[0])

class ValueParameter(Parameter):
    def __init__(self, default_value, min_value=None, max_value=None, step=None, **kwargs):
        self._initial_value = default_value
        self._min_value = min_value
        self._max_value = max_value
        self._step = step
        super().__init__(**kwargs)

    def _createWidget(self):
        if isinstance(self._initial_value, float):
            self._spinbox = wid.ScienceSpinBox()
            if self._step:
                self._spinbox.lastWheelStep=self._step
        elif isinstance(self._initial_value, int):
            self._spinbox = QtWidgets.QSpinBox()
            if self._step:
                self._spinbox.setSingleStep(self._step)
        else:
            raise Exception("Cannot create ValueParameter for type ", type(default_value))
        if self._min_value:
            self._spinbox.setMinimum(self._min_value)
        if self._max_value:
            self._spinbox.setMaximum(self._max_value)
        self._spinbox.setValue(self._initial_value)
        self._spinbox.valueChanged.connect(self.changed.emit)
        return self._spinbox

    def getValue(self):
        return self._spinbox.value()

    def setValue(self, val):
        self._spinbox.setValue(val)

    def __getstate__(self):
        return (super().__getstate__(),
                self.getValue(),
                self._min_value,
                self._max_value,
                self._step)

    def __setstate__(self, state):
        self._initial_value, self._min_value, self._max_value, self._step = state[1:]
        super().__setstate__(state[0])

class SingleOptionParameter(Parameter):
    def __init__(self, name, *args, values = None, default_value = None, **kwargs):
        self._values = values if values else []
        self._initial_value = default_value
        super().__init__(name, *args, **kwargs)

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
    def __init__(self, filt, *args,**kwargs):
        self.filt = filt
        self.filename = ""
        self.txt = ""
        super().__init__(*args,**kwargs)

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
        self._parameters = {}
        for group in state[1]:
            self.addParameters(group,*state[1][group])
        self.createOptions()
        self.createQtWidget()

    def createParameters(self):
        self._parameters = {}

    def createOptions(self):
        self._widgets = {}

    @inmain_decorator(True)
    def createQtWidget(self):
        self.widgets = wid.OptionWidgets()
        for group in self._widgets:
            self.widgets.addGroup(group,*self._widgets[group].values())
        for group,params in self._parameters.items():
            for param in params:
                if param.getOption("updateWidgets"):
                    param.changed.connect(self.widgets.update)
            widgets = [par.getWidget() for par in params]
            self.widgets.addGroup(group, *widgets)

    @inmain_decorator(True)
    def updateWidgets(self):
        """Updates setting widgets"""
        self.widgets.update()

    def _attachParameter(self, parameter):
        if parameter.name:
            if hasattr(parameter, "getValue"):
                setattr(self, "get" + parameter.name, parameter.getValue)
            setattr(self, "set" + parameter.name, parameter.setValue)
        parameter._attachTo(self)

    def addParameters(self, group, *parameters):
        if group not in self._parameters:
            self._parameters[group] = []
        for par in parameters:
            self._parameters[group].append(par)
            self._attachParameter(par)

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
