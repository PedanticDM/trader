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

    if ((COLS < MIN_COLUMNS) || (LINES < MIN_LINES)) {
	err_exit("terminal size is too small (%d x %d required)",
		 MIN_COLUMNS, MIN_LINES);
    }

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
    move(0, 0);

    attrset(has_colors() ? COLOR_PAIR(YELLOW_ON_CYAN) | A_BOLD :
	    A_REVERSE | A_BOLD);
    center(stdscr, true, PACKAGE_NAME);
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
    clear();
    refresh();
    endwin();
}


/*-----------------------------------------------------------------------
  Function:   center   - Centre a string on the current line
  Arguments:  win      - Window to use
              clrline  - True to print spaces on both sides of line
              format   - printf()-like format string
              ...      - printf()-like arguments
  Returns:    int      - Return code from wprintw()

  This function prints a string (formated with wprintw(format, ...)) in
  the centre of the current line in the window win.  If clrline is TRUE,
  spaces are printed before and after the line to make sure the current
  attributes are set.  The cursor is then moved to the start of the next
  line, or the start of the current line (if already on the last line of
  the screen).  Please note that wrefresh() is NOT called.
*/

int center (WINDOW *win, const bool clrline, const char *format, ...)
{
    va_list args;

    int len, ret;
    int y, x, maxy, maxx;
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

    getyx(win, y, x);
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
    } else {
	ret = mvwprintw(win, y, fill > 0 ? fill : 0, "%s", buf);
    }

    wmove(win, (y + 1 >= maxy ? y : y + 1), 0);

    free(buf);
    return ret;
}
