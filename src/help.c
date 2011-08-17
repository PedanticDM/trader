/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, help.c, contains the actual implementation of help functions
  as used in Star Traders.


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
*                         Help text definition                          *
************************************************************************/

static const char *help_text[] = {

  /*
    TRANSLATORS: The help text for Star Traders is marked up using a
    custom mark-up format NOT used anywhere else in the source code.

    Each string is a single page of text that is displayed in an area 76
    columns wide by 16 lines high.  Ideally, each line within the string
    should be (manually) space-justified or centred; each line is
    separated by "\n".  If a string starts with "@" as the very first
    character, that string (and all strings following) are ignored: this
    allows a variable number of help text pages (from one to twelve).

    The ASCII circumflex accent character "^" switches to a different
    character rendition (also called attributes), depending on the
    character following the "^":

    ^^ - Print the circumflex accent (ASCII code U+005E)
    ^N - Switch to using the normal character rendition
    ^B - Switch to using the bold character rendition
    ^B - Switch to using the highlight character rendition
    ^K - Switch to using the keycode character rendition (such as used for "<CTRL><C>")
    ^e - Switch to using the character rendition used for empty space
    ^o - Switch to using the character rendition used for outposts
    ^s - Switch to using the character rendition used for stars
    ^c - Switch to using the character rendition used for companies
    ^k - Switch to using the character rendition used for keyboard choices on the galaxy map

    The help text parsing routines also understand the following "value
    escapes" introduced by the ASCII tilde character "~"; these act like
    "%" conversion specifiers in printf():

    ~~       - Print the tilde character (ASCII code U+007E) [*]
    ~x       - Print the width of the galaxy map (MAX_X) [**]
    ~y       - Print the height of the galaxy map (MAX_Y) [**]
    ~m       - Print the number of moves available (NUMBER_MOVES) [**]
    ~c       - Print the maximum number of companies that can be formed (MAX_COMPANIES) [*]
    ~t       - Prints the default number of turns in the game (DEFAULT_MAX_TURN) [**]
    ~1 to ~9 - Print the keycode for the N-th choice of move, appropriately localised [*]
    ~M       - Print the keycode for the last choice of move [*]
    ~A to ~H - Print the character used to represent the company on the galaxy map, appropriately localised [*]

    [*]  Takes one character space in the output
    [**] Takes two character spaces in the output

    Note that the tilde value escapes do NOT change the current character
    rendition: a circumflex accent escape is needed for that.  For
    example, to display the first choice of move as it would be shown on
    the galaxy map, use something like "^k~1^N" (a six-character sequence
    that would translate to just one character in the output text).
  */
    N_(""
       "^BStar Traders^N  is a simple game  of  interstellar trading.  The object of the\n"
       "game is to amass  the greatest amount  of wealth  possible.  This is done by\n"
       "creating interstellar  shipping lanes, expanding them  and buying shares  in\n"
       "the companies  controlling  them.  Shares  appreciate  in value  as  company\n"
       "operations expand.  In addition, the return  on each share (as a percentage)\n"
       "also changes.  Players may also borrow from the Interstellar Trading Bank to\n"
       "finance additional purchases on the Stock Exchange.\n"
       "\n"
       "The map of the galaxy  is represented  by a ^B~x^N x ^B~y^N grid.  A typical section\n"
       "of it may be:\n"
       "\n"
       "        ^e . . ^s*^e . . . ^s*^e ^s*^e . ^N\n"
       "        ^e . . . . . . . . . ^N        ^e . ^N represents ^Bempty space^N,\n"
       "        ^e . ^s*^e . . . . . . . ^N        ^s * ^N represents a ^Bstar^N.\n"
       "        ^e . . . . . . . ^s*^e . ^N\n"
       "        ^e . . . . ^s*^e . . . . ^N\n"
       ""),

    N_(""
       "The computer selects ^B~m^N moves  (labeled ^k~1^N to ^k~M^N)  at random, and places these\n"
       "on the map.  To select  any of the highlighted positions, press that letter.\n"
       "As an example, some of the moves on the map may be:\n"
       "\n"
       "\n"
       "        ^e ^k~1^e . ^s*^e . . . ^s*^e ^s*^e . ^N\n"
       "        ^e . . . ^k~3^e . . . . . ^N\n"
       "        ^e . ^s*^e . . . . ^k~5^e . . ^N        Moves ^k~1^N to ^k~5^N shown.\n"
       "        ^e . ^k~2^e . . ^k~4^e . . ^s*^e . ^N\n"
       "        ^e . . . . ^s*^e . . . . ^N\n"
       "\n"
       "\n"
       "Selecting a position  that is  ^Bnot^N  next to a star (such as moves ^k~1^N, ^k~3^N or ^k~5^N)\n"
       "will set up  an ^Boutpost^N,  not belonging to any company.  Thus, if move ^k~3^N was\n"
       "selected on the above map, a ^o + ^N would be placed at that position.\n"
       ""),

    N_(""
       "If, on the other hand, a position  next to  a star  (or another outpost)  is\n"
       "selected, a ^Bcompany^N would be formed and its first letter would appear on the\n"
       "map.  As a reward  for creating the company, you are granted  the first five\n"
       "shares.  Up to ^B~c^N companies can be created in this way.\n"
       "\n"
       "If a position  next to  an existing company  is selected, the company  would\n"
       "expand its operations  by one square.  This increases the cost of its shares\n"
       "and hence  your return.  Thus,  if the map  was as shown below,  selecting ^k~6^N\n"
       "or ^k~8^N increases Company ^B~B^N's shipping lane:\n"
       "\n"
       "        ^e ^k~1^e . ^s*^e . . . ^s*^e ^s*^e . ^N\n"
       "        ^e . . . ^o+^e . . ^k~6^e . . ^N\n"
       "        ^e . ^s*^e . . . . ^c~B^e ^c~B^e ^c~B^e ^N        Move ^k~6^N or ^k~8^N increases Company ^B~B^N.\n"
       "        ^e . ^k~2^e . . ^k~4^e . . ^s*^e ^c~B^e ^N\n"
       "        ^e . . . . ^s*^e . . . ^k~8^e ^N\n"
       ""),

    N_(""
       "Selecting positions next to stars increases the value of your stock by about\n"
       "five times as much  as an extension  not next to a star.  Thus move ^k~6^N should\n"
       "be preferred to move ^k~8^N.\n"
       "\n"
       "        ^e ^c~C^e . ^s*^e . . . ^s*^e ^s*^e . ^N\n"
       "        ^e ^k~1^e ^o+^e . ^o+^e . . ^k~6^e . . ^N\n"
       "        ^e . ^s*^e . . . . ^c~B^e ^c~B^e ^c~B^e ^N        Move ^k~6^N is preferred to ^k~8^N.\n"
       "        ^e . ^k~2^e . . ^k~4^e . . ^s*^e ^c~B^e ^N\n"
       "        ^e . . . . ^s*^e . . . ^k~8^e ^N\n"
       "\n"
       "You can also expand  any company  by selecting positions  next to  outposts.\n"
       "Such outposts  will be swallowed up  by  that company.  Thus,  move  ^k~1^N  will\n"
       "extend  Company ^B~C^N by ^Btwo^N squares.  As a bonus,  outposts  next to  stars are\n"
       "more valuable:  the company's share price  will increase by a greater amount\n"
       "than it would for outposts not next to stars.\n"
       ""),

    N_(""
       "If two companies  are separated on the map by only one square, then they can\n"
       "be ^Bmerged^N into  one company  by selecting that position (if available).  For\n"
       "example, on the map below, companies ^B~A^N and ^B~B^N  can be merged  by selecting ^k~5^N.\n"
       "When this occurs, the company  with the greater assets value  takes over the\n"
       "other one.  Here, Company ^B~B^N might take over  Company ^B~A^N.  Company ^B~A^N ceases to\n"
       "exist, although it may reappear as an entirely new company at a later stage.\n"
       "\n"
       "        ^e ^k~1^e . ^s*^e . . . ^s*^e ^s*^e . ^N\n"
       "        ^e . . . ^c~A^e ^c~A^e ^k~5^e ^c~B^e . . ^N\n"
       "        ^e . ^s*^e . . ^c~A^e . ^c~B^e ^c~B^e ^c~B^e ^N        Move ^k~5^N merges companies ^B~A^N and ^B~B^N.\n"
       "        ^e . ^k~2^e . . . . . ^s*^e ^c~B^e ^N\n"
       "        ^e . . . . ^s*^e . ^o+^e . . ^N\n"
       "\n"
       "When  companies  merge, players are granted  shares in the  dominant company\n"
       "proportional to the amount  owned in the old company.  As well, a cash bonus\n"
       "is also paid, proportional to the percentage of the old company owned.\n"
       ""),

    N_(""
       "Once you select your move, you enter  the ^BInterstellar Stock Exchange^N.  Here\n"
       "you can  purchase shares,  sell them, borrow from  the Trading Bank or repay\n"
       "some of your debt (if applicable).  Note that each company  issues a limited\n"
       "number  of shares --- you cannot  go on buying for ever!  You can,  however,\n"
       "bid for more shares to be issued.  You have a better chance of succeeding if\n"
       "you own a larger proportion of the company.\n"
       "\n"
       "The game usually ends after ^B~t^N turns.  However, you can  end the game sooner\n"
       "by pressing  ^K<CTRL><C>^N  when asked  to select  a move.  As  well, individual\n"
       "players can declare themselves bankrupt at  any time.  If your debt is large\n"
       "enough, the Bank  may do this for you!  If you  do not complete your game in\n"
       "the time you have available, you may save the game and continue it later.\n"
       "\n"
       "\n"
       "The ^Bwinner of the game^N  is the person  with the greatest  net  worth  (total\n"
       "value of cash, stock and debt).  ^HGood luck^N and may the best person win!\n"
       ""),

#ifdef ENABLE_NLS
    N_("@ Help text, page 7"),
    N_("@ Help text, page 8"),
    N_("@ Help text, page 9"),
    N_("@ Help text, page 10"),
    N_("@ Help text, page 11"),
    N_("@ Help text, page 12"),
#endif

    NULL
};


