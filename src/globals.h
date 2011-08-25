/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, globals.h, contains declarations for global variables and
  structures used in Star Traders.


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


#ifndef included_GLOBALS_H
#define included_GLOBALS_H 1


#include <system.h>


/************************************************************************
*                            Game constants                             *
************************************************************************/

#define MAX_X			38	// Map dimensions MAX_X x MAX_Y
#define MAX_Y			14
#define STAR_RATIO		0.10	// 10% of map should be stars

#define NUMBER_MOVES		20	// Number of choices on map per turn
#define DEFAULT_MAX_TURN	50	// Default number of turns per game
#define MIN_MAX_TURN		10	// Minimum that can be specified for max_turn

#define MAX_PLAYERS		8	// Maximum number of players
#define INITIAL_CASH		6000.00	// Initial cash per player
#define MAX_OVERDRAFT		1000.00	// Max. value player can go negative
#define PROB_BANKRUPTCY		0.07	// If overdraft, 7% chance of bankruptcy

#define MAX_COMPANIES		8	// Max. number of companies (do not change!)
#define INITIAL_STOCK_ISSUED	5	// Initial number of shares issued out
#define INITIAL_MAX_STOCK	50	// Initial max. number of shares available
#define INITIAL_SHARE_PRICE	60.00	// Initial share price

#define SHARE_PRICE_INC		60.00	// Share price incr. for increase in shipping
#define SHARE_PRICE_INC_OUTPOST	70.00	// Incr. for adding an outpost
#define SHARE_PRICE_INC_OUTSTAR	70.00	// Extra incr. for adding outpost next to star
#define SHARE_PRICE_INC_STAR	300.00	// Incr. for adding next to star
#define SHARE_PRICE_INC_EXTRA	0.50	// Extra factor when incr. share price

#define MERGE_STOCK_RATIO	0.50	// 50% of old shares are credited to new company
#define COMPANY_BANKRUPTCY	0.01	// 1% chance of company bankruptcy
#define ALL_ASSETS_TAKEN	0.20	// 20% chance of assets taken of same

#define INC_SHARE_PRICE		0.30	// 30% chance for increasing share price
#define DEC_SHARE_PRICE		0.65	// 65% x 30% chance of decrementing same
#define PRICE_CHANGE_RATE	0.25	// Up to 25% of share price is used to incr./decr.
#define INITIAL_RETURN		0.05	// Initial return per share: 5%
#define GROWING_RETURN_CHANGE	0.25	// Chance of changing return when company grows
#define GROWING_RETURN_INC	0.60	// 60% chance such change will increase return
#define CHANGE_COMPANY_RETURN	0.40	// Chance of randomly changing return
#define COMPANY_RETURN_INC	0.75	// 75% chance such change will increase return
#define MAX_COMPANY_RETURN	0.40	// Maximum return per share
#define RETURN_DIVIDER		1.50	// Min. amount by which to divide if return too large
#define OWNERSHIP_BONUS		2.00	// Bonus amount based on percentage ownership
#define BID_CHANCE		0.75	// 75% chance of successful bidding
#define MAX_SHARES_BIDDED	200	// Max. number of shares issued

#define INITIAL_INTEREST_RATE	0.10	// Initial bank interest rate: 10%
#define CHANGE_INTEREST_RATE	0.30	// 30% chance of changing interest rate
#define INTEREST_RATE_INC	0.65	// 65% chance of above being an increment
#define MAX_INTEREST_RATE	0.30	// Maximum interest rate
#define INTEREST_RATE_DIVIDER	1.50	// Min. amount by which to divide if interest is too high
#define CREDIT_LIMIT_RATE	2.00	// Multiplier for credit limit

#define ROUNDING_AMOUNT		0.01	// Round off smaller amounts to zero


/************************************************************************
*                        Game type declarations                         *
************************************************************************/

// Information about each company
typedef struct company_info {
    wchar_t	*name;			// Company name
    double	share_price;		// Share price
    double	share_return;		// Return per share
    long int	stock_issued;		// Total stock sold to players
    long int	max_stock;		// Max. stock that company has
    bool	on_map;			// True if company on map
} company_info_t;


