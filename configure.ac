#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Microde], [0.1], [aqiank@gmail.com])
AC_CONFIG_SRCDIR([src/ui.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([GTK2], [gtk+-2.0])
PKG_CHECK_MODULES([GTKSOURCEVIEW2], [gtksourceview-2.0])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strchr strdup strstr strtol])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
