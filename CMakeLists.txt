cmake_minimum_required(VERSION 3.3)
project(NGVIS)

option( USE_OCC     "(not supported) compile with OpenCascade geometry kernel" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_modules")

set(NGVIS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NGVIS_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

find_package(NGSolve CONFIG REQUIRED PATHS $ENV{NETGENDIR}/.. /opt/netgen /Applications/Netgen.app/Contents/Resources/CMake)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${NGSOLVE_INSTALL_DIR} CACHE PATH "Install dir" FORCE)
endif()


if(USE_OCC)
  find_package(OpenCasCade REQUIRED)
  add_definitions(-DOCCGEOMETRY -D_OCC64)
  include_directories(${OCC_INCLUDE_DIR})
endif(USE_OCC)

add_subdirectory(src)
