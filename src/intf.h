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

// Keycodes for inserting the default value in input routines
#define KEY_DEFAULTVAL1	'='
#define KEY_DEFAULTVAL2	';'

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

/*
  The functions in this interface create and manage a "stack of windows"
  that can overlap.  It is similar in spirit to the panels library, but
  does not allow windows to be raised or lowered.  In spite of this
  limitation, these functions are ample for the needs of this program.
*/

/*
  Function:   init_screen - Initialise the screen (terminal display)
  Parameters: (none)
  Returns:    (nothing)

  This function initialises the input keyboard and output terminal
  display using the Curses library.  It also draws an overall title at
  the top with the name of the game.  This function must be called before
  calling any other functions in this header file.
*/
extern void init_screen (void);


/*
  Function:   end_screen - Deinitialise the screen (terminal display)
  Parameters: (none)
  Returns:    (nothing)

  This function closes the input keyboard and output terminal display.
  It makes sure the screen is cleared before doing so.
*/
extern void end_screen (void);


/*
  Function:   newtxwin  - Create a new window, inserted into window stack
  Parameters: nlines    - Number of lines in new window
              ncols     - Number of columns in new window
              begin_y   - Starting line number (0 to LINES-1)
              begin_x   - Starting column number (0 to COLS-1)
              dofill    - True to draw background and box frame
              bkgd_attr - Background attribute
  Returns:    WINDOW *  - Pointer to new window

  This function creates a window using the Curses newwin() function and
  places it top-most in the stack of windows managed by this module.  A
  pointer to this new window is returned; the global variable curwin also
  points to this new window.  Note that begin_y and begin_x are zero-
  based global coordinates.  If dofill is true, bkgd_attr is used to fill
  the background and box(curwin, 0, 0) is called.  Note that wrefresh()
  is NOT called on the new window.

  If newtxwin() fails to create a new window due to insufficient memory,
  this function does NOT return: it terminates the program with an "out
  of memory" error message.
*/
extern WINDOW *newtxwin (int nlines, int ncols, int begin_y, int begin_x,
			 bool dofill, chtype bkgd_attr);


/*
  Function:   deltxwin - Delete the top-most window in the window stack
  Parameters: (none)
  Returns:    int      - ERR on failure, OK otherwise

  This function removes the top-most window from the Curses screen and
  from the stack managed by this module.  ERR is returned if there is no
  such window or if the Curses delwin() function fails.

  Note that the actual terminal screen is NOT refreshed: a call to
  txrefresh() should follow this one.  This allows multiple windows to be
  deleted without any annoying screen flashes.
*/
extern int deltxwin (void);


/*
  Function:   delalltxwin - Delete all windows in the window stack
  Parameters: (none)
  Returns:    int         - OK is always returned

  This function deletes all windows in the stack of windows managed by
  this module.  After calling this function, the global variable curwin
  will point to stdscr.  Note that the actual terminal screen is NOT
  refreshed: a call to txrefresh() should follow this one if appropriate.
*/
extern int delalltxwin (void);


/*
  Function:   txrefresh - Redraw all windows in the window stack
  Parameters: (none)
  Returns:    int       - Return code from doupdate(): either OK or ERR

  This function redraws (refreshes) all windows in the stack of windows
  managed by this module.  Windows are refreshed from bottom (first) to
  top (last).  The result of doupdate() is returned.

  Normal window output does not require calling txrefresh(): a call to
  wrefresh(curwin) is sufficient.  However, once a window has been
  deleted with deltxwin() (or all windows with delalltxwin()), windows
  under that one will need refreshing by calling txrefresh().
*/
extern int txrefresh (void);


/*
  Function:   attrpr - Print a string with a particular character rendition
  Parameters: win    - Window to use (should be curwin)
              attr   - Character rendition to use for the string
              format - printf()-like format string
              ...    - printf()-like arguments
  Returns:    int    - Return code from wprintw(): OK or ERR

  This function sets the character rendition (attributes) to attr, prints
  the string using wprintw(), then restores the previous character
  rendition.  The return code is as returned from wprintw().  Note that
  wrefresh() is NOT called.
*/
extern int attrpr (WINDOW *win, chtype attr, const char *restrict format, ...)
    __attribute__((format (printf, 3, 4)));


