dnl *********************************************************************
dnl *                                                                   *
dnl *           Star Traders: A Game of Interstellar Trading            *
dnl *              Copyright (C) 1990-2016, John Zaitseff               *
dnl *                                                                   *
dnl *********************************************************************
dnl
dnl Author: John Zaitseff <J.Zaitseff@zap.org.au>
dnl $Id$
dnl
dnl This file, configure.ac, contains information required by autoconf to
dnl generate the corresponding configure script.
dnl
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or (at
dnl your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see http://www.gnu.org/licenses/.


AC_INIT([Star Traders], [7.10], [J.Zaitseff@zap.org.au], [trader], [http://www.zap.org.au/software/trader/])
AC_DEFINE([PACKAGE_AUTHOR], ["John Zaitseff"], [Package author])

AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([src/trader.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 -Wall -Werror foreign dist-bzip2])
AC_CONFIG_LIBOBJ_DIR([lib])

AC_PROG_CC
AC_PROG_CC_C99

AS_IF([test "x$ac_cv_prog_cc_c99" = xno], [
	AC_MSG_ERROR([requires an ISO/IEC 9899:1999 (C99) compiler])
])

gl_EARLY

AX_CFLAGS_WARN_ALL
AX_C___ATTRIBUTE__
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.3])

gl_INIT

AX_WITH_CURSES
AS_IF([test "x$ax_cv_curses" != xyes || test "x$ax_cv_curses_color" != xyes], [
	AC_MSG_ERROR([requires an X/Open-compatible Curses library with colour])
])
AS_IF([test "x$ax_cv_curses_enhanced" != xyes], [
	AC_MSG_WARN([no X/Open-compatible Enhanced Curses library found

*****************    Locales with multibyte character sequences (such as
***  WARNING  ***    UTF-8) require a wide-character version of Curses,
*****************    such as NcursesW, to work correctly.
])
])

AC_CONFIG_FILES([
	Makefile
	lib/Makefile
	src/Makefile
	doc/Makefile
	po/Makefile.in
	m4/Makefile
])
AC_OUTPUT
