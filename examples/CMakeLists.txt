find_package(NGSolve CONFIG REQUIRED HINTS /opt/netgen)
add_executable(main main.cpp)
target_link_libraries(main ngvis_deps ngfem glfw)
