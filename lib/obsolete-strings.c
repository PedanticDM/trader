/************************************************************************
*                                                                       *
*                Obsolete Strings From Various Libraries                *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file contains strings that are present in older versions of
  various libraries but are no longer present in files distributed as
  part of the GNU Portability Library.  This allows programs to provide
  translations for strings used in, for example, older versions of the
  GNU C Library as released with various Linux distributions.

  This file is NOT intended to be linked into any program.  Instead, it
  is simply meant to be listed in po/POTFILES.in.


  This file is free software: you can redistribute it and/or modify it
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


/************************************************************************
*                   From GNU C Library posix/getopt.c                   *
************************************************************************/

const char *glibc_posix_getopt_strings[] = {
    /* TRANSLATORS: These strings are used in older versions of various
       libraries, such as the GNU C Library as released with various
       Linux distributions. */
    N_("%s: option `%s%s' is ambiguous\n"),
    N_("%s: option '%s%s' is ambiguous\n"),
    N_("%s: option `%s%s' is ambiguous; possibilities:"),
    N_("%s: option '%s%s' is ambiguous; possibilities:"),
    N_("%s: unrecognized option `%s%s'\n"),
    N_("%s: unrecognized option '%s%s'\n"),
    N_("%s: option `%s%s' doesn't allow an argument\n"),
    N_("%s: option '%s%s' doesn't allow an argument\n"),
    N_("%s: option `%s%s' requires an argument\n"),
    N_("%s: option '%s%s' requires an argument\n"),
    N_("%s: invalid option -- `%c'\n"),
    N_("%s: invalid option -- '%c'\n"),
    N_("%s: option requires an argument -- `%c'\n"),
    N_("%s: option requires an argument -- '%c'\n"),

    N_("%s: option `%s' is ambiguous; possibilities:"),
    N_("%s: option '%s' is ambiguous; possibilities:"),
    N_("%s: option `%s' is ambiguous\n"),
    N_("%s: option '%s' is ambiguous\n"),
    N_("%s: option `--%s' doesn't allow an argument\n"),
    N_("%s: option '--%s' doesn't allow an argument\n"),
    N_("%s: option `%c%s' doesn't allow an argument\n"),
    N_("%s: option '%c%s' doesn't allow an argument\n"),
    N_("%s: option `--%s' requires an argument\n"),
    N_("%s: option '--%s' requires an argument\n"),
    N_("%s: unrecognized option `--%s'\n"),
    N_("%s: unrecognized option '--%s'\n"),
    N_("%s: unrecognized option `%c%s'\n"),
    N_("%s: unrecognized option '%c%s'\n"),
    N_("%s: invalid option -- `%c'\n"),
    N_("%s: invalid option -- '%c'\n"),
    N_("%s: invalid option -- `%c'\n"),
    N_("%s: invalid option -- '%c'\n"),
    N_("%s: option requires an argument -- `%c'\n"),
    N_("%s: option requires an argument -- '%c'\n"),
    N_("%s: option `-W %s' is ambiguous\n"),
    N_("%s: option '-W %s' is ambiguous\n"),
    N_("%s: option `-W %s' doesn't allow an argument\n"),
    N_("%s: option '-W %s' doesn't allow an argument\n"),
    N_("%s: option `-W %s' requires an argument\n"),
    N_("%s: option '-W %s' requires an argument\n"),
    N_("%s: option requires an argument -- `%c'\n"),
    N_("%s: option requires an argument -- '%c'\n"),

    ""
};


/***********************************************************************/
// End of file
