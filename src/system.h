/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2017, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, system.h, contains system-specific definitions and #include
  directives for Star Traders.


  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/.
*/


#ifndef included_SYSTEM_H
#define included_SYSTEM_H 1


/************************************************************************
*                        Portability definitions                        *
************************************************************************/

#include "config.h"			// Generated by configure


/************************************************************************
*                          System header files                          *
************************************************************************/

#define _XOPEN_SOURCE	700		// Use SUSv4 where possible
#define _GNU_SOURCE	1		// Use GNU extensions as well


// Headers defined by ISO/IEC 9899:1999 (C99)

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <locale.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>
#include <wctype.h>


// Headers defined by X/Open Single Unix Specification v4

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <monetary.h>
#include <langinfo.h>


// Headers defined by the GNU C Library

#include <crc.h>
#include <getopt.h>


// Internationalisation using GNU gettext

#include "gettext.h"			// This handles ENABLE_NLS correctly

#define _(string)	gettext(string)
#define N_(string)	gettext_noop(string)


// Character set conversion for game files

#ifdef HAVE_ICONV
#  define USE_UTF8_GAME_FILE	1
#  include "striconv.h"
#else
#  undef USE_UTF8_GAME_FILE
#endif


// X/Open-compatible Curses library

#define _XOPEN_SOURCE_EXTENDED	1	// Required by old versions of NcursesW

#if defined(HAVE_NCURSESW_CURSES_H)
#  include <ncursesw/curses.h>
#elif defined(HAVE_NCURSESW_H)
#  include <ncursesw.h>
#elif defined(HAVE_NCURSES_CURSES_H)
#  include <ncurses/curses.h>
#elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
#elif defined(HAVE_CURSES_H)
#  include <curses.h>
#else
#  error "X/Open-compatible Curses header file required"
#endif


/************************************************************************
*                       Miscellaneous definitions                       *
************************************************************************/

// Compiler __attributes__ for less-capable compilers
#ifndef HAVE___ATTRIBUTE__
#  define __attribute__(x)
#endif


#endif /* included_SYSTEM_H */
