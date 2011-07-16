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


/************************************************************************
*                    Internal function declarations                     *
************************************************************************/

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
  Function:   get_move  - Wait for the player to enter their move
  Arguments:  (none)
  Returns:    (nothing)

  This function displays the galaxy map and the current moves, then waits
  for the player to select one of the moves.  On entry, current_player
  points to the current player; quit_selected and/or abort_game may be
  true (if so, get_move() justs returns without waiting for the player to
  select a move).  On exit, selection contains the choice made by the
  player.  Note that two windows (the "Select move" window and the galaxy
  map window) are left on the screen: they are closed in process_move().
*/

void get_move (void)
{
    int i, x, y;


    if (quit_selected || abort_game) {
	selection = SEL_QUIT;
	return;
    } else {
	selection = SEL_NONE;
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
}


/*-----------------------------------------------------------------------
  Function:   process_move  - Process the move selected by the player
  Arguments:  (none)
  Returns:    (nothing)

  This function processes the move in the global variable selection.  It
  assumes the "Select move" and galaxy map windows are still open.
*/

void process_move (void)
{
    if (! quit_selected && ! abort_game) {
	switch(selection) {
	case SEL_QUIT:
	    quit_selected = true;
	    break;

	case SEL_BANKRUPT:
	    /* @@@ */
	    break;

	default:
	    assert(selection >= SEL_MOVE_FIRST && selection <= SEL_MOVE_LAST);
	    /* @@@ */
	    break;
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