/*
  Function:   center - Centre a string in a given window
  Parameters: win    - Window to use (should be curwin)
              y      - Line on which to centre the string
              attr   - Character rendition to use for the string
              format - printf()-like format string
              ...    - printf()-like arguments
  Returns:    int    - Return code from wprintw(): OK or ERR

  This function prints a string formatted with wprintw() in the centre of
  line y in the window win, using the character rendition (attributes) in
  attr.  If the string is too long to fit the window, it is truncated.
  Please note that wrefresh() is NOT called.

  At the current time, the implementation of this function does NOT
  handle multibyte strings correctly: strlen() is used to determine the
  printing width of the string.
*/
extern int center (WINDOW *win, int y, chtype attr,
		   const char *restrict format, ...)
    __attribute__((format (printf, 4, 5)));


/*
  Function:   center2 - Centre two strings in a given window
  Parameters: win     - Window to use (should be curwin)
              y       - Line on which to centre the strings
              attr1   - Character rendition to use for initial string
	      attr2   - Character rendition to use for main string
	      initial - Initial string (no printf() formatting)
              format  - printf()-like format string for main string
              ...     - printf()-like arguments
  Returns:    int     - Return code from wprintw(): OK or ERR

  This function prints two strings side-by-side on line y in the centre
  of window win.  The first (initial) string is printed with character
  rendition (attributes) attr1 using waddstr().  The second (main) string
  uses wprintw(format, ...) with rendition attr2.  If the main string is
  too long to fit in the window alongside the initial string, the main
  string is truncated to fit.  Please note that wrefresh() is NOT called.

  As with center(), the current implementation does NOT handle multibyte
  strings correctly.
*/
extern int center2 (WINDOW *win, int y, chtype attr1, chtype attr2,
		    const char *initial, const char *restrict format, ...)
    __attribute__((format (printf, 6, 7)));


/*
  Function:   center3 - Centre three strings in a given window
  Parameters: win     - Window to use (should be curwin)
              y       - Line on which to centre the strings
              attr1   - Character rendition to use for initial string
              attr3   - Character rendition to use for final string
	      attr2   - Character rendition to use for main string
	      initial - Initial string (no printf() formatting)
              final   - Final string (no printf() formatting)
              format  - printf()-like format string for main string
              ...     - printf()-like arguments
  Returns:    int     - Return code from wprintw(): OK or ERR

  This function prints three strings side-by-side on line y in the centre
  of window win.  The first (initial) string is printed with character
  rendition (attributes) attr1 using waddstr().  The second (main) string
  uses wprintw(format, ...) with rendition attr2.  The third (final)
  string is then printed with rendition attr3 using waddstr().  If the
  strings are too long to fit the window width, the main (centre) string
  is truncated.  Please note that wrefresh() is NOT called.  Also note
  the order of rendition values: 1, 3, 2, NOT 1, 2, 3!

  As with center(), the current implementation does NOT handle multibyte
  strings correctly.
*/

extern int center3 (WINDOW *win, int y, chtype attr1, chtype attr3,
		    chtype attr2, const char *initial, const char *final,
		    const char *restrict format, ...)
    __attribute__((format (printf, 8, 9)));


/*
  Function:   gettxchar - Read a character from the keyboard
  Parameters: win       - Window to use (should be curwin)
  Returns:    int       - The keyboard character

  This function reads a single character from the keyboard.  The key is
  NOT echoed to the terminal display, nor is the cursor visibility
  affected.

  This implementation does not handle multibyte characters correctly:
  each part of the multibyte character most likely appears as a separate
  keyboard press.
*/
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


/*
  Function:   answer_yesno - Wait for a Yes/No answer
  Parameters: win          - Window to use (should be curwin)
              attr_keys    - Window rendition to use for key choices
  Returns:    bool         - True if Yes was selected, false if No

  This function prompts the user by printing " [Y/N] " using appropriate
  character renditions ("Y" and "N" in attr_keys, the rest in the current
  rendition), then waits for the user to press either "Y" (for Yes) or
  "N" (for No) on the keyboard, then prints the answer using A_BOLD.
  True is returned if "Y" was selected, false if "N".  Note that the
  cursor becomes invisible after calling this function.
*/
extern bool answer_yesno (WINDOW *win, chtype attr_keys);


/*
  Function:   wait_for_key - Print a message and wait for any key
  Parameters: win          - Window to use (should be curwin)
              y            - Line on which to print message
              attr         - Character rendition to use for message
  Returns:    (nothing)

  This function displays the message "Press <SPACE> to continue" in the
  centre of line y in window win, then waits for any key to be pressed.

  The reason the user is not asked "Press any key to continue" is
  historical: many, many people used to ask "where is the <ANY> key?" :-)

  The current implementation does not handle multibyte characters
  correctly: only the first byte of the character is consumed, with
  further bytes left in the keyboard queue.
*/
extern void wait_for_key (WINDOW *win, int y, chtype attr);


#endif /* included_INTF_H */