// Information about each player
typedef struct player_info {
    wchar_t	*name;			// Player name
    char	*name_utf8;		// Player name (in UTF-8, for load/save)
    double	cash;			// Cash available
    double	debt;			// Amount of debt
    long int	stock_owned[MAX_COMPANIES];	// How much stock is owned
    bool	in_game;		// True if still in the game
    double	sort_value;		// Total value (only used in end_game())
} player_info_t;


// Map values
typedef enum map_val {
    MAP_EMPTY	= '.',			// Empty space
    MAP_OUTPOST	= '+',			// Unowned outpost
    MAP_STAR	= '*',			// Star
    MAP_A	= 'A',			// Company A, etc
    MAP_LAST	= MAP_A + MAX_COMPANIES - 1
} map_val_t;

#define COMPANY_TO_MAP(i)	((i) + MAP_A)
#define MAP_TO_COMPANY(m)	((m) - MAP_A)
#define IS_MAP_COMPANY(m)	((m) >= MAP_A && (m) <= MAP_LAST)

#define MAP_TO_INDEX(m)							\
    (((m) == MAP_EMPTY)   ? 0 :						\
    (((m) == MAP_OUTPOST) ? 1 :						\
    (((m) == MAP_STAR)    ? 2 :						\
			    ((m) - MAP_A + 3))))

#define PRINTABLE_MAP_VAL(m)	printable_map_val[MAP_TO_INDEX(m)]
#define CHTYPE_MAP_VAL(m)	chtype_map_val[MAP_TO_INDEX(m)]


// Information about a move
typedef struct move_rec {
    int x;
    int y;
} move_rec_t;

#define PRINTABLE_GAME_MOVE(m)	(printable_game_move[m])
#define CHTYPE_GAME_MOVE(m)	(chtype_game_move[m])


// Player moves / selection values
typedef enum selection {
    SEL_COMPANY_FIRST	= 0,
    SEL_COMPANY_LAST	= MAX_COMPANIES - 1,

    SEL_MOVE_FIRST	= 0,
    SEL_MOVE_LAST	= NUMBER_MOVES - 1,

    SEL_BANKRUPT,			// Player wishes to give up
    SEL_SAVE,				// Save and end the game
    SEL_QUIT,				// Just end the game

    SEL_BANK,				// Visit the Trading Bank
    SEL_EXIT,				// Exit the Stock Exchange

    SEL_NONE		= -1		// Nothing yet selected
} selection_t;


// Company names
extern const char *company_name[MAX_COMPANIES];

// Default keycodes (keyboard input characters) for each company
extern const char *default_keycode_company;

// Default keycodes (keyboard input characters) for each move
extern const char *default_keycode_game_move;

// Default printable output representations for each map element
extern const char *default_printable_map_val;

// Default printable output representations for each move
extern const char *default_printable_game_move;

// Ordinal strings
extern const char *ordinal[MAX_PLAYERS + 1];


/************************************************************************
*                     Global variable declarations                      *
************************************************************************/

extern company_info_t	company[MAX_COMPANIES];		// Array of companies
extern player_info_t	player[MAX_PLAYERS];		// Array of players
extern map_val_t	galaxy_map[MAX_X][MAX_Y];	// Map of the galaxy
extern move_rec_t	game_move[NUMBER_MOVES];	// Current moves

extern wchar_t	*keycode_company;	// Keycodes for each company
extern wchar_t	*keycode_game_move;	// Keycodes for each game move
extern wchar_t	*printable_map_val;	// Printable output for each map value
extern wchar_t	*printable_game_move;	// Printable output for each game move
extern chtype	*chtype_map_val[MAX_COMPANIES + 3];	// as chtype strings
extern chtype	*chtype_game_move[NUMBER_MOVES];	// as chtype strings

extern int	max_turn;		// Max. number of turns in game
extern int	turn_number;		// Current turn (1 to max_turn)
extern int	number_players;		// Number of players
extern int	current_player;		// Current player (0 to number_players-1)
extern int	first_player;		// Who WAS the first player to go?

extern double	interest_rate;		// Current interest rate

extern bool	game_loaded;		// True if game was loaded from disk
extern int	game_num;		// Game number (1-9)

extern bool	quit_selected;		// Is a player trying to quit the game?
extern bool	abort_game;		// Abort game without declaring winner?

extern bool	option_no_color;	// True if --no-color was specified
extern bool	option_dont_encrypt;	// True if --dont-encrypt was specified
extern int	option_max_turn;	// Max. turns if --max-turn was specified


#endif /* included_GLOBALS_H */
