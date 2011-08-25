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
*                      Global variable definitions                      *
************************************************************************/

WINDOW *curwin = NULL;		// Top-most (current) window
bool use_color = true;		// True to use colour


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
*               Game printing global variable definitions               *
************************************************************************/

wchar_t	*keycode_company;		// Keycodes for each company
wchar_t	*printable_map_val;		// Printable output for each map value
chtype	*chtype_map_val[MAX_COMPANIES + 3];	// as chtype strings

wchar_t	*keycode_game_move;		// Keycodes for each game move
wchar_t	*printable_game_move;		// Printable output for each game move
chtype	*chtype_game_move[NUMBER_MOVES];	// as chtype strings


/************************************************************************
*        Module-specific constants, type declarations and macros        *
************************************************************************/

typedef struct txwin {
    WINDOW		*win;		// Pointer to window structure
    struct txwin	*next;		// Next window in stack
    struct txwin	*prev;		// Previous window in stack
} txwin_t;


// Initialisation macros used in init_screen()

#define __stringify(s) #s

#define init_game_str(_var, _default, _checkpos)			\
    do {								\
	char *s = gettext(_default);					\
	if (xmbstowcs(buf, s, BUFSIZE) < (_checkpos) + 1		\
	    || buf[_checkpos] != '|') {					\
	    err_exit(_("%s: string has incorrect format: `%s'"),	\
		     __stringify(_var), s);				\
	}								\
	(_var) = xwcsdup(buf);						\
	(_var)[_checkpos] = '\0';					\
    } while (0)

#define init_game_chstr(_chvar, _var, _attr, _err)			\
    do {								\
	chtype *p = chbuf;						\
	wchar_t c;							\
	int w, n;							\
	mbstate_t mbstate;						\
									\
	c = (_var);							\
	if ((w = wcwidth(c)) < 1) {					\
	    err_exit(_("%s: character has illegal width: `%lc'"),	\
		     __stringify(_err), c);				\
	}								\
									\
	memset(&mbstate, 0, sizeof(mbstate));				\
	n = xwcrtomb(convbuf, c, &mbstate);				\
	for (int i = 0; i < n; i++) {					\
	    *p++ = (unsigned char) convbuf[i] | (_attr);		\
	}								\
									\
	if (w == 1) {							\
	    n = xwcrtomb(convbuf, ' ', &mbstate);			\
	    for (int i = 0; i < n; i++) {				\
		*p++ = (unsigned char) convbuf[i] | attr_map_empty;	\
	    }								\
	}								\
									\
	n = xwcrtomb(convbuf, '\0', &mbstate);				\
	for (int i = 0; i < n; i++) {					\
	    *p++ = (unsigned char) convbuf[i];				\
	}								\
									\
	(_chvar) = xchstrdup(chbuf);					\
    } while (0)


// Declarations for argument processing in mkchstr()

#define MAXFMTARGS	8	// Maximum number of positional arguments

enum argument_type {
    TYPE_NONE,			// No type yet assigned
    TYPE_CHAR,			// char
    TYPE_WCHAR,			// wchar_t
    TYPE_INT,			// int
    TYPE_LONGINT,		// long int
    TYPE_DOUBLE,		// double
    TYPE_STRING,		// const char *
    TYPE_WSTRING		// const wchar_t *
};

struct argument {
    enum argument_type a_type;
    union a {
	char		a_char;
	wchar_t		a_wchar;
	int		a_int;
	long int	a_longint;
	double		a_double;
	const char	*a_string;
	const wchar_t	*a_wstring;
    } a;
};


#define MAXFMTSPECS	16	// Maximum number of conversion specifiers

struct convspec {
    wchar_t	spec;		// Conversion specifier: c d f N s
    int		arg_num;	// Which variable argument to use
    ptrdiff_t	len;		// Length of conversion specifier, 0 = unused
    int		precision;	// Precision value
    bool	flag_group;	// Flag "'" (thousands grouping)
    bool	flag_nosym;	// Flag "!" (omit currency symbol)
    bool	flag_prec;	// Flag "." (precision)
    bool	flag_long;	// Length modifier "l" (long)
};


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
  Function:   mkchstr_parse - Parse the format string for mkchstr()
  Parameters: format        - Format string as described for mkchstr()
              format_arg    - Pointer to variable arguments array
              format_spec   - Pointer to conversion specifiers array
              args          - Variable argument list passed to mkchstr()
  Returns:    int           - 0 if OK, -1 if error (with errno set)

  This helper function parses the format string passed to mkchstr(),
  setting the format_arg and format_spec arrays appropriately.
*/
static int mkchstr_parse (const wchar_t *restrict format,
			  struct argument *restrict format_arg,
			  struct convspec *restrict format_spec,
			  va_list args);


/*
  Function:   mkchstr_add  - Add one character to the mkchstr() buffers
  Parameters: outbuf       - Pointer to wchar_t pointer in which to store char
              attrbuf      - Pointer to chtype pointer in which to store attr
              count        - Pointer to number of wchar_t elements left in outbuf
              attr         - Character rendition to use
              maxlines     - Maximum number of screen lines to use
              maxwidth     - Maximum width of each line, in column positions
              line         - Pointer to current line number
              width        - Pointer to current line width
              lastspc      - Pointer to wchar_t * pointer to last space
              spcattr      - Pointer to corresponding place in attrbuf
              widthspc     - Pointer to width just before last space
              widthbuf     - Pointer to buffer to store widths of each line
              widthbufsize - Number of int elements in widthbuf
              str          - Pointer to const wchar_t * pointer to string
  Returns:    int          - -1 on error (with errno set), 0 otherwise

  This helper function adds one wide character from **str to **outbuf,
  and the character rendition attr to **attrbuf, incrementing *str and
  *outbuf and decrementing *count.  If a string is too long for the
  current line, a previous space in the current line is converted to a
  new line (if possible), else a new line is inserted into the current
  location (if not on the last line).  *line, *width, *lastspc, *widthspc
  and widthbuf[] are all updated appropriately.
*/
static int mkchstr_add (wchar_t *restrict *restrict outbuf,
			chtype *restrict *restrict attrbuf,
			int *restrict count, chtype attr, int maxlines,
			int maxwidth, int *restrict line, int *restrict width,
			wchar_t *restrict *restrict lastspc,
			chtype *restrict *restrict spcattr,
			int *restrict widthspc, int *restrict widthbuf,
			int widthbufsize, const wchar_t *restrict *restrict str);


/*
  Function:   mkchstr_conv - Convert (wcbuf, attrbuf) to chbuf
  Parameters: chbuf        - Pointer to chtype buffer in which to store string
              chbufsize    - Number of chtype elements in chbuf
              wcbuf        - Wide-character string from which to convert
              attrbuf      - Associated character rendition array
  Returns:    (nothing)

  This helper function converts the wide-character string in wcbuf and
  the array of character renditions in attrbuf to a chtype * string.
*/
static void mkchstr_conv (chtype *restrict chbuf, int chbufsize,
			  wchar_t *restrict wcbuf, chtype *restrict attrbuf);


/*
  Function:   getwch - Get a wide character from the keyboard
  Parameters: win    - Window to use (should be curwin)
              wch    - Pointer to wide character result
  Returns:    int    - OK, KEY_CODE_YES or ERR

  This internal function waits for a complete wide character to be typed
  on the keyboard.  OK is returned if wch contains an ordinary wide
  character, KEY_CODE_YES if a function key or control key, or ERR on
  error.

  This function is either a wrapper (with modifications) for wget_wch()
  from Curses, or an implementation of that function using wgetch().
*/
static int getwch (WINDOW *win, wint_t *wch);


/*
  Function:   cpos_endl - Adjust cpos and st for printing the ending part of buf
  Parameters: buf       - Pointer to current editing buffer
              cpos      - Pointer to current cursor position (result)
              st        - Pointer to current starting offset for buf[] (result)
              clen      - Current column width of entire string
              width     - Width of editing field in column spaces
              len       - Length of string being edited (wchar_t elements)
  Returns:    (nothing)

  This helper function adjusts *cpos and *st so that the cursor is placed
  at the end of the current editing buffer buf[].
*/
static void cpos_endl (wchar_t *restrict buf, int *restrict cpos,
		       int *restrict st, int clen, int width, int len);


