/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  Star Traders is a simple game of interstellar trading, where the object
  of the game is to create companies, buy and sell shares, borrow and
  repay money, in order to become the wealthiest player (the winner).

  This file, trader.c, contains the main program for Star Traders.


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
*                    Internal function declarations                     *
************************************************************************/

int main (int argc, char *argv[]);
static void process_cmdline (int argc, char *argv[]);
static void show_version (void);
static void show_usage (int status);


/************************************************************************
*                             Main program                              *
************************************************************************/

int main (int argc, char *argv[])
{
    init_program_name(argv);

    // Process command line arguments
    process_cmdline(argc, argv);


    // Testing...
    printf(_("Program name:   %s\n"), program_name());
    printf(_("Home directory: %s\n"), home_directory());
    printf(_("Data directory: %s\n"), data_directory());
    printf(_("Game filename:  %s\n"), game_filename);

    return EXIT_SUCCESS;
}


/************************************************************************
*                        Command line processing                        *
************************************************************************/

/* Constants for command line options */

static const char options_short[] = "hV";
    /* -h   --help
       -V   --version
    */

static struct option const options_long[] = {
    { "help",    no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
    { NULL,      0,           NULL, 0 }
};


/*-----------------------------------------------------------------------
  Function:   process_cmdline  - Process command line arguments
  Arguments:  argc             - Same as passed to main()
              argv             - Same as passed to main()
  Returns:    (nothing)

  This function processes the command line arguments passed through argc
  and argv, setting global variables as appropriate.
*/

static void process_cmdline (int argc, char *argv[])
{
    int c;

    // Process arguments starting with "-" or "--"
    opterr = TRUE;
    while (TRUE) {
	c = getopt_long(argc, argv, options_short, options_long, NULL);
	if (c == EOF)
	    break;

	switch (c) {
	case 'h':
	    /* -h, --help: show help */
	    show_usage(EXIT_SUCCESS);

	case 'V':
	    /* -V, --version: show version information */
	    show_version();

	default:
	    show_usage(EXIT_FAILURE);
	}
    }

    // Process remaining arguments

    if ((optind < argc) && (argv[optind] != NULL)) {
	if (argv[optind][0] == '-') {
	    fprintf(stderr, _("%s: invalid operand `%s'\n"), program_name(),
		    argv[optind]);
	    show_usage(EXIT_FAILURE);
	}

	game_filename = strto_game_filename(argv[optind]);
	optind++;
    }

    if ((optind < argc) && (argv[optind] != NULL)) {
	fprintf(stderr, _("%s: extra operand `%s'\n"), program_name(),
		argv[optind]);
	show_usage(EXIT_FAILURE);
    }
}


/*-----------------------------------------------------------------------
  Function:   show_version  - Show program version information
  Arguments:  (none)
  Returns:    (nothing)

  This function displays version information about this program, then
  terminates.
*/

static void show_version (void)
{
    printf(_("\
" PACKAGE_NAME " (%s) %s\n\
Copyright (C) %s, John Zaitseff.\n\
\n\
Star Traders is a simple game of interstellar trading, where the object\n\
of the game is to create companies, buy and sell shares, borrow and repay\n\
money, in order to become the wealthiest player (the winner).\n\
\n\
This program is free software that is distributed under the terms of the\n\
GNU General Public License, version 3 or later.  You are welcome to\n\
modify and/or distribute it under certain conditions.  This program has\n\
NO WARRANTY, to the extent permitted by law; see the License for details.\n\
"), program_name(), PACKAGE_VERSION, "1990-2011");

    exit(EXIT_SUCCESS);
}


/*-----------------------------------------------------------------------
  Function:   show_usage  - Show command line usage information
  Arguments:  status      - Exit status
  Returns:    (nothing)

  This function displays usage information to standard output or standard
  error, then terminates.
*/

static void show_usage (int status)
{
    if (status != EXIT_SUCCESS) {
	fprintf(stderr, _("%s: Try `%s --help' for more information.\n"),
		program_name(), program_name());
    } else {
	printf(_("Usage: %s [OPTION ...] [GAME]\n"), program_name());
	printf(_("\
Play Star Traders, a simple game of interstellar trading.\n\n\
"));
	printf(_("\
Options:\n\
  -V, --version   output version information and exit\n\
  -h, --help      display this help and exit\n\n\
"));
	printf(_("\
If GAME is specified as a number between 1 and 9, load and continue\n\
playing that game.  If GAME is not specified, start a new game.\n\n\
"));

#ifdef PACKAGE_AUTHOR
	printf(_("Report bugs to %s <%s>.\n"), PACKAGE_AUTHOR, PACKAGE_BUGREPORT);
#else
	printf(_("Report bugs to <%s>.\n"), PACKAGE_BUGREPORT);
#endif
#ifdef PACKAGE_PACKAGER_BUG_REPORTS
	printf(_("Report %s bugs to <%s>.\n"), PACKAGE_PACKAGER, PACKAGE_PACKAGER_BUG_REPORTS);
#endif
#ifdef PACKAGE_URL
	printf(_("Star Traders home page: <%s>.\n"), PACKAGE_URL);
#endif
    }

    exit(status);
}
