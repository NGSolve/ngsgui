#! /usr/bin/python3

from setuptools import find_packages, setup
import os

icons = [ filename for filename in os.listdir("src/icons")]
shaders = [ filename for filename in os.listdir("src/shader")]

modules = ['ngsgui'] + ['ngsgui.' + pkg for pkg in find_packages('src')]
dirs = { module : module.replace('ngsgui.','src/') if 'ngsgui.' in module else 'src' for module in modules
         }
modules += ['spyder_ngsgui']
dirs['spyder_ngsgui'] = 'src/spyder_ngsgui'

setup(name="ngsgui",
      version="0.1.10",
      description="New graphical interface for NGSolve",
      packages=modules,
      package_dir=dirs,
      package_data = {'ngsgui.shader': shaders,
                      'ngsgui.icons' : icons},
      include_package_data=True,
      classifiers=("Programming Language :: Python :: 3",
                   "Operating System :: OS Independent",
                   "Development Status :: 2 - Pre-Alpha",
                   "Environment :: X11 Applications :: Qt",
                   "License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)"),
      install_requires=["PyOpenGL", "psutil", "qtconsole>=4.4.0", "numpy",
                        "matplotlib>=2.2.3", "qtpy>=1.5.2"],
      entry_points={ "console_scripts" : "ngsolve = ngsgui.__main__:main" })

