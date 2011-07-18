/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, move.c, contains the implementation of functions that make
  and process a game move in Star Traders.


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
*                      Module constants and macros                      *
************************************************************************/

#define GALAXY_MAP_LEFT(x, y)	(((x) <= 0)     ? MAP_EMPTY : galaxy_map[(x) - 1][(y)])
#define GALAXY_MAP_RIGHT(x, y)	(((x) >= MAX_X) ? MAP_EMPTY : galaxy_map[(x) + 1][(y)])
#define GALAXY_MAP_UP(x, y)	(((y) <= 0)     ? MAP_EMPTY : galaxy_map[(x)][(y) - 1])
#define GALAXY_MAP_DOWN(x, y)	(((y) >= MAX_Y) ? MAP_EMPTY : galaxy_map[(x)][(y) + 1])

#define assign_vals(x, y, left, right, up, down)			\
    do {								\
	(left)  = GALAXY_MAP_LEFT((x), (y));				\
	(right) = GALAXY_MAP_RIGHT((x), (y));				\
	(up)    = GALAXY_MAP_UP((x), (y));				\
	(down)  = GALAXY_MAP_DOWN((x), (y));				\
    } while (0)


/************************************************************************
*                    Internal function declarations                     *
************************************************************************/

void bankrupt_player (bool forced);
void try_start_new_company (int x, int y);
void merge_companies (map_val_t a, map_val_t b);
void include_outpost (int num, int x, int y);
void inc_share_price (int num, double inc);
void adjust_values (void);

int cmp_game_move (const void *a, const void *b);


/************************************************************************
*                    Game move function definitions                     *
************************************************************************/

/*-----------------------------------------------------------------------
  Function:   select_moves  - Select NUMBER_MOVES random moves
  Arguments:  (none)
  Returns:    (nothing)

  This function selects NUMBER_MOVES random moves and stores them in the
  game_move[] array.  If there are less than NUMBER_MOVES empty spaces in
  the galaxy map, the game is automatically finished by setting
  quit_selected to true.
*/

void select_moves (void)
{
    int count;
    int x, y, i, j;
    int tx, ty;
    bool unique;


    // How many empty spaces are there in the galaxy map?
    count = 0;
    for (x = 0; x < MAX_X; x++) {
	for (y = 0; y < MAX_Y; y++) {
	    if (galaxy_map[x][y] == MAP_EMPTY) {
		count++;
	    }
	}
    }

    if (count < NUMBER_MOVES) {
	quit_selected = true;
	return;
    }

    // Generate unique random moves
    for (i = 0; i < NUMBER_MOVES; i++) {
	do {
	    do {
		tx = randi(MAX_X);
		ty = randi(MAX_Y);
	    } while (galaxy_map[tx][ty] != MAP_EMPTY);

	    unique = true;
	    for (j = i - 1; j >= 0; j--) {
		if (tx == game_move[j].x && ty == game_move[j].y) {
		    unique = false;
		    break;
		}
	    }
	} while (! unique);

	game_move[i].x = tx;
	game_move[i].y = ty;
    }

    // Sort moves from left to right
    qsort(game_move, NUMBER_MOVES, sizeof(move_rec_t), cmp_game_move);

    quit_selected = false;
}


/*-----------------------------------------------------------------------
  Function:   get_move     - Wait for the player to enter their move
  Arguments:  (none)
  Returns:    selection_t  - Choice selected by player

  This function displays the galaxy map and the current moves, then waits
  for the player to select one of the moves.  On entry, current_player
  points to the current player; quit_selected and/or abort_game may be
  true (if so, get_move() justs returns without waiting for the player to
  select a move).  The return value is the choice made by the player.

  Note that two windows (the "Select move" window and the galaxy map
  window) are left on the screen: they are closed in process_move().
*/

