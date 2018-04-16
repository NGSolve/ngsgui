# from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
# from PySide2.QtCore import Qt
# from OpenGL import *
# from . import widgets
# import ngsolve
# from .gl import *
# import numpy
# import time
# from . import glmath
# from . import ngui

from .gl import Texture, Program, ArrayBuffer
from . import widgets as wid
from .widgets import ArrangeH, ArrangeV
from . import glmath
from . import ngui

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
        else:
            self._value_widget.setValue(value)

def addOption(self, group, name, default_value, typ=None, update_on_change=False, update_widget_on_change=False, widget_type=None, label=None, *args, **kwargs):
    if not group in self._widgets:
        self._widgets[group] = {}

    label = label or name
    propname = "_"+name
    widgetname = "_"+name+"Widget"
    setter_name = "set"+name

    setattr(self, propname, default_value) 

    if typ==None and widget_type==None:
        typ = type(default_value)

    elif widget_type:
        w = widget_type(*args, **kwargs)
        w.setValue(default_value)
        self._widgets[group][name] = w

    elif typ==bool:
        cb = QtWidgets.QCheckBox(label)

        cb.setCheckState(QtCore.Qt.Checked if default_value else QtCore.Qt.Unchecked)
        cb.stateChanged.connect(lambda value: getattr(self, setter_name)(bool(value)))
        self._widgets[group][name] = WidgetWithLabel(cb)

    elif typ==int:
        box = QtWidgets.QSpinBox()
        box.valueChanged[int].connect(lambda value: getattr(self, setter_name)(value))
        w = WidgetWithLabel(box, label)
        self._widgets[group][name] = w 

    else:
        print("unknown type: ", typ)

    def getValue(self):
        return getattr(self, propname)

    def setValue(self, value, redraw=True, update_gui=True):
        if getattr(self, propname) == value:
            return

        setattr(self, propname, value) 
        
        if update_on_change:
            self.update()
        if update_widget_on_change:
            self.widgets.update()
        if redraw:
            self.widgets.updateGLSignal.emit()
            
        if update_gui:
            widget = self._widgets[group][name]
            widget.setValue(value)

    cls = type(self)

    if not hasattr(cls, setter_name):
        setattr(cls, setter_name, setValue)
    if not hasattr(cls, 'get'+name):
        setattr(cls, 'get'+name, getValue)

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
        self.update()

    def update(self):
        meshdata = ngui.GetMeshData(self.mesh())

        self.vertices.store(meshdata['vertices'])
        self.elements.store(meshdata["elements"])
        self.nedge_elements = meshdata["n_edge_elements"]
        self.nsurface_elements = meshdata["n_surface_elements"]
        self.volume_elements_offset = meshdata["volume_elements_offset"]
        self.surface_elements_offset = meshdata["surface_elements_offset"]
        self.min = meshdata['min']
        self.max = meshdata['max']

def MeshData(mesh):
    """Helper function to avoid redundant copies of the same mesh on the GPU."""
    try:
        return mesh._opengl_data
    except:
        return CMeshData(mesh)

class TextRenderer:
    class Font:
        pass

    def __init__(self):
        self.fonts = {}

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        self.addFont(0)

        self.program = Program('font.vert', 'font.geom', 'font.frag')
        self.characters = ArrayBuffer(usage=GL_DYNAMIC_DRAW)

    def addFont(self, font_size):
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

        glBindVertexArray(0)

    def draw(self, rendering_params, text, pos, font_size=0, use_absolute_pos=True, alignment=QtCore.Qt.AlignTop|QtCore.Qt.AlignLeft):

        if not font_size in self.fonts:
            self.addFont(font_size)

        glBindVertexArray(self.vao)
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

