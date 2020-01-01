/************************************************************************
*                                                                       *
*             Star Traders: A Game of Interstellar Trading              *
*                Copyright (C) 1990-2020, John Zaitseff                 *
*                                                                       *
************************************************************************/

/*
  Author: John Zaitseff <J.Zaitseff@zap.org.au>
  $Id$

  This file, xopen-source.h, redefines _XOPEN_SOURCE to be the latest
  version that is supported by the operating system's C library.  It is
  used by the Autoconf macro in ../m4/xopen-source.m4.


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


#ifndef included_XOPEN_SOURCE_H
#define included_XOPEN_SOURCE_H


#ifdef LATEST_XOPEN_SOURCE
#  if ! defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < LATEST_XOPEN_SOURCE
#    undef  _XOPEN_SOURCE
#    define _XOPEN_SOURCE LATEST_XOPEN_SOURCE
#  endif
#endif


#endif /* included_XOPEN_SOURCE_H */
