/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2021, John Zaitseff                 *
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
  along with this program.  If not, see https://www.gnu.org/licenses/.
*/


#ifndef included_GLOBALS_H
#define included_GLOBALS_H 1


/************************************************************************
*                            Game constants                             *
************************************************************************/

#define MAX_X			38	// Map dimensions MAX_X x MAX_Y (do not change!)
#define MAX_Y			14
#define STAR_RATIO		0.10	// Approximately 10% of the map should be stars

#define NUMBER_MOVES		20	// Number of choices on the galaxy map per turn
#define DEFAULT_MAX_TURN	50	// Default number of turns per game
#define MIN_MAX_TURN		10	// Minimum that can be specified for max_turn

#define MAX_PLAYERS		8	// Maximum number of players (do not change!)
#define INITIAL_CASH		6000.00	// Initial cash per player
#define MAX_OVERDRAFT		1000.00	// Maximum value any player can go negative
#define MAKE_BANKRUPT		0.07	// If a player is overdraft, 7% chance of bankruptcy

#define MAX_COMPANIES		8	// Maximum number of companies (do not change!)
#define INITIAL_STOCK_ISSUED	5	// Initial number of shares issued on company creation
#define INITIAL_MAX_STOCK	50	// Initial maximum number of shares available
#define INITIAL_SHARE_PRICE	60.00	// Initial share price (before increments)

#define SHARE_PRICE_INC		60.00	// Share price increment for increase in shipping
#define SHARE_PRICE_INC_OUTPOST	75.00	// Increment for adding an outpost
#define SHARE_PRICE_INC_OUTSTAR	150.00	// Extra increment for adding an outpost next to a star
#define SHARE_PRICE_INC_STAR	300.00	// Increment for expanding directly next to a star
#define PRICE_INC_ADJUST_MIN	0.75	// Actual increment may be as low as 75% of nominal
#define PRICE_INC_ADJUST_MAX	1.25	// Actual increment may be as high as 125%
#define MAX_STOCK_RATIO_MIN	0.10	// Minimum multiplier to increment available shares
#define MAX_STOCK_RATIO_MAX	0.25	// Maximum multiplier, using share price increment

#define MERGE_STOCK_RATIO	0.50	// 50% of old shares are credited to new company
#define MERGE_BONUS_RATE	10.0	// Multiplier for merger cash bonus
#define MERGE_PRICE_ADJUST_MIN	0.40	// Minimum of old share price used as increment
#define MERGE_PRICE_ADJUST_MAX	0.60	// Maximum of old share price used as increment
#define COMPANY_BANKRUPTCY	0.07	// 7% chance of company bankruptcy (if return < 0)
#define ALL_ASSETS_TAKEN	0.20	// 20% chance of all assets taken in that case

#define CHANGE_SHARE_PRICE	0.40	// Chance of changing a share price each move
#define DEC_SHARE_PRICE		0.65	// Chance that change will be negative
#define PRICE_CHANGE_RATE	0.25	// Up to 25% of share price is used to increment/decrement
#define INITIAL_RETURN		0.05	// Initial return per share: 5%
#define MIN_COMPANY_RETURN	-0.15	// Minimum return per share (yes, negative!)
#define MAX_COMPANY_RETURN	0.25	// Maximum return per share
#define CHANGE_COMPANY_RETURN	0.60	// Chance of randomly changing a return each move
#define DEC_COMPANY_RETURN	0.45	// Chance such a change will decrease return
#define RETURN_MAX_CHANGE	0.04	// Maximum that company return will change
#define CHANGE_RETURN_GROWING	0.35	// Chance of changing return when the company grows
#define DEC_RETURN_GROWING	0.30	// Chance such a change will decrement return
#define GROWING_MAX_CHANGE	0.03	// Maximum that that return can change

#define OWNERSHIP_BONUS		2.00	// Bonus payment based on percentage ownership
#define BID_CHANCE		0.70	// 70% chance of successful bidding
#define MAX_SHARES_BIDDED	250	// Maximum number of shares issued

#define INITIAL_INTEREST_RATE	0.10	// Initial bank interest rate: 10%
#define MIN_INTEREST_RATE	0.02	// Minimum interest rate
#define MAX_INTEREST_RATE	0.20	// Maximum interest rate
#define CHANGE_INTEREST_RATE	0.35	// Chance of changing the interest rate each move
#define DEC_INTEREST_RATE	0.45	// Chance that change will be a decrease
#define INTEREST_MAX_CHANGE	0.03	// Maximum that interest rate will change
#define CREDIT_LIMIT_RATE	2.00	// Multiplier for a player's credit limit

#define ROUNDING_AMOUNT		0.01	// Round off smaller amounts to zero


/************************************************************************
*                        Game type declarations                         *
************************************************************************/

// Information about each company
typedef struct company_info {
    wchar_t	*name;			// Company name
    double	share_price;		// Share price
    double	share_return;		// Return per share (may be negative)
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


// Information about a move
typedef struct move_rec {
    int x;
    int y;
} move_rec_t;


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
