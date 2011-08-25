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

#define HELP_TEXT_PAGES	10

static const char *help_text[HELP_TEXT_PAGES] = {

  /*
    TRANSLATORS: The help text for Star Traders is marked up using a
    custom mark-up format NOT used anywhere else in the source code.

    Each string is a single page of text that is displayed in an area 76
    columns wide by 16 lines high.  Each line is delimited by "\n".  NO
    word-wrapping is performed: you must place the "\n" characters in the
    appropriate place.  Ideally, each line within the string should be
    also (manually) space-justified or centred.  TAB characters and other
    control codes must NOT be used.  If a string starts with "@" as the
    very first character, that string is ignored (as are all strings
    following): this allows a variable number of help text pages (from
    one to ten).  Multibyte strings are handled correctly (even those
    requiring shift sequences!).

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
    ~1 to ~9 - Print the keycode for the N-th choice of move [***]
    ~M       - Print the keycode for the last choice of move [***]
    ~A to ~H - Print the character used to represent the company on the galaxy map [***]
    ~.       - Print the character used to represent empty space on the map [***]
    ~+       - Print the character used to represent outposts on the map [***]
    ~*       - Print the character used to represent stars on the map [***]

    [*]   Takes one character space (column space) in the output
    [**]  Takes two column spaces in the output
    [***] Takes one or two column spaces in the output, depending on the
          appropriate strings in the current PO file.

    Note that all keycodes and map representation characters use locale-
    specific characters; double-width characters ARE supported.  Note
    also that the tilde value escapes do NOT change the current character
    rendition: a circumflex accent escape is needed for that.  For
    example, to display the first choice of move as it would be shown on
    the galaxy map, use something like "^k~1^N" (a six-character sequence
    that would translate to just one character (or maybe two) in the
    output text).
  */
    N_( ""
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
	"        ^e ~. ~. ^s~*^e ~. ~. ~. ^s~*^e ^s~*^e ~. ^N\n"
	"        ^e ~. ~. ~. ~. ~. ~. ~. ~. ~. ^N        ^e ~. ^N represents ^Bempty space^N,\n"
	"        ^e ~. ^s~*^e ~. ~. ~. ~. ~. ~. ~. ^N        ^s ~* ^N represents a ^Bstar^N.\n"
	"        ^e ~. ~. ~. ~. ~. ~. ~. ^s~*^e ~. ^N\n"
	"        ^e ~. ~. ~. ~. ^s~*^e ~. ~. ~. ~. ^N\n"
	""),

    N_( ""
	"The computer selects ^B~m^N moves  (labeled ^k~1^N to ^k~M^N)  at random, and places these\n"
	"on the map.  To select  any of the highlighted positions, press that letter.\n"
	"For example, some of the moves on the map may be:\n"
	"\n"
	"\n"
	"        ^e ^k~1^e ~. ^s~*^e ~. ~. ~. ^s~*^e ^s~*^e ~. ^N\n"
	"        ^e ~. ~. ~. ^k~3^e ~. ~. ~. ~. ~. ^N\n"
	"        ^e ~. ^s~*^e ~. ~. ~. ~. ^k~5^e ~. ~. ^N        Moves ^k~1^N to ^k~5^N shown.\n"
	"        ^e ~. ^k~2^e ~. ~. ^k~4^e ~. ~. ^s~*^e ~. ^N\n"
	"        ^e ~. ~. ~. ~. ^s~*^e ~. ~. ~. ~. ^N\n"
	"\n"
	"\n"
	"Selecting a position  that is  ^Bnot^N  next to a star (such as moves ^k~1^N, ^k~3^N or ^k~5^N)\n"
	"will set up  an ^Boutpost^N,  not belonging  to any company.  Thus, if move ^k~3^N is\n"
	"selected on the above map, a ^o ~+ ^N would be placed at that position.\n"
	""),

    N_( ""
	"If, on the other hand, a position  next to  a star  (or another outpost)  is\n"
	"selected, a ^Bcompany^N would be formed  and its letter would appear on the map.\n"
	"As a reward for creating the company, you are granted the first five shares.\n"
	"Up to ^B~c^N companies can be created in this way.\n"
	"\n"
	"If a position  next to  an existing company  is selected, the company  would\n"
	"expand its operations  by one square.  This increases the cost of its shares\n"
	"and hence  your return.  Thus,  if the map  was as shown below,  selecting ^k~6^N\n"
	"or ^k~8^N increases Company ^B~B^N's shipping lane:\n"
	"\n"
	"        ^e ^k~1^e ~. ^s~*^e ~. ~. ~. ^s~*^e ^s~*^e ~. ^N\n"
	"        ^e ~. ~. ~. ^o~+^e ~. ~. ^k~6^e ~. ~. ^N\n"
	"        ^e ~. ^s~*^e ~. ~. ~. ~. ^c~B^e ^c~B^e ^c~B^e ^N        Move ^k~6^N or ^k~8^N increases Company ^B~B^N.\n"
	"        ^e ~. ^k~2^e ~. ~. ^k~4^e ~. ~. ^s~*^e ^c~B^e ^N\n"
	"        ^e ~. ~. ~. ~. ^s~*^e ~. ~. ~. ^k~8^e ^N\n"
	""),

    N_( ""
	"Selecting positions next to stars increases the value of your stock by about\n"
	"five times as much  as an extension  not next to a star.  Thus move ^k~6^N should\n"
	"be preferred to move ^k~8^N.\n"
	"\n"
	"        ^e ^c~C^e ~. ^s~*^e ~. ~. ~. ^s~*^e ^s~*^e ~. ^N\n"
	"        ^e ^k~1^e ^o~+^e ~. ^o~+^e ~. ~. ^k~6^e ~. ~. ^N\n"
	"        ^e ~. ^s~*^e ~. ~. ~. ~. ^c~B^e ^c~B^e ^c~B^e ^N        Move ^k~6^N is preferred to ^k~8^N.\n"
	"        ^e ~. ^k~2^e ~. ~. ^k~4^e ~. ~. ^s~*^e ^c~B^e ^N\n"
	"        ^e ~. ~. ~. ~. ^s~*^e ~. ~. ~. ^k~8^e ^N\n"
	"\n"
	"You may also expand  any company  by selecting positions  next to  outposts.\n"
	"Such outposts  will be swallowed up  by  that company.  Thus,  move  ^k~1^N  will\n"
	"extend  Company ^B~C^N by ^Btwo^N squares.  As a bonus,  outposts  next to  stars are\n"
	"more valuable:  the company's share price  will increase by a greater amount\n"
	"than it would for outposts not next to stars.\n"
	""),

    N_( ""
	"If two companies  are separated on the map by only one square, then they can\n"
	"be ^Bmerged^N into  one company  by selecting that position (if available).  For\n"
	"example, on the map below, companies ^B~A^N and ^B~B^N  can be merged  by selecting ^k~5^N.\n"
	"When this occurs, the company  with the greater assets value  takes over the\n"
	"other one.  Here, Company ^B~B^N might take over  Company ^B~A^N.  Company ^B~A^N ceases to\n"
	"exist, although it may reappear as an entirely new company at a later stage.\n"
	"\n"
	"        ^e ^k~1^e ~. ^s~*^e ~. ~. ~. ^s~*^e ^s~*^e ~. ^N\n"
	"        ^e ~. ~. ~. ^c~A^e ^c~A^e ^k~5^e ^c~B^e ~. ~. ^N\n"
	"        ^e ~. ^s~*^e ~. ~. ^c~A^e ~. ^c~B^e ^c~B^e ^c~B^e ^N        Move ^k~5^N merges companies ^B~A^N and ^B~B^N.\n"
	"        ^e ~. ^k~2^e ~. ~. ~. ~. ~. ^s~*^e ^c~B^e ^N\n"
	"        ^e ~. ~. ~. ~. ^s~*^e ~. ^o~+^e ~. ~. ^N\n"
	"\n"
	"When  companies  merge, players are granted  shares in the  dominant company\n"
	"proportional to the amount  owned in the old company.  As well, a cash bonus\n"
	"is also paid, proportional to the percentage of the old company owned.\n"
	""),

    N_( ""
	"Once you select your move, you enter  the ^BInterstellar Stock Exchange^N.  Here\n"
	"you may  purchase shares,  sell them, borrow from  the Trading Bank or repay\n"
	"some of your debt (if applicable).  Note that each company  issues a limited\n"
	"number of shares -- you cannot go on buying for ever!  You may, however, bid\n"
	"for more shares to be issued.  You have a better chance of succeeding if you\n"
	"own a larger proportion of the company.\n"
	"\n"
	"The game usually ends after ^B~t^N turns.  However, you may  end the game sooner\n"
	"by pressing  ^K<CTRL><C>^N  when asked  to select  a move.  As  well, individual\n"
	"players can declare themselves bankrupt at  any time.  If your debt is large\n"
	"enough, the Bank  may do this for you!  If you  do not complete your game in\n"
	"the time you have available, you may save the game and continue it later.\n"
	"\n"
	"\n"
	"The ^Bwinner of the game^N  is the person  with the greatest  net  worth  (total\n"
	"value of cash, stock and debt).  ^HGood luck^N and may the best person win!\n"
	"")

#ifdef ENABLE_NLS
    , N_("@ Help text, page 7")
    , N_("@ Help text, page 8")
    , N_("@ Help text, page 9")
    , N_("@ Help text, page 10")
#endif
};


