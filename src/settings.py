
from . import widgets as wid
from . import scenes
from .thread import inthread, inmain_decorator

from .widgets import ArrangeH, ArrangeV

from PySide2 import QtWidgets, QtGui, QtCore

import ngsolve as ngs

class Parameter(QtCore.QObject):
    changed = QtCore.Signal(object)
    def __init__(self, name=None, label=None, label_above=False, **kwargs):
        super().__init__()
        self.name = name
        self.label = label
        self._label_above = label_above
        self.__options = kwargs if kwargs else {}
        self._createWithLabel()

    def _attachTo(self, obj):
        pass

    def _createWithLabel(self):
        if self.label:
            self._widget = QtWidgets.QWidget()
            arrange = ArrangeV if self._label_above else ArrangeH
            layout = arrange(QtWidgets.QLabel(self.label), self._createWidget())
            layout.setContentsMargins(0,0,0,0)
            self._widget.setLayout(layout)
        else:
            self._widget = self._createWidget()

    def _createWidget(self):
        raise NotImplementedError("Parameter class must overload _createWidget!")

    def getOption(self, name):
        if not name in self.__options:
            return False
        else:
            return self.__options[name]

    def setValue(self, val):
        self.changed.emit(val)

    def getWidget(self):
        return self._widget

    def __getstate__(self):
        return (self.name, self.label, self.__options)

    def __setstate__(self, state):
        Parameter.__init__(self, name=state[0], label=state[1])
        self.__options = state[2]

class CombinedParameters(Parameter):
    def __init__(self, parameters, vertical=False, **kwargs):
        self._parameters = parameters
        self._vertical = False
        super().__init__(**kwargs)

    def _createWidget(self):
        arrange = ArrangeV if self._vertical else ArrangeH
        widget = QtWidgets.QWidget()
        widget.setLayout(arrange(*(par._widget for par in self._parameters)))
        return widget

    def _attachTo(self, obj):
        for par in self._parameters:
            obj._attachParameter(par)

    def __getstate__(self):
        return (super().__getstate__(), self._parameters, self._vertical, self._label)

    def __setstate__(self,state):
        self._parameters, self._vertical, self._label = state[1:]
        super().__setstate__(state[0])

class ColorParameter(Parameter):
    def __init__(self, *args, default_value=(0,0,255,255), values, **kwargs):
        self._values = values
        self._default_value = default_value
        super().__init__(*args, **kwargs)

    def _createWidget(self):
        self._colorWidget = wid.ColorPickerWidget(self._values, initial_color = self._default_value)
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
        for i, (btn, cb) in enumerate(zip(self._colorWidget._colorbtns.values(), self._colorWidget._checkboxes)):
            btn.setColor(QtGui.QColor(*(state[2][i*4:(i+1)*4])))
            cb.setChecked(state[2][i*4+3])

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
        sublayout = ArrangeV(*subwidgets)
        # set top,right,left to 0, right to 11 (should be default on most platforms)
        sublayout.setContentsMargins(11,0,0,0)
        self._vbox.setLayout(sublayout)
        self._vbox.setVisible(self.getValue())
        self.changed.connect(lambda : self._vbox.setVisible(self.getValue()))
        layout =  ArrangeV(checkbox, self._vbox)
        # set margins to 0, same margins as CheckboxParameter
        layout.setContentsMargins(0,0,0,0)
        widget.setLayout(layout)
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
        if self._min_value != None:
            self._spinbox.setMinimum(self._min_value)
        if self._max_value != None:
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

