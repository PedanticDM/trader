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
  size.  The newtxwin() function automatically places a new window in the
  centre-top of the terminal screen.
*/

#define MIN_LINES	24	// Minimum number of lines in terminal
#define MIN_COLS	80	// Minimum number of columns in terminal

#define WIN_LINES	MIN_LINES   // Number of lines used in main window
#define WIN_COLS	MIN_COLS    // Number of columns used in main window

#define WCENTER			-1  // Centre the new window

#define MAX_DLG_LINES		10  // Default maximum lines of text in dialog box
#define YESNO_COLS		4   // Space to allow for "Yes" or "No" response
#define ORDINAL_COLS		5   // Space for ordinals (1st, 2nd, etc)
#define TOTAL_VALUE_COLS	18  // Space for total value (monetary)
#define SHARE_PRICE_COLS	12  // Space for "Price per share"
#define SHARE_RETURN_COLS	10  // Space for "Return per share"
#define STOCK_OWNED_COLS	10  // Space for "Holdings (shares)"
#define OWNERSHIP_COLS		10  // Space for "Company ownership (%)"


// Check if resizing events are supported
#ifdef KEY_RESIZE
#  define HANDLE_RESIZE_EVENTS	1
#else
#  undef HANDLE_RESIZE_EVENTS
#endif


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

// Timeout value (in ms) for Meta-X-style keyboard input
#ifdef NCURSES_VERSION
#  define META_TIMEOUT	ESCDELAY
#else
#  define META_TIMEOUT	1000
#endif


/************************************************************************
*                     Global variable declarations                      *
************************************************************************/

extern WINDOW *curwin;			// Top-most (current) window
extern bool use_color;			// True to use colour


// Character renditions (attributes) used by Star Traders

extern chtype attr_root_window;		// Root window (behind all others)
extern chtype attr_game_title;		// One-line game title at top

extern chtype attr_normal_window;	// Normal window background
extern chtype attr_title;		// Normal window title
extern chtype attr_subtitle;		// Normal window subtitle
extern chtype attr_normal;		// Normal window text
extern chtype attr_highlight;		// Normal window highlighted string
extern chtype attr_blink;		// Blinking text in normal window
extern chtype attr_keycode;		// Make keycodes like <1> stand out
extern chtype attr_choice;		// Make map/company choices stand out
extern chtype attr_input_field;		// Background for input text field
extern chtype attr_waitforkey;		// "Press any key", normal window

extern chtype attr_map_window;		// Map window background
extern chtype attr_mapwin_title;	// Map window title (player name, turn)
extern chtype attr_mapwin_highlight;	// Map window title highlight
extern chtype attr_mapwin_blink;	// Map window title blinking text
extern chtype attr_map_empty;		// On map, empty space
extern chtype attr_map_outpost;		// On map, outpost
extern chtype attr_map_star;		// On map, star
extern chtype attr_map_company;		// On map, company
extern chtype attr_map_choice;		// On map, a choice of moves

extern chtype attr_status_window;	// Status window background

