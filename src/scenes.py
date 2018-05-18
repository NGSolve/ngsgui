# from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
# from PySide2.QtCore import Qt
# from OpenGL import *
# from . import widgets
import ngsolve
# from .gl import *
# import numpy
# import time
# from . import glmath
# from . import ngui

from .gl import Texture, Program, ArrayBuffer, VertexArray
from . import widgets as wid
from .widgets import ArrangeH, ArrangeV
from . import glmath
from . import ngui
import math, cmath
from .thread import inmain_decorator

from PySide2 import QtWidgets, QtCore, QtGui
from OpenGL.GL import *
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

def addOption(self, group, name, default_value, typ=None, update_on_change=False, update_widget_on_change=False, widget_type=None, label=None, values=None, on_change=None, *args, **kwargs):
    if not group in self._widgets:
        self._widgets[group] = {}

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
        self._widgets[group][name] = WidgetWithLabel(w,label)

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
        self._widgets[group][name] = WidgetWithLabel(w)

    elif typ==int:
        w = QtWidgets.QSpinBox()
        w.setValue(default_value)
        w.valueChanged[int].connect(lambda value: getattr(self, setter_name)(value))
        if "min" in kwargs:
            w.setMinimum(kwargs["min"])
        if "max" in kwargs:
            w.setMaximum(kwargs["max"])
        self._widgets[group][name] = WidgetWithLabel(w,label)

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
        self._widgets[group][name] = WidgetWithLabel(w, label)

    elif typ=="button":
        def doAction(self, redraw=True):
            getattr(self,default_value)(*args, **kwargs)
            if update_on_change:
                self.update()
            if redraw:
                self.widgets.updateGLSignal.emit()

        cls = type(self)

        if not hasattr(cls, name):
            setattr(cls, name, doAction)

        w = wid.Button(label, getattr(self, name))
        self._widgets[group][name] = w

    else:
        raise RuntimeError("unknown type: ", typ)

    if typ!='button':
        def getValue(self):
            return getattr(self, propname)

        def setValue(self, value, redraw=True, update_gui=True):
            if getattr(self, propname) == value:
                return

            setattr(self, propname, value) 
            
            if update_widget_on_change:
                self.widgets.update()
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
    return w

import ngsolve
import numpy
import ctypes


class CMeshData:
    """Helper class to avoid redundant copies of the same mesh on the GPU."""

    """
    Vertex data:
        vec3 pos

    Surface elements:
        int v0,v1,v2;
        int curved_id; // to fetch curved element data, negative if element is not curved

    Surface curved elements:
        vec3 pos[3];     // Additional points for P2 interpolation
        vec3 normal[3];  // Normals for outer vertices

    Volume elements:
        int v0,v1,v2,v3;
        int curved_id; // to fetch curved element data, negative if element is not curved

    Volume curved elements:
        vec3 pos[6]; // Additional points for p2 interpolation

    Solution data (volume or surface):
        float values[N];   // N depends on order, subdivision
        vec3 gradients[N]; // N depends on order, subdivision

    """

    def __init__(self, mesh):
        import weakref
        self.mesh = weakref.ref(mesh)
        self.elements = Texture(GL_TEXTURE_BUFFER, GL_R32I)
        self.vertices = Texture(GL_TEXTURE_BUFFER, GL_RGB32F)
        mesh._opengl_data = self
        self.timestamp = self.mesh().ngmesh._timestamp
        self.update()

    def check_timestamp(self):
        if self.timestamp != self.mesh().ngmesh._timestamp:
            self.timestamp = self.mesh().ngmesh._timestamp
            self.mesh()._updateBuffers()
            print("do update")
            self.update()
        return self

    @inmain_decorator(True)
    def update(self):
        meshdata = ngui.GetMeshData(self.mesh())

        self.vertices.store(meshdata['vertices'])
        self.elements.store(meshdata["elements"])
        self.nedge_elements = meshdata["n_edge_elements"]
        self.nedges = meshdata["n_edges"]
        self.nperiodic_vertices = meshdata["n_periodic_vertices"]

        self.edges_offset = meshdata["edges_offset"]
        self.periodic_vertices_offset = meshdata["periodic_vertices_offset"]
        self.nsurface_elements = meshdata["n_surface_elements"]
        self.volume_elements_offset = meshdata["volume_elements_offset"]
        self.surface_elements_offset = meshdata["surface_elements_offset"]

        self.min = meshdata['min']
        self.max = meshdata['max']

def MeshData(mesh):
    """Helper function to avoid redundant copies of the same mesh on the GPU."""
    if hasattr(mesh,"_opengl_data"):
        return mesh._opengl_data.check_timestamp()
    else:
        return CMeshData(mesh)

class CGeoData:
    def __init__(self, geo):
        import weakref
        self.geo = weakref.ref(geo)
        self.vertices = Texture(GL_TEXTURE_BUFFER, GL_RGB32F)
        self.triangles = Texture(GL_TEXTURE_BUFFER, GL_RGBA32I)
        self.normals = Texture(GL_TEXTURE_BUFFER, GL_RGB32F)
        geo._opengl_data = self
        self.update()

    @inmain_decorator(True)
    def update(self):
        geodata = self.getGeoData()
        self.vertices.store(geodata["vertices"])
        self.triangles.store(geodata["triangles"])
        self.normals.store(geodata["normals"])
        self.surfnames = geodata["surfnames"]
        self.min = geodata["min"]
        self.max = geodata["max"]
        self.ntriangles = len(geodata["triangles"])//4*3

    def getGeoData(self):
        return ngui.GetGeoData(self.geo())

def GeoData(geo):
    try:
        return geo._opengl_data
    except:
        return CGeoData(geo)


