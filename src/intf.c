/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, intf.c, contains the actual implementation of basic text
  input/output routines as used in Star Traders.


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


#include "system.h"
#include "intf.h"
#include "utils.h"


/************************************************************************
*                Module constants and type declarations                 *
************************************************************************/

#define BUFSIZE		(1024)		/* Size of various string buffers */


typedef struct txwin {
    WINDOW		*win;		// Pointer to window structure
    struct txwin	*next;		// Next window in stack
    struct txwin	*prev;		// Previous window in stack
} txwin_t;


/************************************************************************
*                      Global variable definitions                      *
************************************************************************/

WINDOW *curwin = NULL;		// Top-most (current) window
bool use_color = false;		// True to use colour in Star Traders


/************************************************************************
*                           Module variables                            *
************************************************************************/

txwin_t *topwin   = NULL;	// Top-most txwin structure
txwin_t *firstwin = NULL;	// First (bottom-most) txwin structure


/************************************************************************
*             Basic text input/output function definitions              *
************************************************************************/

/*-----------------------------------------------------------------------
  Function:   init_screen  - Initialise the screen (terminal)
  Arguments:  (none)
  Returns:    (nothing)

  This function initialises the input (keyboard) and output (screen)
  using the Curses library.
*/

void init_screen (void)
{
    int i;


    initscr();

    if ((COLS < MIN_COLS) || (LINES < MIN_LINES)) {
	err_exit("terminal size is too small (%d x %d required)",
		 MIN_COLS, MIN_LINES);
    }

    use_color = has_colors();

    curwin = stdscr;
    topwin = NULL;
    firstwin = NULL;

    noecho();
    curs_set(CURS_OFF);
    raw();

    if (use_color) {
	start_color();

	init_pair(WHITE_ON_BLACK,  COLOR_WHITE,  COLOR_BLACK);
	init_pair(WHITE_ON_BLUE,   COLOR_WHITE,  COLOR_BLUE);
	init_pair(WHITE_ON_RED,    COLOR_WHITE,  COLOR_RED);
	init_pair(YELLOW_ON_BLACK, COLOR_YELLOW, COLOR_BLACK);
	init_pair(YELLOW_ON_BLUE,  COLOR_YELLOW, COLOR_BLUE);
	init_pair(YELLOW_ON_CYAN,  COLOR_YELLOW, COLOR_CYAN);
	init_pair(CYAN_ON_BLUE,    COLOR_CYAN,   COLOR_BLUE);
	init_pair(BLACK_ON_WHITE,  COLOR_BLACK,  COLOR_WHITE);

	bkgd(ATTR_ROOT_WINDOW);
    }

    clear();

    for (i = 0; i < COLS; i++) {
	mvwaddch(stdscr, 0, i, ATTR_GAME_TITLE | ' ');
    }
    center(stdscr, 0, ATTR_GAME_TITLE, PACKAGE_NAME);

    attrset(ATTR_ROOT_WINDOW);
    refresh();
}


/*-----------------------------------------------------------------------
  Function:   end_screen  - End using the screen (terminal)
  Arguments:  (none)
  Returns:    (nothing)

  This function closes the input (keyboard) and output (screen) using the
  Curses library.  It makes sure the screen is cleared before doing so.
*/

void end_screen (void)
{
    delalltxwin();

    clear();
    refresh();
    endwin();

    curwin = NULL;
}


/************************************************************************
*                Simplified panel-like window functions                 *
************************************************************************/

/*-----------------------------------------------------------------------
  Function:   newtxwin  - Create a new window, inserted into window stack
  Arguments:  nlines    - Number of lines in new window
              ncols     - Number of columns in new window
              begin_y   - Starting line number (global coordinates)
              begin_x   - Starting column number (global coordinates)
  Returns:    WINDOW *  - Pointer to new window structure

  This function creates a window (using the Curses newwin() function) and
  places it top-most in the stack of windows managed by this module.  A
  pointer to the new window is returned; the global variable "curwin"
  also points to this new window.  Please note that wrefresh() is NOT
  called on the new window.
*/

