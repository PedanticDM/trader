/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2021, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  Star Traders is a simple game of interstellar trading, where the object
  of the game is to create companies, buy and sell shares, borrow and
  repay money, in order to become the wealthiest player (the winner).

  This file, trader.c, contains the main program and command-line
  interface for Star Traders.


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
*                 Module-specific constant definitions                  *
************************************************************************/

// Constants for command line options

enum options_char {
    OPTION_NO_COLOR = 1,
    OPTION_DONT_ENCRYPT,
    OPTION_MAX_TURN
};

static const char options_short[] = "hV";
    // -h, --help
    // -V, --version

static struct option const options_long[] = {
    { "help",         no_argument,       NULL, 'h' },
    { "version",      no_argument,       NULL, 'V' },
    { "no-color",     no_argument,       NULL, OPTION_NO_COLOR },
    { "no-colour",    no_argument,       NULL, OPTION_NO_COLOR },
    { "dont-encrypt", no_argument,       NULL, OPTION_DONT_ENCRYPT },
    { "max-turn",     required_argument, NULL, OPTION_MAX_TURN },
    { NULL,           0,                 NULL, 0 }
};


/************************************************************************
*                  Module-specific function prototypes                  *
************************************************************************/

/*
  Function:   main - Main program implementing Star Traders
  Parameters: argc - Command-line argument count
              argv - Command-line argument vector
  Returns:    int  - Operating system return code: 0 if all well, 1 if not.

  The main() function is, of course, where all the action starts in a C
  program.  This function contains the main game loop, a series of game
  functions that are called in sequence.
*/
int main (int argc, char *argv[]);


/*
  Function:   process_cmdline - Process command line arguments
  Parameters: argc            - Same as passed to main()
              argv            - Same as passed to main()
  Returns:    (nothing)

  This function processes the command line arguments passed through argc
  and argv.  If required, it shows the program version number and/or
  command-line help.  It also sets global variables starting with option_
  to appropriate values.
*/
static void process_cmdline (int argc, char *argv[]);


/*
  Function:   show_version - Show program version information
  Parameters: (none)
  Returns:    (does not return)

  This function displays version information about this program, then
  terminates with exit code EXIT_SUCCESS.
*/
static void show_version (void) __attribute__((noreturn));


/*
  Function:   show_usage - Show command line usage information
  Parameters: status     - Exit status
  Returns:    (does not return)

  This function displays usage information for this program.  If status
  is zero, a detailed explanation is sent to stdout; otherwise, a brief
  message is sent to stderr.  It exits to the operating system with
  status as the exit code.
*/
static void show_usage (int status) __attribute__((noreturn));


/*
  Function:   init_program_prelim - Initialise program preliminaries
  Parameters: argc                - Same as passed to main()
              argv                - Same as passed to main()
  Returns:    (nothing)

  This function initialises the program name, locale and message
  catalogs.  It must be called before command line arguments are
  processed.
*/
static void init_program_prelim (int argc, char *argv[]);


/*
  Function:   init_program - Initialise program-wide functions
  Parameters: (none)
  Returns:    (nothing)

  This function initialises the terminal display, internal low-level
  routines, etc.  It should be called before the game starts.
*/
static void init_program (void);


/*
  Function:   end_program - Deinitialise program-wide functions
  Parameters: (none)
  Returns:    (nothing)

  This function finalises the terminal display, internal low-level
  routines, etc.  It should be the last function called in the ordinary
  course of program execution.
*/
static void end_program (void);


/************************************************************************
*                             Main program                              *
************************************************************************/

int main (int argc, char *argv[])
{
    // Initialise program name, locale and message catalogs
    init_program_prelim(argc, argv);

    // Process command line arguments
    process_cmdline(argc, argv);

    // Set up the display, internal low-level routines, etc.
    init_program();

    // Play the actual game
    init_game();
    while (! quit_selected && ! abort_game && turn_number <= max_turn) {
	selection_t selection;

	select_moves();
	selection = get_move();
	process_move(selection);
	exchange_stock();
	next_player();
    }
    end_game();

    // Finish up...
    end_program();
    return EXIT_SUCCESS;
}


/************************************************************************
*                        Command line processing                        *
************************************************************************/

// These functions are documented at the start of this file


/***********************************************************************/
// process_cmdline: Process command line arguments

