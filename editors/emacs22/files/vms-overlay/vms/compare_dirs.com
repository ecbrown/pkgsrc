$ __save_verif = 'f$verify(0)
$! COMPARE_DIRS.COM -- Compares directory specifications
$! Copyright (C) 1994 the Free Software Foundation, Inc.
$!
$! Author: Richard Levitte <levitte@e.kth.se>
$!
$! Please see the end of file for further comments
$!
$! Description:
$!
$! P1 = first directory
$! P2 = second directory
$! P3 = global variable to put result in
$
$ proc = f$environment("PROCEDURE")
$ proc_dir = f$parse(proc,,,"NODE")+f$parse(proc,,,"DEVICE")+f$parse(proc,,,"DIRECTORY")
$ __debug = f$trnlnm("DEBUG_COMPARE_DIRS")
$ if __debug
$  then
$   write sys$output "%COMPARE_DIRS-I-DEBUG, P1 = """,p1,""""
$   write sys$output "%COMPARE_DIRS-I-DEBUG, P2 = """,p2,""""
$  endif
$ __result = ""
$ delete_/symbol/local __result
$ @'proc_dir'canonicaldir 'p1' __result COMPLETE
$ if __debug then sh sym __result
$ @'proc_dir'decomplnm '__result' __c_d_dir1 DIRS,RETURN_DEVICE,DEEP
$ if __debug then sh sym __c_d_dir1
$ @'proc_dir'canonicaldir 'p2' __result COMPLETE
$ if __debug then sh sym __result
$ @'proc_dir'decomplnm '__result' __c_d_dir2 DIRS,RETURN_DEVICE,DEEP
$ if __debug then sh sym __c_d_dir2
$ if __debug
$  then
$   write sys$output "%COMPARE_DIRS-I-DEBUG, __c_d_dir1 = """,__c_d_dir1,""""
$   write sys$output "%COMPARE_DIRS-I-DEBUG, __c_d_dir2 = """,__c_d_dir2,""""
$  endif
$ 'p3' == f$edit(__c_d_dir1,"UPCASE") .eqs. f$edit(__c_d_dir2,"UPCASE")
$
$exit:
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