class TextRenderer:
    class Font:
        pass

    def __init__(self):
        self.fonts = {}

        self.vao = VertexArray()
        self.addFont(0)

        self.program = Program('font.vert', 'font.geom', 'font.frag')
        self.characters = ArrayBuffer(usage=GL_DYNAMIC_DRAW)
        self.vao.unbind()

    def addFont(self, font_size):
        self.vao.bind()
        font = TextRenderer.Font()
        font.size = font_size

        db = QtGui.QFontDatabase()
        qfont = db.systemFont(db.FixedFont)
        if font_size>0:
            qfont.setPointSize(font_size)
        else:
            self.fonts[0] = font

        self.fonts[qfont.pointSize()] = font

        metrics = QtGui.QFontMetrics(qfont)

        font.width = metrics.maxWidth()
        font.height = metrics.height()

        font.tex_width = (1+128-32)*metrics.maxWidth()
        font.tex_width = (font.tex_width+3)//4*4 # should be multiple of 4
        font.tex_height = metrics.height()
        for i in range(32,128):
            c = bytes([i]).decode()

        image = QtGui.QImage(font.tex_width, font.tex_height, QtGui.QImage.Format_Grayscale8)
        image.fill(QtCore.Qt.black)

        painter = QtGui.QPainter()
        painter.begin(image)
        painter.setFont(qfont)
        painter.setPen(QtCore.Qt.white)
        for i in range(32,128):
            w = metrics.maxWidth()
            text = bytes([i]).decode()
            painter.drawText((i-32)*w,0, (i+1-32)*w, font.height, QtCore.Qt.AlignTop | QtCore.Qt.AlignLeft, text)
        painter.end()
        Z = numpy.array(image.bits()).reshape(font.tex_height, font.tex_width)

        font.tex = Texture(GL_TEXTURE_2D, GL_RED)
        font.tex.store(Z, GL_UNSIGNED_BYTE, Z.shape[1], Z.shape[0] )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST )

        self.vao.unbind()

    def draw(self, rendering_params, text, pos, font_size=0, use_absolute_pos=True, alignment=QtCore.Qt.AlignTop|QtCore.Qt.AlignLeft):

        if not font_size in self.fonts:
            self.addFont(font_size)

        self.vao.bind()
        glUseProgram(self.program.id)

        viewport = glGetIntegerv( GL_VIEWPORT )
        screen_width = viewport[2]-viewport[0]
        screen_height = viewport[3]-viewport[1]

        font = self.fonts[font_size]
        font.tex.bind()

        uniforms = self.program.uniforms
        uniforms.set('font_width_in_texture', font.width/font.tex_width)
        uniforms.set('font_height_in_texture', font.height/font.tex_height)
        uniforms.set('font_width_on_screen', 2*font.width/screen_width)
        uniforms.set('font_height_on_screen', 2*font.height/screen_height)

        if not use_absolute_pos:
            x = ngsolve.bla.Vector(4)
            for i in range(3):
                x[i] = pos[i]
            x[3] = 1.0
            model, view, projection = rendering_params.model, rendering_params.view, rendering_params.projection
            x = projection*view*model*x
            for i in range(3):
                pos[i] = x[i]/x[3]


        text_width = len(text)*2*font.width/screen_width
        text_height = 2*font.height/screen_height

        if alignment&QtCore.Qt.AlignRight:
            pos[0] -= text_width
        if alignment&QtCore.Qt.AlignBottom:
            pos[1] += text_height

        if alignment&QtCore.Qt.AlignCenter:
            pos[0] -= 0.5*text_width
        if alignment&QtCore.Qt.AlignVCenter:
            pos[1] += 0.5*text_height

        uniforms.set('start_pos', pos)

        s = numpy.array(list(text.encode('ascii', 'ignore')), dtype=numpy.uint8)
        self.characters.store(s)

        char_id = glGetAttribLocation(self.program.id, b'char_')
        glVertexAttribIPointer(char_id, 1, GL_UNSIGNED_BYTE, 0, ctypes.c_void_p());
        glEnableVertexAttribArray( char_id )

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_POINTS, 0, len(s))
        self.vao.unbind()

class SceneObject():
    scene_counter = 1
    @inmain_decorator(wait_for_return=True)
    def __init__(self,active=True, name = None, **kwargs):
        self.gl_initialized = False
        self.actions = {}
        self.active_action = None
        self.active = active
        self._widgets = {}
        if name is None:
            self.name = type(self).__name__.split('.')[-1] + str(SceneObject.scene_counter)
            SceneObject.scene_counter += 1
        else:
            self.name = name
        self.toolboxupdate = lambda me: None

    def __getstate__(self):
        return (self.name, self.active)

    def __setstate__(self,state):
        self.name = state[0]
        self.active = state[1]
        # TODO: can we pickle actions somehow?
        self.actions = {}
        self.gl_initialized = False

    def initGL(self):
        self.gl_initialized = True

    @inmain_decorator(True)
    def update(self):
        self.initGL()

    def render(self, settings):
        pass

    def deferRendering(self):
        """used to render some scenes later (eg. overlays, transparency)
        the higher the return value, the later it will be rendered"""
        return 0

    def getBoundingBox(self):
        box_min = ngsolve.bla.Vector(3)
        box_max = ngsolve.bla.Vector(3)
        box_min[:] = 1e99
        box_max[:] = -1e99
        return box_min,box_max

    def setActive(self, active, updateGL):
        self.active = active
        updateGL()

    def setWindow(self,window):
        self.window = window

    def getQtWidget(self, updateGL, params):
        self.widgets = wid.OptionWidgets(updateGL=updateGL)
        self._rendering_params = params

        self.actionCheckboxes = []

        class cbHolder:
            def __init__(self,cb,scene,name):
                self.scene = scene
                self.name = name
                self.cb = cb

            def __call__(self,state):
                if state:
                    self.scene.active_action = self.name
                    for cb in self.scene.actionCheckboxes:
                        if cb is not self.cb:
                            cb.setCheckState(QtCore.Qt.Unchecked)
                else:
                    if self.scene.active_action == self.name:
                        self.scene.active_action = None

        if self.actions:
            layout = QtWidgets.QVBoxLayout()
            for name,action in self.actions.items():
                cb = QtWidgets.QCheckBox(name)
                if self.active_action == name:
                    cb.setCheckState(QtCore.Qt.Checked)
                cb.stateChanged.connect(cbHolder(cb,self,name))
                self.actionCheckboxes.append(cb)
                layout.addWidget(cb)
            widget = QtWidgets.QWidget()
            widget.setLayout(layout)
            self.widgets.addGroup("Actions",widget)

        for group in self._widgets:
            self.widgets.addGroup(group,*self._widgets[group].values())

        return self.widgets

    def addAction(self,action,name=None):
        if name is None:
            name = "Action" + str(len(self.actions)+1)
        self.actions[name] = action
        self.active_action = name
        self.toolboxupdate(self)

    def doubleClickAction(self,point):
        if self.active_action:
            self.actions[self.active_action](point)

