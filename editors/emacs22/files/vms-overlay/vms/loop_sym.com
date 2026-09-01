$ __save_verif = 'f$verify(0)
$! LOOP_SYM.COM - loops around a set of arguments in a symbol and does what
$! you ask it to.
$! Copyright (C) 1994 the Free Software Foundation, Inc.
$!
$! Author: Richard Levitte <levitte@e.kth.se>
$!
$! Please see the end of file for further comments
$!
$! Description:
$!
$! P1 = symbol to assign entries to
$! P2 = symbol containing the list of things to loop on
$!	(comma- or space-separated, depemding on P2)
$! P3 = character separating the entries
$! P4 = command(s) to execute on the file
$! P5 = character separating the commands
$! P6 = symbol to assign the next entry to (optional)
$! P7 = flags
$ __debug = f$trnlnm("DEBUG_LOOP_ARGS")
$ p7 = "," + p7 + ","
$ __la_verify = f$locate(",VERIFY,",p7) .ne. f$length(p7)
$ __la_verbose = f$locate(",VERBOSE,",p7) .ne. f$length(p7)
$ if __la_verify then set verify
$ __p1 = p1
$ __p2 = p2
$ __p3 = p3
$ __p4 = p4
$ __p5 = p5
$ __P6 = P6
$ if __P6 .eqs. "" then __P6 := __tmp_e
$ __looped_once := no
$ __thing_i = 0
$thing_loop:
$ '__P6' = f$element(__thing_i,__p3,'__p2')
$ if __looped_once
$  then
$   '__p1' = __thing
$   if __thing .nes. __p3
$    then
$     __cmd_i = 0
$    cmd_loop:
$     __cmd = f$element(__cmd_i,__p5,__p4)
$     __cmd_i = __cmd_i + 1
$     if __cmd .nes. __p5
$      then
$       __la_save_verif = f$verify(0)
$       if __debug .or. __la_verbose then set verify ! 'f$verify(__la_save_verif)
$       '__cmd'
$       ! 'f$verify(__la_save_verif)
$       goto cmd_loop
$      endif
$    endif
$  endif
$ __looped_once := yes
$ __thing = '__P6'
$ __thing_i = __thing_i + 1
$ if '__P6' .nes. __P3 then goto thing_loop
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
