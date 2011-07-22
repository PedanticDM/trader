/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, utils.h, contains declarations for various utility functions
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


#ifndef included_UTILS_H
#define included_UTILS_H 1


/************************************************************************
*                       Utility macro definitions                       *
************************************************************************/

// Type-unsafe minimum and maximum macros

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


/************************************************************************
*                     Global variable declarations                      *
************************************************************************/

// Global copy, suitably modified, of localeconv() information
extern struct lconv lconvinfo;


/************************************************************************
*          Initialisation and environment function prototypes           *
************************************************************************/

/*
  Function:   init_program_name - Make the program name "canonical"
  Parameters: argv              - Same as passed to main()
  Returns:    (nothing)

  This function modifies the argv[0] pointer to eliminate any leading
  pathname (directory) components from the program name, leaving just the
  basename of the program.  It also saves a copy that can be accessed via
  the program_name() function.
*/
extern void init_program_name (char *argv[]);


/*
  Function:   program_name - Return the canonical program name
  Parameters: (none)
  Returns:    const char * - Pointer to program name

  This function returns the canonical program name (the program name as
  invoked on the command line, without any leading pathname components).
  NULL should never be returned; however, init_program_name() SHOULD be
  called before using this function.
*/
extern const char *program_name (void);


/*
  Function:   home_directory - Return home directory pathname
  Parameters: (none)
  Returns:    const char *   - Pointer to home directory

  This function returns the full pathname to the user's home directory,
  using the HOME environment variable.  Note that the existance or
  writability of this pathname is NOT checked by this function.  NULL is
  returned if the home directory cannot be determined.
*/
extern const char *home_directory (void);


/*
  Function:   data_directory - Return writable data directory pathname
  Parameters: (none)
  Returns:    const char *   - Pointer to data directory

  This function returns the full pathname to a potentially-writable
  subdirectory within the user's home directory.  Essentially, this
  function returns home_directory() + "/."  + program_name().  Note that
  this path is NOT created by this function, nor is the writability of
  this path checked.  NULL is returned if this path cannot be determined.
*/
extern const char *data_directory (void);


/*
  Function:   game_filename - Convert an integer to a game filename
  Parameters: gamenum       - Game number (1-9) as an integer
  Returns:    char *        - Pointer to game filename string

  This function returns the full game filename as a malloc()ed string
  (ie, a string that must be freed at a later time by calling free()).

  If gamenum is between 1 and 9 inclusive, the string returned is in the
  form data_directory() + "/" + GAME_FILENAME(gamenum).  If gamenum is
  any other integer, NULL is returned.
*/
extern char *game_filename (int gamenum);


/************************************************************************
*                  Error-reporting function prototypes                  *
************************************************************************/

/*
  Function:   err_exit - Print an error and exit
  Parameters: format   - printf()-like format of error message
              ...      - printf()-like arguments
  Returns:    (does not return)

  This function closes all Curses windows, prints the name of the program
  and the error message to stderr (using format and following arguments
  as if passed to printf()) and exits with error code EXIT_FAILURE.

  The format supplied does NOT need to supply the program name nor the
  trailing end-line character.  The format should not be NULL; user-
  supplied strings should ALWAYS be printed using "%s" as the format (and
  with the user string as a second argument), NOT passed in as the format
  itself.
*/
extern void err_exit (const char *restrict format, ...)
    __attribute__((noreturn, format (printf, 1, 2)));


/*
  Function:   errno_exit - Print an error message (using errno) and exit
  Parameters: format     - printf()-like format of error message
              ...        - printf()-like arguments
  Returns:    (does not return)

  This function closes all Curses windows, prints the name of the
  program, the supplied error message (using format and following
  arguments as if passed to printf()), then the string corresponding to
  errno to stderr.  It then exits with error code EXIT_FAILURE.

  The format supplied does NOT need to supply the program name, any
  colons nor the trailing end-line character.  The format may be NULL if
  no intermediate message is needed.
*/
extern void errno_exit (const char *restrict format, ...)
    __attribute__((noreturn, format (printf, 1, 2)));


