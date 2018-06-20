
import ngsolve
import numpy

from .gl import Texture, getProgram, ArrayBuffer, VertexArray, TextRenderer
from . import widgets as wid
from .widgets import ArrangeH, ArrangeV
from . import glmath
from . import ngui
import math, cmath
from .thread import inmain_decorator
from .gl_interface import getOpenGLData
from .gui import GUI
import netgen.meshing
from . import settings

from PySide2 import QtWidgets, QtCore, QtGui
from OpenGL.GL import *


class BaseScene(settings.BaseSettings):
    """Base class for drawing opengl objects.

Parameters
----------
active : bool = True
  Specifies if scene should be visible.
name : str = type(self).__name__ + scene_counter
  Name of scene in right hand side menu.
"""
    scene_counter = 1
    @inmain_decorator(wait_for_return=True)
    def __init__(self,active=True, name = None, **kwargs):
        self.window = None
        self._gl_initialized = False
        self._actions = {}
        self._active_action = None
        self._active = active
        if name is None:
            self.name = type(self).__name__.split('.')[-1] + str(BaseScene.scene_counter)
            BaseScene.scene_counter += 1
        else:
            self.name = name
        super().__init__()

    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state, self.name, self.active)

    def __setstate__(self,state):
        self.window = None
        self._gl_initialized = False
        self._actions = {}
        self._active_action = None
        self.name = state[1]
        self.active = state[2]
        super().__setstate__(state[0])
        # TODO: can we pickle actions somehow?

    def initGL(self):
        """Called once after the scene is created and initializes all OpenGL objects."""
        self._gl_initialized = True

    @inmain_decorator(True)
    def update(self):
        """Called on startup and if underlying object changes, reloads data on GPU if drawn object changed"""
        if not self._gl_initialized:
            self.initGL()

    def render(self, settings):
        """Render scene, must be overloaded by derived class"""
        pass

    def deferRendering(self):
        """used to render some scenes later (eg. overlays, transparency)
        the higher the return value, the later it will be rendered"""
        return 0

    def getBoundingBox(self):
        """Returns bounding box of scene object, The center of the drawn scene will be in the
center of this box. Rotation will be around this center."""
        box_min = ngsolve.bla.Vector(3)
        box_max = ngsolve.bla.Vector(3)
        box_min[:] = 1e99
        box_max[:] = -1e99
        return box_min,box_max

    def _setActive(self, _active):
        """Toggle visibility of scene"""
        self._active = _active
        self._updateGL()
    def _getActive(self):
        return self._active
    active = property(_getActive,_setActive)

    @inmain_decorator(True)
    def _createQtWidget(self):
        super()._createQtWidget()
        self.widgets.updateGLSignal.connect(self._updateGL)
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

        if self._actions:
            layout = QtWidgets.QVBoxLayout()
            for name,action in self._actions.items():
                cb = QtWidgets.QCheckBox(name)
                if self._active_action == name:
                    cb.setCheckState(QtCore.Qt.Checked)
                cb.stateChanged.connect(cbHolder(cb,self,name))
                self.actionCheckboxes.append(cb)
                layout.addWidget(cb)
            widget = QtWidgets.QWidget()
            widget.setLayout(layout)
            self.widgets.addGroup("Actions",widget)

    def _updateGL(self):
        if self.window:
            self.window.glWidget.updateGL()

    def _attachParameter(self, parameter):
        super()._attachParameter(parameter)
        if parameter.getOption("updateScene"):
            parameter.changed.connect(lambda val: self.update())
        if not parameter.getOption("notUpdateGL"):
            parameter.changed.connect(self._updateGL)

    def addAction(self,action,name=None):
        """Add double click action. Adds a checkbox to the widget to activate/deactivate the action.
If double clicked on a point in the drawing domain, the action function is executed with the coordinates
of the clicked point in the scene.

Parameters
----------
action : function
  Action must take in a tuple of the 3 coordinates returned from clicking in the 3D drawing domain.
  for example : action = lambda p: print(p)
  would print the coordinates of the clicked point
name : str = "action" + consecutive number
  Name of the action. The checkbox in the right hand menu is label accordingly.
"""
        if name is None:
            name = "Action" + str(len(self._actions)+1)
        self._actions[name] = action
        self._active_action = name

    def doubleClickAction(self,point):
        if self._active_action:
            self._actions[self._active_action](point)

