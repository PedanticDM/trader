/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, fileio.h, contains declarations for the load and save game
  functions used in Star Traders.


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


#ifndef included_FILEIO_H
#define included_FILEIO_H 1


/************************************************************************
*                Game load and save function prototypes                 *
************************************************************************/

/*
  Function:   load_game - Load a previously-saved game from disk
  Parameters: num       - Game number to load (1-9)
  Returns:    bool      - True if game loaded successfully, else false

  This function loads a previously-saved game from disk, initialising all
  game global variables appropriately.  True is returned if this could be
  done successfully.
*/
extern bool load_game (int num);


/*
  Function:   save_game - Save the current game to disk
  Parameters: num       - Game number to use (1-9)
  Returns:    bool      - True if game saved successfully, else false

  This function saves the current game to disk.  True is returned if this
  could be done successfully.
*/
extern bool save_game (int num);


#endif /* included_FILEIO_H */
