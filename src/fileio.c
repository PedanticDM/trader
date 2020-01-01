/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2020, John Zaitseff                 *
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
  along with this program.  If not, see https://www.gnu.org/licenses/.
*/


#include "trader.h"


/************************************************************************
*                        Module-specific macros                         *
************************************************************************/

// Macros used in load_game()

#define load_game_scanf(_fmt, _var, _cond)				\
    do {								\
	if (fgets(inbuf, BIGBUFSIZE, file) == NULL) {			\
	    err_exit(_("%s: missing field on line %d"),			\
		     filename, lineno);					\
	}								\
	if (unscramble(buf, inbuf, BUFSIZE, crypt_key_p) == NULL) {	\
	    err_exit(_("%s: illegal field on line %d"),			\
		     filename, lineno);					\
	}								\
	if (sscanf(buf, _fmt "\n", &(_var)) != 1) {			\
	    err_exit(_("%s: illegal field on line %d: '%s'"),		\
		     filename, lineno, buf);				\
	}								\
	if (! (_cond)) {						\
	    err_exit(_("%s: illegal value on line %d: '%s'"),		\
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

#ifdef USE_UTF8_GAME_FILE
#  define load_game_read_string(_var, _var_utf8)			\
    do {								\
	char *s;							\
	int len;							\
									\
	if (fgets(inbuf, BIGBUFSIZE, file) == NULL) {			\
	    err_exit(_("%s: missing field on line %d"),			\
		     filename, lineno);					\
	}								\
	if (unscramble(buf, inbuf, BUFSIZE, crypt_key_p) == NULL) {	\
	    err_exit(_("%s: illegal field on line %d"),			\
		     filename, lineno);					\
	}								\
	if (strlen(buf) == 0) {						\
	    err_exit(_("%s: illegal value on line %d"),			\
		     filename, lineno);					\
	}								\
	if (need_icd) {							\
	    s = str_cd_iconv(buf, icd);					\
	    if (s == NULL) {						\
		if (errno == EILSEQ) {					\
		    err_exit(_("%s: illegal characters on line %d"),	\
			     filename, lineno);				\
		} else {						\
		    errno_exit("str_cd_iconv");				\
		}							\
	    }								\
	} else {							\
	    s = xstrdup(buf);						\
	}								\
									\
	len = strlen(s);						\
	if (len > 0 && s[len - 1] == '\n') {				\
	    s[len - 1] = '\0';						\
	}								\
									\
	xmbstowcs(wcbuf, s, BUFSIZE);					\
	(_var) = xwcsdup(wcbuf);					\
	(_var_utf8) = s;						\
									\
	lineno++;							\
    } while (0)
#else // ! USE_UTF8_GAME_FILE
#  define load_game_read_string(_var, _var_utf8)			\
    do {								\
	char *s;							\
	int len;							\
									\
	if (fgets(inbuf, BIGBUFSIZE, file) == NULL) {			\
	    err_exit(_("%s: missing field on line %d"),			\
		     filename, lineno);					\
	}								\
	if (unscramble(buf, inbuf, BUFSIZE, crypt_key_p) == NULL) {	\
	    err_exit(_("%s: illegal field on line %d"),			\
		     filename, lineno);					\
	}								\
	if (strlen(buf) == 0) {						\
	    err_exit(_("%s: illegal value on line %d"),			\
		     filename, lineno);					\
	}								\
									\
	s = xstrdup(buf);						\
									\
	len = strlen(s);						\
	if (len > 0 && s[len - 1] == '\n') {				\
	    s[len - 1] = '\0';						\
	}								\
									\
	xmbstowcs(wcbuf, s, BUFSIZE);					\
	(_var) = xwcsdup(wcbuf);					\
	(_var_utf8) = s;						\
									\
	lineno++;							\
    } while (0)
#endif // ! USE_UTF8_GAME_FILE


// Macros used in save_game()

#define save_game_printf(_fmt, _var)					\
    do {								\
	snprintf(buf, BUFSIZE, _fmt "\n", _var);			\
	scramble(encbuf, buf, BIGBUFSIZE, crypt_key_p);			\
	fprintf(file, "%s", encbuf);					\
    } while (0)

#define save_game_write_int(_var)					\
    save_game_printf("%d", _var)
#define save_game_write_long(_var)					\
    save_game_printf("%ld", _var)
#define save_game_write_double(_var)					\
    save_game_printf("%2.20e", _var)
#define save_game_write_bool(_var)					\
    save_game_printf("%d", (int) _var)

#ifdef USE_UTF8_GAME_FILE
#  define save_game_write_string(_var, _var_utf8)			\
    do {								\
	if ((_var_utf8) != NULL) {					\
	    save_game_printf("%s", _var_utf8);				\
	} else {							\
	    if (need_icd) {						\
		snprintf(buf, BUFSIZE, "%ls", _var);			\
		char *s = str_cd_iconv(buf, icd);			\
		if (s == NULL) {					\
		    if (errno == EILSEQ) {				\
			err_exit(_("%s: could not convert string"),	\
				 filename);				\
		    } else {						\
			errno_exit("str_cd_iconv");			\
		    }							\
		}							\
		save_game_printf("%s", s);				\
		free(s);						\
	    } else {							\
		save_game_printf("%ls", _var);				\
	    }								\
	}								\
    } while (0)
#else // ! USE_UTF8_GAME_FILE
#  define save_game_write_string(_var, _var_utf8)			\
    do {								\
	if ((_var_utf8) != NULL) {					\
	    save_game_printf("%s", _var_utf8);				\
	} else {							\
	    save_game_printf("%ls", _var);				\
	}								\
    } while (0)
#endif // ! USE_UTF8_GAME_FILE


/************************************************************************
*                Game load and save function definitions                *
************************************************************************/

// These functions are documented in the file "fileio.h"


/***********************************************************************/
// load_game: Load a previously-saved game from disk

bool load_game (int num)
{
    char *filename;
    FILE *file;
    char *codeset, *codeset_nl;
    int saved_errno, lineno;
    char *prev_locale;

    char *buf, *inbuf;
    wchar_t *wcbuf;

    unsigned int crypt_key;
    unsigned int *crypt_key_p;
    int is_encrypted_input;
    int n, i, j;

#ifdef USE_UTF8_GAME_FILE
    iconv_t icd;
    bool need_icd;
#endif


    assert(num >= 1 && num <= 9);

    buf = xmalloc(BUFSIZE);
    inbuf = xmalloc(BIGBUFSIZE);
    wcbuf = xmalloc(BUFSIZE * sizeof(wchar_t));

    filename = game_filename(num);
    assert(filename != NULL);

    file = fopen(filename, "r");
    if (file == NULL) {
	// File could not be opened

	if (errno == ENOENT) {
	    // File not found
	    txdlgbox(MAX_DLG_LINES, 50, 9, WCENTER, attr_error_window,
		     attr_error_title, attr_error_highlight, 0, 0,
		     attr_error_waitforkey, _("  Game Not Found  "),
		     _("Game %d has not been saved to disk."), num);
	} else {
	    // Some other file error
	    saved_errno = errno;
	    txdlgbox(MAX_DLG_LINES, 60, 9, WCENTER, attr_error_window,
		     attr_error_title, attr_error_highlight,
		     attr_error_normal, 0, attr_error_waitforkey,
		     _("  Game Not Loaded  "),
		     _("Game %d could not be loaded from disk.\n\n"
		       "^{File %s: %s^}"), num, filename,
		     strerror(saved_errno));
	}

	free(buf);
	free(inbuf);
	free(wcbuf);
	free(filename);
	return false;
    }

#ifdef USE_UTF8_GAME_FILE
    // Make sure all strings are read in UTF-8 format for consistency
    codeset = nl_langinfo(CODESET);
    if (codeset == NULL) {
	errno_exit("nl_langinfo(CODESET)");
    }
    need_icd = (strcmp(codeset, GAME_FILE_CHARSET) != 0);
    if (need_icd) {
	// Try using the GNU libiconv "//TRANSLIT" option
	strcpy(buf, codeset);
	strcat(buf, GAME_FILE_TRANSLIT);

	icd = iconv_open(buf, GAME_FILE_CHARSET);
	if (icd == (iconv_t) -1) {
	    // Try iconv_open() without "//TRANSLIT"
	    icd = iconv_open(codeset, GAME_FILE_CHARSET);
	    if (icd == (iconv_t) -1) {
		errno_exit("iconv_open");
	    }
	}
    } else {
	icd = (iconv_t) -1;
    }
    codeset_nl = xstrdup(GAME_FILE_CHARSET "\n");
#else // ! USE_UTF8_GAME_FILE
    // Make sure all strings are read in the correct codeset
    codeset = nl_langinfo(CODESET);
    if (codeset == NULL) {
	errno_exit("nl_langinfo(CODESET)");
    }
    codeset_nl = xmalloc(strlen(codeset) + 2);
    strcpy(codeset_nl, codeset);
    strcat(codeset_nl, "\n");
#endif // ! USE_UTF8_GAME_FILE

    // Change the formatting of numbers to the POSIX locale for consistency
    prev_locale = xstrdup(setlocale(LC_NUMERIC, NULL));
    setlocale(LC_NUMERIC, "C");

    // Read the game file header
    if (fgets(buf, BUFSIZE, file) == NULL) {
	err_exit(_("%s: missing header in game file"), filename);
    }
    if (strcmp(buf, GAME_FILE_HEADER "\n") != 0) {
	err_exit(_("%s: not a valid game file"), filename);
    }
    if (fgets(buf, BUFSIZE, file) == NULL) {
	err_exit(_("%s: missing subheader in game file"), filename);
    }
    if (strcmp(buf, GAME_FILE_API_VERSION "\n") != 0) {
	err_exit(_("%s: saved under a different version of Star Traders"),
		 filename);
    }
    if (fgets(buf, BUFSIZE, file) == NULL) {
	err_exit(_("%s: missing subheader in game file"), filename);
    }
    if (strcmp(buf, codeset_nl) != 0) {
	err_exit(_("%s: saved under an incompatible character encoding"),
		 filename);
    }

    lineno = 4;

    // Read in the game file encryption status
    if (fscanf(file, "%i\n", &is_encrypted_input) != 1) {
	err_exit(_("%s: illegal or missing field on line %d"), filename, lineno);
    }
    lineno++;

    crypt_key = 0;
    crypt_key_p = is_encrypted_input ? &crypt_key : NULL;

    // Read in various game variables
    load_game_read_int(n,                n == MAX_X);
    load_game_read_int(n,                n == MAX_Y);
    load_game_read_int(max_turn,         max_turn >= 1);
    load_game_read_int(turn_number,      turn_number >= 1 && turn_number <= max_turn);
    load_game_read_int(number_players,   number_players >= 1 && number_players <= MAX_PLAYERS);
    load_game_read_int(current_player,   current_player >= 0 && current_player < number_players);
    load_game_read_int(first_player,     first_player >= 0 && first_player < number_players);
    load_game_read_int(n,                n == MAX_COMPANIES);
    load_game_read_double(interest_rate, interest_rate > 0.0);

    // Read in player data
    for (i = 0; i < number_players; i++) {
	load_game_read_string(player[i].name, player[i].name_utf8);
	load_game_read_double(player[i].cash, player[i].cash >= 0.0);
	load_game_read_double(player[i].debt, player[i].debt >= 0.0);
	load_game_read_bool(player[i].in_game);

	for (j = 0; j < MAX_COMPANIES; j++) {
	    load_game_read_long(player[i].stock_owned[j], player[i].stock_owned[j] >= 0);
	}
    }

    // Read in company data
    for (i = 0; i < MAX_COMPANIES; i++) {
	xmbstowcs(wcbuf, gettext(company_name[i]), BUFSIZE);
	company[i].name = xwcsdup(wcbuf);
	load_game_read_double(company[i].share_price,  company[i].share_price >= 0.0);
	load_game_read_double(company[i].share_return, true);
	load_game_read_long(company[i].stock_issued,   company[i].stock_issued >= 0);
	load_game_read_long(company[i].max_stock,      company[i].max_stock >= 0);
	load_game_read_bool(company[i].on_map);
    }

    // Read in galaxy map
    for (int x = 0; x < MAX_X; x++) {
	if (fgets(inbuf, BIGBUFSIZE, file) == NULL) {
	    err_exit(_("%s: missing field on line %d"), filename, lineno);
	}
	if (unscramble(buf, inbuf, BUFSIZE, crypt_key_p) == NULL) {
	    err_exit(_("%s: illegal field on line %d"), filename, lineno);
	}
	if (strlen(buf) != MAX_Y + 1) {
	    err_exit(_("%s: illegal field on line %d"), filename, lineno);
	}

	for (int y = 0; y < MAX_Y; y++) {
	    char c = buf[y];
	    if (c == MAP_EMPTY || c == MAP_OUTPOST || c == MAP_STAR
		|| (c >= MAP_A && c <= MAP_LAST)) {
		galaxy_map[x][y] = (map_val_t) c;
	    } else {
		err_exit(_("%s: illegal value on line %d"), filename, lineno);
	    }
	}

	lineno++;
    }

    // Read in a dummy sentinel value
    load_game_read_int(n, n == GAME_FILE_SENTINEL);

    if (fclose(file) == EOF) {
	errno_exit("%s", filename);
    }

    // Change the formatting of numbers back to the user-supplied locale
    setlocale(LC_NUMERIC, prev_locale);

#ifdef USE_UTF8_GAME_FILE
    if (need_icd) {
	iconv_close(icd);
    }
#endif

    free(buf);
    free(inbuf);
    free(wcbuf);
    free(filename);
    free(prev_locale);
    free(codeset_nl);
    return true;
}


/***********************************************************************/
// save_game: Save the current game to disk

bool save_game (int num)
{
    const char *data_dir;
    char *buf, *encbuf;
    char *filename;
    FILE *file;
    char *codeset;
    int saved_errno;
    char *prev_locale;
    struct stat statbuf;
    int i, j, x, y;
    unsigned int crypt_key;
    unsigned int *crypt_key_p;

#ifdef USE_UTF8_GAME_FILE
    iconv_t icd;
    bool need_icd;
#endif


    assert(num >= 1 && num <= 9);

    buf = xmalloc(BUFSIZE);
    encbuf = xmalloc(BIGBUFSIZE);

    crypt_key = 0;
    crypt_key_p = option_dont_encrypt ? NULL : &crypt_key;

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
		txdlgbox(MAX_DLG_LINES, 60, 7, WCENTER, attr_error_window,
			 attr_error_title, attr_error_highlight,
			 attr_error_normal, 0, attr_error_waitforkey,
			 _("  Game Not Saved  "),
			 _("Game %d could not be saved to disk.\n\n"
			   "^{Directory %s: %s^}"), num, data_dir,
			 strerror(saved_errno));

		free(buf);
		free(encbuf);
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
	txdlgbox(MAX_DLG_LINES, 60, 7, WCENTER, attr_error_window,
		 attr_error_title, attr_error_highlight,
		 attr_error_normal, 0, attr_error_waitforkey,
		 _("  Game Not Saved  "),
		 _("Game %d could not be saved to disk.\n\n"
		   "^{File %s: %s^}"), num, filename, strerror(saved_errno));

	free(buf);
	free(encbuf);
	free(filename);
	return false;
    }

#ifdef USE_UTF8_GAME_FILE
    // Make sure all strings are output in UTF-8 format for consistency
    codeset = nl_langinfo(CODESET);
    if (codeset == NULL) {
	errno_exit("nl_langinfo(CODESET)");
    }
    need_icd = (strcmp(codeset, GAME_FILE_CHARSET) != 0);
    if (need_icd) {
	icd = iconv_open(GAME_FILE_CHARSET, codeset);
	if (icd == (iconv_t) -1) {
	    errno_exit("iconv_open");
	}
    } else {
	icd = (iconv_t) -1;
    }
    codeset = GAME_FILE_CHARSET;	// Now contains output codeset
#else // ! USE_UTF8_GAME_FILE
    // Make sure all strings are output in the correct codeset
    codeset = nl_langinfo(CODESET);
    if (codeset == NULL) {
	errno_exit("nl_langinfo(CODESET)");
    }
#endif // ! USE_UTF8_GAME_FILE

    // Change the formatting of numbers to the POSIX locale for consistency
    prev_locale = xstrdup(setlocale(LC_NUMERIC, NULL));
    setlocale(LC_NUMERIC, "C");

    // Write out the game file header and encryption status
    fprintf(file, "%s\n" "%s\n", GAME_FILE_HEADER, GAME_FILE_API_VERSION);
    fprintf(file, "%s\n" "%d\n", codeset, ! option_dont_encrypt);

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
	save_game_write_string(player[i].name, player[i].name_utf8);
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

	scramble(encbuf, buf, BIGBUFSIZE, crypt_key_p);
	fprintf(file, "%s", encbuf);
    }

    // Write out a dummy sentinel value
    save_game_write_int(GAME_FILE_SENTINEL);

    if (fclose(file) == EOF) {
	errno_exit("%s", filename);
    }

    // Change the formatting of numbers back to the user-supplied locale
    setlocale(LC_NUMERIC, prev_locale);

#ifdef USE_UTF8_GAME_FILE
    if (need_icd) {
	iconv_close(icd);
    }
#endif

    free(buf);
    free(encbuf);
    free(filename);
    free(prev_locale);
    return true;
}


/***********************************************************************/
// End of file
