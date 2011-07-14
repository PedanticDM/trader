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
*                      Module constants and macros                      *
************************************************************************/

// Game loading and saving constants

#define GAME_BUFSIZE	(1024)

static const int game_file_crypt_key[] = {
    0x50, 0x52, 0x55, 0x59, 0x5A, 0x5C, 0x5F,
    0x90, 0x92, 0x95, 0x99, 0x9A, 0x9C, 0x9F,
    0xA0, 0xA2, 0xA5, 0xA9, 0xAA, 0xAC, 0xAF,
    0xD0, 0xD2, 0xD5, 0xD9, 0xDA, 0xDC, 0xDF
};

#define GAME_FILE_CRYPT_KEY_SIZE (sizeof(game_file_crypt_key) / sizeof(int))


// Macros used in load_game()

#define load_game_scanf(_fmt, _var, _cond)				\
    {									\
	if (fgets(buf, GAME_BUFSIZE, file) == NULL) {			\
	    err_exit("%s: missing field on line %d", filename, lineno);	\
	}								\
	if (sscanf(unscramble(crypt_key, buf, GAME_BUFSIZE), _fmt "\n",	\
		   &(_var)) != 1) {					\
	    err_exit("%s: illegal field on line %d", filename, lineno);	\
	}								\
	if (! (_cond)) {						\
	    err_exit("%s: illegal value on line %d", filename, lineno);	\
	}								\
	lineno++;							\
    }

#define load_game_read_int(_var, _cond)					\
    load_game_scanf("%d", _var, _cond)
#define load_game_read_long(_var, _cond)				\
    load_game_scanf("%ld", _var, _cond)
#define load_game_read_double(_var, _cond)				\
    load_game_scanf("%lf", _var, _cond)

#define load_game_read_bool(_var)					\
    {									\
	int b;								\
									\
	load_game_scanf("%d", b, (b == false) || (b == true));		\
	(_var) = b;							\
    }

#define load_game_read_string(_var)					\
    {									\
	char *s;							\
	int len;							\
									\
	if (fgets(buf, GAME_BUFSIZE, file) == NULL) {			\
	    err_exit("%s: missing field on line %d", filename, lineno);	\
	}								\
	if (strlen(unscramble(crypt_key, buf, GAME_BUFSIZE)) == 0) {	\
	    err_exit("%s: illegal value on line %d", filename, lineno);	\
	}								\
	lineno++;							\
									\
	s = malloc(strlen(buf) + 1);					\
	if (s == NULL) {						\
	    err_exit("out of memory");					\
	}								\
									\
	strcpy(s, buf);							\
	len = strlen(s);						\
	if ((len > 0) && (s[len - 1] == '\n')) {			\
	    s[len - 1] = '\0';						\
	}								\
									\
	(_var) = s;							\
    }


// Macros used in save_game()

#define save_game_printf(_fmt, _var)					\
    {									\
	snprintf(buf, GAME_BUFSIZE, _fmt "\n", _var);			\
	scramble(crypt_key, buf, GAME_BUFSIZE);				\
	fprintf(file, "%s", buf);					\
    }

#define save_game_write_int(_var)					\
    save_game_printf("%d", _var)
#define save_game_write_long(_var)					\
    save_game_printf("%ld", _var)
#define save_game_write_double(_var)					\
    save_game_printf("%2.20e", _var)
#define save_game_write_bool(_var)					\
    save_game_printf("%d", (int) _var)
#define save_game_write_string(_var)					\
    save_game_printf("%s", _var)


/************************************************************************
*                    Internal function declarations                     *
************************************************************************/

