# Copyright 2021 J. Mark Deric

# only one tested
cmake_minimum_required (VERSION 3.10)

project (sudoku_dlx)

# The version number.
set (sudoku_dlx_MAJOR_VERSION 0)
set (sudoku_dlx_MINOR_VERSION 1)
set (sudoku_dlx_PATCH_VERSION 0)
set(sudoku_dlx_VERSION
  ${sudoku_dlx_MAJOR_VERSION}.${sudoku_dlx_MINOR_VERSION}.${sudoku_dlx_PATCH_VERSION})

# basic compile standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## PROJECT specific

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/sudoku_dlx_config.h.in"
  "${PROJECT_BINARY_DIR}/sudoku_dlx_config.h"
  )
 
# add the binary tree to the search path for include files
# so that we will find sudoku_dlx_config.h
include_directories(
  "${PROJECT_BINARY_DIR}"
  )

# applies to all targets.  Maybe what we want?
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")

option (ASAN_ENABLED
  "Build with AddressSanitizer"
  OFF
  )

option (LSAN_ENABLED
  "Build with LeakSanitizer"
  OFF
  )

if(ASAN_ENABLED OR LSAN_ENABLED)
set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fno-omit-frame-pointer")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer")
endif(ASAN_ENABLED OR LSAN_ENABLED)

if(ASAN_ENABLED)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif(ASAN_ENABLED)

if(LSAN_ENABLED)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=leak")
endif(LSAN_ENABLED)

## TARTGET specific
add_subdirectory(src)

## testing gtest integration
add_subdirectory(test)
