#########################################################################
#                                                                       #
#             Star Traders: A Game of Interstellar Trading              #
#                Copyright (C) 1990-2019, John Zaitseff                 #
#                                                                       #
#########################################################################

# Author: John Zaitseff <J.Zaitseff@zap.org.au>
# $Id$
#
# This file, data/Makefile.am, contains the non-code data directory
# Makefile for Star Traders.  It needs to be processed by automake to
# produce the corresponding Makefile.in.
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


dist_desktop_DATA       = trader.desktop

icons16dir              = ${iconsdir}/16x16/apps
icons22dir              = ${iconsdir}/22x22/apps
icons24dir              = ${iconsdir}/24x24/apps
icons32dir              = ${iconsdir}/32x32/apps
icons48dir              = ${iconsdir}/48x48/apps
icons64dir              = ${iconsdir}/64x64/apps
icons96dir              = ${iconsdir}/96x96/apps
icons128dir             = ${iconsdir}/128x128/apps
icons256dir             = ${iconsdir}/256x256/apps
icons512dir             = ${iconsdir}/512x512/apps
iconsscalabledir        = ${iconsdir}/scalable/apps

dist_icons16_DATA       = icons-16/trader.png
dist_icons22_DATA       = icons-22/trader.png
dist_icons24_DATA       = icons-24/trader.png
dist_icons32_DATA       = icons-32/trader.png
dist_icons48_DATA       = icons-48/trader.png
dist_icons64_DATA       = icons-64/trader.png
dist_icons96_DATA       = icons-96/trader.png
dist_icons128_DATA      = icons-128/trader.png
dist_icons256_DATA      = icons-256/trader.png
dist_icons512_DATA      = icons-512/trader.png
dist_iconsscalable_DATA = trader.svg

EXTRA_DIST = \
	README			\
	trader.desktop.in


trader.desktop: trader.desktop.in
	$(top_srcdir)/build-aux/msgfmt-desktop -d $(top_srcdir)/po $< $@
