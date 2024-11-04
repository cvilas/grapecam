# =================================================================================================
# Copyright (C) 2024 grapecam contributors
# =================================================================================================

find_package(grape ${GRAPE_VERSION_REQUIRED} REQUIRED COMPONENTS ${GRAPE_MODULES})
find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)

add_executable(${CMAKE_PROJECT_NAME}_list list.cpp)
add_clang_format(${CMAKE_PROJECT_NAME}_list)
target_link_libraries(${CMAKE_PROJECT_NAME}_list PUBLIC PkgConfig::LIBCAMERA)

add_executable(${CMAKE_PROJECT_NAME}_view view.cpp)
add_clang_format(${CMAKE_PROJECT_NAME}_view)
target_link_libraries(${CMAKE_PROJECT_NAME}_view PUBLIC PkgConfig::LIBCAMERA SDL3::SDL3)

add_executable(${CMAKE_PROJECT_NAME}_pub pub.cpp)
add_clang_format(${CMAKE_PROJECT_NAME}_pub)
target_link_libraries(${CMAKE_PROJECT_NAME}_pub PUBLIC PkgConfig::LIBCAMERA grape::app)

install(
  TARGETS 
  ${CMAKE_PROJECT_NAME}_list 
  ${CMAKE_PROJECT_NAME}_pub
  ${CMAKE_PROJECT_NAME}_view
  RUNTIME DESTINATION bin)