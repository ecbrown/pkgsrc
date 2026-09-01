$! GNU_STARTUP.COM -- version 2.1.10 -- General startup script.
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
$! GNU_STARTUP.COM should get updated automagically with
$! GNU_STARTUP_MANAGER.COM
$!
$ goto past_description
$description:
$ type sys$input
   Description:
  
   Pn1 = comma-separated utilities to install, n1 = 1, 3, 5, 7
   Pn2 = flags for this group of utilities, n2 = 2, 4, 6, 8
  
   The flags can be any of these.  If one flag appears several times,
   the last occurense gets precedense over the others:
  	TABLE=/SYSTEM		logicals are defined in the system table
  	TABLE=/GROUP		logicals are defined in the group table
  	TABLE=/JOB		logicals are defined in the job table
  	TABLE=/PROCESS		logicals are defined in the process table
  				(this is the default)
        INSTALL_IMAGE		install the image
  	NOINSTALL_IMAGE 	do NOT install the image (default)
  	LOGICALS		define the needed logicals if not yet defined
  				(default)
  	NOLOGICALS		do NOT define any logical
        FORCELOGICALS		define the needed logicals even if already
  				defined!
        NOTON=(node[,...])	do not do this on the node "node"
        EXCLUDE=(prg[,...])	do not do this on the node "node"
  	VERBOSE			Tell the user what you're doing (default)
        NOVERBOSE		Be quiet!
  	QUIET			alias for NOVERBOSE
  
  	DEBUG			Write the statements on the screen, and
  				do NOT execute them.
        NODEBUG			Do not debug.
  	VERIFY			As DEBUG, but DO execute the statements.
  	NOVERIFY
  
  	HELP			Generates som this help text, then exits.
$ return
$!
$past_description:
$ on control_y then goto __st_exit
$ on error then goto __st_exit
$ _tmp_status = 1
$ __st_proc = f$environment("PROCEDURE")
$ __st_node = f$edit(f$getsyi("NODENAME"),"TRIM")
$ __st_n2 = 0
$ _gnu_debug := no
$ _gnu_verify := no
$ __st_ulist = ","
$!
$! A list of known utilities.  The form of each item is:
$! GNU_LABEL:doc-string
$ __st_known_utility1 = "GNU_EMACS$19_28:EMACS-19.28 Emacs version 19.28"
$ __st_known_utility_n = 1
$ __st_keywords = "/TABLE=/NOTON=/EXCLUDE=/INSTALL_IMAGE/LOGICALS/FORCELOGICALS/NOINSTALL_IMAGE/NOLOGICALS/VERBOSE/QUIET/NOVERBOSE/DEBUG/NODEBUG/VERIFY/NOVERIFY/HELP/"
$ exn = 0
$__st_loop_args:
$ __st_n1 = __st_n2 + 1
$ __st_n2 = __st_n1 + 1
$ if __st_n2 .gt. 8 then goto __st_exit
$ _gnu_install_image := NO
$ _gnu_logicals := YES
$ _gnu_exclusions = ","
$ _gnu_antinodes = ","
$ _gnu_verbose := YES
$ _gnu_forcelogicals := no
$ _gnu_table_switch :=
$ _gnu_table := LNM$PROCESS
$ _gnu_known_tables = "/LNM$SYSTEM/LNM$GROUP/LNM$JOB/LNM$PROCESS/"
$ __st_flags = p'__st_n2'
$__st_loop_flags:
$ if __st_flags .nes. ""
$  then
$   __st_endp = f$locate(",",__st_flags)
$   if __st_endp .ne. f$length(__st_flags)
$    then
$     __st_p = f$locate("(",__st_flags)
$     if __st_p .lt. __st_endp
$      then
$	__st_p = __st_p + -
	f$locate(")",f$extract(__st_p, f$length(__st_flags), __st_flags))
$	if __st_p .gt. __st_endp then
		__st_endp = __st_p + -
		f$locate(",",-
			 f$extract(__st_p, f$length(__st_flags), __st_flags))
$      endif
$    endif
$   __st_e = f$extract(0,__st_endp,__st_flags)
$   if f$extract(__st_endp,1,__st_flags) .eqs. "," then -
	__st_endp = __st_endp + 1
$   __st_flags = f$extract(__st_endp, f$length(__st_flags), __st_flags)
$   __st_keyword = f$extract(0,f$locate("=",__st_e),__st_e)
$   __st_lable = __st_keyword
$   if __st_keyword .nes. __st_e then __st_keyword = __st_keyword + "="
$   __st_value = f$extract(f$length(__st_keyword),f$length(__st_e),__st_e)
$   if f$extract(0,1,__st_value) .eqs. "(" -
	.and. f$extract(f$length(__st_value)-1,1,__st_value) .eqs. ")" then -
	__st_value = f$extract(1,f$length(__st_value)-2,__st_value)
