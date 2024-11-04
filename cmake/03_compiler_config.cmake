# =================================================================================================
# Copyright (C) 2018 GRAPE Contributors
# =================================================================================================

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # required by source analysis tools

# Baseline compiler warning settings for project and external targets
add_compile_options(-Wall -Wextra -Wpedantic -Werror)
set(THIRD_PARTY_COMPILER_WARNINGS -Wall -Wextra -Wpedantic)

# clang warnings
set(CLANG_WARNINGS -Weverything 
  -Wno-c++20-compat 
  -Wno-pre-c++20-compat-pedantic 
  -Wno-pre-c++17-compat 
  -Wno-c++98-compat 
  -Wno-c++98-compat-pedantic 
  -Wno-unsafe-buffer-usage 
  -Wno-padded 
  -Wno-switch-default 
  -Wno-ctad-maybe-unsupported
  -Wno-global-constructors
  -Wno-weak-vtables
  -Wno-exit-time-destructors)

# GCC warnings
set(GCC_WARNINGS
  -Wshadow # warn the user if a variable declaration shadows one from a parent context
  -Wnon-virtual-dtor # warn if a class with virtual functions has a non-virtual destructor.
  #-Wold-style-cast # warn for c-style casts
  -Wcast-align # warn for potential performance problem casts
  -Wunused # warn on anything being unused
  -Woverloaded-virtual # warn if you overload (not override) a virtual function
  -Wconversion # warn on type conversions that may lose data
  -Wsign-conversion # warn on sign conversions
  -Wnull-dereference # warn if a null dereference is detected
  -Wdouble-promotion # warn if float is implicit promoted to double
  -Wformat=2 # warn on security issues around functions that format output (ie printf)
  -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation    
  -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
  -Wduplicated-cond # warn if if / else chain has duplicated conditions
  -Wduplicated-branches # warn if if / else branches have duplicated code
  -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
  #-Wuseless-cast # warn if you perform a cast to the same type
  -Wconversion 
  -Wcast-qual 
  -Wpointer-arith 
)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  add_compile_options(-fcolor-diagnostics ${CLANG_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-fdiagnostics-color=always ${GCC_WARNINGS})
else()
  message(FATAL_ERROR "Unsupported compiler '${CMAKE_CXX_COMPILER_ID}'")
endif()

# Linter (clang-tidy)
option(ENABLE_LINTER "Enable static analysis" ON)
if(ENABLE_LINTER)
  find_program(LINTER_BIN NAMES clang-tidy QUIET)
  if(LINTER_BIN)
    set(LINTER_ARGS 
      -extra-arg=-Wno-ignored-optimization-argument 
      -extra-arg=-Wno-unknown-warning-option)
    # NOTE: To speed up linting, clang-tidy is invoked via clang-tidy-cache.
    # (https://github.com/matus-chochlik/ctcache) Cache location is set by environment variable
    # CTCACHE_DIR
    set(LINTER_INVOKE_COMMAND ${TEMPLATES_DIR}/clang-tidy-cache.py ${LINTER_BIN} -p ${CMAKE_BINARY_DIR} ${LINTER_ARGS})
    set(CMAKE_C_CLANG_TIDY ${LINTER_INVOKE_COMMAND})
    set(CMAKE_CXX_CLANG_TIDY ${LINTER_INVOKE_COMMAND})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      set(FLAGS_FOR_CLANG_TIDY_WITH_GCC -D__cpp_concepts=202002L) # to enable std::expected
      add_compile_options(${FLAGS_FOR_CLANG_TIDY_WITH_GCC})
    endif()
  else()
    message(WARNING "Linter (clang-tidy) not found.")
  endif()
endif()

# print summary
message(STATUS "Compiler configuration:")
message(STATUS "\tCompiler          : ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "\tENABLE_LINTER     : ${ENABLE_LINTER} (${LINTER_BIN})")
