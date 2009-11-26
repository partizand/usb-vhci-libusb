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

MY_CHECK_WS

MY_ARG_ENABLE(
    debug,
    no,
    [Compile with debugging flags.]
)

AM_CONDITIONAL(ENABLE_DEBUG, [test "x$enable_debug" != "xno"])
AM_CONDITIONAL(ENABLE_RELEASE, [test "x$enable_debug" = "xno"])

if test "x$enable_debug" != "xno"; then
	CFLAGS="$CFLAGS -g3 -O0"
	CXXFLAGS="$CXXFLAGS -g3 -O0"
else
	CFLAGS="$CFLAGS -O2"
	CXXFLAGS="$CXXFLAGS -O2"
fi

# Checks for programs.
AC_LANG_C
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_self], [],
	[AC_MSG_ERROR([pthread library is missing])]
)

# Checks for header files.
AC_CHECK_HEADERS([pthread.h stdlib.h stdio.h string.h errno.h fcntl.h unistd.h sys/ioctl.h], [],
	[AC_MSG_ERROR([missing header files])]
)
AC_CHECK_HEADERS([linux/ioctl.h], [],
	[AC_MSG_ERROR([missing linux kernel header files])]
)

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([
Makefile
src/Makefile

])

AC_OUTPUT
