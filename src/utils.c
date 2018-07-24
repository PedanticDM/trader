/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2018, John Zaitseff                 *
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
  along with this program.  If not, see https://www.gnu.org/licenses/.
*/


#include "trader.h"


/************************************************************************
*                      Global variable definitions                      *
************************************************************************/

const char *program_name = NULL;	// Canonical program name


// Global copy, suitably modified, of localeconv() information
struct lconv lconvinfo;

// localeconv() information, converted to wide strings
wchar_t *decimal_point;			// Locale's radix character
wchar_t *thousands_sep;			// Locale's thousands separator
wchar_t *currency_symbol;		// Local currency symbol
wchar_t *mon_decimal_point;		// Local monetary radix character
wchar_t *mon_thousands_sep;		// Local monetary thousands separator


/************************************************************************
*                 Module-specific constants and macros                  *
************************************************************************/

#define GAME_FILENAME_PROTO	"game%d"
#define GAME_FILENAME_BUFSIZE	16

// Default values used to override POSIX locale
#define MOD_POSIX_CURRENCY_SYMBOL	"$"
#define MOD_POSIX_FRAC_DIGITS		2
#define MOD_POSIX_P_CS_PRECEDES		1
#define MOD_POSIX_P_SEP_BY_SPACE	0

// Constants used for scrambling and unscrambling game data
#define SCRAMBLE_CRC_LEN	8	// Length of CRC in ASCII (excl NUL)
#define SCRAMBLE_CHKSUM_LEN	3	// For checksum, excluding NUL byte
#define SCRAMBLE_CRC_MASK	0xFFFFFFFF	// Bits of CRC to keep
#define SCRAMBLE_CHKSUM_MASK	0x0FFF		// Bits of checksum to keep

#define SCRAMBLE_PAD_CHAR	'*'
#define SCRAMBLE_IGNORE_CHAR	'~'
#define UNSCRAMBLE_INVALID	(-1)
#define UNSCRAMBLE_IGNORE	(-2)
#define UNSCRAMBLE_PAD_CHAR	(-3)

static const char scramble_table[] =
    "0123456789AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz-_";

#define _b(n)								     \
    ((n) == '0' ? 0  : (n) == '1' ? 1  : (n) == '2' ? 2  : (n) == '3' ? 3  : \
     (n) == '4' ? 4  : (n) == '5' ? 5  : (n) == '6' ? 6  : (n) == '7' ? 7  : \
     (n) == '8' ? 8  : (n) == '9' ? 9  : (n) == 'A' ? 10 : (n) == 'a' ? 11 : \
     (n) == 'B' ? 12 : (n) == 'b' ? 13 : (n) == 'C' ? 14 : (n) == 'c' ? 15 : \
     (n) == 'D' ? 16 : (n) == 'd' ? 17 : (n) == 'E' ? 18 : (n) == 'e' ? 19 : \
     (n) == 'F' ? 20 : (n) == 'f' ? 21 : (n) == 'G' ? 22 : (n) == 'g' ? 23 : \
     (n) == 'H' ? 24 : (n) == 'h' ? 25 : (n) == 'I' ? 26 : (n) == 'i' ? 27 : \
     (n) == 'J' ? 28 : (n) == 'j' ? 29 : (n) == 'K' ? 30 : (n) == 'k' ? 31 : \
     (n) == 'L' ? 32 : (n) == 'l' ? 33 : (n) == 'M' ? 34 : (n) == 'm' ? 35 : \
     (n) == 'N' ? 36 : (n) == 'n' ? 37 : (n) == 'O' ? 38 : (n) == 'o' ? 39 : \
     (n) == 'P' ? 40 : (n) == 'p' ? 41 : (n) == 'Q' ? 42 : (n) == 'q' ? 43 : \
     (n) == 'R' ? 44 : (n) == 'r' ? 45 : (n) == 'S' ? 46 : (n) == 's' ? 47 : \
     (n) == 'T' ? 48 : (n) == 't' ? 49 : (n) == 'U' ? 50 : (n) == 'u' ? 51 : \
     (n) == 'V' ? 52 : (n) == 'v' ? 53 : (n) == 'W' ? 54 : (n) == 'w' ? 55 : \
     (n) == 'X' ? 56 : (n) == 'x' ? 57 : (n) == 'Y' ? 58 : (n) == 'y' ? 59 : \
     (n) == 'Z' ? 60 : (n) == 'z' ? 61 : (n) == '-' ? 62 : (n) == '_' ? 63 : \
     (n) == ' '  ? UNSCRAMBLE_IGNORE :					     \
     (n) == '\t' ? UNSCRAMBLE_IGNORE :					     \
     (n) == '\n' ? UNSCRAMBLE_IGNORE :					     \
     (n) == '\r' ? UNSCRAMBLE_IGNORE :					     \
     (n) == SCRAMBLE_IGNORE_CHAR ? UNSCRAMBLE_IGNORE :			     \
     (n) == SCRAMBLE_PAD_CHAR    ? UNSCRAMBLE_PAD_CHAR :		     \
     UNSCRAMBLE_INVALID)

