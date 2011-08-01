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


#include "trader.h"


/************************************************************************
*            Module-specific constants and type declarations            *
************************************************************************/

typedef struct txwin {
    WINDOW		*win;		// Pointer to window structure
    struct txwin	*next;		// Next window in stack
    struct txwin	*prev;		// Previous window in stack
} txwin_t;


/************************************************************************
*                      Global variable definitions                      *
************************************************************************/

WINDOW *curwin = NULL;		// Top-most (current) window


// Character renditions (attributes) used by Star Traders

chtype attr_root_window;	// Root window (behind all others)
chtype attr_game_title;		// One-line game title at top

chtype attr_normal_window;	// Normal window background
chtype attr_title;		// Normal window title
chtype attr_subtitle;		// Normal window subtitle
chtype attr_normal;		// Normal window text
chtype attr_highlight;		// Normal window highlighted string
chtype attr_blink;		// Blinking text in normal window
chtype attr_keycode;		// Make keycodes like <1> stand out
chtype attr_choice;		// Make map/company choices stand out
chtype attr_input_field;	// Background for input text field
chtype attr_waitforkey;		// "Press any key", normal window

chtype attr_map_window;		// Map window background
chtype attr_mapwin_title;	// Map window title (player name, turn)
chtype attr_mapwin_highlight;	// Map window title highlight
chtype attr_mapwin_blink;	// Map window title blinking text
chtype attr_map_empty;		// On map, empty space
chtype attr_map_outpost;	// On map, outpost
chtype attr_map_star;		// On map, star
chtype attr_map_company;	// On map, company
chtype attr_map_choice;		// On map, a choice of moves

chtype attr_status_window;	// Status window background

chtype attr_error_window;	// Error message window background
chtype attr_error_title;	// Error window title
chtype attr_error_normal;	// Error window ordinary text
chtype attr_error_highlight;	// Error window highlighted string
chtype attr_error_waitforkey;	// "Press any key", error window


/************************************************************************
*                       Module-specific variables                       *
************************************************************************/

txwin_t *topwin   = NULL;	// Top-most txwin structure
txwin_t *firstwin = NULL;	// First (bottom-most) txwin structure


/************************************************************************
*                  Module-specific function prototypes                  *
************************************************************************/

/*
  Function:   txinput_fixup - Copy strings with fixup
  Parameters: dest          - Destination buffer of size BUFSIZE
              src           - Source buffer of size BUFSIZE
              isfloat       - True if src contains a floating point number

  This helper function copies the string in src to dest, performing
  certain fixups along the way.  In particular, thousands separators are
  removed and (if isfloat is true) the monetary radix (decimal point) is
  replaced by the normal one.

  This function is used by gettxdouble() and gettxlong() to share some
  common code.
*/
static void txinput_fixup (char *restrict dest, char *restrict src,
			   bool isfloat);


/************************************************************************
*             Basic text input/output function definitions              *
************************************************************************/

// These functions are documented in the file "intf.h"


/***********************************************************************/
// init_screen: Initialise the screen (terminal display)

