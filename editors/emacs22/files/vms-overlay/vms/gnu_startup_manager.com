$! GNU_INSERT_STARTUP.COM - Command file to update GNU startup database.
$ __save_verif = 'f$verify(0)
$! Copyright (C) 1994 the Free Software Foundation, Inc.
$!
$! Author: Richard Levitte <levitte@e.kth.se>
$!
$! Please see the end of file for further comments
$!
$! Description:
$!
$! P1..P8 = comma-separated commands
$!	DEBUG		Tells you what happens.
$!	NODEBUG		Turn off debugging. (default)
$!	ACTION		Really runs the management commands. (default)
$!	NOACTION	Does not perform the actions.  Good for debugging
$!			together with the VERIFY flag.
$!	VERIFY		Turn on verify.
$!	NOVERIFY	Turns off verify. (default)
$!	INSERT=file	Insert the data file in the database.  On by default.
$!			Turns off EXTRACT and UNINSERT
$!	EXTRACT=prgm	Extracts the data file from the database..
$!			File name is SYS$COMMON:[SYS$STARTUP]prgm_STARTUP.DAT.
$!	EXTRACT=(prgm,file)	Extracts the data file from the database
$!			to a given filename
$!	UNINSERT=prgm	Completely removes the data file for prgm from the
$!			database.  This demands an exact reference, with
$!			version number, if that is applicable.
$!	REMOVE=prgm	An alias for UNINSERT.
$!	UNINSERTALL=pgm	Like UNINSERT, but only wants the name of the program,
$!			And removes all versions of the program.
$!	REMOVEALL=prgm	An alias for UNINSERTALL.
$!	DATABASE=file	Tells what database file should be handled.
$!			Default is SYS$COMMON:[SYS$STARTUP]GNU_STARTUP.COM
$!      NEWVERSION=file	Replaces the main code of the database with the code
$!			in file, if the version number on the first line in
$!			file is higher than the old one, or if the old database
$!			doesn't have a code at all. (Not Yet Implemented)
$!
$
$ goto skip_echo
$ echo_error: subroutine
$ exit 'p1'
$ endsubroutine
$ skip_echo:
$
$ keywords = "/DEBUG/NODEBUG/ACTION/NOACTION/VERIFY/NOVERIFY/INSERT=/EXTRACT=/EXTRACTALL=/UNINSERT=/REMOVE=/UNINSERTALL=/REMOVEALL=/DATABASE=/NEWVERSION=/"
$ debug := NO
$ action := NO
$ verify := NO
$ database = ""
$ on control_y then goto loop_databases2
$ on error then goto loop_databases2
$ database1 := SYS$COMMON:[SYS$STARTUP]GNU_STARTUP.COM
$! The format for each entry of removes'n' is:
$! /pgm\flag\debug,action,verify/
$! where flag is "" or "ALL"
$ removes1 = "/"
$! The format for each entry of extracts'n' is:
$! /pgm\file\debug,action,verify/
$ extracts1 = "/"
$! The format for each entry of inserts'n' is:
$! /file\debug,action,verify/
$ inserts1 = "/"
$ newversion1 = "/"
$ cmd_n = 1
$ cmd_total = 1
$ n = 0
$ flags = ""
$loop_flags:
$ if n .lt. 8
$  then
$   if flags .eqs. ""
$    then
$     n = n + 1
$     flags = p'n'
$    endif
$   if flags .eqs. "" then goto loop_flags
$   endp = f$locate(",",flags)
$   if endp .ne. f$length(flags)
$    then
$     p = f$locate("(",flags)
$     if p .lt. endp
$      then
$	p = p + f$locate(")",f$extract(p, f$length(flags), flags))
$	if p .gt. endp then
		endp = p + f$locate(",",f$extract(p, f$length(flags), flags))
$      endif
$    endif
$   e = f$extract(0,endp,flags)
$   if f$extract(endp,1,flags) .eqs. "," then endp = endp + 1
$   flags = f$extract(endp, f$length(flags), flags)
$   keyword = f$extract(0,f$locate("=",e),e)
$   lable = keyword
$   if keyword .nes. e then keyword = keyword + "="
$   value = f$extract(f$length(keyword),f$length(e),e)
$   if f$extract(0,1,value) .eqs. "(" -
	.and. f$extract(f$length(value)-1,1,value) .eqs. ")" then -
	value = f$extract(1,f$length(value)-2,value)
