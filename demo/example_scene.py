import ngsgui
from ngsgui.thread import inmain_decorator, inthread
import ngsgui.gui
import ngsgui.scenes
import ngsgui.settings as settings
import ngsolve

class GenerateSphereScene(ngsgui.scenes.BaseScene):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)


    @inmain_decorator(True)
    def _generateMesh(self, *args):
        from netgen.csg import CSGeometry, Sphere, Pnt
        from ngsolve import Mesh
        r = self.getRadius()
        geometry = CSGeometry()
        sphere = Sphere(Pnt(0,0,0),r).bc("sphere")
        geometry.Add (sphere)
        self.geometry = geometry
        self.mesh = Mesh(geometry.GenerateMesh(maxh=r/5))
        ngsolve.Draw(self.mesh)

    
    @inmain_decorator(True)
    def _createParameters(self):
        super()._createParameters()
        self.addParameters("Radius", settings.ValueParameter(name="Radius", default_value=1.0))
        genmesh = settings.Button(name="GenerateMesh", label="Generate mesh")
        genmesh.changed.connect(self._generateMesh)
        self.addParameters("GenerateMesh", genmesh)


s = GenerateSphereScene()

ngsolve.Draw(s)
