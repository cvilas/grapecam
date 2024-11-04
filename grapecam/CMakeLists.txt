# =================================================================================================
# Copyright (C) 2024 grapecam contributors
# =================================================================================================

find_package(grape ${GRAPE_VERSION_REQUIRED} REQUIRED COMPONENTS ${GRAPE_MODULES})

add_executable(${CMAKE_PROJECT_NAME}_list list.cpp)
add_clang_format(${CMAKE_PROJECT_NAME}_list)
target_link_libraries(${CMAKE_PROJECT_NAME}_list PRIVATE PkgConfig::LIBCAMERA)

add_executable(${CMAKE_PROJECT_NAME}_pub pub.cpp)
add_clang_format(${CMAKE_PROJECT_NAME}_pub)
target_link_libraries(${CMAKE_PROJECT_NAME}_pub PRIVATE PkgConfig::LIBCAMERA grape::app)

install(
  TARGETS 
  ${CMAKE_PROJECT_NAME}_list 
  ${CMAKE_PROJECT_NAME}_pub
  RUNTIME DESTINATION bin)