
from . import widgets as wid
from . import scenes
from . import glmath
from .thread import inthread, inmain_decorator

from .widgets import ArrangeH, ArrangeV

from PySide2 import QtWidgets, QtGui, QtCore

import ngsolve as ngs
import math

import os
_have_qt = not 'NGSGUI_HEADLESS' in os.environ
del os

class Parameter(QtCore.QObject):
    changed = QtCore.Signal(object)
    def __init__(self, name=None, label=None, label_above=False, **kwargs):
        super().__init__()
        self.name = name
        self.label = label
        self._label_above = label_above
        self.__options = kwargs if kwargs else {}
        if _have_qt:
            self._createWithLabel()

    def _attachTo(self, obj):
        if self.name:
            obj._par_name_dict[self.name] = self

    def setVisible(self, val):
        if _have_qt:
            self._widget._visible = val
            self._widget.setVisible(val)

    def isVisible(self):
        return self._widget._visible if hasattr(self._widget,"_visible") else True

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

    def setValue(self, val=None):
        self.changed.emit(val)

    def getWidget(self):
        return self._widget

    def __getstate__(self):
        return (self.name, self.label, self.__options, self._label_above)

    def __setstate__(self, state):
        Parameter.__init__(self, name=state[0], label=state[1], label_above=state[3])
        self.__options = state[2]

class Button(Parameter):
    def __init__(self, label=None, icon=None, tooltip=None, **kwargs):
        assert label or icon, "Button must be created with a label or an icon"
        self._label = label
        self._icon = icon
        self._tooltip = tooltip
        super().__init__(**kwargs)

    def _createWidget(self):
        btn = QtWidgets.QPushButton(self._label)
        if self._icon:
            btn.setIcon(self._icon)
        if self._tooltip:
            btn.setToolTip(self._tooltip)
        btn.clicked.connect(lambda : self.changed.emit(None))
        return btn

    def __getstate__(self):
        return (super().__getstate__(), self._label, self._icon, self._tooltip)

    def __setstate__(self, state):
        self._label, self._icon, self._tooltip = state[1:]
        super().__setstate__(state[0])

class Slider(Parameter):
    def __init__(self, range = (0,100), tickInterval=1, default_value = None, *args, **kwargs):
        self._range = range
        self._tickInterval = tickInterval
        self._default_value = int(default_value)
        super().__init__(*args,**kwargs)

    def _createWidget(self):
        self._slider = slider = QtWidgets.QSlider()
        slider.setRange(*self._range)
        slider.setTickInterval(self._tickInterval)
        slider.setOrientation(QtCore.Qt.Horizontal)
        slider.sliderMoved.connect(self.changed.emit)
        if self._default_value:
            self._slider.setValue(self._default_value)
        return slider

    def setValue(self, value):
        super().setValue(value)
        self._slider.setValue(value)

    def getValue(self):
        return self._slider.value()

    def __getstate__(self):
        return (super().__getstate__(), self._range, self._tickInterval, self._slider.value())

    def __setstate__(self, state):
        self._range, self._tickInterval, self._default_value = state[1:]
        super().__setstate__(state[0])

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
        super()._attachTo(obj)
        for par in self._parameters:
            obj._attachParameter(par)

    def __getstate__(self):
        return (super().__getstate__(), self._parameters, self._vertical)

    def __setstate__(self,state):
        self._parameters, self._vertical = state[1:]
        super().__setstate__(state[0])