selection_t get_move (void)
{
    int i, x, y;
    selection_t selection = SEL_NONE;


    if (quit_selected || abort_game) {
	return SEL_QUIT;
    }

    // Display map without closing window
    show_map(false);

    // Display current move choices on the galaxy map
    for (i = 0; i < NUMBER_MOVES; i++) {
	mvwaddch(curwin, game_move[i].y + 3, game_move[i].x * 2 + 2,
		 MOVE_TO_KEY(i) | ATTR_MAP_CHOICE);
    }
    wrefresh(curwin);

    // Show menu of choices for the player
    newtxwin(5, 80, LINE_OFFSET + 19, COL_CENTER(80));
    while (selection == SEL_NONE) {
	wbkgd(curwin, ATTR_NORMAL_WINDOW);
	werase(curwin);
	box(curwin, 0, 0);

	wmove(curwin, 2, 2);
	attrpr(curwin, ATTR_KEYCODE_STR, "<1>");
	waddstr(curwin, " Display stock portfolio");

	wmove(curwin, 3, 2);
	attrpr(curwin, ATTR_KEYCODE_STR, "<2>");
	waddstr(curwin, " Declare bankruptcy");

	wmove(curwin, 2, 41);
	attrpr(curwin, ATTR_KEYCODE_STR, "<3>");
	waddstr(curwin, "  Save and end the game");

	wmove(curwin, 3, 40);
	attrpr(curwin, ATTR_KEYCODE_STR, "<ESC>");
	waddstr(curwin, " Quit the game");

	mvwaddstr(curwin, 1, 2, "          Select move ");
	waddstr(curwin, "[");
	attrpr(curwin, ATTR_MAP_CHOICE, "%c", MOVE_TO_KEY(0));
	waddstr(curwin, "-");
	attrpr(curwin, ATTR_MAP_CHOICE, "%c", MOVE_TO_KEY(NUMBER_MOVES - 1));
	waddstr(curwin, "/");
	attrpr(curwin, ATTR_KEYCODE_STR, "1");
	waddstr(curwin, "-");
	attrpr(curwin, ATTR_KEYCODE_STR, "3");
	waddstr(curwin, "/");
	attrpr(curwin, ATTR_KEYCODE_STR, "<ESC>");
	waddstr(curwin, "]: ");

	curs_set(CURS_ON);
	wrefresh(curwin);

	// Get the actual selection made by the player
	while (selection == SEL_NONE) {
	    int key = tolower(gettxchar(curwin));

	    if (IS_MOVE_KEY(key)) {
		selection = KEY_TO_MOVE(key);

		curs_set(CURS_OFF);
		waddstr(curwin, "Move ");
		attrpr(curwin, ATTR_MAP_CHOICE, "%c", key);
	    } else {
		switch (key) {
		case '1':
		    curs_set(CURS_OFF);
		    show_status(current_player);
		    curs_set(CURS_ON);
		    break;

		case '2':
		    selection = SEL_BANKRUPT;

		    curs_set(CURS_OFF);
		    wattron(curwin, A_BOLD);
		    waddstr(curwin, "<2>");
		    wattroff(curwin, A_BOLD);
		    waddstr(curwin, " (Declare bankruptcy)");
		    break;

		case '3':
		    selection = SEL_SAVE;

		    curs_set(CURS_OFF);
		    wattron(curwin, A_BOLD);
		    waddstr(curwin, "<3>");
		    wattroff(curwin, A_BOLD);
		    waddstr(curwin, " (Save and end the game)");
		    break;

		case KEY_ESC:
		case KEY_CANCEL:
		case KEY_CTRL('C'):
		case KEY_CTRL('G'):
		case KEY_CTRL('\\'):
		    selection = SEL_QUIT;

		    curs_set(CURS_OFF);
		    wattron(curwin, A_BOLD);
		    waddstr(curwin, "<ESC>");
		    wattroff(curwin, A_BOLD);
		    waddstr(curwin, " (Quit the game)");
		    break;

		default:
		    beep();
		    break;
		}
	    }
	}

	// Clear the menu choices
	wattrset(curwin, ATTR_NORMAL_WINDOW);
	for (y = 2; y < 4; y++) {
	    wmove(curwin, y, 2);
	    for (x = 2; x < getmaxx(curwin) - 2; x++) {
		waddch(curwin, ' ' | ATTR_NORMAL_WINDOW);
	    }
	}
	wrefresh(curwin);

	// Ask the player to confirm their choice
	mvwaddstr(curwin, 2, 2, "                   Are you sure? ");
	waddstr(curwin, "[");
	attrpr(curwin, ATTR_KEYCODE_STR, "Y");
	waddstr(curwin, "/");
	attrpr(curwin, ATTR_KEYCODE_STR, "N");
	waddstr(curwin, "] ");

	if (! answer_yesno(curwin)) {
	    selection = SEL_NONE;
	}

	// Save the game if required
	if (selection == SEL_SAVE) {
	    bool saved = false;

	    if (game_loaded) {
		// Save the game to the same game number
		newtxwin(5, 30, LINE_OFFSET + 7, COL_CENTER(30));
		wbkgd(curwin, ATTR_STATUS_WINDOW);
		box(curwin, 0, 0);
		center(curwin, 2, ATTR_STATUS_WINDOW,
		       "Saving game %d... ", game_num);
		wrefresh(curwin);

		saved = save_game(game_num);

		deltxwin();
		txrefresh();
	    }

	    if (! saved) {
		int key;
		bool done;

		// Ask which game to save
		newtxwin(6, 50, LINE_OFFSET + 8, COL_CENTER(50));
		wbkgd(curwin, ATTR_NORMAL_WINDOW);
		box(curwin, 0, 0);

		center(curwin, 1, ATTR_WINDOW_TITLE, "  Save Game  ");
		mvwaddstr(curwin, 3, 2, "Enter game number ");
		waddstr(curwin, "[");
		attrpr(curwin, ATTR_KEYCODE_STR, "1");
		waddstr(curwin, "-");
		attrpr(curwin, ATTR_KEYCODE_STR, "9");
		waddstr(curwin, "]");
		waddstr(curwin, " or ");
		attrpr(curwin, ATTR_KEYCODE_STR, "<ESC>");
		waddstr(curwin, " to cancel: ");

		curs_set(CURS_ON);
		wrefresh(curwin);

		do {
		    key = gettxchar(curwin);
		    done = (key >= '1' && key <= '9');

		    switch (key) {
		    case KEY_ESC:
		    case KEY_CANCEL:
		    case KEY_CTRL('C'):
		    case KEY_CTRL('G'):
		    case KEY_CTRL('\\'):
			key = KEY_ESC;
			done = true;
			break;

		    default:
			// Do nothing
			break;
		    }

		    if (! done) {
			beep();
		    }
		} while (! done);

		curs_set(CURS_OFF);

		if (key != KEY_ESC) {
		    game_num = key - '0';

		    wechochar(curwin, key | A_BOLD);

		    // Try to save the game, if possible
		    newtxwin(5, 30, LINE_OFFSET + 7, COL_CENTER(30));
		    wbkgd(curwin, ATTR_STATUS_WINDOW);
		    box(curwin, 0, 0);
		    center(curwin, 2, ATTR_STATUS_WINDOW,
			   "Saving game %d... ", game_num);
		    wrefresh(curwin);

		    saved = save_game(game_num);

		    deltxwin();
		    txrefresh();
		}

		deltxwin();		// "Enter game number" window
		txrefresh();
	    }

	    if (saved) {
		selection = SEL_QUIT;
	    } else {
		// Make the next try at saving ask the player for a game number
		game_loaded = false;
		game_num = 0;

		selection = SEL_NONE;
	    }
	}
    }

    return selection;
}