class SceneObject():
    scene_counter = 1
    def __init__(self,active=True, name = None):
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
    def __init__(self, mesh,**kwargs):
        super().__init__(**kwargs)
        self.mesh = mesh

    def initGL(self):
        if self.gl_initialized:
            return
        super().initGL()

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
    def __init__(self, cf, mesh=None, order=3, gradient=None, **kwargs):
        self.cf = cf
        if isinstance(cf, ngsolve.comp.GridFunction):
            if not gradient and cf.dim == 1:
                gradient = ngsolve.grad(cf)
            mesh = cf.space.mesh
        else:
            if mesh==None:
                raise RuntimeError("A mesh is needed if the given function is no GridFunction")

        if gradient and cf.dim == 1:
            self.cf = ngsolve.CoefficientFunction((cf, gradient))
            self.have_gradient = True
        else:
            self.have_gradient = False

        super().__init__(mesh,**kwargs)

        addOption(self, "Subdivision", "Subdivision", typ=int, default_value=0, update_on_change=True)
        addOption(self, "Subdivision", "Order", typ=int, default_value=1, update_on_change=True)

        n = self.getOrder()*(2**self.getSubdivision())+1

        self.colormap_min = -1
        self.colormap_max = 1
        self.colormap_linear = False

    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state, self.cf, self.getSubdivision(), self.getOrder(),
                self.colormap_min, self.colormap_max, self.colormap_linear)

    def __setstate__(self, state):
        super().__setstate__(state[0])
        self.cf = state[1]
        self.setSubdivision(state[2])
        self.setOrder(state[3])
        self.colormap_min = state[4]
        self.colormap_max = state[5]
        self.colormap_linear = state[6]

    def setColorMapMin(self, value):
        self.colormap_min = value

    def setColorMapMax(self, value):
        self.colormap_max = value

    def setColorMapLinear(self, value):
        self.colormap_linear = value

    def getQtWidget(self, updateGL, params):

        settings = wid.ColorMapSettings(min=-2, max=2, min_value=self.colormap_min, max_value=self.colormap_max)
        settings.layout().setAlignment(QtCore.Qt.AlignTop)

        settings.rangeMin.valueChanged.connect(self.setColorMapMin)
        settings.rangeMin.valueChanged.connect(updateGL)

        settings.rangeMax.valueChanged.connect(self.setColorMapMax)
        settings.rangeMax.valueChanged.connect(updateGL)

        settings.linearChanged.connect(self.setColorMapLinear)
        settings.linearChanged.connect(updateGL)

        super().getQtWidget(updateGL, params)
        self.widgets.addGroup("Colormap", settings)

        return self.widgets

class OverlayScene(SceneObject):
    """Class  for overlay objects (Colormap, coordinate system, logo)"""
    def __init__(self,**kwargs):
        super().__init__(**kwargs)
        self.show_logo = True
        self.show_cross = True
        self.cross_scale = 0.3
        self.cross_shift = -0.10
        self.active_layout = QtWidgets.QVBoxLayout()
        self.updateGL = lambda : None

    def deferRendering(self):
        return 99

    def initGL(self):
        if self.gl_initialized:
            return
        super().initGL()

        self.text_renderer = TextRenderer()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        self.cross_points = ArrayBuffer()
        points = [self.cross_shift + (self.cross_scale if i%7==3 else 0) for i in range(24)]
        self.cross_points.store(numpy.array(points, dtype=numpy.float32))

        self.program = Program('cross.vert','cross.frag')

        self.program.attributes.bind('pos', self.cross_points)

        glBindVertexArray(0)

    def render(self, settings):
        if not self.active:
            return

        self.update()
        glUseProgram(self.program.id)
        glBindVertexArray(self.vao)

        glDisable(GL_DEPTH_TEST)
        if self.show_cross:
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
        if self.show_logo:
            self.text_renderer.draw(settings, "NGSolve " + ngsolve.__version__, [0.99,-0.99,0], alignment=QtCore.Qt.AlignRight|QtCore.Qt.AlignBottom)

        glEnable(GL_DEPTH_TEST)
        glBindVertexArray(0)

    def setShowLogo(self, show):
        self.show_logo = show

    def setShowCross(self, show):
        self.show_cross = show

    def update(self):
        super().update()

    def callupdateGL(self):
        self.updateGL()

    def addScene(self,scene):
        callupdate = self.callupdateGL
        self.active_layout.addWidget(wid.CheckBox(scene.name,
                                                  wid.ObjectHolder(scene,
                                                                   lambda self,state:
                                                                   self.obj.setActive(state,callupdate)),
                                                  self.callupdateGL,
                                                  checked = scene.active))

    def getQtWidget(self, updateGL, params):
        self.updateGL = updateGL
        super().getQtWidget(updateGL, params)

        self.widgets.addGroup("Active Scenes",self.active_layout)

        logo = wid.CheckBox("Show version number", self.setShowLogo, updateGL, checked=self.show_logo)
        cross = wid.CheckBox("Show coordinate cross", self.setShowCross, updateGL,
                             checked=self.show_cross)
        self.widgets.addGroup("Overlay",logo, cross)
        clipx = wid.Button("X", lambda : params.setClippingPlaneNormal([1,0,0]), updateGL)
        clipy = wid.Button("Y", lambda : params.setClippingPlaneNormal([0,1,0]), updateGL)
        clipz = wid.Button("Z", lambda : params.setClippingPlaneNormal([0,0,1]))
        clip_flip = wid.Button("flip", lambda : params.setClippingPlaneNormal(-1.0*params.getClippingPlaneNormal()), updateGL)
        self.widgets.addGroup("Clipping plane",ArrangeH(clipx, clipy, clipz, clip_flip))
        return self.widgets

    