static const signed char unscramble_table[] = {
    _b(0),   _b(1),   _b(2),   _b(3),   _b(4),   _b(5),   _b(6),   _b(7),
    _b(8),   _b(9),   _b(10) , _b(11),  _b(12),  _b(13),  _b(14),  _b(15),
    _b(16),  _b(17),  _b(18),  _b(19),  _b(20),  _b(21),  _b(22),  _b(23),
    _b(24),  _b(25),  _b(26),  _b(27),  _b(28),  _b(29),  _b(30),  _b(31),
    _b(32),  _b(33),  _b(34),  _b(35),  _b(36),  _b(37),  _b(38),  _b(39),
    _b(40),  _b(41),  _b(42),  _b(43),  _b(44),  _b(45),  _b(46),  _b(47),
    _b(48),  _b(49),  _b(50),  _b(51),  _b(52),  _b(53),  _b(54),  _b(55),
    _b(56),  _b(57),  _b(58),  _b(59),  _b(60),  _b(61),  _b(62),  _b(63),
    _b(64),  _b(65),  _b(66),  _b(67),  _b(68),  _b(69),  _b(70),  _b(71),
    _b(72),  _b(73),  _b(74),  _b(75),  _b(76),  _b(77),  _b(78),  _b(79),
    _b(80),  _b(81),  _b(82),  _b(83),  _b(84),  _b(85),  _b(86),  _b(87),
    _b(88),  _b(89),  _b(90),  _b(91),  _b(92),  _b(93),  _b(94),  _b(95),
    _b(96),  _b(97),  _b(98),  _b(99),  _b(100), _b(101), _b(102), _b(103),
    _b(104), _b(105), _b(106), _b(107), _b(108), _b(109), _b(110), _b(111),
    _b(112), _b(113), _b(114), _b(115), _b(116), _b(117), _b(118), _b(119),
    _b(120), _b(121), _b(122), _b(123), _b(124), _b(125), _b(126), _b(127),
    _b(128), _b(129), _b(130), _b(131), _b(132), _b(133), _b(134), _b(135),
    _b(136), _b(137), _b(138), _b(139), _b(140), _b(141), _b(142), _b(143),
    _b(144), _b(145), _b(146), _b(147), _b(148), _b(149), _b(150), _b(151),
    _b(152), _b(153), _b(154), _b(155), _b(156), _b(157), _b(158), _b(159),
    _b(160), _b(161), _b(162), _b(163), _b(164), _b(165), _b(166), _b(167),
    _b(168), _b(169), _b(170), _b(171), _b(172), _b(173), _b(174), _b(175),
    _b(176), _b(177), _b(178), _b(179), _b(180), _b(181), _b(182), _b(183),
    _b(184), _b(185), _b(186), _b(187), _b(188), _b(189), _b(190), _b(191),
    _b(192), _b(193), _b(194), _b(195), _b(196), _b(197), _b(198), _b(199),
    _b(200), _b(201), _b(202), _b(203), _b(204), _b(205), _b(206), _b(207),
    _b(208), _b(209), _b(210), _b(211), _b(212), _b(213), _b(214), _b(215),
    _b(216), _b(217), _b(218), _b(219), _b(220), _b(221), _b(222), _b(223),
    _b(224), _b(225), _b(226), _b(227), _b(228), _b(229), _b(230), _b(231),
    _b(232), _b(233), _b(234), _b(235), _b(236), _b(237), _b(238), _b(239),
    _b(240), _b(241), _b(242), _b(243), _b(244), _b(245), _b(246), _b(247),
    _b(248), _b(249), _b(250), _b(251), _b(252), _b(253), _b(254), _b(255)
};

