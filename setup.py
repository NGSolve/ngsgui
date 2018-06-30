#! /usr/bin/python3

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from setuptools.dist import Distribution
import subprocess, pathlib, os


class CMakeExtension(Extension):
    def __init__(self, name):
        # don't invoke the original build_ext for this special extension
        super().__init__(name, sources=[])

class cmakeBuild_ext(build_ext):
    def run(self):
        for extension in self.extensions:
            self.build_cmake(extension)
        super().run()

    def build_cmake(self, ext):
        cwd = pathlib.Path().absolute()
        # these dirs will be created in build_py, so if you don't have
        # any python sources to bundle, the dirs will be missing
        build_temp = pathlib.Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)
        extdir = pathlib.Path(self.get_ext_fullpath(ext.name))
        extdir.mkdir(parents=True, exist_ok=True)

        # example of cmake args
        config = 'Debug' if self.debug else 'Release'
        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + str(extdir.parent.absolute()),
            '-DCMAKE_BUILD_TYPE=' + config,
            '-DUSE_OCC=ON'
        ]

        # example of build args
        build_args = [
            '--config', config
        ]

        os.chdir(str(build_temp))
        self.spawn(['cmake', str(cwd)] + cmake_args)
        if not self.dry_run:
            self.spawn(['cmake', '--build', '.'] + build_args)
        os.chdir(str(cwd))

icons = [ "src/icons/" + filename for filename in os.listdir("src/icons")]
shaders = [ "src/shader/" + filename for filename in os.listdir("src/shader")]

setup(name="ngsgui",
      version="0.1",
      description="New graphical interface for NGSolve",
      packages=['ngsgui', 'ngsgui.code_editor'],
      package_dir={'ngsgui' : 'src',
                   'ngsgui.code_editor' : 'src/code_editor'},
      data_files=[('ngsgui/icons', icons),
                  ('ngsgui/shader', shaders)],
      ext_modules=[CMakeExtension(os.path.join('ngsgui','ngsgui'))],
      cmdclass = {"build_ext" : cmakeBuild_ext},
      entry_points={
          "gui_scripts" : "ngsolve = ngsgui.start:main"})