$   if __st_keywords - ("/"+__st_keyword+"/") .nes. __st_keywords then -
	goto __st_F_'__st_lable'
$  __st_F_TABLE:
$   _gnu_'__st_lable'_switch := '__st_value'
$   IF _gnu_table_switch .EQS. "/SYSTEM" THEN _gnu_table = "LNM$SYSTEM"
$   IF _gnu_table_switch .EQS. "/GROUP" THEN _gnu_table = "LNM$GROUP"
$   IF _gnu_table_switch .EQS. "/JOB" THEN _gnu_table = "LNM$JOB"
$   IF _gnu_table_switch .EQS. "/PROCESS" -
	.OR. _gnu_table_switch .EQS. "" THEN -
	_gnu_table = "LNM$PROCESS"
$   goto __st_loop_flags
$  __st_F_NOTON:
$   _gnu_antinodes = "," + __st_value + ","
$   if _gnu_antinodes - (","+__st_node+",") .nes. __st_antinodes then -
	goto __st_loop_args
$   goto __st_loop_flags
$  __st_F_EXCLUDE:
$   _gnu_exclusions = __st_exclusions + f$edit(__st_value,"UPCASE") + ","
$   goto __st_loop_flags
$  __st_F_QUIET:
$   __st_keyword := NOVERBOSE
$  __st_F_NOVERBOSE:
$  __st_F_NODEBUG:
$  __st_F_NOVERIFY:
$  __st_F_NOLOGICALS:
$  __st_F_NOINSTALL_IMAGE:
$   _gnu_'f$extract(2,f$length(__st_keyword)-2,__st_keyword)' := NO
$   goto __st_loop_flags
$  __st_F_DEBUG:
$   _gnu_verify := NO
$   goto __st_F_VERBOSE
$  __st_F_VERIFY:
$   _gnu_debug := NO
$  __st_F_VERBOSE:
$  __st_F_LOGICALS:
$  __st_F_INSTALL_IMAGE:
$  __st_F_FORCELOGICALS:
$   _gnu_'__st_keyword' := YES
$   goto __st_loop_flags
$  __st_F_HELP:
$   gosub description
$   exit 1
$  endif
$ __st_en = 0
$__st_loop_utilities: ! GNU_RETURN
$ __st_e = f$edit(f$element(__st_en,",",p'__st_n1'),"UPCASE")
$ __st_en = __st_en + 1
$ if __st_e .eqs. "," .or. p'__st_n1' .eqs. "" then goto __st_loop_args
$ __st_ku = 0
$ __st_resume_lable = "__st_loop_utilities"
$__st_loop_known_utilities:
$ __st_ku = __st_ku + 1
$ if __st_ku .le. __st_known_utility_n
$  then
$   __st_lable = f$element(0,":",__st_known_utility'__st_ku')
$   if __st_e .nes. "*"
$    then
$     __st_u = f$element(0," ",f$element(1,":",__st_known_utility'__st_ku'))
$     __st_u = f$edit(__st_u,"UPCASE")
$     if __st_u - __st_e .eqs. __st_u then goto __st_loop_known_utilities
$     __st_c = f$extract(0,1,__st_u - __st_e)
$     if __st_c .nes. "" -
	.and. __st_c .nes. "." .and. __st_c .nes. "-" then -
		goto __st_loop_known_utilities
$    else
$     __st_resume_lable = "__st_loop_known_utilities"
$    endif
$   goto '__st_lable'
$  endif
$ if __st_e .nes. "*" then -
  write sys$error "%GNU_STARTUP-W-NOSUCHUTILITY, there is no utility ",	__st_e
