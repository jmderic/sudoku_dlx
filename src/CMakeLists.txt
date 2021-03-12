# Copyright 2021 J. Mark Deric

# creating the executable
set (sudoku_dlx_SOURCES
  sudoku_main.cpp
  )

## TARTGET specific

add_executable(sudoku_dlx ${sudoku_dlx_SOURCES})

target_compile_options(sudoku_dlx PRIVATE -Werror -Wall -Wextra)

target_include_directories(sudoku_dlx PRIVATE
  .
  ../include
  )
