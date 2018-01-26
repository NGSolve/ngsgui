from PySide2 import QtCore, QtGui, QtWidgets, QtOpenGL
from PySide2.QtCore import Qt
from OpenGL.GL import *
from .gui import ColorMapSettings, Qt, RangeGroup, CollColors, ArrangeV, ArrangeH, GUIHelper
import ngsolve
from .gl import *
import numpy
from . import glmath

class CMeshData:
    """Helper class to avoid redundant copies of the same mesh on the GPU."""

    def __init__(self, mesh):
        import weakref
        from . import ngui
        self.mesh = weakref.ref(mesh)
        self.ntrigs, trig_coordinates_data, trig_bary_coordinates_data, trig_element_number_data, trig_element_index_data, self.trig_max_index, self.min, self.max = ngui.GetFaceData(mesh)
        self.ntets, self.tet_max_index, tet_coordinates_data, tet_bary_coordinates_data, tet_element_number_data, tet_element_index_data, tet_element_coordinates_data = ngui.GetTetData(mesh)

        self.tet_bary_coordinates = ArrayBuffer()
        self.tet_bary_coordinates.store(tet_bary_coordinates_data)
        self.tet_coordinates = ArrayBuffer()
        self.tet_coordinates.store(tet_coordinates_data)
        self.tet_element_index = ArrayBuffer()
        self.tet_element_index.store(tet_element_index_data)
        self.tet_element_number = ArrayBuffer()
        self.tet_element_number.store(tet_element_number_data)
        self.tet_element_coordinates = ArrayBuffer()
        self.tet_element_coordinates.store(tet_element_coordinates_data)
        self.trig_bary_coordinates = ArrayBuffer()
        self.trig_bary_coordinates.store(trig_bary_coordinates_data)
        self.trig_coordinates = ArrayBuffer()
        self.trig_coordinates.store(trig_coordinates_data)
        self.trig_element_index = ArrayBuffer()
        self.trig_element_index.store(trig_element_index_data)
        self.trig_element_number = ArrayBuffer()
        self.trig_element_number.store(trig_element_number_data)

        mesh._opengl_data = self

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

        shaders = [
            Shader('font.vert'),
            Shader('font.geom'),
            Shader('font.frag')
        ]
        self.program = Program(shaders)
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
            painter.drawText((i-32)*w,0, (i+1-32)*w, font.height, QtCore.Qt.AlignTop | Qt.AlignLeft, text)
        painter.end()
        Z = numpy.array(image.bits()).reshape(font.tex_height, font.tex_width)

        font.tex = Texture(GL_TEXTURE_2D, GL_RED)
        font.tex.store(Z, GL_UNSIGNED_BYTE, Z.shape[1], Z.shape[0] )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST )

        glBindVertexArray(0)

    def draw(self, rendering_params, text, pos, font_size=0, use_absolute_pos=True, alignment=Qt.AlignTop|Qt.AlignLeft):

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

        if alignment&Qt.AlignRight:
            pos[0] -= text_width
        if alignment&Qt.AlignBottom:
            pos[1] += text_height

        if alignment&Qt.AlignCenter:
            pos[0] -= 0.5*text_width
        if alignment&Qt.AlignVCenter:
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
    action_counter = 1
    def __init__(self,active=True):
        self.actions = {}
        self.active_action = None
        self.timestamp = -1
        self.active = active

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

    def setActive(self, active):
        self.active = active

    def getQtWidget(self, updateGL, params):
        widgets = {}

        helper = GUIHelper(updateGL)
        cb = helper.CheckBox("active", self.setActive, self.active)
        widgets["General"] = cb
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
            widgets["Actions"] = widget

        return widgets

    def addAction(self,action,name=None):
        if name is None:
            name = "Action" + str(action_counter)
            action_counter += 1
        self.actions[name] = action
        self.active_action = name

    def doubleClickAction(self,point):
        if self.active_action:
            self.actions[self.active_action](point)

class BaseMeshSceneObject(SceneObject):
    """Base class for all scenes that depend on a mesh"""
    def __init__(self, mesh,**kwargs):
        super().__init__(**kwargs)
        self.mesh = mesh

    def initGL(self):
        self.mesh_data = MeshData(self.mesh)

    def getBoundingBox(self):
        return self.mesh_data.min, self.mesh_data.max

