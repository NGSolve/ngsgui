
from ngsgui import gui
import numpy as np
x = np.linspace(0,2*np.pi, 10000)
gui.plot(x, [np.sin(xi) for xi in x])