class ColorParameter(Parameter):
    def __init__(self, *args, default_value=(0,0,255,255), values, **kwargs):
        self._values = values
        self._default_value = default_value
        super().__init__(*args, **kwargs)

    def _createWidget(self):
        self._colorWidget = wid.ColorPickerWidget(self._values, initial_color = self._default_value)
        self._colorWidget.colors_changed.connect(lambda : self.changed.emit(self.getValue()))
        return self._colorWidget

    @inmain_decorator(True)
    def getValue(self):
        return [f() for c in self._colorWidget.getColors() for f in [c.red, c.green, c.blue, c.alpha] ]

    def setValue(self, vals):
        self._colorWidget.setColors(vals)

    def __getstate__(self):
        superstate = super().__getstate__()
        return (superstate,self._values, self.getValue())

    def __setstate__(self, state):
        self._values = state[1]
        self._default_value = (0,0,255,255)
        super().__setstate__(state[0])
        self.setValue(state[2])

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
        self._cb.setCheckState(QtCore.Qt.Checked if val else QtCore.Qt.Unchecked)

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
        super()._attachTo(obj)
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

class TextParameter(Parameter):
    def __init__(self, default_value, **kwargs):
        self._initial_value = default_value
        super().__init__(**kwargs)

    def _createWidget(self):
        self._edit = QtWidgets.QLineEdit()
        self._edit.setText(self._initial_value)
        self._edit.returnPressed.connect(lambda: self.changed.emit(self._edit.text()))
        return self._edit

    def getValue(self):
        return self._edit.text()

    def setValue(self, val):
        self._edit.setText(val)

    def __getstate__(self):
        return (super().__getstate__(),
                self.text())

    def __setstate__(self, state):
        self._initial_value = state[1]
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

class VectorParameter(Parameter):
    def __init__(self, default_value, min_value=None, max_value=None, step=None, **kwargs):
        self._initial_value = default_value
        self._step = step
        self._min_value = min_value
        self._max_value = max_value
        super().__init__(**kwargs)

    def _createWidget(self):
        self._spinboxes = []
        for i in range(len(self._initial_value)):
            sb = wid.ScienceSpinBox()

            if self._step:
                sb.lastWheelStep=self._step
            if self._min_value != None:
                sb.setMinimum(self._min_value)
            if self._max_value != None:
                sb.setMaximum(self._max_value)
            sb.setValue(self._initial_value[i])
            sb.valueChanged.connect(self.changed.emit)
            self._spinboxes.append(sb)
        return ArrangeH(*self._spinboxes)

    def getValue(self):
        return [sb.value() for sb in self._spinboxes]

    def setValue(self, val):
        for sb,v in zip(self._spinboxes, val):
            sb.blockSignals(True)
            sb.setValue(v)
            sb.blockSignals(False)
        self.changed.emit(self.getValue())

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
        super()._attachTo(obj)
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
            self.setVisible(False)

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
        self._values.append(value)
        self._combobox.addItem(value)
        self._combobox.setCurrentText(value)
        self.setVisible(True)

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
            self.setVisible(False)

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
        super().__setstate__(state[0])
        self.filt = state[1]
        self.filename = state[2]
        self.txt = state[3]
        self.createWidget()

if not _have_qt:
    # patch access functions to work without GUI
    def getNoGUI(self):
        return self._initial_value

    def setNoGUI(self, value):
        self._initial_value = value
        self.changed.emit(value)

    for par in (ColorParameter, ValueParameter, VectorParameter, TextParameter, SingleOptionParameter,
                CheckboxParameter, CheckboxParameterCluster):
        par.getValue = getNoGUI
        par.setValue = setNoGUI