WINDOW *newtxwin (int nlines, int ncols, int begin_y, int begin_x)
{
    WINDOW *win;
    txwin_t *nw;


    win = newwin(nlines, ncols, begin_y, begin_x);
    if (win == NULL) {
	return NULL;
    }

    nw = malloc(sizeof(txwin_t));
    if (nw == NULL) {
	delwin(win);
	return NULL;
    }

    nw->win = win;
    nw->next = NULL;
    nw->prev = topwin;

    if (topwin != NULL) {
	topwin->next = nw;
    }

    topwin = nw;
    curwin = win;

    if (firstwin == NULL) {
	firstwin = nw;
    }

    return win;
}


/*-----------------------------------------------------------------------
  Function:   deltxwin  - Delete the top-most window in window stack
  Arguments:  (none)
  Returns:    int       - OK if all well, ERR if not

  This function deletes the top-most window in the stack of windows
  managed by this module.  ERR is returned if there is no such window, or
  if delwin() fails.  Please note that the actual screen is NOT
  refreshed: a call to txrefresh() should follow this one.  This allows
  multiple windows to be deleted without screen flashing.
*/

int deltxwin (void)
{
    txwin_t *cur, *prev;
    int ret;


    if (topwin == NULL) {
	return ERR;
    }

    cur = topwin;
    prev = topwin->prev;
    topwin = prev;

    if (prev != NULL) {
	prev->next = NULL;
	curwin = prev->win;
    } else {
	firstwin = NULL;
	curwin = stdscr;
    }

    ret = delwin(cur->win);
    free(cur);

    return ret;
}


/*-----------------------------------------------------------------------
  Function:   delalltxwin  - Delete all windows in the window stack
  Arguments:  (none)
  Returns:    int          - OK is always returned

  This function deletes all windows in the stack of windows managed by
  this module.  After calling this function, the global variable "curwin"
  points to "stdscr", the only window for which output is now permitted.
  Please note that the screen is NOT refreshed; a call to txrefresh()
  should follow this one if appropriate.
*/

int delalltxwin (void)
{
    while (topwin != NULL) {
	deltxwin();
    }

    return OK;
}


/*-----------------------------------------------------------------------
  Function:   txrefresh  - Redraw all windows in the window stack
  Arguments:  (none)
  Returns:    int        - OK if all well, ERR if not

  This function redraws (refreshes) all windows in the stack of windows
  managed by this module.  Windows are refreshed from bottom (first) to
  top (last).  The result of doupdate() is returned.
*/

int txrefresh (void)
{
    txwin_t *p;


    touchwin(stdscr);
    wnoutrefresh(stdscr);

    for (p = firstwin; p != NULL; p = p->next) {
	touchwin(p->win);
	wnoutrefresh(p->win);
    }

    return doupdate();
}


/************************************************************************
*                            Output routines                            *
************************************************************************/

/*-----------------------------------------------------------------------
  Function:   center   - Centre a string on the current line
  Arguments:  win      - Window to use
              y        - Line on which to centre the string
              attr     - Window attributes to use for string
              format   - printf()-like format string
              ...      - printf()-like arguments
  Returns:    int      - Return code from wprintw()

  This function prints a string (formated with wprintw(format, ...)) in
  the centre of line y in the window win, using the window attributes in
  attr.  Please note that wrefresh() is NOT called.
*/

int center (WINDOW *win, int y, int attr, const char *format, ...)
{
    va_list args;

    int oldattr;
    int len, ret;
    int maxy, maxx;
    int fill;

    char *buf;


    buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit("out of memory");
    }

    oldattr = getbkgd(win) & ~A_CHARTEXT;
    wattrset(win, attr);

    va_start(args, format);
    len = vsnprintf(buf, BUFSIZE, format, args);
    va_end(args);
    if (len < 0) {
	return ERR;
    }

    getmaxyx(win, maxy, maxx);
    fill = (maxx - len) / 2;

    ret = mvwprintw(win, y, fill > 0 ? fill : 0, "%s", buf);

    wattrset(win, oldattr);

    free(buf);
    return ret;
}


/*-----------------------------------------------------------------------
  Function:   attrpr  - Print a string with special attributes
  Arguments:  win     - Window to use
              attr    - Attribute to use for the string
              format  - printf()-like format string
              ...     - printf()-like arguments
  Returns:    int     - Return code from wprintw()

  This function sets the window attributes to attr, then prints the given
  string (using wprintw()), then restores the previous window attributes.
*/

int attrpr (WINDOW *win, int attr, const char *format, ...)
{
    va_list args;
    int oldattr;
    int ret;


    oldattr = getbkgd(win) & ~A_CHARTEXT;
    wattrset(win, attr);

    va_start(args, format);
    ret = vwprintw(win, format, args);
    va_end(args);

    wattrset(win, oldattr);

    return ret;
}


