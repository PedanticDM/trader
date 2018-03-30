#########################################################################
#                                                                       #
#             Star Traders: A Game of Interstellar Trading              #
#                Copyright (C) 1990-2018, John Zaitseff                 #
#                                                                       #
#########################################################################

# Author: John Zaitseff <J.Zaitseff@zap.org.au>
# $Id$
#
# This file, src/Makefile.am, contains the source-code Makefile for Star
# Traders.  It needs to be processed by automake to produce the
# corresponding Makefile.in.
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


bin_PROGRAMS	= trader

trader_SOURCES	= \
	trader.c	trader.h	\
	globals.c	globals.h	\
	game.c		game.h		\
	move.c		move.h		\
	exch.c		exch.h		\
	fileio.c	fileio.h	\
	help.c		help.h		\
	intf.c		intf.h		\
	utils.c		utils.h		\
			system.h

trader_CPPFLAGS	= -I$(top_builddir)/lib -I$(top_srcdir)/lib	\
		  @CURSES_CFLAGS@ -DLOCALEDIR=\"$(localedir)\"
trader_LDADD	= @CURSES_LIBS@ $(top_builddir)/lib/libgnu.a @LIBICONV@ @LIBINTL@

EXTRA_DIST	= README
