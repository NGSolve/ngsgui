#! /usr/bin/python3

from skbuild import setup
from skbuild.command.install import install
import subprocess, pathlib, os
import pip._internal as pip

icons = [ "src/icons/" + filename for filename in os.listdir("src/icons")]
shaders = [ "src/shader/" + filename for filename in os.listdir("src/shader")]

# dirty hack to get pyside dependency cause it's not in pip... maybe we can get this cleaner?
# or we wait till it's in pip?
class installWithPySide(install):
    def run(self):
        try:
            import PySide2
        except ModuleNotFoundError:
            pip.main(["install", "--user",
                      "--index-url=http://download.qt.io/snapshots/ci/pyside/5.11/latest/",
                      "pyside2", "--trusted-host", "download.qt.io"])
        super().run()

setup(name="ngsgui",
      version="0.1.1",
      description="New graphical interface for NGSolve",
      packages=['ngsgui', 'ngsgui.code_editor'],
      package_dir={'ngsgui' : 'src',
                   'ngsgui.code_editor' : 'src/code_editor'},
      data_files=[('ngsgui/icons', icons),
                  ('ngsgui/shader', shaders)],
      cmake_args=['-DUSE_OCC=ON', '-DUSE_CCACHE=ON'],
      classifiers=("Programming Language :: Python :: 3",
                   "Operating System :: OS Independent",
                   "Development Status :: 2 - Pre-Alpha",
                   "Environment :: X11 Applications :: Qt",
                   "License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)"),
      install_requires=["scikit-build", "PyOpenGL", "psutil", "qtconsole", "numpy",
                        "matplotlib"],
      entry_points={ "gui_scripts" : "ngsolve = ngsgui.start:main" },
      cmdclass = {"install" : installWithPySide})

