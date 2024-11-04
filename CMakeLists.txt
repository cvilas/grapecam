# =================================================================================================
# Copyright (C) 2024 grapecam contributors
# =================================================================================================

cmake_minimum_required(VERSION 3.28.0)
project(grapecam LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TEMPLATES_DIR ${CMAKE_SOURCE_DIR}/cmake)
include(${TEMPLATES_DIR}/03_compiler_config.cmake)
include(${TEMPLATES_DIR}/04_code_formatter.cmake)

include(${CMAKE_SOURCE_DIR}/external/third_party_versions.cmake)

# define staging directories for external projects
set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/external)  # location of build tree for external projects
set(EP_DEPLOY_DIR ${EP_BINARY_DIR}/deploy)       # location of deployables from external projects
file(MAKE_DIRECTORY ${EP_DEPLOY_DIR})

# Configuration of all external dependencies are in external/CMakeLists.txt
# NOTE: external/CMakeLists.txt is processed as if it was a separate project. This means:
# - Variables set there are not shared by the rest of this project
# - CMake parameters must be explicitly passed as if cmake was called on it from the command line
message(STATUS "========= External projects: Configuring =========")
execute_process(
  COMMAND ${CMAKE_COMMAND}
  -G "Ninja" ${CMAKE_SOURCE_DIR}/external
  -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}
  -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DCMAKE_INSTALL_PREFIX=${EP_DEPLOY_DIR}
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
  -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
  -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
  WORKING_DIRECTORY ${EP_BINARY_DIR}
  RESULT_VARIABLE _result
)
if (NOT ${_result} EQUAL 0)
  message(FATAL_ERROR "External projects: Configuration Error")
endif()

message(STATUS "========= External projects: Building  =========")
execute_process(
  COMMAND ${CMAKE_COMMAND} --build .
  WORKING_DIRECTORY ${EP_BINARY_DIR}
  RESULT_VARIABLE _result
)
if (NOT ${_result} EQUAL 0)
  message(FATAL_ERROR "External projects: Build Error")
endif ()

list(PREPEND CMAKE_PREFIX_PATH ${EP_DEPLOY_DIR}/)
install(DIRECTORY ${EP_DEPLOY_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX} USE_SOURCE_PERMISSIONS)
if (CMAKE_CROSSCOMPILING)
  list(APPEND CMAKE_FIND_ROOT_PATH ${EP_DEPLOY_DIR})
endif ()
message(STATUS "======= External projects: Build completed =======")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED IMPORTED_TARGET libcamera)
message(STATUS "libcamera: Using version: ${LIBCAMERA_VERSION} (${LIBCAMERA_LINK_LIBRARIES})")

# Now configure and build this project
add_subdirectory(hello)
add_subdirectory(qcam)
add_subdirectory(grapecam)
