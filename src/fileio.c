/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, fileio.c, contains the implementation of the game load and
  save functions used in Star Traders.


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
*                 Module-specific constants and macros                  *
************************************************************************/

// Game loading and saving constants

static const unsigned char game_file_crypt_key[] = {
    0x50, 0x52, 0x55, 0x59, 0x5A, 0x5C, 0x5F,
    0x90, 0x92, 0x95, 0x99, 0x9A, 0x9C, 0x9F,
    0xA0, 0xA2, 0xA5, 0xA9, 0xAA, 0xAC, 0xAF,
    0xD0, 0xD2, 0xD5, 0xD9, 0xDA, 0xDC, 0xDF
};

#define GAME_FILE_CRYPT_KEY_SIZE (sizeof(game_file_crypt_key) / sizeof(game_file_crypt_key[0]))


// Macros used in load_game()

#define load_game_scanf(_fmt, _var, _cond)				\
    do {								\
	if (fgets(buf, BUFSIZE, file) == NULL) {			\
	    err_exit("%s: missing field on line %d", filename, lineno);	\
	}								\
	if (sscanf(unscramble(crypt_key, buf, BUFSIZE), _fmt "\n",	\
		   &(_var)) != 1) {					\
	    err_exit("%s: illegal field on line %d: `%s'",		\
		     filename, lineno, buf);				\
	}								\
	if (! (_cond)) {						\
	    err_exit("%s: illegal value on line %d: `%s'",		\
		     filename, lineno, buf);				\
	}								\
	lineno++;							\
    } while (0)

#define load_game_read_int(_var, _cond)					\
    load_game_scanf("%d", _var, _cond)
#define load_game_read_long(_var, _cond)				\
    load_game_scanf("%ld", _var, _cond)
#define load_game_read_double(_var, _cond)				\
    load_game_scanf("%lf", _var, _cond)

#define load_game_read_bool(_var)					\
    do {								\
	int b;								\
									\
	load_game_scanf("%d", b, b == false || b == true);		\
	(_var) = b;							\
    } while (0)

#define load_game_read_string(_var)					\
    do {								\
	char *s;							\
	int len;							\
									\
	if (fgets(buf, BUFSIZE, file) == NULL) {			\
	    err_exit("%s: missing field on line %d", filename, lineno);	\
	}								\
	if (strlen(unscramble(crypt_key, buf, BUFSIZE)) == 0) {		\
	    err_exit("%s: illegal value on line %d", filename, lineno);	\
	}								\
	lineno++;							\
									\
	s = malloc(strlen(buf) + 1);					\
	if (s == NULL) {						\
	    err_exit_nomem();						\
	}								\
									\
	strcpy(s, buf);							\
	len = strlen(s);						\
	if (len > 0 && s[len - 1] == '\n') {				\
	    s[len - 1] = '\0';						\
	}								\
									\
	(_var) = s;							\
    } while (0)


// Macros used in save_game()

#define save_game_printf(_fmt, _var)					\
    do {								\
	snprintf(buf, BUFSIZE, _fmt "\n", _var);			\
	scramble(crypt_key, buf, BUFSIZE);				\
	fprintf(file, "%s", buf);					\
    } while (0)

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
*                Game load and save function definitions                *
************************************************************************/

// These functions are documented in the file "fileio.h"


/***********************************************************************/
// load_game: Load a previously-saved game from disk

