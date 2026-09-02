$! DELETE.COM -- Recursive deletion of a directory tree.
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
$! P2 = comma-separated flags.
$!	FORCE		try to force deletion even when the
$!			delete permition is not set.
$!	REMOVE_NAME	only remove the name (SET FILE/REMOVE).
$!	
$!
$ save_def = f$environment("DEFAULT")
$ proc = f$environment("PROCEDURE")
$ proc_d =f$parse(proc,,,"NODE")+f$parse(proc,,,"DEVICE")+-
	f$parse(proc,,,"DIRECTORY")
$ say := write sys$output
$
$ if f$type(__force_delete) .eqs. "" then -
	__force_delete := no
$ if f$type(__debug_delete) .eqs. "" then -
	__debug_delete = f$trnlnm("DEBUG_DELETE")
$ if f$type(__remove_dir_delete) .eqs. "" then -
	__remove_dir_delete := no
$ if f$type(__remove_name_delete) .eqs. "" then -
	__remove_name_delete := no
$ if f$type(__simulate_delete) .eqs. "" then -
	__simulate_delete = f$trnlnm("SIMULATE_DELETE")
$ i = 0
$loop_flags:
$ e = f$element(i,",",P2)
$ i = i + 1
$ if e .nes. ","
$  then
$   if f$edit(e,"UPCASE") .eqs. "REMOVE" then e := remove_name
$   if e .nes. "" then __'e'_delete := yes
$   goto loop_flags
$  endif
$
$ save_mesg = f$environment ("MESSAGE") ! DELETE is way too talkative
$ set message/nofacility/noident/noseverity/notext
$ i = 0
$loop_args:
$ e = f$element(i,",",P1)
$ i = i + 1
$ if e .nes. ","
$  then
$   e = f$parse(e,".;*",save_def)
$   if e .nes. ""
$    then
$     passes = 1
$     __tmp_remove_name_delete = __remove_name_delete
$     __tmp_e = e
$     __tmp_type = f$edit(f$parse(e,,,"TYPE"),"UPCASE")
$     if (__tmp_type .eqs. ".DIR" .or. __tmp_type .eqs. ".*")
$      then
$       __remove_name_delete = __remove_dir_delete
$       if __tmp_type .eqs. ".*" then passes = 2
$       e = f$parse(".DIR",e)
$      endif
$    loop_delete:
$     if __simulate_delete .or. __debug_delete
$      then
$       if __force_delete then say "%DELETE-I-DEBUG,  Debug: SET FILE/PROT=O:D ",e
$	if __remove_name_delete
$	 then say "%DELETE-I-DEBUG,  Debug: SET FILE/REMOVE ",e
$	 else say "%DELETE-I-DEBUG,  Debug: DELETE ",e
$	 endif
$      endif
$     if .not. __simulate_delete
$      then
$!      ;; "HELP SET FILE /PROTECTION" sez it is obsoleted, to be replaced
$!      ;; by SET SECURITY /PROTECTION, but doesn't say if the former can
$!      ;; still be used.  blech.  --ttn
$       if __force_delete .and. f$search(e) .nes. "" then -
		set security/prot=(w:d) 'e'
$       if __remove_name_delete
$	 then set file/remove 'e'
$	 else delete 'e'
$	 endif
$      endif
$     e = __tmp_e
$     __remove_name_delete = __tmp_remove_name_delete
$     passes = passes - 1
$     if passes .ne. 0 then goto loop_delete
$    endif
$   goto loop_args
$  endif
$ set message 'save_mesg'
$ set def 'save_def'
$ exit 1
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