/*-----------------------------------------------------------------------
  Function:   process_move  - Process the move selected by the player
  Arguments:  selection     - Selection made by current player
  Returns:    (nothing)

  This function processes the move in selection.  It assumes the "Select
  move" and galaxy map windows are still open.
*/

void process_move (selection_t selection)
{
    if (! quit_selected && ! abort_game) {
	switch(selection) {
	case SEL_QUIT:
	    // The players want to end the game
	    quit_selected = true;
	    break;

	case SEL_BANKRUPT:
	    // A player wants to give up: make them bankrupt
	    bankrupt_player(false);
	    break;

	default:
	    // Process a selection from game_move[]
	    {
		assert(selection >= SEL_MOVE_FIRST && selection <= SEL_MOVE_LAST);

		map_val_t left, right, up, down;
		map_val_t nearby, cur;

		int x = game_move[selection].x;
		int y = game_move[selection].y;


		assign_vals(x, y, left, right, up, down);

		if (left == MAP_EMPTY && right == MAP_EMPTY &&
		    up   == MAP_EMPTY && down  == MAP_EMPTY) {
		    // The position is out in the middle of nowhere...
		    galaxy_map[x][y] = MAP_OUTPOST;

		} else if (! IS_MAP_COMPANY(left) && ! IS_MAP_COMPANY(right) &&
			   ! IS_MAP_COMPANY(up)   && ! IS_MAP_COMPANY(down)) {
		    // See if a company can be established
		    try_start_new_company(x, y);

		} else {
		    // See if two (or more!) companies can be merged

		    if (IS_MAP_COMPANY(left) && IS_MAP_COMPANY(right) &&
			left != right) {
			galaxy_map[x][y] = left;
			merge_companies(left, right);
			assign_vals(x, y, left, right, up, down);
		    }

		    if (IS_MAP_COMPANY(left) && IS_MAP_COMPANY(up) &&
			left != up) {
			galaxy_map[x][y] = left;
			merge_companies(left, up);
			assign_vals(x, y, left, right, up, down);
		    }

		    if (IS_MAP_COMPANY(left) && IS_MAP_COMPANY(down) &&
			left != down) {
			galaxy_map[x][y] = left;
			merge_companies(left, down);
			assign_vals(x, y, left, right, up, down);
		    }

		    if (IS_MAP_COMPANY(right) && IS_MAP_COMPANY(up) &&
			right != up) {
			galaxy_map[x][y] = right;
			merge_companies(right, up);
			assign_vals(x, y, left, right, up, down);
		    }

		    if (IS_MAP_COMPANY(right) && IS_MAP_COMPANY(down) &&
			right != down) {
			galaxy_map[x][y] = right;
			merge_companies(right, down);
			assign_vals(x, y, left, right, up, down);
		    }

		    if (IS_MAP_COMPANY(up) && IS_MAP_COMPANY(down) &&
			up != down) {
			galaxy_map[x][y] = up;
			merge_companies(up, down);
			assign_vals(x, y, left, right, up, down);
		    }
		}

		// See if an existing company can be expanded
		nearby = (IS_MAP_COMPANY(left)    ? left :
			  (IS_MAP_COMPANY(right)  ? right :
			   (IS_MAP_COMPANY(up)    ? up :
			    (IS_MAP_COMPANY(down) ? down :
			     MAP_EMPTY))));
		if (nearby != MAP_EMPTY) {
		    galaxy_map[x][y] = nearby;
		    inc_share_price(MAP_TO_COMPANY(nearby), SHARE_PRICE_INC);
		}

		// If a company expanded (or merged or formed), see if
		// share price should be incremented
		cur = galaxy_map[x][y];
		if (IS_MAP_COMPANY(cur)) {

		    // Is a star nearby?
		    if (left == MAP_STAR) {
			inc_share_price(MAP_TO_COMPANY(cur), SHARE_PRICE_INC_STAR);
		    }
		    if (right == MAP_STAR) {
			inc_share_price(MAP_TO_COMPANY(cur), SHARE_PRICE_INC_STAR);
		    }
		    if (up == MAP_STAR) {
			inc_share_price(MAP_TO_COMPANY(cur), SHARE_PRICE_INC_STAR);
		    }
		    if (down == MAP_STAR) {
			inc_share_price(MAP_TO_COMPANY(cur), SHARE_PRICE_INC_STAR);
		    }

		    // Is an outpost nearby?
		    if (left == MAP_OUTPOST) {
			include_outpost(MAP_TO_COMPANY(cur), x - 1, y);
		    }
		    if (right == MAP_OUTPOST) {
			include_outpost(MAP_TO_COMPANY(cur), x + 1, y);
		    }
		    if (up == MAP_OUTPOST) {
			include_outpost(MAP_TO_COMPANY(cur), x, y - 1);
		    }
		    if (down == MAP_OUTPOST) {
			include_outpost(MAP_TO_COMPANY(cur), x, y + 1);
		    }
		}
	    }
	    break;
	}

	if (! quit_selected) {
	    adjust_values();
	}
    }

    deltxwin();			// "Select move" window
    deltxwin();			// Galaxy map window
    txrefresh();
}


