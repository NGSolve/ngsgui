add_ngsolve_python_module(ngui gui.cpp)
install(TARGETS ngui DESTINATION .)
target_link_libraries(ngui ngcomp)

install(TARGETS ngui DESTINATION ngsolve/gui)

add_subdirectory(icons)

configure_file(config.py.in config.py @ONLY)

install(FILES __init__.py gl.py gui.py ${CMAKE_CURRENT_BINARY_DIR}/config.py plugin.py
  settings.py widgets.py glmath.py gl_interface.py glwindow.py
  scenes.py thread.py console.py menu.py plot.py DESTINATION ngsolve/gui)

install(PROGRAMS ngsolve DESTINATION ../../../bin)
add_subdirectory(shader)
add_subdirectory(code_editor)