#define UNSCRAMBLE_TABLE_SIZE (sizeof(unscramble_table) / sizeof(unscramble_table[0]))

static const unsigned char xor_table[] = {
    /* Set of bytes 0x00 to 0xFF in random order; each byte in an input
       string is XORed with successive bytes in this table. */
    0x00, 0xCE, 0xB1, 0x9F, 0xE4, 0xE0, 0xE3, 0x79,
    0xA1, 0x3B, 0x4E, 0x89, 0x81, 0x84, 0x43, 0xC8,
    0xBE, 0x0F, 0x67, 0x2A, 0xB4, 0xD8, 0xBA, 0x5D,
    0x94, 0x06, 0x69, 0x0E, 0x1C, 0x48, 0x9E, 0x0A,
    0x1D, 0x09, 0x02, 0xCD, 0xD4, 0xF6, 0x5B, 0x8A,
    0xAE, 0x65, 0xB3, 0xB5, 0xA7, 0x13, 0x03, 0xF2,
    0x42, 0xF0, 0xA6, 0xAA, 0x35, 0xCB, 0x2C, 0x55,
    0xF5, 0xC7, 0x32, 0xB7, 0x6B, 0xEA, 0xC3, 0x6F,
    0x41, 0xFF, 0xD1, 0x24, 0x54, 0xA9, 0xC6, 0xC2,
    0x74, 0xEE, 0xBC, 0x99, 0x59, 0x71, 0x3D, 0x85,
    0x0B, 0xF7, 0x3A, 0x7E, 0xDB, 0x45, 0xE8, 0x96,
    0xD0, 0xC1, 0xE6, 0xFD, 0x86, 0x8C, 0x9B, 0x0C,
    0x66, 0x5F, 0xE5, 0x14, 0x98, 0x3C, 0xBD, 0xE2,
    0x88, 0xA3, 0x30, 0x38, 0x2F, 0xA2, 0x37, 0x70,
    0xB8, 0x11, 0x61, 0x93, 0x52, 0x1B, 0xDD, 0x20,
    0x60, 0x19, 0xEF, 0xD2, 0xEC, 0x73, 0x07, 0x92,
    0x4C, 0x6A, 0xA8, 0x9D, 0x34, 0x04, 0x87, 0x2E,
    0x1E, 0xA4, 0xCA, 0x72, 0x63, 0xD7, 0x7F, 0xFB,
    0x68, 0xE1, 0xBF, 0x10, 0x8E, 0xAF, 0x9A, 0xFA,
    0xA0, 0xDE, 0x1F, 0x31, 0x15, 0x97, 0xED, 0x2B,
    0x36, 0x8D, 0x12, 0xC5, 0x23, 0x95, 0x33, 0x56,
    0x4F, 0xE7, 0xAD, 0x5C, 0x4B, 0x83, 0xDC, 0x29,
    0xE9, 0xCF, 0x8F, 0x58, 0x4D, 0x5A, 0x08, 0x49,
    0xFC, 0x6D, 0x7C, 0xB6, 0xD3, 0x7B, 0xD6, 0x53,
    0x57, 0x82, 0x0D, 0xD9, 0x7D, 0xDA, 0x4A, 0xDF,
    0x27, 0x40, 0x1A, 0x22, 0xC9, 0x51, 0x3E, 0x6C,
    0xC4, 0x18, 0xCC, 0xAC, 0xEB, 0xA5, 0xF4, 0x44,
    0xFE, 0x76, 0xF8, 0x75, 0xF3, 0x2D, 0xB0, 0xB9,
    0x9C, 0x47, 0x7A, 0x28, 0xBB, 0xF1, 0x16, 0x64,
    0x46, 0x21, 0x78, 0x90, 0xD5, 0x80, 0x3F, 0x39,
    0x25, 0xB2, 0x6E, 0x8B, 0x77, 0xC0, 0x05, 0x50,
    0x17, 0xF9, 0x01, 0x26, 0x91, 0x5E, 0x62, 0xAB
};

#define XOR_TABLE_SIZE (sizeof(xor_table) / sizeof(xor_table[0]))


