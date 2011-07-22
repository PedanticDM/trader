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
*                  Module-specific function prototypes                  *
************************************************************************/

/*
  Function:   visit_bank - Visit the Interstellar Trading Bank
  Parameters: (none)
  Returns:    (nothing)

  This function allows the current player to borrow or repay money from
  the Interstellar Trading Bank.
*/
static void visit_bank (void);


/*
  Function:   trade_shares - Buy and sell shares in a particular company
  Parameters: num          - Company with which to trade
              bid_used     - Has the player used up their bid?
  Returns:    (nothing)

  This function allows the current player to buy and sell shares in
  company num.  The variable *bid_used is set to true if the player tries
  to bid for more shares to be released by the company (whether or not
  that bid was successful).
*/
static void trade_shares (int num, bool *bid_used);


/************************************************************************
*                  Stock Exchange function definitions                  *
************************************************************************/

// This function is documented in the file "exch.h"


/***********************************************************************/
// exchange_stock: Visit the Interstellar Stock Exchange

void exchange_stock (void)
{
    selection_t selection = SEL_NONE;
    bool bid_used = false;
    bool all_off_map;
    int i, line;


    if (quit_selected || abort_game || ! player[current_player].in_game) {
	return;
    }

    newtxwin(17, WIN_COLS, 1, WCENTER(WIN_COLS), false, 0);

    while (selection != SEL_EXIT) {
	selection = SEL_NONE;

	// Display (or refresh) the Stock Exchange window
	wbkgd(curwin, attr_normal_window);
	werase(curwin);
	box(curwin, 0, 0);

	center(curwin, 1, attr_title, "  Interstellar Stock Exchange  ");
	center2(curwin, 2, attr_normal, attr_highlight, "Player: ", "%s",
		player[current_player].name);

	all_off_map = true;
	for (i = 0; i < MAX_COMPANIES; i++) {
	    if (company[i].on_map) {
		all_off_map = false;
		break;
	    }
	}

	if (all_off_map) {
	    center(curwin, 8, attr_normal, "No companies on the map");
	} else {
	    char *buf = malloc(BUFSIZE);
	    if (buf == NULL) {
		err_exit_nomem();
	    }

	    // Handle the locale's currency symbol
	    snprintf(buf, BUFSIZE, "share (%s)", lconvinfo.currency_symbol);

	    wattrset(curwin, attr_subtitle);
	    mvwprintw(curwin, 4, 2, "  %-22s  %12s  %10s  %10s  %10s  ",
		      "", "Price per", "", "Shares", "Shares");
	    mvwprintw(curwin, 5, 2, "  %-22s  %12s  %10s  %10s  %10s  ",
		      "Company", buf, "Return (%)", "issued", "left");
	    wattrset(curwin, attr_normal);

	    for (line = 6, i = 0; i < MAX_COMPANIES; i++) {
		if (company[i].on_map) {
		    mvwaddch(curwin, line, 2, PRINTABLE_MAP_VAL(COMPANY_TO_MAP(i))
			     | attr_choice);
		    l_strfmon(buf, BUFSIZE, "%!12n", company[i].share_price);
		    mvwprintw(curwin, line, 4, "%-22s  %12s  %10.2f  %'10ld  %'10ld  ",
			      company[i].name, buf, company[i].share_return
			      * 100.0, company[i].stock_issued,
			      company[i].max_stock - company[i].stock_issued);
		    line++;
		}
	    }

	    free(buf);
	}

	wrefresh(curwin);

	// Show menu of choices for the player
	newtxwin(6, WIN_COLS, 18, WCENTER(WIN_COLS), true, attr_normal_window);

	wmove(curwin, 3, 2);
	attrpr(curwin, attr_keycode, "<1>");
	waddstr(curwin, " Display stock portfolio");

	wmove(curwin, 4, 2);
	attrpr(curwin, attr_keycode, "<2>");
	waddstr(curwin, " Display galaxy map");

	wmove(curwin, 3, 40);
	attrpr(curwin, attr_keycode, "<3>");
	waddstr(curwin, " Visit the Trading Bank");

	wmove(curwin, 4, 40);
	attrpr(curwin, attr_keycode, "<4>");
	waddstr(curwin, " Exit the Stock Exchange");

	mvwaddstr(curwin, 1, 18, "Enter selection ");
	waddstr(curwin, "[");
	attrpr(curwin, attr_highlight, "Company letter");
	waddstr(curwin, "/");
	attrpr(curwin, attr_keycode, "1");
	waddstr(curwin, "-");
	attrpr(curwin, attr_keycode, "4");
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
		case KEY_CANCEL:
		case KEY_EXIT:
		case KEY_CTRL('C'):
		case KEY_CTRL('G'):
		case KEY_CTRL('\\'):
		    selection = SEL_EXIT;
		    break;

		default:
		    beep();
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


/************************************************************************
*                  Module-specific function prototypes                  *
************************************************************************/

// These functions are documented at the start of this file


/***********************************************************************/
// visit_bank: Visit the Interstellar Trading Bank

void visit_bank (void)
{
    double credit_limit;
    double val, max;
    int key;
    bool done;
    char *buf;


    buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit_nomem();
    }

    credit_limit = (total_value(current_player) - player[current_player].debt)
	* CREDIT_LIMIT_RATE;
    if (credit_limit < 0.0) {
	credit_limit = 0.0;
    }

    // Show the informational part of the Bank
    newtxwin(10, WIN_COLS - 4, 5, WCENTER(WIN_COLS - 4), true,
	     attr_normal_window);

    center(curwin, 1, attr_title, "  Interstellar Trading Bank  ");

    l_strfmon(buf, BUFSIZE, "%18n", player[current_player].cash);
    center2(curwin, 3, attr_normal, attr_highlight, "Current cash:  ",
	    " %s ", buf);

    l_strfmon(buf, BUFSIZE, "%18n", player[current_player].debt);
    center2(curwin, 4, attr_normal, attr_highlight, "Current debt:  ",
	    " %s ", buf);

    center2(curwin, 5, attr_normal, attr_highlight, "Interest rate: ",
	    " %17.2f%% ", interest_rate * 100.0);

    l_strfmon(buf, BUFSIZE, "%18n", credit_limit);
    center2(curwin, 7, attr_highlight, attr_title, "Credit limit:  ",
	    " %s ", buf);

    wrefresh(curwin);

    // Show menu of choices for the player
    newtxwin(7, WIN_COLS - 4, 15, WCENTER(WIN_COLS - 4), true,
	     attr_normal_window);

    center2(curwin, 3, attr_keycode, attr_normal, "<1>", " Borrow money      ");
    center2(curwin, 4, attr_keycode, attr_normal, "<2>", " Repay debt        ");
    center2(curwin, 5, attr_keycode, attr_normal, "<3>", " Exit from the Bank");

    mvwaddstr(curwin, 1, 24, "Enter selection ");
    waddstr(curwin, "[");
    attrpr(curwin, attr_keycode, "1");
    waddstr(curwin, "-");
    attrpr(curwin, attr_keycode, "3");
    waddstr(curwin, "]: ");

    curs_set(CURS_ON);
    wrefresh(curwin);

    done = false;
    while (! done) {
	key = gettxchar(curwin);

	switch (key) {
	case '1':
	case '2':
	case '3':
	    wechochar(curwin, key | A_BOLD);
	    done = true;
	    break;

	case ' ':
	case KEY_CANCEL:
	case KEY_EXIT:
	case KEY_CTRL('C'):
	case KEY_CTRL('G'):
	case KEY_CTRL('\\'):
	    done = true;
	    break;

	default:
	    beep();
	}
    }

    curs_set(CURS_OFF);

    switch (key) {
    case '1':
	// Borrow money from the Bank
	if (credit_limit == 0.0) {
	    newtxwin(7, 50, 8, WCENTER(50), true, attr_error_window);

	    center(curwin, 1, attr_error_title, "  Insufficient Credit Limit  ");
	    center(curwin, 3, attr_error_highlight,
		   "The Bank will not lend you any more money");

	    wait_for_key(curwin, 5, attr_error_waitforkey);
	    deltxwin();

	} else {
	    int x, y, n;
	    int ret;


	    wbkgd(curwin, attr_normal_window);
	    werase(curwin);
	    box(curwin, 0, 0);

	    mvwprintw(curwin, 3, 10, "How much do you wish to borrow? ");

	    // Show the currency symbol before or after the input field
	    wattron(curwin, A_BOLD);
	    if (lconvinfo.p_cs_precedes == 1) {
		wprintw(curwin, "%s%s", lconvinfo.currency_symbol,
			(lconvinfo.p_sep_by_space == 1) ? " " : "");
		n = 10;
	    } else {
		getyx(curwin, y, x);
		n = strlen(lconvinfo.currency_symbol) + 10
		    + (lconvinfo.p_sep_by_space == 1);
		mvwprintw(curwin, y, getmaxx(curwin) - n, "%s%s",
			  (lconvinfo.p_sep_by_space == 1) ? " " : "",
			  lconvinfo.currency_symbol);
		wmove(curwin, y, x);
	    }
	    wattroff(curwin, A_BOLD);
	    x = getcurx(curwin);

	    ret = gettxdouble(curwin, &val, 0.0, credit_limit + ROUNDING_AMOUNT,
			      0.0, credit_limit, 3, x, getmaxx(curwin) - x - n,
			      attr_input_field);

	    if (ret == OK && val > ROUNDING_AMOUNT) {
		player[current_player].cash += val;
		player[current_player].debt += val * (interest_rate + 1.0);
	    }
	}
	break;

    case '2':
	// Repay a debt
	if (player[current_player].debt == 0.0) {
	    newtxwin(7, 50, 8, WCENTER(50), true, attr_error_window);

	    center(curwin, 1, attr_error_title, "  No Debt  ");
	    center(curwin, 3, attr_error_highlight,
		   "You have no debt to repay");

	    wait_for_key(curwin, 5, attr_error_waitforkey);
	    deltxwin();

	} else if (player[current_player].cash == 0.0) {
	    newtxwin(7, 60, 8, WCENTER(60), true, attr_error_window);

	    center(curwin, 1, attr_error_title, "  No Cash  ");
	    center(curwin, 3, attr_error_highlight,
		   "You have no cash with which to repay the debt!");

	    wait_for_key(curwin, 5, attr_error_waitforkey);
	    deltxwin();

	} else {
	    int x, y, n;
	    int ret;


	    wbkgd(curwin, attr_normal_window);
	    werase(curwin);
	    box(curwin, 0, 0);

	    mvwprintw(curwin, 3, 10, "How much do you wish to repay? ");

	    // Show the currency symbol before or after the input field
	    wattron(curwin, A_BOLD);
	    if (lconvinfo.p_cs_precedes == 1) {
		wprintw(curwin, "%s%s", lconvinfo.currency_symbol,
			(lconvinfo.p_sep_by_space == 1) ? " " : "");
		n = 10;
	    } else {
		getyx(curwin, y, x);
		n = strlen(lconvinfo.currency_symbol) + 10
		    + (lconvinfo.p_sep_by_space == 1);
		mvwprintw(curwin, y, getmaxx(curwin) - n, "%s%s",
			  (lconvinfo.p_sep_by_space == 1) ? " " : "",
			  lconvinfo.currency_symbol);
		wmove(curwin, y, x);
	    }
	    wattroff(curwin, A_BOLD);
	    x = getcurx(curwin);

	    max = MIN(player[current_player].cash, player[current_player].debt);

	    ret = gettxdouble(curwin, &val, 0.0, max + ROUNDING_AMOUNT, 0.0,
			      max, 3, x, getmaxx(curwin) - x - n,
			      attr_input_field);

	    if (ret == OK) {
		player[current_player].cash -= val;
		player[current_player].debt -= val;

		if (player[current_player].cash < ROUNDING_AMOUNT) {
		    player[current_player].cash = 0.0;
		}
		if (player[current_player].debt < ROUNDING_AMOUNT) {
		    player[current_player].debt = 0.0;
		}
	    }
	}
	break;

    default:
	break;
    }

    deltxwin();			// "Enter selection" window
    deltxwin();			// Trading Bank window
    txrefresh();

    free(buf);
}


/***********************************************************************/
// trade_shares: Buy and sell shares in a particular company

void trade_shares (int num, bool *bid_used)
{
    bool done;
    int key, ret, x;
    long int maxshares, val;
    double ownership;
    char *buf;


    assert(num >= 0 && num < MAX_COMPANIES);
    assert(company[num].on_map);

    buf = malloc(BUFSIZE);
    if (buf == NULL) {
	err_exit_nomem();
    }

    ownership = (company[num].stock_issued == 0) ? 0.0 :
	((double) player[current_player].stock_owned[num]
	 / company[num].stock_issued);

    // Show the informational part of the trade window
    newtxwin(9, WIN_COLS - 4, 5, WCENTER(WIN_COLS - 4), true,
	     attr_normal_window);

    center(curwin, 1, attr_title, "  Stock Transaction in %s  ",
	   company[num].name);

    mvwaddstr(curwin, 3, 2, "Shares issued:   ");
    attrpr(curwin, attr_highlight, "%'12ld", company[num].stock_issued);

    mvwaddstr(curwin, 4, 2, "Shares left:     ");
    attrpr(curwin, attr_highlight, "%'12ld",
	   company[num].max_stock - company[num].stock_issued);

    mvwaddstr(curwin, 5, 2, "Price per share: ");
    l_strfmon(buf, BUFSIZE, "%12n", company[num].share_price);
    attrpr(curwin, attr_highlight, "%12s", buf);

    mvwaddstr(curwin, 6, 2, "Return:          ");
    attrpr(curwin, attr_highlight, "%11.2f%%",
	   company[num].share_return * 100.0);

    mvwaddstr(curwin, 3, 38, "Current holdings: ");
    attrpr(curwin, attr_highlight, " %'16ld ",
	   player[current_player].stock_owned[num]);

    mvwaddstr(curwin, 4, 38, "Percentage owned: ");
    attrpr(curwin, attr_highlight, " %'15.2f%% ", ownership * 100.0);

    wmove(curwin, 6, 38);
    attrpr(curwin, attr_highlight, "Current cash:     ");
    l_strfmon(buf, BUFSIZE, "%16n", player[current_player].cash);
    attrpr(curwin, attr_title, " %16s ", buf);

    wrefresh(curwin);

    // Show menu of choices for the player
    newtxwin(7, WIN_COLS - 4, 14, WCENTER(WIN_COLS - 4), true,
	     attr_normal_window);

    wmove(curwin, 3, 2);
    attrpr(curwin, attr_keycode, "<1>");
    waddstr(curwin, " Buy stock from company");

    wmove(curwin, 4, 2);
    attrpr(curwin, attr_keycode, "<2>");
    waddstr(curwin, " Sell stock back to company");

    wmove(curwin, 3, 38);
    attrpr(curwin, attr_keycode, "<3>");
    waddstr(curwin, " Bid company to issue more shares");

    wmove(curwin, 4, 38);
    attrpr(curwin, attr_keycode, "<4>");
    waddstr(curwin, " Exit to the Stock Exchange");

    mvwaddstr(curwin, 1, 24, "Enter selection ");
    waddstr(curwin, "[");
    attrpr(curwin, attr_keycode, "1");
    waddstr(curwin, "-");
    attrpr(curwin, attr_keycode, "4");
    waddstr(curwin, "]: ");

    curs_set(CURS_ON);
    wrefresh(curwin);

    done = false;
    while (! done) {
	key = gettxchar(curwin);

	switch (key) {
	case '1':
	case '2':
	case '3':
	case '4':
	    wechochar(curwin, key | A_BOLD);
	    done = true;
	    break;

	case ' ':
	case KEY_CANCEL:
	case KEY_EXIT:
	case KEY_CTRL('C'):
	case KEY_CTRL('G'):
	case KEY_CTRL('\\'):
	    done = true;
	    break;

	default:
	    beep();
	}
    }

    curs_set(CURS_OFF);

    switch (key) {
    case '1':
	// Buy stock in company
	maxshares = player[current_player].cash / company[num].share_price;

	if (company[num].max_stock - company[num].stock_issued == 0) {
	    newtxwin(7, 50, 8, WCENTER(50), true, attr_error_window);

	    center(curwin, 1, attr_error_title, "  No Shares Available  ");
	    center(curwin, 3, attr_error_highlight,
		   "No more shares are available for purchase");

	    wait_for_key(curwin, 5, attr_error_waitforkey);
	    deltxwin();

	} else if (maxshares <= 0) {
	    newtxwin(7, 50, 8, WCENTER(50), true, attr_error_window);

	    center(curwin, 1, attr_error_title, "  Insufficient Cash  ");
	    center(curwin, 3, attr_error_highlight,
		   "Not enough cash to purchase shares");

	    wait_for_key(curwin, 5, attr_error_waitforkey);
	    deltxwin();

	} else {
	    maxshares = MIN(maxshares, company[num].max_stock -
			    company[num].stock_issued);

	    wbkgd(curwin, attr_normal_window);
	    werase(curwin);
	    box(curwin, 0, 0);

	    center3(curwin, 2, attr_normal, attr_normal, attr_highlight,
		    "You can purchase up to ", " shares.", "%'ld", maxshares);

	    mvwprintw(curwin, 4, 10, "How many shares do you wish to purchase? ");
	    x = getcurx(curwin);

	    ret = gettxlong(curwin, &val, 0, maxshares, 0, maxshares, 4, x,
			    getmaxx(curwin) - x - 10, attr_input_field);

	    if (ret == OK) {
		player[current_player].cash -= val * company[num].share_price;
		player[current_player].stock_owned[num] += val;
		company[num].stock_issued += val;
	    }
	}
	break;

    case '2':
	// Sell stock back to company
	maxshares = player[current_player].stock_owned[num];
	if (maxshares == 0) {
	    newtxwin(7, 50, 8, WCENTER(50), true, attr_error_window);

	    center(curwin, 1, attr_error_title, "  No Shares  ");
	    center(curwin, 3, attr_error_highlight,
		   "You do not have any shares to sell");

	    wait_for_key(curwin, 5, attr_error_waitforkey);
	    deltxwin();

	} else {
	    wbkgd(curwin, attr_normal_window);
	    werase(curwin);
	    box(curwin, 0, 0);

	    center3(curwin, 2, attr_normal, attr_normal, attr_highlight,
		    "You can sell up to ", " shares.", "%'ld", maxshares);

	    mvwprintw(curwin, 4, 10, "How many shares do you wish to sell? ");
	    x = getcurx(curwin);

	    ret = gettxlong(curwin, &val, 0, maxshares, 0, maxshares, 4, x,
			    getmaxx(curwin) - x - 10, attr_input_field);

	    if (ret == OK) {
		company[num].stock_issued -= val;
		player[current_player].stock_owned[num] -= val;
		player[current_player].cash += val * company[num].share_price;
	    }
	}
	break;

    case '3':
	// Bid company to issue more shares
	maxshares = 0;
	if (! *bid_used && randf() < ownership && randf() < BID_CHANCE) {
	    maxshares = randf() * ownership * MAX_SHARES_BIDDED;
	    company[num].max_stock += maxshares;
	}

	*bid_used = true;

	if (maxshares == 0) {
	    newtxwin(8, 50, 8, WCENTER(50), true, attr_error_window);

	    center(curwin, 1, attr_error_title, "  No Shares Issued  ");
	    center(curwin, 3, attr_error_highlight, "%s", company[num].name);
	    center(curwin, 4, attr_error_highlight,
		   "has refused to issue more shares");

	    wait_for_key(curwin, 6, attr_error_waitforkey);
	    deltxwin();
	} else {
	    newtxwin(8, 50, 8, WCENTER(50), true, attr_normal_window);

	    center(curwin, 1, attr_title, "  Shares Issued  ");
	    center(curwin, 3, attr_highlight, "%s", company[num].name);
	    center(curwin, 4, attr_highlight, "has issued %'ld more shares",
		maxshares);

	    wait_for_key(curwin, 6, attr_waitforkey);
	    deltxwin();
	}
	break;

    default:
	break;
    }

    deltxwin();			// "Enter selection" window
    deltxwin();			// Stock Transaction window
    txrefresh();

    free(buf);
}


/***********************************************************************/
// End of file