/*
  Function:   err_exit_nomem - Print an "out of memory" error and exit
  Parameters: (none)
  Returns:    (does not return)

  This function calls err_exit() with an "out of memory" error message.
  It simply ensures all memory exhaustion error messages are consistent.
*/
extern void err_exit_nomem (void)
    __attribute__((noreturn));


/************************************************************************
*                   Random-number function prototypes                   *
************************************************************************/

/*
  Function:   init_rand - Initialise the random number generator
  Parameters: (none)
  Returns:    (nothing)

  This function initialises the pseudo-random number generator.  It
  should be called before any random-number functions declared in this
  header are called.
*/
extern void init_rand (void);


/*
  Function:   randf  - Return a random number between 0.0 and 1.0
  Parameters: (none)
  Returns:    double - The random number

  This function returns a pseudo-random number between 0.0 (inclusive)
  and 1.0 (not inclusive) as a floating-point number.  By default, a
  linear congruential algorithm is used to generate the random number.
*/
extern double randf (void);


/*
  Function:   randi - Return a random number between 0 and limit
  Parameters: limit - Upper limit of random number
  Returns:    int   - The random number

  This function returns a pseudo-random number between 0 (inclusive) and
  limit (not inclusive) as an integer.  It uses the same algorithm as
  randf() to generate the random number.
*/
extern int randi (int limit);


/************************************************************************
*                   Locale-aware function prototypes                    *
************************************************************************/

/*
  Function:   init_locale - Initialise locale-specific variables
  Parameters: (none)
  Returns:    (nothing)

  This function initialises the global variable lconvinfo with values
  suitable for this program.  In particular, if the POSIX or C locale is
  in effect, the currency_symbol and frac_digits members are updated to
  be something reasonable.  This function must be called before using
  localeconf_info.
*/
extern void init_locale (void);


/*
  Function:   l_strfmon - Convert monetary value to a string
  Parameters: s         - Buffer to receive result
              maxsize   - Maximum size of buffer
              format    - strfmon() format to use
	      val       - Monetary value to convert
  Returns:    ssize_t   - Size of returned string

  This function calls strfmon() to convert val to a suitable monetary
  value string.  If the POSIX or C locale is in effect, and "!" does NOT
  appear in the format, "$" is inserted into the resulting string.  This
  function overcomes the limitation that the POSIX locale does not define
  anything for localeconv()->currency_symbol.
*/
extern ssize_t l_strfmon (char *restrict s, size_t maxsize,
			  const char *restrict format, double val);


/************************************************************************
*                    Encryption function prototypes                     *
************************************************************************/

/*
  The functions described here are simple in the extreme: they are only
  designed to stop casual cheating!
*/

/*
  Function:   scramble - Scramble (encrypt) the buffer
  Parameters: key      - Encryption key
              buf      - Pointer to buffer to encrypt
              bufsize  - Size of buffer
  Returns:    char *   - Pointer to buffer

  This function scrambles (encrypts) the buffer *buf using a trivial
  in-place encryption algorithm.  If key is zero, or buf is NULL or
  bufsize is less than 1, no encryption takes place.

  The buffer should contain a C-style string terminated by '\0'.  The
  characters '\r', '\n' and '\0' are guaranteed to remain the same before
  and after encryption.  At most bufsize bytes are encrypted; buf is
  returned as the result.
*/
extern char *scramble (int key, char *restrict buf, int bufsize);


/*
  Function:   unscramble - Unscramble (decrypt) the buffer
  Parameters: key        - Encryption/decryption key
              buf        - Pointer to buffer to decrypt
              bufsize    - Size of buffer
  Returns:    char *     - Pointer to buffer

  This function does the reverse of scramble(): it unscrambles (decrypts)
  the buffer *buf using an in-place algorithm.  If key is zero, or buf is
  NULL or bufsize is less than 1, no decryption takes place.

  The buffer should contain a C-style string terminated by '\0'.  As for
  scramble(), the characters '\r', '\n' and '\0' will not be changed (nor
  will any encrypted character map back to these values).  At most
  bufsize bytes are decrypted; buf is returned as the result.
*/
extern char *unscramble (int key, char *restrict buf, int bufsize);


#endif /* included_UTILS_H */
