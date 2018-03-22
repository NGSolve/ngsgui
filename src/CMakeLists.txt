add_ngsolve_python_module(ngui gui.cpp)
install(TARGETS ngui DESTINATION .)
target_link_libraries(ngui ngcomp)

install(TARGETS ngui DESTINATION ngsolve/gui)
install(FILES __init__.py gl.py gui.py glwindow.py plugin.py settings.py widgets.py glmath.py scenes.py thread.py DESTINATION ngsolve/gui)
install(PROGRAMS ngsolve DESTINATION ../../../bin)
add_subdirectory(shader)
add_subdirectory(code_editor)
