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
  function for Star Traders.


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


#include "system.h"
#include "utils.h"
#include "intf.h"


/************************************************************************
*                           Module variables                            *
************************************************************************/

static char *program_name_str   = NULL;		// Canonical program name
static char *home_directory_str = NULL;		// Full pathname to home
static char *data_directory_str = NULL;		// Writable data dir pathname


/************************************************************************
*                     Utility function definitions                      *
************************************************************************/

/*-----------------------------------------------------------------------
  Function:   init_program_name  - Make the program name "canonical"
  Arguments:  argv               - Same as passed to main()
  Returns:    (nothing)

  This function modifies the argv[0] pointer to eliminate the leading
  path name from the program name.  In other words, it strips any leading
  directories and leaves just the base name of the program.  It also
  assigns the module variable "program_name_str".
*/

void init_program_name (char *argv[])
{
    if ((argv == NULL) || (argv[0] == NULL) || (*(argv[0]) == '\0')) {
	program_name_str = PACKAGE;
    } else {
	char *p = strrchr(argv[0], '/');

	if ((p != NULL) && (*++p != '\0')) {
	    argv[0] = p;
	}

	program_name_str = argv[0];
    }
}


/*-----------------------------------------------------------------------
  Function:   program_name  - Return the canonical program name
  Arguments:  (none)
  Returns:    const char *  - Pointer to program name

  This function returns the canonical program name (the program name as
  invoked on the command line, without any leading pathname).  NULL
  should never be returned.
*/

const char *program_name (void)
{
    if (program_name_str == NULL) {
	init_program_name(NULL);
    }

    return program_name_str;
}


/*-----------------------------------------------------------------------
  Function:   home_directory  - Return home directory pathname
  Arguments:  (none)
  Returns:    const char *    - Pointer to home directory

  This function returns the full pathname to the user's home directory,
  using the HOME environment variable.  Note that the existance or
  writability of this pathname is NOT checked by this function.  NULL is
  returned if the home directory cannot be determined.
*/

const char *home_directory (void)
{
    if (home_directory_str == NULL) {
	// Use the HOME environment variable where possible
	char *p = getenv("HOME");
	if ((p != NULL) && (*p != '\0')) {
	    home_directory_str = strdup(p);
	}
    }

    return home_directory_str;
}


/*-----------------------------------------------------------------------
  Function:   data_directory  - Return writable data directory pathname
  Arguments:  (none)
  Returns:    const char *    - Pointer to data directory

  This function returns the full pathname to a writable subdirectory
  within the user's home directory.  Essentially, home_directory() + "/."
  + program_name() is returned.  Note that this path is NOT created by
  this function, nor is the writability of this path checked.  NULL is
  returned if this path cannot be determined.
*/