class BaseSettings(QtCore.QObject):
    _individual_rendering_parameters = True
    # signals defined here because qt has a problem with multiple inheritance and I can't get it running if
    # they are defined in the matching classes...
    individualColormapChanged = QtCore.Signal()
    individualClippingPlaneChanged = QtCore.Signal()
    individualLightChanged = QtCore.Signal()
    def __init__(self):
        super().__init__()
        self._createParameters()
        if _have_qt:
            self._createQtWidget()

    def getSettings(self):
        res = {}
        for group in self._parameters:
            for p in self._parameters[group]:
                res[p.name] = p.getValue()
        return res

    def setSettings(self, settings):
        for group in self._parameters:
            for p in self._parameters[group]:
                if p.name in settings:
                    p.setValue(settings[p.name])

    def __getstate__(self):
        return (self._parameters,)

    def __setstate__(self, state):
        self._parameters = {}
        self._par_name_dict = {}
        for group, items in state[0].items():
            self.addParameters(group,*items)
        if _have_qt:
            self._createQtWidget()

    def _createParameters(self):
        self._parameters = {}
        self._par_name_dict = {}

    @inmain_decorator(True)
    def _createQtWidget(self):
        self.widgets = wid.OptionWidgets()
        for group,params in self._parameters.items():
            for param in params:
                if param.getOption("updateWidgets"):
                    param.changed.connect(lambda *a, **b : self.widgets.update())
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

class CameraSettings(BaseSettings):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.rotmat = glmath.Identity()
        self.zoom = 0.0
        self.ratio = 1.0
        self.dx = 0.0
        self.dy = 0.0

        self.fastmode = False

        self.min = glmath.Vector(3)
        self.min[:] = 0.0
        self.max = glmath.Vector(3)
        self.max[:] = 0.0

        self.near_plane = 0.1
        self.far_plane = 20.
        self.field_of_view = 0.8

        self.__members = ['zoom', 'ratio', 'dx', 'dy', 'fastmode', 'near_plane', 'far_plane', 'field_of_view']

    def rotateCamera(self, dx, dy):
        self.rotmat = glmath.RotateY(-dx/50.0)*self.rotmat
        self.rotmat = glmath.RotateX(-dy/50.0)*self.rotmat

    def moveCamera(self, dx, dy):
        s = 200.0*math.exp(-self.zoom/100)
        self.dx += dx/s
        self.dy += dy/s

    @property
    def center(self):
        if self._individual_rendering_parameters:
            return self._global_rendering_parameters.center
        return 0.5*(self.min+self.max)

    @property
    def _modelSize(self):
        if self._individual_rendering_parameters:
            return self._global_rendering_parameters._modelSize
        return math.sqrt(sum((self.max[i]-self.min[i])**2 for i in range(3)))

    @property
    def model(self):
        if self._individual_rendering_parameters:
            return self._global_rendering_parameters.model
        mat = glmath.Identity();
        mat = self.rotmat*mat;
        mat = glmath.Scale(2./self._modelSize) * mat if self._modelSize else mat
        mat = glmath.Translate(self.dx, -self.dy, -0 )*mat;
        mat = glmath.Scale(math.exp(-self.zoom/100))*mat;
        mat = glmath.Translate(0, -0, -5 )*mat;
        mat = mat*glmath.Translate(-self.center[0], -self.center[1], -self.center[2]) #move to center
        return mat

    @property
    def view(self):
        if self._individual_rendering_parameters:
            return self._global_rendering_parameters.view
        return glmath.LookAt()

    @property
    def projection(self):
        if self._individual_rendering_parameters:
            return self._global_rendering_parameters.projection
        return glmath.Perspective(self.field_of_view, self.ratio, self.near_plane, self.far_plane)

    def getSettings(self, recursive=True):
        res = super().getSettings() if recursive else {}
        for m in self.__members:
            res['Camera_' + m] = getattr(self, m)
        res['Camera_min'] = list(self.min)
        res['Camera_max'] = list(self.max)
        rotmat = [[self.rotmat[i,j] for j in range(3)] for i in range(3)]
        res['Camera_rotmat'] = rotmat
        return res

    def setSettings(self, settings):
        for m in self.__members:
            name = 'Camera_' + m
            setattr(self, m, settings.pop(name))
        self.min = glmath.Vector(settings.pop('Camera_min'))
        self.max = glmath.Vector(settings.pop('Camera_max'))
        self.rotmat = glmath.Identity()
        rm = settings.pop('Camera_rotmat')
        for i in range(3):
            for j in range(3):
                self.rotmat[i,j] = rm[i][j]
        if settings!= {}:
            super().setSettings(settings)

    def __getstate__(self):
        rotmat = [[self.rotmat[i,j] for j in range(3)] for i in range(3)]
        return (super().__getstate__(), self.getSettings(False))

    def __setstate__(self, state):
        super().__setstate__(state[0])
        self.setSettings(state[1])
        
