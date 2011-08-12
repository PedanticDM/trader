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


// Declarations for argument processing in prepstr()

#define MAXFMTARGS	8	// Maximum number of positional arguments

enum argument_type {
    TYPE_NONE,			// No type yet assigned
    TYPE_INT,			// int
    TYPE_LONGINT,		// long int
    TYPE_DOUBLE,		// double
    TYPE_STRING			// const char *
};

struct argument {
    enum argument_type a_type;
    union a {
	int		a_int;
	long int	a_longint;
	double		a_double;
	const char	*a_string;
    } a;
};


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
  Function:   init_title - Draw the main window title
  Parameters: (none)
  Returns:    (nothing)

  This function draws the main window game title, "Star Traders", and
  clears the rest of the screen.  It does NOT call wrefresh().
*/
static void init_title (void);


/*
  Function:   sigterm_handler - Handle program termination signals
  Parameters: sig             - Signal number
  Returns:    (nothing)

  This function handles termination signals (like SIGINT, SIGTERM and
  SIGQUIT) by clearing the screen, uninstalling itself and reraising the
  signal.
*/
static void sigterm_handler (int sig);


/*
  Function:   txresize - Handle a terminal resize event
  Parameters: (none)
  Returns:    (nothing)

  This function handles a SIGWINCH (terminal window size changed) event
  by refreshing Curses windows as appropriate.
*/
#ifdef HANDLE_RESIZE_EVENTS
static void txresize (void);
#endif


/*
  Function:   prepstr_addch - Add a character to the prepstr buffer
  Parameters: chbuf         - Pointer to chtype pointer in which to store string
              chbufsize     - Pointer to number of chtype elements in chbuf
              attr          - Character rendition to use
              maxlines      - Maximum number of screen lines to use
              maxwidth      - Maximum width of each line, in chars
              line          - Pointer to current line number
              width         - Pointer to current line width
              lastspc       - Pointer to const char * pointer to last space
              widthspc      - Pointer to width just before last space
              widthbuf      - Pointer to buffer to store widths of each line
              widthbufsize  - Number of int elements in widthbuf
              str           - Pointer to const char * pointer to string
  Returns:    int           - -1 on error (with errno set), 0 otherwise

  This helper function adds the character **str to **chbuf, using attr as
  the character rendition (attributes), incrementing both *str and *chbuf
  and decrementing *chbufsize.  If a string is too long for the current
  line, a previous space in the current line is converted to a new line
  (if possible), else a new line is inserted into the current location
  (if not on the last line).  *line, *width, *lastspc, *widthspc and
  widthbuf[] are all updated appropriately.
*/
static int prepstr_addch (chtype *restrict *restrict chbuf,
			  int *restrict chbufsize, chtype attr,
			  int maxlines, int maxwidth, int *restrict line,
			  int *restrict width,
			  chtype *restrict *restrict lastspc,
			  int *restrict widthspc, int *restrict widthbuf,
			  int widthbufsize,
			  const char *restrict *restrict str);


/*
  Function:   txinput_fixup - Copy strings with fixup
  Parameters: dest          - Destination buffer of size BUFSIZE
              src           - Source buffer of size BUFSIZE
              isfloat       - True if src contains a floating point number
  Returns:    (nothing)

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

/* These functions are documented either in the file "intf.h" or in the
   comments above. */


/***********************************************************************/
// init_screen: Initialise the screen (terminal display)

void init_screen (void)
{
    struct sigaction sa;


    // Initialise signal handlers
    sa.sa_handler = sigterm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGQUIT);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
	errno_exit("sigaction(SIGINT)");
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
	errno_exit("sigaction(SIGTERM)");
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
	errno_exit("sigaction(SIGQUIT)");
    }

    // Initialise the screen
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

    init_title();
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
// init_title: Draw the main window title

void init_title (void)
{
    chtype *chbuf;
    int width;
    int lines;


    bkgd(attr_root_window);
    clear();

    move(0, 0);
    for (int i = 0; i < COLS; i++) {
	addch(attr_game_title | ' ');
    }

    chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    lines = prepstr(chbuf, BUFSIZE, attr_game_title, 0, 0, 1, COLS,
		    &width, 1, _("Star Traders"));
    pr_center(stdscr, 0, 0, chbuf, lines, &width);
    attrset(attr_root_window);
    free(chbuf);
}


