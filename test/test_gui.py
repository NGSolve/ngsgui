
from unittest.mock import Mock
import ngsgui.gui as G
import pytest, os

@pytest.fixture
def gui(qtbot):
    G.gui = gui = G.GUI(startApplication=False, flags=[])
    return gui

def test_loadPython(gui, qtbot):
    with open("small.py", "w") as f:
        f.write("a = 5")
    gui.loadPythonFile("small.py")
    qtbot.wait(1000)
    gui.callback_called = False
    def callback(msg):
        gui.callback_called = True
        assert msg['data']['text/plain'] == '5'
    gui.console._silent_exec_callback("a", callback)
    qtbot.wait(1000)
    assert gui.callback_called
    os.remove("small.py")

    
