
find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets)
qt_standard_project_setup()

set(SOURCES 
  common/image.h
  common/image.cpp
  common/options.h
  common/options.cpp
  common/stream_options.h
  common/stream_options.cpp
  cam_select_dialog.cpp
  format_converter.h
  format_converter.cpp
  main.cpp
  main_window.cpp
  message_handler.cpp
  viewfinder_gl.cpp
  viewfinder_qt.cpp
  viewfinder.h
)

set(MOC_HEADERS 
  cam_select_dialog.h
  main_window.h
  viewfinder_gl.h
  viewfinder_qt.h
)

set(RESOURCES
  assets/feathericons/feathericons.qrc
  assets/shader/shaders.qrc
)

qt_add_resources(qcam_resources_src ${RESOURCES})
qt_add_executable(qcam ${SOURCES} ${MOC_HEADERS} ${qcam_resources_src})
set_target_properties(qcam PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(qcam PROPERTIES COMPILE_OPTIONS ${THIRD_PARTY_COMPILER_WARNINGS})
add_clang_format(qcam)
target_link_libraries(qcam 
  PRIVATE PkgConfig::LIBCAMERA
  Qt6::Core Qt6::Gui Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Widgets
)
target_compile_options(qcam PRIVATE
    -DQT_NO_KEYWORDS
    -Wno-extra-semi
)
