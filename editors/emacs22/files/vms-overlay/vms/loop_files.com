$ __save_verif = 'f$verify(0)
$! LOOP_FILES.COM - loops around a set of files and does what you ask it to.
$! Copyright (C) 1994 the Free Software Foundation, Inc.
$!
$! Author: Richard Levitte <levitte@e.kth.se>
$!
$! Please see the end of file for further comments
$!
$! Description:
$!
$! P1 = symbol to assign entries to
$! P2 = List of filess to loop on (comma- or space-separated, depemding on P2)
$! P3 = character separating the entries
$! P4 = command(s) to execute on the file
$! P5 = character separating the commands
$! P6 = flags
$ __debug = f$trnlnm("DEBUG_LOOP_ARGS")
$ p6 = "," + p6 + ","
$ __la_verify = f$locate(",VERIFY,",p6) .ne. f$length(p6)
$ __la_verbose = f$locate(",VERBOSE,",p6) .ne. f$length(p6)
$ if __la_verify then set verify
$ __p1 = p1
$ __p2 = p2
$ __p3 = p3
$ __p4 = p4
$ __p5 = p5
$ __thing_i = 0
$thing_loop:
$ __thing = f$element(__thing_i,__p3,__p2)
$ __thing_i = __thing_i + 1
$ if __thing .nes. __p3
$  then
$  file_loop:
$   '__p1' = f$search(__thing)
$   if '__p1' .nes. ""
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
$     goto file_loop
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