def _patchGetterFunctionsWithGlobalSettings(obj, name_prefix, names, individual):
    def patchFunction(self, name):
        setattr(self, '_'+name, getattr(self, name))
        setattr(self, name, lambda: getattr(self, '_'+name)() if getattr(self, individual) else getattr(self._global_rendering_parameters, name)())
    for name in names:
        patchFunction(obj, name_prefix+name)

class ClippingSettings(BaseSettings):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if self._individual_rendering_parameters:
            options = [None, True, False]
        else:
            options = [False, True]
        self._individualClippingPlane = options[0]
        self.individualClippingPlaneChanged.connect(lambda: setattr(self,
                                                            "_individualClippingPlane",
                                                            options[(options.index(self._individualClippingPlane)+1)%len(options)]))
        self.individualClippingPlaneChanged.connect(self._updateGL)
        for par in self._individualClippingPlaneSubparameters:
            par.setVisible(False)
        if _have_qt:
            self.individualClippingPlaneChanged.connect(lambda : [par.setVisible(self._individualClippingPlane == True) for par in self._individualClippingPlaneSubparameters])
            self.individualClippingPlaneChanged.connect(self._updateVisibility)
            self.individualClippingPlaneChanged.connect(self.widgets.update)

    def _setter(self, value):
        while not self._individualClippingPlane == value:
            self.individualClippingPlaneChanged.emit()
    individualClippingPlane = property(lambda self: self._individualClippingPlane, _setter)

    def rotateClippingNormal(self, dx, dy, rotmat):
        """rotmat ... current camera rotation matrix"""
        n = glmath.Vector(self.getClippingNormal())
        n = rotmat.T*glmath.RotateY(-dx/50.0)*rotmat*n
        n = rotmat.T*glmath.RotateX(-dy/50.0)*rotmat*n
        self.setClippingNormal(n)

    def moveClippingPoint(self, d):
        n = glmath.Vector(self.getClippingNormal())
        p = glmath.Vector(self.getClippingPoint())
        self.setClippingPoint(p+d*n)

    def getClippingPlane(self):
        n = glmath.Vector(self.getClippingNormal())
        p = glmath.Vector(self.getClippingPoint())
        return glmath.Vector( [n[0], n[1], n[2], -glmath.Dot(n,p)])

    def getClippingPlanes(self):
        n = self.getClippingNPlanes()
        res = []
        for i in range(n):
            p = self._clipping_points[i].getValue()
            n = self._clipping_normals[i].getValue()
            res += n
            res.append( -sum([p[j]*n[j] for j in range(3)]) )
        return res

    def _updateVisibility(self, nplanes=None):
        if nplanes is None:
            nplanes = self.getClippingNPlanes()
        if self.individualClippingPlane:
            if nplanes<2:
                self._individualClippingPlaneSubparameters[1].setValue('p[0]')
                self._individualClippingPlaneSubparameters[1].setVisible(False)
            else:
                self._individualClippingPlaneSubparameters[1].setVisible(True)

            for i in range(3):
                self._clipping_points[i].setVisible(i<nplanes)
                self._clipping_normals[i].setVisible(i<nplanes)

    def _createParameters(self):
        super()._createParameters()
        self._individualClippingPlaneSubparameters = [
            ValueParameter(name="ClippingNPlanes", label="Number", default_value=1, max_value=3, min_value=1),
            TextParameter(name="ClippingExpression", label="Expression", default_value='p[0]')
        ]
        self._clipping_points = [
            VectorParameter(name="ClippingPoint", label="Point", default_value=(0.5,0.5,0.5), step=0.1),
            VectorParameter(name="ClippingPoint1", label="Point", default_value=(0.5,0.5,0.5), step=0.1),
            VectorParameter(name="ClippingPoint2", label="Point", default_value=(0.5,0.5,0.5), step=0.1),
            ]
        self._clipping_normals = [
            VectorParameter(name="ClippingNormal", label="Normal", min_value=-1.0, max_value=1.0, default_value=(1.0,0.0,0.0), step=0.1),
            VectorParameter(name="ClippingNormal1", label="Normal", min_value=-1.0, max_value=1.0, default_value=(0.0,1.0,0.0), step=0.1),
            VectorParameter(name="ClippingNormal2", label="Normal", min_value=-1.0, max_value=1.0, default_value=(0.0,0.0,1.0), step=0.1),
            ]
        if _have_qt:
            self._individualClippingPlaneSubparameters[0]._spinbox.valueChanged[int].connect(self._updateVisibility)
            self.individualClippingPlaneChanged.connect(self._updateVisibility)
        self._individualClippingPlaneSubparameters += [p for pair in zip(self._clipping_points,self._clipping_normals) for p in pair]
        if not self._individual_rendering_parameters:
            self.getClippingEnable = lambda : self.individualClippingPlane
        else:
            self.getClippingEnable = lambda : self.individualClippingPlane == True or (self.individualClippingPlane == None and self._global_rendering_parameters.individualClippingPlane == True)
        self.addParameters("Clipping", *self._individualClippingPlaneSubparameters)
        if self._individual_rendering_parameters:
            _patchGetterFunctionsWithGlobalSettings(self, 'getClipping', ['Point', 'Normal', 'NPlanes', 'Expression', 'Point1', 'Point2', 'Normal1', 'Normal2', 'Planes'], 'individualClippingPlane')