$   if keywords - ("/"+keyword+"/") .nes. keywords then goto F_'lable'
$   write sys$error "%GNU_STARTUP_MANAGER-E-BADCMD, I did not understand ",e
$   exit 1
$  F_EXTRACT:
$  F_EXTRACTALL:
$   all = ""
$   if f$edit(f$extract(f$length(lable)-3,3,lable),"UPCASE") .eqs. "ALL"
$    then
$     lable = f$extract(0,f$length(lable)-3,lable)
$     all := all
$    endif
$   en = 0
$   new_value = ""
$   e_pgm = f$element(0,",",value)
$   e_file = f$element(1,",",value)
$   cmd_output := 'e_file'
$   if e_file .eqs. ","
$    then
$    loop_dots:
$     e = f$element(en,".",e_pgm)
$     en = en + 1
$     if e .nes. "."
$      then
$       new_value = new_value + "_" + e
$       goto loop_dots
$      else
$       new_value = new_value - "_"
$      endif
$     cmd_output := SYS$STARTUP:[SYS$STARTUP]'new_value'_STARTUP.DAT
$    endif
$   cmd := 'e_pgm'\'cmd_output'\'all'\'debug','action','verify'/
$   extracts'cmd_n' = extracts'cmd_n' + cmd
$   ! sh sym extracts'cmd_n'
$   goto loop_flags
$  F_INSERT:
$   cmd := 'value'\'debug','action','verify'/
$   inserts'cmd_n' = inserts'cmd_n' + cmd
$   ! sh sym inserts'cmd_n'
$   goto loop_flags
$  F_NEWVERSION:
$   cmd := /'value'\'debug','action','verify'/
$   newversion'cmd_n' = cmd
$   ! sh sym newversion'cmd_n'
$   goto loop_flags
$  F_UNINSERT:
$  F_UNINSERTALL:
$   keyword = "REMOVE" + (f$edit(keyword,"UPCASE") - "UNINSERT")
$  F_REMOVE:
$  F_REMOVEALL:
$   all = ""
$   if f$edit(f$extract(f$length(lable)-3,3,lable),"UPCASE") .eqs. "ALL"
$    then
$     lable = f$extract(0,f$length(lable)-3,lable)
$     all := all
$    endif
$   cmd := 'value'\'all'\'debug','action','verify'/
$   removes'cmd_n' = removes'cmd_n' + cmd
$   ! sh sym removes'cmd_n'
$   goto loop_flags
$  F_DEBUG:
$  F_ACTION:
$  F_VERIFY:
$   'lable' := yes
$   goto loop_flags
$  F_NODEBUG:
$  F_NOACTION:
$  F_NOVERIFY:
$   'f$extract(2,f$length(lable)-2,lable)' := no
$   goto loop_flags
$  F_DATABASE:
$   tmp := 'value'
$   cmd_n = 1
$  loop_databases:
$   if cmd_n .le. cmd_total
$    then
$     if tmp .eqs. database'cmd_n'
$      then
$       goto loop_flags
$      endif
$     cmd_n = cmd_n + 1
$     goto loop_databases
$    endif
$   removes'cmd_n' = "/"
$   extracts'cmd_n' = "/"
$   inserts'cmd_n' = "/"
$   newversion'cmd_n' = "/"
$   cmd_total = cmd_n
$   database'cmd_n' = tmp
$   ! sh sym database'cmd_n'
$   goto loop_flags
$  endif
$
$ cmd_total = cmd_n
$ cmd_n = 0
$ database = ""
$ newversion = ""
$ pid = f$getjpi("","PID")
$loop_databases2:
$ tmp_status = $status
$ if f$trnlnm("gnu_insert","LNM$PROCESS") .nes. "" then close gnu_insert
$ if f$trnlnm("gnu_database","LNM$PROCESS") .nes. "" then close gnu_database
$ if f$trnlnm("gnu_newversion","LNM$PROCESS") .nes. "" then -
	close gnu_newversion
$ if database .nes. ""
$  then
$   if newversion .nes. ""
$    then
$     tmp = newversion
$     newversion = ""	! So we don't loop for ever.
$     open/read/error=loop_databases2 gnu_database 'tmp'
$     database = database'cmd_n'
$     write sys$error "%GNU_STARTUP_MANAGER-I-DEBUG, Copying main code from ",-
	tmp," to ",database
$     first_line := no
$     read_new := no
$     read_old := yes
$     write_old := yes
$     write_new := no
$     goto 'resume'
$    endif
$   if f$trnlnm("gnu_part1","LNM$PROCESS") .nes. "" then close gnu_part1
$   if f$trnlnm("gnu_part2","LNM$PROCESS") .nes. "" then close gnu_part2
$   if f$trnlnm("gnu_part3","LNM$PROCESS") .nes. "" then close gnu_part3
$   if ok
$    then
$     copy/conc sys$scratch:gnu_tmp_'pid'.part1,-
	sys$scratch:gnu_tmp_'pid'.part2,-
	sys$scratch:gnu_tmp_'pid'.part3 -
	'database'
