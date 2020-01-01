/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2020, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, move.h, contains declarations for functions that make and
  process a game move in Star Traders.


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


#ifndef included_MOVE_H
#define included_MOVE_H 1


/************************************************************************
*                     Game move function prototypes                     *
************************************************************************/

/*
  Function:   select_moves - Select NUMBER_MOVES random moves
  Parameters: (none)
  Returns:    (nothing)

  This function selects NUMBER_MOVES random moves and stores them in the
  game_move[] array.  If there are less than NUMBER_MOVES empty spaces in
  the galaxy map, the game is automatically finished by setting
  quit_selected to true.
*/
extern void select_moves (void);


/*
  Function:   get_move    - Wait for the player to enter their move
  Parameters: (none)
  Returns:    selection_t - Choice selected by player

  This function displays the galaxy map and the current moves, then waits
  for the player to select one of the moves.  On entry, current_player
  contains the current player number; quit_selected and/or abort_game may
  be true (if so, get_move() just returns SEL_QUIT without waiting for
  the player to select a move).  The return value is the choice made by
  the player.

  Note that two windows (the "Select move" window and the galaxy map
  window) are left on the screen: they are closed in process_move().
*/
extern selection_t get_move (void);


/*
  Function:   process_move - Process the move selected by the player
  Parameters: selection    - Selection made by current player
  Returns:    (nothing)

  This function processes the move in selection.  It assumes the "Select
  move" and galaxy map windows are still open.  In particular, this
  function tries to start new companies, merge companies, bankrupt
  companies and/or players, adjust values, etc.
*/
extern void process_move (selection_t selection);


/*
  Function:   next_player - Get the next player
  Parameters: (none)
  Returns:    (nothing)

  This function sets the global variable current_player to the next
  eligible player.  If no player is still in the game, quit_selected is
  set to true.  The variable turn_number is also incremented if required.
*/
extern void next_player (void);


#endif /* included_MOVE_H */
