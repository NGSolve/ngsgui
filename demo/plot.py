
from ngsgui.gui import gui
import numpy as np
from matplotlib.figure import Figure

x = np.linspace(0,2*np.pi, 10000)

fig = Figure(figsize=(5,3))
axes = fig.subplots()
axes.plot(x, [np.sin(xi) for xi in x])

gui.plot(fig, label="sin(x)")