extern chtype attr_error_window;	// Error message window background
extern chtype attr_error_title;		// Error window title
extern chtype attr_error_normal;	// Error window ordinary text
extern chtype attr_error_highlight;	// Error window highlighted string
extern chtype attr_error_waitforkey;	// "Press any key", error window


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
              begin_y   - Starting line number (0 to LINES-1) or WCENTER
              begin_x   - Starting column number (0 to COLS-1) or WCENTER
              dofill    - True to draw background and box frame
              bkgd_attr - Background character rendition
  Returns:    WINDOW *  - Pointer to new window

  This function creates a window using the Curses newwin() function and
  places it top-most in the stack of windows managed by this module.  A
  pointer to this new window is returned; the global variable curwin also
  points to this new window.  Note that begin_y and begin_x are zero-
  based global coordinates; either (or both) can be WCENTER to centre
  that dimension within the terminal screen.  If dofill is true,
  bkgd_attr is used to fill the background and box(curwin, 0, 0) is
  called.  Note that wrefresh() is NOT called on the new window.

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
  Function:   txdlgbox     - Display a dialog box and wait for any key
  Parameters: maxlines     - Maximum number of lines of text in window
              ncols        - Number of columns in dialog box window
              begin_y      - Starting line number (0 to LINES-1) or WCENTER
              begin_x      - Starting column number (0 to COLS-1) or WCENTER
              bkgd_attr    - Background character rendition
              title_attr   - Character rendition to use for dialog box title
              norm_attr    - Normal character rendition in box
              alt1_attr    - Alternate character rendition 1 (highlight)
              alt2_attr    - Alternate character rendition 2 (more highlighted)
              keywait_attr - "Press any key" character rendition
              boxtitle     - Dialog box title (may be NULL)
              format       - Dialog box text, as passed to prepstr()
              ...          - Dialog box text format parameters
  Returns:    int          - OK is always returned

  This function creates a dialog box window using newtxwin(), displays
  boxtitle centred on the first line (if boxtitle is not NULL), displays
  format (and associated parameters) centred using prepstr(), then waits
  for the user to press any key before closing the dialog box window.
  Note that txrefresh() is NOT called once the window is closed.
*/
extern int txdlgbox (int maxlines, int ncols, int begin_y, int begin_x,
		     chtype bkgd_attr, chtype title_attr, chtype norm_attr,
		     chtype alt1_attr, chtype alt2_attr, chtype keywait_attr,
		     const char *restrict boxtitle,
		     const char *restrict format, ...);


/*
  Function:   mkchstr      - Prepare a string for printing to screen
  Parameters: chbuf        - Pointer to chtype buffer in which to store string
              chbufsize    - Number of chtype elements in chbuf
              attr_norm    - Normal character rendition to use
              attr_alt1    - First alternate character rendition to use
              attr_alt2    - Second alternate character rendition to use
              maxlines     - Maximum number of screen lines to use
              maxwidth     - Maximum width of each line, in chars
              widthbuf     - Pointer to buffer to store widths of each line
              widthbufsize - Number of int elements in widthbuf
              format       - Format string as described below
              ...          - Arguments for the format string
  Returns:    int          - Number of lines actually used

  This function converts the format string and following arguments into
  chbuf, a chtype buffer that can be used for calls to leftch(), centerch()
  and rightch().  At most maxlines lines are used, each with a maximum
  width of maxwidth.  The actual widths of each resulting line are stored
  in widthbuf (which must not be NULL).  If maxlines is greater than 1,
  lines are wrapped as needed.

  The format string is similar to but more limited than printf().  In
  particular, only the following conversion specifiers are understood:

    %%      - Print the ASCII percent sign (ASCII code U+0025)

    %c      - Insert the next parameter as a character (type char)
    %s      - Insert the next parameter as a string (type char *)
    %d      - Insert the next parameter as an integer (type int)
    %'d     - As above, using the locale's thousands group separator
    %ld     - Insert the next parameter as a long int
    %'ld    - As above, using the locale's thousands group separator
    %f      - Insert the next parameter as a floating point number (double)
    %.mf    - As above, with precision "m" (a positive integer > 0)
    %'.mf   - As above, using the locale's thousands group separator
    %N      - Insert the next parameter as a double, using the locale's
              national currency format (extension to printf())
    %!N     - As above, using the locale's national currency format without
              the actual currency symbol (extension to printf())

  Instead of using "%" to convert the next parameter, "%m$" can be used
  to indicate fixed parameter m (where m is an integer from 1 to 8).  For
  example, "%4$s" inserts the fourth parameter after "format" as a string
  into chbuf.  As with printf(), using "%m$" together with ordinary "%"
  forms is undefined.  If "%m$" is used, no parameter m can be skipped.

  Note that no other flag, field width, precision or length modifier
  characters are recognised: if needed, these should be formatted FIRST
  with snprintf(), then inserted using %s as appropriate.

  In addition to the conversion specifiers, the following character
  rendition flags are understood, where the "^" character is a literal
  ASCII circumflex accent:

    ^^   - Print the circumflex accent (ASCII code U+005E)
    ^{   - Switch to using attr_alt1 character rendition (alternate mode 1)
    ^}   - Switch to using attr_norm character rendition
    ^[   - Switch to using attr_alt2 character rendition (alternate mode 2)
    ^]   - Switch to using attr_norm character rendition

  Characters other than these are inserted as literals, except that '\n'
  will force the start of a new line.  By default, attr_norm is used as
  the character rendition (attributes).

  Please note that this function does NOT handle multibyte characters
  correctly: widths may be incorrect (byte count, not actual width) and
  multibyte characters may be split over two lines.

  This function returns the actual number of lines used (from 0 to
  maxlines).  If an error is detected, the application terminates.
*/
extern int mkchstr (chtype *restrict chbuf, int chbufsize, chtype attr_norm,
		    chtype attr_alt1, chtype attr_alt2, int maxlines,
		    int maxwidth, int *restrict widthbuf, int widthbufsize,
		    const char *restrict format, ...);