/*-----------------------------------------------------------------------
  Function:   next_player  - Get the next player
  Arguments:  (none)
  Returns:    (nothing)

  This function sets the global variable current_player to the next
  eligible player.  If no player is still in the game, quit_selected is
  set to true.  The variable turn_number is also incremented if required.
*/

void next_player (void)
{
    int i;
    bool all_out;


    all_out = true;
    for (i = 0; i < number_players; i++) {
	if (player[i].in_game) {
	    all_out = false;
	    break;
	}
    }

    if (all_out) {
	quit_selected = true;
    } else {
	do {
	    current_player++;
	    if (current_player == number_players) {
		current_player = 0;
	    }
	    if (current_player == first_player) {
		turn_number++;
	    }
	} while (! player[current_player].in_game);
    }
}


/*-----------------------------------------------------------------------
  Function:   bankrupt_player  - Make the current player bankrupt
  Arguments:  forced           - True if bankruptcy is forced by Bank
  Returns:    (nothing)

  This function makes the current player bankrupt, whether by their own
  choice or as a result of action by the Interstellar Trading Bank.
  On exit, quit_selected is true if all players are bankrupt.
*/

void bankrupt_player (bool forced)
{
    int i;
    int longname = (strlen(player[current_player].name) > 20);

    if (forced) {
	newtxwin((longname ? 9 : 8), 54, LINE_OFFSET + 7, COL_CENTER(54));
    } else {
	newtxwin((longname ? 8 : 7), 50, LINE_OFFSET + 7, COL_CENTER(50));
    }
    wbkgd(curwin, ATTR_ERROR_WINDOW);
    box(curwin, 0, 0);

    center(curwin, 1, ATTR_ERROR_TITLE, "  Bankruptcy Court  ");

    if (forced) {
	if (longname) {
	    center(curwin, 3, ATTR_ERROR_STR, "%s", player[current_player].name);
	    center(curwin, 4, ATTR_ERROR_STR, "has been declared bankrupt by the");
	    center(curwin, 5, ATTR_ERROR_STR, "Interstellar Trading Bank");
	} else {
	    center(curwin, 3, ATTR_ERROR_STR, "%s has been declared bankrupt",
		   player[current_player].name);
	    center(curwin, 4, ATTR_ERROR_STR, "by the Interstellar Trading Bank");
	}
    } else {
	if (longname) {
	    center(curwin, 3, ATTR_ERROR_STR, "%s", player[current_player].name);
	    center(curwin, 4, ATTR_ERROR_STR, "has declared bankruptcy");
	} else {
	    center(curwin, 3, ATTR_ERROR_STR, "%s has declared bankruptcy",
		   player[current_player].name);
	}
    }

    wait_for_key(curwin, getmaxy(curwin) - 2, ATTR_WAITERROR_STR);

    deltxwin();
    txrefresh();

    // Confiscate all assets belonging to player
    player[current_player].in_game = false;
    for (i = 0; i < MAX_COMPANIES; i++) {
	company[i].stock_issued -= player[current_player].stock_owned[i];
	player[current_player].stock_owned[i] = 0;
    }
    player[current_player].cash = 0.0;
    player[current_player].debt = 0.0;

    // Is anyone still left in the game?
    bool all_out = true;
    for (i = 0; i < number_players; i++) {
	if (player[i].in_game) {
	    all_out = false;
	    break;
	}
    }

    if (all_out) {
	quit_selected = true;
    }
}


