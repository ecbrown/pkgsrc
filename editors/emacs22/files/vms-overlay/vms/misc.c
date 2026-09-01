/*
 * Copyright (C) 1994 the Free Software Foundation, Inc.
 *
 * Author: Roland B. Roberts (roberts@nsrl.rochester.edu)
 *
 * This file is a part of GNU VMSLIB, the GNU library for porting GNU
 * software to VMS.
 *
 * GNU VMSLIB is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNU VMSLIB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU VMSLIB; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Miscellaneous utilities used by hackargv().
 * Some of these are useful in their own right.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <lib$routines.h>

/* Print message and force core dump (unix) or traceback (vms)
 * These are taken from other GNU software more-or-less as-is
 */
void fatal (char *s)
{
  int *x;
  printf ("%s\n", s);
  lib$signal(44);
}

void *xmalloc (int size)
{
  register void *val;
  /* Avoid failure if malloc (0) returns 0.  */
  if (size == 0)
    size = 1;
  val = (void *) malloc (size);
  if (!val) fatal ("xmalloc: can't satisfy request");
  return val;
}

void *xrealloc (void *block, int size)
{
  register void *val;
  /* Avoid failure if malloc (0) returns 0.  */
  if (size == 0)
    size = 1;
  val = (void *) realloc (block, size);
  if (!val) fatal ("xrealloc: can't satisfy request");
  return val;
}

/* See in misc.h why it is done like this.  */
void x_free (void *block)
{
  free (block);
}

/*
 * Some string utilities.
 */
char *downcase (char *s)
{
  register char *t;
  for (t = s ; *t; t++)
    *t = tolower ((unsigned char) *t);
  return (s);
}

#if defined(__DECC) && __VMS_VER < 70000000
char *strdup (char *src) {
  char *dst = (char*) xmalloc (strlen(src) + 1);
  strcpy (dst, src);
  return (dst);
}
#endif

#include <string.h>

/*
 * int fixpath (char *path)
 *
 * Synopsis:
 *   `Fix' VMS pathnames, converting them to canonical form.
 *
 * Description:
 *   The following conversions are performed
 *     x:[y.][000000.z] --> x:[y.z]
 *     x:[y.][z]        --> x:[y.z]
 *     x:[000000.y]     --> x:[y]
 *
 * Author:
 *   Roland B Roberts (roberts@nsrl.rochester.edu)
 *   March 1994
 */
int fixpath (char *path)
{
  char *s, *d, *t;
  int skip = 0;
  d = s = path;
  if (t = strstr(path ,".][000000"))
    skip = 9;
  else if (t = strstr(path,"]["))
    skip = 2;
  else if (t = strstr(path,"[000000."))
    t++, skip = 7;
  if (t) {
    while (s < t)
      *d++ = *s++;
    s += skip;
    while (*d++ = *s++);
  }
  return 0;
}


#include <ctype.h>
#include <string.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/*
 * char *argvconcat (int argc, char **argv)
 *
 * Synopsis:
 *   Concatenate all elements of argv into a single string suitable for
 *   use as a command line.
 *
 * Description:
 *   This is intended for use with hackargv() in order to build a command
 *   line for background() or popen().  Each element of argv (except the
 *   first) is surrounded by double quotes to insure the command line is
 *   unaltered when DCL rereads it.
 *
 * Side Effect:
 *   Space for the new string is allocated with xmalloc().
 *
 * Author:
 *   Roland B Roberts (roberts@nsrl.rochester.edu)
 *   March 1994
 */

char *argvconcat (int argc, char **argv)
{
  int i, j;
  size_t size = 1;
  char *cmdline, *out;

  for (j = 0; j < argc; j++)
    {
      size_t extra = strlen (argv[j]) + (j != 0);
      if (j != 0)
	{
	  extra += 2;
	  for (i = 0; argv[j][i]; i++)
	    if (argv[j][i] == '"')
	      extra++;
	}
      if (extra > INT_MAX - size)
	fatal ("argvconcat: command line too long");
      size += extra;
    }

  cmdline = (char *) xmalloc ((int) size);
  out = cmdline;
  for (j = 0; j < argc; j++)
    {
      const char *arg = argv[j];
      if (j != 0)
	{
	  *out++ = ' ';
	  *out++ = '"';
	}
      while (*arg)
	{
	  if (j != 0 && *arg == '"')
	    *out++ = '"';
	  *out++ = *arg++;
	}
      if (j != 0)
	*out++ = '"';
    }
  *out = '\0';
  return (cmdline);
}
