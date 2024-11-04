add_executable(hello hello.cpp)
add_clang_format(hello)
target_link_libraries(hello PRIVATE PkgConfig::LIBCAMERA)