class BaseMeshSceneObject(SceneObject):
    """Base class for all scenes that depend on a mesh"""
    @inmain_decorator(wait_for_return=True)
    def __init__(self, mesh,**kwargs):
        super().__init__(**kwargs)
        self.mesh = mesh

    def initGL(self):
        if self.gl_initialized:
            return
        super().initGL()

    @inmain_decorator(True)
    def update(self):
        super().update()
        self.mesh_data = MeshData(self.mesh)

    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state, self.mesh)

    def __setstate__(self,state):
        super().__setstate__(state[0])
        self.mesh = state[1]

    def getBoundingBox(self):
        return self.mesh_data.min, self.mesh_data.max

class BaseFunctionSceneObject(BaseMeshSceneObject):
    """Base class for all scenes that depend on a coefficient function and a mesh"""
    @inmain_decorator(wait_for_return=True)
    def __init__(self, cf, mesh=None, order=3, gradient=None,**kwargs):
        self.cf = cf

        if gradient and cf.dim == 1:
            self.cf = ngsolve.CoefficientFunction((cf, gradient))
            self.have_gradient = True
        else:
            self.have_gradient = False

        super().__init__(mesh,**kwargs)

        addOption(self, "Subdivision", "Subdivision", typ=int, default_value=1, min=0, update_on_change=True)
        addOption(self, "Subdivision", "Order", typ=int, default_value=2, min=1, max=4, update_on_change=True)

        if 'sd' in kwargs:
            self.setSubdivision(kwargs['sd'], False, False)

        n = self.getOrder()*(2**self.getSubdivision())+1

    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state, self.cf, self.getSubdivision(), self.getOrder())

    def __setstate__(self, state):
        super().__setstate__(state[0])
        self.cf = state[1]
        self.setSubdivision(state[2])
        self.setOrder(state[3])


class OverlayScene(SceneObject):
    """Class  for overlay objects (Colormap, coordinate system, logo)"""
    @inmain_decorator(wait_for_return=True)
    def __init__(self,**kwargs):
        super().__init__(**kwargs)
        self.show_logo = True
        self.show_cross = True
        self.cross_scale = 0.3
        self.cross_shift = -0.10
        self.updateGL = lambda : None

        addOption(self, "Overlay", "ShowCross", True, label = "Axis", typ=bool)
        addOption(self, "Overlay", "ShowVersion", True, label = "Version", typ=bool)
        addOption(self, "Overlay", "ShowColorBar", True, label = "Color bar", typ=bool)

        import ngsolve.gui as G
        fastmode = hasattr(G.gui,'fastmode') and G.gui.fastmode
        addOption(self, "Rendering options", "FastRender", label='Fast mode', typ=bool, on_change=lambda val: setattr(self._rendering_params,'fastmode',val), default_value=fastmode)

        addOption(self, "Clipping plane", "clipX", label='X', typ='button', default_value='_setClippingPlane', action="clipX")
        addOption(self, "Clipping plane", "clipY", label='Y', typ='button', default_value='_setClippingPlane', action="clipY")
        addOption(self, "Clipping plane", "clipZ", label='Z', typ='button', default_value='_setClippingPlane', action="clipZ")
        addOption(self, "Clipping plane", "clipFlip", label='flip', typ='button', default_value='_setClippingPlane', action="clipFlip")

    def _setClippingPlane(self, action):
        if action == "clipX":
            self._rendering_params.setClippingPlaneNormal([1,0,0])
        if action == "clipY":
            self._rendering_params.setClippingPlaneNormal([0,1,0])
        if action == "clipZ":
            self._rendering_params.setClippingPlaneNormal([0,0,1])
        if action == "clipFlip":
            self._rendering_params.setClippingPlaneNormal(-1.0*self._rendering_params.getClippingPlaneNormal())


    def deferRendering(self):
        return 99

    def initGL(self):
        if self.gl_initialized:
            return
        super().initGL()

        self.text_renderer = TextRenderer()

        self.vao = VertexArray()
        self.cross_points = ArrayBuffer()
        points = [self.cross_shift + (self.cross_scale if i%7==3 else 0) for i in range(24)]
        self.cross_points.store(numpy.array(points, dtype=numpy.float32))

        self.program = Program('cross.vert','cross.frag')
        self.colorbar_program = Program('colorbar.vert','colorbar.frag')

        self.program.attributes.bind('pos', self.cross_points)

        self.vao.unbind()

    def render(self, settings):
        if not self.active:
            return

        self.update()
        glUseProgram(self.program.id)
        self.vao.bind()

        glDisable(GL_DEPTH_TEST)
        if self.getShowCross():
            model, view, projection = settings.model, settings.view, settings.projection
            mvp = glmath.Translate(-1+0.15/settings.ratio,-0.85,0)*projection*view*glmath.Translate(0,0,-5)*settings.rotmat

            self.program.uniforms.set('MVP',mvp)
            coords = glmath.Identity()
            for i in range(3):
                for j in range(3):
                    coords[i,j] = self.cross_shift+int(i==j)*self.cross_scale*1.2
            coords[3,:] = 1.0
            coords = mvp*coords
            for i in range(4):
                for j in range(4):
                    coords[i,j] = coords[i,j]/coords[3,j]

            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            glDrawArrays(GL_LINES, 0, 6)
            for i in range(3):
                self.text_renderer.draw(settings, "xyz"[i], coords[0:3,i], alignment=QtCore.Qt.AlignCenter|QtCore.Qt.AlignVCenter)
        if self.getShowVersion():
            self.text_renderer.draw(settings, "NGSolve " + ngsolve.__version__, [0.99,-0.99,0], alignment=QtCore.Qt.AlignRight|QtCore.Qt.AlignBottom)

        if self.getShowColorBar():
            glUseProgram(self.colorbar_program.id)
            uniforms = self.colorbar_program.uniforms
            x0,y0 = -0.6, 0.82
            dx,dy = 1.2, 0.03
            uniforms.set('x0', x0)
            uniforms.set('dx', dx)
            uniforms.set('y0', y0)
            uniforms.set('dy', dy)

            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            glDrawArrays(GL_TRIANGLES, 0, 6)
            cmin = settings.colormap_min
            cmax = settings.colormap_max
            for i in range(5):
                x = x0+i*dx/4
                val = cmin + i*(cmax-cmin)/4
                self.text_renderer.draw(settings, f'{val:.2g}'.replace("e+", "e"), [x,y0-0.03,0], alignment=QtCore.Qt.AlignCenter|QtCore.Qt.AlignTop)

        glEnable(GL_DEPTH_TEST)
        self.vao.unbind()

    @inmain_decorator(True)
    def update(self):
        super().update()

    def callupdateGL(self):
        self.updateGL()

    