$     delete/nolog sys$scratch:gnu_tmp_'pid'.part1;*,-
	sys$scratch:gnu_tmp_'pid'.part2;*,-
	sys$scratch:gnu_tmp_'pid'.part3;*
$    else
$     set noon
$     call echo_error 'tmp_status'
$     set on
$     if (tmp_status .and. 1) .nes. 1
$      then
$       write sys$error "-GNU_STARTUP_MANAGER-I-DATABSE, database is  ",database
$       write sys$error "-GNU_STARTUP_MANAGER-I-PART1, part 1 is in SYS$SCRATCH:GNU_TMP_",pid,".PART1"
$       write sys$error "-GNU_STARTUP_MANAGER-I-PART2, part 2 is in SYS$SCRATCH:GNU_TMP_",pid,".PART2"
$       write sys$error "-GNU_STARTUP_MANAGER-I-PART3, part 3 is in SYS$SCRATCH:GNU_TMP_",pid,".PART3"
$      endif
$    endif
$  else
$   if newversion .nes. ""
$    then
$     if f$trnlnm("gnu_newversion","LNM$PROCESS") .nes. "" then -
	close gnu_newversion
$     tmp = newversion
$     newversion = ""	! So we don't loop for ever.
$     open/read/error=loop_databases2 gnu_database 'tmp'
$     database = database'cmd_n'
$     write sys$error "%GNU_STARTUP_MANAGER-I-DEBUG, Copying main code from ",-
	tmp," to ",database
$     first_line := no
$     read_new := no
$     read_old := yes
$     write_old := yes
$     write_new := no
$     goto 'resume'
$    endif
$  endif
$ ok := no
$ cmd_n = cmd_n + 1
$ if cmd_n .gt. cmd_total then goto exit
$   if removes'cmd_n' .eqs. "/" .and. extracts'cmd_n' .eqs. "/" -
	.and. inserts'cmd_n' .eqs. "/" .and. newversion'cmd_n' .eqs. "/" then -
	goto loop_databases2
