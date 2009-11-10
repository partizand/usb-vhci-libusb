#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([libusb_vhci], [0.1])
AC_CONFIG_SRCDIR([src/libusb_vhci.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([gnu])
AM_MAINTAINER_MODE

LT_PREREQ([2.2.6])
LT_INIT

# Checks for programs.
AC_LANG_C
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([
Makefile
src/Makefile

])

AC_OUTPUT