const char *data_directory (void)
{
    if (data_directory_str == NULL) {
	const char *name = program_name();
	const char *home = home_directory();
	if ((name != NULL) && (home != NULL)) {
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


/*-----------------------------------------------------------------------
  Function:   game_filename  - Convert an integer to a game filename
  Arguments:  game_num       - Game number (1-9) as an integer
  Returns:    char *         - Pointer to game filename string

  This function returns the full game filename as a malloc()ed string.
  If game_num is between 1 and 9 inclusive, the string returned is in the
  form data_directory() + "/" + GAME_FILENAME(game_num).  If game_num is
  any other integer, NULL is returned.
*/

char *game_filename (const int game_num)
{
    char buf[GAME_FILENAME_BUFSIZE];		// Buffer for part of filename
    const char *dd;				// Data directory


    if ((game_num < 1) || (game_num > 9)) {
	return NULL;
    }

    dd = data_directory();
    snprintf(buf, GAME_FILENAME_BUFSIZE, GAME_FILENAME_PROTO, game_num);

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


/*-----------------------------------------------------------------------
  Function:   err_exit  - Print an error and exit
  Arguments:  format    - printf()-like format of error message
              ...       - printf()-like arguments
  Returns:    (does not return)

  This function closes all curses windows, prints the name of the program
  and the error message to stderr (using format and following arguments
  as if passed to printf()) and exits with error code EXIT_FAILURE.  The
  format supplied does NOT need to supply the program name nor the
  trailing end-line character.  The format should not be NULL; user-
  supplied strings should ALWAYS be printed using "%s" as the format (and
  with the user string as a second argument), NOT passed in as the format
  itself.
*/

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


/*-----------------------------------------------------------------------
  Function:   errno_exit  - Print an error (using errno) and exit
  Arguments:  format      - printf()-like format of error message
              ...         - printf()-like arguments
  Returns:    (does not return)

  This function closes all curses windows, prints the name of the
  program, the error message (using format and following arguments as if
  passed to printf()) and the string corresponding to errno to stderr,
  then exits with error code EXIT_FAILURE.  The format supplied does NOT
  need to supply the program name, any colons nor the trailing end-line
  character.  The format may be NULL if no intermediate message is
  needed.
*/

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


/*-----------------------------------------------------------------------
  Function:   err_exit_nomem  - Print an "out of memory" error and exit
  Arguments:  (none)
  Returns:    (does not return)

  This function calls err_exit with an "out of memory" error message.
*/

void err_exit_nomem (void)
{
    err_exit("out of memory");
}


/*-----------------------------------------------------------------------
  Function:   init_rand  - Initialise the random number generator
  Arguments:  (none)
  Returns:    (nothing)

  This function initialises the pseudo-random number generator.
*/

void init_rand (void)
{
    time_t curtime = time(NULL);    // NB: time_t may be larger than long int
    srand48((long int) curtime);
}


/*-----------------------------------------------------------------------
  Function:   randf   - Return a random number between 0.0 and 1.0
  Arguments:  (none)
  Returns:    double  - The random number

  This function returns a pseudo-random number between 0.0 (inclusive)
  and 1.0 (not inclusive) as a floating-point number.
*/

extern double randf (void)
{
    return drand48();
}


/*-----------------------------------------------------------------------
  Function:   randi  - Return a random number between 0 and limit
  Arguments:  limit  - Upper limit of random number
  Returns:    int    - The random number

  This function returns a pseudo-random number between 0 (inclusive) and
  limit (not inclusive) as an integer.
*/

extern int randi (int limit)
{
    return randf() * (double) limit;
}


/*-----------------------------------------------------------------------
  Function:   scramble  - Scramble (encrypt) the buffer
  Arguments:  key       - Encryption/decryption key
              buf       - Pointer to buffer to encrypt
              bufsize   - Size of buffer
  Returns:    char *    - Pointer to buffer

  This function scrambles (encrypts) the buffer pointed to by buf using a
  trivial in-place encryption algorithm.  If key is zero, no encryption
  takes place.  The buffer buf must contain a string terminated by '\0'.
  The characters '\r', '\n' and '\0' are guaranteed to remain the same
  after encryption.  At most bufsize characters are encrypted; buf is
  returned as the result.
*/

char *scramble (int key, char *buf, int bufsize)
{
    if ((buf != NULL) && (key != 0)) {
	char *p = buf;
	unsigned char k = ~key;
	int i;

	for (i = 0; (i < bufsize) && (*p != '\0'); i++, k++, p++) {
	    char c = *p;
	    char r = c ^ k;	// Simple encryption: XOR on a moving key

	    if ((c != '\r') && (c != '\n') &&
		(r != '\r') && (r != '\n') && (r != '\0')) {
		*p = r;
	    }
	}
    }

    return buf;
}


/*-----------------------------------------------------------------------
  Function:   unscramble  - Unscramble (decrypt) the buffer
  Arguments:  key         - Encryption/decryption key
              buf         - Pointer to buffer to decrypt
              bufsize     - Size of buffer
  Returns:    char *      - Pointer to buffer

  This function unscrambles (decrypts) the buffer pointed to by buf using
  a trivial in-place decryption algorithm.  If key is zero, no decryption
  takes place.  The buffer buf must contain a string terminated by '\0'.
  The characters '\r', '\n' and '\0' are guaranteed to remain the same
  after decryption.  At most bufsize characters are decrypted; buf is
  returned as the result.
*/

char *unscramble (int key, char *buf, int bufsize)
{
    return scramble(key, buf, bufsize);
}
