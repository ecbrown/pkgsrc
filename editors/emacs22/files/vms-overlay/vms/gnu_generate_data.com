$! GNU_GENERATE_DATA.COM -- General startup script.
$! Copyright (C) 1994 the Free Software Foundation, Inc.
$!
$! Author: Richard Levitte <levitte@e.kth.se>
$!
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
$
$! GNU_STARTUP.COM reads the data from the file GNU_STARTUP.DAT in the
$! same directory.
$!
$! Description:
$!
$! This command file will just read a startup template file, given through
$! the argument P1, a configuration data file give through the argument P2
$! and generate a startup data file from it.  The name of the command file is
$! given in P3.
$!
$! A description of the format of the input is given at the end of this file.
$!
$! The generated file will have a specific format, to make it possible to
$! tuck in into a larger command file.
$!
$ open/read/error=noinfile1 gnu_data_file1 'p1'
$ open/read/error=noinfile2 gnu_data_file2 'p2'
$ open/write/error=nooutfile gnu_data_file3 'p3'
$ on control_y then goto bugout
$ on error then goto bugout
$!
$ write/error=bugout gnu_data_file3 "!# Generated from ",p1," and ",p2
$!
$
$ keep := yes
$ current_symbol = ""
$ __subst_symbols = ","
$loop_file:
$ read/error=loop_file_end/end=loop_file_end gnu_data_file2 line
$! sh sym line
$ first = f$extract(0,1,line)
$ last = f$extract(f$length(line)-1,1,line)
$ middle = f$extract(1,f$length(line)-2,line)
$ if first .eqs. "@" .and. last .eqs. "@"
$  then
$   current_symbol = middle
$   if __subst_symbols - (","+current_symbol+",") .eqs. __subst_symbols then -
	__subst_symbols = __subst_symbols + current_symbol + ","
$   if f$type(__subst_'current_symbol'_max) .eqs. "" then -
	__subst_'current_symbol'_max = 0
$   goto loop_file
$  endif
$ tmp = __subst_'current_symbol'_max
$ __subst_'current_symbol'_'tmp' = line
$ __subst_'current_symbol'_max = __subst_'current_symbol'_max + 1
$ goto loop_file
$loop_file_end:
$
$loop_file2:
$ read/error=nomore/end=nomore gnu_data_file1 line
$loop_subst1:
$ i = 1
$loop_subst2:
$ e = f$element(i,",",__subst_symbols)
$ i = i + 1
$ if e .eqs. "" then goto loop_subst2
$ if e .eqs. "," then goto loop_subst_end
$ p = f$locate("@"+e+"@",line)
$ if p .eqs. f$length(line) then goto loop_subst2
$ prefix = f$extract(0,p,line)
$ suffix = f$extract(p+2+f$length(e),f$length(line),line)
$ tmp = __subst_'e'_max - 1
$ if __subst_'e'_max .gt. 0 then suffix = __subst_'e'_'tmp' + suffix
$ if __subst_'e'_max .le. 1 then suffix = prefix + suffix
$ j = 0
$loop_write1:
$ if j .ge. __subst_'e'_max - 1 then goto loop_write2
$ write/error=bugout gnu_data_file3 prefix,__subst_'e'_'j'
$ j = j + 1
$ prefix = ""
$ goto loop_write1
$loop_write2:
$ line = suffix
$ goto loop_subst1
$loop_subst_end:
$ write/error=bugout gnu_data_file3 line
$ goto loop_file2
$ 
$bugout:
$ tmp_status=$status
$ if "''gnu_extra_file'" .nes. ""
$  then
$   close 'gnu_extra_file
$  endif
$ line0 = ""
$ keep := no
$nomore:
$ close gnu_data_file3
$ if .not. keep then delete 'f$parse(p3,".;*")'
$nooutfile:
$ close gnu_data_file2
$noinfile2:
$ close gnu_data_file1
$noinfile1:
$exit:
$ exit
