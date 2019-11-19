/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2019, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, game.c, contains the implementation of the starting and
  ending game functions used in Star Traders, as well as the functions
  for displaying the galaxy map and the player's status.


  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see https://www.gnu.org/licenses/.
*/


#include "trader.h"


/************************************************************************
*                  Module-specific function prototypes                  *
************************************************************************/

/*
  Function:   ask_number_players - Ask for the number of players
  Parameters: (none)
  Returns:    int    - Number of players, 0 to load game, ERR to cancel

  This internal function asks the user how many people will play.  It
  returns a number 1 to MAX_PLAYERS as a response, or 0 if a previous
  game is to be loaded, or ERR if the user wishes to abort.

  Please note that the window opened by this function is NOT closed!
*/
static int ask_number_players (void);


/*
  Function:   ask_game_number - Ask for the game number
  Parameters: (none)
  Returns:    int    - Game number (1-9) or ERR to cancel

  This internal function asks the user which game number to load.  It
  returns a number 1 to 9 as a response, or ERR if the user wishes to
  cancel loading a game.

  Please note that the window opened by this function is NOT closed!
*/
static int ask_game_number (void);


/*
  Function:   ask_player_names - Ask for each of the players' names
  Parameters: (none)
  Returns:    (nothing)

  This internal function asks each player to type in their name.  After
  doing so, the players are asked whether they need instructions on
  playing the game.

  On entry, the global variable number_players is used to determine how
  many people are playing.  On exit, each player[].name is set.  The
  windows created by this function ARE closed, but not any other window.
  Note also that txrefresh() is NOT called.
*/
static void ask_player_names (void);


/*
  Function:   cmp_player - Compare two player[] elements for sorting
  Parameters: a, b       - Pointers to elements to compare
  Returns:    int        - Comparison of a and b

  This internal function compares two player[] elements (of type
  player_info_t) on the basis of total value, and returns -1 if a > b, 0
  if a == b and 1 if a < b (note the change from the usual order!).  It
  is used for sorting players into descending total value order.  Note
  that the sort_value field MUST be computed before calling qsort()!
*/
static int cmp_player (const void *a, const void *b);


/************************************************************************
*                       Game function definitions                       *
************************************************************************/

/* These functions are documented either in the file "game.h" or in the
   comments above. */


/***********************************************************************/
// init_game: Initialise a new game or load an old one

