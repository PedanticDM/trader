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
*                       Module-specific constants                       *
************************************************************************/

#define GAME_FILENAME_PROTO	"game%d"
#define GAME_FILENAME_BUFSIZE	16


/************************************************************************
*                           Module variables                            *
************************************************************************/

static char *program_name_str   = NULL;		// Canonical program name
static char *home_directory_str = NULL;		// Full pathname to home
static char *data_directory_str = NULL;		// Writable data dir pathname


/************************************************************************
*          Initialisation and environment function definitions          *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
void init_program_name (char *argv[])
{
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
const char *program_name (void)
{
    if (program_name_str == NULL) {
	init_program_name(NULL);
    }

    return program_name_str;
}


/***********************************************************************/
const char *home_directory (void)
{
    if (home_directory_str == NULL) {
	// Use the HOME environment variable where possible
	char *home = getenv("HOME");

	if (home != NULL && *home != '\0') {
	    home_directory_str = strdup(home);
	}
    }

    return home_directory_str;
}


/***********************************************************************/
const char *data_directory (void)
{
    /* This implementation assumes a POSIX environment by using "/" as
       the directory separator.  It also assumes a dot-starting directory
       name is permissible (again, true on POSIX systems) */

    if (data_directory_str == NULL) {
	const char *name = program_name();
	const char *home = home_directory();

	if (name != NULL && home != NULL) {
	    char *p = malloc(strlen(home) + strlen(name) + 3);
	    if (p != NULL) {
		strcpy(p, home);
		strcat(p, "/.");
		strcat(p, name);
		data_directory_str = p;
	    }
	}
    }

    return data_directory_str;
}


/***********************************************************************/
char *game_filename (int gamenum)
{
    /* This implementation assumes a POSIX environment by using "/" as
       the directory separator */

    char buf[GAME_FILENAME_BUFSIZE];	// Buffer for part of filename
    const char *dd;			// Data directory


    if (gamenum < 1 || gamenum > 9) {
	return NULL;
    }

    dd = data_directory();
    snprintf(buf, GAME_FILENAME_BUFSIZE, GAME_FILENAME_PROTO, gamenum);

    if (dd == NULL) {
	char *p = malloc(strlen(buf) + 1);

	if (p != NULL) {
	    strcpy(p, buf);
	}
	return p;
    } else {
	char *p = malloc(strlen(dd) + strlen(buf) + 2);

	if (p != NULL) {
	    strcpy(p, dd);
	    strcat(p, "/");
	    strcat(p, buf);
	}
	return p;
    }
}


/************************************************************************
*                 Error-reporting function definitions                  *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
void err_exit (const char *format, ...)
{
    va_list args;


    end_screen();

    fprintf(stderr, "%s: ", program_name());
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    exit(EXIT_FAILURE);
}


/***********************************************************************/
void errno_exit (const char *format, ...)
{
    va_list args;
    int saved_errno = errno;


    end_screen();

    fprintf(stderr, "%s: ", program_name());
    if (format != NULL) {
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs(": ", stderr);
    }
    fprintf(stderr, "%s\n", strerror(saved_errno));

    exit(EXIT_FAILURE);
}


/***********************************************************************/
void err_exit_nomem (void)
{
    err_exit("out of memory");
}


/************************************************************************
*                  Random-number function definitions                   *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
void init_rand (void)
{
    /* Ideally, initialisation of the random number generator should be
       made using seed48() and lcong48().  However, we can only be
       assured of no more than 32 bits of "randomness" by using time(),
       available on all POSIX systems.  If time_t is larger than long
       int, we throw away the top bits. */

    time_t curtime = time(NULL);    // NB: time_t may be larger than long int
    srand48((long int) curtime);
}


/***********************************************************************/
extern double randf (void)
{
    return drand48();
}


/***********************************************************************/
extern int randi (int limit)
{
    return drand48() * (double) limit;
}


/***********************************************************************/
char *scramble (int key, char *buf, int bufsize)
{
    /* The algorithm used here is reversable: scramble(scramble(...))
       will (or, at least, should!) return the same as the original
       buffer.  Problematic characters are ignored; however, this
       function assumes all other characters are permitted in files.
       This is true on all POSIX systems. */

    if (buf != NULL && key != 0) {
	char *p = buf;
	unsigned char k = ~key;
	int i;

	for (i = 0; i < bufsize && *p != '\0'; i++, k++, p++) {
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
char *unscramble (int key, char *buf, int bufsize)
{
    return scramble(key, buf, bufsize);
}


/***********************************************************************/
// End of file