void init_screen (void)
{
    initscr();

    if (COLS < MIN_COLS || LINES < MIN_LINES) {
	err_exit(_("terminal size is too small (%d x %d required)"),
		 MIN_COLS, MIN_LINES);
    }

    // Initialise variables controlling the stack of windows
    curwin = stdscr;
    topwin = NULL;
    firstwin = NULL;

    noecho();
    curs_set(CURS_OFF);
    raw();

    // Initialise all character renditions used in the game
    if (! option_no_color && has_colors()) {
	start_color();

	init_pair(1,  COLOR_BLACK,  COLOR_WHITE);
	init_pair(2,  COLOR_BLUE,   COLOR_BLACK);
	init_pair(3,  COLOR_GREEN,  COLOR_BLACK);
	init_pair(4,  COLOR_CYAN,   COLOR_BLUE);
	init_pair(5,  COLOR_RED,    COLOR_BLACK);
	init_pair(6,  COLOR_YELLOW, COLOR_BLACK);
	init_pair(7,  COLOR_YELLOW, COLOR_BLUE);
	init_pair(8,  COLOR_YELLOW, COLOR_CYAN);
	init_pair(9,  COLOR_WHITE,  COLOR_BLACK);
	init_pair(10, COLOR_WHITE,  COLOR_BLUE);
	init_pair(11, COLOR_WHITE,  COLOR_RED);

	attr_root_window      = COLOR_PAIR(9);
	attr_game_title	      = COLOR_PAIR(8)   | A_BOLD;

	attr_normal_window    = COLOR_PAIR(10);
	attr_title	      = COLOR_PAIR(6)   | A_BOLD;
	attr_subtitle	      = COLOR_PAIR(9);
	attr_normal	      = attr_normal_window;
	attr_highlight	      = COLOR_PAIR(7)   | A_BOLD;
	attr_blink	      = COLOR_PAIR(7)   | A_BOLD  | A_BLINK;
	attr_keycode	      = COLOR_PAIR(6)   | A_BOLD;
	attr_choice	      = COLOR_PAIR(11)  | A_BOLD;
	attr_input_field      = COLOR_PAIR(9);
	attr_waitforkey	      = COLOR_PAIR(4);

	attr_map_window	      = COLOR_PAIR(9);
	attr_mapwin_title     = COLOR_PAIR(10);
	attr_mapwin_highlight = COLOR_PAIR(7)   | A_BOLD;
	attr_mapwin_blink     = COLOR_PAIR(7)   | A_BOLD  | A_BLINK;
	attr_map_empty	      = COLOR_PAIR(2)   | A_BOLD;
	attr_map_outpost      = COLOR_PAIR(3)   | A_BOLD;
	attr_map_star	      = COLOR_PAIR(6)   | A_BOLD;
	attr_map_company      = COLOR_PAIR(5)   | A_BOLD;
	attr_map_choice	      = COLOR_PAIR(11)  | A_BOLD;

	attr_status_window    = COLOR_PAIR(1);

	attr_error_window     = COLOR_PAIR(11);
	attr_error_title      = COLOR_PAIR(6)   | A_BOLD;
	attr_error_normal     = attr_error_window;
	attr_error_highlight  = COLOR_PAIR(11)  | A_BOLD;
	attr_error_waitforkey = COLOR_PAIR(11);

    } else {
	// No colour is to be used

	attr_root_window      = A_NORMAL;
	attr_game_title	      = A_REVERSE | A_BOLD;

	attr_normal_window    = A_NORMAL;
	attr_title	      = A_REVERSE;
	attr_subtitle	      = A_REVERSE;
	attr_normal	      = attr_normal_window;
	attr_highlight	      = A_BOLD;
	attr_blink	      = A_BOLD | A_BLINK;
	attr_keycode	      = A_REVERSE;
	attr_choice	      = A_REVERSE;
	attr_input_field      = A_BOLD | '_';
	attr_waitforkey	      = A_NORMAL;

	attr_map_window	      = A_NORMAL;
	attr_mapwin_title     = A_NORMAL;
	attr_mapwin_highlight = A_BOLD;
	attr_mapwin_blink     = A_BOLD | A_BLINK;
	attr_map_empty	      = A_NORMAL;
	attr_map_outpost      = A_NORMAL;
	attr_map_star	      = A_BOLD;
	attr_map_company      = A_BOLD;
	attr_map_choice	      = A_REVERSE;

	attr_status_window    = A_REVERSE;

	attr_error_window     = A_REVERSE;
	attr_error_title      = A_BOLD;
	attr_error_normal     = attr_error_window;
	attr_error_highlight  = A_REVERSE;
	attr_error_waitforkey = A_REVERSE;
    }

    bkgd(attr_root_window);
    clear();

    move(0, 0);
    for (int i = 0; i < COLS; i++) {
	addch(attr_game_title | ' ');
    }

    center(stdscr, 0, attr_game_title, "Star Traders");

    attrset(attr_root_window);
    refresh();
}


/***********************************************************************/
// end_screen: Deinitialise the screen (terminal display)

void end_screen (void)
{
    delalltxwin();

    curs_set(CURS_ON);
    clear();
    refresh();
    endwin();

    curwin = NULL;
    topwin = NULL;
    firstwin = NULL;
}