class MeshScene(BaseMeshSceneObject):
    def __init__(self, mesh, wireframe=True, surface=True, elements=False, shrink=1., **kwargs):
        super().__init__(mesh, **kwargs)

        self.qtWidget = None
        self.shrink = shrink
        self.tesslevel = 1.0

        addOption(self, "Show", "ShowWireframe", typ=bool, default_value=True, update_widget_on_change=True)
        addOption(self, "Show", "ShowSurface", typ=bool, default_value=True, update_widget_on_change=True)
        addOption(self, "Show", "ShowElements", typ=bool, default_value=False, update_widget_on_change=True)
        addOption(self, "Show", "ShowEdges", typ=bool, default_value=False, update_widget_on_change=True)

    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state, self.show_wireframe, self.show_surface, self.show_elements, self.shrink,
                self.tesslevel)

    def __setstate__(self, state):
        super().__setstate__(state[0])
        self.show_wireframe, self.show_surface, self.show_elements, self.shrink, self.tesslevel = state[1:]
        self.qtWidget = None

    def initGL(self):
        if self.gl_initialized:
            return

        super().initGL()

        self.bbnd_vao = glGenVertexArrays(1)
        glBindVertexArray(self.bbnd_vao)
        self.bbnd_program = Program("bbnd.vert","bbnd.frag")
        self.bbnd_colors = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.bbnd_colors.store([1,0,0,1] * len(self.mesh.GetBBoundaries()),
                               data_format = GL_UNSIGNED_BYTE)

        self.surface_vao = glGenVertexArrays(1)
        glBindVertexArray(self.surface_vao)

        self.surface_program = Program('mesh.vert', 'tess.tesc', 'tess.tese', 'mesh.frag')
        self.bc_colors = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.bc_colors.store( [0,1,0,1]*len(self.mesh.GetBoundaries()),
                              data_format=GL_UNSIGNED_BYTE )

        self.element_program = Program('elements.vert','elements.geom','elements.frag')

        self.elements_vao = glGenVertexArrays(1)
        glBindVertexArray(self.elements_vao)

        glBindVertexArray(0)

    def renderBBND(self, settings):
        glUseProgram(self.bbnd_program.id)
        glBindVertexArray(self.bbnd_vao)
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

        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', True);

        if self.getShowEdges():
            uniforms.set('light_ambient', 0.3)
            uniforms.set('light_diffuse', 0.7)
            glLineWidth(3)
            glPolygonOffset (2, 2)
            glDrawArrays(GL_LINES, 0, 2*self.mesh_data.nedge_elements)
            # glDisable(GL_LINE_SMOOTH)
            glLineWidth(1)

    def renderSurface(self, settings):
        glUseProgram(self.surface_program.id)
        glBindVertexArray(self.surface_vao)

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

        glActiveTexture(GL_TEXTURE3)
        if self.mesh.dim == 3:
            self.bc_colors.bind()
        elif self.mesh.dim == 2:
            self.tex_mat_color.bind()
        uniforms.set('colors', 3)

        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', True);
        uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
        uniforms.set('mesh.volume_elements_offset', self.mesh_data.volume_elements_offset)
        uniforms.set('mesh.surface_elements_offset', self.mesh_data.surface_elements_offset)


        if self.getShowSurface():
            uniforms.set('light_ambient', 0.3)
            uniforms.set('light_diffuse', 0.7)
            uniforms.set('TessLevel', self.tesslevel)
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
            uniforms.set('TessLevel', self.tesslevel)
            uniforms.set('wireframe', True)
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            glPolygonOffset (1, 1)
            glEnable(GL_POLYGON_OFFSET_LINE)
            glPatchParameteri(GL_PATCH_VERTICES, 1)
            glDrawArrays(GL_PATCHES, 0, self.mesh_data.nsurface_elements)
            glDisable(GL_POLYGON_OFFSET_LINE)


    def update(self):
        print('mesh update')
        super().update()
        glBindVertexArray(self.surface_vao)

        glBindVertexArray(self.elements_vao)
        nmats = len(self.mesh.GetMaterials())
        if self.mesh.dim == 3:
            self.mat_colors = [0,0,255,255] * nmats
        else:
            self.mat_colors = [0,255,0,255] * nmats
        self.tex_mat_color = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.tex_mat_color.store(self.mat_colors, GL_UNSIGNED_BYTE, nmats)
        glBindVertexArray(0)

    def render(self, settings):
        if not self.active:
            return

        self.renderSurface(settings)

        if self.getShowElements():
            self.renderElements(settings)
        if self.getShowEdges():
            self.renderBBND(settings)

    def renderElements(self, settings):
        glBindVertexArray(self.elements_vao)
        glUseProgram(self.element_program.id)

        model, view, projection = settings.model, settings.view, settings.projection
        uniforms = self.element_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('light_ambient', 0.3)
        uniforms.set('light_diffuse', 0.7)

        uniforms.set('shrink_elements', self.shrink)
        uniforms.set('clipping_plane', settings.clipping_plane)

        glActiveTexture(GL_TEXTURE0)
        self.mesh_data.vertices.bind()
        uniforms.set('mesh.vertices', 0)

        glActiveTexture(GL_TEXTURE1)
        self.mesh_data.elements.bind()
        uniforms.set('mesh.elements', 1)

        uniforms.set('mesh.volume_elements_offset', self.mesh_data.volume_elements_offset)
        glActiveTexture(GL_TEXTURE3)
        self.tex_mat_color.bind()
        uniforms.set('colors', 3)

