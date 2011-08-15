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
*                        Module-specific macros                         *
************************************************************************/

// Calculate positions near (x,y), taking the edge of the galaxy into account

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
*                  Module-specific function prototypes                  *
************************************************************************/

/*
  Function:   bankrupt_player - Make the current player bankrupt
  Parameters: forced          - True if bankruptcy is forced by Bank
  Returns:    (nothing)

  This function makes the current player bankrupt, whether by their own
  choice or as a result of action by the Interstellar Trading Bank.  All
  shares are returned to the appropriate companies, any debt is cancelled
  and any cash is confiscated.  On exit, quit_selected is true if all
  players are bankrupt.
*/
static void bankrupt_player (bool forced);


/*
  Function:   try_start_new_company - See if a new company can be started
  Parameters: x, y                  - Coordinates of position on map
  Returns:    (nothing)

  This function attempts to establish a new company if the position (x,y)
  is in a suitable location and if no more than MAX_COMPANIES are already
  present.
*/
static void try_start_new_company (int x, int y);


/*
  Function:   merge_companies - Merge two companies together
  Parameters: a, b            - Companies to merge
  Returns:    (nothing)

  This function merges two companies on the galaxy map; the one with the
  highest value takes over.  The parameters a and b are actual values
  from the galaxy map.
*/
static void merge_companies (map_val_t a, map_val_t b);


/*
  Function:   include_outpost - Include any outposts into the company
  Parameters: num             - Company on which to operate
              x, y            - Coordinates of position on map
  Returns:    (nothing)

  This function includes the outpost at (x,y) into company num,
  increasing the share price by calling inc_share_price().  It also
  checks surrounding locations for further outposts to include.
*/
static void include_outpost (int num, int x, int y);


/*
  Function:   inc_share_price - Increase the share price of a company
  Parameters: num             - Company on which to operate
              inc             - Base increment for the share price
  Returns:    (nothing)

  This function increments the share price, maximum stock available and
  the share return of company num, using inc as the basis for doing so.
*/
static void inc_share_price (int num, double inc);


/*
  Function:   adjust_values - Adjust various company-related values
  Parameters: (none)
  Returns:    (nothing)

  This function adjusts the cost of shares for companies on the galaxy
  map, their return, the Bank interest rate, etc.
*/
static void adjust_values (void);


/*
  Function:   cmp_game_move - Compare two game_move[] elements for sorting
  Parameters: a, b          - Elements to compare
  Returns:    int           - Comparison of a and b

  This internal function compares two game_move[] elements (of type
  move_rec_t) and returns -1 if a < b, 0 if a == b and 1 if a > b.  It is
  used for sorting game moves into ascending order.
*/
static int cmp_game_move (const void *a, const void *b);


/************************************************************************
*                    Game move function definitions                     *
************************************************************************/

// These functions are documented in the file "move.h"


/***********************************************************************/
// select_moves: Select NUMBER_MOVES random moves

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


/***********************************************************************/
// get_move: Wait for the player to enter their move