/***********************************************************************/
// newtxwin: Create a new window, inserted into window stack

WINDOW *newtxwin (int nlines, int ncols, int begin_y, int begin_x,
		  bool dofill, chtype bkgd_attr)
{
    WINDOW *win;
    txwin_t *nw;


    // Centre the window, if required
    if (begin_y == WCENTER) {
	begin_y = (nlines == 0) ? 0 : (LINES - nlines) / 2;
    }
    if (begin_x == WCENTER) {
	begin_x = (ncols == 0) ? 0 : (COLS - ncols) / 2;
    }

    // Create the new window

    win = newwin(nlines, ncols, begin_y, begin_x);
    if (win == NULL) {
	err_exit_nomem();
    }

    nw = malloc(sizeof(txwin_t));
    if (nw == NULL) {
	err_exit_nomem();
    }

    // Insert the new window into the txwin stack

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

    // Paint the background and border, if required

    if (dofill) {
	wbkgd(win, bkgd_attr);
	box(win, 0, 0);
    }

    return win;
}


/***********************************************************************/
// deltxwin: Delete the top-most window in the window stack

int deltxwin (void)
{
    txwin_t *cur, *prev;
    int ret;


    if (topwin == NULL) {
	return ERR;
    }

    // Remove window from the txwin stack

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


/***********************************************************************/
// delalltxwin: Delete all windows in the window stack

int delalltxwin (void)
{
    while (topwin != NULL) {
	deltxwin();
    }

    return OK;
}


/***********************************************************************/
// txrefresh: Redraw all windows in the window stack

int txrefresh (void)
{
    touchwin(stdscr);
    wnoutrefresh(stdscr);

    for (txwin_t *p = firstwin; p != NULL; p = p->next) {
	touchwin(p->win);
	wnoutrefresh(p->win);
    }

    return doupdate();
}


/***********************************************************************/
// attrpr: Print a string with a particular character rendition

int attrpr (WINDOW *win, chtype attr, const char *restrict format, ...)
{
    va_list args;
    int ret;


    assert(win != NULL);
    assert(format != NULL);

    chtype oldattr = getattrs(win);
    chtype oldbkgd = getbkgd(win);

    /* Note that wattrset() will override parts of wbkgdset() and vice
       versa: don't swap the order of these two lines! */
    wbkgdset(win, (oldbkgd & A_COLOR) | A_NORMAL);
    wattrset(win, attr);

    va_start(args, format);
    ret = vw_printw(win, format, args);
    va_end(args);

    wbkgdset(win, oldbkgd);
    wattrset(win, oldattr);

    return ret;
}


/***********************************************************************/
// center: Centre a string in a given window

int center (WINDOW *win, int y, chtype attr, const char *restrict format, ...)
{
    va_list args;
    int ret, len, x;
    char *buf;


    assert(win != NULL);
    assert(format != NULL);

    buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit_nomem();
    }

    chtype oldattr = getattrs(win);
    chtype oldbkgd = getbkgd(win);

    // Order is important: see attrpr()
    wbkgdset(win, (oldbkgd & A_COLOR) | A_NORMAL);
    wattrset(win, attr);

    va_start(args, format);
    len = vsnprintf(buf, BUFSIZE, format, args);
    va_end(args);

    if (len < 0) {
	ret = ERR;
    } else if (len == 0) {
	ret = OK;
    } else {
	x = (getmaxx(win) - len) / 2;
	ret = mvwprintw(win, y, MAX(x, 2), "%1.*s", getmaxx(win) - 4, buf);
    }

    wbkgdset(win, oldbkgd);
    wattrset(win, oldattr);

    free(buf);
    return ret;
}


/***********************************************************************/
// center2: Centre two strings in a given window