#         glPolygonOffset (2,2)
#         glEnable(GL_POLYGON_OFFSET_FILL)
        glDisable(GL_POLYGON_OFFSET_FILL)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_POINTS, 0, self.mesh.ne)
        glDisable(GL_POLYGON_OFFSET_FILL)

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

    def setShrink(self, value):
        self.shrink = value

    def setTessellation(self, value):
        self.tesslevel = value

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

        self.bbndcolors = wid.CollColors(self.mesh.GetBBoundaries(), initial_color=(255,0,0,255))
        self.bbndcolors.colors_changed.connect(self.updateBBNDColors)
        self.bbndcolors.colors_changed.connect(updateGL)
        self.updateBBNDColors()
        def showBBND():
            if self.mesh.dim == 3:
                return self.getShowEdges()
            return False
        self.widgets.addGroup("BBoundaries", self.bbndcolors, connectedVisibility=showBBND)

        if self.mesh.dim == 3:
            shrink = wid.RangeGroup("Shrink", min=0.0, max=1.0, value=self.shrink)
            shrink.valueChanged.connect(self.setShrink)
            shrink.valueChanged.connect(updateGL)
            self.widgets.addGroup("Shrink",shrink, connectedVisibility = lambda: self.getShowElements())

        tess = QtWidgets.QDoubleSpinBox()
        tess.setRange(1, 20)
        tess.valueChanged[float].connect(self.setTessellation)
        tess.setSingleStep(1.0)
        tess.valueChanged[float].connect(updateGL)
        self.widgets.addGroup("Tesselation", tess)

        return self.widgets