class MeshScene(BaseMeshSceneObject):
    @inmain_decorator(wait_for_return=True)
    def __init__(self, mesh, wireframe=True, surface=True, elements=False, edgeElements=False, edges=False,
                 showPeriodic=False, pointNumbers=False, edgeNumbers=False, elementNumbers=False, **kwargs):
        super().__init__(mesh, **kwargs)

        self.qtWidget = None

        addOption(self, "Show", "ShowWireframe", typ=bool, default_value=wireframe, update_widget_on_change=True)
        addOption(self, "Show", "ShowSurface", typ=bool, default_value=surface, update_widget_on_change=True)
        addOption(self, "Show", "ShowElements", typ=bool, default_value=elements, update_widget_on_change=True)
        addOption(self, "Show", "ShowEdges", typ=bool, default_value=edges, update_widget_on_change=True)
        addOption(self, "Show", "ShowEdgeElements", typ=bool, default_value=edgeElements, update_widget_on_change=True)
        addOption(self, "Show", "ShowPeriodicVertices", typ=bool, default_value=showPeriodic, update_widget_on_change=True)
        addOption(self, "Numbers", "ShowPointNumbers", label="Points", typ=bool, default_value=pointNumbers, update_widget_on_change=True)
        addOption(self, "Numbers", "ShowEdgeNumbers", label="Edges", typ=bool, default_value=edgeNumbers, update_widget_on_change=True)
        addOption(self, "Numbers", "ShowElementNumbers", label="Elements", typ=bool, default_value=elementNumbers, update_widget_on_change=True)
        addOption(self, "", "GeomSubdivision", label="Subdivision", typ=int, default_value=5, min=1, max=20, update_widget_on_change=True)
        addOption(self, "", "Shrink", typ=float, default_value=1.0, min=0.0, max=1.0, step=0.01, update_widget_on_change=True)

    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state, self.show_wireframe, self.show_surface, self.show_elements)

    def __setstate__(self, state):
        super().__setstate__(state[0])
        self.show_wireframe, self.show_surface, self.show_elements = state[1:]
        self.qtWidget = None

    def initGL(self):
        if self.gl_initialized:
            return

        super().initGL()

        self.vao = VertexArray()
        self.numbers_program = Program('filter_elements.vert', 'numbers.geom', 'font.frag')
        self.bbnd_program = Program('filter_elements.vert', 'lines.tesc', 'lines.tese', 'mesh.frag')

        self.text_renderer = TextRenderer()

    def renderEdges(self, settings):
        self.vao.bind()
        glUseProgram(self.bbnd_program.id)
        model,view,projection = settings.model, settings.view, settings.projection
        uniforms = self.bbnd_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)

        glActiveTexture(GL_TEXTURE3)
        if self.mesh.dim == 3:
            self.bbnd_colors.bind()
        elif self.mesh.dim == 2:
            self.bc_colors.bind()
        else: # dim == 1
            self.tex_mat_color.bind()
        uniforms.set('colors', 3)

        uniforms.set('do_clipping', False);

        uniforms.set('mesh.dim', 1);
        uniforms.set('light_ambient', 1.0)
        uniforms.set('light_diffuse', 0.0)
        uniforms.set('TessLevel', self.getGeomSubdivision())
        uniforms.set('wireframe', True)
        if self.getShowEdges():
            glPatchParameteri(GL_PATCH_VERTICES, 1)
            glDrawArrays(GL_PATCHES, 0, self.mesh_data.nedges)
        if self.getShowEdgeElements():
            glLineWidth(3)
            glPatchParameteri(GL_PATCH_VERTICES, 1)
            glDrawArrays(GL_PATCHES, self.mesh_data.nedges,self.mesh_data.nedge_elements)
            glLineWidth(1)
        if self.getShowPeriodicVertices():
            glLineWidth(3)
            glPatchParameteri(GL_PATCH_VERTICES, 1)
            glDrawArrays(GL_PATCHES, self.mesh_data.nedge_elements+self.mesh_data.nedges, self.mesh_data.nperiodic_vertices)
            glLineWidth(1)

        self.vao.unbind()

    def renderSurface(self, settings):
        glUseProgram(self.surface_program.id)
        self.vao.bind()
        model, view, projection = settings.model, settings.view, settings.projection
        uniforms = self.surface_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)


        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', True);
        uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
        uniforms.set('mesh.volume_elements_offset', self.mesh_data.volume_elements_offset)
        uniforms.set('mesh.surface_elements_offset', self.mesh_data.surface_elements_offset)
        uniforms.set('mesh.dim', 2);
        uniforms.set('shrink_elements', self.getShrink())
        uniforms.set('clip_whole_elements', False)
        glActiveTexture(GL_TEXTURE3)
        if self.mesh.dim == 3:
            self.bc_colors.bind()
        elif self.mesh.dim == 2:
            self.tex_mat_color.bind()
        uniforms.set('colors', 3)


        if self.getShowSurface():
            uniforms.set('light_ambient', 0.3)
            uniforms.set('light_diffuse', 0.7)
            uniforms.set('TessLevel', self.getGeomSubdivision())
            uniforms.set('wireframe', False)
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )
            glPolygonOffset (2, 2)
            glEnable(GL_POLYGON_OFFSET_FILL)
            glPatchParameteri(GL_PATCH_VERTICES, 1)
            glDrawArrays(GL_PATCHES, 0, self.mesh_data.nsurface_elements)
            glDisable(GL_POLYGON_OFFSET_FILL)

        if self.getShowWireframe():
            uniforms.set('light_ambient', 0.0)
            uniforms.set('light_diffuse', 0.0)
            uniforms.set('TessLevel', self.getGeomSubdivision())
            uniforms.set('wireframe', True)
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            glPolygonOffset (1, 1)
            glEnable(GL_POLYGON_OFFSET_LINE)
            glPatchParameteri(GL_PATCH_VERTICES, 1)
            glDrawArrays(GL_PATCHES, 0, self.mesh_data.nsurface_elements)
            glDisable(GL_POLYGON_OFFSET_LINE)

        if self.getShowElements():
            uniforms.set('clip_whole_elements', True)
            uniforms.set('do_clipping', False);
            uniforms.set('light_ambient', 0.3)
            uniforms.set('light_diffuse', 0.7)
            uniforms.set('TessLevel', self.getGeomSubdivision())
            uniforms.set('wireframe', False)
            uniforms.set('mesh.dim', 3);
            glActiveTexture(GL_TEXTURE3)
            self.tex_mat_color.bind()
            uniforms.set('colors', 3)
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )
            glPatchParameteri(GL_PATCH_VERTICES, 1)
            glDrawArrays(GL_PATCHES, 0, self.mesh.ne)
        self.vao.unbind()

    def renderNumbers(self, settings):
        glUseProgram(self.numbers_program.id)
        self.vao.bind()
        model, view, projection = settings.model, settings.view, settings.projection
        uniforms = self.numbers_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        viewport = glGetIntegerv( GL_VIEWPORT )
        screen_width = viewport[2]-viewport[0]
        screen_height = viewport[3]-viewport[1]

        font_size = 0
        if not font_size in self.text_renderer.fonts:
            self.text_renderer.addFont(font_size)
        font = self.text_renderer.fonts[font_size]

        glActiveTexture(GL_TEXTURE2)
        font.tex.bind()
        uniforms.set('font', 2)

        uniforms.set('font_width_in_texture', font.width/font.tex_width)
        uniforms.set('font_height_in_texture', font.height/font.tex_height)
        uniforms.set('font_width_on_screen', 2*font.width/screen_width)
        uniforms.set('font_height_on_screen', 2*font.height/screen_height)
        uniforms.set('clipping_plane', settings.clipping_plane)

        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)

        if self.getShowPointNumbers():
            uniforms.set('mesh.dim', 0);
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )
            glPolygonOffset (0,0)
            glDrawArrays(GL_POINTS, 0, self.mesh.nv)

        if self.getShowEdgeNumbers():
            uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
            uniforms.set('mesh.volume_elements_offset', self.mesh_data.volume_elements_offset)
            uniforms.set('mesh.surface_elements_offset', self.mesh_data.surface_elements_offset)
            uniforms.set('mesh.dim', 1)
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )
            glPolygonOffset (0,0)
            glDrawArrays(GL_POINTS, 0, self.mesh_data.nedges)

        if self.getShowElementNumbers():
            uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
            uniforms.set('mesh.volume_elements_offset', self.mesh_data.volume_elements_offset)
            uniforms.set('mesh.surface_elements_offset', self.mesh_data.surface_elements_offset)
            uniforms.set('mesh.dim', self.mesh.dim)
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )
            glPolygonOffset (0,0)
            glDrawArrays(GL_POINTS, 0, self.mesh.ne)
        self.vao.unbind()



    @inmain_decorator(True)
    def update(self):
        super().update()
        nmats = len(self.mesh.GetMaterials())
        if self.mesh.dim == 3:
            self.mat_colors = [0,0,255,255] * nmats
        else:
            self.mat_colors = [0,255,0,255] * nmats
        self.tex_mat_color = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.tex_mat_color.store(self.mat_colors, GL_UNSIGNED_BYTE, nmats)
        self.bbnd_colors = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.bbnd_colors.store([1,0,0,1] * len(self.mesh.GetBBoundaries()),
                               data_format = GL_UNSIGNED_BYTE)

        self.surface_program = Program('filter_elements.vert', 'tess.tesc', 'tess.tese', 'mesh.geom', 'mesh.frag')
        self.bc_colors = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.bc_colors.store( [0,1,0,1]*len(self.mesh.GetBoundaries()),
                              data_format=GL_UNSIGNED_BYTE )

    def render(self, settings):
        if not self.active:
            return

        self.renderEdges(settings)

        self.renderSurface(settings)

        self.renderNumbers(settings)

    def updateBBNDColors(self):
        colors = []
        for c in self.bbndcolors.getColors():
            colors.append(c.red())
            colors.append(c.green())
            colors.append(c.blue())
            colors.append(c.alpha())
        self.bbnd_colors.store(colors, width=len(colors), data_format=GL_UNSIGNED_BYTE)

    def updateBndColors(self):
        colors = []
        for c in self.bndcolors.getColors():
            colors.append(c.red())
            colors.append(c.green())
            colors.append(c.blue())
            colors.append(c.alpha())
        self.bc_colors.store( colors, width=len(colors), data_format=GL_UNSIGNED_BYTE )

    def updateMatColors(self):
        colors = []
        for c in self.matcolors.getColors():
            colors.append(c.red())
            colors.append(c.green())
            colors.append(c.blue())
            colors.append(c.alpha())
        self.mat_colors = colors
        self.tex_mat_color.store(self.mat_colors, GL_UNSIGNED_BYTE, len(self.mesh.GetMaterials()))

    def getQtWidget(self, updateGL, params):
        super().getQtWidget(updateGL, params)

        mats = self.mesh.GetMaterials()
        bnds = self.mesh.GetBoundaries()
        if self.mesh.dim == 3:
            bbnds = self.mesh.GetBBoundaries()
        initial_mat_color = (0,0,255,255) if self.mesh.dim == 3 else (0,255,0,255)
        self.matcolors = wid.CollColors(self.mesh.GetMaterials(),initial_color=initial_mat_color)
        self.matcolors.colors_changed.connect(self.updateMatColors)
        self.matcolors.colors_changed.connect(updateGL)
        self.updateMatColors()
        def showVOL():
            if self.mesh.dim == 3:
                return self.getShowElements()
            elif self.mesh.dim == 2:
                return self.getShowSurface()
            elif self.mesh.dim == 1:
                return self.getShowEdges()
            return False
        self.widgets.addGroup("Materials", self.matcolors, connectedVisibility = showVOL)

        self.bndcolors = wid.CollColors(self.mesh.GetBoundaries(), initial_color=(0,255,0,255))
        self.bndcolors.colors_changed.connect(self.updateBndColors)
        self.bndcolors.colors_changed.connect(updateGL)
        self.updateBndColors()
        def showBND():
            if self.mesh.dim == 3:
                return self.getShowSurface()
            elif self.mesh.dim == 2:
                return self.getShowEdges()
            return False
        self.widgets.addGroup("Boundary Conditions", self.bndcolors, connectedVisibility = showBND)

        self.bbndcolors = wid.CollColors(self.mesh.GetBBoundaries(), initial_color=(0,0,0,255))
        self.bbndcolors.colors_changed.connect(self.updateBBNDColors)
        self.bbndcolors.colors_changed.connect(updateGL)
        self.updateBBNDColors()
        def showBBND():
            if self.mesh.dim == 3:
                return self.getShowEdgeElements()
            return False
        self.widgets.addGroup("BBoundaries", self.bbndcolors, connectedVisibility=showBBND)

        return self.widgets


