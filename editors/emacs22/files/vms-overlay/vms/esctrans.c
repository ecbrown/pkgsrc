/*
 * Copyright (C) 1994 the Free Software Foundation, Inc.
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
 * int esctrans (char *dst, const char *src)
 *
 * Synopsis:
 *   Copy SRC to DST translating any "special" characters.
 *
 * Description:
 *   If SRC contains no "special" characters, SRC will be copied without
 *   modification into DST.  The backslash character is used, as in printf(),
 *   to introduce any special characters.  The list of special characters
 *   includes all those listed by Kernighan and Ritchie, 2nd edition.  Any
 *   character which is not understood will be passed through after stripping
 *   the backslash.
 *
 *   Additionally, `^' is used to introduce a control character.  This latter
 *   extension is only known to work for the ASCII character set.
 *
 * Bugs:
 *   The newline character, '\n', is translated into CRLF.  This is correct
 *   for VAX/VMS, and probably for MS-DOG.
 *   
 *   The code '\0' (and its octal and hexadecimal equivalents) will be
 *   correctly converted, but will probably cause problems since the normal
 *   C string utilities will assume that is the end of the string.
 *
 * Author:
 *   Roland B Roberts (roberts@nsrl.rochester.edu)
 *   March 1994
 */

#include <ctype.h>
#include <stdio.h>
int esctrans (char *dst, char *src)
{
  char *s, *d;
  int i, val;
  char tmp[4];
  for (s = src, d = dst; *s != 0; ) {
    switch (*s) {
    case '\\':
      /* A trailing escape used to advance S past the terminating NUL, so the
         outer loop read beyond SRC.  Preserve a lone backslash literally.  */
      if (s[1] == '\0') {
	*d++ = *s++;
	break;
      }
      switch (*++s) {
      case 'a':			/* alert (bell) */
	*d++ = '\a'; s++; break;
      case 'b':			/* backspace */
	*d++ = '\b'; s++; break;
      case 'f':			/* formfeed */
	*d++ = '\f'; s++; break;
      case 'n':			/* newline */
	*d++ = '\r'; *d++ = '\n'; s++; break;
      case 'r':			/* carraige return */
	*d++ = '\r'; s++; break;
      case 't':			/* horizontal tab */
	*d++ = '\t'; s++; break;
      case 'v':			/* vertical tab */
	*d++ = '\v'; s++; break;
      case '\'':		/* single quote */
	*d++ = '\''; s++; break;
      case '\"':		/* double quotes */
	*d++ = '\"'; s++; break;
      case 'e':			/* real escape */
	*d++ = '\033'; s++; break;
      case '0': case '1':	/* octal, 1--3 digits allowed */
      case '2': case '3': case '4': case '5': case '6': case '7':
	tmp[i=0] = *s++;
	if (isdigit((unsigned char) *s) && *s != '8' && *s != '9') {
	  tmp[++i] = *s++;
	  if (isdigit((unsigned char) *s) && *s != '8' && *s != '9')
	    tmp[++i] = *s++;
	}
	tmp[++i] = 0;
	sscanf (tmp, "%o", &val);
	*d++ = val; break;
      case 'x':			/* hexadecimal, 1--2 digits allowed */
	s++;
	if (isxdigit((unsigned char) *s)) {
	  tmp[i=0] = *s++;
	  if (isxdigit((unsigned char) *s)) {
	    tmp[++i] = *s++;
	  }
	  tmp[++i] = 0;
	  sscanf (tmp, "%x", &val);
	  *d++ = val;
	}
	else			/* invalid or incomplete hexadecimal */
	  {
	    if (*s == '\0')
	      *d++ = 'x';
	    else
	      *d++ = *s++;
	  }
	break;
      default:			/* just copy as-is */
	*d++ = *s++; break;
      }
      break;
    case '^':			/* control character */
	if (s[1] == '\0')
	  *d++ = *s++;
	else
	  {
	    s++;
	    *d++ = *s++ & 0x1F;
	  }
	break;
    default:
      *d++ = *s++; break;
    }
  }
  *d++ = 0;
  return(0);
}