GUI.sceneCreators.append((BaseScene,lambda scene,*args,**kwargs: scene))

class BaseMeshScene(BaseScene):
    """Base class for all scenes that depend on a mesh"""
    @inmain_decorator(wait_for_return=True)
    def __init__(self, mesh,**kwargs):
        self.mesh = mesh
        super().__init__(**kwargs)

    def initGL(self):
        super().initGL()

    @inmain_decorator(True)
    def update(self):
        super().update()
        self.mesh_data = getOpenGLData(self.mesh)

    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state, self.mesh)

    def __setstate__(self,state):
        self.mesh = state[1]
        super().__setstate__(state[0])

    def getBoundingBox(self):
        return self.mesh_data.min, self.mesh_data.max

class OverlayScene(BaseScene):
    """Class  for overlay objects (Colormap, coordinate system, logo)"""
    @inmain_decorator(wait_for_return=True)
    def __init__(self,rendering_parameters=None,**kwargs):
        import ngsolve.gui as G
        self._rendering_parameters = rendering_parameters
        self._initial_values = { "ShowCross" : True,
                                 "ShowVersion" : True,
                                 "ShowColorBar" : True,
                                 "FastRender" : hasattr(G.gui,'fastmode') and G.gui.fastmode}
        super().__init__(**kwargs)

    def __getstate__(self):
        superstate = super().__getstate__()
        return (superstate,self._rendering_parameters)

    def __setstate__(self, state):
        super().__setstate__(state[0])
        self._rendering_parameters = state[1]

    @inmain_decorator(True)
    def _createParameters(self):
        super()._createParameters()
        self.addParameters("Overlay",
                           settings.CheckboxParameter(name="ShowVersion",
                                                      label="Version",
                                                      default_value=True),
                           settings.CheckboxParameter(name="ShowCross",
                                                      label = "Axis", default_value=True),
                           settings.CheckboxParameter(name="ShowColorBar",
                                                      label = "Color Bar",
                                                      default_value=True))
        fastmode_par = settings.CheckboxParameter(name="FastRender",
                                                  label="Fast mode")
        fastmode_par.changed.connect(lambda val: setattr(self._rendering_parameters,"fastmode", val))
        self.addParameters("Rendering options", fastmode_par)


    @inmain_decorator(True)
    def _createOptions(self):
        super()._createOptions()
        self.addButton( "Clipping plane", "clipX", self._setClippingPlane, label='X',action="clipX")
        self.addButton( "Clipping plane", "clipY", self._setClippingPlane, label='Y',action="clipY")
        self.addButton( "Clipping plane", "clipZ", self._setClippingPlane, label='Z',action="clipZ")
        self.addButton( "Clipping plane", "clipFlip", self._setClippingPlane, label='flip', action="clipFlip")
        self.cross_scale = 0.3
        self.cross_shift = -0.10

    def copyOptionsFrom(self, other):
        self.setShowCross(other.getShowCross())
        self.setShowVersion(other.getShowVersion())
        self.setShowColorBar(other.getShowColorBar())
        self.setFastRender(other.getFastRender())
        self._rendering_parameters.__dict__.update(other._rendering_parameters.__dict__)

    def _setClippingPlane(self, action):
        if action == "clipX":
            self._rendering_parameters.setClippingPlaneNormal([1,0,0])
        if action == "clipY":
            self._rendering_parameters.setClippingPlaneNormal([0,1,0])
        if action == "clipZ":
            self._rendering_parameters.setClippingPlaneNormal([0,0,1])
        if action == "clipFlip":
            self._rendering_parameters.setClippingPlaneNormal(-1.0*self._rendering_parameters.getClippingPlaneNormal())

    def deferRendering(self):
        return 99

    def initGL(self):
        super().initGL()

        self.text_renderer = TextRenderer()

        self.vao = VertexArray()
        self.cross_points = ArrayBuffer()

        self.program = getProgram('cross.vert','cross.frag')

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
            self.program.attributes.bind('pos', self.cross_points)
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
            prog = getProgram('colorbar.vert','colorbar.frag')
            self.vao.bind()
            uniforms = prog.uniforms
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
        points = [self.cross_shift + (self.cross_scale if i%7==3 else 0) for i in range(24)]
        self.cross_points.store(numpy.array(points, dtype=numpy.float32))


