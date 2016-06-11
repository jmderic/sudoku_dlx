#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

dnl $Id: configure.ac,v 1.1 2008/04/15 04:39:31 mark Exp $

AC_PREREQ(2.59)
AC_INIT(jmd_dlx, 0.1)dnl optional 3rd arg: BUG-REPORT-ADDRESS
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AC_REVISION($Revision: 1.1 $)dnl
AM_MAINTAINER_MODE


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.

# ad hoc fix-up because 
# _LT_AC_TAGVAR(always_export_symbols, $1)=yes didn't work
# FIX ME
CXXFLAGS="`echo $CXXFLAGS | sed -e 's/-O[[0-9]]*/-O0/'`"

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 lib/Makefile
                 src/Makefile])
AC_OUTPUT
