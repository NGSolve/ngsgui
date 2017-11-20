add_ngsolve_python_module(ngui gui.cpp shader.cpp)
install(TARGETS ngui DESTINATION .)
target_link_libraries(ngui ngvis_deps ngcomp GLEW)