void init_game (void)
{
    // Try to load an old game, if possible
    if (game_num != 0) {
	chtype *chbuf = xmalloc(BUFSIZE * sizeof(chtype));
	int width;

	mkchstr(chbuf, BUFSIZE, attr_status_window, 0, 0, 1, WIN_COLS - 7,
		&width, 1, _("Loading game %d... "), game_num);
	newtxwin(5, width + 5, 6, WCENTER, true, attr_status_window);
	centerch(curwin, 2, 0, chbuf, 1, &width);
	wrefresh(curwin);

	game_loaded = load_game(game_num);

	deltxwin();
	txrefresh();
	free(chbuf);
    }

    // Initialise game data, if not already loaded
    if (! game_loaded) {
	number_players = 0;
	while (number_players == 0) {
	    int choice = ask_number_players();

	    if (choice == ERR) {
		abort_game = true;
		return;

	    } else if (choice == 0) {
		choice = ask_game_number();

		if (choice != ERR) {
		    // Try to load the game, if possible

		    chtype *chbuf = xmalloc(BUFSIZE * sizeof(chtype));
		    int width;

		    game_num = choice;

		    mkchstr(chbuf, BUFSIZE, attr_status_window, 0, 0, 1,
			    WIN_COLS - 7, &width, 1,
			    _("Loading game %d... "), game_num);
		    newtxwin(5, width + 5, 9, WCENTER, true, attr_status_window);
		    centerch(curwin, 2, 0, chbuf, 1, &width);
		    wrefresh(curwin);

		    game_loaded = load_game(game_num);

		    deltxwin();
		    txrefresh();
		    free(chbuf);
		}

		deltxwin();		// "Enter game number" window
		deltxwin();		// "Number of players" window
		txrefresh();

	    } else {
		number_players = choice;
	    }
	}

	if (! game_loaded) {
	    wchar_t *buf = xmalloc(BUFSIZE * sizeof(wchar_t));

	    ask_player_names();

	    deltxwin();			// "Number of players" window
	    txrefresh();

	    // Initialise player data (other than names)
	    for (int i = 0; i < number_players; i++) {
		player[i].cash    = INITIAL_CASH;
		player[i].debt    = 0.0;
		player[i].in_game = true;

		for (int j = 0; j < MAX_COMPANIES; j++) {
		    player[i].stock_owned[j] = 0;
		}
	    }

	    // Initialise company data
	    for (int i = 0; i < MAX_COMPANIES; i++) {
		xmbstowcs(buf, gettext(company_name[i]), BUFSIZE);
		company[i].name         = xwcsdup(buf);
		company[i].share_price  = 0.0;
		company[i].share_return = INITIAL_RETURN;
		company[i].stock_issued = 0;
		company[i].max_stock    = 0;
		company[i].on_map       = false;
	    }

	    // Initialise galaxy map
	    for (int x = 0; x < MAX_X; x++) {
		for (int y = 0; y < MAX_Y; y++) {
		    galaxy_map[x][y] = (randf() < STAR_RATIO) ?
			MAP_STAR : MAP_EMPTY;
		}
	    }

	    // Miscellaneous initialisation
	    interest_rate = INITIAL_INTEREST_RATE;
	    max_turn = option_max_turn ? option_max_turn : DEFAULT_MAX_TURN;
	    turn_number = 1;

	    // Select who is to go first
	    if (number_players == 1) {
		first_player   = 0;
		current_player = 0;
	    } else {
		first_player   = randi(number_players);
		current_player = first_player;

		txdlgbox(MAX_DLG_LINES, 50, 8, WCENTER, attr_normal_window,
			 attr_title, attr_normal, attr_highlight, 0,
			 attr_waitforkey, _("  First Player  "),
			 _("The first player to go is ^{%ls^}."),
			 player[first_player].name);
		txrefresh();
	    }

	    free(buf);
	}
    }

    quit_selected = false;
    abort_game = false;
}


/***********************************************************************/
// ask_number_players: Ask for the number of players

static int ask_number_players (void)
{
    wchar_t *keycode_contgame = xmalloc(BUFSIZE * sizeof(wchar_t));
    chtype *chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    int lines, maxwidth;
    int widthbuf[2];
    int ret;
    bool done;


    lines = mkchstr(chbuf, BUFSIZE, attr_normal, attr_keycode, 0, 2,
		    WIN_COLS - 7, widthbuf, 2,
		    /* TRANSLATORS: The keycode <C> should be modified to
		       match that (or those) specified with msgctxt
		       "input|ContinueGame". */
		    _("Enter number of players [^{1^}-^{%d^}] "
		      "or ^{<C>^} to continue a game: "), MAX_PLAYERS);
    assert(lines == 1 || lines == 2);
    maxwidth = (lines == 1 ? widthbuf[0] : MAX(widthbuf[0], widthbuf[1])) + 5;

    newtxwin(lines + 4, maxwidth, 3, WCENTER, true, attr_normal_window);
    leftch(curwin, 2, 2, chbuf, lines, widthbuf);
    free(chbuf);

    curs_set(CURS_ON);
    wrefresh(curwin);

    /* TRANSLATORS: This string specifies the keycodes used to continue a
       game; these must NOT contain any numeric digit from 1 to 9.  The
       first character (keyboard input code) is used to print the user's
       response if one of those keys is pressed.  Both upper and
       lower-case versions should be present. */
    xmbstowcs(keycode_contgame, pgettext("input|ContinueGame", "Cc"), BUFSIZE);

    done = false;
    while (! done) {
	wint_t key;

	if (gettxchar(curwin, &key) == OK) {
	    // Ordinary wide character
	    if (key >= L'1' && key <= MAX_PLAYERS + L'0') {
		left(curwin, getcury(curwin), getcurx(curwin), A_BOLD,
		     0, 0, 1, "%lc", key);
		wrefresh(curwin);

		ret = key - L'0';
		done = true;
	    } else if (wcschr(keycode_contgame, key) != NULL) {
		left(curwin, getcury(curwin), getcurx(curwin), A_BOLD,
		     0, 0, 1, "%lc", (wint_t) *keycode_contgame);
		wrefresh(curwin);

		ret = 0;
		done = true;
	    } else {
		beep();
	    }
	} else {
	    // Function or control key
	    switch (key) {
	    case KEY_ESC:
	    case KEY_CANCEL:
	    case KEY_EXIT:
	    case KEY_CTRL('C'):
	    case KEY_CTRL('G'):
	    case KEY_CTRL('\\'):
		ret = ERR;
		done = true;
		break;

	    default:
		beep();
	    }
	}
    }

    curs_set(CURS_OFF);
    free(keycode_contgame);
    return ret;
}