/*
  Function:   vmkchstr     - Prepare a string for printing to screen
  Parameters: chbuf        - Pointer to chtype buffer in which to store string
              chbufsize    - Number of chtype elements in chbuf
              attr_norm    - Normal character rendition to use
              attr_alt1    - First alternate character rendition to use
              attr_alt2    - Second alternate character rendition to use
              maxlines     - Maximum number of screen lines to use
              maxwidth     - Maximum width of each line, in chars
              widthbuf     - Pointer to buffer to store widths of each line
              widthbufsize - Number of int elements in widthbuf
              format       - Format string as described for mkchstr()
              args         - Variable argument list
  Returns:    int          - Number of lines actually used

  This function is exactly the same as mkchstr(), except that it is
  called with a va_list parameter args instead of a variable number of
  arguments.  Note that va_end() is NOT called on args, and that args is
  undefined after this function.
*/
extern int vmkchstr (chtype *restrict chbuf, int chbufsize, chtype attr_norm,
		     chtype attr_alt1, chtype attr_alt2, int maxlines,
		     int maxwidth, int *restrict widthbuf, int widthbufsize,
		     const char *restrict format, va_list args);


/*
  Function:   chstrdup  - Duplicate a chtype string
  Parameters: chstr     - String to duplicate
              chstrsize - Maximum number of chtype elements in chstr
  Returns:    chtype *  - Pointer to new (duplicated) string

  This function returns a new string of type chtype * that contains a
  copy of the string in chstr.  No errors are returned: if sufficient
  memory is not available, the program terminates with an "Out of memory"
  message.
*/
extern chtype *chstrdup (const chtype *restrict chstr, int chstrsize);


/*
  Function:   leftch   - Print strings in chstr left-aligned
  Parameters: win      - Window to use (should be curwin)
              y        - Line on which to print first string
              x        - Starting column number for each line
              chstr    - chtype string as returned from mkchstr()
              lines    - Number of lines in chstr as returned from mkchstr()
              widthbuf - Widths of each line as returned from mkchstr()
  Returns:    int      - Always returns OK

  This function takes the strings in the chtype array chstr and prints
  them left-aligned in the window win.  Note that wrefresh() is NOT
  called.
*/
extern int leftch (WINDOW *win, int y, int x, const chtype *restrict chstr,
		   int lines, const int *restrict widthbuf);


/*
  Function:   centerch - Print strings in chstr centred in window
  Parameters: win      - Window to use (should be curwin)
              y        - Line on which to print first string
              offset   - Column offset to add to position for each line
              chstr    - chtype string as returned from mkchstr()
              lines    - Number of lines in chstr as returned from mkchstr()
              widthbuf - Widths of each line as returned from mkchstr()
  Returns:    int      - ERR if more lines in chstr[] than lines, else OK

  This function takes the strings in the chtype array chstr and prints
  them centred in the window win, offset by the parameter offset.  Note
  that wrefresh() is NOT called.  ERR is returned if there are more lines
  in chstr[] than are passed in the parameter lines.
*/
extern int centerch (WINDOW *win, int y, int offset,
		     const chtype *restrict chstr, int lines,
		     const int *restrict widthbuf);