class MeshScene(BaseMeshScene):
    @inmain_decorator(wait_for_return=True)
    def __init__(self, mesh, wireframe=True, surface=True, elements=False, edgeElements=False, edges=False,
                 showPeriodic=False, pointNumbers=False, edgeNumbers=False, elementNumbers=False, **kwargs):
        self._initial_values = { "ShowWireframe" : wireframe,
                                 "ShowSurface" : surface,
                                 "ShowElements" : elements,
                                 "ShowEdges" : edges,
                                 "ShowEdgeElements" : edgeElements,
                                 "ShowPeriodicVertices" : showPeriodic,
                                 "ShowPointNumbers" : pointNumbers,
                                 "ShowEdgeNumbers" : edgeNumbers,
                                 "ShowElementNumbers" : elementNumbers}
        self.tex_mat_colors = self.tex_bc_colors = self.tex_bbnd_colors = None
        super().__init__(mesh, **kwargs)

    @inmain_decorator(True)
    def _createParameters(self):
        super()._createParameters()
        self.addParameters("Show",
                           settings.CheckboxParameter(name="ShowWireframe", label="Show Wireframe",
                                                      default_value = self._initial_values["ShowWireframe"]))
        if self.mesh.dim > 1:
            surf_values = self.mesh.GetBoundaries() if self.mesh.dim == 3 else self.mesh.GetMaterials()
            surf_color = settings.ColorParameter(name="SurfaceColors", values = surf_values,
                                                 default_value = (0,255,0,255))
            surf_color.changed.connect(lambda : self.tex_bc_colors.store(surf_color.getValue(),
                                                                         data_format=GL_UNSIGNED_BYTE))
            self.addParameters("Show",
                               settings.CheckboxParameterCluster(name="ShowSurface", label="Surface Elements",
                                                                 default_value = self._initial_values["ShowSurface"],
                                                                 sub_parameters = [surf_color],
                                                                 updateWidgets=True))
        if self.mesh.dim > 2:
            shrink_par = settings.ValueParameter(name="Shrink", label="Shrink",
                                                 default_value=1.0, min_value = 0.0, max_value = 1.0,
                                                 step = 0.1)
            color_par = settings.ColorParameter(name="MaterialColors", values=self.mesh.GetMaterials())
            color_par.changed.connect(lambda : self.tex_mat_colors.store(color_par.getValue(),
                                                                         data_format=GL_UNSIGNED_BYTE))
            self.addParameters("Show",
                               settings.CheckboxParameterCluster(name="ShowElements",
                                                                 label="Volume Elements",
                                                                 default_value = self._initial_values["ShowElements"],
                                                                 sub_parameters=[color_par,
                                                                                shrink_par],
                                                                 updateWidgets=True),
                               settings.CheckboxParameter(name="ShowEdges", label="Edges",
                                                          default_value=self._initial_values["ShowEdges"]))
        if self.mesh.dim == 1:
            edge_names = self.mesh.GetMaterials()
        elif self.mesh.dim == 2:
            edge_names = self.mesh.GetBoundaries()
        else:
            edge_names = self.mesh.GetBBoundaries()
        edge_color = settings.ColorParameter(name="EdgeColors", default_value=(0,0,0,255),
                                             values = edge_names)
        edge_color.changed.connect(lambda : self.tex_bbnd_colors.store(edge_color.getValue(),
                                                                       data_format=GL_UNSIGNED_BYTE))
        self.addParameters("Show",
                           settings.CheckboxParameterCluster(name="ShowEdgeElements", label="Edge Elements",
                                                             default_value=self._initial_values["ShowEdgeElements"],
                                                             sub_parameters = [edge_color],
                                                             updateWidgets=True),
                           settings.CheckboxParameter(name="ShowPeriodicVertices",
                                                      label="Periodic Identification",
                                                      default_value=self._initial_values["ShowPeriodicVertices"]))
        self.addParameters("Numbers",
                           settings.CheckboxParameter(name="ShowPointNumbers",
                                                      label="Points",
                                                      default_value=self._initial_values["ShowPointNumbers"]),
                           settings.CheckboxParameter(name="ShowEdgeNumbers",
                                                      label="Edges",
                                                      default_value=self._initial_values["ShowEdgeNumbers"]))
        if self.mesh.dim > 2:
            self.addParameters("Numbers",
                               settings.CheckboxParameter(name="ShowElementNumbers",
                                                          label="Elements",
                                                          default_value=self._initial_values["ShowElementNumbers"]))
        self.addParameters("",
                           settings.ValueParameter(name="GeomSubdivision", label="Subdivision",
                                                   default_value=5, min_value=1, max_value=20))
                                 
    @inmain_decorator(True)
    def _createOptions(self):
        super()._createOptions()

    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state,)

    def __setstate__(self, state):
        self.tex_mat_colors = self.tex_bc_colors = self.tex_bbnd_colors = None
        super().__setstate__(state[0])

    def initGL(self):
        super().initGL()

        self.vao = VertexArray()
        self.tex_mat_colors = Texture(GL_TEXTURE_1D, GL_RGBA)
        if self.mesh.dim > 2:
            self.tex_mat_colors.store(self.getMaterialColors(), data_format=GL_UNSIGNED_BYTE)
        self.tex_bbnd_colors = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.tex_bbnd_colors.store(self.getEdgeColors(), data_format=GL_UNSIGNED_BYTE)
        self.tex_bc_colors = Texture(GL_TEXTURE_1D, GL_RGBA)
        if self.mesh.dim > 1:
            self.tex_bc_colors.store(self.getSurfaceColors(), data_format=GL_UNSIGNED_BYTE)

        self.text_renderer = TextRenderer()

    def renderEdges(self, settings):
        self.vao.bind()
        prog = getProgram('filter_elements.vert', 'lines.tesc', 'lines.tese', 'mesh.frag')
        model,view,projection = settings.model, settings.view, settings.projection
        uniforms = prog.uniforms
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
            self.tex_bbnd_colors.bind()
        elif self.mesh.dim == 2:
            self.tex_bc_colors.bind()
        else: # dim == 1
            self.tex_mat_colors.bind()
        uniforms.set('colors', 3)

        uniforms.set('do_clipping', False);

        uniforms.set('mesh.dim', 1);
        uniforms.set('light_ambient', 1.0)
        uniforms.set('light_diffuse', 0.0)
        uniforms.set('TessLevel', self.getGeomSubdivision())
        uniforms.set('wireframe', True)
        if self.mesh.dim > 2 and self.getShowEdges():
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
        prog = getProgram('filter_elements.vert', 'tess.tesc', 'tess.tese', 'mesh.geom', 'mesh.frag')
        self.vao.bind()
        model, view, projection = settings.model, settings.view, settings.projection
        uniforms = prog.uniforms
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
        if self.mesh.dim > 2:
            uniforms.set('shrink_elements', self.getShrink())
        uniforms.set('clip_whole_elements', False)
        glActiveTexture(GL_TEXTURE3)
        if self.mesh.dim == 3:
            self.tex_bc_colors.bind()
        elif self.mesh.dim == 2:
            self.tex_mat_colors.bind()
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

        if self.mesh.dim > 2 and self.getShowElements():
            uniforms.set('clip_whole_elements', True)
            uniforms.set('do_clipping', False);
            uniforms.set('light_ambient', 0.3)
            uniforms.set('light_diffuse', 0.7)
            uniforms.set('TessLevel', self.getGeomSubdivision())
            uniforms.set('wireframe', False)
            uniforms.set('mesh.dim', 3);
            glActiveTexture(GL_TEXTURE3)
            self.tex_mat_colors.bind()
            uniforms.set('colors', 3)
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )
            glPatchParameteri(GL_PATCH_VERTICES, 1)
            glDrawArrays(GL_PATCHES, 0, self.mesh.ne)
        self.vao.unbind()

    def renderNumbers(self, settings):
        prog = getProgram('filter_elements.vert', 'numbers.geom', 'font.frag')
        self.vao.bind()
        model, view, projection = settings.model, settings.view, settings.projection
        uniforms = prog.uniforms
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

        if self.mesh.dim > 2 and self.getShowElementNumbers():
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

    def render(self, settings):
        if not self.active:
            return
        self.renderEdges(settings)
        self.renderSurface(settings)
        self.renderNumbers(settings)

    @inmain_decorator(True)
    def _createQtWidget(self):
        super()._createQtWidget()