$   tmp = database'cmd_n'
$   resume := loop_part1
$   write sys$error "%GNU_STARTUP_MANAGER-I-PART1, starting on part 1 (checking the main code)"
$   write sys$error "%GNU_STARTUP_MANAGER-I-DATABASE, looking at database ",tmp
$   newversion = f$element(0,"\",f$element(1,"/",newversion'cmd_n'))
$   open/write gnu_part1 sys$scratch:gnu_tmp_'pid'.part1
$   database = ""
$   open/read/error=loop_databases2 gnu_database 'tmp'
$   database = tmp
$   first_line := yes
$   read_new := no
$   read_old := yes
$   write_old := yes
$   write_new := no
$  loop_part1:
$   if read_old then -
	read/end=loop_databases2/error=loop_databases2 gnu_database line
$   if read_old then ! sh sym line
$   if read_new then -
	read/end=loop_databases2/error=loop_databases2 gnu_newversion new_line
$   if read_new then ! sh sym new_line
$   if first_line .and. newversion .nes. ""
$    then
$     first_line := no
$     tmp = newversion
$     newversion = ""
$     open/read/error=loop_databases2 gnu_newversion 'tmp'
$     read/end=loop_databases2/err=loop_databases2 gnu_newversion new_line
$     ! sh sym new_line
$     write sys$error "%GNU_STARTUP_MANAGER-I-DEBUG, Checking database versions."
$     if line - " -- version " .eqs. line
$      then
$       read_new := yes
$       write_new := yes
$       write_old := no
$       read_old := no
$      else
$       if new_line - " -- version " .eqs. new_line then goto loop_databases2
$       p = f$locate(" -- version ",line)+12
$       ! sh sym p
$       old_version = f$element(0," ",f$extract(p,f$length(line)-p,line))
$       ! sh sym old_version
$       p = f$locate(" -- version ",new_line)+12
$       ! sh sym p
$       new_version = f$element(0," ",f$extract(p,f$length(new_line)-p,new_line))
$       ! sh sym new_version
$       vn = 0
$      loop_versions:
$       oe = f$element(vn,".",old_version)
$       ne = f$element(vn,".",new_version)
$       ! sh sym oe
$       ! sh sym ne
$       vn = vn + 1
$       if oe .eqs. "."
$        then if ne .nes. "."
$         then
$          write sys$error "%GNU_STARTUP_MANAGER-I-LESSER, old version < new version.  Inserting new code."
$          read_new := yes
$          write_new := yes
$          write_old := no
$          read_old := no
$         endif
$        else if ne .nes. "."
$         then if 0+oe .lt. 0+ne
$          then
$           write sys$error "%GNU_STARTUP_MANAGER-I-LESSER, old version < new version.  Inserting new code."
$           read_new := yes
$           write_new := yes
$           write_old := no
$           read_old := no
$          else
$           if 0+oe .eq. 0+ne
$            then
$             write sys$error "%GNU_STARTUP_MANAGER-I-SAMESOFAR, so far, old version == new version"
$             goto loop_versions
$            endif
$          endif
$         endif
$        endif
$       if read_old then write sys$error "%GNU_STARTUP_MANAGER-I-SAMEVERSION, looks like you have the latest..."
$      endif
$    endif
$   if write_new
$    then if f$extract(0,20,new_line) .nes. "$ __st_known_utility"
$     then
$      write/error=loop_databases2 gnu_part1 new_line
$      goto loop_part1
$     else
$      read_old := yes
$      read_new := no
$      write_new := no
$     endif
$    endif
$   if f$extract(0,20,line) .nes. "$ __st_known_utility"
$    then
$     if write_old then write/error=loop_databases2 gnu_part1 line
$     goto loop_part1
$    else
$     if .not. write_old
$      then
$       read_old := no
$       read_new := yes
$       write_new := yes
$      endif
$    endif
$   close gnu_part1
$   known_utilities = 0
$   pseudo_removes = ""
$   open/write gnu_part2 sys$scratch:gnu_tmp_'pid'.part2
$   write sys$error "%GNU_STARTUP_MANAGER-I-PART2, starting on part 2 (checking known utilities)"
$   goto loop_part2_2
$  loop_part2_1:
$   read/end=loop_databases2/error=loop_databases2 gnu_database line
$   ! sh sym line
$  loop_part2_2:
$   if f$extract(0,20,line) .eqs. "$ __st_known_utility" -
	.and. f$extract(20,1,line) .nes. "_"
$    then
$     tmp = f$edit('f$element(1,"=",line)',"TRIM")
$     util = f$edit(f$element(0," ",f$element(1,":",tmp)),"UPCASE")
$     ! sh sym util
$     tmp2 = removes'cmd_n' + pseudo_removes
$     p = f$locate("/"+f$element(0,"-",util),tmp2)
$     rem = f$element(1,"/",f$extract(p,f$length(tmp2)-p,tmp2))
$     all = f$element(1,"\",rem)
$     ! sh sym all
$     rem = f$element(0,"\",rem)
$     ! sh sym rem
$     if util - rem .nes. util
$      then
$       c = f$extract(0,1,util - rem)
$       ! sh sym c
$       if c .eqs. "" .or. -
		((c .eqs. "." .or. c .eqs. "-") -
		 .and. all .eqs. "ALL") then goto loop_part2_1
$      endif
$     cmd := 'util'\\'gnu_debug','gnu_action','gnu_verify'/
$     pseudo_removes = pseudo_removes + cmd
$     known_utilities = known_utilities + 1
$     write gnu_part2 "$ __st_known_utility",known_utilities," = """,tmp,""""
$     goto loop_part2_1
$    endif
$   open/write gnu_part3 sys$scratch:gnu_tmp_'pid'.part3
$   extracting := no
$   removing := no
$   resume := loop_part3_1
$   tmp_inhibit_write := no
$   write sys$error "%GNU_STARTUP_MANAGER-I-PART3, starting on part 3 (managing the utility setups)"
$   goto loop_part3_1
$  loop_part3_0:
$   close gnu_newversion
$   read_new := no
$   write_new := no
$   read_old := yes
$   write_old := no
$   tmp_inhibit_write := yes
$   extracting := no
$   removing := no
$  loop_part3_1:
$   ok := no
$   new_line = ""
$   line = ""
$   if read_new then -
	read/end=loop_part3_0/error=loop_databases2 gnu_newversion new_line
$   if read_new then ! sh sym new_line
$   if read_old then -
	read/end=loop_part3_end/error=loop_databases2 gnu_database line
$   if read_old then ! sh sym line
$   if f$extract(0,11,line) .eqs. "$!GNU NODE:" -
	.or. f$extract(0,11,new_line) .eqs. "$!GNU NODE:"
$    then
$     read_new := no
$     write_new := no
$     read_old := yes
$     write_old := yes
$     tmp_inhibit_write := no
$     if extracting then close gnu_extract
$     extracting := no
$     removing := no
$     util = f$edit(f$element(0," ",-
			      f$element(1,":",-
					f$extract(11,f$length(line)-11,-
						  line))),-
		    "UPCASE")
$     ! sh sym util
$     p = f$locate("/"+f$element(0,"-",util),removes'cmd_n')
$     rem = f$element(1,"/",f$extract(p,f$length(removes'cmd_n')-p,removes'cmd_n'))
$     all = f$element(1,"\",rem)
$     ! sh sym all
$     rem = f$element(0,"\",rem)
$     ! sh sym rem
$     if util - rem .nes. util
$      then
$       c = f$extract(0,1,util - rem)
$       ! sh sym c
$       if c .eqs. "" .or. -
		((c .eqs. "." .or. c .eqs. "-") -
		 .and. all .eqs. "ALL")
$        then
$         removing := yes
$         tmp_inhibit_write := yes
$         write sys$error "%GNU_STARTUP_MANAGER-I-REMOVE, removing old setup for ",util
$        endif
$      endif
$     ! sh sym removing
$     p = f$locate("/"+f$element(0,"-",util),extracts'cmd_n')
$     ext = f$element(1,"/",f$extract(p,f$length(extracts'cmd_n')-p,extracts'cmd_n'))
$     ! sh sym ext
$     extract_file := 'f$element(1,"\",ext)'
$     ! sh sym extract_file
$     all = f$element(2,"\",ext)
$     ! sh sym all
$     ext = f$element(0,"\",ext)
$     ! sh sym ext
$     if util - ext .nes. util
$      then
$       c = f$extract(0,1,util - ext)
$       ! sh sym c
$       if c .eqs. "" .or. -
		((c .eqs. "." .or. c .eqs. "-") -
		 .and. all .eqs. "ALL")
$        then
$         extracting := yes
$         write sys$error "%GNU_STARTUP_MANAGER-I-EXTRACT, extracting data file for ",util
$         open/write gnu_extract 'extract_file'
$        endif
$      endif
$     ! sh sym extracting
$    endif
$   if f$extract(0,11,line) .eqs. "$!GNU DATA:" .and. extracting then -
     write/error=loop_databases2 gnu_extract f$extract(12,f$length(line)-12,line)
$   if .not. removing .and. .not. tmp_inhibit_write
$    then
$     if write_new then write/error=loop_databases2 gnu_part3 new_line
$     if write_old then write/error=loop_databases2 gnu_part3 line
$    endif
$   if line .eqs. "$!GNU NODE END" .and. .not. removing
$    then
$     write/error=loop_databases2 gnu_part3 "$ GOTO '__st_resume_lable'"
$     tmp_inhibit_write := yes
$    endif
$   goto loop_part3_1
$  loop_part3_end:
$   if extracting then close gnu_extract
$   ins_i = 0
$  loop_part3_ins_1:
$   ins = f$element(ins_i,"/",inserts'cmd_n')
$   ins_i = ins_i + 1
$   if ins .eqs. "" then goto loop_part3_ins_1
$   if ins .eqs. "/" then goto loop_part3_ins_end2
$   ins_file = f$element(0,"\",ins)
$   open/read gnu_insert 'ins_file'
$   extracting := no
$  loop_part3_ins_2:
$   read/end=loop_part3_ins_end/error=loop_databases2 gnu_insert line
$   ! sh sym line
$   if f$extract(0,11,line) .eqs. "$!GNU NODE:"
$    then
$     known_utilities = known_utilities + 1
$     known_utility'known_utilities' = -
	f$edit(f$extract(11,f$length(line)-11,line),"TRIM")
$     write/error=loop_databases2 gnu_part2 "$ __st_known_utility",known_utilities," = """,known_utility'known_utilities',""""
$     extracting := yes
$     write sys$error "%GNU_STARTUP_MANAGER-I-INSERT, inserting new setup for ",f$element(0," ",known_utility'known_utilities')
$    endif
$   if extracting then write/error=loop_databases2 gnu_part3 line
$   if f$extract(0,14,line) .eqs. "$!GNU NODE END" .and. extracting
$    then
$     write/error=loop_databases2 gnu_part3 "$ GOTO '__st_resume_lable'"
$     extracting := no
$     goto loop_part3_ins_1
$    endif
$   goto loop_part3_ins_2
$  loop_part3_ins_end:
$   close gnu_insert
$  loop_part3_ins_end2:
$   write/error=loop_databases2 gnu_part2 "$ __st_known_utility_n = ",known_utilities
$   write sys$error "%GNU_STARTUP_MANAGER-I-SETUPS, all in all, there now are ",known_utilities," setups."
$   ok := yes
$   goto loop_databases2
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
