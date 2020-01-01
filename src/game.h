/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2020, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, game.h, contains declarations for the starting and ending
  game functions used in Star Traders.  It also contains prototypes of
  functions for displaying the galaxy map and the player's status.


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


#ifndef included_GAME_H
#define included_GAME_H 1


#include <stdbool.h>


/************************************************************************
*                       Game function prototypes                        *
************************************************************************/

/*
  Function:   init_game - Initialise a new game or load an old one
  Parameters: (none)
  Returns:    (nothing)

  This function initialises all game variables and structures, either by
  creating a new game or by loading an old one from disk.  In particular,
  if a new game is to be created, it asks how many people will play, and
  what their names are.  If needed, instructions on how to play the game
  are also displayed.

  On entry to this function, the global variable game_num determines
  whether an old game is loaded (if possible).  If option_max_turn
  contains a non-zero value, it is used to initialise max_turn.

  On exit, all global variables in globals.h are initialised, apart from
  game_move[] (and the previously-set option_XXX variables).  If the user
  aborts entering the necessary information, abort_game is set to true.
*/
extern void init_game (void);


/*
  Function:   end_game - Finish playing the current game
  Parameters: (none)
  Returns:    (nothing)

  This function displays every player's status before declaring the
  winner of the game.  Note that turn_number is used instead of max_turns
  as select_moves() may terminate the game earlier.
*/
extern void end_game (void);


/*
  Function:   show_map - Display the galaxy map on the screen
  Parameters: closewin - Wait for user, then close window if true
  Returns:    (nothing)

  This function displays the galaxy map on the screen, using the global
  variable galaxy_map[][] to do so.  If closewin is true, a prompt is
  shown for the user to press any key; the map window is then closed.  If
  closewin is false, no prompt is shown, wrefresh() is NOT called and the
  text window must be closed by the caller.
*/
extern void show_map (bool closewin);


/*
  Function:   show_status - Display the player's status
  Parameters: num         - Player number (0 to number_players - 1)
  Returns:    (nothing)

  This function displays the financial status of the player num, using
  the global variable player[num] to do so.  The show status window is
  closed before returning from this function.
*/
extern void show_status (int num);


/*
  Function:   total_value - Calculate a player's total financial worth
  Parameters: num         - Player number (0 to number_players - 1)
  Returns:    double      - Financial value of player

  This function calculates the total financial value (worth) of the
  player num, using the global variables player[num] and company[] to do
  so.
*/
extern double total_value (int num);


#endif /* included_GAME_H */
