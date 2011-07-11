/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, game.c, contains the implementation of the game functions
  used in Star Traders.


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
*                       Game function definitions                       *
************************************************************************/

/*-----------------------------------------------------------------------
  Function:   init_game  - Initialise a new game or load an old one
  Arguments:  (none)
  Returns:    (nothing)

  This function initialises all game variables and structures, either by
  creating a new game or by loading an old one from disk.  In particular,
  if a new game is to be created, it asks how many people will play, and
  what their names are.  If needed, instructions on how to play the game
  are also displayed.

  On entry to this function, the "game_num" global variable determines
  whether an old game is loaded (if possible).  On exit, all global
  variables in globals.h are initialised, apart from game_move[].  If the
  user aborts entering the necessary information, quit_selected is set to
  true and number_players is 0.
*/

void init_game (void)
{
    int i, j, x, y;
    int key, ret;
    bool done, modified, entered[MAX_PLAYERS];


    // Try to load an old game, if possible
    if (game_num != 0) {
	newtxwin(5, 30, LINE_OFFSET + 6, COL_CENTER(30));
	wbkgd(curwin, ATTR_STATUS_WINDOW);
	box(curwin, 0, 0);
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

	    // Ask for the number of players
	    newtxwin(5, 62, LINE_OFFSET + 3, COL_CENTER(62));
	    wbkgd(curwin, ATTR_NORMAL_WINDOW);
	    box(curwin, 0, 0);

	    mvwaddstr(curwin, 2, 2, "Enter number of players ");
	    waddstr(curwin, "[");
	    attrpr(curwin, ATTR_KEYCODE_STR, "1");
	    waddstr(curwin, "-");
	    attrpr(curwin, ATTR_KEYCODE_STR, "%d", MAX_PLAYERS);
	    waddstr(curwin, "]");
	    waddstr(curwin, " or ");
	    attrpr(curwin, ATTR_KEYCODE_STR, "<C>");
	    waddstr(curwin, " to continue a game: ");

	    curs_set(CURS_ON);
	    wrefresh(curwin);

	    do {
		key = toupper(gettxchar(curwin));
		done = ((key >= '1') && (key <= (MAX_PLAYERS + '0'))) || (key == 'C');

		if ((key == KEY_ESC) || (key == KEY_CTRL('C')) || (key == KEY_CTRL('\\'))) {
		    quit_selected = true;
		    return;
		}

		if (! done) {
		    beep();
		}
	    } while (! done);

	    curs_set(CURS_OFF);
	    wechochar(curwin, key | A_BOLD);

	    if (key != 'C') {
		number_players = key - '0';
	    } else {

		// Ask which game to load
		newtxwin(5, 49, LINE_OFFSET + 6, COL_CENTER(49));
		wbkgd(curwin, ATTR_NORMAL_WINDOW);
		box(curwin, 0, 0);

		mvwaddstr(curwin, 2, 2, "Enter game number ");
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
		    key = toupper(gettxchar(curwin));
		    done = ((key >= '1') && (key <= '9')) || (key == KEY_ESC);

		    if (! done) {
			beep();
		    }
		} while (! done);

		curs_set(CURS_OFF);

		if (key != KEY_ESC) {
		    game_num = key - '0';

		    wechochar(curwin, key | A_BOLD);

		    // Try to load the game, if possible
		    newtxwin(5, 30, LINE_OFFSET + 9, COL_CENTER(30));
		    wbkgd(curwin, ATTR_STATUS_WINDOW);
		    box(curwin, 0, 0);
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
	    }
	}

	if (! game_loaded) {
	    if (number_players == 1) {
		// Ask for the player name

		newtxwin(5, 76, LINE_OFFSET + 9, COL_CENTER(76));
		wbkgd(curwin, ATTR_NORMAL_WINDOW);
		box(curwin, 0, 0);

		mvwaddstr(curwin, 2, 2, "Please enter your name: ");

		player[0].name = NULL;
		do {
		    ret = gettxstring(curwin, &player[0].name, false, 2, 26,
				      48, ATTR_INPUT_FIELD, NULL);
		    done = ((ret == OK) && (strlen(player[0].name) != 0));

		    if (! done) {
			beep();
		    }
		} while (! done);

		newtxwin(5, 44, LINE_OFFSET + 6, COL_CENTER(44));
		wbkgd(curwin, ATTR_NORMAL_WINDOW);
		box(curwin, 0, 0);

		mvwaddstr(curwin, 2, 2, "Do you need any instructions? ");
		waddstr(curwin, "[");
		attrpr(curwin, ATTR_KEYCODE_STR, "Y");
		waddstr(curwin, "/");
		attrpr(curwin, ATTR_KEYCODE_STR, "N");
		waddstr(curwin, "] ");

		if (answer_yesno(curwin)) {
		    show_help();
		}

		deltxwin();		// "Do you need instructions?" window
		deltxwin();		// "Enter your name" window
		deltxwin();		// "Number of players" window
		txrefresh();
	    } else {

		// Ask for all of the player names
		newtxwin(number_players + 5, 76, LINE_OFFSET + 9, COL_CENTER(76));
		wbkgd(curwin, ATTR_NORMAL_WINDOW);
		box(curwin, 0, 0);

		center(curwin, 1, ATTR_WINDOW_TITLE, "  Enter player names  ");

		for (i = 0; i < number_players; i++) {
		    player[i].name = NULL;
		    entered[i] = false;
		    mvwprintw(curwin, i + 3, 2, "Player %d:", i + 1);
		}

		i = 0;
		done = false;
		while (! done) {
		    ret = gettxstring(curwin, &player[i].name, true, 3 + i, 12,
				      62, ATTR_INPUT_FIELD, &modified);

		    switch (ret) {
		    case OK:
			// Make sure name is not an empty string
			j = strlen(player[i].name);
			entered[i] = (j != 0);
			if (j == 0) {
			    beep();
			}

			// Make sure name has not been entered already
			for (j = 0; j < number_players; j++) {
			    if ((i != j) && (player[j].name != NULL) &&
				(strcmp(player[i].name, player[j].name) == 0)) {
				entered[i] = false;
				beep();
				break;
			    }
			}

			// Move to first name for which ENTER has not been pressed
			done = true;
			for (i = 0; i < number_players; i++) {
			    if (! entered[i]) {
				done = false;
				break;
			    }
			}
			break;

		    case ERR:
			beep();
			break;

		    case KEY_UP:
			if (modified) {
			    entered[i] = false;
			}

			if (i == 0) {
			    i = number_players - 1;
			} else {
			    i--;
			}
			break;

		    case KEY_DOWN:
			if (modified) {
			    entered[i] = false;
			}

			if (i == number_players - 1) {
			    i = 0;
			} else {
			    i++;
			}
			break;

		    default:
			beep();
			break;
		    }
		}

		newtxwin(5, 50, LINE_OFFSET + 6, COL_CENTER(50));
		wbkgd(curwin, ATTR_NORMAL_WINDOW);
		box(curwin, 0, 0);

		mvwaddstr(curwin, 2, 2, "Does any player need instructions? ");
		waddstr(curwin, "[");
		attrpr(curwin, ATTR_KEYCODE_STR, "Y");
		waddstr(curwin, "/");
		attrpr(curwin, ATTR_KEYCODE_STR, "N");
		waddstr(curwin, "] ");

		if (answer_yesno(curwin)) {
		    show_help();
		}

		deltxwin();		// "Need instructions?" window
		deltxwin();		// "Enter player names" window
		deltxwin();		// "Number of players" window
		txrefresh();
	    }

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
	    max_turn = DEFAULT_MAX_TURN;
	    turn_number = 1;

	    // Select who is to go first
	    if (number_players == 1) {
		first_player   = 0;
		current_player = 0;
	    } else {
		first_player   = randi(number_players);
		current_player = first_player;

		newtxwin(7, 50, LINE_OFFSET + 8, COL_CENTER(50));
		wbkgd(curwin, ATTR_NORMAL_WINDOW);
		box(curwin, 0, 0);

		center(curwin, 2, ATTR_NORMAL_WINDOW,
		       "The first player to go is");
		center(curwin, 3, ATTR_HIGHLIGHT_STR, "%1.46s",
		       player[first_player].name);

		wait_for_key(curwin, 5, ATTR_WAITNORMAL_STR);
		deltxwin();
		txrefresh();
	    }
	}
    }

    quit_selected = false;
}


