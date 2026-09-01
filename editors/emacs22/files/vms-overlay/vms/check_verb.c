/*
 * Copyright © 1995 the Free Software Foundation, Inc.
 *
 * Author: Richard Levitte <levitte@e.kth.se>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <descrip.h>
#if 0 /* was `#ifdef __DECC', but apparently, only VMS 6.2 and up has this
	 header file...  */
#include <cli$routines.h>
#else
unsigned long cli$dcl_parse ();
#endif
#include <lib$routines.h>
#include <libclidef.h>

#ifndef __GNUC__
#pragma nostandard
globalref void *ctl$ag_clitable;
#pragma standard
#else
void *ctl$ag_clitable asm("_$$PsectAttributes_GLOBALSYMBOL$$ctl$ag_clitable");
#endif

main (argc, argv)
     int argc;
     char *argv[];
{
  struct dsc$descriptor_s command;
  unsigned long status;

  if (argc < 2)
    {
      fprintf (stderr, "Usage: check_verb verb\n");
      exit (0);
    }

  command.dsc$b_dtype = DSC$K_DTYPE_T;
  command.dsc$b_class = DSC$K_CLASS_S;
  command.dsc$w_length = strlen (argv[1]);
  command.dsc$a_pointer = argv[1];

  status = cli$dcl_parse (&command, ctl$ag_clitable,
			  lib$get_input, lib$get_input, 0);
  if (argc == 2)
    exit (status);
  else
    {
        struct dsc$descriptor_s symbol, value;
	unsigned long flags = LIB$K_CLI_LOCAL_SYM; /* This is ridiculous */

	symbol.dsc$b_dtype = DSC$K_DTYPE_T;
	symbol.dsc$b_class = DSC$K_CLASS_S;
	symbol.dsc$w_length = strlen (argv[2]);
	symbol.dsc$a_pointer = argv[2];

	value.dsc$b_dtype = DSC$K_DTYPE_T;
	value.dsc$b_class = DSC$K_CLASS_S;
	value.dsc$a_pointer = status & 1 ? "YES" : "NO";
	value.dsc$w_length = strlen (value.dsc$a_pointer);

	exit (lib$set_symbol (&symbol, &value, &flags));
    }
}
