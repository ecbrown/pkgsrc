$! $NetBSD$
$!
$! Define the OpenVMS C RTL features and DCL foreign commands needed by
$! the pkgsrc ispell images.  Invoke this procedure directly from the DCL
$! process that will run ispell; definitions made in a spawned subprocess
$! do not propagate back to its parent.
$!
$ _ispell_saved_verify_proc = f$environment("VERIFY_PROCEDURE")
$ _ispell_saved_verify_image = f$environment("VERIFY_IMAGE")
$ _ispell_junk = f$verify(0,0)
$ _ispell_status = %x10000002
$ _ispell_default_changed = 0
$ _ispell_scope_changed = 0
$ _ispell_rollback = 0
$ _ispell_made_stream = 0
$ _ispell_made_popen = 0
$ _ispell_changed_ispell = 0
$ _ispell_changed_defmt_c = 0
$ _ispell_changed_defmt_sh = 0
$ _ispell_had_ispell = 0
$ _ispell_had_defmt_c = 0
$ _ispell_had_defmt_sh = 0
$ _ispell_old_ispell = ""
$ _ispell_old_defmt_c = ""
$ _ispell_old_defmt_sh = ""
$ _ispell_saved_default = f$environment("DEFAULT")
$ _ispell_saved_scope = f$environment("SYMBOL_SCOPE")
$ _ispell_saved_control = f$environment("CONTROL")
$ _ispell_control_y_enabled = -
	(f$locate("Y",_ispell_saved_control) .lt. f$length(_ispell_saved_control))
$ on control_y then goto _ispell_control_y
$ on error then goto _ispell_failed
$!
$ _ispell_procedure = f$environment("PROCEDURE")
$ _ispell_procdir = f$parse(_ispell_procedure,,,"NODE") + -
	f$parse(_ispell_procedure,,,"DEVICE") + -
	f$parse(_ispell_procedure,,,"DIRECTORY")
$ _ispell_default_changed = 1
$ set default '_ispell_procdir'
$ _ispell_bin = f$parse("[-.BIN]",,,"NODE") + -
	f$parse("[-.BIN]",,,"DEVICE") + -
	f$parse("[-.BIN]",,,"DIRECTORY")
$ set default '_ispell_saved_default'
$ _ispell_default_changed = 0
$!
$ _ispell_image = _ispell_bin + "ISPELL.EXE"
$ if f$search(_ispell_image) .eqs. "" then -
	_ispell_image = _ispell_bin + "ISPELL."
$ if f$search(_ispell_image) .eqs. "" then goto _ispell_missing_ispell
$ _ispell_defmt_c = _ispell_bin + "DEFMT-C.EXE"
$ if f$search(_ispell_defmt_c) .eqs. "" then -
	_ispell_defmt_c = _ispell_bin + "DEFMT-C."
$ if f$search(_ispell_defmt_c) .eqs. "" then goto _ispell_missing_defmt_c
$ _ispell_defmt_sh = _ispell_bin + "DEFMT-SH.EXE"
$ if f$search(_ispell_defmt_sh) .eqs. "" then -
	_ispell_defmt_sh = _ispell_bin + "DEFMT-SH."
$ if f$search(_ispell_defmt_sh) .eqs. "" then goto _ispell_missing_defmt_sh
$!
$! Do not overwrite an existing search list or conflicting feature setting;
$! that cannot be restored losslessly by a setup procedure.  USER-mode
$! definitions must be checked first because they shadow SUPERVISOR mode.
$ _ispell_mode = f$trnlnm("DECC$STREAM_PIPE","LNM$PROCESS_TABLE",0,-
	"USER",,"ACCESS_MODE")
$ _ispell_user_stream = (_ispell_mode .eqs. "USER")
$ if _ispell_user_stream
$ then
$   _ispell_max = f$trnlnm("DECC$STREAM_PIPE","LNM$PROCESS_TABLE",0,-
	"USER",,"MAX_INDEX")
