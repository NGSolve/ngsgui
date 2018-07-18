#! /usr/bin/python3
try:
    import PySide2
except ImportError as e:
    print("")
    print("***********************************************************************************")
    print("")
    print("PySide2 not found, please execute:")
    print("pip3 install --index-url=http://download.qt.io/snapshots/ci/pyside/5.11/latest/ pyside2 --trusted-host download.qt.io")
    print("")
    print("***********************************************************************************")
    print("")
    raise e

from setuptools import find_packages, setup
import os

icons = [ "icons/" + filename for filename in os.listdir("src/icons")]
shaders = [ "shader/" + filename for filename in os.listdir("src/shader")]

modules = ['ngsgui'] + ['ngsgui.' + pkg for pkg in find_packages('src')]
dirs = { module : module.replace('ngsgui.','src/') if 'ngsgui.' in module else 'src' for module in modules}

setup(name="ngsgui",
      version="0.1.8",
      description="New graphical interface for NGSolve",
      packages=modules,
      package_dir=dirs,
      package_data = {'ngsgui':[],
                      'ngsgui.shader': shaders,
                      'ngsgui.icons':  icons},
      include_package_data=True,
      classifiers=("Programming Language :: Python :: 3",
                   "Operating System :: OS Independent",
                   "Development Status :: 2 - Pre-Alpha",
                   "Environment :: X11 Applications :: Qt",
                   "License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)"),
      install_requires=["PyOpenGL", "psutil", "qtconsole", "numpy",
                        "matplotlib"],
      entry_points={ "gui_scripts" : "ngsolve = ngsgui.start:main" })