/*-----------------------------------------------------------------------
  Function:   try_start_new_company  - See if a new company can be started
  Arguments:  x, y                   - Coordinates of position on map
  Returns:    (nothing)

  This function attempts to establish a new company if the position (x,y)
  is in a suitable location and if no more than MAX_COMPANIES are already
  present.
*/

void try_start_new_company (int x, int y)
{
    bool all_on_map;
    map_val_t left, right, up, down;
    int i, j;


    assign_vals(x, y, left, right, up, down);

    if (left  != MAP_OUTPOST && left  != MAP_STAR &&
	right != MAP_OUTPOST && right != MAP_STAR &&
	up    != MAP_OUTPOST && up    != MAP_STAR &&
	down  != MAP_OUTPOST && down  != MAP_STAR) {
	return;
    }

    all_on_map = true;
    for (i = 0; i < MAX_COMPANIES; i++) {
	if (! company[i].on_map) {
	    all_on_map = false;
	    break;
	}
    }

    if (all_on_map) {
	// The galaxy cannot support any more companies
	galaxy_map[x][y] = MAP_OUTPOST;
    } else {
	// Create the new company

	newtxwin(8, 50, LINE_OFFSET + 7, COL_CENTER(50));
	wbkgd(curwin, ATTR_NORMAL_WINDOW);
	box(curwin, 0, 0);

	center(curwin, 1, ATTR_WINDOW_TITLE, "  New Company  ");
	center(curwin, 3, ATTR_NORMAL_WINDOW, "A new company has been formed!");
	center2(curwin, 4, ATTR_NORMAL_WINDOW, ATTR_HIGHLIGHT_STR,
		"Its name is ", "%s", company[i].name);

	wait_for_key(curwin, 6, ATTR_WAITNORMAL_STR);

	deltxwin();
	txrefresh();

	galaxy_map[x][y] = COMPANY_TO_MAP(i);

	company[i].share_price = INITIAL_SHARE_PRICE;
	company[i].share_return = INITIAL_RETURN;
	company[i].stock_issued = INITIAL_STOCK_ISSUED;
	company[i].max_stock = INITIAL_MAX_STOCK;
	company[i].on_map = true;

	for (j = 0; j < number_players; j++) {
	    player[j].stock_owned[i] = 0;
	}

	player[current_player].stock_owned[i] = INITIAL_STOCK_ISSUED;
    }
}


/*-----------------------------------------------------------------------
  Function:   merge_companies  - Merge two companies together
  Arguments:  a, b             - Companies to merge
  Returns:    (nothing)

  This function merges two companies on the galaxy map; the one with the
  highest value takes over.  The parameters a and b are actual values
  from the galaxy map.
*/