$ goto __st_loop_utilities
$__st_exit:
$ exit 1 + 0*f$verify(__debug_save_verify)
$!
$! Here will follow the different nodes...
$!GNU NODE: GNU_EMACS$19_28:EMACS-19.28 Emacs version 19.28
$!GNU DATA: !# Generated from STORE:[SRC.EMACS-19_28.VMS]EMACS_STARTUP.DAT_IN and [.VMS]CONFIG.DAT
$!GNU DATA: EMACS-19.28 Emacs version 19.28
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\STORE\\ -
$!GNU DATA:     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\SYS$COMMON\\ -
$!GNU DATA:     OBELIX$DKA200:[SYS0.SYSCOMMON.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\SYS$COMMON\\ -
$!GNU DATA:     OBELIX$DKA200:[SYS0.SYSCOMMON.]/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\eshell\\"*dcl*"
$!GNU DATA:  SL\bindir\STORE:[EMACS.BIN]
$!GNU DATA:  SL\vmslibdir\STORE:[EMACS.LIB.EMACS.VMS]
$!GNU DATA:  SL\mandir\STORE:[EMACS.HELP]
$!GNU DATA:  SG\runemacs\$'bindir'emacs-19_28 -map 'bindir'emacs-19_28.dump
$!GNU DATA:  SG\emacs\@'vmslibdir'kepteditor emacs
$!GNU DATA:  SG\etags\$'bindir'etags
$!GNU DATA:  SG\b2m\$'bindir'b2m
$!GNU DATA:  SG\emacsclient\@'bindir'emacsclient
$!GNU DATA:  H\'mandir'gnu
$!GNU DATA:  I\'bindir'emacs-19_28.exe\/open/share/header
$!GNU DATA:  I\'bindir'emacs-19_28.exe\/open/share/header/priv=sysgbl
$GNU_EMACS$19_28:
$ _gnu_verify_stmt = " !"
$ _gnu_noverify_stmt = " !"
$ IF _gnu_verify
$  THEN
$   _gnu_verify_stmt = "__save_verify = F$VERIFY(1) !"
$   _gnu_noverify_stmt = "__save_noverify = 'F$VERIFY(__save_verify)' !"
$  ENDIF
$ IF _gnu_debug THEN SET VERIFY
$ if _gnu_verbose then write sys$error "%GNU_STARTUP-I-SETTING_UP,  setting up ",-
	"Emacs version 19.28"
$ '_gnu_verify_stmt'
$ IF .NOT. _gnu_forcelogicals -
	.AND. F$TRNLNM("STORE",,,,,"TABLE_NAME") .NES. ""
$  THEN
$   _tmp_table_i = 0
$  EMACS$19_28_DEFTABLOOP_STORE:
$   _tmp_table_i = _tmp_table_i + 1
$   _tmp_table = F$ELEMENT(_tmp_table_i,"/",_gnu_known_tables)
$   IF _tmp_table .EQS. "" .OR. _tmp_table .EQS. "/" THEN GOTO EMACS$19_28_SKIP_STORE
$   _tmp_exists = F$TRNLNM("STORE",_tmp_table,,,,"TABLE_NAME")
$   IF _tmp_exists .NES. "" THEN GOTO EMACS$19_28_SKIP_STORE
$   IF _tmp_table .NES. _gnu_table THEN GOTO EMACS$19_28_DEFTABLOOP_STORE
$  ENDIF
$ DEFINE '_gnu_table_switch' STORE -
     OBELIX$DKA200:[STORE.]/TRANS=(CONCEAL,-
     TERMINAL)
$EMACS$19_28_SKIP_STORE:
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ IF .NOT. _gnu_forcelogicals -
	.AND. F$TRNLNM("eshell",,,,,"TABLE_NAME") .NES. ""
$  THEN
$   _tmp_table_i = 0
$  EMACS$19_28_DEFTABLOOP_eshell:
$   _tmp_table_i = _tmp_table_i + 1
$   _tmp_table = F$ELEMENT(_tmp_table_i,"/",_gnu_known_tables)
$   IF _tmp_table .EQS. "" .OR. _tmp_table .EQS. "/" THEN GOTO EMACS$19_28_SKIP_eshell
$   _tmp_exists = F$TRNLNM("eshell",_tmp_table,,,,"TABLE_NAME")
$   IF _tmp_exists .NES. "" THEN GOTO EMACS$19_28_SKIP_eshell
$   IF _tmp_table .NES. _gnu_table THEN GOTO EMACS$19_28_DEFTABLOOP_eshell
$  ENDIF
$ DEFINE '_gnu_table_switch' eshell "*dcl*"
$EMACS$19_28_SKIP_eshell:
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ bindir := STORE:[EMACS.BIN$$'f$getsyi("arch_name")]
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ vmslibdir := STORE:[EMACS.LIB.EMACS.VMS]
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ mandir := STORE:[EMACS.HELP]
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ runemacs :== $'bindir'emacs-19_28 -map 'bindir'emacs-19_28.dump
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ emacs :== @'vmslibdir'kepteditor emacs
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ etags :== $'bindir'etags
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ b2m :== $'bindir'b2m
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ emacsclient :== @'bindir'emacsclient
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ _hlp_value := 'mandir'gnu
$ _hlp_value_name := F$PARSE(_hlp_value,,,"NAME")
$ _hlp_table_i = 0
$EMACS$19_28_HLPTABLOOP:
$ _hlp_table_i = _hlp_table_i + 1
$ _tmp_table = F$ELEMENT(_hlp_table_i,"/",_gnu_known_tables)
$ _hlp_i = 0
$EMACS$19_28_HLPLOOP:
$ _hlp_name = "HLP$LIBRARY"
$ IF _hlp_i .NE. 0 THEN _hlp_name = _hlp_name + "_''_hlp_i'"
$ _hlp_i = _hlp_i + 1
$ _tmp_value = F$TRNLNM(_hlp_name,_tmp_table)
$ _tmp_value_name := F$PARSE(_tmp_value,,,"NAME")
$ IF _tmp_value .EQS. _hlp_value THEN GOTO EMACS$19_28_NO_HLP
$ IF _tmp_value_name .EQS. _hlp_value_name .AND. _gnu_forcelogicals THEN -
	GOTO EMACS$19_28_HLPDEF
$ IF _tmp_value .NES. "" THEN GOTO EMACS$19_28_HLPLOOP
$ IF _tmp_table .NES. _gnu_table THEN GOTO EMACS$19_28_HLPTABLOOP
$EMACS$19_28_HLPDEF:
$ DEFINE '_gnu_table_switch' '_hlp_name' '_hlp_value'
$EMACS$19_28_NO_HLP:
$ '_gnu_noverify_stmt'
$EMACS$19_28_DONT_MAKE_HELP:
$ IF .NOT. _gnu_install_image THEN GOTO EMACS$19_28__DONT_INSTALL_IMAGE1
$ '_gnu_verify_stmt'
$ _needed_privs = "CMKRNL,SYSGBL,PRMGBL"
$ _command = "ADD"
$ _tmp := 'bindir'emacs-19_28.exe
$ IF f$file_attrib(_tmp,"KNOWN") THEN _command = "REPLACE"
$ _curpriv = f$getjpi("","CURPRIV")
$ SET PROCESS/PRIV=('_needed_privs)
$ IF f$privilege(_needed_privs) THEN GOTO EMACS$19_28__HAVE_PRIVS1
$ WRITE SYS$ERROR "You need the privileges ",_needed_privs,-
	" to install EMACS$19_28 /open/share/header"
$ GOTO EMACS$19_28__END_INSTALL_IMAGE1
$EMACS$19_28__HAVE_PRIVS1:
$ _install := $INSTALL/COMMAND_MODE
$ set noon
$ _install '_command' 'bindir'emacs-19_28.exe /open/share/header
$ set on
$EMACS$19_28__END_INSTALL_IMAGE1:
$ SET PROCESS/PRIV=(noall,'_curpriv')
$ '_gnu_noverify_stmt'
$EMACS$19_28__DONT_INSTALL_IMAGE1:
$ IF .NOT. _gnu_install_image THEN GOTO EMACS$19_28__DONT_INSTALL_IMAGE2
$ '_gnu_verify_stmt'
$ _needed_privs = "CMKRNL,SYSGBL,PRMGBL"
$ _command = "ADD"
$ _tmp := 'bindir'emacs-19_28.exe
$ IF f$file_attrib(_tmp,"KNOWN") THEN _command = "REPLACE"
$ _curpriv = f$getjpi("","CURPRIV")
$ SET PROCESS/PRIV=('_needed_privs)
$ IF f$privilege(_needed_privs) THEN GOTO EMACS$19_28__HAVE_PRIVS2
$ WRITE SYS$ERROR "You need the privileges ",_needed_privs,-
	" to install EMACS$19_28 /open/share/header/priv=sysgbl"
$ GOTO EMACS$19_28__END_INSTALL_IMAGE2
$EMACS$19_28__HAVE_PRIVS2:
$ _install := $INSTALL/COMMAND_MODE
$ set noon
$ _install '_command' 'bindir'emacs-19_28.exe /open/share/header/priv=sysgbl
$ set on
$EMACS$19_28__END_INSTALL_IMAGE2:
$ SET PROCESS/PRIV=(noall,'_curpriv')
$ '_gnu_noverify_stmt'
$EMACS$19_28__DONT_INSTALL_IMAGE2:
$EMACS$19_28_bugout:
$ a = f$verify(__debug_save_verify)
$!GNU NODE END
$ GOTO '__st_resume_lable'