int cmp_game_move (const void *a, const void *b);


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
  user aborts entering the necessary information, abort_game is set to
  true.
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
		    abort_game = true;
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
		newtxwin(5, 50, LINE_OFFSET + 6, COL_CENTER(50));
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
    abort_game = false;
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
    if (abort_game) {
	// init_game() was cancelled by user
	return;
    }

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
    char *buf, *filename;
    FILE *file;
    int saved_errno, lineno;

    int crypt_key;
    int n, i, j, x, y;
    char c;


    assert((num >= 1) && (num <= 9));

    buf = malloc(GAME_BUFSIZE);
    if (buf == NULL) {
	err_exit("out of memory");
    }

    filename = game_filename(num);
    assert(filename != NULL);

    file = fopen(filename, "r");
    if (file == NULL) {
	// File could not be opened

	if (errno == ENOENT) {
	    // File not found
	    newtxwin(7, 40, LINE_OFFSET + 9, COL_CENTER(40));
	    wbkgd(curwin, ATTR_ERROR_WINDOW);
	    box(curwin, 0, 0);

	    center(curwin, 1, ATTR_ERROR_TITLE, "  Game not found  ");
	    center(curwin, 3, ATTR_ERROR_STR,
		   "Game %d has not been saved to disk", num);

	    wait_for_key(curwin, 5, ATTR_WAITERROR_STR);
	    deltxwin();
	} else {
	    // Some other file error
	    saved_errno = errno;

	    newtxwin(9, 70, LINE_OFFSET + 9, COL_CENTER(70));
	    wbkgd(curwin, ATTR_ERROR_WINDOW);
	    box(curwin, 0, 0);

	    center(curwin, 1, ATTR_ERROR_TITLE, "  Game not loaded  ");
	    center(curwin, 3, ATTR_ERROR_STR,
		   "Game %d could not be loaded from disk", num);
	    center(curwin, 5, ATTR_ERROR_WINDOW, "File %s: %s", filename,
		   strerror(saved_errno));

	    wait_for_key(curwin, 7, ATTR_WAITERROR_STR);
	    deltxwin();
	}

	free(buf);
	free(filename);
	return false;
    }

    // Read the game file header
    if (fgets(buf, GAME_BUFSIZE, file) == NULL) {
	err_exit("%s: missing header in game file", filename);
    }
    if (strcmp(buf, GAME_FILE_HEADER "\n") != 0) {
	err_exit("%s: not a valid game file", filename);
    }
    if (fgets(buf, GAME_BUFSIZE, file) == NULL) {
	err_exit("%s: missing subheader in game file", filename);
    }
    if (strcmp(buf, GAME_FILE_API_VERSION "\n") != 0) {
	err_exit("%s: saved under a different version of Star Traders",
		 filename);
    }

    lineno = 3;

    // Read in the game file encryption key
    if (fscanf(file, "%i\n", &crypt_key) != 1) {
	err_exit("%s: illegal or missing field on line %d", filename, lineno);
    }
    lineno++;

    // Read in various game variables
    load_game_read_int(n,                n == MAX_X);
    load_game_read_int(n,                n == MAX_Y);
    load_game_read_int(max_turn,         max_turn >= 1);
    load_game_read_int(turn_number,      (turn_number >= 1) && (turn_number <= max_turn));
    load_game_read_int(number_players,   (number_players >= 1) && (number_players < MAX_PLAYERS));
    load_game_read_int(current_player,   (current_player >= 0) && (current_player < number_players));
    load_game_read_int(first_player,     (first_player >= 0) && (first_player < number_players));
    load_game_read_int(n,                n == MAX_COMPANIES);
    load_game_read_double(interest_rate, interest_rate > 0.0);

    // Read in player data
    for (i = 0; i < number_players; i++) {
	load_game_read_string(player[i].name);
	load_game_read_double(player[i].cash, player[i].cash >= 0.0);
	load_game_read_double(player[i].debt, player[i].debt >= 0.0);
	load_game_read_bool(player[i].in_game);

	for (j = 0; j < MAX_COMPANIES; j++) {
	    load_game_read_long(player[i].stock_owned[j], player[i].stock_owned[j] >= 0);
	}
    }

    // Read in company data
    for (i = 0; i < MAX_COMPANIES; i++) {
	company[i].name = company_name[i];
	load_game_read_double(company[i].share_price, company[i].share_price > 0.0);
	load_game_read_double(company[i].share_return, true);
	load_game_read_long(company[i].stock_issued, company[i].stock_issued >= 0);
	load_game_read_long(company[i].max_stock, company[i].max_stock >= 0);
	load_game_read_bool(company[i].on_map);
    }

    // Read in galaxy map
    for (x = 0; x < MAX_X; x++) {
	if (fgets(buf, GAME_BUFSIZE, file) == NULL) {
	    err_exit("%s: missing field on line %d", filename, lineno);
	}
	if (strlen(unscramble(crypt_key, buf, GAME_BUFSIZE)) != (MAX_Y + 1)) {
	    err_exit("%s: illegal field on line %d", filename, lineno);
	}
	lineno++;

	for (y = 0; y < MAX_Y; y++) {
	    c = buf[y];
	    if ((c == MAP_EMPTY) || (c == MAP_OUTPOST) || (c == MAP_STAR) ||
		((c >= MAP_A) && (c <= MAP_LAST))) {
		galaxy_map[x][y] = (map_val_t) c;
	    } else {
		err_exit("%s: illegal value on line %d", filename, lineno - 1);
	    }
	}
    }

    // Read in a dummy sentinal value
    load_game_read_int(n, n == GAME_FILE_SENTINEL);

    if (fclose(file) == EOF) {
	errno_exit("%s", filename);
    }

    free(buf);
    free(filename);
    return true;
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
    const char *data_dir;
    char *buf, *filename;
    FILE *file;
    int saved_errno;
    struct stat statbuf;
    int crypt_key;
    int i, j, x, y;
    char *p;


    assert((num >= 1) && (num <= 9));

    buf = malloc(GAME_BUFSIZE);
    if (buf == NULL) {
	err_exit("out of memory");
    }

    crypt_key = game_file_crypt_key[randi(GAME_FILE_CRYPT_KEY_SIZE)];

    // Create the data directory, if needed
    data_dir = data_directory();
    if (data_dir != NULL) {
	if (mkdir(data_dir, S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
	    saved_errno = errno;
	    if ((saved_errno == EEXIST) && (stat(data_dir, &statbuf) == 0)
		&& S_ISDIR(statbuf.st_mode)) {
		;	// Do nothing: directory already exists
	    } else {
		// Data directory could not be created

		newtxwin(9, 70, LINE_OFFSET + 9, COL_CENTER(70));
		wbkgd(curwin, ATTR_ERROR_WINDOW);
		box(curwin, 0, 0);

		center(curwin, 1, ATTR_ERROR_TITLE, "  Game not saved  ");
		center(curwin, 3, ATTR_ERROR_STR,
		       "Game %d could not be saved to disk", num);
		center(curwin, 5, ATTR_ERROR_WINDOW, "Directory %s: %s",
		       data_dir, strerror(saved_errno));

		wait_for_key(curwin, 7, ATTR_WAITERROR_STR);
		deltxwin();

		free(buf);
		return false;
	    }
	}
    }

    filename = game_filename(num);
    assert(filename != NULL);

    file = fopen(filename, "w");
    if (file == NULL) {
	// File could not be opened for writing
	saved_errno = errno;

	newtxwin(9, 70, LINE_OFFSET + 9, COL_CENTER(70));
	wbkgd(curwin, ATTR_ERROR_WINDOW);
	box(curwin, 0, 0);

	center(curwin, 1, ATTR_ERROR_TITLE, "  Game not saved  ");
	center(curwin, 3, ATTR_ERROR_STR,
	       "Game %d could not be saved to disk", num);
	center(curwin, 5, ATTR_ERROR_WINDOW, "File %s: %s", filename,
	       strerror(saved_errno));

	wait_for_key(curwin, 7, ATTR_WAITERROR_STR);
	deltxwin();

	free(buf);
	free(filename);
	return false;
    }

    // Write out the game file header and encryption key
    fprintf(file, "%s\n" "%s\n", GAME_FILE_HEADER, GAME_FILE_API_VERSION);
    fprintf(file, "%d\n", crypt_key);

    // Write out various game variables
    save_game_write_int(MAX_X);
    save_game_write_int(MAX_Y);
    save_game_write_int(max_turn);
    save_game_write_int(turn_number);
    save_game_write_int(number_players);
    save_game_write_int(current_player);
    save_game_write_int(first_player);
    save_game_write_int(MAX_COMPANIES);
    save_game_write_double(interest_rate);

    // Write out player data
    for (i = 0; i < number_players; i++) {
	save_game_write_string(player[i].name);
	save_game_write_double(player[i].cash);
	save_game_write_double(player[i].debt);
	save_game_write_bool(player[i].in_game);

	for (j = 0; j < MAX_COMPANIES; j++) {
	    save_game_write_long(player[i].stock_owned[j]);
	}
    }

    // Write out company data
    for (i = 0; i < MAX_COMPANIES; i++) {
	save_game_write_double(company[i].share_price);
	save_game_write_double(company[i].share_return);
	save_game_write_long(company[i].stock_issued);
	save_game_write_long(company[i].max_stock);
	save_game_write_bool(company[i].on_map);
    }

    // Write out galaxy map
    for (x = 0; x < MAX_X; x++) {
	memset(buf, 0, MAX_Y + 2);
	for (p = buf, y = 0; y < MAX_Y; p++, y++) {
	    *p = (char) galaxy_map[x][y];
	}
	*p++ = '\n';
	*p = '\0';

	scramble(crypt_key, buf, GAME_BUFSIZE);
	fprintf(file, "%s", buf);
    }

    // Write out a dummy sentinal value
    save_game_write_int(GAME_FILE_SENTINEL);

    if (fclose(file) == EOF) {
	errno_exit("%s", filename);
    }

    free(buf);
    free(filename);
    return true;
}


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


/*-----------------------------------------------------------------------
  Function:   cmp_game_move  - Compare two game_move elements
  Arguments:  a, b           - Elements to compare
  Returns:    int            - Comparison of a and b

  This function compares two game_move elements (of type move_rec_t) and
  returns -1 if a < b, 0 if a == b and 1 if a > b.
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
