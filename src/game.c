/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
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
  along with this program.  If not, see http://www.gnu.org/licenses/.
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
	newtxwin(5, 30, 6, WCENTER(30), true, ATTR_STATUS_WINDOW);
	center(curwin, 2, ATTR_STATUS_WINDOW, "Loading game %d... ", game_num);
	wrefresh(curwin);

	game_loaded = load_game(game_num);

	deltxwin();
	txrefresh();
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
		    game_num = choice;

		    // Try to load the game, if possible
		    newtxwin(5, 30, 9, WCENTER(30), true, ATTR_STATUS_WINDOW);
		    center(curwin, 2, ATTR_STATUS_WINDOW,
			   "Loading game %d... ", game_num);
		    wrefresh(curwin);

		    game_loaded = load_game(game_num);

		    deltxwin();
		    txrefresh();
		}

		deltxwin();		// "Enter game number" window
		deltxwin();		// "Number of players" window
		txrefresh();

	    } else {
		number_players = choice;
	    }
	}

	if (! game_loaded) {
	    int i, j, x, y;

	    ask_player_names();

	    deltxwin();			// "Number of players" window
	    txrefresh();

	    // Initialise player data (other than names)
	    for (i = 0; i < number_players; i++) {
		player[i].cash    = INITIAL_CASH;
		player[i].debt    = 0.0;
		player[i].in_game = true;

		for (j = 0; j < MAX_COMPANIES; j++) {
		    player[i].stock_owned[j] = 0;
		}
	    }

	    // Initialise company data
	    for (i = 0; i < MAX_COMPANIES; i++) {
		company[i].name         = company_name[i];
		company[i].share_price  = 0.0;
		company[i].share_return = INITIAL_RETURN;
		company[i].stock_issued = 0;
		company[i].max_stock    = 0;
		company[i].on_map       = false;
	    }

	    // Initialise galaxy map
	    for (x = 0; x < MAX_X; x++) {
		for (y = 0; y < MAX_Y; y++) {
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

		newtxwin(7, 50, 8, WCENTER(50), true, ATTR_NORMAL_WINDOW);

		center(curwin, 2, ATTR_NORMAL, "The first player to go is");
		center(curwin, 3, ATTR_HIGHLIGHT, "%s",
		       player[first_player].name);

		wait_for_key(curwin, 5, ATTR_WAITFORKEY);
		deltxwin();
		txrefresh();
	    }
	}
    }

    quit_selected = false;
    abort_game = false;
}


/***********************************************************************/
// ask_number_players: Ask for the number of players