/***********************************************************************/
// ask_game_number: Ask for the game number

int ask_game_number (void)
{
    chtype *chbuf;
    int lines, maxwidth;
    int widthbuf[2];
    int ret;
    bool done;


    chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    lines = mkchstr(chbuf, BUFSIZE, attr_normal, attr_keycode, 0, 2,
		    WIN_COLS - 7, widthbuf, 2,
		    _("Enter game number [^{1^}-^{9^}] "
		      "or ^{<CTRL><C>^} to cancel: "));
    assert(lines == 1 || lines == 2);
    maxwidth = (lines == 1 ? widthbuf[0] : MAX(widthbuf[0], widthbuf[1])) + 5;

    newtxwin(lines + 4, maxwidth, 6, WCENTER, true, attr_normal_window);
    leftch(curwin, 2, 2, chbuf, lines, widthbuf);
    free(chbuf);

    curs_set(CURS_ON);
    wrefresh(curwin);

    done = false;
    while (! done) {
	wint_t key;

	if (gettxchar(curwin, &key) == OK) {
	    // Ordinary wide character
	    if (key >= L'1' && key <= L'9') {
		left(curwin, getcury(curwin), getcurx(curwin), A_BOLD,
		     0, 0, 1, "%lc", key);
		wrefresh(curwin);

		ret = key - L'0';
		done = true;
	    } else {
		beep();
	    }
	} else {
	    // Function or control key
	    switch (key) {
	    case KEY_ESC:
	    case KEY_CANCEL:
	    case KEY_EXIT:
	    case KEY_CTRL('C'):
	    case KEY_CTRL('G'):
	    case KEY_CTRL('\\'):
		ret = ERR;
		done = true;
		break;

	    default:
		beep();
	    }
	}
    }

    curs_set(CURS_OFF);
    return ret;
}


/***********************************************************************/
// ask_player_names: Ask for each of the players' names

