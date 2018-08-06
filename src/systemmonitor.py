
import sys, psutil
from matplotlib.backends.backend_qt5agg import (FigureCanvas, NavigationToolbar2QT as NavigationToolbar)
from PySide2 import QtWidgets
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
        self.historyMem = np.zeros(100)
        self.figureMem = Figure()
        self.axMem = self.figureMem.add_subplot(111)
        self.axMem.set_xlim(-5,105)
        self.axMem.set_ylim(-5,105)
        self.axMem.get_xaxis().set_visible(False)
        self.axMem.get_yaxis().set_visible(False)
        self.lineMem, = self.axMem.plot(self.time, self.historyMem)
        canvasMem = FigureCanvas(self.figureMem)
        self.cpu_label = QtWidgets.QLabel("0%")
        self.mem_label = QtWidgets.QLabel("0%")
        self.setLayout(wid.ArrangeH(QtWidgets.QLabel("CPU:"),self.cpu_label,canvas,
                                    QtWidgets.QLabel("Memory:"), self.mem_label, canvasMem))

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