$   _ispell_value = f$trnlnm("DECC$STREAM_PIPE","LNM$PROCESS_TABLE",0,-
	"USER",,"VALUE")
$   if (_ispell_max .ne. 0) .or. -
	(f$edit(_ispell_value,"TRIM,UPCASE") .nes. "ENABLE") then -
	goto _ispell_logical_conflict
$ endif
$ _ispell_mode = f$trnlnm("DECC$STREAM_PIPE","LNM$PROCESS_TABLE",0,-
	"SUPERVISOR",,"ACCESS_MODE")
$ _ispell_super_stream = (_ispell_mode .eqs. "SUPERVISOR")
$ _ispell_need_stream = (.not. _ispell_super_stream)
$ if _ispell_super_stream
$ then
$   _ispell_max = f$trnlnm("DECC$STREAM_PIPE","LNM$PROCESS_TABLE",0,-
	"SUPERVISOR",,"MAX_INDEX")
$   _ispell_value = f$trnlnm("DECC$STREAM_PIPE","LNM$PROCESS_TABLE",0,-
	"SUPERVISOR",,"VALUE")
$   if (_ispell_max .ne. 0) .or. -
	(f$edit(_ispell_value,"TRIM,UPCASE") .nes. "ENABLE") then -
	goto _ispell_logical_conflict
$ endif
$ _ispell_mode = f$trnlnm("DECC$POPEN_NO_CRLF_REC_ATTR",-
	"LNM$PROCESS_TABLE",0,"USER",,"ACCESS_MODE")
$ _ispell_user_popen = (_ispell_mode .eqs. "USER")
$ if _ispell_user_popen
$ then
$   _ispell_max = f$trnlnm("DECC$POPEN_NO_CRLF_REC_ATTR",-
	"LNM$PROCESS_TABLE",0,"USER",,"MAX_INDEX")
$   _ispell_value = f$trnlnm("DECC$POPEN_NO_CRLF_REC_ATTR",-
	"LNM$PROCESS_TABLE",0,"USER",,"VALUE")
$   if (_ispell_max .ne. 0) .or. -
	(f$edit(_ispell_value,"TRIM,UPCASE") .nes. "ENABLE") then -
	goto _ispell_logical_conflict
$ endif
$ _ispell_mode = f$trnlnm("DECC$POPEN_NO_CRLF_REC_ATTR",-
	"LNM$PROCESS_TABLE",0,"SUPERVISOR",,"ACCESS_MODE")
$ _ispell_super_popen = (_ispell_mode .eqs. "SUPERVISOR")
$ _ispell_need_popen = (.not. _ispell_super_popen)
$ if _ispell_super_popen
$ then
$   _ispell_max = f$trnlnm("DECC$POPEN_NO_CRLF_REC_ATTR",-
	"LNM$PROCESS_TABLE",0,"SUPERVISOR",,"MAX_INDEX")
$   _ispell_value = f$trnlnm("DECC$POPEN_NO_CRLF_REC_ATTR",-
	"LNM$PROCESS_TABLE",0,"SUPERVISOR",,"VALUE")
$   if (_ispell_max .ne. 0) .or. -
	(f$edit(_ispell_value,"TRIM,UPCASE") .nes. "ENABLE") then -
	goto _ispell_logical_conflict