void ask_player_names (void)
{
    chtype *chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    int width;


    if (number_players == 1) {
	// Ask for the player's name

	newtxwin(5, WIN_COLS - 4, 9, WCENTER, true, attr_normal_window);
	left(curwin, 2, 2, attr_normal, 0, 0, 1, _("Please enter your name: "));

	int x = getcurx(curwin);
	int w = getmaxx(curwin) - x - 2;

	player[0].name = NULL;
	player[0].name_utf8 = NULL;
	while (true) {
	    int ret = gettxstr(curwin, &player[0].name, NULL, false,
			       2, x, w, attr_input_field);
	    if (ret == OK && wcslen(player[0].name) != 0) {
		break;
	    } else {
		beep();
	    }
	}

	mkchstr(chbuf, BUFSIZE, attr_normal, attr_keycode, 0, 1,
		WIN_COLS - YESNO_COLS - 6, &width, 1,
		/* TRANSLATORS: Note that you should replace "Y" and "N"
		   with the localised upper-case characters used in the
		   "input|Yes" and "input|No" strings.  There are other
		   strings in this game, each with "[^{Y^}/^{N^}]", that
		   need the same replacement. */
		_("Do you need any instructions? [^{Y^}/^{N^}] "));
	newtxwin(5, width + YESNO_COLS + 4, 6, WCENTER, true,
		 attr_normal_window);
	leftch(curwin, 2, 2, chbuf, 1, &width);

	if (answer_yesno(curwin)) {
	    show_help();
	}

    } else {
	// Ask for all of the player names

	bool entered[MAX_PLAYERS];
	bool done, modified;
	int cur, len, i;

	newtxwin(number_players + 5, WIN_COLS - 4, 9, WCENTER,
		 true, attr_normal_window);
	center(curwin, 1, 0, attr_title, 0, 0, 1, _("  Enter Player Names  "));

	for (i = 0; i < number_players; i++) {
	    player[i].name = NULL;
	    player[i].name_utf8 = NULL;
	    entered[i] = false;
	    left(curwin, i + 3, 2, attr_normal, 0, 0, 1,
		 /* xgettext:c-format, range: 1..8 */
		 _("Player %d: "), i + 1);
	}

	int x = getcurx(curwin);
	int w = getmaxx(curwin) - x - 2;

	cur = 0;
	done = false;
	while (! done) {
	    int ret = gettxstr(curwin, &player[cur].name, &modified, true,
			       3 + cur, x, w, attr_input_field);

	    switch (ret) {
	    case OK:
		// Make sure name is not an empty string
		len = wcslen(player[cur].name);
		entered[cur] = (len != 0);
		if (len == 0) {
		    beep();
		}

		// Make sure name has not been entered already
		for (i = 0; i < number_players; i++) {
		    if (i != cur && player[i].name != NULL
			&& wcscmp(player[i].name, player[cur].name) == 0) {
			entered[cur] = false;
			beep();
			break;
		    }
		}

		// Move to first name for which ENTER has not been pressed
		done = true;
		for (cur = 0; cur < number_players; cur++) {
		    if (! entered[cur]) {
			done = false;
			break;
		    }
		}
		break;

	    case ERR:
		beep();
		break;

	    case KEY_UP:
		// Scroll up to previous name (with wrap-around)
		if (modified) {
		    entered[cur] = false;
		}

		if (cur == 0) {
		    cur = number_players - 1;
		} else {
		    cur--;
		}
		break;

	    case KEY_DOWN:
		// Scroll down to next name (with wrap-around)
		if (modified) {
		    entered[cur] = false;
		}

		if (cur == number_players - 1) {
		    cur = 0;
		} else {
		    cur++;
		}
		break;

	    default:
		beep();
	    }
	}

	mkchstr(chbuf, BUFSIZE, attr_normal, attr_keycode, 0, 1,
		WIN_COLS - YESNO_COLS - 6, &width, 1,
		_("Does any player need instructions? [^{Y^}/^{N^}] "));
	newtxwin(5, width + YESNO_COLS + 4, 6, WCENTER, true,
		 attr_normal_window);
	leftch(curwin, 2, 2, chbuf, 1, &width);

	if (answer_yesno(curwin)) {
	    show_help();
	}
    }

    deltxwin();				// "Need instructions?" window
    deltxwin();				// "Enter player names" window
    free(chbuf);
}


/***********************************************************************/
// end_game: Finish playing the current game

