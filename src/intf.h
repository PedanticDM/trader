/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, intf.h, contains function declarations for basic text input/
  output routines for Star Traders.


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


#ifndef included_INTF_H
#define included_INTF_H 1


#include "system.h"


/************************************************************************
*                    Constants and type declarations                    *
************************************************************************/

#define MIN_LINES	(24)	/* Minimum number of lines in terminal */
#define MIN_COLS	(80)	/* Minimum number of columns in terminal */

/*
  This version of Star Traders only utilises WIN_COLS x WIN_LINES of a
  terminal window.  COL_OFFSET and LINE_OFFSET define offsets that should
  be added to each newwin() call to position the window correctly.
*/

#define WIN_LINES	MIN_LINES	/* Number of lines in main windows */
#define WIN_COLS	MIN_COLS	/* Number of columns in main windows */

#define LINE_OFFSET	(0)				/* Window offsets */
#define COL_OFFSET	((COLS - WIN_COLS) / 2)
#define COL_CENTER(x)	((COLS - (x)) / 2)


// Colour and non-colour attribute selection
#define ATTR(color, nocolor) (has_colors() ? (color) : (nocolor))


// Colour pairs used in Star Traders
enum color_pairs {
    DEFAULT_COLORS = 0,
    WHITE_ON_BLACK,
    WHITE_ON_BLUE,
    WHITE_ON_RED,
    YELLOW_ON_CYAN,
    BLACK_ON_WHITE,
};


/************************************************************************
*                     Global variable declarations                      *
************************************************************************/

extern WINDOW *curwin;			// Top-most (current) window


/************************************************************************
*             Basic text input/output function declarations             *
************************************************************************/

extern void init_screen (void);
extern void end_screen (void);

// Simplified panel-like window functions
extern WINDOW *newtxwin (int nlines, int ncols, int begin_y, int begin_x);
extern int deltxwin (void);
extern int delalltxwin (void);
extern int txrefresh (void);

extern int center (WINDOW *win, int y, const bool clrline,
		   const char *format, ...)
    __attribute__((format (printf, 4, 5)));


#endif /* included_INTF_H */