bool load_game (int num)
{
    char *buf, *filename;
    FILE *file;
    int saved_errno, lineno;
    char *prev_locale;

    int crypt_key;
    int n, i, j;


    assert(num >= 1 && num <= 9);

    buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit_nomem();
    }

    filename = game_filename(num);
    assert(filename != NULL);

    file = fopen(filename, "r");
    if (file == NULL) {
	// File could not be opened

	if (errno == ENOENT) {
	    // File not found
	    newtxwin(7, 40, 9, WCENTER, true, attr_error_window);

	    center(curwin, 1, attr_error_title, "  Game Not Found  ");
	    center(curwin, 3, attr_error_highlight,
		   "Game %d has not been saved to disk", num);

	    wait_for_key(curwin, 5, attr_error_waitforkey);
	    deltxwin();
	} else {
	    // Some other file error
	    saved_errno = errno;

	    newtxwin(9, 70, 9, WCENTER, true, attr_error_window);

	    center(curwin, 1, attr_error_title, "  Game Not Loaded  ");
	    center(curwin, 3, attr_error_highlight,
		   "Game %d could not be loaded from disk", num);
	    center(curwin, 5, attr_error_normal, "File %s: %s", filename,
		   strerror(saved_errno));

	    wait_for_key(curwin, 7, attr_error_waitforkey);
	    deltxwin();
	}

	free(buf);
	free(filename);
	return false;
    }

    // Change the formatting of numbers to the POSIX locale for consistency
    prev_locale = strdup(setlocale(LC_NUMERIC, NULL));
    if (prev_locale == NULL) {
	err_exit_nomem();
    }
    setlocale(LC_NUMERIC, "C");

    // Read the game file header
    if (fgets(buf, BUFSIZE, file) == NULL) {
	err_exit("%s: missing header in game file", filename);
    }
    if (strcmp(buf, GAME_FILE_HEADER "\n") != 0) {
	err_exit("%s: not a valid game file", filename);
    }
    if (fgets(buf, BUFSIZE, file) == NULL) {
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
    load_game_read_int(turn_number,      turn_number >= 1 && turn_number <= max_turn);
    load_game_read_int(number_players,   number_players >= 1 && number_players < MAX_PLAYERS);
    load_game_read_int(current_player,   current_player >= 0 && current_player < number_players);
    load_game_read_int(first_player,     first_player >= 0 && first_player < number_players);
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
	load_game_read_double(company[i].share_price,  company[i].share_price >= 0.0);
	load_game_read_double(company[i].share_return, true);
	load_game_read_long(company[i].stock_issued,   company[i].stock_issued >= 0);
	load_game_read_long(company[i].max_stock,      company[i].max_stock >= 0);
	load_game_read_bool(company[i].on_map);
    }

    // Read in galaxy map
    for (int x = 0; x < MAX_X; x++) {
	if (fgets(buf, BUFSIZE, file) == NULL) {
	    err_exit("%s: missing field on line %d", filename, lineno);
	}
	if (strlen(unscramble(crypt_key, buf, BUFSIZE)) != MAX_Y + 1) {
	    err_exit("%s: illegal field on line %d", filename, lineno);
	}

	for (int y = 0; y < MAX_Y; y++) {
	    char c = buf[y];
	    if (c == MAP_EMPTY || c == MAP_OUTPOST || c == MAP_STAR
		|| (c >= MAP_A && c <= MAP_LAST)) {
		galaxy_map[x][y] = (map_val_t) c;
	    } else {
		err_exit("%s: illegal value on line %d", filename, lineno);
	    }
	}

	lineno++;
    }

    // Read in a dummy sentinal value
    load_game_read_int(n, n == GAME_FILE_SENTINEL);

    if (fclose(file) == EOF) {
	errno_exit("%s", filename);
    }

    // Change the formatting of numbers back to the user-supplied locale
    setlocale(LC_NUMERIC, prev_locale);

    free(buf);
    free(filename);
    free(prev_locale);
    return true;
}


/***********************************************************************/
// save_game: Save the current game to disk

bool save_game (int num)
{
    const char *data_dir;
    char *buf, *filename;
    FILE *file;
    int saved_errno;
    char *prev_locale;
    struct stat statbuf;
    int crypt_key;
    int i, j, x, y;


    assert(num >= 1 && num <= 9);

    buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit_nomem();
    }

    crypt_key = option_dont_encrypt ? 0 :
	game_file_crypt_key[randi(GAME_FILE_CRYPT_KEY_SIZE)];

    // Create the data directory, if needed
    data_dir = data_directory();
    if (data_dir != NULL) {
	if (mkdir(data_dir, S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
	    saved_errno = errno;
	    if (saved_errno == EEXIST && stat(data_dir, &statbuf) == 0
		&& S_ISDIR(statbuf.st_mode)) {
		;	// Do nothing: directory already exists
	    } else {
		// Data directory could not be created

		newtxwin(9, 70, 7, WCENTER, true, attr_error_window);

		center(curwin, 1, attr_error_title, "  Game Not Saved  ");
		center(curwin, 3, attr_error_highlight,
		       "Game %d could not be saved to disk", num);
		center(curwin, 5, attr_error_normal, "Directory %s: %s",
		       data_dir, strerror(saved_errno));

		wait_for_key(curwin, 7, attr_error_waitforkey);
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

	newtxwin(9, 70, 7, WCENTER, true, attr_error_window);

	center(curwin, 1, attr_error_title, "  Game Not Saved  ");
	center(curwin, 3, attr_error_highlight,
	       "Game %d could not be saved to disk", num);
	center(curwin, 5, attr_error_normal, "File %s: %s", filename,
	       strerror(saved_errno));

	wait_for_key(curwin, 7, attr_error_waitforkey);
	deltxwin();

	free(buf);
	free(filename);
	return false;
    }

    // Change the formatting of numbers to the POSIX locale for consistency
    prev_locale = strdup(setlocale(LC_NUMERIC, NULL));
    if (prev_locale == NULL) {
	err_exit_nomem();
    }
    setlocale(LC_NUMERIC, "C");

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
	char *p;

	memset(buf, 0, MAX_Y + 2);
	for (p = buf, y = 0; y < MAX_Y; p++, y++) {
	    *p = (char) galaxy_map[x][y];
	}
	*p++ = '\n';
	*p = '\0';

	scramble(crypt_key, buf, BUFSIZE);
	fprintf(file, "%s", buf);
    }

    // Write out a dummy sentinal value
    save_game_write_int(GAME_FILE_SENTINEL);

    if (fclose(file) == EOF) {
	errno_exit("%s", filename);
    }

    // Change the formatting of numbers back to the user-supplied locale
    setlocale(LC_NUMERIC, prev_locale);

    free(buf);
    free(filename);
    free(prev_locale);
    return true;
}


/***********************************************************************/
// End of file
