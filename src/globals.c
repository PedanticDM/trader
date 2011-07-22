/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, globals.c, contains the actual global variables and
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


#include "globals.h"


/************************************************************************
*                         Constant definitions                          *
************************************************************************/

// Company names
const char *company_name[MAX_COMPANIES] = {
    "Altair Starways",
    "Betelgeuse, Ltd",
    "Capella Freight Co",
    "Denebola Shippers",
    "Eridani Expediters",
    "Fornax Express",
    "Gemeni Inc",
    "Hercules and Co"
};


// Ordinal strings
const char *ordinal[MAX_PLAYERS + 1] = {
    "0th",
    "1st",
    "2nd",
    "3rd",
    "4th",
    "5th",
    "6th",
    "7th",
    "8th"
};


/************************************************************************
*                      Global variable definitions                      *
************************************************************************/

company_info_t	company[MAX_COMPANIES];		// Array of companies
player_info_t	player[MAX_PLAYERS];		// Array of players
map_val_t	galaxy_map[MAX_X][MAX_Y];	// Map of the galaxy
move_rec_t	game_move[NUMBER_MOVES];	// Current moves

int	max_turn;			// Max. number of turns in game
int	turn_number;			// Current turn (1 to max_turn)
int	number_players;			// Number of players
int	current_player;			// Current player (0 to number_players-1)
int	first_player;			// Who WAS the first player to go?

double	interest_rate;			// Current interest rate

bool	game_loaded	= false;	// True if game was loaded from disk
int	game_num	= 0;		// Game number (1-9)

bool	quit_selected	= false;	// Is a player trying to quit the game?
bool	abort_game	= false;	// Abort game without declaring winner?

bool	option_no_color     = false;	// True if --no-color was specified
bool	option_dont_encrypt = false;	// True if --dont-encrypt was specified
int	option_max_turn     = 0;	// Max. turns if --max-turn was specified


/***********************************************************************/
// End of file