int center2 (WINDOW *win, int y, chtype attr1, chtype attr2,
	     const char *initial, const char *restrict format, ...)
{
    va_list args;
    int ret, len1, len2, x;
    char *buf;


    assert(win != NULL);
    assert(initial != NULL);
    assert(format != NULL);

    buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit_nomem();
    }

    chtype oldattr = getattrs(win);
    chtype oldbkgd = getbkgd(win);

    wbkgdset(win, (oldbkgd & A_COLOR) | A_NORMAL);

    len1 = strlen(initial);

    va_start(args, format);
    len2 = vsnprintf(buf, BUFSIZE, format, args);
    va_end(args);

    if (len2 < 0) {
	ret = ERR;
    } else if (len1 + len2 == 0) {
	ret = OK;
    } else {
	x = (getmaxx(win) - (len1 + len2)) / 2;
	wattrset(win, attr1);
	mvwprintw(win, y, MAX(x, 2), "%s", initial);
	wattrset(win, attr2);
	ret = wprintw(win, "%1.*s", getmaxx(win) - len1 - 4, buf);
    }

    wbkgdset(win, oldbkgd);
    wattrset(win, oldattr);

    free(buf);
    return ret;
}


/***********************************************************************/
// center3: Centre three strings in a given window

int center3 (WINDOW *win, int y, chtype attr1, chtype attr3, chtype attr2,
	     const char *initial, const char *final,
	     const char *restrict format, ...)
{
    va_list args;
    int len1, len2, len3;
    int ret, x;
    char *buf;


    assert(win != NULL);
    assert(initial != NULL);
    assert(final != NULL);
    assert(format != NULL);

    buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit_nomem();
    }

    chtype oldattr = getattrs(win);
    chtype oldbkgd = getbkgd(win);

    wbkgdset(win, (oldbkgd & A_COLOR) | A_NORMAL);

    len1 = strlen(initial);
    len3 = strlen(final);

    va_start(args, format);
    len2 = vsnprintf(buf, BUFSIZE, format, args);
    va_end(args);

    if (len2 < 0) {
	ret = ERR;
    } else if (len1 + len2 + len3 == 0) {
	ret = OK;
    } else {
	x = (getmaxx(win) - (len1 + len2 + len3)) / 2;
	wattrset(win, attr1);
	mvwprintw(win, y, MAX(x, 2), "%s", initial);
	wattrset(win, attr2);
	ret = wprintw(win, "%1.*s", getmaxx(win) - len1 - len3 - 4, buf);
	wattrset(win, attr3);
	wprintw(win, "%s", final);
    }

    wbkgdset(win, oldbkgd);
    wattrset(win, oldattr);

    free(buf);
    return ret;
}


/***********************************************************************/
// gettxchar: Read a character from the keyboard

int gettxchar (WINDOW *win)
{
    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    return wgetch(win);
}


/***********************************************************************/
// gettxline: Read a line from the keyboard (low-level)

