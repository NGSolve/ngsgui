
import sys, psutil, os
os.environ["MPLBACKEND"] = "Qt5Agg"
from matplotlib.backends.backend_qt5agg import (FigureCanvas, NavigationToolbar2QT as NavigationToolbar)
from qtpy import QtWidgets, QtCore
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
import numpy as np
from . import widgets as wid

class SystemMonitor(QtWidgets.QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.history = np.zeros(100)
        self.figure = Figure()
        self.ax = self.figure.add_subplot(111)
        self.ax.set_xlim(0,100)
        self.ax.set_ylim(0,100)
        self.ax.get_xaxis().set_visible(False)
        self.ax.get_yaxis().set_visible(False)
        self.time = np.array([i+1 for i in range(100)])
        self.line, = self.ax.plot(self.time, self.history)
        canvas = FigureCanvas(self.figure)
        canvas.setFixedWidth(100)
        self.historyMem = np.zeros(100)
        self.figureMem = Figure()
        self.axMem = self.figureMem.add_subplot(111)
        self.axMem.set_xlim(-5,105)
        self.axMem.set_ylim(-5,105)
        self.axMem.get_xaxis().set_visible(False)
        self.axMem.get_yaxis().set_visible(False)
        self.lineMem, = self.axMem.plot(self.time, self.historyMem)
        canvasMem = FigureCanvas(self.figureMem)
        canvasMem.setFixedWidth(100)
        self.cpu_label = QtWidgets.QLabel("0%")
        self.cpu_label.setFixedWidth(30)
        self.mem_label = QtWidgets.QLabel("0%")
        self.mem_label.setFixedWidth(30)
        self.progress_bar = QtWidgets.QProgressBar()
        self.progress_bar.setFixedWidth(130)
        self.progress_bar_label = QtWidgets.QLabel("idle")
        cpulbl = QtWidgets.QLabel("CPU:")
        cpulbl.setFixedWidth(25)
        memlbl = QtWidgets.QLabel("Memory:")
        memlbl.setFixedWidth(45)
        self.setLayout(wid.ArrangeH(cpulbl,self.cpu_label,canvas,
                                    memlbl, self.mem_label, canvasMem,
                                    self.progress_bar, self.progress_bar_label))


    def start(self):
        self._cpuTimer = QtCore.QTimer()
        self._cpuTimer.setInterval(100)
        self._cpuTimer.timeout.connect(self.update)
        self._cpuTimer.start()

    def update(self):
        cpuPercent = psutil.cpu_percent()
        mem = psutil.virtual_memory()
        memPercent = mem.used/mem.total * 100
        self.history[:-1] = self.history[1:]
        self.history[-1] = cpuPercent
        self.line.set_ydata(self.history)
        self.figure.canvas.draw()
        self.figure.canvas.flush_events()
        self.historyMem[:-1] = self.historyMem[1:]
        self.historyMem[-1] = memPercent
        self.lineMem.set_ydata(self.historyMem)
        self.figureMem.canvas.draw()
        self.figureMem.canvas.flush_events()
        cpu_text = str(int(cpuPercent))+"%"
        mem_text = str(int(memPercent))+"%"
        self.cpu_label.setText(cpu_text)
        self.mem_label.setText(mem_text)
        import netgen.libngpy._meshing as msh
        status, percent = msh._GetStatus()
        self.progress_bar_label.setText(status)
        self.progress_bar.setValue(percent)

class MemoryUsageProfiler(QtWidgets.QWidget):
    """Prints a memory profile from variables that are defined in the console"""
    def __init__(self, console, *args, **kwargs):
        super().__init__(*args, **kwargs)
        from matplotlib.backends.backend_qt5agg import FigureCanvas
        from matplotlib.figure import Figure
        import psutil, os, ngsolve
        process = psutil.Process(os.getpid())
        all_dir = []
        console.pushVariables({"_getAll" : all_dir})
        console.execute("_getAll.append({val : globals()[val] for val in dir()})", hidden=True)
        known_objects = { "BilinearForm" : ngsolve.BilinearForm,
                          "FESpaces" : ngsolve.FESpace,
                          "GridFunctions" : ngsolve.GridFunction,
                          "LinearForms" : ngsolve.LinearForm }
        mem = { key : 0 for key in known_objects }
        for name, item in all_dir[0].items():
            for namecls, cls in known_objects.items():
                if isinstance(item, cls):
                    for string, memory, blocks in item.__memory__:
                        mem[namecls] += memory
        for name in mem:
            mem[name] /= 1024 *1024
        figure = Figure()
        axes = figure.subplots()
        patches, texts = axes.pie(mem.values())
        labels = [key + " {:.2f}".format(val) + " MB" for key,val in mem.items()]
        patches, labels, dummy = zip(*sorted(zip(patches, labels, mem.values()), key=lambda x: x[2], reverse=True))
        axes.legend(patches, labels, loc="center left")
        canvas = FigureCanvas(figure)
        self.setLayout(wid.ArrangeH(canvas))

    def isGLWindow(self):
        return False