/************************************************************************
*                            Input routines                             *
************************************************************************/

/*-----------------------------------------------------------------------
  Function:   gettxchar  - Read a keyboard character
  Arguments:  win        - Window to use
  Returns:    int        - Keyboard character

  This function reads a single character from the keyboard.  The key is
  NOT echoed to the screen and the cursor visibility is NOT affected.
*/

int gettxchar (WINDOW *win)
{
    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    return wgetch(win);
}


/*-----------------------------------------------------------------------
  Function:   gettxline   - Read a line from the keyboard (generic)
  Arguments:  win         - Window to use
              buf         - Pointer to preallocated buffer
	      bufsize     - Size of buffer
              multifield  - Allow <TAB>, etc, to move between fields
	      maxlen      - Maximum length of string
              emptyval    - String used if an empty string is input
	      defaultval  - Default string, used if KEY_DEFAULTVAL is
                            pressed as the first character
	      allowed     - Characters allowed in the string
              stripspc    - Strip leading and trailing spaces from string
	      y, x        - Start of field (line, col)
	      fieldsize   - Size of field in characters
	      attr        - Curses attribute to use for the field
	      modified    - Pointer to modified status
  Returns:    int         - Status code: OK, ERR or key code

  This low-level function draws an input field fieldsize characters long
  using attr as the window attribute, then reads a line of input from the
  keyboard (of no more than maxlen characters) and places it into the
  preallocated buffer buf[].  On entry, buf[] must contain the current
  input string: this allows for resumed editing of an input line.  On
  exit, buf[] will contain the input string; this string is printed using
  the original window attributes with A_BOLD added.  Many EMACS/Bash-
  style keyboard editing shortcuts are allowed.

  If ENTER (or equivalent) is pressed, OK is returned.  In this case,
  leading and trailing spaces are stripped if stripspc is true; if an
  empty string is entered, the string pointed to by emptyval (if not
  NULL) is stored in buf[].

  If CANCEL, ESC or ^G is pressed, ERR is returned.  In this case, buf[]
  contains the string as edited by the user: emptyval is NOT used, nor
  are leading and trailing spaces stripped.

  If multifield is true, the UP and DOWN arrow keys, as well as TAB,
  Shift-TAB, ^P (Previous) and ^N (Next) return KEY_UP or KEY_DOWN as
  appropriate.  As with CANCEL etc., emptyval is NOT used, nor are
  leading and trailing spaces stripped.

  In all of these cases, the boolean variable pointed to by modified (if
  not NULL) is set to true if the user actually modified the input line.

  If KEY_DEFAULTVAL is pressed when the input line is empty, the string
  pointed to by defaultval (if not NULL) is placed in the buffer as if
  typed by the user.  Editing is NOT terminated in this case.

  If allowed points to a string (ie, is not NULL), only characters in
  that string are allowed to be entered into the input line.  For
  example, if allowed points to "0123456789abcdefABCDEF", only those
  characters would be allowed (allowing a hexadecimal number to be
  entered).
*/