int gettxline (WINDOW *win, char *buf, int bufsize, bool *restrict modified,
	       bool multifield, const char *emptyval, const char *defaultval,
	       const char *allowed, bool stripspc, int y, int x, int width,
	       chtype attr)
{
    int len, pos, cpos, nb, ret;
    bool done, redraw, mod;
    int key, key2;


    assert(win != NULL);
    assert(buf != NULL);
    assert(bufsize > 2);
    assert(width > 1);

    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    chtype oldattr = getattrs(win);
    chtype oldbkgd = getbkgd(win);

    /* Note that wattrset() will override parts of wbkgdset() and vice
       versa: don't swap the order of these two lines! */
    wbkgdset(win, (oldbkgd & A_COLOR) | A_NORMAL);
    wattrset(win, attr & ~A_CHARTEXT);
    curs_set(CURS_ON);

    len = strlen(buf);
    pos = len;			// pos (string position) is from 0 to len
    cpos = MIN(len, width - 1);	// cpos (cursor position) is from 0 to width-1
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
	    mvwprintw(win, y, x, "%-*.*s", width, width, buf + pos - cpos);

	    // nb = number of blanks
	    nb = (len - (pos - cpos) > width) ? 0 : width - (len - (pos - cpos));
	    wmove(win, y, x + width - nb);

	    chtype ch = ((attr & A_CHARTEXT) == 0) ? attr | ' ' : attr;
	    for (int i = 0; i < nb; i++) {
		waddch(win, ch);
	    }

	    wmove(win, y, x + cpos);
	    wrefresh(win);
	}

	key = wgetch(win);
	if (key == ERR) {
	    // Do nothing on ERR
	    ;
	} else if ((key == KEY_DEFAULTVAL1 || key == KEY_DEFAULTVAL2)
		   && defaultval != NULL && len == 0) {
	    // Initialise buffer with the default value

	    strncpy(buf, defaultval, bufsize - 1);
	    buf[bufsize - 1] = '\0';

	    len = strlen(buf);
	    pos = len;
	    cpos = MIN(len, width - 1);
	    mod = true;
	    redraw = true;
	} else if (key < 0400 && ! iscntrl(key)) {
	    if (len >= bufsize - 1
		|| (allowed != NULL && strchr(allowed, key) == NULL)) {
		beep();
	    } else {
		// Process ordinary key press: insert it into the string

		memmove(buf + pos + 1, buf + pos, len - pos + 1);
		buf[pos] = (char) key;
		len++;
		pos++;
		if (cpos < width - 1) {
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
		    int i;

		    for (i = 0; i < len && isspace(buf[i]); i++)
			;
		    if (i > 0) {
			memmove(buf, buf + i, len - i + 1);
			len -= i;
			mod = true;
		    }

		    // Strip trailing spaces
		    for (i = len; i > 0 && isspace(buf[i - 1]); i--)
			;
		    if (i < len) {
			buf[i] = '\0';
			len = i;
			mod = true;
		    }
		}

		if (emptyval != NULL && len == 0) {
		    strncpy(buf, emptyval, bufsize - 1);
		    buf[bufsize - 1] = '\0';
		    mod = true;
		}

		ret = OK;
		done = true;
		break;

	    case KEY_CANCEL:
	    case KEY_EXIT:
	    case KEY_CTRL('G'):
	    case KEY_CTRL('C'):
	    case KEY_CTRL('\\'):
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
		    if (cpos < width - 1) {
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
		cpos = MIN(pos, width - 1);
		redraw = true;
		break;

	    case KEY_CLEFT:
		// Move cursor to start of current or previous word
		while (pos > 0 && ! isalnum(buf[pos - 1])) {
		    pos--;
		    if (cpos > 0) {
			cpos--;
		    }
		}
		while (pos > 0 && isalnum(buf[pos - 1])) {
		    pos--;
		    if (cpos > 0) {
			cpos--;
		    }
		}
		redraw = true;
		break;

	    case KEY_CRIGHT:
		// Move cursor to end of current or next word
		while (pos < len && ! isalnum(buf[pos])) {
		    pos++;
		    if (cpos < width - 1) {
			cpos++;
		    }
		}
		while (pos < len && isalnum(buf[pos])) {
		    pos++;
		    if (cpos < width - 1) {
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
		    int i = pos;
		    while (i > 0 && isspace(buf[i - 1])) {
			i--;
			if (cpos > 0) {
			    cpos--;
			}
		    }
		    while (i > 0 && ! isspace(buf[i - 1])) {
			i--;
			if (cpos > 0) {
			    cpos--;
			}
		    }

		    memmove(buf + i, buf + pos, len - pos + 1);
		    len -= pos - i;
		    pos = i;
		    mod = true;
		    redraw = true;
		}
		break;

	    // Miscellaneous keys and events

	    case KEY_CTRL('T'):
		// Transpose characters
		if (pos == 0 || len <= 1) {
		    beep();
		} else if (pos == len) {
		    char c = buf[pos - 1];
		    buf[pos - 1] = buf[pos - 2];
		    buf[pos - 2] = c;
		    mod = true;
		    redraw = true;
		} else {
		    char c = buf[pos];
		    buf[pos] = buf[pos - 1];
		    buf[pos - 1] = c;

		    pos++;
		    if (cpos < width - 1) {
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
		} else if (key2 == 'O' || key2 == '[') {
		    // Swallow any unknown VT100-style function keys
		    key2 = wgetch(win);
		    while (key2 != ERR && isascii(key2)
			   && strchr("0123456789;", key2) != NULL
			   && strchr("~ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				     "abcdefghijklmnopqrstuvwxyz", key2)
			   == NULL) {
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
			while (pos > 0 && ! isalnum(buf[pos - 1])) {
			    pos--;
			    if (cpos > 0) {
				cpos--;
			    }
			}
			while (pos > 0 && isalnum(buf[pos - 1])) {
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
			while (pos < len && ! isalnum(buf[pos])) {
			    pos++;
			    if (cpos < width - 1) {
				cpos++;
			    }
			}
			while (pos < len && isalnum(buf[pos])) {
			    pos++;
			    if (cpos < width - 1) {
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
			{
			    int i = pos;
			    while (i > 0 && ! isalnum(buf[i - 1])) {
				i--;
				if (cpos > 0) {
				    cpos--;
				}
			    }
			    while (i > 0 && isalnum(buf[i - 1])) {
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

		    case 'D':
		    case 'd':
			// Delete the next word
			{
			    int i = pos;
			    while (i < len && ! isalnum(buf[i])) {
				i++;
			    }
			    while (i < len && isalnum(buf[i])) {
				i++;
			    }

			    memmove(buf + pos, buf + i, len - i + 1);
			    len -= (i - pos);
			    // pos and cpos stay the same
			    mod = true;
			    redraw = true;
			}
			break;

		    case '\\':
		    case ' ':
			// Delete all surrounding spaces; if key2 == ' ',
			// also insert one space
			{
			    int i = pos;
			    while (pos > 0 && isspace(buf[pos - 1])) {
				pos--;
				if (cpos > 0) {
				    cpos--;
				}
			    }
			    while (i < len && isspace(buf[i])) {
				i++;
			    }

			    memmove(buf + pos, buf + i, len - i + 1);
			    len -= (i - pos);

			    if (key2 == ' ') {
				if (len >= bufsize - 1 || (allowed != NULL
				    && strchr(allowed, key) == NULL)) {
				    beep();
				} else {
				    memmove(buf + pos + 1, buf + pos,
					    len - pos + 1);
				    buf[pos] = ' ';
				    len++;
				    pos++;
				    if (cpos < width - 1) {
					cpos++;
				    }
				}
			    }

			    mod = true;
			    redraw = true;
			}
			break;

		    // Transformation keys

		    case 'U':
		    case 'u':
			// Convert word (from cursor onwards) to upper case
			while (pos < len && ! isalnum(buf[pos])) {
			    pos++;
			    if (cpos < width - 1) {
				cpos++;
			    }
			}
			while (pos < len && isalnum(buf[pos])) {
			    buf[pos] = toupper(buf[pos]);
			    pos++;
			    if (cpos < width - 1) {
				cpos++;
			    }
			}
			mod = true;
			redraw = true;
			break;

		    case 'L':
		    case 'l':
			// Convert word (from cursor onwards) to lower case
			while (pos < len && ! isalnum(buf[pos])) {
			    pos++;
			    if (cpos < width - 1) {
				cpos++;
			    }
			}
			while (pos < len && isalnum(buf[pos])) {
			    buf[pos] = tolower(buf[pos]);
			    pos++;
			    if (cpos < width - 1) {
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
			{
			    bool first = true;
			    while (pos < len && ! isalnum(buf[pos])) {
				pos++;
				if (cpos < width - 1) {
				    cpos++;
				}
			    }
			    while (pos < len && isalnum(buf[pos])) {
				if (first) {
				    buf[pos] = toupper(buf[pos]);
				    first = false;
				} else {
				    buf[pos] = tolower(buf[pos]);
				}
				pos++;
				if (cpos < width - 1) {
				    cpos++;
				}
			    }
			    mod = true;
			    redraw = true;
			}
			break;

		    // Miscellaneous keys and events

		    case KEY_RESIZE:
		    case KEY_EVENT:
			ret = key;
			done = true;
			break;

		    default:
			beep();
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
	    }
	}
    }

    curs_set(CURS_OFF);

    wattrset(win, oldattr | A_BOLD);
    mvwprintw(win, y, x, "%-*.*s", width, width, buf);

    wbkgdset(win, oldbkgd);
    wattrset(win, oldattr);
    wrefresh(win);

    if (modified != NULL) {
	*modified = mod;
    }
    return ret;
}


/***********************************************************************/
// gettxstr: Read a string from the keyboard

int gettxstr (WINDOW *win, char **bufptr, bool *restrict modified,
	      bool multifield, int y, int x, int width, chtype attr)
{
    assert(bufptr != NULL);


    // Allocate the result buffer if needed
    if (*bufptr == NULL) {
	*bufptr = malloc(BUFSIZE);
	if (*bufptr == NULL) {
	    err_exit_nomem();
	}

	**bufptr = '\0';
    }

    return gettxline(win, *bufptr, BUFSIZE, modified, multifield, "", "",
		     NULL, true, y, x, width, attr);
}


/***********************************************************************/
// txinput_fixup: Copy strings with fixup

void txinput_fixup (char *restrict dest, char *restrict src, bool isfloat)
{
    struct lconv *lc = &lconvinfo;
    char *p;


    assert(src != NULL);
    assert(dest != NULL);

    strncpy(dest, src, BUFSIZE - 1);
    dest[BUFSIZE - 1] = '\0';

    // Replace mon_decimal_point with decimal_point if these are different
    if (isfloat) {
	if (strcmp(lc->mon_decimal_point, "") != 0
	    && strcmp(lc->decimal_point, "") != 0
	    && strcmp(lc->mon_decimal_point, lc->decimal_point) != 0) {
	    while ((p = strstr(dest, lc->mon_decimal_point)) != NULL) {
		char *pn;
		int len1 = strlen(lc->mon_decimal_point);
		int len2 = strlen(lc->decimal_point);

		// Make space for lc->decimal_point, if needed
		memmove(p + len2, p + len1, strlen(p) - (len2 - len1) + 1);

		// Copy lc->decimal_point over p WITHOUT copying ending NUL
		for (pn = lc->decimal_point; *pn != '\0'; pn++, p++) {
		    *p = *pn;
		}
	    }
	}
    }

    // Remove thousands separators if required
    if (strcmp(lc->thousands_sep, "") != 0) {
	while ((p = strstr(dest, lc->thousands_sep)) != NULL) {
	    int len = strlen(lc->thousands_sep);
	    memmove(p, p + len, strlen(p) - len + 1);
	}
    }
    if (strcmp(lc->mon_thousands_sep, "") != 0) {
	while ((p = strstr(dest, lc->mon_thousands_sep)) != NULL) {
	    int len = strlen(lc->thousands_sep);
	    memmove(p, p + len, strlen(p) - len + 1);
	}
    }
}


/***********************************************************************/
// gettxdouble: Read a floating-point number from the keyboard

int gettxdouble (WINDOW *win, double *restrict result, double min,
		 double max, double emptyval, double defaultval,
		 int y, int x, int width, chtype attr)
{
    struct lconv *lc = &lconvinfo;

    char *buf, *bufcopy;
    char *allowed, *emptystr, *defaultstr;
    double val;
    bool done;
    int ret;


    assert(result != NULL);
    assert(min <= max);

    buf        = malloc(BUFSIZE);
    bufcopy    = malloc(BUFSIZE);
    allowed    = malloc(BUFSIZE);
    emptystr   = malloc(BUFSIZE);
    defaultstr = malloc(BUFSIZE);

    if (buf == NULL || bufcopy == NULL || allowed == NULL
	|| emptystr == NULL || defaultstr == NULL) {
	err_exit_nomem();
    }

    *buf = '\0';

    strcpy(allowed,  "0123456789+-Ee");
    strncat(allowed, lc->decimal_point,     BUFSIZE - strlen(allowed) - 1);
    strncat(allowed, lc->thousands_sep,     BUFSIZE - strlen(allowed) - 1);
    strncat(allowed, lc->mon_decimal_point, BUFSIZE - strlen(allowed) - 1);
    strncat(allowed, lc->mon_thousands_sep, BUFSIZE - strlen(allowed) - 1);

    snprintf(emptystr,   BUFSIZE, "%'1.*f", lc->frac_digits, emptyval);
    snprintf(defaultstr, BUFSIZE, "%'1.*f", lc->frac_digits, defaultval);

    done = false;
    while (! done) {
	ret = gettxline(win, buf, BUFSIZE, NULL, false, emptystr, defaultstr,
			allowed, true, y, x, width, attr);

	if (ret == OK) {
	    char *p;

	    txinput_fixup(bufcopy, buf, true);
	    val = strtod(bufcopy, &p);

	    if (*p == '\0' && val >= min && val <= max) {
		*result = val;
		done = true;
	    } else {
		beep();
	    }
	} else {
	    done = true;
	}
    }

    free(defaultstr);
    free(emptystr);
    free(allowed);
    free(bufcopy);
    free(buf);

    return ret;
}


/***********************************************************************/
// gettxlong: Read an integer number from the keyboard

int gettxlong (WINDOW *win, long int *restrict result, long int min,
	       long int max, long int emptyval, long int defaultval,
	       int y, int x, int width, chtype attr)
{
    struct lconv *lc = &lconvinfo;

    char *buf, *bufcopy;
    char *allowed, *emptystr, *defaultstr;
    long int val;
    bool done;
    int ret;


    assert(result != NULL);
    assert(min <= max);

    buf        = malloc(BUFSIZE);
    bufcopy    = malloc(BUFSIZE);
    allowed    = malloc(BUFSIZE);
    emptystr   = malloc(BUFSIZE);
    defaultstr = malloc(BUFSIZE);

    if (buf == NULL || bufcopy == NULL || allowed == NULL
	|| emptystr == NULL || defaultstr == NULL) {
	err_exit_nomem();
    }

    *buf = '\0';

    strcpy(allowed, "0123456789+-");
    strncat(allowed, lc->thousands_sep,     BUFSIZE - strlen(allowed) - 1);
    strncat(allowed, lc->mon_thousands_sep, BUFSIZE - strlen(allowed) - 1);

    snprintf(emptystr,   BUFSIZE, "%'1ld", emptyval);
    snprintf(defaultstr, BUFSIZE, "%'1ld", defaultval);

    done = false;
    while (! done) {
	ret = gettxline(win, buf, BUFSIZE, NULL, false, emptystr, defaultstr,
			allowed, true, y, x, width, attr);

	if (ret == OK) {
	    char *p;

	    txinput_fixup(bufcopy, buf, false);
	    val = strtol(bufcopy, &p, 10);

	    if (*p == '\0' && val >= min && val <= max) {
		*result = val;
		done = true;
	    } else {
		beep();
	    }
	} else {
	    done = true;
	}
    }

    free(defaultstr);
    free(emptystr);
    free(allowed);
    free(bufcopy);
    free(buf);

    return ret;
}


/***********************************************************************/
// answer_yesno: Wait for a Yes/No answer

bool answer_yesno (WINDOW *win, chtype attr_keys)
{
    int key;
    bool done;

    chtype oldattr = getattrs(win);
    chtype oldbkgd = getbkgd(win);


    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    waddstr(curwin, " [");
    attrpr(curwin, attr_keys, "Y");
    waddstr(curwin, "/");
    attrpr(curwin, attr_keys, "N");
    waddstr(curwin, "] ");

    curs_set(CURS_ON);

    done = false;
    while (! done) {
	key = toupper(wgetch(win));

	if (key == 'Y' || key == 'N') {
	    done = true;
	} else {
	    beep();
	}
    }

    curs_set(CURS_OFF);
    wattron(win, A_BOLD);

    if (key == 'Y') {
	waddstr(win, "Yes");
    } else {
	waddstr(win, "No");
    }

    wbkgdset(win, oldbkgd);
    wattrset(win, oldattr);

    wrefresh(win);
    return (key == 'Y');
}


/***********************************************************************/
// wait_for_key: Print a message and wait for any key

void wait_for_key (WINDOW *win, int y, chtype attr)
{
    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    center(win, y, attr, "[ Press <SPACE> to continue ] ");
    wrefresh(win);

    (void) wgetch(win);
}


/***********************************************************************/
// End of file