/*-----------------------------------------------------------------------
  Function:   end_game  - Finish playing the current game
  Arguments:  (none)
  Returns:    (nothing)

  This function displays every player's status before declaring the
  winner of the game.
*/

void end_game (void)
{
    // @@@ To be written
}


/*-----------------------------------------------------------------------
  Function:   load_game  - Load a saved game from disk
  Arguments:  num        - Game number to load (1-9)
  Returns:    bool       - True if game loaded successfully, else false

  This function loads a previously-saved game from disk.  True is
  returned if this could be done successfully.
*/

bool load_game (int num)
{
    assert((num >= 1) && (num <= 9));

    // @@@ To be written
    return false;
}


/*-----------------------------------------------------------------------
  Function:   save_game  - Save the current game to disk
  Arguments:  num        - Game number to use (1-9)
  Returns:    bool       - True if game saved successfully, else false

  This function saves the current game to disk.  True is returned if this
  could be done successfully.
*/

bool save_game (int num)
{
    assert((num >= 1) && (num <= 9));

    // @@@ To be written
    return true;
}


void select_moves (void)
{
    // @@@ To be written
}

void get_move (void)
{
    // @@@ To be written
}

void process_move (void)
{
    // @@@ To be written
}

void exchange_stock (void)
{
    // @@@ To be written
}

void next_player (void)
{
    // @@@ To be written
    quit_selected = true;
}
