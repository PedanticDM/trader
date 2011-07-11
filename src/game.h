/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, game.h, contains declarations for the main game functions
  used in Star Traders.


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

void init_game (void);
void end_game (void);

bool load_game (int num);
bool save_game (int num);

void select_moves (void);
void get_move (void);
void process_move (void);
void exchange_stock (void);
void next_player (void);


#endif /* included_GAME_H */