class SolutionScene(BaseFunctionSceneObject):
    @inmain_decorator(wait_for_return=True)
    def __init__(self, cf, mesh, min=0,max=1, autoscale=True, linear=False, clippingPlane=True,*args, **kwargs):
        super().__init__(cf,mesh,*args, **kwargs)

        if self.mesh.dim>1:
            addOption(self, "Show", "ShowSurface", typ=bool, default_value=True)

        if self.mesh.dim > 2:
            addOption(self, "Show", "ShowClippingPlane", typ=bool, default_value=clippingPlane)
            addOption(self, "Show", "ShowIsoSurface", typ=bool, default_value=False)

        if cf.dim > 1:
            addOption(self, "Show", "Component", label="Component", typ=int, default_value=0)
            addOption(self, "Show", "ShowVectors", typ=bool, default_value=False)

        if self.cf.is_complex:
            addOption(self, "Complex", "ComplexEvalFunc", label="Func", typ=list, default_value=0, values=["real","imag","abs","arg"])
            addOption(self, "Complex", "ComplexPhaseShift", label="Value shift angle", typ=float, default_value=0.0)

        boxmin = addOption(self, "Colormap", "ColorMapMin", label="Min", typ=float, default_value=min,
                           step=1 if min == 0 else 10**(math.floor(math.log10(abs(min)))))
        boxmax = addOption(self, "Colormap", "ColorMapMax", label="Max" ,typ=float, default_value=max,
                           step=1 if max == 0 else 10**(math.floor(math.log10(abs(max)))))
        autoscale = addOption(self, "Colormap", "Autoscale",typ=bool, default_value=autoscale)
        addOption(self, "Colormap", "ColorMapLinear", label="Linear",typ=bool, default_value=linear)

        boxmin.changed.connect(lambda val: autoscale.stateChanged.emit(False))
        boxmax.changed.connect(lambda val: autoscale.stateChanged.emit(False))

        self.qtWidget = None
        self.vao = None

    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state, self.show_surface, self.show_clipping_plane)

    def __setstate__(self,state):
        super().__setstate__(state[0])
        self.show_surface, self.show_clipping_plane = state[1:]
        self.qtWidget = None
        self.vao = None

    def initGL(self):
        if self.gl_initialized:
            return
        super().initGL()

        formats = [None, GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F];
        self.volume_values = Texture(GL_TEXTURE_BUFFER, formats[self.cf.dim])
        self.volume_values_imag = Texture(GL_TEXTURE_BUFFER, formats[self.cf.dim])
        self.surface_values = Texture(GL_TEXTURE_BUFFER, formats[self.cf.dim])
        self.surface_values_imag = Texture(GL_TEXTURE_BUFFER, formats[self.cf.dim])

        # to filter elements (eg. elements intersecting with clipping plane or iso-surface)
        self.filter_program = Program('filter_elements.vert', 'filter_elements.geom', feedback=['element'])

        self.filter_buffer = ArrayBuffer()
        self.filter_buffer.bind()
        glBufferData(GL_ARRAY_BUFFER, 1000000, ctypes.c_void_p(), GL_STATIC_DRAW)

        self.filter_feedback = glGenTransformFeedbacks(1)
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, self.filter_feedback)
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, self.filter_buffer.id)
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0)

        self._have_filter = False

        # 1d solution (line)
        self.line_vao = VertexArray()
        self.line_program = Program('solution1d.vert', 'solution1d.frag')

        # solution on surface mesh
        self.surface_vao = VertexArray()
        self.surface_program = Program('filter_elements.vert', 'tess.tesc', 'tess.tese', 'solution.geom', 'solution.frag')

        # solution on clipping plane
        self.clipping_vao = VertexArray()
        self.clipping_program = Program('mesh.vert', 'clipping.geom', 'solution.frag')
        glUseProgram(self.clipping_program.id)

        # iso-surface
        self.iso_surface_vao = VertexArray()
        self.iso_surface_program = Program('mesh.vert', 'isosurface.geom', 'solution.frag')

        # vectors (currently one per element)
        self.vector_vao = VertexArray()
        self.vector_program = Program('mesh.vert', 'vector.geom', 'solution.frag')
        self.vector_vao.unbind()

    def _getValues(self, vb, setMinMax=True):
        cf = self.cf
        with ngsolve.TaskManager():
            values = ngui.GetValues(cf, self.mesh, vb, 2**self.getSubdivision()-1, self.getOrder())

        if setMinMax:
            self.min_values = values["min"]
            self.max_values = values["max"]
        return values

    @inmain_decorator(True)
    def update(self):
        super().update()
        self._have_filter = False
        if self.mesh.dim==1:
            try:
                values = self._getValues(ngsolve.VOL)
                self.surface_values.store(values["real"])
                if self.cf.is_complex:
                    self.surface_values_imag.store(values["imag"])
            except Exception as e:
                print("Cannot evaluate given function on 1d elements"+e)
        if self.mesh.dim==2:
            try:
                values = self._getValues(ngsolve.VOL)
                self.surface_values.store(values["real"])
                if self.cf.is_complex:
                    self.surface_values_imag.store(values["imag"])
            except Exception as e:
                print("Cannot evaluate given function on surface elements: "+e)
                self.show_surface = False

        if self.mesh.dim==3:
            values = self._getValues(ngsolve.VOL)
            self.volume_values.store(values["real"])
            if self.cf.is_complex:
                self.volume_values_imag.store(values["imag"])

            try:
                values = self._getValues(ngsolve.BND, False)
                self.surface_values.store(values["real"])
                if self.cf.is_complex:
                    self.surface_values_imag.store(values["imag"])
            except Exception as e:
                print("Cannot evaluate given function on surface elements"+e)
                self.show_surface = False

    def _filterElements(self, settings, filter_type):
        glEnable(GL_RASTERIZER_DISCARD)
        glUseProgram(self.filter_program.id)
        uniforms = self.filter_program.uniforms
        uniforms.set('clipping_plane', settings.clipping_plane)
        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)
        uniforms.set('mesh.dim', 3);

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)

        glActiveTexture(GL_TEXTURE2)
        self.volume_values.bind()
        uniforms.set('coefficients', 2)
        uniforms.set('colormap_min', settings.colormap_min)
        uniforms.set('colormap_max', settings.colormap_max)
        uniforms.set('subdivision', 2**self.getSubdivision()-1)
        uniforms.set('order', self.getOrder())
        if self.cf.dim > 1:
            uniforms.set('component', self.getComponent())
        else:
            uniforms.set('component', 0)

        uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
        uniforms.set('mesh.volume_elements_offset', self.mesh_data.volume_elements_offset)
        uniforms.set('mesh.surface_elements_offset', self.mesh_data.surface_elements_offset)
        uniforms.set('filter_type', filter_type)

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, self.filter_feedback)
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, self.filter_buffer.id)
        glBeginTransformFeedback(GL_POINTS)

        glDrawArrays(GL_POINTS, 0, self.mesh.ne)

        glEndTransformFeedback()
        glDisable(GL_RASTERIZER_DISCARD)

    def render1D(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection

        # surface mesh
        self.line_vao.bind()
        glUseProgram(self.line_program.id)

        uniforms = self.line_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        uniforms.set('colormap_min', settings.colormap_min)
        uniforms.set('colormap_max', settings.colormap_max)
        uniforms.set('colormap_linear', settings.colormap_linear)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', self.mesh.dim==3);
        uniforms.set('subdivision', 2**self.getSubdivision()-1)
        uniforms.set('order', self.getOrder())


        uniforms.set('element_type', 10)

        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)

        glActiveTexture(GL_TEXTURE2)
        self.surface_values.bind()
        uniforms.set('coefficients', 2)

        uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
        uniforms.set('mesh.surface_elements_offset', self.mesh_data.surface_elements_offset)

        glPolygonOffset (2,2)
        glEnable(GL_POLYGON_OFFSET_FILL)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_LINES, 0, self.mesh_data.nedge_elements*(2*(2**self.getSubdivision()*self.getOrder()+1)-2))
        self.line_vao.bind()

    def renderSurface(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection

        # surface mesh
        glUseProgram(self.surface_program.id)
        self.surface_vao.bind()

        uniforms = self.surface_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        uniforms.set('colormap_min', settings.colormap_min)
        uniforms.set('colormap_max', settings.colormap_max)
        uniforms.set('colormap_linear', settings.colormap_linear)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', self.mesh.dim==3);
        uniforms.set('subdivision', 2**self.getSubdivision()-1)
        uniforms.set('order', self.getOrder())
        uniforms.set('mesh.dim', 2);

        uniforms.set('element_type', 10)
        if self.cf.dim > 1:
            uniforms.set('component', self.getComponent())
        else:
            uniforms.set('component', 0)

        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)

        glActiveTexture(GL_TEXTURE2)
        self.surface_values .bind()
        uniforms.set('coefficients', 2)

        uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
        uniforms.set('mesh.surface_elements_offset', self.mesh_data.surface_elements_offset)

        uniforms.set('is_complex', self.cf.is_complex)
        if self.cf.is_complex:
            glActiveTexture(GL_TEXTURE3)
            self.surface_values_imag.bind()
            uniforms.set('coefficients_imag', 3)

            uniforms.set('complex_vis_function', self.getComplexEvalFunc())
            w = cmath.exp(1j*self.getComplexPhaseShift()/180.0*math.pi)
            uniforms.set('complex_factor', [w.real, w.imag])

        uniforms.set('TessLevel', max(1,2*self.getSubdivision()))
        uniforms.set('wireframe', False)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )
        glPolygonOffset (2, 2)
        glEnable(GL_POLYGON_OFFSET_FILL)
        glPatchParameteri(GL_PATCH_VERTICES, 1)
        glDrawArrays(GL_PATCHES, 0, self.mesh_data.nsurface_elements)
        glDisable(GL_POLYGON_OFFSET_FILL)
        self.surface_vao.unbind()

    def renderIsoSurface(self, settings):
        self._filterElements(settings, 1)
        model, view, projection = settings.model, settings.view, settings.projection
        glUseProgram(self.iso_surface_program.id)
        self.iso_surface_vao.bind()

        uniforms = self.iso_surface_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('colormap_min', settings.colormap_min)
        uniforms.set('colormap_max', settings.colormap_max)
        uniforms.set('colormap_linear', settings.colormap_linear)
        uniforms.set('have_gradient', self.have_gradient)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', True);
        uniforms.set('subdivision', 2**self.getSubdivision()-1)
        uniforms.set('order', self.getOrder())
        if self.cf.dim > 1:
            uniforms.set('component', self.getComponent())
        else:
            uniforms.set('component', 0)

        if(self.mesh.dim==2):
            uniforms.set('element_type', 10)
        if(self.mesh.dim==3):
            uniforms.set('element_type', 20)

        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)

        glActiveTexture(GL_TEXTURE2)
        self.volume_values.bind()
        uniforms.set('coefficients', 2)

        uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
        uniforms.set('mesh.volume_elements_offset', self.mesh_data.volume_elements_offset)

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        instances = (self.getOrder()*(2**self.getSubdivision()))**3
        self.iso_surface_program.attributes.bind('element', self.filter_buffer)
        glDrawTransformFeedbackInstanced(GL_POINTS, self.filter_feedback, instances)
        self.iso_surface_vao.unbind()

    def renderVectors(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection
        glUseProgram(self.vector_program.id)
        self.vector_vao.bind()

        uniforms = self.vector_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('colormap_min', 1e99)
        uniforms.set('colormap_max', -1e99)
        uniforms.set('colormap_linear', settings.colormap_linear)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', True);
        uniforms.set('subdivision', 2**self.getSubdivision()-1)
        uniforms.set('order', self.getOrder())

        if(self.mesh.dim==2):
            uniforms.set('element_type', 10)
        if(self.mesh.dim==3):
            uniforms.set('element_type', 20)

        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)

        glActiveTexture(GL_TEXTURE2)
        self.volume_values.bind()
        uniforms.set('coefficients', 2)

        uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
        uniforms.set('mesh.volume_elements_offset', self.mesh_data.volume_elements_offset)

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_POINTS, 0, self.mesh.ne)
        self.vector_vao.unbind()

    def renderClippingPlane(self, settings):
        self._filterElements(settings, 0)
        model, view, projection = settings.model, settings.view, settings.projection
        glUseProgram(self.clipping_program.id)
        self.clipping_vao.bind()

        uniforms = self.clipping_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('colormap_min', settings.colormap_min)
        uniforms.set('colormap_max', settings.colormap_max)
        uniforms.set('colormap_linear', settings.colormap_linear)
        uniforms.set('clipping_plane_deformation', False)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', False);
        uniforms.set('subdivision', 2**self.getSubdivision()-1)
        uniforms.set('order', self.getOrder())
        if self.cf.dim > 1:
            uniforms.set('component', self.getComponent())
        else:
            uniforms.set('component', 0)

        if(self.mesh.dim==2):
            uniforms.set('element_type', 10)
        if(self.mesh.dim==3):
            uniforms.set('element_type', 20)

        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)

        glActiveTexture(GL_TEXTURE2)
        self.volume_values.bind()
        uniforms.set('coefficients', 2)

        uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
        uniforms.set('mesh.volume_elements_offset', self.mesh_data.volume_elements_offset)

        uniforms.set('is_complex', self.cf.is_complex)
        if self.cf.is_complex:
            glActiveTexture(GL_TEXTURE3)
            self.volume_values_imag.bind()
            uniforms.set('coefficients_imag', 3)

            uniforms.set('complex_vis_function', self.getComplexEvalFunc())
            w = cmath.exp(1j*self.getComplexPhaseShift()/180.0*math.pi)
            uniforms.set('complex_factor', [w.real, w.imag])


        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        self.clipping_program.attributes.bind('element', self.filter_buffer)
        glDrawTransformFeedback(GL_POINTS, self.filter_feedback)
        self.clipping_vao.unbind()


    def render(self, settings):
        if not self.active:
            return

        if self.getAutoscale():
            comp = 0 if self.cf.dim==1 else self.getComponent()
            settings.colormap_min = self.min_values[comp]
            settings.colormap_max = self.max_values[comp]
        else:
            settings.colormap_min = self.getColorMapMin()
            settings.colormap_max = self.getColorMapMax()
        settings.colormap_linear = self.getColorMapLinear()

        if self.mesh.dim==1:
            self.render1D(settings)

        if self.mesh.dim > 1:
            if self.getShowSurface():
                self.renderSurface(settings)

        if self.mesh.dim > 2:
            if self.getShowIsoSurface():
                self.renderIsoSurface(settings)
            if self.getShowClippingPlane():
                self.renderClippingPlane(settings)

        if self.cf.dim > 1:
            if self.getShowVectors():
                self.renderVectors(settings)

