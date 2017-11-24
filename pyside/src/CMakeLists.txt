add_ngsolve_python_module(ngui gui.cpp)
install(TARGETS ngui DESTINATION .)
target_link_libraries(ngui ngvis_deps ngcomp GLEW)

install(TARGETS ngui DESTINATION ngsolve/gui)
install(FILES __init__.py gl.py gui.py shader.py glmath.py DESTINATION ngsolve/gui)
