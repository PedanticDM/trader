/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2019, John Zaitseff                 *
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
  along with this program.  If not, see https://www.gnu.org/licenses/.
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
    int w, i, line;


    if (quit_selected || abort_game || ! player[current_player].in_game) {
	return;
    }

    newtxwin(16, WIN_COLS, 1, WCENTER, false, 0);
    w = getmaxx(curwin);

    while (selection != SEL_EXIT) {
	selection = SEL_NONE;

	// Display (or refresh) the Stock Exchange window
	wbkgdset(curwin, attr_normal_window);
	werase(curwin);
	box(curwin, 0, 0);

	center(curwin, 1, 0, attr_title, 0, 0, 1,
	       _("  Interstellar Stock Exchange  "));
	center(curwin, 2, 0, attr_normal, attr_highlight, 0, 1,
	       _("Player: ^{%ls^}"), player[current_player].name);

	all_off_map = true;
	for (i = 0; i < MAX_COMPANIES; i++) {
	    if (company[i].on_map) {
		all_off_map = false;
		break;
	    }
	}

	if (all_off_map) {
	    center(curwin, 8, 0, attr_normal, attr_highlight, 0, 1,
		   _("No companies on the map"));
	} else {
	    mvwhline(curwin, 4, 2, ' ' | attr_subtitle, w - 4);
	    mvwhline(curwin, 5, 2, ' ' | attr_subtitle, w - 4);

	    left(curwin, 4, 4, attr_subtitle, 0, 0, 2,
		 /* TRANSLATORS: "Company" is a two-line column label in
		    a table containing a list of companies. */
		 pgettext("subtitle", " \nCompany"));
	    right(curwin, 4, w - 4, attr_subtitle, 0, 0, 2,
		  /* TRANSLATORS: "Shares left" is a two-line column
		     label in a table containing the number of shares
		     left to be purchased in any given company.  The
		     maximum column width is 10 characters (see
		     STOCK_LEFT_COLS in src/intf.h). */
		  pgettext("subtitle", "Shares\nleft"));
	    right(curwin, 4, w - 6 - STOCK_LEFT_COLS, attr_subtitle, 0, 0, 2,
		  /* TRANSLATORS: "Shares issued" is a two-line column
		     label in a table containing the number of shares
		     already sold (ie, bought by all players) in any
		     given company.  The maximum column width is 10
		     characters (see STOCK_ISSUED_COLS in src/intf.h). */
		  pgettext("subtitle", "Shares\nissued"));
	    right(curwin, 4, w - 8 - STOCK_LEFT_COLS - STOCK_ISSUED_COLS,
		  attr_subtitle, 0, 0, 2,
		  /* TRANSLATORS: "Return" is a two-line column label in
		     a table containing the share return as a percentage
		     in any given company.  The maximum column width is
		     10 characters (see SHARE_RETURN_COLS in src/intf.h). */
		  pgettext("subtitle", "Return\n(%%)"));
	    right(curwin, 4, w - 10 - STOCK_LEFT_COLS - STOCK_ISSUED_COLS
		  - SHARE_RETURN_COLS, attr_subtitle, 0, 0, 2,
		  /* TRANSLATORS: "Price per share" is a two-line column
		     label in a table containing the price per share in
		     any given company.  %ls is the currency symbol in
		     the current locale.  The maximum column width is 12
		     characters INCLUDING the currency symbol (see
		     SHARE_PRICE_COLS in src/intf.h). */
		  pgettext("subtitle", "Price per\nshare (%ls)"),
		  currency_symbol);

	    for (line = 6, i = 0; i < MAX_COMPANIES; i++) {
		if (company[i].on_map) {
		    left(curwin, line, 2, attr_choice, 0, 0, 1, "%lc",
			 (wint_t) PRINTABLE_MAP_VAL(COMPANY_TO_MAP(i)));
		    left(curwin, line, 4, attr_normal, 0, 0, 1, "%ls",
			 company[i].name);

		    right(curwin, line, w - 2, attr_normal, 0, 0, 1, "%'ld  ",
			  company[i].max_stock - company[i].stock_issued);
		    right(curwin, line, w - 4 - STOCK_LEFT_COLS, attr_normal,
			  0, 0, 1, "%'ld  ", company[i].stock_issued);
		    right(curwin, line, w - 6 - STOCK_LEFT_COLS
			  - STOCK_ISSUED_COLS, attr_normal, 0, 0, 1, "%.2f  ",
			  company[i].share_return * 100.0);
		    right(curwin, line, w - 8 - STOCK_LEFT_COLS
			  - STOCK_ISSUED_COLS - SHARE_RETURN_COLS, attr_normal,
			  0, 0, 1, "  %!N  ", company[i].share_price);

		    line++;
		}
	    }
	}

	wrefresh(curwin);

	// Show menu of choices for the player
	newtxwin(7, WIN_COLS, 17, WCENTER, true, attr_normal_window);

	left(curwin, 3, 2, attr_normal, attr_keycode, 0, 1,
	     _("^{<1>^} Display stock portfolio"));
	left(curwin, 4, 2, attr_normal, attr_keycode, 0, 1,
	     /* TRANSLATORS: Each label may be up to 37 characters wide
		(for <1> and <2>) or 38 characters wide (for <3> and <4>). */
	     _("^{<2>^} Display galaxy map"));
	left(curwin, 3, getmaxx(curwin) / 2, attr_normal, attr_keycode, 0, 1,
	     _("^{<3>^} Visit the Trading Bank"));
	left(curwin, 4, getmaxx(curwin) / 2, attr_normal, attr_keycode, 0, 1,
	     _("^{<4>^} Exit the Stock Exchange"));

	center(curwin, 1, -1, attr_normal, attr_keycode, attr_highlight, 1,
	       _("Enter selection [^[Company letter^]/^{1^}-^{4^}]: "));

	curs_set(CURS_ON);
	wrefresh(curwin);

	// Get the actual selection made by the player
	while (selection == SEL_NONE) {
	    wint_t key;

	    if (gettxchar(curwin, &key) == OK) {
		// Ordinary wide character
		bool found;

		if (iswupper(*keycode_company)) {
		    key = towupper(key);
		} else if (iswlower(*keycode_company)) {
		    key = towlower(key);
		}

		for (i = 0, found = false; keycode_company[i] != L'\0'; i++) {
		    if (keycode_company[i] == (wchar_t) key) {
			found = true;
			if (company[i].on_map) {
			    selection = (selection_t) i;
			} else {
			    beep();
			}
			break;
		    }
		}

		if (! found) {
		    switch (key) {
		    case L'1':
			curs_set(CURS_OFF);
			show_status(current_player);
			curs_set(CURS_ON);
			break;

		    case L'2':
			curs_set(CURS_OFF);
			show_map(true);
			curs_set(CURS_ON);
			break;

		    case L'3':
			selection = SEL_BANK;
			break;

		    case L'4':
		    case L' ':
			selection = SEL_EXIT;
			break;

		    default:
			beep();
		    }
		}
	    } else {
		// Function or control key
		switch (key) {
		case KEY_ESC:
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
    wint_t key;
    bool done;

    chtype *chbuf = xmalloc(BUFSIZE * sizeof(chtype));
    int x, width;


    credit_limit = (total_value(current_player) - player[current_player].debt)
	* CREDIT_LIMIT_RATE;
    if (credit_limit < 0.0) {
	credit_limit = 0.0;
    }

    // Show the informational part of the Bank
    newtxwin(10, WIN_COLS - 4, 5, WCENTER, true, attr_normal_window);

    center(curwin, 1, 0, attr_title, 0, 0, 1,
	   _("  Interstellar Trading Bank  "));

    mkchstr(chbuf, BUFSIZE, attr_normal, 0, 0, 1, getmaxx(curwin) - 4, &width,
	    1, pgettext("label", "Current cash:  "));
    x = (getmaxx(curwin) + width - (BANK_VALUE_COLS + 2)) / 2;

    rightch(curwin, 3, x, chbuf, 1, &width);
    right(curwin, 3, x + BANK_VALUE_COLS + 2, attr_normal, attr_highlight, 0,
	  1, " ^{%N^} ", player[current_player].cash);

    right(curwin, 4, x, attr_normal, 0, 0, 1,
	  pgettext("label", "Current debt:  "));
    right(curwin, 4, x + BANK_VALUE_COLS + 2, attr_normal, attr_highlight, 0,
	  1, " ^{%N^} ", player[current_player].debt);

    right(curwin, 5, x, attr_normal, 0, 0, 1,
	  pgettext("label", "Interest rate: "));
    right(curwin, 5, x + BANK_VALUE_COLS + 2, attr_normal, attr_highlight, 0,
	  1, " ^{%.2f%%^} ", interest_rate * 100.0);

    right(curwin, 7, x, attr_highlight, 0, 0, 1,
	  /* TRANSLATORS: The "Total value", "Current cash", "Current
	     debt", "Interest rate" and "Credit limit" labels MUST all be
	     the same length (ie, right-padded with spaces as needed) and
	     must have at least one trailing space so that the display
	     routines work correctly.  The maximum length of each label
	     is 36 characters.

	     Note that some of these labels are used for both the Player
	     Status window and the Trading Bank window. */
	  pgettext("label", "Credit limit:  "));
    whline(curwin, ' ' | attr_title, BANK_VALUE_COLS + 2);
    right(curwin, 7, x + BANK_VALUE_COLS + 2, attr_title, 0, 0, 1,
	  " %N ", credit_limit);

    wrefresh(curwin);

    // Show menu of choices for the player
    newtxwin(7, WIN_COLS - 4, 15, WCENTER, true, attr_normal_window);

    center(curwin, 3, 0, attr_normal, attr_keycode, 0, 1,
	   /* TRANSLATORS: The "Borrow money", "Repay debt" and "Exit
	      from the Bank" menu options must all be the same length
	      (ie, padded with trailing spaces as required).  The maximum
	      length is 72 characters. */
	   _("^{<1>^} Borrow money      "));
    center(curwin, 4, 0, attr_normal, attr_keycode, 0, 1,
	   _("^{<2>^} Repay debt        "));
    center(curwin, 5, 0, attr_normal, attr_keycode, 0, 1,
	   _("^{<3>^} Exit from the Bank"));

    center(curwin, 1, 0, attr_normal, attr_keycode, 0, 1,
	   _("Enter selection [^{1^}-^{3^}]: "));

    curs_set(CURS_ON);
    wrefresh(curwin);

    done = false;
    while (! done) {
	if (gettxchar(curwin, &key) == OK) {
	    // Ordinary wide character
	    switch (key) {
	    case L'1':
	    case L'2':
	    case L'3':
		left(curwin, getcury(curwin), getcurx(curwin), A_BOLD,
		     0, 0, 1, "%lc", key);
		wrefresh(curwin);

		done = true;
		break;

	    case L' ':
		done = true;
		break;

	    default:
		beep();
	    }
	} else {
	    // Function or control key
	    switch (key) {
	    case KEY_ESC:
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
    }

    curs_set(CURS_OFF);

    switch (key) {
    case L'1':
	// Borrow money from the Bank
	if (credit_limit == 0.0) {
	    txdlgbox(MAX_DLG_LINES, 50, 8, WCENTER, attr_error_window,
		     attr_error_title, attr_error_highlight, 0, 0,
		     attr_error_waitforkey, _("  Insufficient Credit Limit  "),
		     _("The Bank will not lend you any more money."));
	} else {
	    chtype *chbuf_cursym;
	    int width_cursym;
	    int n, ret;


	    wbkgdset(curwin, attr_normal_window);
	    werase(curwin);
	    box(curwin, 0, 0);

	    n = (lconvinfo.p_sep_by_space == 1) ? 1 : 0;

	    mkchstr(chbuf, BUFSIZE, attr_normal, attr_normal | A_BOLD, 0, 1,
		    getmaxx(curwin) / 2, &width_cursym, 1, "^{%ls^}",
		    currency_symbol);
	    chbuf_cursym = xchstrdup(chbuf);

	    mkchstr(chbuf, BUFSIZE, attr_normal, 0, 0, 1, getmaxx(curwin)
		    - BANK_INPUT_COLS - width_cursym - 6, &width, 1,
		    _("How much do you wish to borrow? "));
	    x = (getmaxx(curwin) + width - BANK_INPUT_COLS - width_cursym
		 - n) / 2;
	    rightch(curwin, 3, x, chbuf, 1, &width);

	    // Show the currency symbol before or after the input field
	    if (lconvinfo.p_cs_precedes == 1) {
		leftch(curwin, 3, x, chbuf_cursym, 1, &width_cursym);
		x += width_cursym + n;
	    } else {
		leftch(curwin, 3, x + BANK_INPUT_COLS + n, chbuf_cursym, 1,
		       &width_cursym);
	    }

	    ret = gettxdouble(curwin, &val, 0.0, credit_limit + ROUNDING_AMOUNT,
			      0.0, credit_limit, 3, x, BANK_INPUT_COLS,
			      attr_input_field);

	    if (ret == OK && val > ROUNDING_AMOUNT) {
		player[current_player].cash += val;
		player[current_player].debt += val * (interest_rate + 1.0);
	    }

	    free(chbuf_cursym);
	}
	break;

    case L'2':
	// Repay a debt
	if (player[current_player].debt == 0.0) {
	    txdlgbox(MAX_DLG_LINES, 50, 8, WCENTER, attr_error_window,
		     attr_error_title, attr_error_highlight, 0, 0,
		     attr_error_waitforkey, _("  No Debt  "),
		     _("You have no debt to repay."));
	} else if (player[current_player].cash == 0.0) {
	    txdlgbox(MAX_DLG_LINES, 50, 8, WCENTER, attr_error_window,
		     attr_error_title, attr_error_highlight, 0, 0,
		     attr_error_waitforkey, _("  No Cash  "),
		     _("You have no cash with which to repay the debt!"));
	} else {
	    chtype *chbuf_cursym;
	    int width_cursym;
	    int n, ret;


	    wbkgdset(curwin, attr_normal_window);
	    werase(curwin);
	    box(curwin, 0, 0);

	    n = (lconvinfo.p_sep_by_space == 1) ? 1 : 0;

	    mkchstr(chbuf, BUFSIZE, attr_normal, attr_normal | A_BOLD, 0, 1,
		    getmaxx(curwin) / 2, &width_cursym, 1, "^{%ls^}",
		    currency_symbol);
	    chbuf_cursym = xchstrdup(chbuf);

	    mkchstr(chbuf, BUFSIZE, attr_normal, 0, 0, 1, getmaxx(curwin)
		    - BANK_INPUT_COLS - width_cursym - 6, &width, 1,
		    _("How much do you wish to repay? "));
	    x = (getmaxx(curwin) + width - BANK_INPUT_COLS - width_cursym
		 - n) / 2;
	    rightch(curwin, 3, x, chbuf, 1, &width);

	    // Show the currency symbol before or after the input field
	    if (lconvinfo.p_cs_precedes == 1) {
		leftch(curwin, 3, x, chbuf_cursym, 1, &width_cursym);
		x += width_cursym + n;
	    } else {
		leftch(curwin, 3, x + BANK_INPUT_COLS + n, chbuf_cursym, 1,
		       &width_cursym);
	    }

	    max = MIN(player[current_player].cash, player[current_player].debt);

	    ret = gettxdouble(curwin, &val, 0.0, max + ROUNDING_AMOUNT, 0.0,
			      max, 3, x, BANK_INPUT_COLS, attr_input_field);

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

	    free(chbuf_cursym);
	}
	break;

    default:
	break;
    }

    deltxwin();			// "Enter selection" window
    deltxwin();			// Trading Bank window
    txrefresh();

    free(chbuf);
}


/***********************************************************************/
// trade_shares: Buy and sell shares in a particular company

void trade_shares (int num, bool *bid_used)
{
    bool done;
    int ret, w, x, mid;
    long int maxshares, val;
    double ownership;
    chtype *chbuf;
    int width;
    wint_t key;


    assert(num >= 0 && num < MAX_COMPANIES);
    assert(company[num].on_map);

    chbuf = xmalloc(BUFSIZE * sizeof(chtype));

    ownership = (company[num].stock_issued == 0) ? 0.0 :
	((double) player[current_player].stock_owned[num]
	 / company[num].stock_issued);

    // Show the informational part of the trade window
    newtxwin(9, WIN_COLS - 4, 5, WCENTER, true, attr_normal_window);
    w = getmaxx(curwin);

    center(curwin, 1, 0, attr_title, 0, 0, 1,
	   /* TRANSLATORS: %ls represents the company name. */
	   _("  Stock Transaction in %ls  "), company[num].name);

    mkchstr(chbuf, BUFSIZE, attr_normal, 0, 0, 1, w / 2, &width, 1,
	    /* TRANSLATORS: "Shares issued" represents the number of
	       shares already sold by the company to all players.

	       Note that the labels "Shares issued", "Shares left",
	       "Price per share" and "Return" must all be the same length
	       and must have at least one trailing space for the output
	       routines to work correctly.  The maximum length of each
	       label is 22 characters. */
	    pgettext("label|Stock A", "Shares issued:   "));
    leftch(curwin, 3, 2, chbuf, 1, &width);
    right(curwin, 3, width + SHARE_PRICE_COLS + 2, attr_normal, attr_highlight,
	  0, 1, "^{%'ld^}", company[num].stock_issued);

    left(curwin, 4, 2, attr_normal, 0, 0, 1,
	 /* TRANSLATORS: "Shares left" is the number of shares that are
	    left to be purchased in the current company. */
	 pgettext("label|Stock A", "Shares left:     "));
    right(curwin, 4, width + SHARE_PRICE_COLS + 2, attr_normal, attr_highlight,
	  0, 1, "^{%'ld^}", company[num].max_stock - company[num].stock_issued);

    left(curwin, 5, 2, attr_normal, 0, 0, 1,
	 /* TRANSLATORS: "Price per share" is the cost of each share in
	    the current company. */
	 pgettext("label|Stock A", "Price per share: "));
    right(curwin, 5, width + SHARE_PRICE_COLS + 2, attr_normal, attr_highlight,
	  0, 1, "^{%N^}", company[num].share_price);

    left(curwin, 6, 2, attr_normal, 0, 0, 1,
	 /* TRANSLATORS: "Return" is the share return as a percentage. */
	 pgettext("label|Stock A", "Return:          "));
    right(curwin, 6, width + SHARE_PRICE_COLS + 2, attr_normal, attr_highlight,
	  0, 1, "^{%.2f%%^}", company[num].share_return * 100.0);

    mkchstr(chbuf, BUFSIZE, attr_normal, 0, 0, 1, w / 2, &width, 1,
	    /* TRANSLATORS: "Current holdings" is the number of shares
	       the current player owns in this particular company.

	       Note that the labels "Current holdings", "Percentage owned"
	       and "Current cash" MUST all be the same length and contain at
	       least one trailing space for the display routines to work
	       correctly.  The maximum length of each label is 18
	       characters. */
	    pgettext("label|Stock B", "Current holdings: "));
    mid = MIN(w / 2, w - width - TRADE_VALUE_COLS - 4);

    leftch(curwin, 3, mid, chbuf, 1, &width);
    right(curwin, 3, w - 2, attr_normal, attr_highlight, 0, 1, " ^{%'ld^} ",
	  player[current_player].stock_owned[num]);

    left(curwin, 4, mid, attr_normal, 0, 0, 1,
	 /* TRANSLATORS: "Percentage owned" is the current player's
	    percentage ownership in this particular company. */
	 pgettext("label|Stock B", "Percentage owned: "));
    right(curwin, 4, w - 2, attr_normal, attr_highlight, 0, 1, " ^{%.2f%%^} ",
	  ownership * 100.0);

    left(curwin, 6, mid, attr_highlight, 0, 0, 1,
	 pgettext("label|Stock B", "Current cash:     "));
    whline(curwin, ' ' | attr_title, TRADE_VALUE_COLS + 2);
    right(curwin, 6, w - 2, attr_title, 0, 0, 1, " %N ",
	  player[current_player].cash);

    wrefresh(curwin);

    // Show menu of choices for the player
    newtxwin(7, WIN_COLS - 4, 14, WCENTER, true, attr_normal_window);

    left(curwin, 3, 2, attr_normal, attr_keycode, 0, 1,
	     /* TRANSLATORS: Each label may be up to 35 characters wide
		(for <1> and <2>) or 36 characters wide (for <3> and <4>). */
	 _("^{<1>^} Buy stock from company"));
    left(curwin, 4, 2, attr_normal, attr_keycode, 0, 1,
	 _("^{<2>^} Sell stock back to company"));
    left(curwin, 3, getmaxx(curwin) / 2, attr_normal, attr_keycode, 0, 1,
	 _("^{<3>^} Bid company to issue more shares"));
    left(curwin, 4, getmaxx(curwin) / 2, attr_normal, attr_keycode, 0, 1,
	 _("^{<4>^} Exit to the Stock Exchange"));

    center(curwin, 1, 0, attr_normal, attr_keycode, 0, 1,
	   _("Enter selection [^{1^}-^{4^}]: "));

    curs_set(CURS_ON);
    wrefresh(curwin);

    done = false;
    while (! done) {
	if (gettxchar(curwin, &key) == OK) {
	    // Ordinary wide character
	    switch (key) {
	    case L'1':
	    case L'2':
	    case L'3':
	    case L'4':
		left(curwin, getcury(curwin), getcurx(curwin), A_BOLD,
		     0, 0, 1, "%lc", key);
		wrefresh(curwin);

		done = true;
		break;

	    case L' ':
		done = true;
		break;

	    default:
		beep();
	    }
	} else {
	    // Function or control key
	    switch (key) {
	    case KEY_ESC:
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
    }

    curs_set(CURS_OFF);

    switch (key) {
    case L'1':
	// Buy stock in company
	maxshares = player[current_player].cash / company[num].share_price;

	if (company[num].max_stock - company[num].stock_issued == 0) {
	    txdlgbox(MAX_DLG_LINES, 50, 8, WCENTER, attr_error_window,
		     attr_error_title, attr_error_highlight, 0, 0,
		     attr_error_waitforkey, _("  No Shares Available  "),
		     _("No more shares are available for purchase."));
	} else if (maxshares <= 0) {
	    txdlgbox(MAX_DLG_LINES, 50, 8, WCENTER, attr_error_window,
		     attr_error_title, attr_error_highlight, 0, 0,
		     attr_error_waitforkey, _("  Insufficient Cash  "),
		     _("You do not have enough cash\n"
		       "to purchase additional shares."));
	} else {
	    maxshares = MIN(maxshares, company[num].max_stock -
			    company[num].stock_issued);

	    wbkgdset(curwin, attr_normal_window);
	    werase(curwin);
	    box(curwin, 0, 0);

	    center(curwin, 2, 0, attr_normal, attr_highlight, 0, 1,
		   ngettext("You can purchase ^{one^} share.",
			    "You can purchase up to ^{%'ld^} shares.",
			    maxshares), maxshares);

	    mkchstr(chbuf, BUFSIZE, attr_normal, 0, 0, 1,
		    getmaxx(curwin) - TRADE_INPUT_COLS - 4, &width, 1,
		    _("How many shares do you wish to purchase? "));
	    x = (getmaxx(curwin) + width - TRADE_INPUT_COLS) / 2;
	    rightch(curwin, 4, x, chbuf, 1, &width);

	    ret = gettxlong(curwin, &val, 0, maxshares, 0, maxshares, 4, x,
			    TRADE_INPUT_COLS, attr_input_field);

	    if (ret == OK) {
		player[current_player].cash -= val * company[num].share_price;
		player[current_player].stock_owned[num] += val;
		company[num].stock_issued += val;
	    }
	}
	break;

    case L'2':
	// Sell stock back to company
	maxshares = player[current_player].stock_owned[num];
	if (maxshares == 0) {
	    txdlgbox(MAX_DLG_LINES, 50, 8, WCENTER, attr_error_window,
		     attr_error_title, attr_error_highlight, 0, 0,
		     attr_error_waitforkey, _("  No Shares  "),
		     _("You do not have any shares to sell."));
	} else {
	    wbkgdset(curwin, attr_normal_window);
	    werase(curwin);
	    box(curwin, 0, 0);

	    center(curwin, 2, 0, attr_normal, attr_highlight, 0, 1,
		   ngettext("You can sell ^{one^} share.",
			    "You can sell up to ^{%'ld^} shares.",
			    maxshares), maxshares);

	    mkchstr(chbuf, BUFSIZE, attr_normal, 0, 0, 1,
		    getmaxx(curwin) - TRADE_INPUT_COLS - 4, &width, 1,
		    _("How many shares do you wish to sell? "));
	    x = (getmaxx(curwin) + width - TRADE_INPUT_COLS) / 2;
	    rightch(curwin, 4, x, chbuf, 1, &width);

	    ret = gettxlong(curwin, &val, 0, maxshares, 0, maxshares, 4, x,
			    TRADE_INPUT_COLS, attr_input_field);

	    if (ret == OK) {
		company[num].stock_issued -= val;
		player[current_player].stock_owned[num] -= val;
		player[current_player].cash += val * company[num].share_price;
	    }
	}
	break;

    case L'3':
	// Bid company to issue more shares
	maxshares = 0;
	if (! *bid_used && randf() < ownership && randf() < BID_CHANCE) {
	    maxshares = randf() * ownership * MAX_SHARES_BIDDED;
	    company[num].max_stock += maxshares;
	}

	*bid_used = true;

	if (maxshares == 0) {
	    txdlgbox(MAX_DLG_LINES, 50, 8, WCENTER, attr_error_window,
		     attr_error_title, attr_error_highlight, 0, 0,
		     attr_error_waitforkey, _("  No Shares Issued  "),
		     /* TRANSLATORS: %ls represents the company name. */
		     _("%ls has refused\nto issue more shares."),
		     company[num].name);
	} else {
	    txdlgbox(MAX_DLG_LINES, 50, 8, WCENTER, attr_normal_window,
		     attr_title, attr_normal, attr_highlight, 0,
		     attr_waitforkey, _("  Shares Issued  "),
		     /* TRANSLATORS: %ls represents the company name. */
		     ngettext("%ls has issued\n^{one^} more share.",
			      "%ls has issued\n^{%'ld^} more shares.",
			      maxshares), company[num].name, maxshares);
	}
	break;

    default:
	break;
    }

    deltxwin();			// "Enter selection" window
    deltxwin();			// Stock Transaction window
    txrefresh();

    free(chbuf);
}


/***********************************************************************/
// End of file