/************************************************************************
*                     Help text function definition                     *
************************************************************************/

// This function is documented in the file "help.h"


/***********************************************************************/
// show_help: Show instructions on how to play the game

void show_help (void)
{
    int curpage = 0;
    int numpages = 0;
    bool done = false;


    // Count how many pages appear in the (translated) help text
    while (help_text[numpages] != NULL) {
	const char *s = gettext(help_text[numpages]);
	if (s == NULL || *s == '\0' || *s == '@')
	    break;
	numpages++;
    }

    if (numpages == 0)
	return;

    newtxwin(WIN_LINES - 1, WIN_COLS, 1, WCENTER, false, 0);

    while (! done) {
	// Display a page of instructions

	wbkgdset(curwin, attr_normal_window);
	werase(curwin);
	box(curwin, 0, 0);

	center(curwin, 1, 0, attr_title, 0, 0, 1, _("  How to Play  "));
	center(curwin, 2, 0, attr_normal, attr_highlight, 0, 1,
	       _("Page %d of %d"), curpage + 1, numpages);
	wmove(curwin, 4, 2);

	// Process the help text string

	const char *s = gettext(help_text[curpage]);
	int curattr = attr_normal;

	while (*s != '\0') {
	    switch (*s) {
	    case '\n':
		// Start a new line, suitably indented
		wmove(curwin, getcury(curwin) + 1, 2);
		break;

	    case '^':
		// Switch to a different character rendition
		switch (*++s) {
		case '^':
		    waddch(curwin, *s | curattr);
		    break;

		case 'N':
		    curattr = attr_normal;
		    wattrset(curwin, curattr);
		    break;

		case 'B':
		    curattr = attr_normal | A_BOLD;
		    wattrset(curwin, curattr);
		    break;

		case 'H':
		    curattr = attr_highlight;
		    wattrset(curwin, curattr);
		    break;

		case 'K':
		    curattr = attr_keycode;
		    wattrset(curwin, curattr);
		    break;

		case 'e':
		    curattr = attr_map_empty;
		    wattrset(curwin, curattr);
		    break;

		case 'o':
		    curattr = attr_map_outpost;
		    wattrset(curwin, curattr);
		    break;

		case 's':
		    curattr = attr_map_star;
		    wattrset(curwin, curattr);
		    break;

		case 'c':
		    curattr = attr_map_company;
		    wattrset(curwin, curattr);
		    break;

		case 'k':
		    curattr = attr_map_choice;
		    wattrset(curwin, curattr);
		    break;

		default:
		    waddch(curwin, '^' | curattr);
		    waddch(curwin, *s  | curattr);
		}
		break;

	    case '~':
		// Print a global constant
		switch (*++s) {
		case '~':
		    waddch(curwin, *s | curattr);
		    break;

		case 'x':
		    wprintw(curwin, "%2d", MAX_X);
		    break;

		case 'y':
		    wprintw(curwin, "%2d", MAX_Y);
		    break;

		case 'm':
		    wprintw(curwin, "%2d", NUMBER_MOVES);
		    break;

		case 'c':
		    wprintw(curwin, "%d", MAX_COMPANIES);
		    break;

		case 't':
		    wprintw(curwin, "%2d", DEFAULT_MAX_TURN);
		    break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		    // N-th choice of move, as a key press
		    wprintw(curwin, "%c", MOVE_TO_KEY(*s - '1'));
		    break;

		case 'M':
		    // Last choice of move, as a key press
		    wprintw(curwin, "%c", MOVE_TO_KEY(NUMBER_MOVES - 1));
		    break;

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		    // Map representation of company
		    wprintw(curwin, "%c",
			    PRINTABLE_MAP_VAL(COMPANY_TO_MAP(*s - 'A')));
		    break;

		default:
		    waddch(curwin, '~' | curattr);
		    waddch(curwin, *s  | curattr);
		}
		break;

	    default:
		// Print the character
		waddch(curwin, *s | curattr);
	    }

	    s++;
	}

	center(curwin, getmaxy(curwin) - 2, 0, attr_waitforkey, 0, 0, 1,
	       (curpage == 0) ? _("[ Press <SPACE> to continue ] ") :
	       /* TRANSLATORS: The specific use of <SPACE> and
		  <BACKSPACE> is not essential: you can use <DEL>,
		  <PAGE-UP>, <UP>, <LEFT> or <BACK-TAB> instead of
		  <BACKSPACE>, and almost any other key instead of
		  <SPACE> (other than <ESC>, <CANCEL>, <EXIT>, <CTRL><C>,
		  <CTRL><G> or <CTRL><\>). */
	       _("[ Press <SPACE> to continue or <BACKSPACE> "
		 "for the previous page ] "));
	wrefresh(curwin);

	int key = gettxchar(curwin);

	switch (key) {
	case KEY_BS:
	case KEY_BACKSPACE:
	case KEY_DEL:
	case KEY_PPAGE:
	case KEY_UP:
	case KEY_LEFT:
	case KEY_BTAB:
	    if (curpage == 0) {
		beep();
	    } else {
		curpage--;
	    }
	    break;

	case KEY_ESC:
	case KEY_CANCEL:
	case KEY_EXIT:
	case KEY_CTRL('C'):
	case KEY_CTRL('G'):
	case KEY_CTRL('\\'):
	    done = true;
	    break;

	default:
	    curpage++;
	    done = (curpage == numpages);
	}
    }

    deltxwin();
    txrefresh();
}