int gettxline (WINDOW *win, char *buf, int bufsize, bool multifield,
	       int maxlen, const char *emptyval, const char *defaultval,
	       const char *allowed, bool stripspc, int y, int x,
	       int fieldsize, int attr, bool *modified)
{
    int i, len, pos, cpos, nb;
    int oldattr;
    bool done, redraw, first, mod;
    char c;
    int key, key2, ret;


    assert(buf != NULL);
    assert(bufsize > 1);
    assert(maxlen > 0);
    assert(maxlen < bufsize);
    assert(fieldsize > 1);


    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    oldattr = getbkgd(win) & ~A_CHARTEXT;
    wattrset(win, attr & ~A_CHARTEXT);
    curs_set(CURS_ON);

    len = strlen(buf);
    pos = len;			// pos can be from 0 to strlen(buf)
    cpos = MIN(len, fieldsize - 1);
    redraw = true;
    done = false;
    mod = false;
    ret = OK;

    while (! done) {
	if (redraw) {
	    /*
	      Redisplay the visible part of the current input string.
	      Blanks at the end of the input area are replaced with
	      "attr", which may contain a '_' for non-colour mode.
	    */

	    mvwprintw(win, y, x, "%-*.*s", fieldsize, fieldsize,
		      buf + pos - cpos);

	    nb = (len - (pos - cpos) > fieldsize) ?
		0 : fieldsize - (len - (pos - cpos));
	    wmove(win, y, x + fieldsize - nb);
	    for (i = 0; i < nb; i++) {
		waddch(win, ((attr & A_CHARTEXT) == 0) ? attr | ' ' : attr);
	    }

	    wmove(win, y, x + cpos);
	    wrefresh(win);
	}

	key = wgetch(win);
	if (key == ERR) {
	    // Do nothing on ERR
	    ;
	} else if ((key == KEY_DEFAULTVAL) && (defaultval != NULL)
		   && (len == 0)) {
	    // Initialise buffer with the default value

	    strncpy(buf, defaultval, bufsize - 1);
	    buf[bufsize - 1] = '\0';
	    len = strlen(buf);
	    pos = len;
	    cpos = MIN(len, fieldsize - 1);
	    mod = true;
	    redraw = true;
	} else if ((key < 0400) && (! iscntrl(key))) {
	    if ((len >= maxlen) ||
		((allowed != NULL) && (strchr(allowed, key) == NULL))) {
		beep();
	    } else {
		// Process ordinary key press: insert it into the string

		memmove(buf + pos + 1, buf + pos, len - pos + 1);
		buf[pos] = (char) key;
		len++;
		pos++;
		if (cpos < fieldsize - 1) {
		    cpos++;
		}
		mod = true;
		redraw = true;
	    }
	} else {
	    switch (key) {

	    // Terminating keys

	    case KEY_RETURN:
	    case KEY_ENTER:
	    case KEY_CTRL('M'):
		// Finish entering the string

		if (stripspc) {
		    // Strip leading spaces
		    i = 0;
		    while ((i < len) && isspace(buf[i])) {
			i++;
		    }
		    if (i > 0) {
			memmove(buf, buf + i, len - i + 1);
			len -= i;
			mod = true;
		    }

		    // Strip trailing spaces
		    i = len;
		    while ((i > 0) && isspace(buf[i - 1])) {
			i--;
		    }
		    if (i < len) {
			buf[i] = '\0';
			len = i;
			mod = true;
		    }
		}

		if ((emptyval != NULL) && (len == 0)) {
		    strncpy(buf, emptyval, bufsize - 1);
		    buf[bufsize - 1] = '\0';
		    mod = true;
		}

		ret = OK;
		done = true;
		break;

	    case KEY_CANCEL:
	    case KEY_CTRL('G'):
	    // case KEY_CTRL('C'):
	    // case KEY_CTRL('\\'):
		// Cancel entering the string
		ret = ERR;
		done = true;
		break;

	    case KEY_UP:
	    case KEY_BTAB:
	    case KEY_CTRL('P'):
		// Finish entering the string, if multifield is true
		if (multifield) {
		    ret = KEY_UP;
		    done = true;
		} else {
		    beep();
		}
		break;

	    case KEY_DOWN:
	    case KEY_TAB:
	    case KEY_CTRL('N'):
		// Finish entering the string, if multifield is true
		if (multifield) {
		    ret = KEY_DOWN;
		    done = true;
		} else {
		    beep();
		}
		break;

	    // Cursor movement keys

	    case KEY_LEFT:
	    case KEY_CTRL('B'):
		// Move cursor back one character
		if (pos == 0) {
		    beep();
		} else {
		    pos--;
		    if (cpos > 0) {
			cpos--;
		    }
		    redraw = true;
		}
		break;

	    case KEY_RIGHT:
	    case KEY_CTRL('F'):
		// Move cursor forward one character
		if (pos == len) {
		    beep();
		} else {
		    pos++;
		    if (cpos < fieldsize - 1) {
			cpos++;
		    }
		    redraw = true;
		}
		break;

	    case KEY_HOME:
	    case KEY_CTRL('A'):
		// Move cursor to start of string
		pos = 0;
		cpos = 0;
		redraw = true;
		break;

	    case KEY_END:
	    case KEY_CTRL('E'):
		// Move cursor to end of string
		pos = len;
		cpos = MIN(pos, fieldsize - 1);
		redraw = true;
		break;

	    case KEY_CLEFT:
		// Move cursor to start of current or previous word
		while ((pos > 0) && (! isalnum(buf[pos - 1]))) {
		    pos--;
		    if (cpos > 0) {
			cpos--;
		    }
		}
		while ((pos > 0) && isalnum(buf[pos - 1])) {
		    pos--;
		    if (cpos > 0) {
			cpos--;
		    }
		}
		redraw = true;
		break;

	    case KEY_CRIGHT:
		// Move cursor to end of current or next word
		while ((pos < len) && (! isalnum(buf[pos]))) {
		    pos++;
		    if (cpos < fieldsize - 1) {
			cpos++;
		    }
		}
		while ((pos < len) && isalnum(buf[pos])) {
		    pos++;
		    if (cpos < fieldsize - 1) {
			cpos++;
		    }
		}
		redraw = true;
		break;

	    // Deletion keys

	    case KEY_BS:
	    case KEY_BACKSPACE:
	    case KEY_DEL:
		// Delete previous character
		if (pos == 0) {
		    beep();
		} else {
		    memmove(buf + pos - 1, buf + pos, len - pos + 1);
		    len--;
		    pos--;
		    if (cpos > 0) {
			cpos--;
		    }
		    mod = true;
		    redraw = true;
		}
		break;

	    case KEY_DC:
	    case KEY_CTRL('D'):
		// Delete character under cursor
		if (pos == len) {
		    beep();
		} else {
		    memmove(buf + pos, buf + pos + 1, len - pos);
		    len--;
		    // pos and cpos stay the same
		    mod = true;
		    redraw = true;
		}
		break;

	    case KEY_CLEAR:
		// Delete the entire line
		strcpy(buf, "");
		len = 0;
		pos = 0;
		cpos = 0;
		mod = true;
		redraw = true;
		break;

	    case KEY_CTRL('U'):
		// Delete backwards to the start of the line
		if (pos == 0) {
		    beep();
		} else {
		    memmove(buf, buf + pos, len - pos + 1);
		    len -= pos;
		    pos = 0;
		    cpos = 0;
		    mod = true;
		    redraw = true;
		}
		break;

	    case KEY_CTRL('K'):
		// Delete to the end of the line
		if (pos == len) {
		    beep();
		} else {
		    buf[pos] = '\0';
		    len = pos;
		    // pos and cpos stay the same
		    mod = true;
		    redraw = true;
		}
		break;

	    case KEY_CTRL('W'):
		// Delete the previous word
		if (pos == 0) {
		    beep();
		} else {
		    /*
		      Note the use of isspace() instead of isalnum():
		      this makes ^W follow GNU Bash standards, which
		      behaves differently from Meta-DEL.
		    */

		    i = pos;
		    while ((i > 0) && isspace(buf[i - 1])) {
			i--;
			if (cpos > 0) {
			    cpos--;
			}
		    }
		    while ((i > 0) && (! isspace(buf[i - 1]))) {
			i--;
			if (cpos > 0) {
			    cpos--;
			}
		    }

		    memmove(buf + i, buf + pos, len - pos + 1);
		    len -= (pos - i);
		    pos = i;
		    mod = true;
		    redraw = true;
		}
		break;

	    // Miscellaneous keys and events

	    case KEY_CTRL('T'):
		// Transpose characters
		if ((pos == 0) || (len <= 1)) {
		    beep();
		} else if (pos == len) {
		    c = buf[pos - 1];
		    buf[pos - 1] = buf[pos - 2];
		    buf[pos - 2] = c;
		    mod = true;
		    redraw = true;
		} else {
		    c = buf[pos];
		    buf[pos] = buf[pos - 1];
		    buf[pos - 1] = c;

		    pos++;
		    if (cpos < fieldsize - 1) {
			cpos++;
		    }
		    mod = true;
		    redraw = true;
		}
		break;

	    case KEY_ESC:
		// Handle Meta-X-style and other function key presses
		wtimeout(win, META_TIMEOUT);
		key2 = wgetch(win);

		if (key2 == ERR) {
		    // <ESC> by itself: cancel entering the string
		    ret = ERR;
		    done = true;
		} else if ((key2 == 'O') || (key2 == '[')) {
		    // Swallow any unknown VT100-style function keys
		    key2 = wgetch(win);
		    while ((key2 != ERR) && isascii(key2) &&
			   (strchr("0123456789;", key2) != NULL) &&
			   (strchr("~ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				   "abcdefghijklmnopqrstuvwxyz", key2) == NULL)) {
			key2 = wgetch(win);
		    }
		    beep();
		} else {
		    // Handle Meta-X-style keypress
		    switch (key2) {

		    // Cursor movement keys

		    case 'B':
		    case 'b':
			// Move cursor to start of current or previous word
			while ((pos > 0) && (! isalnum(buf[pos - 1]))) {
			    pos--;
			    if (cpos > 0) {
				cpos--;
			    }
			}
			while ((pos > 0) && isalnum(buf[pos - 1])) {
			    pos--;
			    if (cpos > 0) {
				cpos--;
			    }
			}
			redraw = true;
			break;

		    case 'F':
		    case 'f':
			// Move cursor to end of current or next word
			while ((pos < len) && (! isalnum(buf[pos]))) {
			    pos++;
			    if (cpos < fieldsize - 1) {
				cpos++;
			    }
			}
			while ((pos < len) && isalnum(buf[pos])) {
			    pos++;
			    if (cpos < fieldsize - 1) {
				cpos++;
			    }
			}
			redraw = true;
			break;

		    // Deletion keys

		    case KEY_BS:
		    case KEY_BACKSPACE:
		    case KEY_DEL:
			// Delete the previous word (different from ^W)
			i = pos;
			while ((i > 0) && (! isalnum(buf[i - 1]))) {
			    i--;
			    if (cpos > 0) {
				cpos--;
			    }
			}
			while ((i > 0) && isalnum(buf[i - 1])) {
			    i--;
			    if (cpos > 0) {
				cpos--;
			    }
			}

			memmove(buf + i, buf + pos, len - pos + 1);
			len -= (pos - i);
			pos = i;
			mod = true;
			redraw = true;
			break;

		    case 'D':
		    case 'd':
			// Delete the next word
			i = pos;
			while ((i < len) && (! isalnum(buf[i]))) {
			    i++;
			}
			while ((i < len) && isalnum(buf[i])) {
			    i++;
			}

			memmove(buf + pos, buf + i, len - i + 1);
			len -= (i - pos);
			// pos and cpos stay the same
			mod = true;
			redraw = true;
			break;

		    case '\\':
		    case ' ':
			// Delete all surrounding spaces; if key2 == ' ',
			// also insert one space
			i = pos;
			while ((pos > 0) && isspace(buf[pos - 1])) {
			    pos--;
			    if (cpos > 0) {
				cpos--;
			    }
			}
			while ((i < len) && isspace(buf[i])) {
			    i++;
			}

			memmove(buf + pos, buf + i, len - i + 1);
			len -= (i - pos);

			if (key2 == ' ') {
			    if ((len >= maxlen) || ((allowed != NULL) &&
				(strchr(allowed, key) == NULL))) {
				beep();
			    } else {
				memmove(buf + pos + 1, buf + pos, len - pos + 1);
				buf[pos] = ' ';
				len++;
				pos++;
				if (cpos < fieldsize - 1) {
				    cpos++;
				}
			    }
			}

			mod = true;
			redraw = true;

			break;

		    // Transformation keys

		    case 'U':
		    case 'u':
			// Convert word (from cursor onwards) to upper case
			while ((pos < len) && (! isalnum(buf[pos]))) {
			    pos++;
			    if (cpos < fieldsize - 1) {
				cpos++;
			    }
			}
			while ((pos < len) && isalnum(buf[pos])) {
			    buf[pos] = toupper(buf[pos]);
			    pos++;
			    if (cpos < fieldsize - 1) {
				cpos++;
			    }
			}
			mod = true;
			redraw = true;
			break;

		    case 'L':
		    case 'l':
			// Convert word (from cursor onwards) to lower case
			while ((pos < len) && (! isalnum(buf[pos]))) {
			    pos++;
			    if (cpos < fieldsize - 1) {
				cpos++;
			    }
			}
			while ((pos < len) && isalnum(buf[pos])) {
			    buf[pos] = tolower(buf[pos]);
			    pos++;
			    if (cpos < fieldsize - 1) {
				cpos++;
			    }
			}
			mod = true;
			redraw = true;
			break;

		    case 'C':
		    case 'c':
			// Convert current letter to upper case, following
			// letters to lower case
			first = true;
			while ((pos < len) && (! isalnum(buf[pos]))) {
			    pos++;
			    if (cpos < fieldsize - 1) {
				cpos++;
			    }
			}
			while ((pos < len) && isalnum(buf[pos])) {
			    if (first) {
				buf[pos] = toupper(buf[pos]);
				first = false;
			    } else {
				buf[pos] = tolower(buf[pos]);
			    }
			    pos++;
			    if (cpos < fieldsize - 1) {
				cpos++;
			    }
			}
			mod = true;
			redraw = true;
			break;

		    // Miscellaneous keys and events

		    case KEY_RESIZE:
		    case KEY_EVENT:
			ret = key;
			done = true;
			break;

		    default:
			beep();
			break;
		    }
		}

		wtimeout(win, -1);
		break;

	    case KEY_RESIZE:
	    case KEY_EVENT:
		ret = key;
		done = true;
		break;

	    default:
		beep();
		break;
	    }
	}
    }

    wattrset(win, oldattr);
    curs_set(CURS_OFF);

    wattron(win, A_BOLD);
    mvwprintw(win, y, x, "%-*.*s", fieldsize, fieldsize, buf);
    wattrset(win, oldattr);
    wrefresh(win);

    if (modified != NULL) {
	*modified = mod;
    }
    return ret;
}


/*-----------------------------------------------------------------------
  Function:   gettxstring  - Read a string from the keyboard
  Arguments:  win          - Window to use
              bufptr       - Address of pointer to buffer
              multifield   - Allow <TAB>, etc, to move between fields
	      y, x         - Start of field (line, col)
	      fieldsize    - Size of field in characters
	      attr         - Curses attribute to use for the field
	      modified     - Pointer to modified status
  Returns:    int          - Status code: OK, ERR or key code

  This function calls gettxline() to allow the user to input a string.
  Empty strings are specified as the default and the empty values, all
  characters are allowed, leading and trailing spaces are automatically
  stripped.  On entry, the pointer to buffer must either be NULL or
  previously allocated with a call to gettxstring().  If *bufptr is NULL,
  a buffer of BUFSIZE is automatically allocated using malloc(); this
  buffer is used to store the input line.  The same exit codes are
  returned as returned by gettxline().
*/

int gettxstring (WINDOW *win, char **bufptr, bool multifield, int y, int x,
		 int fieldsize, int attr, bool *modified)
{
    assert(bufptr != NULL);


    // Allocate the result buffer if needed
    if (*bufptr == NULL) {
	*bufptr = malloc(BUFSIZE);
	if (*bufptr == NULL) {
	    err_exit("out of memory");
	}

	**bufptr = '\0';
    }

    return gettxline(win, *bufptr, BUFSIZE, multifield, BUFSIZE - 1, "", "",
		     NULL, true, y, x, fieldsize, attr, modified);
}


/*-----------------------------------------------------------------------
  Function:   answer_yesno  - Read a Yes/No answer and return true/false
  Arguments:  win           - Window to use
  Returns:    bool          - true if Yes ("Y") was selected, else false

  This function waits for either "Y" or "N" to be pressed on the
  keyboard.  If "Y" was pressed, "Yes." is printed and true is returned.
  If "N" was pressed, "No." is printed and false is returned.  Note that
  the cursor becomes invisible after this function.
*/

bool answer_yesno (WINDOW *win)
{
    int key, oldattr;
    bool ok;


    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    oldattr = getbkgd(win) & ~A_CHARTEXT;
    wattron(win, A_BOLD);
    curs_set(CURS_ON);

    do {
	key = toupper(wgetch(win));
	ok = ((key == 'Y') || (key == 'N'));

	if (! ok) {
	    beep();
	}
    } while (! ok);

    curs_set(CURS_OFF);

    if (key == 'Y') {
	waddstr(win, "Yes");
    } else {
	waddstr(win, "No");
    }

    wattrset(win, oldattr);
    wrefresh(win);
    return (key == 'Y');
}


/*-----------------------------------------------------------------------
  Function:   wait_for_key  - Print a message and wait for any key
  Arguments:  win           - Window to use
              y             - Line on which to print message
  Returns:    (nothing)

  This function prints a message, then waits for any key to be pressed.
*/

void wait_for_key (WINDOW *win, int y)
{
    int key;


    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    curs_set(CURS_OFF);
    center(win, y, ATTR_WAITFORKEY_STR, "[ Press <SPACE> to continue ] ");
    wrefresh(win);

    key = wgetch(win);
}