/*
  Function:   rightch  - Print strings in chstr right-aligned
  Parameters: win      - Window to use (should be curwin)
              y        - Line on which to print first string
              x        - Ending column number for each line
              chstr    - chtype string as returned from mkchstr()
              lines    - Number of lines in chstr as returned from mkchstr()
              widthbuf - Widths of each line as returned from mkchstr()
  Returns:    int      - ERR if more lines in chstr[] than lines, else OK

  This function takes the strings in the chtype array chstr and prints
  them right-aligned in the window win, with each line ending just before
  column x.  Note that wrefresh() is NOT called.  ERR is returned if
  there are more lines in chstr[] than are passed in the parameter lines.
*/
extern int rightch (WINDOW *win, int y, int x, const chtype *restrict chstr,
		    int lines, const int *restrict widthbuf);


/*
  Function:   left      - Print strings left-aligned
  Parameters: win       - Window to use (should be curwin)
              y         - Line on which to print first string
              x         - Starting column number for each line
              attr_norm - Normal character rendition to use
              attr_alt1 - First alternate character rendition to use
              attr_alt2 - Second alternate character rendition to use
              maxlines  - Maximum number of screen lines to use
              format    - Format string as described for mkchstr()
              ...       - Arguments for the format string
  Returns:    int       - Number of lines actually used

  This shortcut function prepares a chtype string using mkchstr(), then
  prints the string using leftch().  At most MAX_DLG_LINES are printed,
  with the maximum width being that of the window win - x - 2 (the "2" is
  for the right-hand border).
*/
extern int left (WINDOW *win, int y, int x, chtype attr_norm, chtype attr_alt1,
		 chtype attr_alt2, int maxlines, const char *restrict format,
		 ...);


/*
  Function:   center    - Print strings centred in window
  Parameters: win       - Window to use (should be curwin)
              y         - Line on which to print first string
              offset    - Column offset to add to position for each line
              attr_norm - Normal character rendition to use
              attr_alt1 - First alternate character rendition to use
              attr_alt2 - Second alternate character rendition to use
              maxlines  - Maximum number of screen lines to use
              format    - Format string as described for mkchstr()
              ...       - Arguments for the format string
  Returns:    int       - Number of lines actually used

  This shortcut function prepares a chtype string using mkchstr(), then
  prints the string using centerch().  At most MAX_DLG_LINES are printed,
  with the maximum width being that of the window win - 4 (for borders).
*/
extern int center (WINDOW *win, int y, int offset, chtype attr_norm,
		   chtype attr_alt1, chtype attr_alt2, int maxlines,
		   const char *restrict format, ...);


/*
  Function:   right     - Print strings right-aligned
  Parameters: win       - Window to use (should be curwin)
              y         - Line on which to print first string
              x         - Ending column number for each line
              attr_norm - Normal character rendition to use
              attr_alt1 - First alternate character rendition to use
              attr_alt2 - Second alternate character rendition to use
              maxlines  - Maximum number of screen lines to use
              format    - Format string as described for mkchstr()
              ...       - Arguments for the format string
  Returns:    int       - Number of lines actually used

  This shortcut function prepares a chtype string using mkchstr(), then
  prints the string using rightch().  At most MAX_DLG_LINES are printed,
  with the maximum width being that of x - 2 (the "2" is for the
  left-hand border).
*/
extern int right (WINDOW *win, int y, int x, chtype attr_norm, chtype attr_alt1,
		  chtype attr_alt2, int maxlines, const char *restrict format,
		  ...);


