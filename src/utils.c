/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, utils.c, contains the implementation of various utility
  functions used in Star Traders.


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

// Global copy, suitably modified, of localeconv() information
struct lconv lconvinfo;


/************************************************************************
*          Module-specific constants and variable definitions           *
************************************************************************/

#define GAME_FILENAME_PROTO	"game%d"
#define GAME_FILENAME_BUFSIZE	16

// Default values used to override POSIX locale
#define MOD_POSIX_CURRENCY_SYMBOL	"$"
#define MOD_POSIX_FRAC_DIGITS		2
#define MOD_POSIX_P_CS_PRECEDES		1
#define MOD_POSIX_P_SEP_BY_SPACE	0


/************************************************************************
*                       Module-specific variables                       *
************************************************************************/

static char *program_name_str   = NULL;		// Canonical program name
static char *home_directory_str = NULL;		// Full pathname to home
static char *data_directory_str = NULL;		// Writable data dir pathname

static bool add_currency_symbol = false;	// Do we need to add "$"?


/************************************************************************
*          Initialisation and environment function definitions          *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
// init_program_name: Make the program name "canonical"

void init_program_name (char *argv[])
{
    /* This implementation assumes a POSIX environment with an ASCII-safe
       character encoding (such as ASCII or UTF-8). */

    if (argv == NULL || argv[0] == NULL || *argv[0] == '\0') {
	program_name_str = PACKAGE;
    } else {
	char *p = strrchr(argv[0], '/');

	if (p != NULL && *++p != '\0') {
	    argv[0] = p;
	}

	program_name_str = argv[0];
    }
}


/***********************************************************************/
// program_name: Return the canonical program name

const char *program_name (void)
{
    if (program_name_str == NULL) {
	init_program_name(NULL);
    }

    return program_name_str;
}


/***********************************************************************/
// home_directory: Return home directory pathname

const char *home_directory (void)
{
    if (home_directory_str == NULL) {
	// Use the HOME environment variable where possible
	char *home = getenv("HOME");

	if (home != NULL && *home != '\0') {
	    home_directory_str = xstrdup(home);
	}
    }

    return home_directory_str;
}


/***********************************************************************/
// data_directory: Return writable data directory pathname

const char *data_directory (void)
{
    /* This implementation assumes a POSIX environment by using "/" as
       the directory separator.  It also assumes a dot-starting directory
       name is permissible (again, true on POSIX systems) and that the
       character encoding is ASCII-safe. */

    if (data_directory_str == NULL) {
	const char *name = program_name();
	const char *home = home_directory();

	if (name != NULL && home != NULL) {
	    char *p = xmalloc(strlen(home) + strlen(name) + 3);

	    strcpy(p, home);
	    strcat(p, "/.");
	    strcat(p, name);
	    data_directory_str = p;
	}
    }

    return data_directory_str;
}


/***********************************************************************/
// game_filename: Convert an integer to a game filename

char *game_filename (int gamenum)
{
    /* This implementation assumes a POSIX environment and an ASCII-safe
       character encoding. */

    char buf[GAME_FILENAME_BUFSIZE];	// Buffer for part of filename
    const char *dd;			// Data directory


    if (gamenum < 1 || gamenum > 9) {
	return NULL;
    }

    dd = data_directory();
    snprintf(buf, GAME_FILENAME_BUFSIZE, GAME_FILENAME_PROTO, gamenum);

    if (dd == NULL) {
	return xstrdup(buf);
    } else {
	char *p = xmalloc(strlen(dd) + strlen(buf) + 2);

	strcpy(p, dd);
	strcat(p, "/");
	strcat(p, buf);
	return p;
    }
}


/************************************************************************
*                 Error-reporting function definitions                  *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
// err_exit: Print an error and exit

void err_exit (const char *restrict format, ...)
{
    va_list args;


    end_screen();

    fprintf(stderr, _("%s: "), program_name());
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    exit(EXIT_FAILURE);
}


/***********************************************************************/
// errno_exit: Print an error message (using errno) and exit

void errno_exit (const char *restrict format, ...)
{
    va_list args;
    int saved_errno = errno;


    end_screen();

    fprintf(stderr, _("%s: "), program_name());
    if (format != NULL) {
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs(_(": "), stderr);
    }
    fprintf(stderr, "%s\n", strerror(saved_errno));

    exit(EXIT_FAILURE);
}


/***********************************************************************/
// err_exit_nomem: Print an "out of memory" error and exit

void err_exit_nomem (void)
{
    err_exit(_("out of memory"));
}