GUI.sceneCreators.append((ngsolve.Mesh, MeshScene))

class SolutionScene(BaseMeshScene):
    _complex_eval_funcs = {"real" : 0,
                           "imag" : 1,
                           "abs" : 2,
                           "arg" : 3}
    @inmain_decorator(wait_for_return=True)
    def __init__(self, cf, mesh, name=None, min=0,max=1, autoscale=True, linear=False, clippingPlane=True,
                 order=3,gradient=None, *args, **kwargs):
        self.cf = cf
        self.vao = None
        self._initial_values = {"Order" : order,
                                "ColorMapMin" : min,
                                "ColorMapMax" : max,
                                "Autoscale" : autoscale,
                                "ColorMapLinear" : linear,
                                "ShowClippingPlane" : clippingPlane,
                                "Subdivision" : kwargs['sd'] if "sd" in kwargs else 1,
                                "ShowIsoSurface" : False,
                                "ShowVectors" : False,
                                "ShowSurface" : True}


        if gradient and cf.dim == 1:
            self.cf = ngsolve.CoefficientFunction((cf, gradient))
            self.have_gradient = True
        else:
            self.have_gradient = False
        super().__init__(mesh,*args, name=name, **kwargs)


    @inmain_decorator(True)
    def _createParameters(self):
        super()._createParameters()
        self.addParameters("Subdivision",
                           settings.ValueParameter(name="Subdivision",
                                                   default_value=int(self._initial_values["Subdivision"]),
                                                   min_value = 0,
                                                   updateScene=True),
                           settings.ValueParameter(name="Order",
                                                   default_value=int(self._initial_values["Order"]),
                                                   min_value=1,
                                                   max_value=4,
                                                   updateScene=True))
        if self.mesh.dim>1:
            self.addParameters("Show",
                               settings.CheckboxParameter(name="ShowSurface",
                                                          label="Solution on Surface",
                                                          default_value=self._initial_values["ShowSurface"]))

        if self.mesh.dim > 2:
            self.addParameters("Show",
                               settings.CheckboxParameter(name="ShowClippingPlane",
                                                          label="Solution in clipping plane",
                                                          default_value=self._initial_values["ShowClippingPlane"]),
                               settings.CheckboxParameter(name="ShowIsoSurface",
                                                          label="Isosurface",
                                                          default_value = self._initial_values["ShowIsoSurface"]))

        if self.cf.dim > 1:
            self.addParameters("Show",
                               settings.ValueParameter(name="Component", label="Component",
                                                       default_value=0,
                                                       min_value=0,
                                                       max_value=self.cf.dim-1),
                               settings.CheckboxParameter(name="ShowVectors", label="Vectors",
                                                          default_value=self._initial_values["ShowVectors"]))

        if self.cf.is_complex:
            self.addParameters("Complex",
                               settings.SingleOptionParameter(name="ComplexEvalFunc",
                                                              values = list(self._complex_eval_funcs.keys()),
                                                              label="Func",
                                                              default_value = "real"),
                               settings.ValueParameter(name="ComplexPhaseShift",
                                                      label="Value shift angle",
                                                       default_value = 0.0))

    @inmain_decorator(True)
    def _createOptions(self):
        super()._createOptions()
        boxmin = self.addOption( "Colormap", "ColorMapMin", label="Min", typ=float, step=1)
        boxmax = self.addOption( "Colormap", "ColorMapMax", label="Max" ,typ=float, step=1)
        autoscale = self.addOption( "Colormap", "Autoscale",typ=bool)
        self.addOption( "Colormap", "ColorMapLinear", label="Linear",typ=bool)

        boxmin._value_widget.changed.connect(lambda val: autoscale._value_widget.stateChanged.emit(False))
        boxmax._value_widget.changed.connect(lambda val: autoscale._value_widget.stateChanged.emit(False))


    def __getstate__(self):
        super_state = super().__getstate__()
        return (super_state,self.cf)

    def __setstate__(self,state):
        self.cf = state[1]
        super().__setstate__(state[0])
        self.vao = None

    def initGL(self):
        super().initGL()

        formats = [None, GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F];
        self.volume_values = Texture(GL_TEXTURE_BUFFER, formats[self.cf.dim])
        self.volume_values_imag = Texture(GL_TEXTURE_BUFFER, formats[self.cf.dim])
        self.surface_values = Texture(GL_TEXTURE_BUFFER, formats[self.cf.dim])
        self.surface_values_imag = Texture(GL_TEXTURE_BUFFER, formats[self.cf.dim])

        self.filter_buffer = ArrayBuffer()

        self._have_filter = False

        # 1d solution (line)
        self.line_vao = VertexArray()

        # solution on surface mesh
        self.surface_vao = VertexArray()

        # solution on clipping plane
        self.clipping_vao = VertexArray()

        # iso-surface
        self.iso_surface_vao = VertexArray()

        # vectors (currently one per element)
        self.vector_vao = VertexArray()

    def _getValues(self, vb, setMinMax=True):
        cf = self.cf
        with ngsolve.TaskManager():
            try:
                values = ngui.GetValues(cf, self.mesh, vb, 2**self.getSubdivision()-1, self.getOrder())
            except RuntimeError as e:
                assert("Local Heap" in str(e))
                self.setSubdivision(self.getSubdivision()-1)
                print("Localheap overflow, cannot increase subdivision!")
                return

        if setMinMax:
            self.min_values = values["min"]
            self.max_values = values["max"]
        return values

    @inmain_decorator(True)
    def update(self):
        super().update()
        self._have_filter = False
        self.filter_buffer.bind()
        glBufferData(GL_ARRAY_BUFFER, 1000000, ctypes.c_void_p(), GL_STATIC_DRAW)
        if self.mesh.dim==1:
            try:
                values = self._getValues(ngsolve.VOL)
                if values is None:
                    return
                self.surface_values.store(values["real"])
                if self.cf.is_complex:
                    self.surface_values_imag.store(values["imag"])
            except Exception as e:
                print("Cannot evaluate given function on 1d elements"+e)
        if self.mesh.dim==2:
            try:
                values = self._getValues(ngsolve.VOL)
                if values is None:
                    return
                self.surface_values.store(values["real"])
                if self.cf.is_complex:
                    self.surface_values_imag.store(values["imag"])
            except Exception as e:
                raise e
                print("Cannot evaluate given function on surface elements: "+str(e))
                self.show_surface = False

        if self.mesh.dim==3:
            values = self._getValues(ngsolve.VOL)
            if values is None:
                return
            self.volume_values.store(values["real"])
            if self.cf.is_complex:
                self.volume_values_imag.store(values["imag"])

            try:
                values = self._getValues(ngsolve.BND, False)
                if values is None:
                    return
                self.surface_values.store(values["real"])
                if self.cf.is_complex:
                    self.surface_values_imag.store(values["imag"])
            except Exception as e:
                print("Cannot evaluate given function on surface elements"+str(e))
                self.show_surface = False

    def _filterElements(self, settings, filter_type):
        glEnable(GL_RASTERIZER_DISCARD)
        prog = getProgram('filter_elements.vert', 'filter_elements.geom', feedback=['element'])
        uniforms = prog.uniforms
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

        self.filter_feedback = glGenTransformFeedbacks(1)
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
        prog = getProgram('solution1d.vert', 'solution1d.frag')

        uniforms = prog.uniforms
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
        prog = getProgram('filter_elements.vert', 'tess.tesc', 'tess.tese', 'solution.geom', 'solution.frag', ORDER=self.getOrder())
        self.surface_vao.bind()

        uniforms = prog.uniforms
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

            uniforms.set('complex_vis_function', self._complex_eval_funcs[self.getComplexEvalFunc()])
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
        prog = getProgram('mesh.vert', 'isosurface.geom', 'solution.frag', ORDER=self.getOrder())
        self.iso_surface_vao.bind()

        uniforms = prog.uniforms
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
        prog.attributes.bind('element', self.filter_buffer)
        glDrawTransformFeedbackInstanced(GL_POINTS, self.filter_feedback, instances)
        self.iso_surface_vao.unbind()

    def renderVectors(self, settings):
        model, view, projection = settings.model, settings.view, settings.projection
        prog = getProgram('mesh.vert', 'vector.geom', 'solution.frag', ORDER=self.getOrder())
        self.vector_vao.bind()

        uniforms = prog.uniforms
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
        prog = getProgram('mesh.vert', 'clipping.geom', 'solution.frag', ORDER=self.getOrder())
        self.clipping_vao.bind()

        uniforms = prog.uniforms
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
        prog.attributes.bind('element', self.filter_buffer)
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