void process_cmdline (int argc, char *argv[])
{
    // Process arguments starting with "-" or "--"
    opterr = true;
    while (true) {
	int c = getopt_long(argc, argv, options_short, options_long, NULL);
	if (c == EOF)
	    break;

	switch (c) {
	case 'h':
	    // -h, --help: show help
	    show_usage(EXIT_SUCCESS);
	    break;

	case 'V':
	    // -V, --version: show version information
	    show_version();
	    break;

	case OPTION_NO_COLOR:
	    // --no-color, --no-colour: don't use colour
	    option_no_color = true;
	    break;

	case OPTION_DONT_ENCRYPT:
	    // --dont-encrypt: don't encrypt game files
	    option_dont_encrypt = true;
	    break;

	case OPTION_MAX_TURN:
	    // --max-turn: specify the maximum turn number
	    {
		char *p;

		option_max_turn = strtol(optarg, &p, 10);

		if (option_max_turn < MIN_MAX_TURN || p == NULL || *p != '\0') {
		    fprintf(stderr, _("%s: invalid value for --max-turn: '%s'\n"),
			    program_name, optarg);
		    show_usage(EXIT_FAILURE);
		}
	    }
	    break;

	default:
	    show_usage(EXIT_FAILURE);
	}
    }

    // Process remaining arguments

    if (optind < argc && argv[optind] != NULL) {
	if (*argv[optind] == '-') {
	    fprintf(stderr, _("%s: invalid operand '%s'\n"),
		    program_name, argv[optind]);
	    show_usage(EXIT_FAILURE);
	}

	if (strlen(argv[optind]) == 1
	    && *argv[optind] >= '1' && *argv[optind] <= '9') {
	    game_num = *argv[optind] - '0';
	} else {
	    fprintf(stderr, _("%s: invalid game number '%s'\n"),
		    program_name, argv[optind]);
	    show_usage(EXIT_FAILURE);
	}

	optind++;
    }

    if (optind < argc && argv[optind] != NULL) {
	fprintf(stderr, _("%s: extra operand '%s'\n"),
		program_name, argv[optind]);
	show_usage(EXIT_FAILURE);
    }
}


/***********************************************************************/
// show_version: Show program version information

void show_version (void)
{
    /* TRANSLATORS: "John Zaitseff" [IPA d͡ʒɒn ˈzaɪ̯t͡səf] is the proper
       name of the author.  The IPA pronunciation in this comment is in
       UTF-8 encoding. */
    printf(_("\
Star Traders (%s) %s\n\
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
"), program_name, PACKAGE_VERSION, "1990-2021");

    exit(EXIT_SUCCESS);
}


/***********************************************************************/
// show_usage: Show command line usage information

void show_usage (int status)
{
    if (status != EXIT_SUCCESS) {
	fprintf(stderr, _("%s: Try '%s --help' for more information.\n"),
		program_name, program_name);
    } else {
	printf(_("Usage: %s [OPTION ...] [GAME]\n"), program_name);
	printf(_("\
Play Star Traders, a simple game of interstellar trading.\n\n\
"));
	printf(_("\
Options:\n\
  -V, --version        output version information and exit\n\
  -h, --help           display this help and exit\n\
      --no-color       don't use color for displaying text\n\
      --max-turn=NUM   set the number of turns to NUM\n\n\
"));
	printf(_("\
If GAME is specified as a number between 1 and 9, load and continue\n\
playing that game.  If GAME is not specified, start a new game.\n\n\
"));

#ifdef PACKAGE_AUTHOR
	/* TRANSLATORS: The first %s is the proper name of the package
	   author, John Zaitseff [IPA d͡ʒɒn ˈzaɪ̯t͡səf]; the second %s is
	   the email address for reporting bugs.  Please add ANOTHER line
	   with the (translated) text "Report translation bugs to
	   <ADDRESS>\n", with ADDRESS replaced with either an email
	   address or web URL for reporting bugs in your translation. */
	printf(_("Report bugs to %s <%s>.\n"), PACKAGE_AUTHOR, PACKAGE_BUGREPORT);
#else
	/* TRANSLATORS: %s is the email address for reporting bugs.  As
	   with the previous string, please add ANOTHER line with the
	   (translated) text "Report translation bugs to <ADDRESS>\n",
	   with ADDRESS replaced with either an email address or web URL
	   for reporting bugs in your translation. */
	printf(_("Report bugs to <%s>.\n"), PACKAGE_BUGREPORT);
#endif
#ifdef PACKAGE_PACKAGER_BUG_REPORTS
	/* TRANSLATORS: The first %s is for packagers and may be
	   something like "Debian". */
	printf(_("Report %s bugs to <%s>.\n"), PACKAGE_PACKAGER, PACKAGE_PACKAGER_BUG_REPORTS);
#endif
#ifdef PACKAGE_URL
	printf(_("Star Traders home page: <%s>.\n"), PACKAGE_URL);
#endif
    }

    exit(status);
}


/************************************************************************
*       Initialisation and deinitialisation function definitions        *
************************************************************************/

// These functions are documented at the start of this file


/***********************************************************************/
// init_program_prelim: Initialise program preliminaries

void init_program_prelim (int argc, char *argv[])
{
    // Strip off leading pathname components from program name
    init_program_name(argv[0]);

    // Initialise the locale and message catalogs
    init_locale();
}


/***********************************************************************/
// init_program: Initialise program-wide functions

void init_program (void)
{
    // Initialise the random number generator
    init_rand();

    // Initialise locale-specific variables
    init_locale_vars();

    // Initialise the terminal display
    init_screen();
}


/***********************************************************************/
// end_program: Deinitialise program-wide functions

void end_program (void)
{
    end_screen();
}


/***********************************************************************/
// End of file
