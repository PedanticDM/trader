/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, exch.c, contains the implementation of functions dealing
  with the Interstellar Stock Exchange and Trading Bank as used in Star
  Traders.


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

void visit_bank (void);
void trade_shares (int num, bool *bid_used);


/************************************************************************
*                  Stock Exchange function definitions                  *
************************************************************************/

/*-----------------------------------------------------------------------
  Function:   exchange_stock  - Visit the Interstellar Stock Exchange
  Arguments:  (none)
  Returns:    (nothing)

  This function allows the current player (in current_player) to buy,
  sell and bid for shares in companies that appear on the galaxy map.
*/

void exchange_stock (void)
{
    selection_t selection = SEL_NONE;
    bool bid_used = false;
    bool all_off_map;
    int i, line;


    if (quit_selected || abort_game || ! player[current_player].in_game) {
	return;
    }

    newtxwin(17, 80, LINE_OFFSET + 1, COL_CENTER(80));

    while (selection != SEL_EXIT) {
	selection = SEL_NONE;

	// Display (or refresh) the Stock Exchange window
	wbkgd(curwin, ATTR_NORMAL_WINDOW);
	werase(curwin);
	box(curwin, 0, 0);

	center(curwin, 1, ATTR_WINDOW_TITLE, "  Interstellar Stock Exchange  ");
	center2(curwin, 2, ATTR_NORMAL_WINDOW, ATTR_HIGHLIGHT_STR, "Player: ",
		"%s", player[current_player].name);

	all_off_map = true;
	for (i = 0; i < MAX_COMPANIES; i++) {
	    if (company[i].on_map) {
		all_off_map = false;
		break;
	    }
	}

	if (all_off_map) {
	    center(curwin, 8, ATTR_NORMAL_WINDOW, "No companies on the map");
	} else {
	    char *buf = malloc(BUFSIZE);
	    if (buf == NULL) {
		err_exit("out of memory");
	    }

	    // Handle the locale's currency symbol
	    struct lconv *lc = localeconv();
	    assert(lc != NULL);
	    snprintf(buf, BUFSIZE, "share (%s)", lc->currency_symbol);

	    wattrset(curwin, ATTR_WINDOW_SUBTITLE);
	    mvwprintw(curwin, 4, 2, "  %-22s  %10s  %10s  %12s  %10s  ",
		      "", "Shares", "Shares", "Price per", "");
	    mvwprintw(curwin, 5, 2, "  %-22s  %10s  %10s  %12s  %10s  ",
		      "Company", "issued", "left", buf, "Return (%)");
	    wattrset(curwin, ATTR_NORMAL_WINDOW);

	    for (line = 6, i = 0; i < MAX_COMPANIES; i++) {
		if (company[i].on_map) {
		    mvwaddch(curwin, line, 2, PRINTABLE_MAP_VAL(COMPANY_TO_MAP(i)) |
			     ATTR_MAP_CHOICE);
		    strfmon(buf, BUFSIZE, "%!12n", company[i].share_price);
		    mvwprintw(curwin, line, 4, "%-22s  %'10ld  %'10ld  %12s  %10.2f  ",
			      company[i].name, company[i].stock_issued,
			      company[i].max_stock - company[i].stock_issued,
			      buf, company[i].share_return * 100.0);
		    line++;
		}
	    }

	    free(buf);
	}

	wrefresh(curwin);

	// Show menu of choices for the player
	newtxwin(6, 80, LINE_OFFSET + 18, COL_CENTER(80));
	wbkgd(curwin, ATTR_NORMAL_WINDOW);
	box(curwin, 0, 0);

	wmove(curwin, 3, 2);
	attrpr(curwin, ATTR_KEYCODE_STR, "<1>");
	waddstr(curwin, " Display stock portfolio");

	wmove(curwin, 4, 2);
	attrpr(curwin, ATTR_KEYCODE_STR, "<2>");
	waddstr(curwin, " Display galaxy map");

	wmove(curwin, 3, 40);
	attrpr(curwin, ATTR_KEYCODE_STR, "<3>");
	waddstr(curwin, " Visit the Trading Bank");

	wmove(curwin, 4, 40);
	attrpr(curwin, ATTR_KEYCODE_STR, "<4>");
	waddstr(curwin, " Exit the Stock Exchange");

	mvwaddstr(curwin, 1, 2, "                Enter selection ");
	waddstr(curwin, "[");
	attrpr(curwin, ATTR_HIGHLIGHT_STR, "Company letter");
	waddstr(curwin, "/");
	attrpr(curwin, ATTR_KEYCODE_STR, "1");
	waddstr(curwin, "-");
	attrpr(curwin, ATTR_KEYCODE_STR, "4");
	waddstr(curwin, "]: ");

	curs_set(CURS_ON);
	wrefresh(curwin);

	// Get the actual selection made by the player
	while (selection == SEL_NONE) {
	    int key = toupper(gettxchar(curwin));

	    if (IS_COMPANY_KEY(key)) {
		if (company[KEY_TO_COMPANY(key)].on_map) {
		    selection = KEY_TO_COMPANY(key);
		} else {
		    beep();
		}
	    } else {
		switch (key) {
		case '1':
		    curs_set(CURS_OFF);
		    show_status(current_player);
		    curs_set(CURS_ON);
		    break;

		case '2':
		    curs_set(CURS_OFF);
		    show_map(true);
		    curs_set(CURS_ON);
		    break;

		case '3':
		    selection = SEL_BANK;
		    break;

		case '4':
		case ' ':
		    selection = SEL_EXIT;
		    break;

		default:
		    beep();
		    break;
		}
	    }
	}

	curs_set(CURS_OFF);

	deltxwin();		// "Enter selection" window
	txrefresh();

	if (selection == SEL_BANK) {
	    // Visit the Interstellar Trading Bank
	    visit_bank();
	} else if (selection == SEL_EXIT) {
	    // Exit the Stock Exchange: nothing more to do
	    ;
	} else {
	    trade_shares(selection, &bid_used);
	}
    }

    deltxwin();			// "Stock Exchange" window
    txrefresh();
}


/*-----------------------------------------------------------------------
  Function:   visit_bank  - Visit the Interstellar Trading Bank
  Arguments:  (none)
  Returns:    (nothing)

  This function allows the current player to borrow or repay money from
  the Interstellar Trading Bank.
*/

void visit_bank (void)
{
    // @@@ To be written
}


/*-----------------------------------------------------------------------
  Function:   trade_stock  - Trade stock in a particular company
  Arguments:  num          - Company with which to trade
              bid_used     - Has the player used up their bid?
  Returns:    (nothing)

  This function allows the current player to buy and sell stock in
  company num.  The variable *bid_used is set to true if the player tries
  to bid for more shares to be released by the company.
*/

void trade_shares (int num, bool *bid_used)
{
    assert(num >= 0 && num < MAX_COMPANIES);

    // @@@ To be written
}
