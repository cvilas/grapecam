# =================================================================================================
# Copyright (C) 2024 grapecam contributors
# =================================================================================================

# =================================================================================================
# This file configures and builds external dependencies at cmake configuration time and makes 
# it available to this project
# =================================================================================================

cmake_minimum_required(VERSION 3.28.0)
project(grapecam-external LANGUAGES C CXX)

find_package(Git REQUIRED)
include(ExternalProject)
include(${CMAKE_CURRENT_LIST_DIR}/third_party_versions.cmake)

set_directory_properties(PROPERTIES EP_UPDATE_DISCONNECTED 1) # skip update step

# Collect common arguments to configure external projects
set(EP_CMAKE_EXTRA_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}
    -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
    -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})

# helper macro useful for dependency handling
macro(add_dummy_target)
  if(NOT TARGET ${ARGV0})
    add_custom_target(${ARGV0})
  endif()
endmacro()

# replace list separator or ExternalProject_Add won't parse it properly
string(REPLACE ";" "|" GRAPE_MODULES_STR "${GRAPE_MODULES}")

find_package(grape ${GRAPE_VERSION_REQUIRED} QUIET COMPONENTS ${GRAPE_MODULES})
if(grape_FOUND)
  message(STATUS "GRAPE: Using version ${grape_VERSION} from ${grape_DIR}")
  add_dummy_target(grape)
else()
  message(STATUS "GRAPE: Building ${GRAPE_VERSION_REQUIRED} from source")
  set(GRAPE_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
      -DBUILD_MODULES=${GRAPE_MODULES_STR}
      -DENABLE_CACHE=OFF  # breaks gcc builds
      -DENABLE_LINTER=OFF # breaks gcc builds
      -DENABLE_FORMATTER=OFF)
  ExternalProject_Add(
    grape
    GIT_REPOSITORY "https://github.com/cvilas/grape.git"
    GIT_TAG feat/grapecam # TODO: replace with v${GRAPE_VERSION_REQUIRED}
    GIT_SHALLOW true
    LIST_SEPARATOR |
    CMAKE_ARGS ${GRAPE_CMAKE_ARGS})
endif()

find_package(glfw3 ${GLFW3_VERSION_REQUIRED} QUIET)
if(glfw3_FOUND)
  message(STATUS "GLFW3: Using version ${glfw3_VERSION} from ${glfw3_DIR}")
  add_dummy_target(glfw3)
else()
  message(STATUS "GLFW3: Building ${GLFW3_VERSION_REQUIRED} from source")
  set(GLFW3_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
      -DGLFW_BUILD_DOCS=OFF 
      -DGLFW_BUILD_EXAMPLES=OFF
      -DGLFW_BUILD_TESTS=OFF 
      -DGLFW_BUILD_X11=ON 
      -DGLFW_BUILD_WAYLAND=ON)
  ExternalProject_Add(
    glfw3
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG ${GLFW3_VERSION_REQUIRED}
    GIT_SHALLOW true
    CMAKE_ARGS ${GLFW3_CMAKE_ARGS})
endif()
