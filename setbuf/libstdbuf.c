/* libsetbuf -- a shared lib to preload to setup stdio buffering for a command
   Copyright (C) 2009-2012 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Pádraig Brady.  LD_PRELOAD idea from Brian Dessent.  */
/* Adopted by Alex Levkovich */

#include <stdio.h>
#include <stdlib.h>

static const char * fileno_to_name (const int fd) {
  const char *ret = NULL;

  switch (fd)
    {
    case 0:
      ret = "stdin";
      break;
    case 1:
      ret = "stdout";
      break;
    case 2:
      ret = "stderr";
      break;
    default:
      ret = "unknown";
      break;
    }

  return ret;
}

static void apply_mode (FILE *stream) {
  if (setvbuf (stream, NULL, _IONBF, 0) != 0) {
      fprintf(stderr, "could not disable buffering of %s\n",
                      fileno_to_name(fileno (stream)));
    }
}

__attribute__ ((constructor)) static void stdbuf () {
    apply_mode (stderr);
    apply_mode (stdin);
    apply_mode (stdout);
}