/************************************************************************
*                  Random-number function definitions                   *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
// init_rand: Initialise the random number generator

void init_rand (void)
{
    /* Ideally, initialisation of the random number generator should be
       made using seed48() and lcong48().  However, since this is "only a
       game", 32 bits of "randomness" as returned by gettimeofday() is
       probably more than enough... */

    struct timeval tv;
    unsigned long int seed;

    gettimeofday(&tv, NULL);		// If this fails, tv is random enough!
    seed = tv.tv_sec + tv.tv_usec;

    srand48(seed);
}


/***********************************************************************/
// randf: Return a random number between 0.0 and 1.0

extern double randf (void)
{
    return drand48();
}


/***********************************************************************/
// randi: Return a random number between 0 and limit

extern int randi (int limit)
{
    return drand48() * (double) limit;
}


/************************************************************************
*                   Locale-aware function definitions                   *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
// init_locale: Initialise locale-specific variables

void init_locale (void)
{
    char *cur, *cloc;
    struct lconv *lc;


    cur = xstrdup(setlocale(LC_MONETARY, NULL));
    lc = localeconv();
    assert(lc != NULL);

    lconvinfo = *lc;

    add_currency_symbol = false;

    /* Are we in the POSIX locale?  The string returned by setlocale() is
       supposed to be opaque, but in practise is not.  To be on the safe
       side, we explicitly set the locale to "C", then test the returned
       value of that, too. */
    cloc = setlocale(LC_MONETARY, "C");
    if (   strcmp(cur, cloc)      == 0
	|| strcmp(cur, "POSIX")   == 0 || strcmp(cur, "C")      == 0
	|| strcmp(cur, "C.UTF-8") == 0 || strcmp(cur, "C.utf8") == 0) {

	add_currency_symbol = true;
	lconvinfo.currency_symbol = MOD_POSIX_CURRENCY_SYMBOL;
	lconvinfo.frac_digits     = MOD_POSIX_FRAC_DIGITS;
	lconvinfo.p_cs_precedes   = MOD_POSIX_P_CS_PRECEDES;
	lconvinfo.p_sep_by_space  = MOD_POSIX_P_SEP_BY_SPACE;
    }

    setlocale(LC_MONETARY, cur);
    free(cur);
}


/***********************************************************************/
// l_strfmon: Convert monetary value to a string

ssize_t l_strfmon (char *restrict s, size_t maxsize,
		   const char *restrict format, double val)
{
    /* The current implementation assumes MOD_POSIX_P_CS_PRECEDES is 1
       (currency symbol precedes value) and that MOD_POSIX_P_SEP_BY_SPACE
       is 0 (no space separates currency symbol and value).  It does,
       however, handle currency symbols of length > 1. */

    assert(MOD_POSIX_P_CS_PRECEDES  == 1);
    assert(MOD_POSIX_P_SEP_BY_SPACE == 0);

    ssize_t ret = strfmon(s, maxsize, format, val);

    if (ret > 0 && add_currency_symbol) {
	if (strstr(format, "!") == NULL) {
	    /* Insert lconvinfo.currency_symbol to s.

	       NB: add_currecy_symbol == true assumes a POSIX locale and
	       that the character encoding is ASCII-safe (such as by
	       being ASCII itself, or UTF-8). */
	    const char *sym = lconvinfo.currency_symbol;
	    int symlen = strlen(sym);
	    char *p;
	    int spc;

	    assert(maxsize > (unsigned int) symlen);

	    // Count number of leading spaces
	    for (p = s, spc = 0; *p == ' '; p++, spc++)
		;

	    if (symlen <= spc) {
		/* Enough space for currency symbol: copy it WITHOUT
		   copying terminating NUL character */
		for (p -= symlen; *sym != '\0'; p++, sym++) {
		    *p = *sym;
		}
	    } else {
		// Make space for currency symbol, then copy it

		memmove(s + symlen - spc, s, maxsize - (symlen - spc));
		s[maxsize - 1] = '\0';

		for ( ; *sym != '\0'; sym++, s++) {
		    // Make sure terminating NUL character is NOT copied!
		    *s = *sym;
		}

		ret = MIN((unsigned int) ret + symlen - spc, maxsize - 1);
	    }
	}
    }

    return ret;
}