class BaseFunctionSceneObject(BaseMeshSceneObject):
    """Base class for all scenes that depend on a coefficient function and a mesh"""
    def __init__(self, cf, mesh=None, order=3, **kwargs):
        if isinstance(cf, ngsolve.comp.GridFunction):
            self.gf = cf
            mesh = cf.space.mesh
            self.is_gridfunction = True
        else:
            self.is_gridfunction = False
            if mesh==None:
                raise RuntimeError("A mesh is needed if the given function is no GridFunction")
            self.cf = cf
            self.gf = ngsolve.GridFunction(ngsolve.L2(mesh, order=order, all_dofs_together=True))

        super().__init__(mesh,**kwargs)

        self.colormap_min = -1
        self.colormap_max = 1
        self.colormap_linear = False


    def initGL(self):
        super().initGL()
        self.coefficients = Texture(GL_TEXTURE_BUFFER, GL_R32F)

    def update(self):
        self.initGL()
        if not self.is_gridfunction:
            self.gf.Set(self.cf)
        vec = ConvertCoefficients(self.gf)
        self.coefficients.store(vec)


    def setColorMapMin(self, value):
        self.colormap_min = value

    def setColorMapMax(self, value):
        self.colormap_max = value

    def setColorMapLinear(self, value):
        self.colormap_linear = value


    def getQtWidget(self, updateGL, params):

        settings = ColorMapSettings(min=-2, max=2, min_value=self.colormap_min, max_value=self.colormap_max)
        settings.layout().setAlignment(Qt.AlignTop)

        settings.minChanged.connect(self.setColorMapMin)
        settings.minChanged.connect(updateGL)

        settings.maxChanged.connect(self.setColorMapMax)
        settings.maxChanged.connect(updateGL)

        settings.linearChanged.connect(self.setColorMapLinear)
        settings.linearChanged.connect(updateGL)

        widgets = super().getQtWidget(updateGL, params)
        widgets["Colormap"] = settings
        return widgets

class OverlayScene(SceneObject):
    """Class  for overlay objects (Colormap, coordinate system, logo)"""
    def __init__(self,**kwargs):
        super().__init__(**kwargs)
        self.gl_initialized = False
        self.show_logo = True
        self.show_cross = True
        self.cross_scale = 0.3
        self.cross_shift = -0.10

    def deferRendering(self):
        return 99

    def initGL(self):
        if self.gl_initialized:
            return

        self.text_renderer = TextRenderer()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        self.cross_points = ArrayBuffer()
        points = [self.cross_shift + (self.cross_scale if i%7==3 else 0) for i in range(24)]
        self.cross_points.store(numpy.array(points, dtype=numpy.float32))

        vert = Shader(shader_type=GL_VERTEX_SHADER, string="""
#version 150
in vec3 pos;
uniform mat4 MVP;
void main() { gl_Position = MVP*vec4(pos, 1.0); }""")
        frag = Shader(shader_type=GL_FRAGMENT_SHADER, string="""
#version 150
out vec4 color;
void main() { color = vec4(0,0,0,1);}""")
        self.program = Program([vert, frag])

        self.program.attributes.bind('pos', self.cross_points)

        self.gl_initialized = True
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
                self.text_renderer.draw(settings, "xyz"[i], coords[0:3,i], alignment=Qt.AlignCenter|Qt.AlignVCenter)
        if self.show_logo:
            self.text_renderer.draw(settings, "NGSolve " + ngsolve.__version__, [0.99,-0.99,0], font_size=16, alignment=Qt.AlignRight|Qt.AlignBottom)

        glEnable(GL_DEPTH_TEST)
        glBindVertexArray(0)

    def setShowLogo(self, show):
        self.show_logo = show

    def setShowCross(self, show):
        self.show_cross = show

    def update(self):
        self.initGL()

    def getQtWidget(self, updateGL, params):

        widgets = super().getQtWidget(updateGL, params)
        helper = GUIHelper(updateGL)

        logo = helper.CheckBox("Show version number", self.setShowLogo, self.show_logo)
        cross = helper.CheckBox("Show coordinate cross", self.setShowCross, self.show_cross)
        widgets["Overlay"] = ArrangeV(logo, cross)
        clipx = helper.Button("X", lambda : params.setClippingPlaneNormal([1,0,0]))
        clipy = helper.Button("Y", lambda : params.setClippingPlaneNormal([0,1,0]))
        clipz = helper.Button("Z", lambda : params.setClippingPlaneNormal([0,0,1]))
        clip_flip = helper.Button("flip", lambda : params.setClippingPlaneNormal(-1.0*params.getClippingPlaneNormal()))
        widgets["Clipping plane"] = ArrangeH(clipx, clipy, clipz, clip_flip)
        return widgets

    
