# $Id: Makefile.am,v 1.1.1.1 2008/04/09 20:40:19 mark Exp $

bin_PROGRAMS = sudoku_dlx
sudoku_dlx_SOURCES = sudoku_main.cpp sudoku_squares.h

# $(all_includes) sets the include path found by configure
sudoku_dlx_CPPFLAGS= -I$(top_builddir)/include $(all_includes)

# the library search path.
sudoku_dlx_LDFLAGS = $(all_libraries) 

## sudoku_dlx_CXXFLAGS = -DCOLUMN_SETUP_DEBUG=1
## sudoku_dlx_CXXFLAGS = -DROW_SETUP_DEBUG=1
## sudoku_dlx_CXXFLAGS = -DCOLUMN_SETUP_DEBUG=1 -DROW_SETUP_DEBUG=1
