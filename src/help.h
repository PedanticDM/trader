/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2014, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, help.h, contains declarations for help functions as used in
  Star Traders.


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


#ifndef included_HELP_H
#define included_HELP_H 1


/************************************************************************
*                     Help text function prototypes                     *
************************************************************************/

/*
  Function:   show_help - Show instructions on how to play the game
  Parameters: (none)
  Returns:    (nothing)

  This function displays instructions on how to play Star Traders in a
  Curses window.  It does not depend on any global game variables other
  than printable_map_val[] and printable_game_move[].  On exit, the
  previous screen is restored and refreshed.
*/
extern void show_help (void);


#endif /* included_HELP_H */