void end_game (void)
{
    chtype *chbuf;
    int lines, widthbuf[5];


    if (abort_game) {
	// init_game() was cancelled by user
	return;
    }

    chbuf = xmalloc(BUFSIZE * sizeof(chtype));

    txdlgbox(MAX_DLG_LINES, 50, 9, WCENTER, attr_error_window,
	     attr_error_title, attr_error_highlight, 0, 0,
	     attr_error_waitforkey, _("  Game Over  "),
	     ngettext("The game is over after one turn.",
		      "The game is over after %d turns.",
		      turn_number - 1), turn_number - 1);

    for (int i = 0; i < number_players; i++) {
	show_status(i);
    }

    if (number_players == 1) {
	txdlgbox(MAX_DLG_LINES, 60, 8, WCENTER, attr_normal_window,
		 attr_title, attr_normal, attr_highlight, 0, attr_waitforkey,
		 _("  Total Value  "),
		 /* xgettext:c-format */
		 _("Your total value was ^{%N^}."), total_value(0));
    } else {
	// Sort players on the basis of total value
	for (int i = 0; i < number_players; i++) {
	    player[i].sort_value = total_value(i);
	}
	qsort(player, number_players, sizeof(player_info_t), cmp_player);

	lines = mkchstr(chbuf, BUFSIZE, attr_normal, attr_highlight,
			attr_blink, 5, WIN_COLS - 8, widthbuf, 5,
			(player[0].sort_value == 0) ?
			_("The winner is ^{%ls^}\n"
			  "who is ^[*** BANKRUPT ***^]") :
			/* xgettext:c-format */
			_("The winner is ^{%ls^}\n"
			  "with a value of ^{%N^}."),
			player[0].name, player[0].sort_value);

	newtxwin(number_players + lines + 8, WIN_COLS - 4, 3, WCENTER,
		 true, attr_normal_window);
	center(curwin, 1, 0, attr_title, 0, 0, 1, _("  Game Winner  "));
	centerch(curwin, 3, 0, chbuf, lines, widthbuf);

	int w = getmaxx(curwin);

	mvwhline(curwin, lines + 4, 2, ' ' | attr_subtitle, w - 4);
	left(curwin, lines + 4, ORDINAL_COLS + 4, attr_subtitle, 0, 0, 1,
	     /* TRANSLATORS: "Player" is used as a column title in a
		table containing all player names. */
	     pgettext("subtitle", "Player"));
	right(curwin, lines + 4, w - 4, attr_subtitle, 0, 0, 1,
	      /* TRANSLATORS: "Total Value" refers to the total worth
		 (shares, cash and debt) of any given player.  %ls is the
		 currency symbol of the current locale. */
	      pgettext("subtitle", "Total Value (%ls)"), currency_symbol);

	for (int i = 0; i < number_players; i++) {
	    right(curwin, i + lines + 5, ORDINAL_COLS + 2, attr_normal, 0, 0,
		  1, gettext(ordinal[i + 1]));
	    left(curwin, i + lines + 5, ORDINAL_COLS + 4, attr_normal, 0, 0,
		 1, "%ls", player[i].name);
	    right(curwin, i + lines + 5, w - 2, attr_normal, 0, 0,
		  1, "  %!N  ", player[i].sort_value);
	}

	wait_for_key(curwin, getmaxy(curwin) - 2, attr_waitforkey);
	deltxwin();
    }

    free(chbuf);
}


/***********************************************************************/
// show_map: Display the galaxy map on the screen

void show_map (bool closewin)
{
    newtxwin(MAX_Y + 4, WIN_COLS, 1, WCENTER, true, attr_map_window);

    // Draw various borders and highlights
    mvwaddch(curwin, 2, 0, ACS_LTEE);
    whline(curwin, ACS_HLINE, getmaxx(curwin) - 2);
    mvwaddch(curwin, 2, getmaxx(curwin) - 1, ACS_RTEE);
    mvwhline(curwin, 1, 2, ' ' | attr_mapwin_title, getmaxx(curwin) - 4);

    // Display current player and turn number
    left(curwin, 1, 4, attr_mapwin_title, attr_mapwin_highlight, 0, 1,
	 _("Player: ^{%ls^}"), player[current_player].name);
    right(curwin, 1, getmaxx(curwin) - 2, attr_mapwin_title,
	  attr_mapwin_highlight, attr_mapwin_blink, 1,
	  (turn_number != max_turn) ? _("  Turn: ^{%d^}  ") :
	  _("  ^[*** Last Turn ***^]  "), turn_number);

    // Display the actual map
    for (int y = 0; y < MAX_Y; y++) {
	wmove(curwin, y + 3, 2);
	for (int x = 0; x < MAX_X; x++) {
	    chtype *mapstr = CHTYPE_MAP_VAL(galaxy_map[x][y]);

	    while (*mapstr != 0) {
		waddch(curwin, *mapstr++);
	    }
	}
    }

    if (closewin) {
	// Wait for the user to press any key

	wrefresh(curwin);

	newtxwin(WIN_LINES - MAX_Y - 5, WIN_COLS, MAX_Y + 5, WCENTER,
		 true, attr_normal_window);

	wait_for_key(curwin, 2, attr_waitforkey);

	deltxwin();			// Wait for key window
	deltxwin();			// Galaxy map window
	txrefresh();
    }
}


