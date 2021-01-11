/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2021, John Zaitseff                 *
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
  along with this program.  If not, see https://www.gnu.org/licenses/.
*/


#ifndef included_UTILS_H
#define included_UTILS_H 1


/************************************************************************
*                       Utility macro definitions                       *
************************************************************************/

// Type-unsafe minimum and maximum macros

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


#define EILSEQ_REPL	'?'	// Illegal character sequence replacement
#define EILSEQ_REPL_WC	L'?'	// ... wide character version


/************************************************************************
*                     Global variable declarations                      *
************************************************************************/

extern const char *program_name;	// Canonical program name


// Global copy, suitably modified, of localeconv() information
extern struct lconv lconvinfo;

// localeconv() information, converted to wide strings
extern wchar_t *decimal_point;		// Locale's radix character
extern wchar_t *thousands_sep;		// Locale's thousands separator
extern wchar_t *currency_symbol;	// Local currency symbol
extern wchar_t *mon_decimal_point;	// Local monetary radix character
extern wchar_t *mon_thousands_sep;	// Local monetary thousands separator


/************************************************************************
*          Initialisation and environment function prototypes           *
************************************************************************/

/*
  Function:   init_program_name - Make the program name canonical
  Parameters: argv0             - Same as passed to main() as argv[0]
  Returns:    (nothing)

  This function modifies the argv0 pointer to eliminate any leading
  pathname (directory) components from the program name, leaving just the
  basename of the program.  It also saves a copy that can be accessed via
  the program_name global variable.
*/
extern void init_program_name (const char *argv0);


/*
  Function:   home_directory - Return home directory pathname
  Parameters: (none)
  Returns:    const char *   - Pointer to home directory

  This function returns the full pathname to the user's home directory,
  using the HOME environment variable.  Note that the existence of or
  ability to write to this pathname is NOT checked by this function.
  NULL is returned if the home directory cannot be determined.
*/
extern const char *home_directory (void);


/*
  Function:   data_directory - Return writable data directory pathname
  Parameters: (none)
  Returns:    const char *   - Pointer to data directory

  This function returns the full pathname to a potentially-writable data
  directory, usually within the user's home directory.

  Assuming program_name is set to "trader", if "$HOME/.trader" exists,
  that directory is returned as the data directory.  Otherwise, if the
  environment variable XDG_DATA_HOME is set and contains an absolute
  pathname, "$XDG_DATA_HOME/trader" is returned.  Otherwise,
  "$HOME/.local/share/trader" is returned.

  Note that the returned path is NOT created by this function, nor is the
  ability to read from or write to this path checked.  NULL is returned
  if the path cannot be determined.
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

  This function initialises the global variable lconvinfo, as well as
  decimal_point, thousands_sep, currency_symbol, mon_decimal_point and
  mon_thousands_sep, with values suitable for this program.  In
  particular, if the POSIX or C locale is in effect, the currency_symbol
  and frac_digits members of lconvinfo are updated to be something
  reasonable.  This function must be called before using localeconf_info.
*/
extern void init_locale (void);


/*
  Function:   xwcsfmon - Convert monetary value to a wide-character string
  Parameters: buf      - Buffer to receive result
              maxsize  - Maximum size of buffer, in multiples of wchar_t
              format   - strfmon() format to use
              val      - Monetary value to convert
  Returns:    ssize_t  - Size of returned string

  This function calls strfmon() to convert val to a suitable monetary
  value string, then converts the result to a wide-character string and
  places it in buf.  It makes appropriate adjustments to the output if
  the POSIX locale is in effect or if the locale uses no-break spaces.
*/
extern ssize_t xwcsfmon (wchar_t *restrict buf, size_t maxsize,
			 const char *restrict format, double val);


/************************************************************************
*                    Encryption function prototypes                     *
************************************************************************/

/*
  The functions described here are NOT cryptographically secure: they are
  only designed to stop casual cheating!
*/

/*
  Function:   scramble - Scramble (encrypt) the buffer
  Parameters: dest     - Pointer to output buffer
              src      - Pointer to input buffer to encrypt
              size     - Size of output buffer
              key      - Pointer to encryption/decryption key
  Returns:    char *   - Pointer to output buffer

  This function scrambles (encrypts) the buffer *src and places the
  result in *dest.  It uses *key to keep a running encryption key.  If
  the key is NULL, no encryption is performed.

  The input buffer should contain a C-style string terminated by '\0'.
  The output buffer will be terminated with '\n\0', even if the input
  does not have a terminating '\n'.  The pointer dest is returned as the
  output.

  Note that src and dest MUST point to different buffers, and that *dest
  typically must be twice as large as *src.  In addition, *key MUST be
  initialised to zero before calling scramble() for the first time.
*/
extern char *scramble (char *restrict dest, const char *restrict src,
		       size_t size, unsigned int *restrict key);


