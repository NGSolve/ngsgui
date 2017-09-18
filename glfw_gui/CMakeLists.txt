cmake_minimum_required(VERSION 3.3)
project(NGVIS)

set(NGVIS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NGVIS_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_library(ngvis_deps INTERFACE)
target_sources(ngvis_deps INTERFACE ${NGVIS_SRC_DIR}/deps/glad.c)
target_include_directories(ngvis_deps INTERFACE ${NGVIS_SRC_DIR}/deps)
target_include_directories(ngvis_deps INTERFACE ${NGVIS_SRC_DIR}/inc)
target_compile_options(ngvis_deps INTERFACE -g)

find_package(NGSolve CONFIG REQUIRED HINTS /opt/netgen)

set(CMAKE_INSTALL_PREFIX ${NGSOLVE_INSTALL_DIR}/${NGSOLVE_INSTALL_DIR_PYTHON} CACHE PATH "Install dir" FORCE)

add_subdirectory(src)
add_subdirectory(examples)