/*
  Function:   old_attrpr - Print a string with a particular character rendition
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
extern int old_attrpr (WINDOW *win, chtype attr, const char *restrict format, ...)
    __attribute__((format (printf, 3, 4)));


/*
  Function:   old_center - Centre a string in a given window
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
extern int old_center (WINDOW *win, int y, chtype attr,
		   const char *restrict format, ...)
    __attribute__((format (printf, 4, 5)));


/*
  Function:   old_center2 - Centre two strings in a given window
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
extern int old_center2 (WINDOW *win, int y, chtype attr1, chtype attr2,
		    const char *initial, const char *restrict format, ...)
    __attribute__((format (printf, 6, 7)));


/*
  Function:   old_center3 - Centre three strings in a given window
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

extern int old_center3 (WINDOW *win, int y, chtype attr1, chtype attr3,
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


/*
  Function:   gettxline  - Read a line from the keyboard (low-level)
  Parameters: win        - Window to use (should be curwin)
              buf        - Pointer to preallocated buffer
              bufsize    - Size of buffer in bytes
              modified   - Pointer to modified status (result)
              multifield - Allow <TAB>, etc, to exit this function
              emptyval   - String used if input line is empty
              defaultval - String used if default key is pressed
              allowed    - Characters allowed in the input line
              stripspc   - True to strip leading/trailing spaces
              y, x       - Start of the input field (line, column)
              width      - Width of the input field
              attr       - Character rendition to use for input field
  Returns:    int        - Status code: OK, ERR or KEY_ keycode

  This low-level function draws an input field width characters long
  using attr as the character rendition, then reads a line of input from
  the keyboard and places it into the preallocated buffer buf[] of size
  bufsize.  On entry, buf[] must contain a valid C string; this string is
  used as the initial contents of the input field.  On exit, buf[]
  contains the final string as edited or input by the user.  This string
  is printed in place of the input field using the original character
  rendition (attributes) with A_BOLD added.  Many Emacs/Bash-style
  keyboard editing shortcuts are understood.

  If ENTER, RETURN, ^M or ^J is pressed, OK is returned.  In this case,
  leading and trailing spaces are stripped if stripspc is true; if an
  empty string is entered, the string pointed to by emptyval (if not
  NULL) is stored in buf[].

  If CANCEL, EXIT, ESC, ^C, ^\ or ^G is pressed, ERR is returned.  In
  this case, buf[] contains the string as left by the user: emptyval is
  NOT used, nor are leading and trailing spaces stripped.

  If multifield is true, the UP and DOWN arrow keys, as well as TAB,
  Shift-TAB, ^P (Previous) and ^N (Next) return KEY_UP or KEY_DOWN as
  appropriate.  As with CANCEL etc., emptyval is NOT used, nor are
  leading and trailing spaces stripped.

  In all of these cases, the boolean variable *modified (if modified is
  not NULL) is set to true if the input line was actually modified in any
  way (including if the user made any changed, spaces were stripped or if
  emptyval was copied into buf[]).

  If KEY_DEFAULTVAL1 or KEY_DEFAULTVAL2 is pressed when the input line is
  empty, the string pointed to by defaultval (if not NULL) is placed in
  the buffer as if typed by the user.  Editing is NOT terminated in this
  case.

  If allowed is not NULL, only characters in that string are allowed to
  be entered into the input line.  For example, if allowed points to
  "0123456789abcdefABCDEF", only those characters would be allowed (in
  this instance, allowing the user to type in a hexadecimal number).

  Note that the character rendition (attributes) in attr may contain a
  printing character.  For example, A_BOLD | '_' is a valid rendition
  that causes the input field to be a series of "_" characters in bold.

  This implementation does not handle multibyte characters correctly:
  each part of the multibyte character most likely appears as a separate
  keyboard press and is handled as a separate character, causing the
  cursor position to be incorrect.  In addition, allowed is compared on a
  byte-by-byte basis, not character-by-character.
*/
extern int gettxline (WINDOW *win, char *buf, int bufsize,
		      bool *restrict modified, bool multifield,
		      const char *emptyval, const char *defaultval,
		      const char *allowed, bool stripspc, int y, int x,
		      int width, chtype attr);