/*
  Function:   unscramble - Unscramble (decrypt) the buffer
  Parameters: dest       - Pointer to output buffer
              src        - Pointer to input buffer to decrypt
              size       - Size of output buffer
              key        - Pointer to encryption/decryption key
  Returns:    char *     - Pointer to output buffer or NULL on error

  This function does the reverse of scramble(): it unscrambles (decrypts)
  the buffer *src and places the result in *dest.  If key is NULL, no
  decryption takes place: the input buffer is copied to the output buffer
  without changes.

  The buffer should contain a C-style string terminated by '\0'.  Note
  that src and dest MUST point to different buffers.  The pointer dest is
  returned as the output, unless there is an error in the data (such as a
  corrupted checksum), in which case NULL is returned.

  Note that *key MUST be initialised to zero before calling unscramble()
  for the first time.
*/
extern char *unscramble (char *restrict dest, const char *restrict src,
			 size_t size, unsigned int *restrict key);


/************************************************************************
*                   Miscellaneous function prototypes                   *
************************************************************************/

/*
  Function:   xmkdir   - Check and create directory with its parents
  Parameters: pathname - Directory to create
              mode     - Mode for any new directories
  Returns:    int      - 0 on success, -1 if an error occurred

  This function checks whether pathname exists and is a directory: if so,
  0 is returned.  Otherwise, it creates the directory and any parents
  that do not already exist.  If an error occurs, -1 is returned and
  errno is set appropriately.
*/
extern int xmkdir (const char *pathname, mode_t mode);


/*
  Function:   xmalloc - Allocate a new block of memory, with checking
  Parameters: size    - Size of new block of memory in bytes
  Returns:    void *  - Pointer to new block of memory

  This wrapper function allocates a new block of memory by calling
  malloc(), then checks if a NULL pointer has been returned.  If so, the
  program terminates with an "Out of memory" error.
*/
extern void *xmalloc (size_t size);


/*
  Function:   xstrdup - Duplicate a string, with checking
  Parameters: str     - String to duplicate
  Returns:    char *  - Pointer to new string, allocated with malloc()

  This wrapper function duplicates a string by calling strdup(), then
  checks if a NULL pointer has been returned.  If so, the program
  terminates with an "Out of memory" error.
*/
extern char *xstrdup (const char *str);


/*
  Function:   xchstrdup - Duplicate a chtype string
  Parameters: chstr     - String to duplicate
  Returns:    chtype *  - Pointer to new (duplicated) string

  This function returns a new string of type chtype * that contains a
  copy of the string in chstr.  No errors are returned: if sufficient
  memory is not available, the program terminates with an "Out of memory"
  message.
*/
extern chtype *xchstrdup (const chtype *restrict chstr);


/*
  Function:   xwcsdup   - Duplicate a wide-character string, with checking
  Parameters: str       - String to duplicate
  Returns:    wchar_t * - Pointer to new string, allocated with malloc()

  This wrapper function duplicates a string by calling wcsdup(), then
  checks if a NULL pointer has been returned.  If so, the program
  terminates with an "Out of memory" error.
*/
extern wchar_t *xwcsdup (const wchar_t *str);


/*
  Function:   xmbstowcs - Convert a multibyte string to a wide-character string
  Parameters: dest      - Location of wide-string buffer
              src       - String to convert
              len       - Size of dest, in multiples of wchar_t
  Returns:    size_t    - Number of characters placed in dest (excluding NUL)

  This wrapper function converts a multibyte string to a wide-character
  one by calling mbsrtowcs() continually until the whole string is
  converted.  If any illegal sequences are present, they are converted to
  the EILSEQ_REPL character.  If the destination buffer is too small, the
  string is truncated.
*/
extern size_t xmbstowcs (wchar_t *restrict dest, const char *restrict src,
			 size_t len);


/*
  Function:   xwcrtomb - Convert a wide character to a multibyte sequence
  Parameters: dest     - Location of multibyte buffer (size >= MB_CUR_MAX + 1)
              wc       - Character to convert
              mbstate  - Pointer to current multibyte shift state
  Returns:    size_t   - Number of characters placed in dest

  This wrapper function converts the wide character in wc (which may be
  NUL) by calling wcrtomb().  If wc cannot be represented in the current
  locale, EILSEQ_REPL is used instead (with any characters needed to move
  to an initial shift state prior to EILSEQ_REPL).
*/
extern size_t xwcrtomb (char *restrict dest, wchar_t wc,
			mbstate_t *restrict mbstate);


#endif /* included_UTILS_H */
