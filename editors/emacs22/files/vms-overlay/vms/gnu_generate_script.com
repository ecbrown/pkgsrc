$! GNU_GENERATE_SCRIPT.COM -- General startup script.
$ __debug_save_verify = 'f$verify(0)
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
$! This command file will just read a file, given through the argument P1,
$! and generate a command file from it.  The name of the command file is
$! given in P2.
$!
$! A description of the format of the input is given at the end of this file.
$!
$! The generated file will have a specific format, to make it possible to
$! tuck in into a larger command file.
$!
$ tmp_status = 1
$ keep := no
$ line_i = 0
$ line0 = ""
$ gnu_extra_file :=
$ open/read/error=noinfile gnu_data_file 'p1'
$ open/write/error=nooutfile gnu_startup_file 'p2'
$ on control_y then goto bugout
$ on error then goto bugout
$ write/error=bugout gnu_startup_file "$ __debug_save_verify = 'f$verify(0)'"
$ write/error=bugout gnu_startup_file "$! Generated from ",p1
$!
$
$ started := no
$ keep := yes
$ comment_i = 0
$ comment0 = ""
$loop_file:
$ line_i = 0
$ line0 = ""
$loop_file2:
$ line'line_i' = ""
$ read/end=nomore/error=bugout gnu_data_file line'line_i'
$ if f$edit(line'line_i',"TRIM") .eqs. "" then goto loop_file2
$ if f$extract(0,1,line'line_i') .eqs. "!"
$  then
$   if started
$    then
$     write/error=bugout gnu_startup_file "$",line'line_i'
$    else
$     comment'comment_i' = "$" + line'line_i'
$     comment_i = comment_i + 1
$    endif
$   goto loop_file2
$  endif
$ if f$extract(f$length(line'line_i')-1, 1, line'line_i') .eqs. "-"
$  then
$   line_i = line_i + 1
$   goto loop_file2
$  endif
$ if line_i .eq. 0 .and. line0 .eqs. "" then goto loop_file
$resume_loop_file:
$ if f$extract(0,1,line0) .nes. " "
$  then
$   if started then goto several_p ! Shouldn't be necessary!
$   started := yes
$   logicals = ","
$   title = ""
$   e = f$element(0," ",line0)
$   utility = f$element(0,"-",e)
$   version = f$element(1,"-",e)
$   if f$length(line0) .gt. f$length(e) then -
	title=f$edit(f$extract(f$length(e)+1,f$length(line0),line0),"TRIM")
$   if title .eqs. "" then title = e
$   write sys$error "%GNU_GENERATE_SCRIPT-I-GENTITLE, Generating ",p2," for ",title
$
$   write/error=bugout gnu_startup_file "$! This is the startup file for ",title
$   type sys$input /out=gnu_startup_file
$DECK
$!
$! Description:
$!
$! P1 = comma-separated flags.
$!
$! The flags can be any of these.  If one flag appears several times,
$! the last occurense gets precedense over the others:
$!	TABLE=/SYSTEM		logicals are defined in the system table
$!	TABLE=/GROUP		logicals are defined in the group table
$!	TABLE=/JOB		logicals are defined in the job table
$!	TABLE=/PROCESS		logicals are defined in the process table
$!				(this is the default)
$!      INSTALL_IMAGE		install the image
$!	NOINSTALL_IMAGE 	do NOT install the image (default)
$!	LOGICALS		define the needed logicals if not yet defined
$!				(default)
$!	NOLOGICALS		do NOT define any logical
$!      FORCELOGICALS		define the needed logicals even if already
$!				defined!
$!      NOTON=(node[,...])	do not do this on the node "node"
$!
$!	VERBOSE			Tell the user what you're doing (default)
$!      NOVERBOSE		Be quiet!
$!	QUIET			alias for NOVERBOSE
$!
$!	VERIFY			As DEBUG, but DO execute the statements.
$!	NOVERIFY
$!
$ ON CONTROL_Y THEN GOTO bugout
$ ON ERROR THEN GOTO bugout
$ _tmp_status = 1
$ __st_proc = F$ENVIRONMENT("PROCEDURE")
$ __st_startupdir = F$PARSE(__st_proc,,,"NODE")+f$parse(__st_proc,,,"DEVICE")+-
	f$parse(__st_proc,,,"DIRECTORY")