class SingleChoiceParameter(Parameter):
    def __init__(self, *args, options, default_value, sub_parameters=None, **kwargs):
        # list of single choice options
        self._options = options
        # string matching one of the above choices
        self._default_value = default_value
        # list of lists: the i-th list represents the parameters which become visible
        #                when the i-th single choice option is selected
        self._sub_parameters = sub_parameters
        super().__init__(*args, **kwargs)

    def _attachTo(self, obj):
        for lst in self._sub_parameters:
            for par in lst:
                obj._attachParameter(par)

    def _createWidget(self):
        widgetlst = []
        w = QtWidgets.QWidget()
        self._group = QtWidgets.QButtonGroup(w)
        for i,option in enumerate(self._options):
            btn = QtWidgets.QRadioButton(option)
            btn.clicked.connect(lambda : self.changed.emit(self.getValue()))
            self._group.addButton(btn,i)
            widgetlst.append(btn)
            if(self._sub_parameters is not None and self._sub_parameters[i]):
                subwid = QtWidgets.QWidget()
                subpars = [par.getWidget() for par in self._sub_parameters[i]]
                layout = ArrangeV(*subpars)
                # set top,right,left to 0, right to 11 (should be default on most platforms)
                layout.setContentsMargins(11,0,0,0)
                subwid.setLayout(layout)
                btn.toggled.connect(subwid.setVisible)
                widgetlst.append(subwid)
            if(option==self._default_value):
                btn.setChecked(True)
            else:
                btn.toggled.emit(False)
        w.setLayout(ArrangeV(*widgetlst))
        return w

    def getValue(self):
        return self._group.checkedId()

    def __getstate__(self):
        return (super().__getstate__(),
                self._options,
                self._options[self.getValue()],
                self._sub_parameters)

    def __setstate__(self,state):
        self._options = state[1]
        self._default_value = state[2]
        self._sub_parameters = state[3]
        super().__setstate__(state[0])

class SingleOptionParameter(Parameter):
    def __init__(self, name, *args, values = None, default_value = None, **kwargs):
        self._values = values if values else []
        self._initial_value = default_value
        super().__init__(name, *args, **kwargs)
        if not self._values:
            self._widget.setVisible(False)
            self._widget._hidden = True

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

    def append(self, value):
        self._combobox.addItem(value)
        self._combobox.setCurrentText(value)
        self._values.append(value)
        self._widget.setVisible(True)
        self._widget._hidden = False

    def getValue(self):
        return self._values[self._combobox.currentIndex()] if self._values else ""

    def __getstate__(self):
        return (super().__getstate__(),
                self._values,
                self.getValue())

    def __setstate__(self, state):
        self._initial_value = state[2]
        self._values = state[1]
        super().__setstate__(state[0])
        if not self._values:
            self._widget.setVisible(False)
            self._widget._hidden = True


class FileParameter(Parameter):
    def __init__(self, filt, *args,**kwargs):
        self.filt = filt
        self.filename = ""
        self.txt = ""
        super().__init__(*args,**kwargs)

    def getValue(self):
        return self.filename

    def _createWidget(self):
        self._button = QtWidgets.QPushButton("Select File")
        def selectFile():
            self.filename,filt = QtWidgets.QFileDialog.getOpenFileName(caption = self.name,
                                                                       filter = self.filt)
            if self.filename:
                self.changed.emit(self.filename)
        self._button.clicked.connect(selectFile)
        self._label_filename = QtWidgets.QLabel(self.filename if self.filename else "...")
        self.changed.connect(lambda : self._label_filename.setText(self.getValue()))
        widget = QtWidgets.QWidget()
        widget.setLayout(ArrangeH(self._button, self._label_filename))
        return widget

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
        self._createParameters()
        self._createOptions()
        self._createQtWidget()

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
        self._createOptions()
        self._createQtWidget()

    def _createParameters(self):
        self._parameters = {}
        self._par_name_dict = {}

    def _createOptions(self):
        self._widgets = {}

    @inmain_decorator(True)
    def _createQtWidget(self):
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

    def _connectParameters(self):
        pass

    def addParameters(self, group, *parameters):
        if group not in self._parameters:
            self._parameters[group] = []
        for par in parameters:
            self._parameters[group].append(par)
            self._attachParameter(par)
            if par.name:
                self._par_name_dict[par.name] = par

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
