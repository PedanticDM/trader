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

// Visibility of the cursor in Curses
typedef enum curs_type {
    CURS_INVISIBLE	= 0,
    CURS_NORMAL		= 1,
    CURS_VISIBLE	= 2
} curs_type_t;

#define CURS_OFF	(CURS_INVISIBLE)
#define CURS_ON		(CURS_VISIBLE)


// Keycodes
#define KEY_BS		(0010)
#define KEY_TAB		(0011)
#define KEY_RETURN	(0012)
#define KEY_ESC		(0033)
#define KEY_DEL		(0177)

#define KEY_CTRL(x)	(x - 0100)

// Control-arrow key combinations
#ifndef KEY_CDOWN
#  define KEY_CDOWN	(01007)
#endif
#ifndef KEY_CUP
#  define KEY_CUP	(01060)
#endif
#ifndef KEY_CLEFT
#  define KEY_CLEFT	(01033)
#endif
#ifndef KEY_CRIGHT
#  define KEY_CRIGHT	(01052)
#endif

// Keycode for inserting the default value
#define KEY_DEFAULTVAL	'='

// Timeout value (in ms) for Meta-X-style keyboard input
#define META_TIMEOUT	(1000)


/*
  This version of Star Traders only utilises WIN_COLS x WIN_LINES of a
  terminal window.  COL_OFFSET and LINE_OFFSET define offsets that should
  be added to each newwin() call to position the window correctly.
*/

#define MIN_LINES	(24)	/* Minimum number of lines in terminal */
#define MIN_COLS	(80)	/* Minimum number of columns in terminal */

#define WIN_LINES	MIN_LINES	/* Number of lines in main windows */
#define WIN_COLS	MIN_COLS	/* Number of columns in main windows */

#define LINE_OFFSET	(0)				/* Window offsets */
#define COL_OFFSET	((COLS - WIN_COLS) / 2)
#define COL_CENTER(x)	((COLS - (x)) / 2)


// Colour and non-colour attribute selection
#define ATTR(color, nocolor) (use_color ? (color) : (nocolor))


// Colour pairs used in Star Traders
enum color_pairs {
    DEFAULT_COLORS = 0,
    BLACK_ON_WHITE,
    RED_ON_BLACK,
    CYAN_ON_BLUE,
    YELLOW_ON_BLACK,
    YELLOW_ON_BLUE,
    YELLOW_ON_CYAN,
    GREEN_ON_BLACK,
    BLUE_ON_BLACK,
    WHITE_ON_BLACK,
    WHITE_ON_RED,
    WHITE_ON_BLUE,
};


// Window attributes used in Star Traders
#define ATTR_GAME_TITLE		ATTR(COLOR_PAIR(YELLOW_ON_CYAN)  | A_BOLD, A_REVERSE | A_BOLD)
#define ATTR_ROOT_WINDOW	ATTR(COLOR_PAIR(WHITE_ON_BLACK),           A_NORMAL)
#define ATTR_NORMAL_WINDOW	ATTR(COLOR_PAIR(WHITE_ON_BLUE),            A_NORMAL)
#define ATTR_MAP_WINDOW		ATTR(COLOR_PAIR(WHITE_ON_BLACK),           A_NORMAL)
#define ATTR_STATUS_WINDOW	ATTR(COLOR_PAIR(BLACK_ON_WHITE),           A_REVERSE)
#define ATTR_ERROR_WINDOW	ATTR(COLOR_PAIR(WHITE_ON_RED),             A_REVERSE)
#define ATTR_WINDOW_TITLE	ATTR(COLOR_PAIR(YELLOW_ON_BLACK) | A_BOLD, A_REVERSE)
#define ATTR_WINDOW_SUBTITLE	ATTR(COLOR_PAIR(WHITE_ON_BLACK),           A_REVERSE)
#define ATTR_MAP_TITLE		ATTR(COLOR_PAIR(WHITE_ON_BLUE),            A_NORMAL)
#define ATTR_MAP_T_HIGHLIGHT	ATTR(COLOR_PAIR(YELLOW_ON_BLUE)  | A_BOLD, A_BOLD)
#define ATTR_MAP_T_STANDOUT	ATTR(COLOR_PAIR(YELLOW_ON_BLUE)  | A_BOLD | A_BLINK, A_BOLD | A_BLINK)
#define ATTR_ERROR_TITLE	ATTR(COLOR_PAIR(YELLOW_ON_BLACK) | A_BOLD, A_BOLD)
#define ATTR_INPUT_FIELD	ATTR(COLOR_PAIR(WHITE_ON_BLACK),           A_BOLD | '_')
#define ATTR_KEYCODE_STR	ATTR(COLOR_PAIR(YELLOW_ON_BLACK) | A_BOLD, A_REVERSE)
#define ATTR_HIGHLIGHT_STR	ATTR(COLOR_PAIR(YELLOW_ON_BLUE)  | A_BOLD, A_BOLD)
#define ATTR_STANDOUT_STR	ATTR(COLOR_PAIR(YELLOW_ON_BLUE)  | A_BOLD | A_BLINK, A_BOLD | A_BLINK)
#define ATTR_ERROR_STR		ATTR(COLOR_PAIR(WHITE_ON_RED)    | A_BOLD, A_REVERSE)
#define ATTR_WAITNORMAL_STR	ATTR(COLOR_PAIR(CYAN_ON_BLUE),             A_NORMAL)
#define ATTR_WAITERROR_STR	ATTR(COLOR_PAIR(WHITE_ON_RED),             A_REVERSE)
#define ATTR_MAP_EMPTY		ATTR(COLOR_PAIR(BLUE_ON_BLACK)   | A_BOLD, A_NORMAL)
#define ATTR_MAP_OUTPOST	ATTR(COLOR_PAIR(GREEN_ON_BLACK)  | A_BOLD, A_NORMAL)
#define ATTR_MAP_STAR		ATTR(COLOR_PAIR(YELLOW_ON_BLACK) | A_BOLD, A_BOLD)
#define ATTR_MAP_COMPANY	ATTR(COLOR_PAIR(RED_ON_BLACK)    | A_BOLD, A_NORMAL)
#define ATTR_MAP_CHOICE		ATTR(COLOR_PAIR(WHITE_ON_RED)    | A_BOLD, A_REVERSE)


/************************************************************************
*                     Global variable declarations                      *
************************************************************************/

extern WINDOW *curwin;			// Top-most (current) window
extern bool use_color;			// True to use colour in Star Traders


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

// Output routines

extern int center (WINDOW *win, int y, int attr, const char *format, ...)
    __attribute__((format (printf, 4, 5)));

extern int center2 (WINDOW *win, int y, int attr_initial, int attr_string,
		    const char *initial, const char *format, ...)
    __attribute__((format (printf, 6, 7)));

extern int attrpr (WINDOW *win, int attr, const char *format, ...)
    __attribute__((format (printf, 3, 4)));

// Input routines

extern int gettxchar (WINDOW *win);
extern int gettxline (WINDOW *win, char *buf, int bufsize, bool multifield,
		      int maxlen, const char *emptyval, const char *defaultval,
		      const char *allowed, bool stripspc, int y, int x,
		      int fieldsize, int attr, bool *modified);
extern int gettxstring (WINDOW *win, char **bufptr, bool multifield,
			int y, int x, int fieldsize, int attr, bool *modified);

extern bool answer_yesno (WINDOW *win);
extern void wait_for_key (WINDOW *win, int y, int attr);


#endif /* included_INTF_H */
