$ __debug_save_verify = 'f$verify(0)'
$! Generated from [.VMS]EMACS_STARTUP.DAT
$! This is the startup file for Emacs version 21.2
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
$ _gnu_table = ""
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
$!GNU NODE: GNU_EMACS$21_2:EMACS-21.2 Emacs version 21.2
$!GNU DATA: !# Generated from AXPA:[TTN.EMACS.EMACS212_3.VMS]EMACS_STARTUP.DAT_IN and [.VMS]CONFIG.DAT
$!GNU DATA: EMACS-21.2 Emacs version 21.2
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\AXPA\\ -
$!GNU DATA:     ADAM$DKA0:/TRANS=(CONCEAL,-
$!GNU DATA:     TERMINAL)
$!GNU DATA:  D\eshell\\"*dcl*"
$!GNU DATA:  SL\bindir\AXPA:[TTN.LOCAL.BIN]
$!GNU DATA:  SL\vmslibdir\AXPA:[TTN.LOCAL.LIB.EMACS.VMS]
$!GNU DATA:  SL\mandir\AXPA:[TTN.LOCAL.HELP]
$!GNU DATA:  SG\runemacs\$'bindir'emacs-21_2 -map 'bindir'emacs-21_2.dump
$!GNU DATA:  SG\emacs\@'vmslibdir'kepteditor emacs
$!GNU DATA:  SG\etags\$'bindir'etags
$!GNU DATA:  SG\b2m\$'bindir'b2m
$!GNU DATA:  SG\emacsclient\@'bindir'emacsclient
$!GNU DATA:  H\'mandir'gnu
$!GNU DATA:  I\'bindir'emacs-21_2.exe\/open/share/header
$!GNU DATA:  I\'bindir'emacs-21_2.exe\/open/share/header/priv=sysgbl
$GNU_EMACS$21_2:
$ _gnu_verify_stmt = " !"
$ _gnu_noverify_stmt = " !"
$ IF _gnu_verify
$  THEN
$   _gnu_verify_stmt = "__save_verify = F$VERIFY(1) !"
$   _gnu_noverify_stmt = "__save_noverify = 'F$VERIFY(__save_verify)' !"
$  ENDIF
$ IF _gnu_debug THEN SET VERIFY
$ if _gnu_verbose then write sys$error "%GNU_STARTUP-I-SETTING_UP,  setting up ",-
	"Emacs version 21.2"
$ '_gnu_verify_stmt'
$ IF .NOT. _gnu_forcelogicals -
	.AND. F$TRNLNM("AXPA",,,,,"TABLE_NAME") .NES. ""
$  THEN
$   _tmp_table_i = 0
$  EMACS$21_2_DEFTABLOOP_AXPA:
$   _tmp_table_i = _tmp_table_i + 1
$   _tmp_table = F$ELEMENT(_tmp_table_i,"/",_gnu_known_tables)
$   IF _tmp_table .EQS. "" .OR. _tmp_table .EQS. "/" THEN GOTO EMACS$21_2_SKIP_AXPA
$   _tmp_exists = F$TRNLNM("AXPA",_tmp_table,,,,"TABLE_NAME")
$   IF _tmp_exists .NES. "" THEN GOTO EMACS$21_2_SKIP_AXPA
$   IF _tmp_table .NES. _gnu_table THEN GOTO EMACS$21_2_DEFTABLOOP_AXPA
$  ENDIF
$ DEFINE '_gnu_table_switch' AXPA -
     ADAM$DKA0:/TRANS=(CONCEAL,-
     TERMINAL)
$EMACS$21_2_SKIP_AXPA:
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ IF .NOT. _gnu_forcelogicals -
	.AND. F$TRNLNM("eshell",,,,,"TABLE_NAME") .NES. ""
