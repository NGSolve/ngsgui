add_ngsolve_python_module(ngui gui.cpp)
target_link_libraries(ngui ngcomp)

install(TARGETS ngui DESTINATION ${NGSOLVE_INSTALL_DIR_PYTHON}/ngsolve/gui)

add_subdirectory(icons)

configure_file(config.py.in config.py @ONLY)

install(FILES __init__.py gl.py gui.py ${CMAKE_CURRENT_BINARY_DIR}/config.py plugin.py
  settings.py widgets.py glmath.py gl_interface.py glwindow.py
  scenes.py thread.py console.py menu.py DESTINATION ${NGSOLVE_INSTALL_DIR_PYTHON}/ngsolve/gui)

install(PROGRAMS ngsolve DESTINATION ${NGSOLVE_INSTALL_DIR_BIN})
add_subdirectory(shader)
add_subdirectory(code_editor)