selection_t get_move (void)
{
    selection_t selection = SEL_NONE;


    if (quit_selected || abort_game) {
	return SEL_QUIT;
    }

    // Display map without closing window
    show_map(false);

    // Display current move choices on the galaxy map
    for (int i = 0; i < NUMBER_MOVES; i++) {
	mvwaddch(curwin, game_move[i].y + 3, game_move[i].x * 2 + 2,
		 MOVE_TO_KEY(i) | attr_map_choice);
    }
    wrefresh(curwin);

    // Show menu of choices for the player
    newtxwin(5, WIN_COLS, 19, WCENTER, false, 0);
    while (selection == SEL_NONE) {
	wbkgd(curwin, attr_normal_window);
	werase(curwin);
	box(curwin, 0, 0);

	left(curwin, 2, 2, attr_normal, attr_keycode, 0, 1,
	     "^{<1>^} Display stock portfolio");
	left(curwin, 3, 2, attr_normal, attr_keycode, 0, 1,
	     "^{<2>^} Declare bankruptcy");
	left(curwin, 2, getmaxx(curwin) / 2, attr_normal, attr_keycode, 0, 1,
	     "^{<3>^} Save and end the game");
	left(curwin, 3, getmaxx(curwin) / 2, attr_normal, attr_keycode, 0, 1,
	     "^{<CTRL><C>^} Quit the game");

	right(curwin, 1, getmaxx(curwin) / 2, attr_normal, attr_keycode,
	      attr_choice, 1, "Select move "
	      "[^[%c^]-^[%c^]/^{1^}-^{3^}/^{<CTRL><C>^}]: ",
	      MOVE_TO_KEY(0), MOVE_TO_KEY(NUMBER_MOVES - 1));

	curs_set(CURS_ON);
	wrefresh(curwin);

	// Get the actual selection made by the player
	while (selection == SEL_NONE) {
	    int key = tolower(gettxchar(curwin));

	    if (IS_MOVE_KEY(key)) {
		selection = KEY_TO_MOVE(key);

		curs_set(CURS_OFF);
		left(curwin, 1, getmaxx(curwin) / 2, attr_normal, attr_choice,
		     0, 1, "Move ^{%c^}", key);
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
		    left(curwin, 1, getmaxx(curwin) / 2, attr_normal,
			 attr_normal | A_BOLD, 0, 1,
			 "^{<2>^} (Declare bankruptcy)");
		    break;

		case '3':
		    selection = SEL_SAVE;

		    curs_set(CURS_OFF);
		    left(curwin, 1, getmaxx(curwin) / 2, attr_normal,
			 attr_normal | A_BOLD, 0, 1,
			 "^{<3>^} (Save and end the game)");
		    break;

		case KEY_ESC:
		case KEY_CANCEL:
		case KEY_EXIT:
		case KEY_CTRL('C'):
		case KEY_CTRL('G'):
		case KEY_CTRL('\\'):
		    selection = SEL_QUIT;

		    curs_set(CURS_OFF);
		    left(curwin, 1, getmaxx(curwin) / 2, attr_normal,
			 attr_normal | A_BOLD, 0, 1,
			 "^{<CTRL><C>^} (Quit the game)");
		    break;

		default:
		    beep();
		}
	    }
	}

	// Clear the menu choices (but not the prompt!)
	mvwhline(curwin, 2, 2, ' ' | attr_normal, getmaxx(curwin) - 4);
	mvwhline(curwin, 3, 2, ' ' | attr_normal, getmaxx(curwin) - 4);

	// Ask the player to confirm their choice
	right(curwin, 2, getmaxx(curwin) / 2, attr_normal, attr_keycode, 0, 1,
	      "Are you sure? [^{Y^}/^{N^}] ");
	wrefresh(curwin);

	if (! answer_yesno(curwin)) {
	    selection = SEL_NONE;
	}

	// Save the game if required
	if (selection == SEL_SAVE) {
	    chtype *chbuf = xmalloc(BUFSIZE * sizeof(chtype));
	    int width;

	    bool saved = false;

	    if (game_loaded) {
		// Save the game to the same game number
		mkchstr(chbuf, BUFSIZE, attr_status_window, 0, 0, 1, WIN_COLS
			- 7, &width, 1, "Saving game %d... ", game_num);
		newtxwin(5, width + 5, 7, WCENTER, true, attr_status_window);
		centerch(curwin, 2, 0, chbuf, 1, &width);
		wrefresh(curwin);

		saved = save_game(game_num);

		deltxwin();
		txrefresh();
	    }

	    if (! saved) {
		// Ask which game to save

		int key;
		bool done;
		int widthbuf[2];
		int lines, maxwidth;

		lines = mkchstr(chbuf, BUFSIZE, attr_normal, attr_keycode, 0,
				2, WIN_COLS - 7, widthbuf, 2,
				"Enter game number [^{1^}-^{9^}] "
				"or ^{<CTRL><C>^} to cancel: ");
		assert(lines == 1 || lines == 2);
		maxwidth = ((lines == 1) ? widthbuf[0] :
			    MAX(widthbuf[0], widthbuf[1])) + 5;

		newtxwin(lines + 4, maxwidth, 8, WCENTER, true,
			 attr_normal_window);
		leftch(curwin, 2, 2, chbuf, lines, widthbuf);

		curs_set(CURS_ON);
		wrefresh(curwin);

		done = false;
		while (! done) {
		    key = gettxchar(curwin);

		    if (key >= '1' && key <= '9') {
			wechochar(curwin, key | A_BOLD);
			done = true;
		    } else {
			switch (key) {
			case KEY_ESC:
			case KEY_CANCEL:
			case KEY_EXIT:
			case KEY_CTRL('C'):
			case KEY_CTRL('G'):
			case KEY_CTRL('\\'):
			    key = KEY_CANCEL;
			    done = true;
			    break;

			default:
			    beep();
			}
		    }
		}

		curs_set(CURS_OFF);

		if (key != KEY_CANCEL) {
		    // Try to save the game, if possible
		    game_num = key - '0';

		    mkchstr(chbuf, BUFSIZE, attr_status_window, 0, 0, 1,
			    WIN_COLS - 7, &width, 1,
			    "Saving game %d... ", game_num);
		    newtxwin(5, width + 5, 7, WCENTER, true, attr_status_window);
		    centerch(curwin, 2, 0, chbuf, 1, &width);
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

	    free(chbuf);
	}
    }

    return selection;
}


/***********************************************************************/
// process_move: Process the move selected by the player

void process_move (selection_t selection)
{
    if (selection == SEL_QUIT) {
	// The players want to end the game
	quit_selected = true;
    }

    if (quit_selected || abort_game) {
	deltxwin();			// "Select move" window
	deltxwin();			// Galaxy map window
	txrefresh();

	return;
    }

    if (selection == SEL_BANKRUPT) {
	// A player wants to give up: make them bankrupt
	bankrupt_player(false);

    } else {
	// Process a selection from game_move[]

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

	} else if (! IS_MAP_COMPANY(left)  && ! IS_MAP_COMPANY(right)
		   && ! IS_MAP_COMPANY(up) && ! IS_MAP_COMPANY(down)) {
	    // See if a company can be established
	    try_start_new_company(x, y);

	} else {
	    // See if two (or more!) companies can be merged

	    if (IS_MAP_COMPANY(left) && IS_MAP_COMPANY(right)
		&& left != right) {
		galaxy_map[x][y] = left;
		merge_companies(left, right);
		assign_vals(x, y, left, right, up, down);
	    }

	    if (IS_MAP_COMPANY(left) && IS_MAP_COMPANY(up)
		&& left != up) {
		galaxy_map[x][y] = left;
		merge_companies(left, up);
		assign_vals(x, y, left, right, up, down);
	    }

	    if (IS_MAP_COMPANY(left) && IS_MAP_COMPANY(down)
		&& left != down) {
		galaxy_map[x][y] = left;
		merge_companies(left, down);
		assign_vals(x, y, left, right, up, down);
	    }

	    if (IS_MAP_COMPANY(right) && IS_MAP_COMPANY(up)
		&& right != up) {
		galaxy_map[x][y] = right;
		merge_companies(right, up);
		assign_vals(x, y, left, right, up, down);
	    }

	    if (IS_MAP_COMPANY(right) && IS_MAP_COMPANY(down)
		&& right != down) {
		galaxy_map[x][y] = right;
		merge_companies(right, down);
		assign_vals(x, y, left, right, up, down);
	    }

	    if (IS_MAP_COMPANY(up) && IS_MAP_COMPANY(down)
		&& up != down) {
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

	/* If a company expanded (or merged or formed), see if share
	   price should be incremented */
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

    if (! quit_selected) {
	adjust_values();
    }

    deltxwin();			// "Select move" window
    deltxwin();			// Galaxy map window
    txrefresh();
}


/***********************************************************************/
// next_player: Get the next player

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


/************************************************************************
*                 Module-specific function definitions                  *
************************************************************************/

// These functions are documented at the start of this file


/***********************************************************************/
// bankrupt_player: Make the current player bankrupt

void bankrupt_player (bool forced)
{
    if (forced) {
	txdlgbox(MAX_DLG_LINES, 50, 7, WCENTER, attr_error_window,
		 attr_error_title, attr_error_highlight, 0, 0,
		 attr_error_waitforkey, "  Bankruptcy Court  ",
		 "%s has been declared bankrupt by the Interstellar Trading Bank.",
		 player[current_player].name);
    } else {
	txdlgbox(MAX_DLG_LINES, 50, 7, WCENTER, attr_error_window,
		 attr_error_title, attr_error_highlight, 0, 0,
		 attr_error_waitforkey, "  Bankruptcy Court  ",
		 "%s has declared bankruptcy.", player[current_player].name);
    }
    txrefresh();

    // Confiscate all assets belonging to player
    player[current_player].in_game = false;
    for (int i = 0; i < MAX_COMPANIES; i++) {
	company[i].stock_issued -= player[current_player].stock_owned[i];
	player[current_player].stock_owned[i] = 0;
    }
    player[current_player].cash = 0.0;
    player[current_player].debt = 0.0;

    // Is anyone still left in the game?
    bool all_out = true;
    for (int i = 0; i < number_players; i++) {
	if (player[i].in_game) {
	    all_out = false;
	    break;
	}
    }

    if (all_out) {
	quit_selected = true;
    }
}


/***********************************************************************/
// try_start_new_company: See it a new company can be started

void try_start_new_company (int x, int y)
{
    bool all_on_map;
    map_val_t left, right, up, down;
    int i, j;


    assert(x >= 0 && x < MAX_X);
    assert(y >= 0 && y < MAX_Y);

    assign_vals(x, y, left, right, up, down);

    if (   left  != MAP_OUTPOST && left  != MAP_STAR
	&& right != MAP_OUTPOST && right != MAP_STAR
	&& up    != MAP_OUTPOST && up    != MAP_STAR
	&& down  != MAP_OUTPOST && down  != MAP_STAR) {
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

	txdlgbox(MAX_DLG_LINES, 50, 7, WCENTER, attr_normal_window,
		 attr_title, attr_normal, attr_highlight, 0, attr_waitforkey,
		 "  New Company  ", "A new company has been formed!\n"
		 "Its name is ^{%s^}.", company[i].name);
	txrefresh();

	galaxy_map[x][y] = COMPANY_TO_MAP(i);

	company[i].share_price  = INITIAL_SHARE_PRICE;
	company[i].share_return = INITIAL_RETURN;
	company[i].stock_issued = INITIAL_STOCK_ISSUED;
	company[i].max_stock    = INITIAL_MAX_STOCK;
	company[i].on_map       = true;

	for (j = 0; j < number_players; j++) {
	    player[j].stock_owned[i] = 0;
	}

	player[current_player].stock_owned[i] = INITIAL_STOCK_ISSUED;
    }
}


/***********************************************************************/
// merge_companies: Merge two companies together

void merge_companies (map_val_t a, map_val_t b)
{
    int aa = MAP_TO_COMPANY(a);
    int bb = MAP_TO_COMPANY(b);

    double val_aa = company[aa].share_price * company[aa].stock_issued *
	company[aa].share_return;
    double val_bb = company[bb].share_price * company[bb].stock_issued *
	company[bb].share_return;

    long int old_stock, new_stock, total_new;
    int x, y, i, line;
    double bonus;
    char *buf = xmalloc(BUFSIZE);


    if (val_aa < val_bb) {
	// Make sure aa is the dominant company
	map_val_t t;
	int tt;

	t  = a;  a  = b;  b  = t;
	tt = aa; aa = bb; bb = tt;
    }

    // Display information about the merger

    newtxwin(number_players + 14, WIN_COLS - 4, 9 - number_players,
	     WCENTER, true, attr_normal_window);

    old_center(curwin, 1, attr_title, "  Company Merger  ");
    old_center3(curwin, 3, attr_highlight, attr_highlight, attr_normal,
	    company[bb].name, company[aa].name, " has just merged into ");

    old_center(curwin, 5, attr_normal, "Please note the following transactions:");

    old_center2(curwin, 7, attr_normal, attr_highlight, " Old stock: ",
	    "%-20s", company[bb].name);
    old_center2(curwin, 8, attr_normal, attr_highlight, " New stock: ",
	    "%-20s", company[aa].name);

    // Handle the locale's currency symbol
    snprintf(buf, BUFSIZE, "Bonus (%s)", lconvinfo.currency_symbol);

    int w = getmaxx(curwin) - 52;
    wattrset(curwin, attr_subtitle);
    mvwprintw(curwin, 10, 2, "  %-*.*s  %8s  %8s  %8s  %12s  ", w, w,
	      "Player", "Old", "New", "Total", buf);
    wattrset(curwin, attr_normal);

    total_new = 0;
    for (line = 11, i = 0; i < number_players; i++) {
	if (player[i].in_game) {
	    // Calculate new stock and any bonus
	    old_stock = player[i].stock_owned[bb];
	    new_stock = (double) old_stock * MERGE_STOCK_RATIO;
	    total_new += new_stock;

	    bonus = (company[bb].stock_issued == 0) ? 0.0 :
		10.0 * ((double) player[i].stock_owned[bb]
			/ company[bb].stock_issued) * company[bb].share_price;

	    player[i].stock_owned[aa] += new_stock;
	    player[i].stock_owned[bb] = 0;
	    player[i].cash += bonus;

	    l_strfmon(buf, BUFSIZE, "%!12n", bonus);
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

    wait_for_key(curwin, getmaxy(curwin) - 2, attr_waitforkey);

    deltxwin();			// "Company merger" window
    txrefresh();

    free(buf);
}


/***********************************************************************/
// include_outpost: Include any outposts into the company

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

    // Include any nearby outposts
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


/***********************************************************************/
// inc_share_price: Increase the share price of a company

void inc_share_price (int num, double inc)
{
    assert(num >= 0 && num < MAX_COMPANIES);

    company[num].share_price += inc * (1.0 + randf() * SHARE_PRICE_INC_EXTRA);
    company[num].max_stock   += inc / (randf() * 10.0 + 5.0);

    if (randf() < GROWING_RETURN_CHANGE) {
	company[num].share_return *= randf() + GROWING_RETURN_INC;
    }
}


/***********************************************************************/
// adjust_values: Adjust various company-related values

void adjust_values (void)
{
    int i, x, y;
    int which;


    // Declare a company bankrupt!
    if (randf() > (1.0 - COMPANY_BANKRUPTCY)) {
	which = randi(MAX_COMPANIES);

	if (company[which].on_map) {
	    if (randf() < ALL_ASSETS_TAKEN) {
		txdlgbox(MAX_DLG_LINES, 60, 6, WCENTER, attr_error_window,
			 attr_error_title, attr_error_highlight,
			 attr_error_normal, 0, attr_error_waitforkey,
			 "  Bankruptcy Court  ",
			 "%s has been declared bankrupt "
			 "by the Interstellar Trading Bank.\n\n"
			 "^{All assets have been taken "
			 "to repay outstanding loans.^}",
			 company[which].name);
		txrefresh();

	    } else {
		double rate = randf();
		char *buf = xmalloc(BUFSIZE);

		for (i = 0; i < number_players; i++) {
		    if (player[i].in_game) {
			player[i].cash += player[i].stock_owned[which] * rate;
		    }
		}

		newtxwin(14, 60, 4, WCENTER, true, attr_error_window);

		old_center(curwin, 1, attr_error_title, "  Bankruptcy Court  ");
		old_center(curwin, 3, attr_error_highlight, "%s has been declared",
			company[which].name);
		old_center(curwin, 4, attr_error_highlight,
		       "bankrupt by the Interstellar Trading Bank.");

		old_center2(curwin, 6, attr_error_normal, attr_error_highlight,
			"The Bank has agreed to pay stock holders ",
			"%4.2f%%", rate * 100.0);
		old_center(curwin, 7, attr_error_normal,
		       "of the share value on each share owned.");

		l_strfmon(buf, BUFSIZE, "%12n", company[which].share_price);
		old_center2(curwin, 9, attr_error_normal, attr_error_highlight,
			"Old share value:       ", "%s", buf);

		l_strfmon(buf, BUFSIZE, "%12n", company[which].share_price
			  * rate);
		old_center2(curwin, 10, attr_error_normal, attr_error_highlight,
			"Amount paid per share: ", "%s", buf);

		wait_for_key(curwin, 12, attr_error_waitforkey);
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
		    if (galaxy_map[x][y] == COMPANY_TO_MAP((unsigned int) which)) {
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
	    double change = randf() * company[which].share_price
		* PRICE_CHANGE_RATE;
	    if (randf() < DEC_SHARE_PRICE) {
		change = -change;
	    }
	    company[which].share_price += change;
	}
    }

    // Give the current player the companies' dividends
    for (i = 0; i < MAX_COMPANIES; i++) {
	if (company[i].on_map && company[i].stock_issued != 0) {
	    player[current_player].cash += player[current_player].stock_owned[i]
		* company[i].share_price * company[i].share_return
		+ ((double) player[current_player].stock_owned[i]
		   / company[i].stock_issued) * company[i].share_price
		* OWNERSHIP_BONUS;
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
	double impounded = MIN(player[current_player].cash,
			       player[current_player].debt);

	txdlgbox(MAX_DLG_LINES, 60, 7, WCENTER, attr_error_window,
		 attr_error_title, attr_error_highlight, attr_error_normal,
		 0, attr_error_waitforkey, "  Interstellar Trading Bank  ",
		 "Your debt has amounted to %N!\n"
		 "^{The Bank has impounded ^}%N^{ from your cash.^}",
		 player[current_player].debt, impounded);
	txrefresh();

	player[current_player].cash -= impounded;
	player[current_player].debt -= impounded;
	if (player[current_player].cash < ROUNDING_AMOUNT) {
	    player[current_player].cash = 0.0;
	}
	if (player[current_player].debt < ROUNDING_AMOUNT) {
	    player[current_player].debt = 0.0;
	}

	// Shall we declare them bankrupt?
	if (total_value(current_player) <= 0.0 && randf() < PROB_BANKRUPTCY) {
	    bankrupt_player(true);
	}
    }
}


/***********************************************************************/
// cmp_game_move: Compare two game_move[] elements for sorting

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


/***********************************************************************/
// End of file
