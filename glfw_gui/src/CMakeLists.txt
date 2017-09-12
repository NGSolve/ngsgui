add_ngsolve_python_module(ngui gui.cpp shader.cpp)
target_link_libraries(ngui ngvis_deps ngcomp glfw)
add_custom_target(run COMMAND python3 ${NGVIS_SRC_DIR}/examples/poisson.py 
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(debug COMMAND gdb --args python3 ${NGVIS_SRC_DIR}/examples/poisson.py 
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
