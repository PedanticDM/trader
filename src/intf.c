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

#define OUTBUFSIZE	(1024)	/* Output string buffer size */


typedef struct txwin {
    WINDOW		*win;		// Pointer to window structure
    struct txwin	*next;		// Next window in stack
    struct txwin	*prev;		// Previous window in stack
} txwin_t;


/************************************************************************
*                      Global variable definitions                      *
************************************************************************/

WINDOW *curwin = NULL;		// Top-most (current) window


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
    initscr();

    if ((COLS < MIN_COLS) || (LINES < MIN_LINES)) {
	err_exit("terminal size is too small (%d x %d required)",
		 MIN_COLS, MIN_LINES);
    }

    curwin = stdscr;
    topwin = NULL;
    firstwin = NULL;

    noecho();
    curs_set(CURS_INVISIBLE);
    raw();

    if (has_colors()) {
	start_color();

	init_pair(WHITE_ON_BLACK, COLOR_WHITE,  COLOR_BLACK);
	init_pair(WHITE_ON_BLUE,  COLOR_WHITE,  COLOR_BLUE);
	init_pair(YELLOW_ON_CYAN, COLOR_YELLOW, COLOR_CYAN);
	init_pair(WHITE_ON_RED,   COLOR_WHITE,  COLOR_RED);
	init_pair(BLACK_ON_WHITE, COLOR_BLACK,  COLOR_WHITE);

	bkgd(COLOR_PAIR(WHITE_ON_BLACK));
    }

    clear();

    attrset(ATTR(COLOR_PAIR(YELLOW_ON_CYAN) | A_BOLD, A_REVERSE | A_BOLD));
    center(stdscr, 0, true, PACKAGE_NAME);
    attrset(A_NORMAL);

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
    WINDOW	*win;
    txwin_t	*nw;


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
    txwin_t	*cur, *prev;
    int		r;


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

    r = delwin(cur->win);
    free(cur);

    return r;
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


/*-----------------------------------------------------------------------
  Function:   center   - Centre a string on the current line
  Arguments:  win      - Window to use
              y        - Line on which to centre the string
              clrline  - True to print spaces on both sides of line
              format   - printf()-like format string
              ...      - printf()-like arguments
  Returns:    int      - Return code from wprintw()

  This function prints a string (formated with wprintw(format, ...)) in
  the centre of line y in the window win.  If clrline is TRUE, spaces are
  printed before and after the line to make sure the current attributes
  are set; in this case, the cursor is also moved to the start of the
  next line (or the start of the current line if already on the last line
  of the window).  Please note that wrefresh() is NOT called.
*/

int center (WINDOW *win, int y, const bool clrline, const char *format, ...)
{
    va_list args;

    int len, ret;
    int maxy, maxx;
    int fill;

    char *buf = malloc(OUTBUFSIZE);
    if (buf == NULL) {
	err_exit("out of memory");
    }

    va_start(args, format);
    len = vsnprintf(buf, OUTBUFSIZE, format, args);
    if (len < 0) {
	return ERR;
    }

    getmaxyx(win, maxy, maxx);
    fill = (maxx - len) / 2;

    if (clrline) {
	wmove(win, y, 0);

	if (fill > 0) {
	    wprintw(win, "%*c", fill, ' ');
	}
	ret = wprintw(win, "%s", buf);
	if (maxx - len - fill > 0) {
	    wprintw(win, "%*c", maxx - len - fill, ' ');
	}

	wmove(win, (y + 1 >= maxy ? y : y + 1), 0);
    } else {
	ret = mvwprintw(win, y, fill > 0 ? fill : 0, "%s", buf);
    }

    free(buf);
    return ret;
}
