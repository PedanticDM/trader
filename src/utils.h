/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, utils.h, contains various utility function declarations for
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


#ifndef included_UTILS_H
#define included_UTILS_H 1


/************************************************************************
*                    Constants and type declarations                    *
************************************************************************/

#define GAME_FILENAME_PROTO	"game%d"
#define GAME_FILENAME_BUFSIZE	(16)


/************************************************************************
*                       Utility macro definitions                       *
************************************************************************/

// Type-unsafe minimum and maximum macros

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


/************************************************************************
*                     Utility function declarations                     *
************************************************************************/

// Initialisation and environment functions

extern void init_program_name (char *argv[]);
extern const char *program_name (void);

extern const char *home_directory (void);
extern const char *data_directory (void);

extern char *game_filename (const int game_num);

// Error-reporting functions

extern void err_exit   (const char *format, ...)
    __attribute__((noreturn, format (printf, 1, 2)));
extern void errno_exit (const char *format, ...)
    __attribute__((noreturn, format (printf, 1, 2)));

// Random number functions

extern void init_rand (void);
extern double randf (void);
extern int randi (int limit);


#endif /* included_UTILS_H */
