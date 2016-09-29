#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([seismograph], [0], [https://github.com/coreos/seismograph/issues])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/cgpt/cgpt.c])
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules subdir-objects])
AM_SILENT_RULES([yes])

LT_INIT([disable-static])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([BLKID], [blkid])
PKG_CHECK_MODULES([UUID], [uuid])
PKG_CHECK_MODULES([EXT2FS], [ext2fs])
PKG_CHECK_MODULES([MNT], [mount])

# Optional features
AC_ARG_ENABLE([loopy],
              [AS_HELP_STRING([--enable-loopy], [build loopy])])
AM_CONDITIONAL([ENABLE_LOOPY], [test "x$enable_loopy" = "xyes"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