class GeometryScene(SceneObject):
    @inmain_decorator(wait_for_return=True)
    def __init__(self, geo, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.geo = geo

    def initGL(self):
        if self.gl_initialized:
            return
        super().initGL()
        self.program = Program('geo.vert', 'mesh.frag')
        self.colors = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.vao = VertexArray()

    @inmain_decorator(True)
    def update(self):
        super().update()
        self.geo_data = self.getGeoData()
        self.surf_colors = { name : [0,0,255,255] for name in set(self.geo_data.surfnames)}
        self.colors.store([self.surf_colors[name][i] for name in self.geo_data.surfnames for i in range(4)],
                          data_format=GL_UNSIGNED_BYTE)

    def updateColors(self):
        self.colors.store(sum(([color.red(), color.green(), color.blue(), color.alpha()] for color in self.colorpicker.getColors()),[]),data_format=GL_UNSIGNED_BYTE)

    def getQtWidget(self, updateGL, params):
        super().getQtWidget(updateGL, params)
        self.colorpicker = wid.CollColors(self.surf_colors.keys(), initial_color = (0,0,255,255))
        self.colorpicker.colors_changed.connect(self.updateColors)
        self.colorpicker.colors_changed.connect(updateGL)
        self.updateColors()
        self.widgets.addGroup("Surface Colors", self.colorpicker)
        return self.widgets

    def getGeoData(self):
        return GeoData(self.geo)

    def getBoundingBox(self):
        return self.geo_data.min, self.geo_data.max

    def render(self, settings):
        if not self.active:
            return
        glUseProgram(self.program.id)
        self.vao.bind()
        model, view, projection = settings.model, settings.view, settings.projection
        uniforms = self.program.uniforms
        uniforms.set('P', projection)
        uniforms.set('MV', view*model)

        glActiveTexture(GL_TEXTURE0)
        self.geo_data.vertices.bind()
        uniforms.set('vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.geo_data.triangles.bind()
        uniforms.set('triangles',1)

        glActiveTexture(GL_TEXTURE2)
        self.geo_data.normals.bind()
        uniforms.set('normals',2)

        glActiveTexture(GL_TEXTURE3)
        self.colors.bind()
        uniforms.set('colors',3)

        uniforms.set('wireframe',False)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', True)
        uniforms.set('light_ambient', 0.3)
        uniforms.set('light_diffuse', 0.7)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL )
        glDrawArrays(GL_TRIANGLES, 0, self.geo_data.ntriangles)
        self.vao.unbind()