/*
  Function:   cpos_decr - Adjust cpos and st: scroll to the left by w columns
  Parameters: buf       - Pointer to current editing buffer
              cpos      - Pointer to current cursor position (result)
              st        - Pointer to current starting offset for buf[] (result)
              w         - Number of columns to scroll left
              width     - Width of editing field in column spaces
  Returns:    (nothing)

  This helper function adjusts *cpos and *st so that the cursor is moved
  to the left by w column positions.
*/
static void cpos_decr (wchar_t *restrict buf, int *restrict cpos,
		       int *restrict st, int w, int width);


/*
  Function:   cpos_incr - Adjust cpos and st: scroll to the right by w columns
  Parameters: buf       - Pointer to current editing buffer
              cpos      - Pointer to current cursor position (result)
              st        - Pointer to current starting offset for buf[] (result)
              w         - Number of columns to scroll right
              width     - Width of editing field in column spaces
  Returns:    (nothing)

  This helper function adjusts *cpos and *st so that the cursor is moved
  to the right by w column positions.
*/
static void cpos_incr (wchar_t *restrict buf, int *restrict cpos,
		       int *restrict st, int w, int width);


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
static void txinput_fixup (wchar_t *restrict dest, wchar_t *restrict src,
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
    use_color = ! option_no_color && has_colors();
    if (use_color) {
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

    /* Initialise strings used for keycode input and map representations.

       Each string must have an ASCII vertical line (U+007C) in the
       correct position, followed by context information (such as
       "input|Company" and "output|MapVals").  This is done to overcome a
       limitation of gettext_noop() and N_() that does NOT allow context
       IDs.  This vertical line is replaced by a NUL character to
       terminate the resulting string.  The vertical line MAY appear in
       other positions; if so, it is handled correctly. */

    wchar_t *buf = xmalloc(BUFSIZE * sizeof(wchar_t));
    char convbuf[MB_LEN_MAX + 1];
    chtype chbuf[MB_LEN_MAX * 3 + 1];

    init_game_str(keycode_company, default_keycode_company, MAX_COMPANIES);
    init_game_str(keycode_game_move, default_keycode_game_move, NUMBER_MOVES);

    init_game_str(printable_map_val, default_printable_map_val, MAX_COMPANIES + 3);
    init_game_str(printable_game_move, default_printable_game_move, NUMBER_MOVES);

    /* To save time later, convert each output character to its own
       chtype string, with appropriate attributes. */

    init_game_chstr(chtype_map_val[MAP_TO_INDEX(MAP_EMPTY)],
		    printable_map_val[MAP_TO_INDEX(MAP_EMPTY)],
		    attr_map_empty, MAP_EMPTY);
    init_game_chstr(chtype_map_val[MAP_TO_INDEX(MAP_OUTPOST)],
		    printable_map_val[MAP_TO_INDEX(MAP_OUTPOST)],
		    attr_map_outpost, MAP_OUTPOST);
    init_game_chstr(chtype_map_val[MAP_TO_INDEX(MAP_STAR)],
		    printable_map_val[MAP_TO_INDEX(MAP_STAR)],
		    attr_map_star, MAP_STAR);
    for (int i = 0; i < MAX_COMPANIES; i++) {
	init_game_chstr(chtype_map_val[MAP_TO_INDEX(COMPANY_TO_MAP(i))],
			printable_map_val[MAP_TO_INDEX(COMPANY_TO_MAP(i))],
			attr_map_company, COMPANY_TO_MAP(i));
    }

    for (int i = 0; i < NUMBER_MOVES; i++) {
	init_game_chstr(chtype_game_move[i], printable_game_move[i],
			attr_map_choice, printable_game_move[i]);
    }

    free(buf);
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
    bkgd(attr_root_window);
    attrset(attr_root_window);
    clear();

    mvwhline(stdscr, 0, 0, ' ' | attr_game_title, COLS);
    center(stdscr, 0, 0, attr_game_title, 0, 0, 1, _("Star Traders"));
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

    assert(nlines > 0);
    assert(ncols > 0);
    assert(begin_y >= 0);
    assert(begin_x >= 0);

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

    if (! use_color) {
	wbkgdset(win, A_NORMAL);
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
    lines = vmkchstr(chbuf, BUFSIZE, norm_attr, alt1_attr, alt2_attr, maxlines,
		     ncols - 4, widthbuf, maxlines, format, args);
    va_end(args);

    newtxwin(usetitle ? lines + 6 : lines + 5, ncols, begin_y, begin_x,
	     true, bkgd_attr);

    if (usetitle) {
	center(curwin, 1, 0, title_attr, 0, 0, 1, boxtitle);
    }

    centerch(curwin, usetitle ? 3 : 2, 0, chbuf, lines, widthbuf);
    wait_for_key(curwin, getmaxy(curwin) - 2, keywait_attr);
    deltxwin();

    free(widthbuf);
    free(chbuf);
    return OK;
}


/***********************************************************************/
// mkchstr: Prepare a string for printing to screen

int mkchstr (chtype *restrict chbuf, int chbufsize, chtype attr_norm,
	     chtype attr_alt1, chtype attr_alt2, int maxlines, int maxwidth,
	     int *restrict widthbuf, int widthbufsize,
	     const char *restrict format, ...)
{
    va_list args;
    int lines;


    va_start(args, format);
    lines = vmkchstr(chbuf, chbufsize, attr_norm, attr_alt1, attr_alt2,
		     maxlines, maxwidth, widthbuf, widthbufsize, format, args);
    va_end(args);
    return lines;
}


/***********************************************************************/
// mkchstr_parse: Parse the format string for mkchstr()

int mkchstr_parse (const wchar_t *restrict format,
		   struct argument *restrict format_arg,
		   struct convspec *restrict format_spec, va_list args)
{
    int num_args = 0;			// 0 .. MAXFMTARGS
    int arg_num = 0;			// Current index into format_arg[]
    int specs_left = MAXFMTSPECS;	// MAXFMTSPECS .. 0 (counting down)


    memset(format_arg, 0, MAXFMTARGS * sizeof(format_arg[0]));
    memset(format_spec, 0, MAXFMTSPECS * sizeof(format_spec[0]));

    while (*format != '\0') {
	switch (*format++) {
	case '^':
	    // Switch to a different character rendition
	    if (*format == '\0') {
		errno = EINVAL;
		return -1;
	    } else {
		// Ignore next character for now
		format++;
	    }
	    break;

	case '%':
	    // Process a conversion specifier
	    if (*format == '\0') {
		errno = EINVAL;
		return -1;
	    } else if (*format == '%') {
		// Ignore "%%" specifier
		format++;
	    } else {
		const wchar_t *start = format;
		enum argument_type arg_type;
		bool inspec = true;
		bool flag_posn = false;		// Have we already seen "$"?
		bool flag_other = false;	// Have we seen something else?
		int count = 0;

		while (inspec && *format != '\0') {
		    wchar_t wc = *format++;
		    switch (wc) {
		    case '0':
			// Zero flag, or part of numeric count
			if (count == 0) {
			    // Zero flag is NOT supported
			    errno = EINVAL;
			    return -1;
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
			count = count * 10 + (wc - L'0');
			break;

		    case '$':
			// Fixed-position argument
			if (flag_posn || flag_other || count == 0) {
			    errno = EINVAL;
			    return -1;
			}

			if (count > MAXFMTARGS) {
			    errno = E2BIG;
			    return -1;
			}

			flag_posn = true;
			arg_num = count - 1;
			count = 0;
			break;

		    case '\'':
			// Use locale-specific thousands group separator
			if (format_spec->flag_group) {
			    errno = EINVAL;
			    return -1;
			}

			format_spec->flag_group = true;
			flag_other = true;
			break;

		    case '!':
			// Omit the locale-specific currency symbol
			if (format_spec->flag_nosym) {
			    errno = EINVAL;
			    return -1;
			}

			format_spec->flag_nosym = true;
			flag_other = true;
			break;

		    case '.':
			// Precision flag
			if (format_spec->flag_prec || count != 0) {
			    errno = EINVAL;
			    return -1;
			}

			format_spec->flag_prec = true;
			flag_other = true;
			break;

		    case 'l':
			// Long length modifier
			if (format_spec->flag_long) {
			    // "ll" is NOT supported
			    errno = EINVAL;
			    return -1;
			}

			format_spec->flag_long = true;
			flag_other = true;
			break;

		    case 'c':
			// Insert a character (char or wchar_t)
			if (format_spec->flag_group || format_spec->flag_nosym
			    || format_spec->flag_prec || count != 0) {
			    errno = EINVAL;
			    return -1;
			}

			arg_type = format_spec->flag_long ?
			    TYPE_WCHAR : TYPE_CHAR;
			goto handlefmt;

		    case 'd':
			// Insert an integer (int or long int)
			if (format_spec->flag_nosym || format_spec->flag_prec
			    || count != 0) {
			    errno = EINVAL;
			    return -1;
			}

			arg_type = format_spec->flag_long ?
			    TYPE_LONGINT : TYPE_INT;
			goto handlefmt;

		    case 'f':
			// Insert a floating-point number (double)
			if (format_spec->flag_nosym || format_spec->flag_long ||
			    (! format_spec->flag_prec && count != 0)) {
			    errno = EINVAL;
			    return -1;
			}

			format_spec->precision = count;

			arg_type = TYPE_DOUBLE;
			goto handlefmt;

		    case 'N':
			// Insert a monetary amount (double)
			if (format_spec->flag_group || format_spec->flag_prec
			    || format_spec->flag_long || count != 0) {
			    errno = EINVAL;
			    return -1;
			}

			arg_type = TYPE_DOUBLE;
			goto handlefmt;

		    case 's':
			// Insert a string (const char * or const wchar_t *)
			if (format_spec->flag_group || format_spec->flag_nosym
			    || format_spec->flag_prec || count != 0) {
			    errno = EINVAL;
			    return -1;
			}

			arg_type = format_spec->flag_long ?
			    TYPE_WSTRING : TYPE_STRING;

		    handlefmt:
			if (arg_num >= MAXFMTARGS || specs_left == 0) {
			    errno = E2BIG;
			    return -1;
			}

			if (format_arg[arg_num].a_type == TYPE_NONE) {
			    format_arg[arg_num].a_type = arg_type;
			} else if (format_arg[arg_num].a_type != arg_type) {
			    errno = EINVAL;
			    return -1;
			}

			format_spec->len = format - start;
			format_spec->arg_num = arg_num;
			format_spec->spec = wc;

			arg_num++;
			num_args = MAX(num_args, arg_num);

			format_spec++;
			specs_left--;

			inspec = false;
			break;

		    default:
			errno = EINVAL;
			return -1;
		    }
		}
		if (inspec) {
		    errno = EINVAL;
		    return -1;
		}
	    }
	    break;

	default:
	    // Process an ordinary character: do nothing for now
	    ;
	}
    }

    for (int i = 0; i < num_args; format_arg++, i++) {
	switch (format_arg->a_type) {
	case TYPE_CHAR:
	    format_arg->a.a_char = (char) va_arg(args, int);
	    break;

	case TYPE_WCHAR:
	    format_arg->a.a_wchar = va_arg(args, wchar_t);
	    break;

	case TYPE_INT:
	    format_arg->a.a_int = va_arg(args, int);
	    break;

	case TYPE_LONGINT:
	    format_arg->a.a_longint = va_arg(args, long int);
	    break;

	case TYPE_DOUBLE:
	    format_arg->a.a_double = va_arg(args, double);
	    break;

	case TYPE_STRING:
	    format_arg->a.a_string = va_arg(args, const char *);
	    break;

	case TYPE_WSTRING:
	    format_arg->a.a_wstring = va_arg(args, const wchar_t *);
	    break;

	default:
	    /* Cannot allow unused arguments, as we have no way of
	       knowing how much space they take (cf. int vs. long long
	       int). */
	    errno = EINVAL;
	    return -1;
	}
    }

    return 0;
}


/***********************************************************************/
// mkchstr_add: Add a character to the mkchstr buffer

int mkchstr_add (wchar_t *restrict *restrict outbuf,
		 chtype *restrict *restrict attrbuf, int *restrict count,
		 chtype attr, int maxlines, int maxwidth, int *restrict line,
		 int *restrict width, wchar_t *restrict *restrict lastspc,
		 chtype *restrict *restrict spcattr, int *restrict widthspc,
		 int *restrict widthbuf, int widthbufsize,
		 const wchar_t *restrict *restrict str)
{
    int w, wspc;


    if (*line < 0) {
	// First character in buffer: start line 0
	*line = 0;
    }

    if (**str == '\n') {
	// Start a new line

	if (*line < maxlines - 1) {
	    *(*outbuf)++ = '\n';
	    *(*attrbuf)++ = 0;
	    (*count)--;
	}

	widthbuf[*line] = *width;
	*width = 0;

	*lastspc = NULL;
	*spcattr = NULL;
	*widthspc = 0;

	(*line)++;
	(*str)++;
    } else {
	w = wcwidth(**str);
	if (w < 0) {
	    // We don't support control or non-printable characters
	    errno = EILSEQ;
	    return -1;
	}

	if (*width + w > maxwidth) {
	    // Current line would be too long to fit in **str

	    if (! iswspace(**str) && *lastspc != NULL && *line < maxlines - 1) {
		// Break on the last space in this line
		wspc = wcwidth(**lastspc);

		**lastspc = '\n';
		**spcattr = 0;

		widthbuf[*line] = *widthspc;
		*width -= *widthspc + wspc;

		*lastspc = NULL;
		*spcattr = NULL;
		*widthspc = 0;

		(*line)++;
	    } else {
		// Insert a new-line character (if not on last line)
		if (*line < maxlines - 1) {
		    *(*outbuf)++ = '\n';
		    *(*attrbuf)++ = 0;
		    (*count)--;
		}

		widthbuf[*line] = *width;
		*width = 0;

		*lastspc = NULL;
		*spcattr = NULL;
		*widthspc = 0;

		(*line)++;

		/* Skip any following spaces.  This assumes that no-one
		   will ever have combining diacritical marks following a
		   (line-breaking) space! */
		while (iswspace(**str)) {
		    if (*(*str)++ == '\n') {
			break;
		    }
		}
	    }
	} else {
	    // Insert an ordinary character into the output buffer

	    if (iswspace(**str)) {
		*lastspc = *outbuf;
		*spcattr = *attrbuf;
		*widthspc = *width;
	    }

	    *(*outbuf)++ = **str;
	    *(*attrbuf)++ = attr;
	    (*count)--;
	    *width += w;
	    (*str)++;
	}
    }

    return 0;
}


/***********************************************************************/
// mkchstr_conv: Convert (wcbuf, attrbuf) to chbuf

void mkchstr_conv (chtype *restrict chbuf, int chbufsize,
		   wchar_t *restrict wcbuf, chtype *restrict attrbuf)
{
    char convbuf[MB_LEN_MAX + 1];
    char endbuf[MB_LEN_MAX];
    mbstate_t mbstate, mbcopy;
    size_t endsize, n;
    char *p;
    bool done;


    memset(&mbstate, 0, sizeof(mbstate));
    done = false;
    while (! done) {
	// Make sure we always have enough space for ending shift sequence
	memcpy(&mbcopy, &mbstate, sizeof(mbstate));
	endsize = wcrtomb(endbuf, '\0', &mbcopy);
	if (endsize == (size_t) -1) {
	    errno_exit(_("mkchstr_conv: NUL"));
	}

	// Yes, we want to convert a wide NUL, too!
	n = xwcrtomb(convbuf, *wcbuf, &mbstate);

	if (chbufsize > endsize + n) {
	    for (p = convbuf; n > 0; n--, p++, chbuf++, chbufsize--) {
		if (*p == '\0' || *p == '\n') {
		    /* This code assumes '\n' can never appear in a
		       multibyte string except as a control character---
		       which is true of all multibyte encodings (I
		       believe!) */
		    *chbuf = (unsigned char) *p;
		} else {
		    *chbuf = (unsigned char) *p | *attrbuf;
		}
	    }
	} else {
	    // Not enough space for *wcbuf, so terminate chbuf early
	    for (p = endbuf; endsize > 0; endsize--, p++, chbuf++) {
		*chbuf = (unsigned char) *p;
	    }
	    break;
	}

	done = (*wcbuf == '\0');
	wcbuf++;
	attrbuf++;
    }
}


/***********************************************************************/
// vmkchstr: Prepare a string for printing to screen

int vmkchstr (chtype *restrict chbuf, int chbufsize, chtype attr_norm,
	      chtype attr_alt1, chtype attr_alt2, int maxlines, int maxwidth,
	      int *restrict widthbuf, int widthbufsize,
	      const char *restrict format, va_list args)
{
    struct argument format_arg[MAXFMTARGS];
    struct convspec format_spec[MAXFMTSPECS];
    struct convspec *spec;
    const wchar_t *wcformat;
    wchar_t *orig_wcformat;

    wchar_t *outbuf, *orig_outbuf;
    chtype *attrbuf, *orig_attrbuf;
    wchar_t *fmtbuf;

    int count, line, width;
    wchar_t *lastspc;
    chtype *spcattr;
    int widthspc;
    chtype curattr;
    int saved_errno;


    assert(chbuf != NULL);
    assert(chbufsize > 0);
    assert(chbufsize <= BUFSIZE);
    assert(maxlines > 0);
    assert(maxwidth > 0);
    assert(widthbuf != NULL);
    assert(widthbufsize >= maxlines);
    assert(format != NULL);

    outbuf = orig_outbuf = xmalloc(BUFSIZE * sizeof(wchar_t));
    attrbuf = orig_attrbuf = xmalloc(BUFSIZE * sizeof(chtype));
    wcformat = orig_wcformat = xmalloc(chbufsize * sizeof(wchar_t));
    fmtbuf = xmalloc(BUFSIZE * sizeof(wchar_t));

    // Convert format to a wide-character string
    xmbstowcs(orig_wcformat, format, BUFSIZE);

    if (mkchstr_parse(wcformat, format_arg, format_spec, args) < 0) {
	goto error;
    }

    // Construct the (outbuf, attrbuf) pair of arrays

    spec = format_spec;

    curattr = attr_norm;
    count = BUFSIZE;			// Space left in outbuf
    line = -1;				// Current line number (0 = first)
    width = 0;				// Width of the current line

    lastspc = NULL;			// Pointer to last space in line
    spcattr = NULL;			// Equivalent in attrbuf
    widthspc = 0;			// Width of line before last space

    while (*wcformat != '\0' && count > 1 && line < maxlines) {
	switch (*wcformat) {
	case '^':
	    // Switch to a different character rendition
	    if (*++wcformat == '\0') {
		goto error_inval;
	    } else {
		switch (*wcformat) {
		case '^':
		    if (mkchstr_add(&outbuf, &attrbuf, &count, curattr,
				    maxlines, maxwidth, &line, &width,
				    &lastspc, &spcattr, &widthspc, widthbuf,
				    widthbufsize, &wcformat) < 0) {
			goto error;
		    }
		    break;

		case '{':
		    curattr = attr_alt1;
		    wcformat++;
		    break;

		case '[':
		    curattr = attr_alt2;
		    wcformat++;
		    break;

		case '}':
		case ']':
		    curattr = attr_norm;
		    wcformat++;
		    break;

		default:
		    goto error_inval;
		}
	    }
	    break;

	case '%':
	    // Process a conversion specifier
	    if (*++wcformat == '\0') {
		goto error_inval;
	    } else if (*wcformat == '%') {
		if (mkchstr_add(&outbuf, &attrbuf, &count, curattr, maxlines,
				maxwidth, &line, &width, &lastspc, &spcattr,
				&widthspc, widthbuf, widthbufsize, &wcformat)
		    < 0) {
		    goto error;
		}
	    } else {
		assert(spec->len != 0);
		const wchar_t *str;
		wint_t wc;

		switch (spec->spec) {
		case 'c':
		    // Insert a character (char or wchar_t) into the output
		    if (spec->flag_long) {
			wc = format_arg[spec->arg_num].a.a_wchar;
		    } else {
			wc = btowc(format_arg[spec->arg_num].a.a_char);
		    }

		    if (wc == '\0' || wc == WEOF) {
			wc = EILSEQ_REPL_WC;
		    }

		    fmtbuf[0] = wc;
		    fmtbuf[1] = '\0';

		    str = fmtbuf;
		    goto insertstr;

		case 'd':
		    // Insert an integer (int or long int) into the output
		    if (spec->flag_long) {
			if (swprintf(fmtbuf, BUFSIZE, spec->flag_group ?
				     L"%'ld" : L"%ld",
				     format_arg[spec->arg_num].a.a_longint) < 0)
			    goto error;
		    } else {
			if (swprintf(fmtbuf, BUFSIZE, spec->flag_group ?
				     L"%'d" : L"%d",
				     format_arg[spec->arg_num].a.a_int) < 0)
			    goto error;
		    }

		    str = fmtbuf;
		    goto insertstr;

		case 'f':
		    // Insert a floating-point number (double) into the output
		    if (spec->flag_prec) {
			if (swprintf(fmtbuf, BUFSIZE, spec->flag_group ?
				     L"%'.*f" : L"%.*f", spec->precision,
				     format_arg[spec->arg_num].a.a_double) < 0)
			    goto error;
		    } else {
			if (swprintf(fmtbuf, BUFSIZE, spec->flag_group ?
				     L"%'f" : L"%f",
				     format_arg[spec->arg_num].a.a_double) < 0)
			    goto error;
		    }

		    str = fmtbuf;
		    goto insertstr;

		case 'N':
		    // Insert a monetary amount (double) into the output
		    {
			/* strfmon() is not available in a wide-char
			   version, so we need a multibyte char buffer */
			char *buf = xmalloc(BUFSIZE);

			if (l_strfmon(buf, BUFSIZE, spec->flag_nosym ? "%!n" : "%n",
				      format_arg[spec->arg_num].a.a_double) < 0) {
			    saved_errno = errno;
			    free(buf);
			    errno = saved_errno;
			    goto error;
			}

			xmbstowcs(fmtbuf, buf, BUFSIZE);
			free(buf);
		    }

		    str = fmtbuf;
		    goto insertstr;

		case 's':
		    // Insert a string (const char * or const wchar_t *)
		    if (spec->flag_long) {
			str = format_arg[spec->arg_num].a.a_wstring;
		    } else {
			const char *p = format_arg[spec->arg_num].a.a_string;
			if (p == NULL) {
			    str = NULL;
			} else {
			    xmbstowcs(fmtbuf, p, BUFSIZE);
			    str = fmtbuf;
			}
		    }

		    if (str == NULL) {
			str = L"(NULL)";	// As per GNU printf()
		    }

		insertstr:
		    // Insert the string pointed to by str
		    while (*str != '\0' && count > 1 && line < maxlines) {
			if (mkchstr_add(&outbuf, &attrbuf, &count, curattr,
					maxlines, maxwidth, &line, &width,
					&lastspc, &spcattr, &widthspc,
					widthbuf, widthbufsize, &str) < 0) {
			    goto error;
			}
		    }

		    wcformat += spec->len;
		    spec++;
		    break;

		default:
		    assert(spec->spec);
		}
	    }
	    break;

	default:
	    // Process an ordinary character (including new-line)
	    if (mkchstr_add(&outbuf, &attrbuf, &count, curattr, maxlines,
			    maxwidth, &line, &width, &lastspc, &spcattr,
			    &widthspc, widthbuf, widthbufsize, &wcformat) < 0) {
		goto error;
	    }
	}
    }

    *outbuf = '\0';			// Terminating NUL character
    *attrbuf = 0;

    if (line >= 0 && line < maxlines) {
	widthbuf[line] = width;
    } else if (line >= maxlines) {
	line = maxlines - 1;
    }

    // Convert the (outbuf, attrbuf) pair of arrays to chbuf
    mkchstr_conv(chbuf, chbufsize, orig_outbuf, orig_attrbuf);

    free(fmtbuf);
    free(orig_wcformat);
    free(orig_attrbuf);
    free(orig_outbuf);

    return line + 1;


error_inval:
    errno = EINVAL;

error:
    saved_errno = errno;
    free(fmtbuf);
    free(orig_wcformat);
    free(orig_attrbuf);
    free(orig_outbuf);
    errno = saved_errno;

    errno_exit(_("mkchstr: `%s'"), format);
}


/***********************************************************************/
// leftch: Print strings in chstr left-aligned

int leftch (WINDOW *win, int y, int x, const chtype *restrict chstr,
	    int lines, const int *restrict widthbuf)
{
    assert(win != NULL);
    assert(chstr != NULL);
    assert(lines > 0);
    assert(widthbuf != NULL);

    wmove(win, y, x);
    for ( ; *chstr != '\0'; chstr++) {
	if (*chstr == '\n') {
	    wmove(win, getcury(win) + 1, x);
	} else {
	    waddch(win, *chstr);
	}
    }

    return OK;
}


/***********************************************************************/
// centerch: Print strings in chstr centred in window

int centerch (WINDOW *win, int y, int offset, const chtype *restrict chstr,
	      int lines, const int *restrict widthbuf)
{
    int ln = 0;


    assert(win != NULL);
    assert(chstr != NULL);
    assert(lines > 0);
    assert(widthbuf != NULL);

    wmove(win, y, (getmaxx(win) - widthbuf[ln]) / 2 + offset);
    for ( ; *chstr != '\0'; chstr++) {
	if (*chstr == '\n') {
	    if (ln++ >= lines) {
		return ERR;
	    } else {
		wmove(win, getcury(win) + 1,
		      (getmaxx(win) - widthbuf[ln]) / 2 + offset);
	    }
	} else {
	    waddch(win, *chstr);
	}
    }

    return OK;
}


/***********************************************************************/
// rightch: Print strings in chstr right-aligned

int rightch (WINDOW *win, int y, int x, const chtype *restrict chstr,
	     int lines, const int *restrict widthbuf)
{
    int ln = 0;


    assert(win != NULL);
    assert(chstr != NULL);
    assert(lines > 0);
    assert(widthbuf != NULL);

    wmove(win, y, x - widthbuf[ln]);
    for ( ; *chstr != '\0'; chstr++) {
	if (*chstr == '\n') {
	    if (ln++ >= lines) {
		return ERR;
	    } else {
		wmove(win, getcury(win) + 1, x - widthbuf[ln]);
	    }
	} else {
	    waddch(win, *chstr);
	}
    }

    return OK;
}


/***********************************************************************/
// left: Print strings left-aligned

int left (WINDOW *win, int y, int x, chtype attr_norm, chtype attr_alt1,
	  chtype attr_alt2, int maxlines, const char *restrict format, ...)
{
    va_list args;
    chtype *chbuf;
    int *widthbuf;
    int lines;
    int ret;


    assert(maxlines > 0);

    chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    widthbuf = xmalloc(maxlines * sizeof(int));

    va_start(args, format);
    lines = vmkchstr(chbuf, BUFSIZE, attr_norm, attr_alt1, attr_alt2, maxlines,
		     getmaxx(win) - x - 2, widthbuf, maxlines, format, args);
    ret = leftch(win, y, x, chbuf, lines, widthbuf);
    assert(ret == OK);
    va_end(args);

    free(widthbuf);
    free(chbuf);
    return lines;
}


/***********************************************************************/
// center: Print strings centred in window

int center (WINDOW *win, int y, int offset, chtype attr_norm, chtype attr_alt1,
	    chtype attr_alt2, int maxlines, const char *restrict format, ...)
{
    va_list args;
    chtype *chbuf;
    int *widthbuf;
    int lines;
    int ret;


    assert(maxlines > 0);

    chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    widthbuf = xmalloc(maxlines * sizeof(int));

    va_start(args, format);
    lines = vmkchstr(chbuf, BUFSIZE, attr_norm, attr_alt1, attr_alt2, maxlines,
		     getmaxx(win) - 4, widthbuf, maxlines, format, args);
    ret = centerch(win, y, offset, chbuf, lines, widthbuf);
    assert(ret == OK);
    va_end(args);

    free(widthbuf);
    free(chbuf);
    return lines;
}


/***********************************************************************/
// right: Print strings right-aligned

int right (WINDOW *win, int y, int x, chtype attr_norm, chtype attr_alt1,
	   chtype attr_alt2, int maxlines, const char *restrict format, ...)
{
    va_list args;
    chtype *chbuf;
    int *widthbuf;
    int lines;
    int ret;


    assert(maxlines > 0);

    chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    widthbuf = xmalloc(maxlines * sizeof(int));

    va_start(args, format);
    lines = vmkchstr(chbuf, BUFSIZE, attr_norm, attr_alt1, attr_alt2, maxlines,
		     x - 2, widthbuf, maxlines, format, args);
    ret = rightch(win, y, x, chbuf, lines, widthbuf);
    assert(ret == OK);
    va_end(args);

    free(widthbuf);
    free(chbuf);
    return lines;
}


/***********************************************************************/
// getwch: Get a wide character from the keyboard

/* There are two implementations of getwch(): one used if enhanced Curses
   is present (with wide-character functions), the other if only
   single-byte functions are available. */

#if defined(HAVE_CURSES_ENHANCED) || defined(HAVE_NCURSESW)

int getwch (WINDOW *win, wint_t *wch)
{
    int ret = wget_wch(win, wch);

    if (ret == OK) {
	char c = wctob(*wch);
	if ((c >= 0 && c < ' ') || c == 0x7F) {
	    /* Make control characters (and DEL) appear to be similar to
	       function keys.  This assumes the KEY_xxx definitions do
	       not overlap, which is true of all Curses implementations
	       (due to the same codes being returned by getch()).  We do
	       not use iswcntrl() as certain additional Unicode
	       characters are also control characters (eg, U+2028) */
	    ret = KEY_CODE_YES;
	}
    }

    return ret;
}

#else // !defined(HAVE_CURSES_ENHANCED) && !defined(HAVE_NCURSESW)

int getwch (WINDOW *win, wint_t *wch)
{
#error "Single-byte version of getwch() not yet implemented"
"Single-byte version of getwch() not yet implemented"
}

#endif // !defined(HAVE_CURSES_ENHANCED) && !defined(HAVE_NCURSESW)


/***********************************************************************/
// gettxchar: Read a character from the keyboard

int gettxchar (WINDOW *win, wint_t *wch)
{
    int ret;


    assert(win != NULL);
    assert(wch != NULL);

    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    while (true) {
	ret = getwch(win, wch);
	if (ret == OK) {
	    break;
	} else if (ret == KEY_CODE_YES) {

#ifdef HANDLE_RESIZE_EVENTS
	    if (*wch == KEY_RESIZE) {
		txresize();
	    } else {
		break;
	    }
#else // ! HANDLE_RESIZE_EVENTS
	    break;
#endif // ! HANDLE_RESIZE_EVENTS

	} else {
	    // ret == ERR
	    beep();
	}
    }

    return ret;
}


/***********************************************************************/
// cpos_endl: Adjust cpos and st for printing the ending part of buf

void cpos_endl (wchar_t *restrict buf, int *restrict cpos, int *restrict st,
		int clen, int width, int len)
{
    *cpos = MIN(clen, width - 1);

    if (clen <= width - 1) {
	// Whole string can be displayed
	*st = 0;
    } else {
	// String too long: figure out offset from which to print (value of st)
	int i = width - 1;
	*st = len;
	while (i > 0) {
	    i -= wcwidth(buf[--(*st)]);
	}
	if (i < 0) {
	    /* Don't truncate a double-width character if the second half
	       would appear in the first column position. */
	    i += wcwidth(buf[(*st)++]);
	    while (wcwidth(buf[*st]) == 0) {
		// Skip over zero-width characters (mostly combining ones)
		(*st)++;
	    }
	    *cpos -= i;
	}
    }

    assert(*st >= 0);
}


/***********************************************************************/
// cpos_decr: Adjust cpos and st: scroll to the left by w columns

void cpos_decr (wchar_t *restrict buf, int *restrict cpos, int *restrict st,
		int w, int width)
{
    if (*cpos > 0) {
	// Cursor position is not yet in first column
	*cpos -= w;
    } else if (*st > 0) {
	(*st)--;
	if (w == 0) {
	    /* Make sure zero-width characters (esp. combining ones) do
	       not appear without the associated base character */
	    w = wcwidth(buf[*st]);
	    while (*st > 0 && w == 0) {
		w = wcwidth(buf[--(*st)]);
	    }
	    *cpos = w;
	}
    }
}


/***********************************************************************/
// cpos_incr: Adjust cpos and st: scroll to the right by w columns

void cpos_incr (wchar_t *restrict buf, int *restrict cpos, int *restrict st,
		int w, int width)
{
    if (*cpos + w <= width - 1) {
	// Cursor position is not yet in second-last column
	*cpos += w;
    } else {
	int i = 0;
	while (i < w) {
	    i += wcwidth(buf[(*st)++]);
	}
	while (wcwidth(buf[*st]) == 0) {
	    // Skip over zero-width characters (mainly combining ones)
	    (*st)++;
	}
	if (i > w) {
	    // Take double-width characters into account
	    *cpos -= i - w;
	}
    }
}


/***********************************************************************/
// gettxline: Read a line of input from the keyboard (low-level)

int gettxline (WINDOW *win, wchar_t *restrict buf, int bufsize,
	       bool *restrict modified, bool multifield,
	       const wchar_t *emptyval, const wchar_t *defaultval,
	       const wchar_t *allowed, bool stripspc, int y, int x,
	       int width, chtype attr)
{
    bool done, redraw, mod;
    int len, pos, st;
    int clen, cpos;
    int rcode, ret;
    wint_t key;
    chtype oldattr;
    chtype *chbuf;
    int chbufwidth;


    assert(win != NULL);
    assert(buf != NULL);
    assert(bufsize > 2);
    assert(width > 2);

    chbuf = xmalloc(BUFSIZE * sizeof(chtype));

    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    oldattr = getattrs(win);
    curs_set(CURS_ON);

    len = wcslen(buf);			// len is number of wide chars in buf
    pos = len;				// pos (string position): 0 to len
    clen = wcswidth(buf, bufsize);	// clen is number of column positions

    if (clen < 0) {
	err_exit("gettxline: illegal character in string: `%ls'", buf);
    }

    // Find the point from which buf should be displayed to screen
    cpos_endl(buf, &cpos, &st, clen, width, len);

    redraw = true;
    done = false;
    mod = false;
    ret = OK;

    while (! done) {
	if (redraw) {
	    /* Redisplay the visible part of the current input string.
	       Blanks at the end of the input area are replaced with
	       "attr", which may contain a '_' for non-colour mode. */
	    mvwhline(win, y, x, ((attr & A_CHARTEXT) == 0) ?
		     ' ' | attr : attr, width);
	    mkchstr(chbuf, BUFSIZE, attr & ~A_CHARTEXT, 0, 0, 1, width,
		    &chbufwidth, 1, "%ls", buf + st);
	    leftch(win, y, x, chbuf, 1, &chbufwidth);

	    wmove(win, y, x + cpos);
	    wrefresh(win);
	}

	rcode = getwch(win, &key);

	if (rcode == OK) {
	    // Ordinary wide character

	    if ((key == CHAR_DEFVAL1 || key == CHAR_DEFVAL2)
		&& defaultval != NULL && len == 0) {
		// Initialise buffer with the default value

		wcsncpy(buf, defaultval, bufsize - 1);
		buf[bufsize - 1] = L'\0';

		len = wcslen(buf);
		pos = len;
		clen = wcswidth(buf, bufsize);

		if (clen == -1) {
		    err_exit("gettxline: illegal character in string: `%ls'",
			     buf);
		}

		cpos_endl(buf, &cpos, &st, clen, width, len);

		mod = true;
		redraw = true;

	    } else if (len >= bufsize - 1
		       || (allowed != NULL && wcschr(allowed, key) == NULL)) {
		beep();

	    } else {
		// Process an ordinary character: insert it into the string
		int w = wcwidth(key);

		if (w < 0) {
		    // Non-printing character
		    beep();

		} else {
		    wmemmove(buf + pos + 1, buf + pos, len - pos + 1);
		    buf[pos] = (wchar_t) key;
		    len++;
		    pos++;

		    clen += w;
		    cpos_incr(buf, &cpos, &st, w, width);

		    mod = true;
		    redraw = true;
		}
	    }

	} else if (rcode == KEY_CODE_YES) {
	    // Function or control key
	    switch (key) {

	    // Terminating keys

	    case KEY_RETURN:
	    case KEY_ENTER:
	    case KEY_CTRL('M'):
		// Finish entering the string

		if (stripspc) {
		    int i;

		    // Strip leading spaces
		    for (i = 0; i < len && iswspace(buf[i]); i++)
			;
		    if (i > 0) {
			wmemmove(buf, buf + i, len - i + 1);
			len -= i;
			mod = true;
		    }

		    // Strip trailing spaces
		    for (i = len; i > 0 && iswspace(buf[i - 1]); i--)
			;
		    if (i < len) {
			buf[i] = L'\0';
			len = i;
			mod = true;
		    }
		}

		if (emptyval != NULL && len == 0) {
		    wcsncpy(buf, emptyval, bufsize - 1);
		    buf[bufsize - 1] = L'\0';
		    mod = true;
		}

		ret = OK;
		done = true;
		break;

	    case KEY_CANCEL:
	    case KEY_EXIT:
	    case KEY_CTRL('C'):
	    case KEY_CTRL('G'):
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
		    cpos_decr(buf, &cpos, &st, wcwidth(buf[pos]), width);
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
		    cpos_incr(buf, &cpos, &st, wcwidth(buf[pos - 1]), width);
		    redraw = true;
		}
		break;

	    case KEY_HOME:
	    case KEY_CTRL('A'):
		// Move cursor to start of string
		pos = 0;
		cpos = 0;
		st = 0;
		redraw = true;
		break;

	    case KEY_END:
	    case KEY_CTRL('E'):
		// Move cursor to end of string
		pos = len;
		cpos_endl(buf, &cpos, &st, clen, width, len);
		redraw = true;
		break;

	    case KEY_CLEFT:
		// Move cursor to start of current or previous word
		while (pos > 0 && ! iswalnum(buf[pos - 1])) {
		    pos--;
		    cpos_decr(buf, &cpos, &st, wcwidth(buf[pos]), width);
		}
		while (pos > 0 && (iswalnum(buf[pos - 1])
				   || (pos > 1 && wcwidth(buf[pos - 1]) == 0
				       && iswalnum(buf[pos - 2])))) {
		    /* Treat zero-width characters preceded by an
		       alphanumeric character as alphanumeric. */
		    pos--;
		    cpos_decr(buf, &cpos, &st, wcwidth(buf[pos]), width);
		}
		redraw = true;
		break;

	    case KEY_CRIGHT:
		// Move cursor to end of current or next word
		while (pos < len && ! iswalnum(buf[pos])) {
		    pos++;
		    cpos_incr(buf, &cpos, &st, wcwidth(buf[pos - 1]), width);
		}
		while (pos < len
		       && (iswalnum(buf[pos]) || wcwidth(buf[pos]) == 0)) {
		    /* Treat zero-width characters following an
		       alphanumeric character as alphanumeric. */
		    pos++;
		    cpos_incr(buf, &cpos, &st, wcwidth(buf[pos - 1]), width);
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
		    int w = wcwidth(buf[pos - 1]);
		    wmemmove(buf + pos - 1, buf + pos, len - pos + 1);
		    len--;
		    pos--;
		    clen -= w;
		    cpos_decr(buf, &cpos, &st, w, width);
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
		    int w = wcwidth(buf[pos]);
		    wmemmove(buf + pos, buf + pos + 1, len - pos);
		    len--;
		    clen -= w;
		    // pos, cpos and st stay the same
		    mod = true;
		    redraw = true;
		}
		break;

	    case KEY_CLEAR:
		// Delete the entire line
		wcscpy(buf, L"");
		len = 0;
		pos = 0;
		clen = 0;
		cpos = 0;
		st = 0;
		mod = true;
		redraw = true;
		break;

	    case KEY_CTRL('U'):
		// Delete backwards to the start of the line
		if (pos == 0) {
		    beep();
		} else {
		    int i, ww;
		    for (i = 0, ww = 0; i < pos; i++) {
			ww += wcwidth(buf[i]);
		    }

		    wmemmove(buf, buf + pos, len - pos + 1);
		    len -= pos;
		    pos = 0;
		    clen -= ww;
		    cpos = 0;
		    st = 0;
		    mod = true;
		    redraw = true;
		}
		break;

	    case KEY_CTRL('K'):
		// Delete to the end of the line
		if (pos == len) {
		    beep();
		} else {
		    int i, ww;
		    for (i = pos, ww = 0; i < len; i++) {
			ww += wcwidth(buf[i]);
		    }

		    buf[pos] = L'\0';
		    len = pos;
		    clen -= ww;
		    // pos, cpos and st stay the same
		    mod = true;
		    redraw = true;
		}
		break;

	    case KEY_CTRL('W'):
		// Delete the previous word
		if (pos == 0) {
		    beep();
		} else {
		    /* Note the use of iswspace() instead of iswalnum():
		       this makes ^W follow GNU Bash standards, which
		       behaves differently from Meta-DEL. */
		    int i = pos;
		    int ww = 0;
		    while (i > 0 && iswspace(buf[i - 1])) {
			i--;
			int w = wcwidth(buf[i]);
			ww += w;
			cpos_decr(buf, &cpos, &st, w, width);
		    }
		    while (i > 0 && ! iswspace(buf[i - 1])) {
			i--;
			int w = wcwidth(buf[i]);
			ww += w;
			cpos_decr(buf, &cpos, &st, w, width);
		    }

		    wmemmove(buf + i, buf + pos, len - pos + 1);
		    len -= pos - i;
		    pos = i;
		    clen -= ww;
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
		    wchar_t c = buf[pos - 1];
		    buf[pos - 1] = buf[pos - 2];
		    buf[pos - 2] = c;
		    mod = true;
		    redraw = true;
		} else {
		    wchar_t c = buf[pos];
		    int w = wcwidth(c);

		    buf[pos] = buf[pos - 1];
		    buf[pos - 1] = c;

		    pos++;
		    cpos_incr(buf, &cpos, &st, w, width);

		    mod = true;
		    redraw = true;
		}
		break;

	    case KEY_ESC:
		// Handle Meta-X-style and other function key presses
		wtimeout(win, META_TIMEOUT);
		rcode = getwch(win, &key);

		if (rcode == OK) {
		    // Ordinary wide character

		    // Swallow any unknown VT100-style function keys
		    if (key == L'O' || key == L'[') {
			rcode = getwch(win, &key);
			while (rcode == OK
			       && wcschr(L"0123456789;", key) != NULL
			       && wcschr(L"~ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					 L"abcdefghijklmnopqrstuvwxyz", key)
			       == NULL) {
			    rcode = getwch(win, &key);
			}
			beep();

		    } else {
			// Handle Meta-X-style keypress
			switch (key) {

			// Cursor movement keys

			case L'B':
			case L'b':
			    // Move cursor to start of current or previous word
			    while (pos > 0 && ! iswalnum(buf[pos - 1])) {
				pos--;
				cpos_decr(buf, &cpos, &st, wcwidth(buf[pos]),
					  width);
			    }
			    while (pos > 0 && (iswalnum(buf[pos - 1])
					       || (pos > 1
						   && wcwidth(buf[pos - 1]) == 0
						   && iswalnum(buf[pos - 2])))) {
				/* Treat zero-width characters preceded by an
				   alphanumeric character as alphanumeric. */
				pos--;
				cpos_decr(buf, &cpos, &st, wcwidth(buf[pos]),
					  width);
			    }
			    redraw = true;
			    break;

			case L'F':
			case L'f':
			    // Move cursor to end of current or next word
			    while (pos < len && ! iswalnum(buf[pos])) {
				pos++;
				cpos_incr(buf, &cpos, &st,
					  wcwidth(buf[pos - 1]), width);
			    }
			    while (pos < len
				   && (iswalnum(buf[pos])
				       || wcwidth(buf[pos]) == 0)) {
				/* Treat zero-width characters following an
				   alphanumeric character as alphanumeric. */
				pos++;
				cpos_incr(buf, &cpos, &st,
					  wcwidth(buf[pos - 1]), width);
			    }
			    redraw = true;
			    break;

			// Deletion keys

			case L'D':
			case L'd':
			    // Delete the next word
			    {
				int i = pos;
				int ww = 0;
				while (i < len && ! iswalnum(buf[i])) {
				    i++;
				    ww += wcwidth(buf[i - 1]);
				}
				while (i < len
				       && (iswalnum(buf[i])
					   || wcwidth(buf[pos]) == 0)) {
				    /* Treat zero-width characters
				       following an alphanumeric
				       character as alphanumeric. */
				    i++;
				    ww += wcwidth(buf[i - 1]);
				}

				wmemmove(buf + pos, buf + i, len - i + 1);
				len -= (i - pos);
				clen -= ww;
				// pos, cpos and st stay the same
				mod = true;
				redraw = true;
			    }
			    break;

			case L'\\':
			case L' ':
			    // Delete all surrounding spaces; if key == ' ',
			    // also insert one space
			    {
				int i = pos;
				int ww = 0;
				while (pos > 0 && iswspace(buf[pos - 1])) {
				    pos--;
				    int w = wcwidth(buf[pos]);
				    ww += w;
				    cpos_decr(buf, &cpos, &st, w, width);
				}
				while (i < len && iswspace(buf[i])) {
				    i++;
				    ww += wcwidth(buf[i - 1]);
				}

				wmemmove(buf + pos, buf + i, len - i + 1);
				len -= (i - pos);
				clen -= ww;

				if (key == L' ') {
				    if (len >= bufsize - 1 || (allowed != NULL
					&& wcschr(allowed, key) == NULL)) {
					beep();
				    } else {
					wchar_t c = L' ';
					wmemmove(buf + pos + 1, buf + pos,
						 len - pos + 1);
					buf[pos] = c;
					len++;
					pos++;

					int w = wcwidth(c);
					clen += w;
					cpos_incr(buf, &cpos, &st, w, width);
				    }
				}

				mod = true;
				redraw = true;
			    }
			    break;

			// Transformation keys

			case L'U':
			case L'u':
			    // Convert word (from cursor onwards) to upper case
			    while (pos < len && ! iswalnum(buf[pos])) {
				pos++;
				cpos_incr(buf, &cpos, &st,
					  wcwidth(buf[pos - 1]), width);
			    }
			    while (pos < len
				   && (iswalnum(buf[pos])
				       || wcwidth(buf[pos]) == 0)) {
				buf[pos] = towupper(buf[pos]);
				pos++;
				cpos_incr(buf, &cpos, &st,
					  wcwidth(buf[pos - 1]), width);
			    }
			    mod = true;
			    redraw = true;
			    break;

			case L'L':
			case L'l':
			    // Convert word (from cursor onwards) to lower case
			    while (pos < len && ! iswalnum(buf[pos])) {
				pos++;
				cpos_incr(buf, &cpos, &st,
					  wcwidth(buf[pos - 1]), width);
			    }
			    while (pos < len
				   && (iswalnum(buf[pos])
				       || wcwidth(buf[pos]) == 0)) {
				buf[pos] = towlower(buf[pos]);
				pos++;
				cpos_incr(buf, &cpos, &st,
					  wcwidth(buf[pos - 1]), width);
			    }
			    mod = true;
			    redraw = true;
			    break;

			case L'C':
			case L'c':
			    // Convert current letter to upper case,
			    // following letters to lower case
			    {
				bool first = true;
				while (pos < len && ! iswalnum(buf[pos])) {
				    pos++;
				    cpos_incr(buf, &cpos, &st,
					      wcwidth(buf[pos - 1]), width);
				}
				while (pos < len
				       && (iswalnum(buf[pos])
					   || wcwidth(buf[pos]) == 0)) {
				    if (first) {
					buf[pos] = towupper(buf[pos]);
					first = false;
				    } else {
					buf[pos] = towlower(buf[pos]);
				    }
				    pos++;
				    cpos_incr(buf, &cpos, &st,
					      wcwidth(buf[pos - 1]), width);
				}
				mod = true;
				redraw = true;
			    }
			    break;

			default:
			    beep();
			}
		    }

		} else if (rcode == KEY_CODE_YES) {
		    // Function or control key (with preceding Meta key)

		    switch (key) {

		    // Deletion keys

		    case KEY_BS:
		    case KEY_BACKSPACE:
		    case KEY_DEL:
			// Delete the previous word (different from ^W)
			{
			    int ww = 0;
			    int i = pos;
			    while (i > 0 && ! iswalnum(buf[i - 1])) {
				i--;
				int w = wcwidth(buf[i]);
				ww += w;
				cpos_decr(buf, &cpos, &st, w, width);
			    }
			    while (i > 0
				   && (iswalnum(buf[i - 1])
				       || (i > 1 && wcwidth(buf[i - 1]) == 0
					   && iswalnum(buf[i - 2])))) {
				/* Treat zero-width characters preceded by an
				   alphanumeric character as alphanumeric. */
				i--;
				int w = wcwidth(buf[i]);
				ww += w;
				cpos_decr(buf, &cpos, &st, w, width);
			    }

			    wmemmove(buf + i, buf + pos, len - pos + 1);
			    len -= (pos - i);
			    pos = i;
			    clen -= ww;
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

		} else {
		    /* rcode == ERR (timeout): <ESC> by itself, so cancel
		       entering the string. */
		    ret = ERR;
		    done = true;
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
	} else {
	    // rcode == ERR: Do nothing
	    ;
	}
    }

    curs_set(CURS_OFF);

    mvwhline(win, y, x, ' ' | oldattr, width);
    mkchstr(chbuf, BUFSIZE, oldattr | A_BOLD, 0, 0, 1, width,
	    &chbufwidth, 1, "%ls", buf);
    leftch(win, y, x, chbuf, 1, &chbufwidth);
    wrefresh(win);

    if (modified != NULL) {
	*modified = mod;
    }

    free(chbuf);
    return ret;
}


/***********************************************************************/
// gettxstr: Read a string from the keyboard

int gettxstr (WINDOW *win, wchar_t *restrict *restrict bufptr,
	      bool *restrict modified, bool multifield, int y, int x,
	      int width, chtype attr)
{
    assert(bufptr != NULL);


    // Allocate the result buffer if needed
    if (*bufptr == NULL) {
	*bufptr = xmalloc(BUFSIZE * sizeof(wchar_t));
	**bufptr = '\0';
    }

    return gettxline(win, *bufptr, BUFSIZE, modified, multifield, L"", L"",
		     NULL, true, y, x, width, attr);
}


/***********************************************************************/
// txinput_fixup: Copy strings with fixup

void txinput_fixup (wchar_t *restrict dest, wchar_t *restrict src, bool isfloat)
{
    assert(src != NULL);
    assert(dest != NULL);

    wcsncpy(dest, src, BUFSIZE - 1);
    dest[BUFSIZE - 1] = '\0';

    // Replace mon_decimal_point with decimal_point if these are different
    if (isfloat) {
	if (*mon_decimal_point != L'\0' && *decimal_point != L'\0'
	    && wcscmp(mon_decimal_point, decimal_point) != 0) {

	    int len_dp  = wcslen(decimal_point);
	    int len_mdp = wcslen(mon_decimal_point);
	    wchar_t *p;

	    while ((p = wcsstr(dest, mon_decimal_point)) != NULL) {
		// Make space for decimal_point, if needed
		if (len_mdp != len_dp) {
		    wmemmove(p + len_dp, p + len_mdp,
			     wcslen(p) - (len_dp - len_mdp) + 1);
		}

		// Copy decimal_point over p WITHOUT copying ending NUL
		for (wchar_t *pn = decimal_point; *pn != L'\0'; pn++, p++) {
		    *p = *pn;
		}
	    }
	}
    }

    // Remove thousands separators if required
    if (*thousands_sep != L'\0') {
	int len = wcslen(thousands_sep);
	wchar_t *p;

	while ((p = wcsstr(dest, thousands_sep)) != NULL) {
	    wmemmove(p, p + len, wcslen(p) - len + 1);
	}
    }
    if (*mon_thousands_sep != L'\0') {
	int len = wcslen(mon_thousands_sep);
	wchar_t *p;

	while ((p = wcsstr(dest, mon_thousands_sep)) != NULL) {
	    wmemmove(p, p + len, wcslen(p) - len + 1);
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

    wchar_t *buf, *bufcopy;
    wchar_t *allowed, *emptystr, *defaultstr;
    double val;
    bool done;
    int ret;


    assert(result != NULL);
    assert(min <= max);

    buf        = xmalloc(BUFSIZE * sizeof(wchar_t));
    bufcopy    = xmalloc(BUFSIZE * sizeof(wchar_t));
    allowed    = xmalloc(BUFSIZE * sizeof(wchar_t));
    emptystr   = xmalloc(BUFSIZE * sizeof(wchar_t));
    defaultstr = xmalloc(BUFSIZE * sizeof(wchar_t));

    *buf = L'\0';

    wcscpy(allowed,  L"0123456789+-Ee");
    wcsncat(allowed, decimal_point,     BUFSIZE - wcslen(allowed) - 1);
    wcsncat(allowed, thousands_sep,     BUFSIZE - wcslen(allowed) - 1);
    wcsncat(allowed, mon_decimal_point, BUFSIZE - wcslen(allowed) - 1);
    wcsncat(allowed, mon_thousands_sep, BUFSIZE - wcslen(allowed) - 1);

    swprintf(emptystr,   BUFSIZE, L"%'1.*f", lc->frac_digits, emptyval);
    swprintf(defaultstr, BUFSIZE, L"%'1.*f", lc->frac_digits, defaultval);

    done = false;
    while (! done) {
	ret = gettxline(win, buf, BUFSIZE, NULL, false, emptystr, defaultstr,
			allowed, true, y, x, width, attr);

	if (ret == OK) {
	    wchar_t *p;

	    txinput_fixup(bufcopy, buf, true);
	    val = wcstod(bufcopy, &p);

	    if (*p == L'\0' && val >= min && val <= max) {
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
    wchar_t *buf, *bufcopy;
    wchar_t *allowed, *emptystr, *defaultstr;
    long int val;
    bool done;
    int ret;


    assert(result != NULL);
    assert(min <= max);

    buf        = xmalloc(BUFSIZE * sizeof(wchar_t));
    bufcopy    = xmalloc(BUFSIZE * sizeof(wchar_t));
    allowed    = xmalloc(BUFSIZE * sizeof(wchar_t));
    emptystr   = xmalloc(BUFSIZE * sizeof(wchar_t));
    defaultstr = xmalloc(BUFSIZE * sizeof(wchar_t));

    *buf = L'\0';

    wcscpy(allowed, L"0123456789+-");
    wcsncat(allowed, thousands_sep,     BUFSIZE - wcslen(allowed) - 1);
    wcsncat(allowed, mon_thousands_sep, BUFSIZE - wcslen(allowed) - 1);

    swprintf(emptystr,   BUFSIZE, L"%'1ld", emptyval);
    swprintf(defaultstr, BUFSIZE, L"%'1ld", defaultval);

    done = false;
    while (! done) {
	ret = gettxline(win, buf, BUFSIZE, NULL, false, emptystr, defaultstr,
			allowed, true, y, x, width, attr);

	if (ret == OK) {
	    wchar_t *p;

	    txinput_fixup(bufcopy, buf, false);
	    val = wcstol(bufcopy, &p, 10);

	    if (*p == L'\0' && val >= min && val <= max) {
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

bool answer_yesno (WINDOW *win)
{
    static wchar_t *keycode_yes;
    static wchar_t *keycode_no;

    bool ret;

    chtype oldattr = getattrs(win);
    chtype oldbkgd = getbkgd(win);


    if (keycode_yes == NULL) {
	wchar_t *buf = xmalloc(BUFSIZE * sizeof(wchar_t));

	/* TRANSLATORS: The strings with msgctxt "input|Yes" and
	   "input|No" contain the keycodes used to determine whether a
	   user is answering "Yes" or "No" in response to some question.
	   Both upper and lower-case versions should be present. */
	xmbstowcs(buf, pgettext("input|Yes", "Yy"), BUFSIZE);
	keycode_yes = xwcsdup(buf);
	xmbstowcs(buf, pgettext("input|No",  "Nn"), BUFSIZE);
	keycode_no = xwcsdup(buf);

	free(buf);
    }


    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    curs_set(CURS_ON);

    while (true) {
	wint_t key;
	int r = getwch(win, &key);

	if (r == OK) {
	    if (wcschr(keycode_yes, key) != NULL) {
		ret = true;
		break;
	    } else if (wcschr(keycode_no, key) != NULL) {
		ret = false;
		break;
	    } else {
		beep();
	    }
	} else if (r == KEY_CODE_YES) {

#ifdef HANDLE_RESIZE_EVENTS
	    if (key == KEY_RESIZE) {
		txresize();
	    } else {
		beep();
	    }
#else // ! HANDLE_RESIZE_EVENTS
	    beep();
#endif // ! HANDLE_RESIZE_EVENTS

	} else {
	    beep();
	}
    }

    curs_set(CURS_OFF);
    wattron(win, A_BOLD);

    if (ret) {
	/* TRANSLATORS: The strings "Yes" and "No" are printed as a
	   response to user input in answer to questions like "Are you
	   sure? [Y/N] " */
	waddstr(win, pgettext("answer", "Yes"));
    } else {
	waddstr(win, pgettext("answer", "No"));
    }

    wbkgdset(win, oldbkgd);
    wattrset(win, oldattr);

    wrefresh(win);
    return ret;
}


/***********************************************************************/
// wait_for_key: Print a message and wait for any key

void wait_for_key (WINDOW *win, int y, chtype attr)
{
    wint_t key;
    int r;


    keypad(win, true);
    meta(win, true);
    wtimeout(win, -1);

    center(curwin, y, 0, attr, 0, 0, 1,
	   /* TRANSLATORS: The reason the user is not asked "Press any
	      key to continue" is historical: many, many people used to
	      ask "where is the <ANY> key?" :-) */
	   _("[ Press <SPACE> to continue ] "));
    wrefresh(win);

    while (true) {
	r = getwch(win, &key);
	if (r == OK) {
	    break;
	} else if (r == KEY_CODE_YES) {

#ifdef HANDLE_RESIZE_EVENTS
	    if (key == KEY_RESIZE) {
		txresize();
	    } else {
		break;
	    }
#else // ! HANDLE_RESIZE_EVENTS
	    break;
#endif // ! HANDLE_RESIZE_EVENTS

	} else {
	    beep();
	}
    }
}


/***********************************************************************/
// End of file
