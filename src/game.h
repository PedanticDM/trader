/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, game.h, contains declarations for the starting and ending
  game functions used in Star Traders.


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


#ifndef included_GAME_H
#define included_GAME_H 1


#include <stdbool.h>


/************************************************************************
*                      Game function declarations                       *
************************************************************************/

extern void init_game (void);
extern void end_game (void);

extern void select_moves (void);
extern void get_move (void);
extern void process_move (void);
extern void exchange_stock (void);
extern void next_player (void);

extern void show_map (bool show_moves);
extern void show_status (int num);
extern double total_value (int num);


#endif /* included_GAME_H */