def _createCFScene(cf, mesh, *args, **kwargs):
    return SolutionScene(cf, mesh, *args,
                         autoscale = kwargs["autoscale"] if "autoscale" in kwargs else not ("min" in kwargs or "max" in kwargs),
                         **kwargs)

def _createGFScene(gf, mesh=None, name=None, *args, **kwargs):
    if not mesh:
        mesh = gf.space.mesh
    if not name:
        name = gf.name
    return _createCFScene(gf,mesh,*args, name=name, **kwargs)

GUI.sceneCreators.append((ngsolve.GridFunction, _createGFScene))
GUI.sceneCreators.append((ngsolve.CoefficientFunction, _createCFScene))

class GeometryScene(BaseScene):
    @inmain_decorator(wait_for_return=True)
    def __init__(self, geo, *args, **kwargs):
        super().__init__(*args,**kwargs)
        self.geo = geo

    def initGL(self):
        super().initGL()
        self.tex_colors = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.vao = VertexArray()

    @inmain_decorator(True)
    def update(self):
        super().update()
        self.geo_data = self.getOpenGLData(self.geo)
        self.surf_colors = { name : [0,0,255,255] for name in set(self.geo_data.surfnames)}
        self.tex_colors.store([self.surf_colors[name][i] for name in self.geo_data.surfnames for i in range(4)],
                          data_format=GL_UNSIGNED_BYTE)

    def updateColors(self):
        self.tex_colors.store(sum(([color.red(), color.green(), color.blue(), color.alpha()] for color in self.colorpicker.getColors()),[]),data_format=GL_UNSIGNED_BYTE)

    @inmain_decorator(True)
    def _createQtWidget(self):
        super()._createQtWidget()
        self.colorpicker = wid.CollColors(self.surf_colors.keys(), initial_color = (0,0,255,255))
        self.colorpicker.colors_changed.connect(self.updateColors)
        self.colorpicker.colors_changed.connect(self._updateGL)
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
        prog = getProgram('geo.vert', 'mesh.frag')
        self.vao.bind()
        model, view, projection = settings.model, settings.view, settings.projection
        uniforms = prog.uniforms
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
        self.tex_colors.bind()
        uniforms.set('colors',3)

        uniforms.set('wireframe',False)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', True)
        uniforms.set('light_ambient', 0.3)
        uniforms.set('light_diffuse', 0.7)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL )
        glDrawArrays(GL_TRIANGLES, 0, self.geo_data.npoints)
        self.vao.unbind()

GUI.sceneCreators.append((netgen.meshing.NetgenGeometry,GeometryScene))