class LightSettings(BaseSettings):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if self._individual_rendering_parameters:
            options = [None, True, False]
        else:
            options = [True, False]
        self._individualLight = options[0]
        self.individualLightChanged.connect(lambda: setattr(self,
                                                            "_individualLight",
                                                            options[(options.index(self._individualLight)+1)%len(options)]))
        self.individualLightChanged.connect(self._updateGL)
        for par in self._individualLightSubparameters:
            par.setVisible(False)
        if _have_qt:
            self.individualLightChanged.connect(lambda : [par.setVisible(self._individualLight == True) for par in self._individualLightSubparameters])
            self.individualLightChanged.connect(self.widgets.update)

    def _setter(self, value):
        while not self._individualLight == value:
            self.individualLightChanged.emit()
    individualLight = property(lambda self: self._individualLight, _setter)
    def _createParameters(self):
        super()._createParameters()
        self._individualLightSubparameters = [
            ValueParameter(name="LightAmbient", label="ambient", min_value=0.0, max_value=1.0, default_value=0.3, step=0.1),
            ValueParameter(name="LightDiffuse", label="diffuse", min_value=0.0, max_value=1.0, default_value=0.7, step=0.1),
            ValueParameter(name="LightSpecular", label="specular", min_value=0.0, max_value=2.0, default_value=0.5, step=0.1),
            ValueParameter(name="LightShininess", label="shininess", min_value=0.0, max_value=100.0, default_value=50, step=1.0)]
        if not self._individual_rendering_parameters:
            self.getLightDisable = lambda : not self.individualLight
        else:
            self.getLightDisable = lambda : self.individualLight == False or (self.individualLight == None and self._global_rendering_parameters.individualLight == False)
        self.addParameters("Light", *self._individualLightSubparameters)
        if self._individual_rendering_parameters:
            _patchGetterFunctionsWithGlobalSettings(self, 'getLight', ['Ambient', 'Diffuse', 'Specular', 'Shininess'], 'individualLight')