/************************************************************************
*                     Help text function definition                     *
************************************************************************/

// This function is documented in the file "help.h"


/***********************************************************************/
// show_help: Show instructions on how to play the game

void show_help (void)
{
    wchar_t *wchelp_text[HELP_TEXT_PAGES];
    wchar_t *wcbuf = xmalloc(BIGBUFSIZE * sizeof(wchar_t));
    chtype *outbuf = xmalloc(BIGBUFSIZE * sizeof(chtype));

    int curpage = 0;
    int numpages = 0;
    bool done = false;


    // Count how many pages appear in the (translated) help text
    while (numpages < HELP_TEXT_PAGES) {
	const char *s = gettext(help_text[numpages]);
	if (s == NULL || *s == '\0' || *s == '@')
	    break;

	xmbstowcs(wcbuf, s, BIGBUFSIZE);
	wchelp_text[numpages] = xwcsdup(wcbuf);
	numpages++;
    }

    if (numpages == 0) {
	free(outbuf);
	free(wcbuf);
	return;
    }

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

	const wchar_t *htxt = wchelp_text[curpage];
	char convbuf[MB_LEN_MAX + 1];
	char *cp;
	mbstate_t mbstate;
	chtype *outp;
	size_t i, n;

	int count = BIGBUFSIZE;
	int maxchar = MB_CUR_MAX;
	int curattr = attr_normal;

	memset(&mbstate, 0, sizeof(mbstate));
	outp = outbuf;

	while (*htxt != '\0' && count > maxchar * 2) {
	    switch (*htxt) {
	    case '\n':
		// Start a new line
		*outp++ = '\n';
		count--;
		break;

	    case '^':
		// Switch to a different character rendition
		switch (*++htxt) {
		case '^':
		    wcbuf[0] = *htxt;
		    wcbuf[1] = '\0';
		    goto addwcbuf;

		case 'N':
		    curattr = attr_normal;
		    break;

		case 'B':
		    curattr = attr_normal | A_BOLD;
		    break;

		case 'H':
		    curattr = attr_highlight;
		    break;

		case 'K':
		    curattr = attr_keycode;
		    break;

		case 'e':
		    curattr = attr_map_empty;
		    break;

		case 'o':
		    curattr = attr_map_outpost;
		    break;

		case 's':
		    curattr = attr_map_star;
		    break;

		case 'c':
		    curattr = attr_map_company;
		    break;

		case 'k':
		    curattr = attr_map_choice;
		    break;

		default:
		    wcbuf[0] = '^';
		    wcbuf[1] = *htxt;
		    wcbuf[2] = '\0';
		    goto addwcbuf;
		}
		break;

	    case '~':
		// Print a global constant
		switch (*++htxt) {
		case '~':
		    wcbuf[0] = *htxt;
		    wcbuf[1] = '\0';
		    goto addwcbuf;

		case 'x':
		    swprintf(wcbuf, BIGBUFSIZE, L"%2d", MAX_X);
		    goto addwcbuf;

		case 'y':
		    swprintf(wcbuf, BIGBUFSIZE, L"%2d", MAX_Y);
		    goto addwcbuf;

		case 'm':
		    swprintf(wcbuf, BIGBUFSIZE, L"%2d", NUMBER_MOVES);
		    goto addwcbuf;

		case 'c':
		    swprintf(wcbuf, BIGBUFSIZE, L"%d", MAX_COMPANIES);
		    goto addwcbuf;

		case 't':
		    swprintf(wcbuf, BIGBUFSIZE, L"%2d", DEFAULT_MAX_TURN);
		    goto addwcbuf;

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
		    wcbuf[0] = PRINTABLE_GAME_MOVE(*htxt - L'1');
		    wcbuf[1] = '\0';
		    goto addwcbuf;

		case 'M':
		    // Last choice of move, as a key press
		    wcbuf[0] = PRINTABLE_GAME_MOVE(NUMBER_MOVES - 1);
		    wcbuf[1] = '\0';
		    goto addwcbuf;

		case '.':
		    // Map representation of empty space
		    wcbuf[0] = PRINTABLE_MAP_VAL(MAP_EMPTY);
		    wcbuf[1] = '\0';
		    goto addwcbuf;

		case '+':
		    // Map representation of an outpost
		    wcbuf[0] = PRINTABLE_MAP_VAL(MAP_OUTPOST);
		    wcbuf[1] = '\0';
		    goto addwcbuf;

		case '*':
		    // Map representation of a star
		    wcbuf[0] = PRINTABLE_MAP_VAL(MAP_STAR);
		    wcbuf[1] = '\0';
		    goto addwcbuf;

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		    // Map representation of company
		    assert((*htxt - L'A') < MAX_COMPANIES);
		    wcbuf[0] = PRINTABLE_MAP_VAL(COMPANY_TO_MAP(*htxt - L'A'));
		    wcbuf[1] = '\0';
		    goto addwcbuf;

		default:
		    wcbuf[0] = '~';
		    wcbuf[1] = *htxt;
		    goto addwcbuf;
		}
		break;

	    default:
		// Print the character
		wcbuf[0] = *htxt;
		wcbuf[1] = '\0';

	    addwcbuf:
		for (wchar_t *p = wcbuf; *p != '\0' && count > maxchar * 2; p++) {
		    n = xwcrtomb(convbuf, *p, &mbstate);
		    for (i = 0, cp = convbuf; i < n; i++, cp++, outp++, count--) {
			*outp = (unsigned char) *cp | curattr;
		    }
		}
	    }

	    htxt++;
	}

	// Add the terminating NUL (possibly with a preceding shift sequence)
	n = xwcrtomb(convbuf, '\0', &mbstate);
	for (i = 0, cp = convbuf; i < n; i++, cp++, outp++, count--) {
	    *outp = (unsigned char) *cp;
	}
	assert(count >= 0);

	// Display the formatted text in outbuf
	for (outp = outbuf; *outp != '\0'; outp++) {
	    if (*outp == '\n') {
		wmove(curwin, getcury(curwin) + 1, 2);
	    } else {
		waddch(curwin, *outp);
	    }
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

	wint_t key;
	if (gettxchar(curwin, &key) == OK) {
	    // Ordinary wide character
	    curpage++;
	    done = (curpage == numpages);
	} else {
	    // Function or control character
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
    }

    deltxwin();
    txrefresh();

    for (curpage = 0; curpage < numpages; curpage++) {
	free(wchelp_text[curpage]);
    }
    free(outbuf);
    free(wcbuf);
}