$ __st_node = F$EDIT(F$GETSYI("NODENAME"),"TRIM")
$ __st_keywords = "/TABLE=/NOTON=/INSTALL_IMAGE/LOGICALS/FORCELOGICALS/NOINSTALL_IMAGE/NOLOGICALS/VERBOSE/QUIET/NOVERBOSE/VERIFY/NOVERIFY/"
$ _gnu_debug := NO
$ _gnu_verify := NO
$ _gnu_table = "LNM$PROCESS"
$ _gnu_table_switch = ""
$ _gnu_known_tables = "/LNM$SYSTEM/LNM$GROUP/LNM$JOB/LNM$PROCESS/"
$ _gnu_install_image := NO
$ _gnu_logicals := YES
$ _gnu_exclusions = ","
$ _gnu_antinodes = ","
$ _gnu_verbose := YES
$ _gnu_forcelogicals := NO
$ __st_flags = p1
$__st_loop_flags:
$ IF __st_flags .NES. ""
$  THEN
$   __st_endp = F$LOCATE(",",__st_flags)
$   IF __st_endp .NE. F$LENGTH(__st_flags)
$    THEN
$     __st_p = F$LOCATE("(",__st_flags)
$     IF __st_p .LT. __st_endp
$      THEN
$	__st_p = __st_p + F$LOCATE(")",F$EXTRACT(__st_p, F$LENGTH(__st_flags), __st_flags))
$	IF __st_p .GT. __st_endp THEN
		__st_endp = __st_p + F$LOCATE(",",F$EXTRACT(__st_p, F$LENGTH(__st_flags), __st_flags))
$      ENDIF
$    ENDIF
$   __st_e = F$EXTRACT(0,__st_endp,__st_flags)
$   IF F$EXTRACT(__st_endp,1,__st_flags) .EQS. "," THEN -
	__st_endp = __st_endp + 1
$   __st_flags = F$EXTRACT(__st_endp, F$LENGTH(__st_flags), __st_flags)
$   __st_keyword = F$EXTRACT(0,F$LOCATE("=",__st_e),__st_e)
$   __st_lable = __st_keyword
$   IF __st_keyword .nes. __st_e THEN __st_keyword = __st_keyword + "="
$   __st_value = F$EXTRACT(F$LENGTH(__st_keyword),F$LENGTH(__st_e),__st_e)
$   IF F$EXTRACT(0,1,__st_value) .EQS. "(" -
	.AND. F$EXTRACT(F$LENGTH(__st_value)-1,1,__st_value) .EQS. ")" THEN -
	__st_value = F$EXTRACT(1,F$LENGTH(__st_value)-2,__st_value)
$   IF __st_keywords - ("/"+__st_keyword+"/") .NES. __st_keywords THEN -
	GOTO F_'__st_lable'
$  F_TABLE:
$   _gnu_table_switch := '__st_value'
$   IF _gnu_table_switch .EQS. "/SYSTEM" THEN _gnu_table = "LNM$SYSTEM"
$   IF _gnu_table_switch .EQS. "/GROUP" THEN _gnu_table = "LNM$GROUP"
$   IF _gnu_table_switch .EQS. "/JOB" THEN _gnu_table = "LNM$JOB"
$   IF _gnu_table_switch .EQS. "/PROCESS" .OR. _gnu_table_switch .EQS. "" THEN -
	_gnu_table = "LNM$PROCESS"
$   GOTO __st_loop_flags
$  F_NOTON:
$   _gnu_antinodes = "," + __st_value + ","
$   IF _gnu_antinodes - (","+__st_node+",") .NES. _gnu_antinodes THEN GOTO __st_exit
$   GOTO __st_loop_flags
$  F_EXCLUDE:
$   _gnu_exclusions = _gnu_exclusions + F$EDIT(__st_value,"UPCASE") + ","
$   GOTO __st_loop_flags
$  F_QUIET:
$   __st_keyword := NOVERBOSE
$  F_NOVERBOSE:
$  F_NODEBUG:
$  F_NOVERIFY:
$  F_NOLOGICALS:
$  F_NOINSTALL_IMAGE:
$   _gnu_'F$EXTRACT(2,F$LENGTH(__st_keyword)-2,__st_keyword)' := NO
$   GOTO __st_loop_flags
$  F_DEBUG:
$   _gnu_verify := NO
$   GOTO F_VERBOSE
$  F_VERIFY:
$   _gnu_debug := NO
$  F_VERBOSE:
$  F_LOGICALS:
$  F_INSTALL_IMAGE:
$  F_FORCELOGICALS:
$   _gnu_'__st_keyword' := YES
$   GOTO __st_loop_flags
$  ENDIF
$
$EOD
$   hacked_utility = utility
$   if version .nes. "-"
$    then
$     hacked_utility = hacked_utility
$     version_i = 0
$     c = "$"
$    loop_utility:
$     e = f$element(version_i,".",version)
$     version_i = version_i + 1
$     if e .nes. "."
$      then
$       hacked_utility = hacked_utility + c + e
$       c = "_"
$       goto loop_utility
$      endif
$    endif
$   write/error=bugout gnu_startup_file -
	"$!GNU NODE: GNU_",hacked_utility,":",line0