class ClippingPlaneScene(BaseFunctionSceneObject):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.gl_initialized = False
        self.vao = None

    def initGL(self):
        if self.gl_initialized:
            return

        super().initGL()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        Shader.includes['shader_functions'] = ngsolve.fem.GenerateL2ElementCode(self.gf.space.globalorder)

        shaders = [
            Shader('solution.vert'),
            Shader('clipping.geom'),
            Shader('solution.frag')
        ]
        self.program = Program(shaders)
        glUseProgram(self.program.id)

        attributes = self.program.attributes
        attributes.bind('vPos', self.mesh_data.tet_coordinates)
        attributes.bind('vLam', self.mesh_data.tet_bary_coordinates)
        attributes.bind('vElementNumber', self.mesh_data.tet_element_number)

        self.gl_initialized = True
        glBindVertexArray(0)


    def update(self):
        self.initGL()
        glBindVertexArray(self.vao)
        vec = ConvertCoefficients(self.gf)
        self.coefficients.store(vec)
        glBindVertexArray(0)


    def render(self, settings):
        if not self.active:
            return
        model, view, projection = settings.model, settings.view, settings.projection
        glUseProgram(self.program.id)
        glBindVertexArray(self.vao)

        uniforms = self.program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('colormap_min', self.colormap_min)
        uniforms.set('colormap_max', self.colormap_max)
        uniforms.set('colormap_linear', self.colormap_linear)
        uniforms.set('clipping_plane_deformation', False)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', False);

        if(self.mesh.dim==2):
            uniforms.set('element_type', 10)
        if(self.mesh.dim==3):
            uniforms.set('element_type', 20)

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_LINES_ADJACENCY, 0, 4*self.mesh_data.ntets)
        glBindVertexArray(0)



class MeshScene(BaseMeshSceneObject):
    def __init__(self, mesh, **kwargs):
        super().__init__(mesh, **kwargs)

        self.qtWidget = None
        self.gl_initialized = False
        self.show_wireframe = True
        self.show_surface = True

    def initGL(self):
        if self.gl_initialized:
            return

        super().initGL()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        shaders = [
            Shader('mesh.vert'),
            Shader('mesh.frag')
        ]
        self.program = Program(shaders)

        self.gl_initialized = True
        glBindVertexArray(0)

    def update(self):
        self.initGL()
        glBindVertexArray(self.vao)
        self.index_colors = [0, 255, 0, 255] * (self.mesh_data.trig_max_index+1)

        attributes = self.program.attributes
        attributes.bind('pos', self.mesh_data.trig_coordinates)
        attributes.bind('index', self.mesh_data.trig_element_index)

        self.tex_index_color = glGenTextures(1)

        glBindTexture(GL_TEXTURE_1D, self.tex_index_color)

        # copy texture
        glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, self.mesh_data.trig_max_index+1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes(self.index_colors))

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glBindVertexArray(0)

    def setupRender(self, settings):
        glUseProgram(self.program.id)

        model, view, projection = settings.model, settings.view, settings.projection
        uniforms = self.program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        glBindTexture(GL_TEXTURE_1D, self.tex_index_color)

    def render(self, settings):
        if not self.active:
            return

        if self.show_surface:
            self.renderMesh(settings)

        if self.show_wireframe:
            self.renderWireframe(settings)

    def renderMesh(self, settings):
        glBindVertexArray(self.vao)
        self.setupRender(settings)

        uniforms = self.program.uniforms
        uniforms.set('fColor', [0,1,0,0] )
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('use_index_color', True)
        uniforms.set('do_clipping', self.mesh.dim==3);

        glPolygonOffset (2,2)
        glEnable(GL_POLYGON_OFFSET_FILL)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh_data.ntrigs)
        glDisable(GL_POLYGON_OFFSET_FILL)

    def renderWireframe(self, settings):
        glBindVertexArray(self.vao)
        self.setupRender(settings)

        uniforms = self.program.uniforms
        uniforms.set('fColor', [0,0,0,1] )
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('use_index_color', False)
        uniforms.set('do_clipping', self.mesh.dim==3);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glPolygonOffset (1, 1)
        glEnable(GL_POLYGON_OFFSET_LINE)
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh_data.ntrigs)
        glDisable(GL_POLYGON_OFFSET_LINE)


    def updateIndexColors(self):
        colors = []
        for c in self.bccolors.getColors():
            colors.append(c.red())
            colors.append(c.green())
            colors.append(c.blue())
            colors.append(c.alpha())
        self.index_colors = colors
        glBindTexture(GL_TEXTURE_1D, self.tex_index_color)
        glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, self.mesh_data.trig_max_index+1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes(self.index_colors))

    def setShowWireframe(self, show_wireframe):
        self.show_wireframe = show_wireframe

    def setShowSurface(self, show_surface):
        self.show_surface = show_surface

    def getQtWidget(self, updateGL, params):
        if self.qtWidget!=None:
            return self.qtWidget

        self.bccolors = CollColors(self.mesh.GetBoundaries())
        self.bccolors.colors_changed.connect(self.updateIndexColors)
        self.bccolors.colors_changed.connect(updateGL)
        self.updateIndexColors()

        widgets = super().getQtWidget(updateGL, params)
        widgets["BCColors"] = self.bccolors

        helper = GUIHelper(updateGL)
        cb_mesh = helper.CheckBox("Surface", self.setShowSurface, self.show_surface)
        cb_wireframe = helper.CheckBox("Wireframe", self.setShowWireframe, self.show_wireframe)

        widgets["Components"] = ArrangeV(cb_mesh, cb_wireframe)
        return widgets


