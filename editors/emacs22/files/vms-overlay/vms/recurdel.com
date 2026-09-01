$ __save_verif = 'f$verify(0)
$! RECURDEL.COM -- Recursive deletion of a directory tree.
$! Copyright (C) 1994 the Free Software Foundation, Inc.
$!
$! Author: Richard Levitte <levitte@e.kth.se>
$!
$! Please see the end of file for further comments
$!
$! Description:
$!
$! P1 = comma-separated list of files (directory files also).
$!	Wildcards are permitted.
$! P2 = comma-separated flags.  See DELETE.COM for more info.
$!
$ save_def = f$environment("DEFAULT")
$ proc = f$environment("PROCEDURE")
$ proc_d =f$parse(proc,,,"NODE")+f$parse(proc,,,"DEVICE")+-
	f$parse(proc,,,"DIRECTORY")
$ RECURSE = "@" + proc
$ i = 0
$loop_args:
$ e = f$element(i,",",P1)
$ i = i + 1
$ if e .nes. ","
$  then
$   d = f$parse(e,save_def)
$   ft = f$parse(e,,,"TYPE")
$   sd = f$parse(".DIR","''e'",,,"SYNTAX_ONLY")
$   sd = f$parse(sd,,,"NAME")+f$parse(sd,,,"TYPE")
$   d = f$parse(d,,,"NODE")+f$parse(d,,,"DEVICE")+f$parse(d,,,"DIRECTORY")
$   set def 'd'
$   dc = 0
$  loop_dirs1:
$   f = f$search("*.DIR")
$   flag1 = (ft .eqs. ".*" .or. ft .eqs. ".DIR")
$   flag2 = (f$parse(f,,,"NAME") .eqs. f$parse(e,,,"NAME") .or. f$parse(e,,,"NAME") .eqs. "*")
$   if f .nes. ""
$    then
$     if flag1 .and. flag2
$      then
$       _d'dc' = f
$       dc = dc + 1
$      endif
$     goto loop_dirs1
$    endif
$   j = 0
$  loop_dirs2:
$   if j .lt. dc
$    then
$     extra_save_def = f$environment("DEFAULT")
$     tmp = _d'j'
$     j = j + 1
$     set def [.'f$parse(tmp,,,"NAME")']
$     RECURSE *.*.* 'p2'
$     set def 'extra_save_def'
$     goto loop_dirs2
$    endif
$   f = f$parse(e,,,"NAME") + f$parse(e,,,"TYPE")
$   @'proc_d'delete 'f' 'p2'
$   goto loop_args
$  endif
$ set def 'save_def'
$exit: ! end of RECURDEL.COM
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