/************************************************************************
*                       Module-specific variables                       *
************************************************************************/

static char *home_directory_str = NULL;		// Full pathname to home
static char *data_directory_str = NULL;		// Writable data dir pathname

static bool add_currency_symbol = false;	// Do we need to add "$"?


/************************************************************************
*                  Module-specific function prototypes                  *
************************************************************************/

/*
  Function:   apply_xor - Scramble a buffer using xor_table
  Parameters: dest      - Location of destination buffer
              src       - Location of source buffer
              n         - Number of bytes to scramble
              key       - Pointer to xor_table index
  Returns:    (nothing)

  This function copies n bytes from *src into *dest, applying a XOR with
  the contents of xor_table in the process.  It is a reversable function:
  apply_xor(apply_xor(buffer)) == buffer.  It is used by both scramble()
  and unscramble().
*/
static void apply_xor (void *restrict dest, const void *restrict src,
		       size_t n, unsigned int *restrict key);


/*
  Function:   b64encode - Convert a block to non-standard Base64 encoding
  Parameters: in        - Location of input buffer
              inlen     - Size of input buffer
              out       - Location of output buffer
              outlen    - Size of output buffer
  Returns:    size_t    - Number of bytes placed in output buffer

  This function encodes inlen bytes in the input buffer into the output
  buffer using a non-standard Base64 encoding (as contained above in
  scramble_table[]).  The resulting encoded string length is returned
  (including trailing '\n' but NOT including trailing NUL).

  Note that the output buffer must be at least 4/3 the size of the input
  buffer; if not, an assert is generated.

  This function is used by scramble().
*/
static size_t b64encode (const void *restrict in, size_t inlen,
			 void *restrict out, size_t outlen);


/*
  Function:   b64decode - Convert a block from non-standard Base64 encoding
  Parameters: in        - Location of input buffer
              inlen     - Size of input buffer
              out       - Location of output buffer
              outlen    - Size of output buffer
  Returns:    ssize_t   - Number of bytes placed in output buffer, or -1

  This function decodes up to inlen bytes in the input buffer into the
  output buffer using a non-standard Base64 encoding (as contained above
  in unscramble_table[]).  The resulting decoded buffer length is
  returned; that buffer may contain NUL bytes.  If an error occurs during
  decoding, -1 is returned instead.

  This function is used by unscramble().
*/
static ssize_t b64decode (const void *restrict in, size_t inlen,
			  void *restrict out, size_t outlen);


/************************************************************************
*          Initialisation and environment function definitions          *
************************************************************************/

// These functions are documented in the file "utils.h"


/***********************************************************************/
// init_program_name: Make the program name canonical

void init_program_name (const char *argv0)
{
    /* This implementation assumes a POSIX environment with an ASCII-safe
       character encoding (such as ASCII or UTF-8). */

    if (argv0 == NULL || *argv0 == '\0') {
	program_name = PACKAGE;
    } else {
	char *p = strrchr(argv0, '/');

	if (p != NULL && *++p != '\0') {
	    program_name = xstrdup(p);
	} else {
	    program_name = xstrdup(argv0);
	}
    }
}


/***********************************************************************/
// home_directory: Return home directory pathname