class SolutionScene(BaseFunctionSceneObject):
    def __init__(self, cf, *args, **kwargs):
        super().__init__(cf,*args, **kwargs)

        if self.mesh.dim>1:
            addOption(self, "Show", "ShowSurface", typ=bool, default_value=True)

        if self.mesh.dim > 2:
            addOption(self, "Show", "ShowClippingPlane", typ=bool, default_value=False)

        if cf.dim > 1:
            addOption(self, "Show", "Component", label="Component", typ=int, default_value=0)
            addOption(self, "Show", "ShowVectors", typ=bool, default_value=False)

        if self.mesh.dim > 2:
            addOption(self, "Show", "ShowIsoSurface", typ=bool, default_value=False)

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
        self.surface_values = Texture(GL_TEXTURE_BUFFER, formats[self.cf.dim])

        # 1d solution (line)
        self.line_vao = glGenVertexArrays(1)
        glBindVertexArray(self.line_vao)
        self.line_program = Program('solution1d.vert', 'solution1d.frag')
        glBindVertexArray(0)

        # solution on surface mesh
        self.surface_vao = glGenVertexArrays(1)
        glBindVertexArray(self.surface_vao)
        self.surface_program = Program('solution.vert', 'solution.frag')
        glBindVertexArray(0)

        # solution on clipping plane
        self.clipping_vao = glGenVertexArrays(1)
        glBindVertexArray(self.clipping_vao)
        self.clipping_program = Program('clipping.vert', 'clipping.geom', 'solution.frag')
        glUseProgram(self.clipping_program.id)
        glBindVertexArray(0)

        # iso-surface
        self.iso_surface_vao = glGenVertexArrays(1)
        glBindVertexArray(self.iso_surface_vao)
        self.iso_surface_program = Program('clipping.vert', 'isosurface.geom', 'solution.frag')
        glBindVertexArray(0)

        self.vector_vao = glGenVertexArrays(1)
        glBindVertexArray(self.vector_vao)
        self.vector_program = Program('clipping.vert', 'vector.geom', 'solution.frag')
        glBindVertexArray(0)


    def update(self):
        super().update()
        if self.mesh.dim==1:
            try:
                values = ngui.GetValues(self.cf, self.mesh, ngsolve.VOL, 2**self.getSubdivision()-1, self.getOrder())
                self.surface_values.store(values)
                self.min_values = values[0:self.cf.dim]
                self.max_values = values[self.cf.dim:2*self.cf.dim]
            except:
                print("Cannot evaluate given function on 1d elements")
        if self.mesh.dim==2:
            try:
                self.surface_values.store(ngui.GetValues(self.cf, self.mesh, ngsolve.VOL, 2**self.getSubdivision()-1, self.getOrder()))
            except:
                print("Cannot evaluate given function on surface elements")
                self.show_surface = False

        if self.mesh.dim==3:
            values = ngui.GetValues(self.cf, self.mesh, ngsolve.VOL, 2**self.getSubdivision()-1, self.getOrder() )
            self.volume_values.store(values)
            try:
                self.surface_values.store(ngui.GetValues(self.cf, self.mesh, ngsolve.BND, 2**self.getSubdivision()-1, self.getOrder()))
            except:
                print("Cannot evaluate given function on surface elements")
                self.show_surface = False

    def render1D(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection

        # surface mesh
        glBindVertexArray(self.line_vao)
        glUseProgram(self.line_program.id)

        uniforms = self.line_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        uniforms.set('colormap_min', self.colormap_min)
        uniforms.set('colormap_max', self.colormap_max)
        uniforms.set('colormap_linear', self.colormap_linear)
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
        glBindVertexArray(self.line_vao)

    def renderSurface(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection

        # surface mesh
        glBindVertexArray(self.surface_vao)
        glUseProgram(self.surface_program.id)

        uniforms = self.surface_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        uniforms.set('colormap_min', self.colormap_min)
        uniforms.set('colormap_max', self.colormap_max)
        uniforms.set('colormap_linear', self.colormap_linear)
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
        self.surface_values .bind()
        uniforms.set('coefficients', 2)

        uniforms.set('mesh.surface_curved_offset', self.mesh.nv)
        uniforms.set('mesh.surface_elements_offset', self.mesh_data.surface_elements_offset)

        glPolygonOffset (2,2)
        glEnable(GL_POLYGON_OFFSET_FILL)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh_data.nsurface_elements)

    def renderIsoSurface(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection
        glUseProgram(self.iso_surface_program.id)
        glBindVertexArray(self.iso_surface_vao)

        uniforms = self.iso_surface_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('colormap_min', self.colormap_min)
        uniforms.set('colormap_max', self.colormap_max)
        uniforms.set('colormap_linear', self.colormap_linear)
        uniforms.set('have_gradient', self.have_gradient)
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
        instances = (self.getOrder()*(2**self.getSubdivision()))**3
        glDrawArraysInstanced(GL_POINTS, 0, self.mesh.ne, instances)
        glBindVertexArray(0)

    def renderVectors(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection
        glUseProgram(self.vector_program.id)
        glBindVertexArray(self.vector_vao)

        uniforms = self.vector_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('colormap_min', 1e99)
        uniforms.set('colormap_max', -1e99)
        uniforms.set('colormap_linear', self.colormap_linear)
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
        glBindVertexArray(0)

    def renderClippingPlane(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection
        glUseProgram(self.clipping_program.id)
        glBindVertexArray(self.clipping_vao)

        uniforms = self.clipping_program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('colormap_min', self.colormap_min)
        uniforms.set('colormap_max', self.colormap_max)
        uniforms.set('colormap_linear', self.colormap_linear)
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

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_POINTS, 0, self.mesh.ne)
        glBindVertexArray(0)


    def render(self, settings):
        if not self.active:
            return

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
