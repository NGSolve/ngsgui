from .thread import inmain_decorator
from PySide2 import QtWidgets

class MenuWithDict(QtWidgets.QMenu):
    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self._dict = {}

    @inmain_decorator(wait_for_return=True)
    def addMenu(self, name, *args, **kwargs):
        menu = MenuWithDict(title=name, parent=self, *args, **kwargs)
        super().addMenu(menu)
        self._dict[name] = menu
        return menu

    def __getitem__(self, index):
        if index not in self._dict:
            return self.addMenu(index)
        return self._dict[index]


class MenuBarWithDict(QtWidgets.QMenuBar):
    def __init__(self,*args, **kwargs):
        super().__init__(*args,**kwargs)
        self._dict = {}

    @inmain_decorator(wait_for_return=True)
    def addMenu(self, name, *args, **kwargs):
        menu = MenuWithDict(title=name, parent=self, *args, **kwargs)
        super().addMenu(menu)
        self._dict[name] = menu
        return menu

    def __getitem__(self, index):
        if index not in self._dict:
            return self.addMenu(index)
        return self._dict[index]
