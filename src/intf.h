/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, intf.h, contains declarations for basic text input/output
  functions used in Star Traders.  It uses the X/Open Curses library to
  provide terminal-independent functionality.


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

/*
  This version of Star Traders only utilises WIN_COLS x WIN_LINES of a
  terminal screen; this terminal must be at least MIN_COLS x MIN_LINES in
  size; the newtxwin() function automatically places a new window in the
  centre-top of the terminal screen.  The program does not yet handle
  terminal resizing events.
*/

#define MIN_LINES	24	// Minimum number of lines in terminal
#define MIN_COLS	80	// Minimum number of columns in terminal

#define WIN_LINES	MIN_LINES	// Number of lines in main window
#define WIN_COLS	MIN_COLS	// Number of columns in main window

#define WCENTER(x)	((COLS - (x)) / 2)	// Centre the window


// Visibility of the cursor in Curses (for curs_set())
typedef enum curs_type {
    CURS_INVISIBLE	= 0,
    CURS_NORMAL		= 1,
    CURS_VISIBLE	= 2
} curs_type_t;

#define CURS_OFF	CURS_INVISIBLE
#define CURS_ON		CURS_NORMAL


// Keycodes not defined by Curses
#define KEY_BS		0010		// ASCII ^H backspace
#define KEY_TAB		0011		// ASCII ^I character tabulation
#define KEY_RETURN	0012		// ASCII ^J line feed
#define KEY_ESC		0033		// ASCII ^[ escape
#define KEY_DEL		0177		// ASCII    delete

#define KEY_CTRL(x)	((x) - 0100)	// ASCII control character

#define KEY_ILLEGAL	077777		// No key should ever return this!

// Keycode for inserting the default value in input routines
#define KEY_DEFAULTVAL	'='

// Control-arrow key combinations, as returned by NCurses
#ifndef KEY_CDOWN
#  define KEY_CDOWN	01007		// CTRL + Down Arrow
#  define KEY_CUP	01060		// CTRL + Up Arrow
#  define KEY_CLEFT	01033		// CTRL + Left Arrow
#  define KEY_CRIGHT	01052		// CTRL + Right Arrow
#endif

// Keycodes only defined by NCurses
#ifndef KEY_RESIZE
#  define KEY_RESIZE	KEY_ILLEGAL
#endif
#ifndef KEY_EVENT
#  define KEY_EVENT	KEY_ILLEGAL
#endif
#ifndef KEY_MOUSE
#  define KEY_MOUSE	KEY_ILLEGAL
#endif

// Timeout value (in ms) for Meta-X-style keyboard input
#ifdef NCURSES_VERSION
#  define META_TIMEOUT	ESCDELAY
#else
#  define META_TIMEOUT	1000
#endif


/*
  Colour pairs used in Star Traders.  This list MUST be synchronised with
  the initialisation of colour pairs in init_screen().

  X/Open Curses only lists the following colours: black, blue, green,
  cyan, red, magenta, yellow, white.  Most implementations allow these
  colours plus bold versions (for the foreground).
*/
enum color_pairs {
    DEFAULT_COLORS = 0,
    BLACK_ON_WHITE,
    BLUE_ON_BLACK,
    GREEN_ON_BLACK,
    CYAN_ON_BLUE,
    RED_ON_BLACK,
    YELLOW_ON_BLACK,
    YELLOW_ON_BLUE,
    YELLOW_ON_CYAN,
    WHITE_ON_BLACK,
    WHITE_ON_BLUE,
    WHITE_ON_RED
};

// Colour and non-colour character rendition selection
#define ATTR(color, nocolor)	(use_color ? (color) : (nocolor))

/*
  Character renditions (attributes) used in Star Traders
*/
#define ATTR_ROOT_WINDOW	ATTR(COLOR_PAIR(WHITE_ON_BLACK),                     A_NORMAL)
#define ATTR_GAME_TITLE		ATTR(COLOR_PAIR(YELLOW_ON_CYAN)  | A_BOLD,           A_REVERSE | A_BOLD)