$
$   open/read gnu_temp_data 'p1'
$   gnu_extra_file := gnu_temp_data
$  loop_data_file:
$   read/end=loop_data_file_end/err=loop_data_file_end gnu_temp_data temp_line
$   write/error=bugout gnu_startup_file "$!GNU DATA: ",temp_line
$   goto loop_data_file
$  loop_data_file_end:
$   close gnu_temp_data
$   gnu_extra_file :=
$
$   write/error=bugout gnu_startup_file "$GNU_",hacked_utility,":"
$   write/error=bugout gnu_startup_file "$ ON CONTROL_Y THEN GOTO ",hacked_utility,"_bugout"
$   write/error=bugout gnu_startup_file "$ ON ERROR THEN GOTO ",hacked_utility,"_bugout"
$   type sys$input /out=gnu_startup_file
$DECK
$ _gnu_verify_stmt = " !"
$ _gnu_noverify_stmt = " !"
$ IF _gnu_verify
$  THEN
$   _gnu_verify_stmt = "__save_verify = F$VERIFY(1) !"
$   _gnu_noverify_stmt = "__save_noverify = 'F$VERIFY(__save_verify)' !"
$  ENDIF
$ IF _gnu_debug THEN SET VERIFY
$EOD
$ write/error=bugout gnu_startup_file -
	"$ if _gnu_verbose then write sys$error ""%GNU_STARTUP-I-SETTING_UP,  setting up "",-"
