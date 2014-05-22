/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2014, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, exch.h, contains declarations for functions dealing with the
  Interstellar Stock Exchange and Trading Bank as used in Star Traders.


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


#ifndef included_EXCH_H
#define included_EXCH_H 1


/************************************************************************
*                  Stock Exchange function prototypes                   *
************************************************************************/

/*
  Function:   exchange_stock - Visit the Interstellar Stock Exchange
  Parameters: (none)
  Returns:    (nothing)

  This function allows the current player (in current_player) to buy,
  sell and bid for shares in companies that appear on the galaxy map.  If
  either quit_selected or abort_game is true, or the current player is
  not in the game, this function does nothing.
*/
extern void exchange_stock (void);


#endif /* included_EXCH_H */