/*
  Function:   gettxstr   - Read a string from the keyboard
  Parameters: win        - Window to use (should be curwin)
              bufptr     - Address of pointer to buffer
              modified   - Pointer to modified status (result)
              multifield - Allow <TAB>, etc, to exit this function
              y, x       - Start of the input field (line, column)
              width      - Width of the input field
              attr       - Character rendition to use for input field
  Returns:    int        - Status code: OK, ERR or KEY_ keycode

  This function calls gettxline() to allow the user to enter a string via
  the keyboard.  On entry, bufptr must be the address of a char * pointer
  variable; that pointer (*bufptr) must either be NULL or contain the
  address of a buffer previously allocated with gettxstr().  If *bufptr
  is NULL, a buffer of BUFSIZE is automatically allocated using malloc();
  this buffer is used to store and return the input line.

  Apart from bufptr, all parameters are as used for gettxline().  The
  gettxline() parameters emptyval and defaultval are passed as "",
  allowed is NULL and stripspc is true.
*/
extern int gettxstr (WINDOW *win, char **bufptr, bool *restrict modified,
		     bool multifield, int y, int x, int width, chtype attr);


/*
  Function:   gettxdouble - Read a floating-point number from the keyboard
  Parameters: win         - Window to use (should be curwin)
              result      - Pointer to result variable
              min         - Minimum value allowed (may be -INFINITY)
              max         - Maximum value allowed (may be INFINITY)
              emptyval    - Value to use for empty input
              defaultval  - Value to use for default input
              y, x        - Start of the input field (line, column)
              width       - Width of the input field
              attr        - Character rendition to use for input field
  Returns:    int         - Status code: OK, ERR or KEY_ keycode

  This function calls gettxline() to allow the user to type in a valid
  floating-point number between min and max (inclusive).  If gettxline()
  returns OK, the entered number is checked for validity.  If it is
  valid, it is stored in *result and the function returns OK.  If it is
  not valid, the user is prompted to reenter the number.  Any other
  result from gettxline() is passed back to the caller.  Note that the
  low-level function gettxline() is called with multifield set to false.

  This function is locale-aware, although multibyte strings are not
  handled correctly.  In particular, the default value is formatted using
  strfmon() and uses the locale monetary default decimal places
  (frac_digits).  In addition, the user is allowed to use the locale's
  radix character (decimal point) and the thousands separator, as well as
  the monetary versions of these.
*/
extern int gettxdouble (WINDOW *win, double *restrict result, double min,
			double max, double emptyval, double defaultval,
			int y, int x, int width, chtype attr);


/*
  Function:   gettxlong  - Read an integer number from the keyboard
  Parameters: win        - Window to use (should be curwin)
              result     - Pointer to result variable
              min        - Minimum value allowed
              max        - Maximum value allowed
              emptyval   - Value to use for empty input
              defaultval - Value to use for default input
              y, x       - Start of the input field (line, column)
              width      - Width of the input field
              attr       - Character rendition to use for input field
  Returns:    int        - Status code: OK, ERR or KEY_ keycode

  This function behaves almost exactly like gettxdouble(), except that
  only integer numbers are allowed to be entered.

  This function is locale-aware, although multibyte strings are not
  handled correctly.  In particular, the user is allowed to use the
  locale's thousands separator and the monetary thousands separator.
*/
extern int gettxlong (WINDOW *win, long int *restrict result, long int min,
		      long int max, long int emptyval, long int defaultval,
		      int y, int x, int width, chtype attr);


/*
  Function:   answer_yesno - Wait for a Yes/No answer
  Parameters: win          - Window to use (should be curwin)
  Returns:    bool         - True if Yes was selected, false if No

  This function waits for the user to press either "Y" (for Yes) or "N"
  (for No) on the keyboard, then prints the answer using A_BOLD.  True is
  returned if "Y" was selected, false if "N".  Note that the cursor
  becomes invisible after calling this function.
*/
extern bool answer_yesno (WINDOW *win);


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