static int ask_number_players (void)
{
    int key, ret;
    bool done;


    // Ask for the number of players
    newtxwin(5, 62, 3, WCENTER(62), true, ATTR_NORMAL_WINDOW);

    mvwaddstr(curwin, 2, 2, "Enter number of players ");
    waddstr(curwin, "[");
    attrpr(curwin, ATTR_KEYCODE, "1");
    waddstr(curwin, "-");
    attrpr(curwin, ATTR_KEYCODE, "%d", MAX_PLAYERS);
    waddstr(curwin, "]");
    waddstr(curwin, " or ");
    attrpr(curwin, ATTR_KEYCODE, "<C>");
    waddstr(curwin, " to continue a game: ");

    curs_set(CURS_ON);
    wrefresh(curwin);

    done = false;
    while (! done) {
	key = toupper(gettxchar(curwin));

	if (key >= '1' && key <= MAX_PLAYERS + '0') {
	    wechochar(curwin, key | A_BOLD);
	    ret = key - '0';
	    done = true;
	} else {
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

	    case 'C':
		wechochar(curwin, key | A_BOLD);
		ret = 0;
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
// ask_game_number: Ask for the game number

int ask_game_number (void)
{
    int key, ret;
    bool done;


    // Ask which game to load
    newtxwin(5, 54, 6, WCENTER(54), true, ATTR_NORMAL_WINDOW);

    mvwaddstr(curwin, 2, 2, "Enter game number ");
    waddstr(curwin, "[");
    attrpr(curwin, ATTR_KEYCODE, "1");
    waddstr(curwin, "-");
    attrpr(curwin, ATTR_KEYCODE, "9");
    waddstr(curwin, "]");
    waddstr(curwin, " or ");
    attrpr(curwin, ATTR_KEYCODE, "<CTRL><C>");
    waddstr(curwin, " to cancel: ");

    curs_set(CURS_ON);
    wrefresh(curwin);

    done = false;
    while (! done) {
	key = gettxchar(curwin);

	if (key >= '1' && key <= '9') {
	    wechochar(curwin, key | A_BOLD);
	    ret = key - '0';
	    done = true;
	} else {
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
    if (number_players == 1) {
	// Ask for the player's name

	newtxwin(5, WIN_COLS - 4, 9, WCENTER(WIN_COLS - 4), true,
		 ATTR_NORMAL_WINDOW);

	mvwaddstr(curwin, 2, 2, "Please enter your name: ");

	int x = getcurx(curwin);
	int w = getmaxx(curwin) - x - 2;

	player[0].name = NULL;
	while (true) {
	    int ret = gettxstr(curwin, &player[0].name, NULL, false,
			       2, x, w, ATTR_INPUT_FIELD);
	    if (ret == OK && strlen(player[0].name) != 0) {
		break;
	    } else {
		beep();
	    }
	}

	newtxwin(5, 44, 6, WCENTER(44), true, ATTR_NORMAL_WINDOW);
	mvwaddstr(curwin, 2, 2, "Do you need any instructions?");
	if (answer_yesno(curwin, ATTR_KEYCODE)) {
	    show_help();
	}

    } else {
	// Ask for all of the player names

	bool entered[MAX_PLAYERS];
	bool done, modified;
	int cur, len, i;

	newtxwin(number_players + 5, WIN_COLS - 4, 9, WCENTER(WIN_COLS - 4),
		 true, ATTR_NORMAL_WINDOW);

	center(curwin, 1, ATTR_TITLE, "  Enter Player Names  ");

	for (i = 0; i < number_players; i++) {
	    player[i].name = NULL;
	    entered[i] = false;
	    mvwprintw(curwin, i + 3, 2, "Player %d:", i + 1);
	}

	int x = getcurx(curwin) + 1;
	int w = getmaxx(curwin) - x - 2;

	cur = 0;
	done = false;
	while (! done) {
	    int ret = gettxstr(curwin, &player[cur].name, &modified, true,
			       3 + cur, x, w, ATTR_INPUT_FIELD);

	    switch (ret) {
	    case OK:
		// Make sure name is not an empty string
		len = strlen(player[cur].name);
		entered[cur] = (len != 0);
		if (len == 0) {
		    beep();
		}

		// Make sure name has not been entered already
		for (i = 0; i < number_players; i++) {
		    if (i != cur && player[i].name != NULL
			&& strcmp(player[i].name, player[cur].name) == 0) {
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

	newtxwin(5, 50, 6, WCENTER(50), true, ATTR_NORMAL_WINDOW);
	mvwaddstr(curwin, 2, 2, "Does any player need instructions?");
	if (answer_yesno(curwin, ATTR_KEYCODE)) {
	    show_help();
	}
    }

    deltxwin();				// "Need instructions?" window
    deltxwin();				// "Enter player names" window
}


/***********************************************************************/
// end_game: Finish playing the current game

void end_game (void)
{
    int i;
    char *buf;


    if (abort_game) {
	// init_game() was cancelled by user
	return;
    }

    buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit_nomem();
    }

    newtxwin(7, 40, 9, WCENTER(40), true, ATTR_ERROR_WINDOW);

    center(curwin, 1, ATTR_ERROR_TITLE, "  Game Over  ");
    center(curwin, 3, ATTR_ERROR_HIGHLIGHT, "The game is over after %d turns",
	   turn_number - 1);

    wait_for_key(curwin, 5, ATTR_ERROR_WAITFORKEY);
    deltxwin();

    for (i = 0; i < number_players; i++) {
	show_status(i);
    }

    if (number_players == 1) {
	l_strfmon(buf, BUFSIZE, "%1n", total_value(0));

	newtxwin(9, 60, 8, WCENTER(60), true, ATTR_NORMAL_WINDOW);

	center(curwin, 1, ATTR_TITLE, "  Total Value  ");
	center2(curwin, 4, ATTR_NORMAL, ATTR_HIGHLIGHT,
		"Your total value was ", "%s", buf);

	wait_for_key(curwin, 7, ATTR_WAITFORKEY);
	deltxwin();
    } else {
	// Sort players on the basis of total value
	for (i = 0; i < number_players; i++) {
	    player[i].sort_value = total_value(i);
	}
	qsort(player, number_players, sizeof(player_info_t), cmp_player);

	newtxwin(number_players + 10, WIN_COLS - 4, 3, WCENTER(WIN_COLS - 4),
		 true, ATTR_NORMAL_WINDOW);

	center(curwin, 1, ATTR_TITLE, "  Game Winner  ");
	center2(curwin, 3, ATTR_NORMAL, ATTR_HIGHLIGHT, "The winner is ",
		"%s", player[0].name);
	if (player[0].sort_value == 0.0) {
	    center2(curwin, 4, ATTR_NORMAL, ATTR_BLINK, "who is ",
		    "*** BANKRUPT ***");
	} else {
	    l_strfmon(buf, BUFSIZE, "%1n", player[0].sort_value);
	    center2(curwin, 4, ATTR_NORMAL, ATTR_HIGHLIGHT,
		    "with a value of ", "%s", buf);
	}

	int w = getmaxx(curwin) - 33;
	wattrset(curwin, ATTR_SUBTITLE);
	snprintf(buf, BUFSIZE, "Total Value (%s)", lconvinfo.currency_symbol);
	mvwprintw(curwin, 6, 2, "%5s  %-*.*s  %18s  ", "", w, w, "Player", buf);
	wattrset(curwin, ATTR_NORMAL);

	for (i = 0; i < number_players; i++) {
	    l_strfmon(buf, BUFSIZE, "%!18n", player[i].sort_value);
	    mvwprintw(curwin, i + 7, 2, "%5s  %-*.*s  %18s  ",
		      ordinal[i + 1], w, w, player[i].name, buf);
	}

	wait_for_key(curwin, getmaxy(curwin) - 2, ATTR_WAITFORKEY);
	deltxwin();
    }

    free(buf);
}


/***********************************************************************/
// show_map: Display the galaxy map on the screen

void show_map (bool closewin)
{
    int n, x, y;


    newtxwin(MAX_Y + 4, WIN_COLS, 1, WCENTER(WIN_COLS), true, ATTR_MAP_WINDOW);

    // Draw various borders
    mvwaddch(curwin, 2, 0, ACS_LTEE);
    whline(curwin, ACS_HLINE, getmaxx(curwin) - 2);
    mvwaddch(curwin, 2, getmaxx(curwin) - 1, ACS_RTEE);

    // Display current player and turn number
    wattrset(curwin, ATTR_MAPWIN_TITLE);
    mvwaddstr(curwin, 1, 2, "  ");
    waddstr(curwin, "Player: ");
    n = getmaxx(curwin) - getcurx(curwin) - 4;
    wattrset(curwin, ATTR_MAPWIN_HIGHLIGHT);
    wprintw(curwin, "%-*.*s", n, n, player[current_player].name);
    wattrset(curwin, ATTR_MAPWIN_TITLE);
    waddstr(curwin, "  ");

    if (turn_number != max_turn) {
	const char *initial = "Turn: ";

	char *buf = malloc(BUFSIZE);
	if (buf == NULL) {
	    err_exit_nomem();
	}

	int len1 = strlen(initial);
	int len2 = snprintf(buf, BUFSIZE, "%d", turn_number);

	mvwaddstr(curwin, 1, getmaxx(curwin) - (len1 + len2) - 6, "  ");
	waddstr(curwin, initial);
	attrpr(curwin, ATTR_MAPWIN_HIGHLIGHT, "%s", buf);
	waddstr(curwin, "  ");

	free(buf);

    } else {
	const char *buf = "*** Last Turn ***";
	int len = strlen(buf);

	mvwaddstr(curwin, 1, getmaxx(curwin) - len - 6, "  ");
	attrpr(curwin, ATTR_MAPWIN_BLINK, "%s", buf);
	waddstr(curwin, "  ");
    }

    wattrset(curwin, ATTR_MAP_WINDOW);

    // Display the actual map
    for (y = 0; y < MAX_Y; y++) {
	wmove(curwin, y + 3, 2);
	for (x = 0; x < MAX_X; x++) {
	    map_val_t m = galaxy_map[x][y];

	    switch (m) {
	    case MAP_EMPTY:
		waddch(curwin, PRINTABLE_MAP_VAL(m) | ATTR_MAP_EMPTY);
		break;

	    case MAP_OUTPOST:
		waddch(curwin, PRINTABLE_MAP_VAL(m) | ATTR_MAP_OUTPOST);
		break;

	    case MAP_STAR:
		waddch(curwin, PRINTABLE_MAP_VAL(m) | ATTR_MAP_STAR);
		break;

	    default:
		waddch(curwin, PRINTABLE_MAP_VAL(m) | ATTR_MAP_COMPANY);
		break;
	    }
	    waddch(curwin, ' ' | ATTR_MAP_EMPTY);
	}
    }

    if (closewin) {
	// Wait for the user to press any key

	wrefresh(curwin);

	newtxwin(WIN_LINES - MAX_Y - 5, WIN_COLS, MAX_Y + 5, WCENTER(WIN_COLS),
		 true, ATTR_NORMAL_WINDOW);

	wait_for_key(curwin, 2, ATTR_WAITFORKEY);

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
    int i, line;


    assert(num >= 0 && num < number_players);

    newtxwin(MAX_COMPANIES + 15, WIN_COLS, 1, WCENTER(WIN_COLS), true,
	     ATTR_NORMAL_WINDOW);

    center(curwin, 1, ATTR_TITLE, "  Stock Portfolio  ");
    center2(curwin, 2, ATTR_NORMAL, ATTR_HIGHLIGHT, "Player: ", "%s",
	    player[num].name);

    val = total_value(num);
    if (val == 0.0) {
	center(curwin, 11, ATTR_BLINK, "* * *   B A N K R U P T   * * *");

    } else {
	char *buf = malloc(BUFSIZE);
	if (buf == NULL) {
	    err_exit_nomem();
	}

	// Check to see if any companies are on the map
	bool none = true;
	for (i = 0; i < MAX_COMPANIES; i++) {
	    if (company[i].on_map) {
		none = false;
		break;
	    }
	}

	if (none) {
	    center(curwin, 8, ATTR_NORMAL, "No companies on the map");
	} else {
	    // Handle the locale's currency symbol
	    snprintf(buf, BUFSIZE, "share (%s)", lconvinfo.currency_symbol);

	    wattrset(curwin, ATTR_SUBTITLE);
	    mvwprintw(curwin, 4, 2, "  %-22s  %12s  %10s  %10s  %10s  ",
		      "", "Price per", "", "Holdings", "Company");
	    mvwprintw(curwin, 5, 2, "  %-22s  %12s  %10s  %10s  %10s  ",
		      "Company", buf, "Return (%)", "(shares)", "owner (%)");
	    wattrset(curwin, ATTR_NORMAL);

	    for (line = 6, i = 0; i < MAX_COMPANIES; i++) {
		if (company[i].on_map) {
		    l_strfmon(buf, BUFSIZE, "%!12n", company[i].share_price);
		    mvwprintw(curwin, line, 2,
			      "  %-22s  %10s  %10.2f  %'10ld  %10.2f  ",
			      company[i].name, buf,
			      company[i].share_return * 100.0,
			      player[num].stock_owned[i],
			      (company[i].stock_issued == 0) ? 0.0 :
			      ((double) player[num].stock_owned[i] * 100.0)
			      / company[i].stock_issued);
		    line++;
		}
	    }
	}

	line = 15;
	l_strfmon(buf, BUFSIZE, "%18n", player[num].cash);
	center2(curwin, line++, ATTR_NORMAL, ATTR_HIGHLIGHT, "Current cash:  ",
		" %s ", buf);
	if (player[num].debt != 0.0) {
	    l_strfmon(buf, BUFSIZE, "%18n", player[num].debt);
	    center2(curwin, line++, ATTR_NORMAL, ATTR_HIGHLIGHT,
		    "Current debt:  ", " %s ", buf);
	    center2(curwin, line++, ATTR_NORMAL, ATTR_HIGHLIGHT,
		    "Interest rate: ", " %17.2f%% ", interest_rate * 100.0);
	}

	l_strfmon(buf, BUFSIZE, "%18n", val);
	center2(curwin, line + 1, ATTR_HIGHLIGHT, ATTR_TITLE,
		"Total value:   ", " %s ", buf);

	free(buf);
    }

    wait_for_key(curwin, getmaxy(curwin) - 2, ATTR_WAITFORKEY);
    deltxwin();
    txrefresh();
}


/***********************************************************************/
// total_value: Calculate a player's total financial worth

double total_value (int num)
{
    double val;
    int i;


    assert(num >= 0 && num < number_players);

    val = player[num].cash - player[num].debt;
    for (i = 0; i < MAX_COMPANIES; i++) {
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
