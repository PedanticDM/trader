/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2011, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, utils.c, contains the implementation of various utility
  function for Star Traders.


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
#include "utils.h"


/************************************************************************
*                           Module variables                            *
************************************************************************/

static char *program_name_str   = NULL;		// Canonical program name
static char *home_directory_str = NULL;		// Full pathname to home
static char *data_directory_str = NULL;		// Writable data dir pathname


/************************************************************************
*                     Utility function definitions                      *
************************************************************************/


/*-----------------------------------------------------------------------
  Function:   init_program_name  - Make the program name "canonical"
  Arguments:  argv               - Same as passed to main()
  Returns:    (nothing)

  This function modifies the argv[0] pointer to eliminate the leading
  path name from the program name.  In other words, it strips any leading
  directories and leaves just the base name of the program.  It also
  assigns the module variable "program_name_str".
*/

void init_program_name (char *argv[])
{
    if ((argv == NULL) || (argv[0] == NULL) || (*(argv[0]) == '\0')) {
	program_name_str = PACKAGE;
    } else {
	char *p = strrchr(argv[0], '/');

	if ((p != NULL) && (*++p != '\0')) {
	    argv[0] = p;
	}

	program_name_str = argv[0];
    }
}


/*-----------------------------------------------------------------------
  Function:   program_name  - Return the canonical program name
  Arguments:  (none)
  Returns:    const char *  - Pointer to program name

  This function returns the canonical program name (the program name as
  invoked on the command line, without any leading pathname).  NULL
  should never be returned.
*/

const char *program_name (void)
{
    if (program_name_str == NULL) {
	init_program_name(NULL);
    }

    return program_name_str;
}


/*-----------------------------------------------------------------------
  Function:   home_directory  - Return home directory pathname
  Arguments:  (none)
  Returns:    const char *    - Pointer to home directory

  This function returns the full pathname to the user's home directory,
  using the HOME environment variable.  Note that the existance or
  writability of this pathname is NOT checked by this function.  NULL is
  returned if the home directory cannot be determined.
*/

const char *home_directory (void)
{
    if (home_directory_str == NULL) {
	// Use the HOME environment variable where possible
	char *p = getenv("HOME");
	if ((p != NULL) && (*p != '\0')) {
	    home_directory_str = strdup(p);
	}
    }

    return home_directory_str;
}


/*-----------------------------------------------------------------------
  Function:   data_directory  - Return writable data directory pathname
  Arguments:  (none)
  Returns:    const char *    - Pointer to data directory

  This function returns the full pathname to a writable subdirectory
  within the user's home directory.  Essentially, home_directory() + "/."
  + program_name() is returned.  Note that this path is NOT created by
  this function, nor is the writability of this path checked.  NULL is
  returned if this path cannot be determined.
*/

const char *data_directory (void)
{
    if (data_directory_str == NULL) {
	const char *name = program_name();
	const char *home = home_directory();
	if ((name != NULL) && (home != NULL)) {
	    char *p = malloc(strlen(home) + strlen(name) + 3);
	    if (p != NULL) {
		strcpy(p, home);
		strcat(p, "/.");
		strcat(p, name);
		data_directory_str = p;
	    }
	}
    }

    return data_directory_str;
}