void merge_companies (map_val_t a, map_val_t b)
{
    int aa = MAP_TO_COMPANY(a);
    int bb = MAP_TO_COMPANY(b);

    double val_aa = company[aa].share_price * company[aa].stock_issued *
	company[aa].share_return;
    double val_bb = company[bb].share_price * company[bb].stock_issued *
	company[bb].share_return;

    int x, y, i, line;
    long old_stock, new_stock, total_new;
    double bonus;


    char *buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit("out of memory");
    }


    if (val_aa < val_bb) {
	// Make sure aa is the dominant company
	map_val_t t;
	int tt;

	t  = a;  a  = b;  b  = t;
	tt = aa; aa = bb; bb = tt;
    }

    // Display information about the merger

    newtxwin(number_players + 14, 76, LINE_OFFSET + (9 - number_players),
	     COL_CENTER(76));
    wbkgd(curwin, ATTR_NORMAL_WINDOW);
    box(curwin, 0, 0);

    center(curwin, 1, ATTR_WINDOW_TITLE, "  Company Merger  ");
    center3(curwin, 3, ATTR_HIGHLIGHT_STR, ATTR_HIGHLIGHT_STR, ATTR_NORMAL_WINDOW,
	    company[bb].name, company[aa].name, " has just merged into ");

    center(curwin, 5, ATTR_NORMAL_WINDOW, "Please note the following transactions:");

    center2(curwin, 7, ATTR_NORMAL_WINDOW, ATTR_HIGHLIGHT_STR, " Old stock: ",
	    "%-20s", company[bb].name);
    center2(curwin, 8, ATTR_NORMAL_WINDOW, ATTR_HIGHLIGHT_STR, " New stock: ",
	    "%-20s", company[aa].name);

    // Handle the locale's currency symbol
    struct lconv *lc = localeconv();
    assert(lc != NULL);

    snprintf(buf, BUFSIZE, "Bonus (%s)", lc->currency_symbol);

    int w = getmaxx(curwin) - 52;
    wattrset(curwin, ATTR_WINDOW_SUBTITLE);
    mvwprintw(curwin, 10, 2, "  %-*.*s  %8s  %8s  %8s  %12s  ", w, w,
	      "Player", "Old", "New", "Total", buf);
    wattrset(curwin, ATTR_NORMAL_WINDOW);

    total_new = 0;
    for (line = 11, i = 0; i < number_players; i++) {
	if (player[i].in_game) {
	    // Calculate new stock and any bonus
	    old_stock = player[i].stock_owned[bb];
	    new_stock = (double) old_stock * MERGE_STOCK_RATIO;
	    total_new += new_stock;

	    bonus = (company[bb].stock_issued == 0) ? 0.0 :
		10.0 * ((double) player[i].stock_owned[bb] /
			company[bb].stock_issued) * company[bb].share_price;

	    player[i].stock_owned[aa] += new_stock;
	    player[i].stock_owned[bb] = 0;
	    player[i].cash += bonus;

	    strfmon(buf, BUFSIZE, "%!12n", bonus);
	    mvwprintw(curwin, line, 2, "  %-*.*s  %'8ld  %'8ld  %'8ld  %12s  ",
		      w, w, player[i].name, old_stock, new_stock,
		      player[i].stock_owned[aa], buf);
	    line++;
	}
    }

    // Adjust the company records appropriately
    company[aa].stock_issued += total_new;
    company[aa].max_stock    += total_new;
    company[aa].share_price  += company[bb].share_price / (randf() + 1.5);

    company[bb].stock_issued = 0;
    company[bb].max_stock    = 0;
    company[bb].on_map       = false;

    // Adjust the galaxy map appropriately
    for (x = 0; x < MAX_X; x++) {
	for (y = 0; y < MAX_Y; y++) {
	    if (galaxy_map[x][y] == b) {
		galaxy_map[x][y] = a;
	    }
	}
    }

    wait_for_key(curwin, getmaxy(curwin) - 2, ATTR_WAITNORMAL_STR);

    deltxwin();			// "Company merger" window
    txrefresh();

    free(buf);
}


/*-----------------------------------------------------------------------
  Function:   include_outpost  - Include any outposts into the company
  Arguments:  num              - Company on which to operate
              x, y             - Coordinates of position on map
  Returns:    (nothing)

  This function includes the outpost at (x,y) into company num.  It also
  checks surrounding locations for further outposts.
*/

void include_outpost (int num, int x, int y)
{
    map_val_t left, right, up, down;


    assert(num >= 0 && num < MAX_COMPANIES);
    assert(x >= 0 && x < MAX_X);
    assert(y >= 0 && y < MAX_Y);

    assign_vals(x, y, left, right, up, down);

    galaxy_map[x][y] = COMPANY_TO_MAP(num);
    inc_share_price(num, SHARE_PRICE_INC_OUTPOST);

    // Outposts next to stars are more valuable: increment again
    if (left == MAP_STAR) {
	inc_share_price(num, SHARE_PRICE_INC_OUTSTAR);
    }
    if (right == MAP_STAR) {
	inc_share_price(num, SHARE_PRICE_INC_OUTSTAR);
    }
    if (up == MAP_STAR) {
	inc_share_price(num, SHARE_PRICE_INC_OUTSTAR);
    }
    if (down == MAP_STAR) {
	inc_share_price(num, SHARE_PRICE_INC_OUTSTAR);
    }

    if (left == MAP_OUTPOST) {
	include_outpost(num, x - 1, y);
    }
    if (right == MAP_OUTPOST) {
	include_outpost(num, x + 1, y);
    }
    if (up == MAP_OUTPOST) {
	include_outpost(num, x, y - 1);
    }
    if (down == MAP_OUTPOST) {
	include_outpost(num, x, y + 1);
    }
}


