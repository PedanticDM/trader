/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2016, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, trader.h, contains overall definitions for Star Traders.
  This allows source files to include just one file: this one.


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


#ifndef included_TRADER_H
#define included_TRADER_H 1


/************************************************************************
*                         Included header files                         *
************************************************************************/

#include "system.h"		// System header files

#include "globals.h"		// Global game constants and variables
#include "game.h"		// Game start, end and display functions
#include "move.h"		// Making and processing a move
#include "exch.h"		// Stock Exchange and Bank functions
#include "fileio.h"		// Load and save game file functions
#include "help.h"		// Help text functions: how to play
#include "intf.h"		// Basic text input/output functions
#include "utils.h"		// Utility functions needed by Star Traders


/************************************************************************
*                          Global definitions                           *
************************************************************************/

#define GAME_FILE_HEADER	"Star Traders Saved Game"
#define GAME_FILE_API_VERSION	"File API 7.5"	// For game loads and saves
#define GAME_FILE_SENTINEL	42		// End of game file sentinel

#ifdef USE_UTF8_GAME_FILE
#  define GAME_FILE_CHARSET	"UTF-8"		// For strings in game file
#  define GAME_FILE_TRANSLIT	"//TRANSLIT"	// Transliterate (GNU libiconv)
#endif

#define BUFSIZE			1024	// For various string buffers
#define BIGBUFSIZE		2048	// For buffers known to be larger


#endif /* included_TRADER_H */
