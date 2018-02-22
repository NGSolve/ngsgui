
class GuiPlugin():
    @staticmethod
    def loadPlugin(gui):
        """Load the plugin into the gui, this function must be overridden by derived Plugins"""
        raise NotImplementedError("loadPlugin not implemented for Plugin!")