$ endif
$!
$! Hide inherited local symbols while taking exact snapshots of the globals.
$ _ispell_scope_changed = 1
$ set symbol/general/scope=(nolocal,global)
$ _ispell_had_ispell = (f$type(ISPELL) .nes. "")
$ if _ispell_had_ispell then _ispell_old_ispell = ISPELL
$ _ispell_had_defmt_c = (f$type(DEFMT_C) .nes. "")
$ if _ispell_had_defmt_c then _ispell_old_defmt_c = DEFMT_C
$ _ispell_had_defmt_sh = (f$type(DEFMT_SH) .nes. "")
$ if _ispell_had_defmt_sh then _ispell_old_defmt_sh = DEFMT_SH
$!
$! Disable Ctrl/Y for the small mutation transaction.  ON ERROR remains in
$! force, and cleanup restores the caller's original Ctrl/Y setting.
$ set nocontrol=y
$ _ispell_rollback = 1
$ if _ispell_need_stream
$ then
$   _ispell_made_stream = 1
$   define/process/supervisor_mode DECC$STREAM_PIPE "ENABLE"
$ endif
$ if _ispell_need_popen
$ then
$   _ispell_made_popen = 1
$   define/process/supervisor_mode DECC$POPEN_NO_CRLF_REC_ATTR "ENABLE"
$ endif
$ _ispell_changed_ispell = 1
$ ISPELL == "$" + _ispell_image
$ _ispell_changed_defmt_c = 1
$ DEFMT_C == "$" + _ispell_defmt_c
$ _ispell_changed_defmt_sh = 1
$ DEFMT_SH == "$" + _ispell_defmt_sh
$ _ispell_rollback = 0
$ _ispell_status = 1
$ goto _ispell_done
$!
$_ispell_missing_ispell:
$ write sys$error "%ISPELL-E-NOIMAGE, cannot find ISPELL image in ''_ispell_bin'"
$ goto _ispell_done
$_ispell_missing_defmt_c:
$ write sys$error "%ISPELL-E-NOIMAGE, cannot find DEFMT-C image in ''_ispell_bin'"
$ goto _ispell_done
$_ispell_missing_defmt_sh:
$ write sys$error "%ISPELL-E-NOIMAGE, cannot find DEFMT-SH image in ''_ispell_bin'"
$ goto _ispell_done
$_ispell_logical_conflict:
$ write sys$error -
  "%ISPELL-E-LOGICAL, conflicting process feature logical is already defined"
$ goto _ispell_done
$_ispell_failed:
$ _ispell_status = $status .or. %x10000000
$ goto _ispell_done
$_ispell_control_y:
$ _ispell_status = %x1000002C
$_ispell_done:
$ _ispell_exit_status = _ispell_status
$ set noon
$ set nocontrol=y
$ if _ispell_rollback .and. _ispell_changed_defmt_sh .and. -
	_ispell_had_defmt_sh then DEFMT_SH == _ispell_old_defmt_sh
$ if _ispell_rollback .and. _ispell_changed_defmt_sh .and. -
	(.not. _ispell_had_defmt_sh) then delete/symbol/global/nolog DEFMT_SH
$ if _ispell_rollback .and. _ispell_changed_defmt_c .and. -
	_ispell_had_defmt_c then DEFMT_C == _ispell_old_defmt_c
$ if _ispell_rollback .and. _ispell_changed_defmt_c .and. -
	(.not. _ispell_had_defmt_c) then delete/symbol/global/nolog DEFMT_C
$ if _ispell_rollback .and. _ispell_changed_ispell .and. -
	_ispell_had_ispell then ISPELL == _ispell_old_ispell
$ if _ispell_rollback .and. _ispell_changed_ispell .and. -
	(.not. _ispell_had_ispell) then delete/symbol/global/nolog ISPELL
$ if _ispell_rollback .and. _ispell_made_popen then -
	deassign/process/supervisor_mode/nolog DECC$POPEN_NO_CRLF_REC_ATTR
$ if _ispell_rollback .and. _ispell_made_stream then -
	deassign/process/supervisor_mode/nolog DECC$STREAM_PIPE
$ if _ispell_default_changed then set default '_ispell_saved_default'
$ if _ispell_scope_changed then -
	set symbol/general/scope=('_ispell_saved_scope')
$ if _ispell_control_y_enabled then set control=y
$ _ispell_junk = f$verify(_ispell_saved_verify_proc,-
	_ispell_saved_verify_image)
$ exit _ispell_exit_status
