/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, globals.h, contains definitions for global variables and
  structures for Star Traders.


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


#include <stdbool.h>


/************************************************************************
*                    Constants and type declarations                    *
************************************************************************/

#define MAX_X			(38)	/* Map dimensions MAP_X x MAP_Y */
#define MAX_Y			(12)
#define STAR_RATIO		(0.10)	/* 10% of map should be stars */

#define NUMBER_MOVES		(20)	/* Number of choices on map per turn */
#define DEFAULT_MAX_TURN	(50)	/* Default number of turns per game */

#define MAX_PLAYERS		(8)	    /* Maximum number of players */
#define INITIAL_CASH		(6000.00)   /* Initial cash per player */
#define MAX_OVERDRAFT		(1000.00)   /* Max. value player can go negative */
#define PROB_BANKRUPTCY		(0.07)	    /* If overdraft, 7% chance of bankruptcy */

#define MAX_COMPANIES		(8)	    /* Max. number of companies (must be <= 26) */
#define INITIAL_SHARE_PRICE	(60.00)	    /* Initial share price */
#define SHARE_PRICE_INC		(60.00)	    /* Share price incr. for increase in shipping */
#define SHARE_PRICE_INC_OUTPOST	(70.00)	    /* Incr. for adding an outpost */
#define SHARE_PRICE_INC_STAR	(300.00)    /* Incr. for adding next to star */
#define INC_SHARE_PRICE		(0.30)	    /* 30% chance for increasing share price */
#define DEC_SHARE_PRICE		(0.65)	    /* 65% x 30% chance of decrementing same */
#define INITIAL_RETURN		(0.05)	    /* Initial return per share: 5% */
#define PROB_INC_RETURN		(0.25)	    /* Chance of incr. return when company grows */
#define INC_COMPANY_RETURN	(0.40)	    /* Chance of randomly incrementing return */
#define MAX_COMPANY_RETURN	(0.40)	    /* Maximum return per share */
#define INITIAL_STOCK_ISSUED	(5)	    /* Initial number of shares issued out */
#define INITIAL_MAX_STOCK	(50)	    /* Initial max. number of shares available */
#define COMPANY_BANKRUPTCY	(0.01)	    /* 1% chance of company bankruptcy */
#define ALL_ASSETS_TAKEN	(0.20)	    /* 20% chance of assets taken of same */
#define BID_CHANCE		(0.75)	    /* 75% chance of successful bidding */
#define MAX_SHARES_BIDDED	(200)	    /* Max. number of shares issued */

#define INITIAL_INTEREST_RATE	(0.10)	/* Initial bank interest rate: 10% */
#define INC_INTEREST_RATE	(0.30)	/* 30% chance of incr./decr. interest rate */
#define MAX_INTEREST_RATE	(0.30)	/* Maximum interest rate */


// Information about each company
typedef struct company_info {
    char	*name;			// Company name
    double	share_price;		// Share price
    double	share_return;		// Return per share
    long	stock_issued;		// Total stock sold to players
    long	max_stock;		// Max. stock that company has
    bool	on_map;			// True if company on map
} company_info_t;


// Information about each player
typedef struct player_info {
    char	*name;			// Player name
    double	cash;			// Cash available
    double	debt;			// Amount of debt
    long	stock_owned[MAX_COMPANIES];	// How much stock is owned
    bool	in_game;		// True if still in the game
} player_info_t;


// Map values
typedef enum map_val {
    MAP_EMPTY	= '.',			// Empty space
    MAP_OUTPOST	= '+',			// Unowned outpost
    MAP_STAR	= '*',			// Star
    MAP_A	= 'A',			// Company A, etc
    MAP_LAST	= MAP_A + MAX_COMPANIES - 1
} map_val_t;

#define MAP_COMPANY(x) (MAP_A + x)


// Information about a move
typedef struct move_rec {
    int x;
    int y;
} move_rec_t;


// Company names
extern const char *company_names[MAX_COMPANIES];


/************************************************************************
*                     Global variable declarations                      *
************************************************************************/

extern company_info_t	company[MAX_COMPANIES];		// Array of companies
extern player_info_t	player[MAX_PLAYERS];		// Array of players
extern map_val_t	galaxy_map[MAX_X][MAX_Y];	// Map of the galaxy
extern move_rec_t	game_move[NUMBER_MOVES];	// Current moves

extern double	interest_rate;		// Current interest rate
extern double	credit_limit;		// Credit limit of current player
extern bool	bid_used;		// True if bid used for player

extern int	max_turn;		// Max. number of turns in game
extern int	turn_number;
extern int	current_player;
extern int	number_players;
extern int	first_player;		// Who WAS the first player to go?

extern bool	game_loaded;		// True if game was loaded from disk
extern char	*game_filename;		// Game file filename


#endif /* included_GLOBALS_H */