#define ATTR_NORMAL_WINDOW	ATTR(COLOR_PAIR(WHITE_ON_BLUE),                      A_NORMAL)
#define ATTR_TITLE		ATTR(COLOR_PAIR(YELLOW_ON_BLACK) | A_BOLD,           A_REVERSE)
#define ATTR_SUBTITLE		ATTR(COLOR_PAIR(WHITE_ON_BLACK),                     A_REVERSE)
#define ATTR_NORMAL		ATTR_NORMAL_WINDOW
#define ATTR_HIGHLIGHT		ATTR(COLOR_PAIR(YELLOW_ON_BLUE)  | A_BOLD,           A_BOLD)
#define ATTR_BLINK		ATTR(COLOR_PAIR(YELLOW_ON_BLUE)  | A_BOLD | A_BLINK, A_BOLD | A_BLINK)
#define ATTR_KEYCODE		ATTR(COLOR_PAIR(YELLOW_ON_BLACK) | A_BOLD,           A_REVERSE)
#define ATTR_CHOICE		ATTR(COLOR_PAIR(WHITE_ON_RED)    | A_BOLD,           A_REVERSE)
#define ATTR_INPUT_FIELD	ATTR(COLOR_PAIR(WHITE_ON_BLACK),                     A_BOLD | '_')
#define ATTR_WAITFORKEY		ATTR(COLOR_PAIR(CYAN_ON_BLUE),                       A_NORMAL)

#define ATTR_MAP_WINDOW		ATTR(COLOR_PAIR(WHITE_ON_BLACK),                     A_NORMAL)
#define ATTR_MAPWIN_TITLE	ATTR(COLOR_PAIR(WHITE_ON_BLUE),                      A_NORMAL)
#define ATTR_MAPWIN_HIGHLIGHT	ATTR(COLOR_PAIR(YELLOW_ON_BLUE)  | A_BOLD,           A_BOLD)
#define ATTR_MAPWIN_BLINK	ATTR(COLOR_PAIR(YELLOW_ON_BLUE)  | A_BOLD | A_BLINK, A_BOLD | A_BLINK)
#define ATTR_MAP_EMPTY		ATTR(COLOR_PAIR(BLUE_ON_BLACK)   | A_BOLD,           A_NORMAL)
#define ATTR_MAP_OUTPOST	ATTR(COLOR_PAIR(GREEN_ON_BLACK)  | A_BOLD,           A_NORMAL)
#define ATTR_MAP_STAR		ATTR(COLOR_PAIR(YELLOW_ON_BLACK) | A_BOLD,           A_BOLD)
#define ATTR_MAP_COMPANY	ATTR(COLOR_PAIR(RED_ON_BLACK)    | A_BOLD,           A_BOLD)
#define ATTR_MAP_CHOICE		ATTR(COLOR_PAIR(WHITE_ON_RED)    | A_BOLD,           A_REVERSE)

#define ATTR_STATUS_WINDOW	ATTR(COLOR_PAIR(BLACK_ON_WHITE),                     A_REVERSE)

#define ATTR_ERROR_WINDOW	ATTR(COLOR_PAIR(WHITE_ON_RED),                       A_REVERSE)
#define ATTR_ERROR_TITLE	ATTR(COLOR_PAIR(YELLOW_ON_BLACK) | A_BOLD,           A_BOLD)
#define ATTR_ERROR_NORMAL	ATTR_ERROR_WINDOW
#define ATTR_ERROR_HIGHLIGHT	ATTR(COLOR_PAIR(WHITE_ON_RED)    | A_BOLD,           A_REVERSE)
#define ATTR_ERROR_WAITFORKEY	ATTR(COLOR_PAIR(WHITE_ON_RED),                       A_REVERSE)


/************************************************************************
*                     Global variable declarations                      *
************************************************************************/

extern WINDOW *curwin;			// Top-most (current) window
extern bool use_color;			// True to use colour in Star Traders


/************************************************************************
*              Basic text input/output function prototypes              *
************************************************************************/

extern void init_screen (void);
extern void end_screen (void);

// Simplified panel-like window functions

extern WINDOW *newtxwin (int nlines, int ncols, int begin_y, int begin_x,
			 bool draw_bkgd_box, chtype bkgd_attr);
extern int deltxwin (void);
extern int delalltxwin (void);
extern int txrefresh (void);

// Output routines

extern int center (WINDOW *win, int y, int attr, const char *format, ...)
    __attribute__((format (printf, 4, 5)));

extern int center2 (WINDOW *win, int y, int attr_initial, int attr_string,
		    const char *initial, const char *format, ...)
    __attribute__((format (printf, 6, 7)));

extern int center3 (WINDOW *win, int y, int attr_initial, int attr_final,
		    int attr_string, const char *initial, const char *final,
		    const char *format, ...)
    __attribute__((format (printf, 8, 9)));

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
extern int gettxdouble (WINDOW *win, double *result, double min, double max,
			double emptyval, double defaultval, int y, int x,
			int fieldsize, int attr);
extern int gettxlong (WINDOW *win, long *result, long min, long max,
		      long emptyval, long defaultval, int y, int x,
		      int fieldsize, int attr);

extern bool answer_yesno (WINDOW *win);
extern void wait_for_key (WINDOW *win, int y, int attr);


#endif /* included_INTF_H */