$  THEN
$   _tmp_table_i = 0
$  EMACS$21_2_DEFTABLOOP_eshell:
$   _tmp_table_i = _tmp_table_i + 1
$   _tmp_table = F$ELEMENT(_tmp_table_i,"/",_gnu_known_tables)
$   IF _tmp_table .EQS. "" .OR. _tmp_table .EQS. "/" THEN GOTO EMACS$21_2_SKIP_eshell
$   _tmp_exists = F$TRNLNM("eshell",_tmp_table,,,,"TABLE_NAME")
$   IF _tmp_exists .NES. "" THEN GOTO EMACS$21_2_SKIP_eshell
$   IF _tmp_table .NES. _gnu_table THEN GOTO EMACS$21_2_DEFTABLOOP_eshell
$  ENDIF
$ DEFINE '_gnu_table_switch' eshell "*dcl*"
$EMACS$21_2_SKIP_eshell:
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ bindir := AXPA:[TTN.LOCAL.BIN]
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ vmslibdir := AXPA:[TTN.LOCAL.LIB.EMACS.VMS]
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ mandir := AXPA:[TTN.LOCAL.HELP]
$ '_gnu_noverify_stmt'
$ '_gnu_verify_stmt'
$ runemacs :== $'bindir'emacs-21_2 -map 'bindir'emacs-21_2.dump
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
$EMACS$21_2_HLPTABLOOP:
$ _hlp_table_i = _hlp_table_i + 1
$ _tmp_table = F$ELEMENT(_hlp_table_i,"/",_gnu_known_tables)
$ _hlp_i = 0
$EMACS$21_2_HLPLOOP:
$ _hlp_name = "HLP$LIBRARY"
$ IF _hlp_i .NE. 0 THEN _hlp_name = _hlp_name + "_''_hlp_i'"
$ _hlp_i = _hlp_i + 1
$ _tmp_value = F$TRNLNM(_hlp_name,_tmp_table)
$ _tmp_value_name := F$PARSE(_tmp_value,,,"NAME")
$ IF _tmp_value .EQS. _hlp_value THEN GOTO EMACS$21_2_NO_HLP
$ IF _tmp_value_name .EQS. _hlp_value_name .AND. _gnu_forcelogicals THEN -
	GOTO EMACS$21_2_HLPDEF
$ IF _tmp_value .NES. "" THEN GOTO EMACS$21_2_HLPLOOP
$ IF _tmp_table .NES. _gnu_table THEN GOTO EMACS$21_2_HLPTABLOOP
$EMACS$21_2_HLPDEF:
$ DEFINE '_gnu_table_switch' '_hlp_name' '_hlp_value'
$EMACS$21_2_NO_HLP:
$ '_gnu_noverify_stmt'
$EMACS$21_2_DONT_MAKE_HELP:
$ IF .NOT. _gnu_install_image THEN GOTO EMACS$21_2__DONT_INSTALL_IMAGE1
$ '_gnu_verify_stmt'
$ _needed_privs = "CMKRNL,SYSGBL,PRMGBL"
$ _command = "ADD"
$ _tmp := 'bindir'emacs-21_2.exe
$ IF f$file_attrib(_tmp,"KNOWN") THEN _command = "REPLACE"
$ _curpriv = f$getjpi("","CURPRIV")
$ SET PROCESS/PRIV=('_needed_privs)
$ IF f$privilege(_needed_privs) THEN GOTO EMACS$21_2__HAVE_PRIVS1
$ WRITE SYS$ERROR "You need the privileges ",_needed_privs,-
	" to install EMACS$21_2 /open/share/header"
$ GOTO EMACS$21_2__END_INSTALL_IMAGE1
$EMACS$21_2__HAVE_PRIVS1:
$ _install := $INSTALL/COMMAND_MODE
$ set noon
$ _install '_command' 'bindir'emacs-21_2.exe /open/share/header
$ set on
$EMACS$21_2__END_INSTALL_IMAGE1:
$ SET PROCESS/PRIV=(noall,'_curpriv')
$ '_gnu_noverify_stmt'
$EMACS$21_2__DONT_INSTALL_IMAGE1:
$ IF .NOT. _gnu_install_image THEN GOTO EMACS$21_2__DONT_INSTALL_IMAGE2
$ '_gnu_verify_stmt'
$ _needed_privs = "CMKRNL,SYSGBL,PRMGBL"
$ _command = "ADD"
$ _tmp := 'bindir'emacs-21_2.exe
$ IF f$file_attrib(_tmp,"KNOWN") THEN _command = "REPLACE"
$ _curpriv = f$getjpi("","CURPRIV")
$ SET PROCESS/PRIV=('_needed_privs)
$ IF f$privilege(_needed_privs) THEN GOTO EMACS$21_2__HAVE_PRIVS2
$ WRITE SYS$ERROR "You need the privileges ",_needed_privs,-
	" to install EMACS$21_2 /open/share/header/priv=sysgbl"
$ GOTO EMACS$21_2__END_INSTALL_IMAGE2
$EMACS$21_2__HAVE_PRIVS2:
$ _install := $INSTALL/COMMAND_MODE
$ set noon
$ _install '_command' 'bindir'emacs-21_2.exe /open/share/header/priv=sysgbl
$ set on
$EMACS$21_2__END_INSTALL_IMAGE2:
$ SET PROCESS/PRIV=(noall,'_curpriv')
$ '_gnu_noverify_stmt'
$EMACS$21_2__DONT_INSTALL_IMAGE2:
$EMACS$21_2_bugout:
$ a = f$verify(__debug_save_verify)
$!GNU NODE END
$exit: EXIT