/************************************************************************
*                    Encryption function definitions                    *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
// scramble: Scramble (encrypt) the buffer

char *scramble (int key, char *restrict buf, int bufsize)
{
    /* The algorithm used here is reversable: scramble(scramble(...))
       will (or, at least, should!) return the same as the original
       buffer.  Problematic characters are ignored; however, this
       function assumes all other characters are permitted in files.
       This is true on all POSIX systems. */

    if (buf != NULL && key != 0) {
	char *p = buf;
	unsigned char k = ~key;

	for (int i = 0; i < bufsize && *p != '\0'; i++, k++, p++) {
	    char c = *p;
	    char r = c ^ k;	// Simple encryption: XOR on a moving key

	    if (c != '\r' && c != '\n'
		&& r != '\r' && r != '\n' && r != '\0') {
		*p = r;
	    }
	}
    }

    return buf;
}


/***********************************************************************/
// unscramble: Unscramble (decrypt) the buffer

char *unscramble (int key, char *restrict buf, int bufsize)
{
    return scramble(key, buf, bufsize);
}


/************************************************************************
*                  Miscellaneous function definitions                   *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
// xmalloc: Allocate a new block of memory, with checking

void *xmalloc (size_t size)
{
    void *p;


    if (size < 1)
	size = 1;

    p = malloc(size);
    if (p == NULL) {
	err_exit_nomem();
    }

    return p;
}


/***********************************************************************/
// xstrdup: Duplicate a string, with checking

char *xstrdup (const char *str)
{
    char *s;


    if (str == NULL)
	str = "";

    s = strdup(str);
    if (s == NULL) {
	err_exit_nomem();
    }

    return s;
}


/***********************************************************************/
// chstrdup: Duplicate a chtype buffer

chtype *xchstrdup (const chtype *restrict chstr)
{
    const chtype *p;
    int len;
    chtype *ret;


    // Determine chstr length, including ending NUL
    for (len = 1, p = chstr; *p != '\0'; p++, len++)
	;

    ret = xmalloc(len * sizeof(chtype));
    memcpy(ret, chstr, len * sizeof(chtype));
    ret[len - 1] = '\0';	// Terminating NUL, just in case not present

    return ret;
}


/***********************************************************************/
// xwcsdup: Duplicate a wide-character string, with checking

wchar_t *xwcsdup (const wchar_t *str)
{
    wchar_t *s;


    if (str == NULL)
	str = L"";

    s = wcsdup(str);
    if (s == NULL) {
	err_exit_nomem();
    }

    return s;
}


/***********************************************************************/
// xmbstowcs: Convert a multibyte string to a wide-character string

size_t xmbstowcs (wchar_t *restrict dest, const char *restrict src, size_t len)
{
    assert(dest != NULL);
    assert(len > 0);

    char *s = xstrdup(src);
    size_t n;

    while (true) {
	mbstate_t mbstate;
	char *p = s;

	memset(&mbstate, 0, sizeof(mbstate));
	if ((n = mbsrtowcs(dest, (const char **) &p, len, &mbstate))
	    == (size_t) -1) {
	    if (errno == EILSEQ) {
		// Illegal sequence detected: replace it and try again
		*p = EILSEQ_REPL;
	    } else {
		errno_exit(_("xmbstowcs: `%s'"), src);
	    }
	} else if (p != NULL) {
	    // Multibyte string was too long: truncate dest
	    dest[len - 1] = '\0';
	    n--;
	    break;
	} else {
	    break;
	}
    }

    free(s);
    return n;
}


/***********************************************************************/
// xwcrtomb: Convert a wide character to a multibyte sequence

size_t xwcrtomb (char *restrict dest, wchar_t wc, mbstate_t *restrict mbstate)
{
    mbstate_t mbcopy;
    size_t n;


    assert(dest != NULL);
    assert(mbstate != NULL);

    memcpy(&mbcopy, mbstate, sizeof(mbcopy));

    if ((n = wcrtomb(dest, wc, &mbcopy)) == (size_t) -1) {
	if (errno == EILSEQ) {
	    /* wc cannot be represented in current locale.

	       Note that the shift state in mbcopy is now undefined.
	       Hence, restore the original, try to store an ending shift
	       sequence, then EILSEQ_REPL. */
	    memcpy(&mbcopy, mbstate, sizeof(mbcopy));
	    if ((n = wcrtomb(dest, '\0', &mbcopy)) == (size_t) -1) {
		errno_exit(_("xwcrtomb: NUL"));
	    }
	    dest[n] = EILSEQ_REPL;
	    dest[n++] = '\0';
	} else {
	    errno_exit(_("xwcrtomb: `%lc'"), wc);
	}
    }

    memcpy(mbstate, &mbcopy, sizeof(mbcopy));
    return n;
}


/***********************************************************************/
// End of file