/***********************************************************************/
// show_status: Display the player's status

void show_status (int num)
{
    double val;
    int w, i, line;


    assert(num >= 0 && num < number_players);

    newtxwin(MAX_COMPANIES + 15, WIN_COLS, 1, WCENTER, true,
	     attr_normal_window);
    center(curwin, 1, 0, attr_title, 0, 0, 1, _("  Stock Portfolio  "));
    center(curwin, 2, 0, attr_normal, attr_highlight, 0, 1,
	   _("Player: ^{%ls^}"), player[num].name);

    val = total_value(num);
    if (val == 0.0) {
	center(curwin, 11, 0, attr_normal, attr_highlight, attr_blink, 1,
	       /* TRANSLATORS: The current player is bankrupt (has no
		  shares or cash, ie, whose total value is zero) */
	       _("^[* * *   B A N K R U P T   * * *^]"));
    } else {
	w = getmaxx(curwin);

	// Check to see if any companies are on the map
	bool none = true;
	for (i = 0; i < MAX_COMPANIES; i++) {
	    if (company[i].on_map) {
		none = false;
		break;
	    }
	}

	if (none) {
	    center(curwin, 8, 0, attr_normal, attr_highlight, 0, 1,
		   _("No companies on the map"));
	} else {
	    mvwhline(curwin, 4, 2, ' ' | attr_subtitle, w - 4);
	    mvwhline(curwin, 5, 2, ' ' | attr_subtitle, w - 4);

	    left(curwin, 4, 4, attr_subtitle, 0, 0, 2,
		 /* TRANSLATORS: "Company" is a two-line column label in
		    a table containing a list of companies. */
		 pgettext("subtitle", " \nCompany"));
	    right(curwin, 4, w - 4, attr_subtitle, 0, 0, 2,
		  /* TRANSLATORS: "Ownership" is a two-line column label
		     in a table containing the current player's
		     percentage ownership in any given company.  The
		     maximum column width is 10 characters (see
		     OWNERSHIP_COLS in src/intf.h). */
		  pgettext("subtitle", "Ownership\n(%%)"));
	    right(curwin, 4, w - 6 - OWNERSHIP_COLS, attr_subtitle, 0, 0, 2,
		  /* TRANSLATORS: "Holdings" is a two-line column label
		     in a table containing the number of shares the
		     current player owns in any given company.  The
		     maximum column width is 10 characters (see
		     STOCK_OWNED_COLS in src/intf.h). */
		  pgettext("subtitle", "Holdings\n(shares)"));
	    right(curwin, 4, w - 8 - OWNERSHIP_COLS - STOCK_OWNED_COLS,
		  attr_subtitle, 0, 0, 2,
		  /* TRANSLATORS: "Return" is a two-line column label in
		     a table containing the share return as a percentage
		     in any given company.  The maximum column width is
		     10 characters (see SHARE_RETURN_COLS in src/intf.h). */
		  pgettext("subtitle", "Return\n(%%)"));
	    right(curwin, 4, w - 10 - OWNERSHIP_COLS - STOCK_OWNED_COLS
		  - SHARE_RETURN_COLS, attr_subtitle, 0, 0, 2,
		  /* TRANSLATORS: "Price per share" is a two-line column
		     label in a table containing the price per share in
		     any given company.  %ls is the currency symbol in
		     the current locale.  The maximum column width is 12
		     characters INCLUDING the currency symbol (see
		     SHARE_PRICE_COLS in src/intf.h). */
		  pgettext("subtitle", "Price per\nshare (%ls)"),
		  currency_symbol);

	    for (line = 6, i = 0; i < MAX_COMPANIES; i++) {
		if (company[i].on_map) {
		    left(curwin, line, 4, attr_normal, 0, 0, 1, "%ls",
			 company[i].name);

		    right(curwin, line, w - 2, attr_normal, 0, 0, 1, "%.2f  ",
			  (company[i].stock_issued == 0) ? 0.0 :
			  ((double) player[num].stock_owned[i] * 100.0)
			  / company[i].stock_issued);
		    right(curwin, line, w - 4 - OWNERSHIP_COLS, attr_normal,
			  0, 0, 1, "%'ld  ", player[num].stock_owned[i]);
		    right(curwin, line, w - 6 - OWNERSHIP_COLS
			  - STOCK_OWNED_COLS, attr_normal, 0, 0, 1, "%.2f  ",
			  company[i].share_return * 100.0);
		    right(curwin, line, w - 8 - OWNERSHIP_COLS
			  - STOCK_OWNED_COLS - SHARE_RETURN_COLS, attr_normal,
			  0, 0, 1, "  %!N  ", company[i].share_price);

		    line++;
		}
	    }
	}

	line = MAX_COMPANIES + 7;

	chtype *chbuf = xmalloc(BUFSIZE * sizeof(chtype));
	int width, x;

	mkchstr(chbuf, BUFSIZE, attr_highlight, 0, 0, 1, w / 2, &width, 1,
		/* TRANSLATORS: The "Total value", "Current cash",
		   "Current debt" and "Interest rate" labels MUST all be
		   the same length (ie, right-padded with spaces as
		   needed) and must have at least one trailing space so
		   that the display routines work correctly.  The maximum
		   length of each label is 36 characters.

		   Note that some of these labels are used for both the
		   Player Status window and the Trading Bank window. */
		pgettext("label", "Total value:   "));
	x = (w + width - (TOTAL_VALUE_COLS + 2)) / 2;

	right(curwin, line, x, attr_normal, attr_highlight, 0, 1,
	      pgettext("label", "Current cash:  "));
	right(curwin, line, x + TOTAL_VALUE_COLS + 2, attr_normal,
	      attr_highlight, 0, 1, " ^{%N^} ", player[num].cash);
	line++;

	if (player[num].debt != 0.0) {
	    right(curwin, line, x, attr_normal, attr_highlight, 0, 1,
		  pgettext("label", "Current debt:  "));
	    right(curwin, line, x + TOTAL_VALUE_COLS + 2, attr_normal,
		  attr_highlight, 0, 1, " ^{%N^} ", player[num].debt);
	    line++;

	    right(curwin, line, x, attr_normal, attr_highlight, 0, 1,
		  pgettext("label", "Interest rate: "));
	    right(curwin, line, x + TOTAL_VALUE_COLS + 2, attr_normal,
		  attr_highlight, 0, 1, " ^{%.2f%%^} ", interest_rate * 100.0);
	    line++;
	}

	rightch(curwin, line + 1, x, chbuf, 1, &width);
	whline(curwin, ' ' | attr_title, TOTAL_VALUE_COLS + 2);
	right(curwin, line + 1, x + TOTAL_VALUE_COLS + 2, attr_title, 0, 0, 1,
	      " %N ", val);

	free(chbuf);
    }

    wait_for_key(curwin, getmaxy(curwin) - 2, attr_waitforkey);
    deltxwin();
    txrefresh();
}


/***********************************************************************/
// total_value: Calculate a player's total financial worth

double total_value (int num)
{
    double val;


    assert(num >= 0 && num < number_players);

    val = player[num].cash - player[num].debt;
    for (int i = 0; i < MAX_COMPANIES; i++) {
	if (company[i].on_map) {
	    val += player[num].stock_owned[i] * company[i].share_price;
	}
    }

    return val;
}


/***********************************************************************/
// cmp_player: Compare two player[] elements for sorting

int cmp_player (const void *a, const void *b)
{
    /* This implementation assumes that each player[] element has already
       had its sort_value set to the result of total_value().  Note also
       that the function result is reversed from the normal order, so
       that players are sorted into descending order  */

    const player_info_t *aa = (const player_info_t *) a;
    const player_info_t *bb = (const player_info_t *) b;


    if (aa->sort_value > bb->sort_value) {
	return -1;
    } else if (aa->sort_value < bb->sort_value) {
	return 1;
    } else {
	return 0;
    }
}


/***********************************************************************/
// End of file
