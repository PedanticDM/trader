/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, system.h, contains system #include directives for Star
  Traders.


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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif


/************************************************************************
*                          System header files                          *
************************************************************************/

#define _GNU_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

#include <getopt.h>

#if defined(HAVE_NCURSESW) && defined(HAVE_NCURSESW_H)
#  include <ncursesw/curses.h>
#elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
#elif defined(HAVE_CURSES_H)
#  include <curses.h>
#else
#  error SysV-compatible curses library required
#endif


/************************************************************************
*                       Miscellaneous definitions                       *
************************************************************************/

// Visibility of the cursor in Curses
#define CURS_INVISIBLE   (0)
#define CURS_NORMAL      (1)
#define CURS_VISIBLE     (1)
#define CURS_VERYVISIBLE (2)


// For future use in internationalisation
#ifndef _
#  define _(x) (x)
#endif
#ifndef N_
#  define N_(x) (x)
#endif


// Compiler __attributes__ for less-capable compilers
#ifndef HAVE___ATTRIBUTE__
#  define __attribute__(x)
#endif


#endif /* included_SYSTEM_H */