const char *home_directory (void)
{
    if (home_directory_str == NULL) {
	// Use the HOME environment variable where possible
	const char *home = getenv("HOME");

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
	const char *home = home_directory();

	if (program_name != NULL && home != NULL) {
	    char *p = xmalloc(strlen(home) + strlen(program_name) + 3);

	    strcpy(p, home);
	    strcat(p, "/.");
	    strcat(p, program_name);
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

    fprintf(stderr, _("%s: "), program_name);
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

    fprintf(stderr, _("%s: "), program_name);
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
    wchar_t *buf;


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

    // Convert localeconv() information to wide strings

    buf = xmalloc(BUFSIZE * sizeof(wchar_t));

    xmbstowcs(buf, lconvinfo.decimal_point, BUFSIZE);
    decimal_point = xwcsdup(buf);

    xmbstowcs(buf, lconvinfo.thousands_sep, BUFSIZE);
    thousands_sep = xwcsdup(buf);

    xmbstowcs(buf, lconvinfo.currency_symbol, BUFSIZE);
    currency_symbol = xwcsdup(buf);

    xmbstowcs(buf, lconvinfo.mon_decimal_point, BUFSIZE);
    mon_decimal_point = xwcsdup(buf);

    xmbstowcs(buf, lconvinfo.mon_thousands_sep, BUFSIZE);
    mon_thousands_sep = xwcsdup(buf);

    free(buf);

    setlocale(LC_MONETARY, cur);
    free(cur);
}


/***********************************************************************/
// l_strfmon: Convert monetary value to a string

ssize_t l_strfmon (char *restrict buf, size_t maxsize,
		   const char *restrict format, double val)
{
    /* The current implementation assumes MOD_POSIX_P_CS_PRECEDES is 1
       (currency symbol precedes value) and that MOD_POSIX_P_SEP_BY_SPACE
       is 0 (no space separates currency symbol and value).  It does,
       however, handle currency symbols of length > 1. */

    assert(MOD_POSIX_P_CS_PRECEDES  == 1);
    assert(MOD_POSIX_P_SEP_BY_SPACE == 0);

    ssize_t ret = strfmon(buf, maxsize, format, val);

    if (ret > 0 && add_currency_symbol) {
	if (strstr(format, "!") == NULL) {
	    /* Insert lconvinfo.currency_symbol to s.

	       NB: add_currency_symbol == true assumes a POSIX locale and
	       that the character encoding is ASCII-safe (such as by
	       being ASCII itself, or UTF-8). */
	    const char *sym = lconvinfo.currency_symbol;
	    int symlen = strlen(sym);
	    char *p;
	    int spc;

	    assert(maxsize > (unsigned int) symlen);

	    // Count number of leading spaces
	    for (p = buf, spc = 0; *p == ' '; p++, spc++)
		;

	    if (symlen <= spc) {
		/* Enough space for currency symbol: copy it WITHOUT
		   copying terminating NUL character */
		for (p -= symlen; *sym != '\0'; p++, sym++) {
		    *p = *sym;
		}
	    } else {
		// Make space for currency symbol, then copy it

		memmove(buf + symlen - spc, buf, maxsize - (symlen - spc));
		buf[maxsize - 1] = '\0';

		for ( ; *sym != '\0'; sym++, buf++) {
		    // Make sure terminating NUL character is NOT copied!
		    *buf = *sym;
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

/* These functions are documented in the file "utils.h" or in the
   comments above. */


/***********************************************************************/
// scramble: Scramble (encrypt) the buffer

char *scramble (char *restrict dest, const char *restrict src,
		size_t size, unsigned int *restrict key)
{
    unsigned long int crc;
    unsigned int chksum;
    size_t srclen;
    char *xorbuf, *midxor;
    char *middest;
    char crcbuf[SCRAMBLE_CRC_LEN + 1];
    char chksumbuf[SCRAMBLE_CHKSUM_LEN + 1];


    assert(dest != NULL);
    assert(src != NULL);
    assert(size > 0);

    srclen = strlen(src);

    if (key == NULL) {
	// No encryption required
	assert(size >= srclen + 2);		// Enough room to add "\n"?

	strcpy(dest, src);

	// Add "\n" if needed
	if (dest[srclen - 1] != '\n') {
	    dest[srclen] = '\n';
	    dest[srclen + 1] = '\0';
	}
    } else {
	// Scramble the input

	xorbuf = xmalloc(srclen + SCRAMBLE_CRC_LEN + 1);

	// Scramble src using *key, leaving room for CRC32 in front
	midxor = xorbuf + SCRAMBLE_CRC_LEN;
	apply_xor(midxor, src, srclen, key);

	// Calculate CRC32 checksum of XORed buffer
	crc = crc32(midxor, srclen) & SCRAMBLE_CRC_MASK;
	snprintf(crcbuf, SCRAMBLE_CRC_LEN + 1, "%08lx", crc);
	memcpy(xorbuf, crcbuf, SCRAMBLE_CRC_LEN);

	// Encode whole buffer (including CRC32) using Base64
	middest = dest + SCRAMBLE_CHKSUM_LEN;
	b64encode(xorbuf, srclen + SCRAMBLE_CRC_LEN,
		  middest, size - SCRAMBLE_CHKSUM_LEN);

	// Calculate simple checksum
	chksum = 0;
	for (char *p = middest; *p != '\0' && *p != '\n'; p++) {
	    chksum += *p;
	}
	chksum &= SCRAMBLE_CHKSUM_MASK;

	// Place checksum in front of Base64 string
	snprintf(chksumbuf, SCRAMBLE_CHKSUM_LEN + 1, "%03x", chksum);
	memcpy(dest, chksumbuf, SCRAMBLE_CHKSUM_LEN);

	free(xorbuf);
    }

    return dest;
}


/***********************************************************************/
// unscramble: Unscramble (decrypt) the buffer

char *unscramble (char *restrict dest, const char *restrict src,
		  size_t size, unsigned int *restrict key)
{
    unsigned long int crc, crc_input;
    unsigned int chksum, chksum_input;
    size_t srclen;
    char *xorbuf, *midxor;
    ssize_t xorlen;
    const char *midsrc;
    char crcbuf[SCRAMBLE_CRC_LEN + 2];		// Leave room for '\n\0'
    char chksumbuf[SCRAMBLE_CHKSUM_LEN + 2];


    assert(dest != NULL);
    assert(src != NULL);
    assert(size > 0);

    srclen = strlen(src);

    if (key == NULL) {
	// No decryption required
	assert(size >= srclen + 1);
	strcpy(dest, src);
    } else {
	// Unscramble the input

	// Copy out simple checksum from input
	memcpy(chksumbuf, src, SCRAMBLE_CHKSUM_LEN);
	chksumbuf[SCRAMBLE_CHKSUM_LEN] = '\n';
	chksumbuf[SCRAMBLE_CHKSUM_LEN + 1] = '\0';
	if (sscanf(chksumbuf, "%x\n", &chksum_input) != 1) {
	    return NULL;
	}

	// Calculate and compare checksums
	midsrc = src + SCRAMBLE_CHKSUM_LEN;
	chksum = 0;
	for (const char *p = midsrc; *p != '\0' && *p != '\n'; p++) {
	    chksum += *p;
	}
	chksum &= SCRAMBLE_CHKSUM_MASK;

	if (chksum != chksum_input) {
	    return NULL;
	}

	xorbuf = xmalloc(size + SCRAMBLE_CRC_LEN);

	// Decode buffer sans checksum using Base64
	xorlen = b64decode(midsrc, srclen - SCRAMBLE_CHKSUM_LEN,
			   xorbuf, size + SCRAMBLE_CRC_LEN);
	if (xorlen < SCRAMBLE_CRC_LEN) {
	    free(xorbuf);
	    return NULL;
	}

	// Copy out CRC32 checksum
	memcpy(crcbuf, xorbuf, SCRAMBLE_CRC_LEN);
	crcbuf[SCRAMBLE_CRC_LEN] = '\n';
	crcbuf[SCRAMBLE_CRC_LEN + 1] = '\0';
	if (sscanf(crcbuf, "%lx\n", &crc_input) != 1) {
	    free(xorbuf);
	    return NULL;
	}

	// Calculate and compare CRC32 checksums
	midxor = xorbuf + SCRAMBLE_CRC_LEN;
	crc = crc32(midxor, xorlen - SCRAMBLE_CRC_LEN) & SCRAMBLE_CRC_MASK;
	if (crc != crc_input) {
	    free(xorbuf);
	    return NULL;
	}

	// Descramble xorbuf using *key, ignoring CRC32 in front
	apply_xor(dest, midxor, xorlen - SCRAMBLE_CRC_LEN, key);

	// Convert the output to a C string
	assert(size >= xorlen - SCRAMBLE_CRC_LEN + 1);
	dest[xorlen - SCRAMBLE_CRC_LEN] = '\0';

	free(xorbuf);
    }

    return dest;
}


/***********************************************************************/
// apply_xor: Scramble a buffer using xor_table

void apply_xor (void *restrict dest, const void *restrict src,
		size_t n, unsigned int *restrict key)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(key != NULL);
    assert(*key < XOR_TABLE_SIZE);

    for (size_t i = 0; i < n; i++, dest++, src++) {
	*(unsigned char *) dest = *(unsigned char *) src ^ xor_table[*key];
	*key = (*key + 1) % XOR_TABLE_SIZE;
    }
}


/***********************************************************************/
// b64encode: Convert a block to non-standard Base64 encoding

size_t b64encode (const void *restrict in, size_t inlen,
		  void *restrict out, size_t outlen)
{
    size_t count;
    size_t padding;

    // Note that bit manipulations on strings require unsigned char!
    const unsigned char *u_in = in;
    unsigned char *u_out      = out;


    assert(u_in != NULL);
    assert(u_out != NULL);
    assert(outlen > 0);
    assert(outlen > inlen);

    count = 0;
    padding = inlen % 3;

    for (size_t i = 0; i < inlen; i += 3, u_in += 3) {
	unsigned long int n;
	unsigned char n0, n1, n2, n3;

	// Convert three input bytes into a 24-bit number
	n = u_in[0] << 16;
	if (i + 1 < inlen) {
	    n += u_in[1] << 8;
	}
	if (i + 2 < inlen) {
	    n += u_in[2];
	}

	// Convert the 24-bit number into four Base64 bytes
	n0 = (unsigned char) (n >> 18) & 0x3F;
	n1 = (unsigned char) (n >> 12) & 0x3F;
	n2 = (unsigned char) (n >> 6) & 0x3F;
	n3 = (unsigned char) n & 0x3F;

	assert(count + 3 < outlen);

	*u_out++ = scramble_table[n0];
	*u_out++ = scramble_table[n1];
	count += 2;

	if (i + 1 < inlen) {
	    *u_out++ = scramble_table[n2];
	    count++;
	}
	if (i + 2 < inlen) {
	    *u_out++ = scramble_table[n3];
	    count++;
	}
    }

    if (padding > 0) {
	assert(count + 2 < outlen);
	for (; padding < 3; padding++) {
	    *u_out++ = SCRAMBLE_PAD_CHAR;
	    count++;
	}
    }

    assert(count + 2 <= outlen);

    *u_out++ = '\n';
    *u_out = '\0';
    count++;

    return count;
}


/***********************************************************************/
// b64decode: Convert a block from non-standard Base64 encoding

ssize_t b64decode (const void *restrict in, size_t inlen,
		   void *restrict out, size_t outlen)
{
    size_t count;
    unsigned long int n;

    // Note that bit manipulations on strings require unsigned char!
    // Using char * results in very subtle bugs indeed...
    const unsigned char *u_in = in;
    unsigned char *u_out      = out;


    assert(u_in != NULL);
    assert(u_out != NULL);
    assert(outlen > 0);

    count = 0;
    n = 1;

    for (size_t i = 0; i < inlen && *u_in != '\0'; i++, u_in++) {
	int v = *u_in > UNSCRAMBLE_TABLE_SIZE ?
	    UNSCRAMBLE_INVALID : unscramble_table[*u_in];

	switch (v) {
	case UNSCRAMBLE_INVALID:
	    return -1;

	case UNSCRAMBLE_IGNORE:
	    continue;

	case UNSCRAMBLE_PAD_CHAR:
	    // Assume end of data
	    i = inlen;
	    continue;

	default:
	    n = n << 6 | v;			// v is 0 .. 63

	    if (n & 0x1000000) {
		// Convert 24-bit number into three output bytes
		count += 3;
		if (count > outlen) {
		    return -1;
		}

		*u_out++ = n >> 16;
		*u_out++ = n >> 8;
		*u_out++ = n;
		n = 1;
	    }
	}
    }

    if (n & 0x40000) {
	count += 2;
	if (count > outlen) {
	    return -1;
	}

	*u_out++ = n >> 10;
	*u_out++ = n >> 2;
    } else if (n & 0x1000) {
	count += 1;
	if (count > outlen) {
	    return -1;
	}

	*u_out++ = n >> 4;
    }

    return count;
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
		errno_exit(_("xmbstowcs: '%s'"), src);
	    }
	} else if (p != NULL) {
	    // Multibyte string was too long: truncate dest
	    dest[len - 1] = L'\0';
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
	    if ((n = wcrtomb(dest, L'\0', &mbcopy)) == (size_t) -1) {
		errno_exit(_("xwcrtomb: NUL"));
	    }
	    dest[n] = EILSEQ_REPL;
	    dest[n++] = '\0';
	} else {
	    errno_exit(_("xwcrtomb: '%lc'"), (wint_t) wc);
	}
    }

    memcpy(mbstate, &mbcopy, sizeof(mbcopy));
    return n;
}


/***********************************************************************/
// End of file
