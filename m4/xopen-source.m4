#########################################################################
#                                                                       #
#             Star Traders: A Game of Interstellar Trading              #
#                Copyright (C) 1990-2021, John Zaitseff                 #
#                                                                       #
#########################################################################

# Author: John Zaitseff <J.Zaitseff@zap.org.au>
# $Id$
#
# This file contains the macro USE_LATEST_XOPEN_SOURCE to determine the
# latest version of _XOPEN_SOURCE supported by the C library.  It does
# this by compiling code with various values of that symbol.  Once a
# particular value compiles without error, it redefines _XOPEN_SOURCE to
# that value using the auxiliary file ../lib/xopen-source.h and sets
# $x_cv_latest_xopen_source to one of the following values:
#
#   700       - _XOPEN_SOURCE=700 (for SUSv4 / XPG7) was accepted
#   600       - _XOPEN_SOURCE=600 (for SUSv3 / XPG6) was accepted
#   unknown   - _XOPEN_SOURCE set to above values failed to compile
#
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see https://www.gnu.org/licenses/.

AC_DEFUN([_CHECK_XOPEN_SOURCE], [dnl
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
	/* Test for $2 */
	@%:@define _XOPEN_SOURCE $1
	@%:@include <stdio.h>
    ])], [$3], [$4])
])dnl

AC_DEFUN([USE_LATEST_XOPEN_SOURCE], [dnl
    AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
    AH_BOTTOM([
/* Redefine _XOPEN_SOURCE as required */
@%:@include <xopen-source.h>
])
    AC_CACHE_CHECK([the latest supported version of _XOPEN_SOURCE],
	[x_cv_latest_xopen_source], [
	x_cv_latest_xopen_source=unknown
	_CHECK_XOPEN_SOURCE([700], [SUSv4 / XPG7], [
	    x_cv_latest_xopen_source=700
	], [_CHECK_XOPEN_SOURCE([600], [SUSv3 / XPG6], [
	    x_cv_latest_xopen_source=600
	])])
    ])
    AS_IF([test "x$x_cv_latest_xopen_source" != xunknown], [dnl
	AC_DEFINE_UNQUOTED([LATEST_XOPEN_SOURCE],
	    [$x_cv_latest_xopen_source],
	    [Define to the latest version of _XOPEN_SOURCE that is supported.])
    ])
])dnl USE_LATEST_XOPEN_SOURCE