/*-----------------------------------------------------------------------
  Function:   inc_share_price  - Increase the share price of a company
  Arguments:  num              - Company on which to operate
              inc              - Base increment for the share price
  Returns:    (nothing)

  This function increments the share price, maximum stock available and
  the share return of company num, using inc as the basis for doing so.
*/

void inc_share_price (int num, double inc)
{
    company[num].share_price += inc * (1.0 + randf() * SHARE_PRICE_INC_EXTRA);
    company[num].max_stock   += inc / (randf() * 10.0 + 5.0);
    if (randf() < PROB_CHANGE_RETURN) {
	company[num].share_return *= randf() + CHANGE_RETURN_INC;
    }
}


/*-----------------------------------------------------------------------
  Function:   adjust_values  - Adjust various company-related values
  Arguments:  (none)
  Returns:    (nothing)

  This function adjusts the cost of shares for companies on the galaxy
  map, their return, the Bank interest rate, etc.
*/

void adjust_values (void)
{
    int i, x, y;
    int which;

    // Declare a company bankrupt!
    if (randf() > (1.0 - COMPANY_BANKRUPTCY)) {
	which = randi(MAX_COMPANIES);

	if (company[which].on_map) {
	    if (randf() < ALL_ASSETS_TAKEN) {
		newtxwin(10, 60, LINE_OFFSET + 6, COL_CENTER(60));
		wbkgd(curwin, ATTR_ERROR_WINDOW);
		box(curwin, 0, 0);

		center(curwin, 1, ATTR_ERROR_TITLE, "  Bankruptcy Court  ");
		center(curwin, 3, ATTR_ERROR_STR, "%s has been declared",
			company[which].name);
		center(curwin, 4, ATTR_ERROR_STR,
		       "bankrupt by the Interstellar Trading Bank.");

		center(curwin, 6, ATTR_ERROR_WINDOW,
		       "All assets have been taken to repay outstanding loans.");

		wait_for_key(curwin, 8, ATTR_WAITERROR_STR);
		deltxwin();
		txrefresh();

	    } else {
		double rate = randf();
		char *buf;

		buf = malloc(BUFSIZE);
		if (buf == NULL) {
		    err_exit("out of memory");
		}

		for (i = 0; i < number_players; i++) {
		    if (player[i].in_game) {
			player[i].cash += player[i].stock_owned[which] * rate;
		    }
		}

		newtxwin(14, 60, LINE_OFFSET + 4, COL_CENTER(60));
		wbkgd(curwin, ATTR_ERROR_WINDOW);
		box(curwin, 0, 0);

		center(curwin, 1, ATTR_ERROR_TITLE, "  Bankruptcy Court  ");
		center(curwin, 3, ATTR_ERROR_STR, "%s has been declared",
			company[which].name);
		center(curwin, 4, ATTR_ERROR_STR,
		       "bankrupt by the Interstellar Trading Bank.");

		center2(curwin, 6, ATTR_ERROR_WINDOW, ATTR_ERROR_STR,
			"The Bank has agreed to pay stock holders ",
			"%4.2f%%", rate * 100.0);
		center(curwin, 7, ATTR_ERROR_WINDOW,
		       "of the share value on each share owned.");

		strfmon(buf, BUFSIZE, "%12n", company[which].share_price);
		center2(curwin, 9, ATTR_ERROR_WINDOW, ATTR_ERROR_STR,
			"Old share value:       ", "%s", buf);

		strfmon(buf, BUFSIZE, "%12n", company[which].share_price * rate);
		center2(curwin, 10, ATTR_ERROR_WINDOW, ATTR_ERROR_STR,
			"Amount paid per share: ", "%s", buf);

		wait_for_key(curwin, 12, ATTR_WAITERROR_STR);
		deltxwin();
		txrefresh();

		free(buf);
	    }

	    for (i = 0; i < number_players; i++) {
		player[i].stock_owned[which] = 0;
	    }

	    company[which].share_price  = 0.0;
	    company[which].share_return = 0.0;
	    company[which].stock_issued = 0;
	    company[which].max_stock    = 0;
	    company[which].on_map       = false;

	    for (x = 0; x < MAX_X; x++) {
		for (y = 0; y < MAX_Y; y++) {
		    if (galaxy_map[x][y] == COMPANY_TO_MAP(which)) {
			galaxy_map[x][y] = MAP_EMPTY;
		    }
		}
	    }
	}
    }

    // Increase or decrease company return
    if (randf() < CHANGE_COMPANY_RETURN) {
	which = randi(MAX_COMPANIES);
	if (company[which].on_map) {
	    company[which].share_return *= randf() + COMPANY_RETURN_INC;
	}
    }

    // Make sure that a company's return is not too large
    for (i = 0; i < MAX_COMPANIES; i++) {
	if (company[i].on_map && company[i].share_return > MAX_COMPANY_RETURN) {
	    company[i].share_return /= randf() + RETURN_DIVIDER;
	}
    }

    // Increase or decrease share price
    if (randf() < INC_SHARE_PRICE) {
	which = randi(MAX_COMPANIES);
	if (company[which].on_map) {
	    double change = randf() * company[which].share_price * PRICE_CHANGE_RATE;
	    if (randf() < DEC_SHARE_PRICE) {
		change = -change;
	    }
	    company[which].share_price += change;
	}
    }

    // Give the current player the companies' dividends
    for (i = 0; i < MAX_COMPANIES; i++) {
	if (company[i].on_map && company[i].stock_issued != 0) {
	    player[current_player].cash += player[current_player].stock_owned[i] *
		company[i].share_price * company[i].share_return +
		((double) player[current_player].stock_owned[i] /
		 company[i].stock_issued) * company[i].share_price *
		OWNERSHIP_BONUS;
	}
    }

    // Change the interest rate
    if (randf() < CHANGE_INTEREST_RATE) {
	interest_rate *= randf() + INTEREST_RATE_INC;
    }
    if (interest_rate > MAX_INTEREST_RATE) {
	interest_rate /= randf() + INTEREST_RATE_DIVIDER;
    }

    // Calculate current player's debt
    player[current_player].debt *= interest_rate + 1.0;

    // Check if a player's debt is too large
    if (total_value(current_player) <= -MAX_OVERDRAFT) {
	char *buf;

	buf = malloc(BUFSIZE);
	if (buf == NULL) {
	    err_exit("out of memory");
	}

	newtxwin(8, 60, LINE_OFFSET + 7, COL_CENTER(60));
	wbkgd(curwin, ATTR_ERROR_WINDOW);
	box(curwin, 0, 0);

	center(curwin, 1, ATTR_ERROR_TITLE, "  Interstellar Trading Bank  ");

	strfmon(buf, BUFSIZE, "%1n", player[current_player].debt);
	center(curwin, 3, ATTR_ERROR_STR, "Your debt has amounted to %s", buf);

	strfmon(buf, BUFSIZE, "%1n", player[current_player].cash);
	center3(curwin, 4, ATTR_ERROR_WINDOW, ATTR_ERROR_WINDOW, ATTR_ERROR_STR,
		"The Bank has impounded ", " from your cash", "%s", buf);

	wait_for_key(curwin, 6, ATTR_WAITERROR_STR);
	deltxwin();
	txrefresh();

	player[current_player].debt /= interest_rate + 1.0;
	player[current_player].debt -= player[current_player].cash;
	player[current_player].cash = 0.0;
	if (player[current_player].debt < ROUNDING_AMOUNT) {
	    player[current_player].debt = 0.0;
	}

	// Shall we declare them bankrupt?
	if (total_value(current_player) <= 0.0 && randf() < PROB_BANKRUPTCY) {
	    bankrupt_player(true);
	}
    }
}


/*-----------------------------------------------------------------------
  Function:   cmp_game_move  - Compare two game_move[] elements
  Arguments:  a, b           - Elements to compare
  Returns:    int            - Comparison of a and b

  This function compares two game_move[] elements (of type move_rec_t)
  and returns -1 if a < b, 0 if a == b and 1 if a > b.  It is used for
  sorting game moves into ascending order.
*/

int cmp_game_move (const void *a, const void *b)
{
    const move_rec_t *aa = (const move_rec_t *) a;
    const move_rec_t *bb = (const move_rec_t *) b;


    if (aa->x < bb->x) {
	return -1;
    } else if (aa->x > bb->x) {
	return 1;
    } else {
	if (aa->y < bb->y) {
	    return -1;
	} else if (aa->y > bb->y) {
	    return 1;
	} else {
	    return 0;
	}
    }
}