$ write/error=bugout gnu_startup_file "	""",title,""""
$!  write/error=bugout gnu_startup_file "$ IF _gnu_debug THEN GOTO ",hacked_utility,"_GNU_DEBUG_",hacked_utility
$  else
$   if started
$    then
$     goto A_'f$edit(f$element(0,"\",line0),"collapse")'
$    A_D:
$     name = f$element(1,"\",line0)
$     if f$locate(","+name+",",logicals) .ne. f$length(logicals) then -
	goto loop_file
$     logicals = logicals + name + ","
$     if f$edit(f$extract(0,4,name),"UPCASE") .eqs. "SYS$" then goto loop_file
$     write/error=bugout gnu_startup_file "$ IF .NOT. _gnu_logicals THEN GOTO ",hacked_utility,"_SKIP_",name
$     write/error=bugout gnu_startup_file "$ '_gnu_verify_stmt'"
$     write/error=bugout gnu_startup_file "$ IF .NOT. _gnu_forcelogicals -"
$     write/error=bugout gnu_startup_file "	.AND. F$TRNLNM(""",name,""",,,,,""TABLE_NAME"") .NES. """""
$     write/error=bugout gnu_startup_file "$  THEN"
$     write/error=bugout gnu_startup_file "$   _tmp_table_i = 0"
$     write/error=bugout gnu_startup_file "$  ",hacked_utility,"_DEFTABLOOP_",name,":"
$     write/error=bugout gnu_startup_file "$   _tmp_table_i = _tmp_table_i + 1"
$     write/error=bugout gnu_startup_file "$   _tmp_table = F$ELEMENT(_tmp_table_i,""/"",_gnu_known_tables)"
$     write/error=bugout gnu_startup_file "$   IF _tmp_table .EQS. """" .OR. _tmp_table .EQS. ""/"" THEN GOTO ",hacked_utility,"_SKIP_",name
$     write/error=bugout gnu_startup_file "$   _tmp_exists = F$TRNLNM(""",name,""",_tmp_table,,,,""TABLE_NAME"")"
$     write/error=bugout gnu_startup_file "$   IF _tmp_exists .NES. """" THEN GOTO ",hacked_utility,"_SKIP_",name
$     write/error=bugout gnu_startup_file "$   IF _tmp_table .NES. _gnu_table THEN GOTO ",hacked_utility,"_DEFTABLOOP_",name
$     write/error=bugout gnu_startup_file "$  ENDIF
$     if line_i .le. 0
$      then
$       write/error=bugout gnu_startup_file "$ DEFINE '_gnu_table_switch' ",-
	name,f$element(2,"\",line0)," ",f$element(3,"\",line0)
$      else
$       write/error=bugout gnu_startup_file "$ DEFINE '_gnu_table_switch' ",-
	name,f$element(2,"\",line0)," -"
$       i = 0
$      sub_A_D:
$       i = i + 1
$       if i .le. line_i
$        then
$	  write/error=bugout gnu_startup_file " ",line'i'
$	  goto sub_A_D
$        endif
$      endif
$     write/error=bugout gnu_startup_file "$",hacked_utility,"_SKIP_",name,":"
$     write/error=bugout gnu_startup_file "$ '_gnu_noverify_stmt'"
$     goto loop_file
$    A_SL:
$     write/error=bugout gnu_startup_file "$ '_gnu_verify_stmt'"
$     write/error=bugout gnu_startup_file "$ ",f$element(1,"\",line0)," := ",f$element(2,"\",line0)
$     write/error=bugout gnu_startup_file "$ '_gnu_noverify_stmt'"
$     goto loop_file
$    A_SG:
$     write/error=bugout gnu_startup_file "$ '_gnu_verify_stmt'"
$     write/error=bugout gnu_startup_file "$ ",f$element(1,"\",line0)," :== ",f$element(2,"\",line0)
$     write/error=bugout gnu_startup_file "$ '_gnu_noverify_stmt'"
$     goto loop_file
$    A_CLD:
$     write/error=bugout gnu_startup_file "$ '_gnu_verify_stmt'"
$     write/error=bugout gnu_startup_file "$ SET COMMAND ",f$element(1,"\",line0)
$     write/error=bugout gnu_startup_file "$ '_gnu_noverify_stmt'"
$     goto loop_file
$    A_H:
$     write/error=bugout gnu_startup_file "$ IF .NOT. _gnu_logicals THEN GOTO ",hacked_utility,"_DONT_MAKE_HELP"
$     value = f$element(1,"\",line0)
$     write/error=bugout gnu_startup_file "$ '_gnu_verify_stmt'"
$     write/error=bugout gnu_startup_file "$ _hlp_value := ",value
$     write/error=bugout gnu_startup_file "$ _hlp_value_name := F$PARSE(_hlp_value,,,""NAME"")"
$     write/error=bugout gnu_startup_file "$ _hlp_table_i = 0"
$     write/error=bugout gnu_startup_file "$",hacked_utility,"_HLPTABLOOP:"
$     write/error=bugout gnu_startup_file "$ _hlp_table_i = _hlp_table_i + 1"
$     write/error=bugout gnu_startup_file "$ _tmp_table = F$ELEMENT(_hlp_table_i,""/"",_gnu_known_tables)"
$     write/error=bugout gnu_startup_file "$ _hlp_i = 0"
$     write/error=bugout gnu_startup_file "$",hacked_utility,"_HLPLOOP:"
$     write/error=bugout gnu_startup_file "$ _hlp_name = ""HLP$LIBRARY"""
$     write/error=bugout gnu_startup_file "$ IF _hlp_i .NE. 0 THEN _hlp_name = _hlp_name + ""_'","'_hlp_i'"""
$     write/error=bugout gnu_startup_file "$ _hlp_i = _hlp_i + 1"
$     write/error=bugout gnu_startup_file "$ _tmp_value = F$TRNLNM(_hlp_name,_tmp_table)"
$     write/error=bugout gnu_startup_file "$ _tmp_value_name := F$PARSE(_tmp_value,,,""NAME"")"
$     write/error=bugout gnu_startup_file "$ IF _tmp_value .EQS. _hlp_value THEN GOTO ",hacked_utility,"_NO_HLP"
$     write/error=bugout gnu_startup_file "$ IF _tmp_value_name .EQS. _hlp_value_name .AND. _gnu_forcelogicals THEN -"
$     write/error=bugout gnu_startup_file "	GOTO ",hacked_utility,"_HLPDEF"
$     write/error=bugout gnu_startup_file "$ IF _tmp_value .NES. """" THEN GOTO ",hacked_utility,"_HLPLOOP"
$     write/error=bugout gnu_startup_file "$ IF _tmp_table .NES. _gnu_table THEN GOTO ",hacked_utility,"_HLPTABLOOP"
$     write/error=bugout gnu_startup_file "$",hacked_utility,"_HLPDEF:"
$     write/error=bugout gnu_startup_file "$ DEFINE '_gnu_table_switch' '_hlp_name' '_hlp_value'"
$     write/error=bugout gnu_startup_file "$",hacked_utility,"_NO_HLP:"
$     write/error=bugout gnu_startup_file "$ '_gnu_noverify_stmt'"
$     write/error=bugout gnu_startup_file "$",hacked_utility,"_DONT_MAKE_HELP:"
$     goto loop_file
$    A_I:
$     Iprogram = f$element(1,"\",line0)
$     Iflags = f$element(2,"\",line0)
$     Iprogramname = f$parse(Iprogram,,,"NAME","SYNTAX_ONLY")
$     if f$type(I'hacked_utility'_'Iprogramname'_cnt) .eqs. "" then -
	I'hacked_utility'_'Iprogramname'_cnt = 0
$     I'hacked_utility'_'Iprogramname'_cnt = I'hacked_utility'_'Iprogramname'_cnt + 1
$     write/error=bugout gnu_startup_file "$ IF .NOT. _gnu_install_image THEN GOTO ",hacked_utility,"_",Iprogramname,"_DONT_INSTALL_IMAGE",I'hacked_utility'_'Iprogramname'_cnt
$     write/error=bugout gnu_startup_file "$ '_gnu_verify_stmt'"
$     write/error=bugout gnu_startup_file "$ _needed_privs = ""CMKRNL,SYSGBL,PRMGBL"""
$     write/error=bugout gnu_startup_file "$ _command = ""ADD"""
$     write/error=bugout gnu_startup_file "$ _tmp := ",f$element(1,"\",line0)
$     write/error=bugout gnu_startup_file "$ IF f$file_attrib(_tmp,""KNOWN"") THEN _command = ""REPLACE"""
$     write/error=bugout gnu_startup_file "$ _curpriv = f$getjpi("""",""CURPRIV"")"
$     write/error=bugout gnu_startup_file "$ SET PROCESS/PRIV=('_needed_privs)"
$     write/error=bugout gnu_startup_file "$ IF f$privilege(_needed_privs) THEN GOTO ",hacked_utility,"_",Iprogramname,"_HAVE_PRIVS",I'hacked_utility'_'Iprogramname'_cnt
$     write/error=bugout gnu_startup_file "$ WRITE SYS$ERROR ""You need the privileges "",_needed_privs,-"
$     write/error=bugout gnu_startup_file "	"" to install ",hacked_utility," ",Iflags,""""
$     write/error=bugout gnu_startup_file "$ GOTO ",hacked_utility,"_",Iprogramname,"_END_INSTALL_IMAGE",I'hacked_utility'_'Iprogramname'_cnt
$     write/error=bugout gnu_startup_file "$",hacked_utility,"_",Iprogramname,"_HAVE_PRIVS",I'hacked_utility'_'Iprogramname'_cnt,":"
$     write/error=bugout gnu_startup_file "$ _install := $INSTALL/COMMAND_MODE"
$     write/error=bugout gnu_startup_file "$ set noon"
$     write/error=bugout gnu_startup_file "$ _install '_command' ",-
	Iprogram," ",Iflags
$     write/error=bugout gnu_startup_file "$ set on"
$     write/error=bugout gnu_startup_file "$",hacked_utility,"_",Iprogramname,"_END_INSTALL_IMAGE",I'hacked_utility'_'Iprogramname'_cnt,":"
$     write/error=bugout gnu_startup_file "$ SET PROCESS/PRIV=(noall,'_curpriv')"
$     write/error=bugout gnu_startup_file "$ '_gnu_noverify_stmt'"
$     write/error=bugout gnu_startup_file "$",hacked_utility,"_",Iprogramname,"_DONT_INSTALL_IMAGE",I'hacked_utility'_'Iprogramname'_cnt,":"
$     goto loop_file
$    endif
$  endif
$ goto loop_file
$bugout:
$ tmp_status=$status
$ set noon
$ if "''gnu_extra_file'" .nes. ""
$  then
$   close 'gnu_extra_file
$  endif
$ line0 = ""
$ keep := no
$nomore:
$ if keep .and. (line_i .ne. 0 .or. line0 .nes. "") then goto resume_loop_file
$ if keep then write/error=bugout gnu_startup_file "$",hacked_utility,"_bugout:"
$ if keep then write/error=bugout gnu_startup_file "$ a = f$verify(__debug_save_verify)"
$ if keep then write/error=bugout gnu_startup_file "$!GNU NODE END"
$ if keep then write/error=bugout gnu_startup_file "$exit: EXIT"
$ close gnu_startup_file
$ if .not. keep then delete 'f$parse(p2,"*.*;*")'
$nooutfile:
$ if tmp_status .eq. 1 .and. .not. keep then tmp_status = $status
$ close gnu_data_file
$noinfile:
$ if tmp_status .eq. 1 .and. .not. keep then tmp_status = $status
$exit:
$ exit tmp_status + 0*f$verify(__debug_save_verify)