class ColormapSettings(BaseSettings):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._colormap_tex = None
        options = [False, True]
        self._individualColormap = False
        self.individualColormapChanged.connect(lambda: setattr(self,
                                                               "_individualColormap",
                                                               options[(options.index(self._individualColormap)+1)%len(options)]))
        self.individualColormapChanged.connect(self._updateGL)
        for par in self._individualColormapSubparameters:
            par.setVisible(False)
        if _have_qt:
            self.individualColormapChanged.connect(lambda : [par.setVisible(self._individualColormap == True) for par in self._individualColormapSubparameters])
            self.individualColormapChanged.connect(self.widgets.update)

    def _setter(self, value):
        while not self._individualColormap == value:
            self.individualColormapChanged.emit()

    individualColormap = property(lambda self: self._individualColormap, _setter)

    def _createParameters(self):
        super()._createParameters()
        colormaps = ['netgen']
        try:
            import matplotlib.pyplot as plt
            colormaps += plt.colormaps()
        except:
            pass
        self._individualColormapSubparameters = [
            CheckboxParameter(name="ColormapAutoscale", label="autoscale", default_value=True),
            ValueParameter(name="ColormapMin", label="min", default_value=0.0),
            ValueParameter(name="ColormapMax", label="max", default_value=1.0),
            CheckboxParameter(name="ColormapLinear", label="linear", default_value=False),
            ValueParameter(name="ColormapSteps", label="steps", min_value=1, default_value=8),
            SingleOptionParameter(name="ColormapName",
                                                      values = colormaps,
                                                      label="map",
                                                      default_value = "netgen"),
            ]
        self._individualColormapSubparameters[-2].changed.connect(self._updateColormap)
        self._individualColormapSubparameters[-1].changed.connect(self._updateColormap)

        self.addParameters("Colormap", *self._individualColormapSubparameters)
        if self._individual_rendering_parameters:
            _patchGetterFunctionsWithGlobalSettings(self, 'getColormap', ['Min', 'Max', 'Name', 'Steps', 'Linear', 'Autoscale'], 'individualColormap')

    def _updateColormap(self):
        name = self.getColormapName()
        N = self.getColormapSteps()
        colors = []
        if name == 'netgen':
            for i in range(N):
                x = 1.0-i/(N-1)
                clamp = lambda x: int(255*(min(1.0, max(0.0, x))))
                colors.append(clamp(2.0-4.0*x))
                colors.append(clamp(2.0-4.0*abs(0.5-x)))
                colors.append(clamp(4.0*x - 2.0))
        else:
            import matplotlib.cm as cm
            import matplotlib.pyplot as plt
            cmap = cm.get_cmap(name, N)
            for i in range(N):
                colors+=cmap(i, bytes=True)[:3]

        import OpenGL.GL as GL
        if self._colormap_tex == None:
            from .gl import Texture
            self._colormap_tex = Texture(GL.GL_TEXTURE_1D, GL.GL_RGB)
            GL.glTexParameteri( GL.GL_TEXTURE_1D, GL.GL_TEXTURE_MAG_FILTER, GL.GL_LINEAR )
            GL.glTexParameteri( GL.GL_TEXTURE_1D, GL.GL_TEXTURE_MIN_FILTER, GL.GL_LINEAR )
            GL.glTexParameteri( GL.GL_TEXTURE_1D, GL.GL_TEXTURE_WRAP_S, GL.GL_CLAMP_TO_EDGE)

        self._colormap_tex.bind()
        GL.glTexImage1D(GL.GL_TEXTURE_1D, 0, GL.GL_RGB, N, 0, GL.GL_RGB, GL.GL_UNSIGNED_BYTE, colors)

    def getColormapTex(self):
        if self._individual_rendering_parameters and not self._individualColormap:
            return self._global_rendering_parameters.getColormapTex()
        if self._colormap_tex == None:
            self._updateColormap()
        return self._colormap_tex