/***********************************************************************/
// sigterm_handler: Handle program termination signals

void sigterm_handler (int sig)
{
    struct sigaction sa;


    /* The following Curses functions are NOT async-signal-safe (ie, are
       not reentrant) as they may well call malloc() or free().  However,
       it does allow us to terminate with the correct signal without
       having convoluted code in the main program. */

    curs_set(CURS_ON);
    clear();
    refresh();
    endwin();

    // Reraise the same signal, using the system-default handler
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(sig, &sa, NULL);
    raise(sig);
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

    // Insert the new window into the txwin stack

    nw = xmalloc(sizeof(txwin_t));
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
// txresize: Handle a terminal resize event

#ifdef HANDLE_RESIZE_EVENTS

void txresize (void)
{
    /* The current implementation cannot resize windows per se: a given
       window would have to be destroyed and recreated in the new
       location, then redrawn, most likely via a call-back function.
       We just redraw the game title, refresh all windows and hope for
       the best! */

    init_title();
    txrefresh();
}

#endif // HANDLE_RESIZE_EVENTS


/***********************************************************************/
// txdlgbox: Display a dialog box and wait for any key

int txdlgbox (int maxlines, int ncols, int begin_y, int begin_x,
	      chtype bkgd_attr, chtype title_attr, chtype norm_attr,
	      chtype alt1_attr, chtype alt2_attr, chtype keywait_attr,
	      const char *restrict boxtitle, const char *restrict format, ...)
{
    bool usetitle = (boxtitle != NULL);

    chtype *chbuf;
    int *widthbuf;
    int lines;
    va_list args;


    assert(maxlines > 0);

    chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    widthbuf = xmalloc(maxlines * sizeof(int));

    va_start(args, format);
    lines = vprepstr(chbuf, BUFSIZE, norm_attr, alt1_attr, alt2_attr, maxlines,
		     ncols - 4, widthbuf, maxlines, format, args);
    va_end(args);

    newtxwin(usetitle ? lines + 6 : lines + 5, ncols, begin_y, begin_x,
	     true, bkgd_attr);

    if (usetitle) {
	chtype *titlebuf = xmalloc(BUFSIZE * sizeof(chtype));
	int titlewidth;
	int titlelines;

	titlelines = prepstr(titlebuf, BUFSIZE, title_attr, 0, 0, 1,
			     ncols - 4, &titlewidth, 1, boxtitle);
	pr_center(curwin, 1, 0, titlebuf, titlelines, &titlewidth);
	free(titlebuf);
    }

    pr_center(curwin, usetitle ? 3 : 2, 0, chbuf, lines, widthbuf);
    wait_for_key(curwin, getmaxy(curwin) - 2, keywait_attr);
    deltxwin();

    free(widthbuf);
    free(chbuf);
    return OK;
}


/***********************************************************************/
// prepstr: Prepare a string for printing to screen

int prepstr (chtype *restrict chbuf, int chbufsize, chtype attr_norm,
	     chtype attr_alt1, chtype attr_alt2, int maxlines, int maxwidth,
	     int *restrict widthbuf, int widthbufsize,
	     const char *restrict format, ...)
{
    va_list args;
    int lines;


    va_start(args, format);
    lines = vprepstr(chbuf, chbufsize, attr_norm, attr_alt1, attr_alt2,
		     maxlines, maxwidth, widthbuf, widthbufsize, format,
		     args);
    va_end(args);
    return lines;
}


/***********************************************************************/
// prepstr_addch: Add a character to the prepstr buffer

int prepstr_addch (chtype *restrict *restrict chbuf, int *restrict chbufsize,
		   chtype attr, int maxlines, int maxwidth,
		   int *restrict line, int *restrict width,
		   chtype *restrict *restrict lastspc, int *restrict widthspc,
		   int *restrict widthbuf, int widthbufsize,
		   const char *restrict *restrict str)
{
    if (*line < 0) {
	// First character in buffer: start line 0
	*line = 0;
    }

    if (**str == '\n') {
	// Start a new line

	if (*line < maxlines - 1) {
	    *(*chbuf)++ = '\n';
	    (*chbufsize)--;
	}

	widthbuf[*line] = *width;
	*width = 0;

	*lastspc = NULL;
	*widthspc = 0;

	(*line)++;
	(*str)++;
    } else if (*width == maxwidth) {
	// Current line is now too long

	if (! isspace(**str) && *lastspc != NULL && *line < maxlines - 1) {
	    // Break on the last space in this line
	    **lastspc = '\n';

	    widthbuf[*line] = *widthspc;
	    *width -= *widthspc + 1;

	    *lastspc = NULL;
	    *widthspc = 0;

	    (*line)++;
	} else {
	    // Insert a new-line character (if not on last line)
	    if (*line < maxlines - 1) {
		*(*chbuf)++ = '\n';
		(*chbufsize)--;
	    }

	    widthbuf[*line] = *width;
	    *width = 0;

	    *lastspc = NULL;
	    *widthspc = 0;

	    (*line)++;

	    // Skip any following spaces
	    while (isspace(**str)) {
		if (*(*str)++ == '\n') {
		    break;
		}
	    }
	}
    } else {
	// Insert an ordinary character into the output string

	if (isspace(**str)) {
	    *lastspc = *chbuf;
	    *widthspc = *width;
	}

	*(*chbuf)++ = (unsigned char) **str | attr;
	(*chbufsize)--;
	(*width)++;
	(*str)++;
    }

    return 0;
}


/***********************************************************************/
// vprepstr: Prepare a string for printing to screen

int vprepstr (chtype *restrict chbuf, int chbufsize, chtype attr_norm,
	      chtype attr_alt1, chtype attr_alt2, int maxlines, int maxwidth,
	      int *restrict widthbuf, int widthbufsize,
	      const char *restrict format, va_list args)
{
    struct argument format_arg[MAXFMTARGS];
    int num_format_args, arg_num;
    const char *orig_format;
    int line, width;
    chtype *lastspc;
    int widthspc;
    chtype curattr;
    int saved_errno;


    assert(chbuf != NULL);
    assert(chbufsize > 0);
    assert(maxlines > 0);
    assert(maxwidth > 0);
    assert(widthbuf != NULL);
    assert(widthbufsize >= maxlines);
    assert(format != NULL);

    /* Do a preliminary scan through the format parameter to determine
       the types of each positional argument (conversion specifier).  If
       we did not support "%m$"-style specifiers, this would not be
       necessary. */

    orig_format = format;
    memset(format_arg, 0, sizeof(format_arg));
    num_format_args = 0;
    arg_num = 0;

    while (*format != '\0') {
	switch (*format++) {
	case '^':
	    // Switch to a different character rendition
	    if (*format == '\0') {
		goto error_inval;
	    } else {
		format++;
	    }
	    break;

	case '%':
	    // Process a conversion specifier
	    if (*format == '\0') {
		goto error_inval;
	    } else if (*format == '%') {
		format++;
	    } else {
		enum argument_type arg_type = TYPE_NONE;
		bool inspec = true;
		bool flag_posn = false;
		bool flag_long = false;
		int count = 0;

		while (inspec && *format != '\0') {
		    char c = *format++;
		    switch (c) {
		    case '0':
			// Zero flag, or part of numeric count
			if (count == 0)
			    goto error_inval;

			count *= 10;
			break;

		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
		    case '8':
		    case '9':
			// Part of some numeric count
			count = count * 10 + (c - '0');
			break;

		    case '$':
			// Fixed-position argument
			if (flag_posn || count == 0)
			    goto error_inval;

			if (count > MAXFMTARGS) {
			    errno = E2BIG;
			    goto error;
			}

			flag_posn = true;
			arg_num = count - 1;
			count = 0;
			break;

		    case '\'':
			// Use locale-specific thousands separator
			break;

		    case 'l':
			// Long length modifier
			if (flag_long)
			    goto error_inval;

			flag_long = true;
			break;

		    case 'd':
			// Insert an integer (int or long int)
			arg_type = flag_long ? TYPE_LONGINT : TYPE_INT;
			goto handlefmt;

		    case 'N':
			// Insert a monetary amount (double)
			if (flag_long)
			    goto error_inval;

			arg_type = TYPE_DOUBLE;
			goto handlefmt;

		    case 's':
			// Insert a string (const char *)
			if (flag_long)
			    goto error_inval;

			arg_type = TYPE_STRING;

		    handlefmt:
			if (arg_num >= MAXFMTARGS) {
			    errno = E2BIG;
			    goto error;
			}

			if (format_arg[arg_num].a_type == TYPE_NONE) {
			    format_arg[arg_num].a_type = arg_type;
			} else if (format_arg[arg_num].a_type != arg_type) {
			    goto error_inval;
			}

			arg_num++;
			num_format_args = MAX(num_format_args, arg_num);

			inspec = false;
			break;

		    default:
			goto error_inval;
		    }
		}
		if (inspec) {
		    goto error_inval;
		}
	    }
	    break;

	default:
	    // Process an ordinary character: do nothing for now
	    ;
	}
    }

    for (int i = 0; i < num_format_args; i++) {
	switch (format_arg[i].a_type) {
	case TYPE_INT:
	    format_arg[i].a.a_int = va_arg(args, int);
	    break;

	case TYPE_LONGINT:
	    format_arg[i].a.a_longint = va_arg(args, long int);
	    break;

	case TYPE_DOUBLE:
	    format_arg[i].a.a_double = va_arg(args, double);
	    break;

	case TYPE_STRING:
	    format_arg[i].a.a_string = va_arg(args, const char *);
	    break;

	default:
	    /* Cannot allow unused arguments, as we have no way of
	       knowing how much space they take (cf. int vs. long long
	       int). */
	    goto error_inval;
	}
    }

    // Actually process the format parameter string

    format = orig_format;
    arg_num = 0;

    curattr = attr_norm;
    line = -1;				// Current line number (0 = first)
    width = 0;				// Width of the current line
    lastspc = NULL;			// Pointer to last space in line
    widthspc = 0;			// Width of line before last space

    while (*format != '\0' && chbufsize > 1 && line < maxlines) {
	switch (*format) {
	case '^':
	    // Switch to a different character rendition
	    if (*++format == '\0') {
		goto error_inval;
	    } else {
		switch (*format) {
		case '^':
		    if (prepstr_addch(&chbuf, &chbufsize, curattr, maxlines,
				      maxwidth, &line, &width, &lastspc,
				      &widthspc, widthbuf, widthbufsize,
				      &format) < 0) {
			goto error;
		    }
		    break;

		case '{':
		    curattr = attr_alt1;
		    format++;
		    break;

		case '[':
		    curattr = attr_alt2;
		    format++;
		    break;

		case '}':
		case ']':
		    curattr = attr_norm;
		    format++;
		    break;

		default:
		    goto error_inval;
		}
	    }
	    break;

	case '%':
	    // Process a conversion specifier
	    if (*++format == '\0') {
		goto error_inval;
	    } else if (*format == '%') {
		if (prepstr_addch(&chbuf, &chbufsize, curattr, maxlines,
				  maxwidth, &line, &width, &lastspc, &widthspc,
				  widthbuf, widthbufsize, &format) < 0) {
		    goto error;
		}
	    } else {
		bool inspec = true;
		bool flag_posn = false;
		bool flag_long = false;
		bool flag_thou = false;
		int count = 0;
		const char *str;

		char *buf = xmalloc(BUFSIZE);

		while (inspec && *format != '\0') {
		    char c = *format++;
		    switch (c) {
		    case '0':
			// Zero flag, or part of numeric count
			if (count == 0) {
			    // Zero flag is not supported
			    free(buf);
			    goto error_inval;
			}

			count *= 10;
			break;

		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
		    case '8':
		    case '9':
			// Part of some numeric count
			count = count * 10 + (c - '0');
			break;

		    case '$':
			// Fixed-position argument
			if (flag_posn || count == 0) {
			    free(buf);
			    goto error_inval;
			}

			if (count > MAXFMTARGS) {
			    free(buf);
			    errno = E2BIG;
			    goto error;
			}

			flag_posn = true;
			arg_num = count - 1;
			count = 0;
			break;

		    case '\'':
			// Use locale-specific thousands separator
			if (flag_thou) {
			    free(buf);
			    goto error_inval;
			}

			flag_thou = true;
			break;

		    case 'l':
			// Long length modifier
			if (flag_long) {
			    free(buf);
			    goto error_inval;
			}

			flag_long = true;
			break;

		    case 'd':
			// Insert an integer (int or long int) into the output
			if (count != 0) {
			    free(buf);
			    goto error_inval;
			}

			if (arg_num >= MAXFMTARGS) {
			    free(buf);
			    errno = E2BIG;
			    goto error;
			}

			if (flag_long) {
			    if (snprintf(buf, BUFSIZE, flag_thou ? "%'ld" : "%ld",
					 format_arg[arg_num].a.a_longint) < 0) {
				saved_errno = errno;
				free(buf);
				errno = saved_errno;
				goto error;
			    }
			} else {
			    if (snprintf(buf, BUFSIZE, flag_thou ? "%'d" : "%d",
					 format_arg[arg_num].a.a_int) < 0) {
				saved_errno = errno;
				free(buf);
				errno = saved_errno;
				goto error;
			    }
			}

			str = buf;
			goto insertstr;

		    case 'N':
			// Insert a monetary amount (double) into the output
			if (count != 0 || flag_thou || flag_long) {
			    free(buf);
			    goto error_inval;
			}

			if (arg_num >= MAXFMTARGS) {
			    free(buf);
			    errno = E2BIG;
			    goto error;
			}

			if (l_strfmon(buf, BUFSIZE, "%n",
				      format_arg[arg_num].a.a_double) < 0) {
			    saved_errno = errno;
			    free(buf);
			    errno = saved_errno;
			    goto error;
			}

			str = buf;
			goto insertstr;

		    case 's':
			// Insert a string (const char *) into the output
			if (count != 0 || flag_thou || flag_long) {
			    free(buf);
			    goto error_inval;
			}

			if (arg_num >= MAXFMTARGS) {
			    free(buf);
			    errno = E2BIG;
			    goto error;
			}

			str = format_arg[arg_num].a.a_string;

			if (str == NULL) {
			    str = "(null)";	// As per GNU printf()
			}

		    insertstr:
			// Insert the string pointed to by str
			while (*str != '\0' && chbufsize > 1 && line < maxlines) {
			    if (prepstr_addch(&chbuf, &chbufsize, curattr,
					      maxlines, maxwidth, &line, &width,
					      &lastspc, &widthspc, widthbuf,
					      widthbufsize, &str) < 0) {
				saved_errno = errno;
				free(buf);
				errno = saved_errno;
				goto error;
			    }
			}

			arg_num++;
			inspec = false;
			break;

		    default:
			free(buf);
			goto error_inval;
		    }
		}
		free(buf);
		if (inspec) {
		    goto error_inval;
		}
	    }
	    break;

	default:
	    // Process an ordinary character (including new-line)
	    if (prepstr_addch(&chbuf, &chbufsize, curattr, maxlines, maxwidth,
			      &line, &width, &lastspc, &widthspc, widthbuf,
			      widthbufsize, &format) < 0) {
		goto error;
	    }
	}
    }

    *chbuf = 0;				// Terminating NUL byte

    if (line >= 0 && line < maxlines) {
	widthbuf[line] = width;
    } else if (line >= maxlines) {
	line = maxlines - 1;
    }

    return line + 1;


error_inval:
    errno = EINVAL;

error:
    errno_exit(_("prepstr: `%s'"), orig_format);
}


/***********************************************************************/
// chbufdup: Duplicate a chtype buffer

chtype *chbufdup (const chtype *restrict chbuf, int chbufsize)
{
    const chtype *p;
    int len;
    chtype *ret;


    // Determine chbuf length, including ending NUL
    for (len = 1, p = chbuf; *p != '\0' && len <= chbufsize; p++, len++)
	;

    ret = xmalloc(len * sizeof(chtype));
    memcpy(ret, chbuf, len * sizeof(chtype));
    ret[len - 1] = '\0';	// Terminating NUL, just in case not present

    return ret;
}


/***********************************************************************/
// pr_left: Print strings in chbuf left-aligned

int pr_left (WINDOW *win, int y, int x, const chtype *restrict chbuf,
	     int lines, const int *restrict widthbuf)
{
    assert(win != NULL);
    assert(chbuf != NULL);
    assert(lines > 0);
    assert(widthbuf != NULL);

    wmove(win, y, x);
    for ( ; *chbuf != '\0'; chbuf++) {
	if (*chbuf == '\n') {
	    wmove(win, getcury(win) + 1, x);
	} else {
	    waddch(win, *chbuf);
	}
    }

    return OK;
}


/***********************************************************************/
// pr_center: Print strings in chbuf centred in window

int pr_center (WINDOW *win, int y, int offset, const chtype *restrict chbuf,
	       int lines, const int *restrict widthbuf)
{
    int ln = 0;


    assert(win != NULL);
    assert(chbuf != NULL);
    assert(lines > 0);
    assert(widthbuf != NULL);

    wmove(win, y, (getmaxx(win) - widthbuf[ln]) / 2 + offset);
    for ( ; *chbuf != '\0'; chbuf++) {
	if (*chbuf == '\n') {
	    if (ln++ >= lines) {
		return ERR;
	    } else {
		wmove(win, getcury(win) + 1,
		      (getmaxx(win) - widthbuf[ln]) / 2 + offset);
	    }
	} else {
	    waddch(win, *chbuf);
	}
    }

    return OK;
}


/***********************************************************************/
// pr_right: Print strings in chbuf right-aligned

int pr_right (WINDOW *win, int y, int x, const chtype *restrict chbuf,
	      int lines, const int *restrict widthbuf)
{
    int ln = 0;


    assert(win != NULL);
    assert(chbuf != NULL);
    assert(lines > 0);
    assert(widthbuf != NULL);

    wmove(win, y, x - widthbuf[ln]);
    for ( ; *chbuf != '\0'; chbuf++) {
	if (*chbuf == '\n') {
	    if (ln++ >= lines) {
		return ERR;
	    } else {
		wmove(win, getcury(win) + 1, x - widthbuf[ln]);
	    }
	} else {
	    waddch(win, *chbuf);
	}
    }

    return OK;
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

    buf = xmalloc(BUFSIZE);

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

    buf = xmalloc(BUFSIZE);

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

    buf = xmalloc(BUFSIZE);

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
    int key;
    bool done;


    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    done = false;
    while (! done) {
	key = wgetch(win);
	switch (key) {
	case ERR:
	    beep();
	    break;

#ifdef HANDLE_RESIZE_EVENTS
	case KEY_RESIZE:
	    txresize();
	    break;
#endif // HANDLE_RESIZE_EVENTS

	default:
	    done = true;
	}
    }

    return key;
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

#ifdef HANDLE_RESIZE_EVENTS
		    case KEY_RESIZE:
			txresize();
			break;
#endif // HANDLE_RESIZE_EVENTS

		    default:
			beep();
		    }
		}

		wtimeout(win, -1);
		break;

