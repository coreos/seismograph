#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([vboot_reference], [0],
        [https://github.com/coreos/vboot_reference/issues])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([cgpt/cgpt.c])
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([BLKID], [blkid])
PKG_CHECK_MODULES([UUID], [uuid])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
