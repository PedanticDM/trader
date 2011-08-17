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


#include "system.h"
#include "globals.h"


/************************************************************************
*                         Constant definitions                          *
************************************************************************/

// Company names
const char *company_name[MAX_COMPANIES] = {
    /* TRANSLATORS: The eight company names do NOT have to be literal
       translations of the English names.  In fact, if possible, the
       names should start with successive letters of your alphabet (in
       English, "A" to "H").  No company name should be more than 24
       characters (column positions, to be precise) long. */
    N_("Altair Starways"),
    N_("Betelgeuse, Ltd"),
    N_("Capella Freight Co"),
    N_("Denebola Shippers"),
    N_("Eridani Expediters"),
    N_("Fornax Express"),
    N_("Gemeni Inc"),
    N_("Hercules and Co")
};


// Ordinal strings
const char *ordinal[MAX_PLAYERS + 1] = {
    "",
    /* TRANSLATORS: The ordinal strings "1st" to "8th" are used in the
       Game Winner dialog box at the end of the game.  If ordinals depend
       on the gender of the player, it may be simpler to list cardinal
       numbers instead (eg, "No. 1").  Up to five characters are allowed
       (see ORDINAL_COLS in src/intf.h). */
    N_("1st"),
    N_("2nd"),
    N_("3rd"),
    N_("4th"),
    N_("5th"),
    N_("6th"),
    N_("7th"),
    N_("8th")
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