#ifdef HANDLE_RESIZE_EVENTS
	    case KEY_RESIZE:
		txresize();
		break;
#endif // HANDLE_RESIZE_EVENTS

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
	*bufptr = xmalloc(BUFSIZE);
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

    buf        = xmalloc(BUFSIZE);
    bufcopy    = xmalloc(BUFSIZE);
    allowed    = xmalloc(BUFSIZE);
    emptystr   = xmalloc(BUFSIZE);
    defaultstr = xmalloc(BUFSIZE);

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

    buf        = xmalloc(BUFSIZE);
    bufcopy    = xmalloc(BUFSIZE);
    allowed    = xmalloc(BUFSIZE);
    emptystr   = xmalloc(BUFSIZE);
    defaultstr = xmalloc(BUFSIZE);

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
	key = wgetch(win);

	switch (key) {
	case 'Y':
	case 'y':
	case 'N':
	case 'n':
	    key = toupper(key);
	    done = true;
	    break;

#ifdef HANDLE_RESIZE_EVENTS
	case KEY_RESIZE:
	    txresize();
	    break;
#endif // HANDLE_RESIZE_EVENTS

	default:
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
    chtype *chbuf;
    int width;
    int lines;
    int key;
    bool done;


    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    lines = prepstr(chbuf, BUFSIZE, attr, 0, 0, 1, getmaxx(win) - 4,
		    &width, 1, _("[ Press <SPACE> to continue ] "));
    pr_center(win, y, 0, chbuf, lines, &width);
    wrefresh(win);

    done = false;
    while (! done) {
	key = wgetch(win);
	switch (key) {
	case ERR:
	    beep();
	    break;

#ifdef HANDLE_RESIZE_EVENTS
	case KEY_RESIZE:
	    txresize();
	    break;
#endif // HANDLE_RESIZE_EVENTS

	default:
	    done = true;
	}
    }

    free(chbuf);
}


/***********************************************************************/
// End of file
