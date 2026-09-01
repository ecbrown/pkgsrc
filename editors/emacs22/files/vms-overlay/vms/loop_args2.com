$ __save_verif = 'f$verify(0)
$! LOOP_ARGS2.COM - loops around two sets of arguments and does what you
$! ask it to.
$! Copyright (C) 1994 the Free Software Foundation, Inc.
$!
$! Author: Richard Levitte <levitte@e.kth.se>
$!
$! Please see the end of file for further comments
$!
$! Description:
$!
$! P1 = symbol to assign entries from P3 to
$! P2 = symbol to assign entries from P4 to
$! P3 = List of things to loop on (comma- or space-separated, depemding on P5)
$! P4 = List of things to loop on (comma- or space-separated, depemding on P5)
$! P5 = character separating the entries
$! P6 = command(s) to execute on the file
$! P7 = character separating the commands
$! P8 = flags
$ __debug = f$trnlnm("DEBUG_LOOP_ARGS")
$ p8 = "," + p8 + ","
$ __la_verify = f$locate(",VERIFY,",p8) .ne. f$length(p8)
$ __la_verbose = f$locate(",VERBOSE,",p8) .ne. f$length(p8)
$ if __la_verify then set verify
$ __p1 = p1
$ __p2 = p2
$ __p3 = p3
$ __p4 = p4
$ __p5 = p5
$ __p6 = p6
$ __p7 = p7
$ __thing_i = 0
$thing_loop:
$ __thing1 = f$element(__thing_i,__p5,__p3)
$ '__p1' = __thing1
$ __thing2 = f$element(__thing_i,__p5,__p4)
$ '__p2' = __thing2
$ __thing_i = __thing_i + 1
$ if __thing1 .nes. __p5
$  then
$   __cmd_i = 0
$  cmd_loop:
$   __cmd = f$element(__cmd_i,__p7,__p6)
$   __cmd_i = __cmd_i + 1
$   if __cmd .nes. __p7
$    then
$     __la_save_verif = f$verify(0)
$     if __debug .or. __la_verbose then set verify ! 'f$verify(__la_save_verif)
$     '__cmd'
$     ! 'f$verify(__la_save_verif)
$     goto cmd_loop
$    endif
$   goto thing_loop
$  endif
$ exit 1 + 0*f$verify(__save_verif)
$
$! This file is a part of GNU VMSLIB, the GNU library for porting GNU
$! software to VMS.
$!
$! GNU VMSLIB is free software; you can redistribute it and/or
$! modify it under the terms of the GNU General Public License
$! as published by the Free Software Foundation; either version 2
$! of the License, or (at your option) any later version.
$! 
$! GNU VMSLIB is distributed in the hope that it will be useful,
$! but WITHOUT ANY WARRANTY; without even the implied warranty of
$! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
$! GNU General Public License for more details.
$! 
$! You should have received a copy of the GNU General Public License
$! along with GNU VMSLIB; if not, write to the Free Software
$! Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