class MeshElementsScene(BaseMeshSceneObject):
    def __init__(self, mesh, **kwargs):
        super().__init__(mesh, **kwargs)

        self.qtWidget = None
        self.gl_initialized = False
        self.shrink = 1.0

    def initGL(self):
        if self.gl_initialized:
            return

        super().initGL()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        shaders = [
            Shader('elements.vert'),
            Shader('elements.geom'),
            Shader('elements.frag')
        ]
        self.program = Program(shaders)

        self.gl_initialized = True
        glBindVertexArray(0)

    def update(self):
        self.initGL()
        glBindVertexArray(self.vao)
        self.mat_colors = [0,0,255,255] * (self.mesh_data.tet_max_index+1)

        attributes = self.program.attributes
        attributes.bind('pos', self.mesh_data.tet_coordinates)
        # attributes.bind('bary_pos', self.mesh_data.tet_bary_coordinates)
        attributes.bind('corners', self.mesh_data.tet_element_coordinates)
        attributes.bind('index', self.mesh_data.tet_element_index)

        self.tex_mat_color = Texture(GL_TEXTURE_1D, GL_RGBA)
        self.tex_mat_color.store(self.mat_colors, GL_UNSIGNED_BYTE, self.mesh_data.tet_max_index+1)

        glBindVertexArray(0)

    def render(self, settings):
        if not self.active:
            return
        glBindVertexArray(self.vao)
        glUseProgram(self.program.id)

        model, view, projection = settings.model, settings.view, settings.projection

        uniforms = self.program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)
        uniforms.set('shrink_elements', self.shrink)
        uniforms.set('clipping_plane', settings.clipping_plane)

        self.tex_mat_color.bind()

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_LINES_ADJACENCY, 0, 4*self.mesh_data.ntets)

    def setShrink(self, value):
        self.shrink = value

    def updateMatColors(self):
        colors = []
        for c in self.matcolors.getColors():
            colors.append(c.red())
            colors.append(c.green())
            colors.append(c.blue())
            colors.append(c.alpha())
        self.mat_colors = colors
        self.tex_mat_color.store(self.mat_colors, GL_UNSIGNED_BYTE, self.mesh_data.tet_max_index+1)

    def getQtWidget(self, updateGL, params):
        shrink = RangeGroup("Shrink", min=0.0, max=1.0, value=1.0)
        shrink.valueChanged.connect(self.setShrink)
        shrink.valueChanged.connect(updateGL)
        self.matcolors = CollColors(self.mesh.GetMaterials(),initial_color=(0,0,255,255))
        self.matcolors.colors_changed.connect(self.updateMatColors)
        self.matcolors.colors_changed.connect(updateGL)
        self.updateMatColors()
        widgets = super().getQtWidget(updateGL, params)
        widgets["Shrink"] = shrink
        widgets["MatColors"] = self.matcolors
        return widgets


class SolutionScene(BaseFunctionSceneObject):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.qtWidget = None
        self.vao = None

    def initGL(self):
        if self.vao:
            return

        super().initGL()

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        Shader.includes['shader_functions'] = ngsolve.fem.GenerateL2ElementCode(self.gf.space.globalorder)

        shaders = [
            Shader('solution.vert'),
            Shader('solution.frag')
        ]
        self.program = Program(shaders)


        attributes = self.program.attributes
        attributes.bind('vPos', self.mesh_data.trig_coordinates)
        attributes.bind('vLam', self.mesh_data.trig_bary_coordinates)
        attributes.bind('vElementNumber', self.mesh_data.trig_element_number)

        glBindVertexArray(0)


    def render(self, settings):
        if not self.active:
            return
        model, view, projection = settings.model, settings.view, settings.projection
        glBindVertexArray(self.vao)
        glUseProgram(self.program.id)

        uniforms = self.program.uniforms
        uniforms.set('P',projection)
        uniforms.set('MV',view*model)

        uniforms.set('colormap_min', self.colormap_min)
        uniforms.set('colormap_max', self.colormap_max)
        uniforms.set('colormap_linear', self.colormap_linear)
        uniforms.set('clipping_plane', settings.clipping_plane)
        uniforms.set('do_clipping', self.mesh.dim==3);

        if(self.mesh.dim==2):
            uniforms.set('element_type', 10)
        if(self.mesh.dim==3):
            uniforms.set('element_type', 20)

        glPolygonOffset (2,2)
        glEnable(GL_POLYGON_OFFSET_FILL)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDrawArrays(GL_TRIANGLES, 0, 3*self.mesh_data.ntrigs);

