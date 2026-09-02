$ verify = 'f$verify (0)
$ !
$ ! Kept_Editor.COM
$ ! Command file for use on VMS to spawn an Emacs process
$ ! that can be suspended with C-z and will not go away
$ ! when other programs are run.  This is the normal way
$ ! for users to invoke Emacs on VMS; the command "emacs"
$ ! is normally defined to execute this file.
$ !
$ ! Joe Kelsey
$ ! FlexComm Corp.
$ !
$ ! September, 1985
$ !
$ ! Run or attach to an editor in a kept fork.
$ !
$ ! Modified by Marty Sasaki to define the job logical name
$ ! "EMACS_FILE_NAME" with the value of the filename on the command
$ ! line. Lisp code can then use the value of the logical to resume or
$ ! to start editing in that file.
$ ! Modified again by Roland B. Roberts to use "EMACS_COMMAND_ARGS"
$ ! instead.
$ ! Modified for pkgsrc to preserve argument case and boundaries with
$ ! one job logical name per argument when attaching to a kept Emacs.
$ !
$ !
$ ! Modified by Richard Levitte to behave a little differently when
$ ! called on an X-windows machine, and also to recognize the -nw
$ ! and the -batch option.
$
$	! Reconstruct the image command line without changing argument case.
$	! Quote each DCL procedure parameter separately so that spaces in an
$	! ODS-5 file name do not turn one argument into several arguments.
$	last_arg = 1
$	i = 2
$ scan_args:
$	if p'i' .nes. "" then last_arg = i
$	i = i + 1
$	if i .le. 8 then goto scan_args
$	args = ""
$	i = 2
$ build_args:
$	if i .gt. last_arg then goto args_built
$	argument = p'i'
$	gosub quote_argument
$	if .not. quote_status then -
		exit quote_status + 0*f$verify(verify)
$	if args .nes. "" then args = args + " "
$	args = args + quoted_argument
$	i = i + 1
$	goto build_args
$ args_built:
$	! Reserve 128 characters for the longest SPAWN command prefix.
$	if f$length(args) .gt. 896
$	then
$		write sys$error -
"%EMACS-E-BADARG, combined argument list exceeds the DCL command limit"
$		exit %x1000002c + 0*f$verify(verify)
$	endif
$	resume_count = 0
$	kept_status = 1
$	parent_pid_changed = 0
$	parent_pid_was_defined = 0
$	privileges_reduced = 0
$	message_status = f$environment("message")
$	is_emacs = p1 .eqs. "EMACS" -
		.or. p1 .eqs. "TEMACS" .or. p1 .eqs. "TEMACS_D"
$ 	attach_to_emacs_process :== yes
$ 	extra_qualifiers == ""
$ 	spawn_message = "[Spawning a new Kept ''P1']"
$ 	quit_label :== quit1
$
$ ! The following loop is needed, to handle the -nw option right.
$ 	i = 1
$ no_win_loop:
$ 	a = f$edit(p'i',"UPCASE")
$ 	if a .eqs. "-NW" then goto keptemacs
$ 	if a .eqs. "-BATCH" .and. is_emacs then goto batch
$ 	i = i + 1
$ 	if i .le. 8 then goto no_win_loop
$
$ 	if (f$trnlnm("DECW$DISPLAY") .eqs. "") then goto keptemacs
$
$  	attach_to_emacs_process :== no
$  	extra_qualifiers == "/NOWAIT/INPUT=NL:"
$  	spawn_message = "[Spawning a new ''P1']"
$  	quit_label :== quit_not_kept
$ keptemacs:
$
$	edit		= ""
$	! OpenVMS process names are limited to 15 characters.  TT can translate
$	! to a node-qualified name such as _X86923$FTA325:, so use the device
$	! portion and retain its rightmost characters when space is tight.
$	terminal_name = f$trnlnm("TT") - ":"
$	terminal_dollar = f$locate("$",terminal_name)
$	if terminal_dollar .lt. f$length(terminal_name) then -
	terminal_name = f$extract(terminal_dollar+1,255,terminal_name)
$	terminal_room = 14 - f$length(p1)
$	if terminal_room .le. 0 then orig_name = f$extract(0,15,p1)
$	if terminal_room .le. 0 then goto process_name_ready
$	terminal_start = f$length(terminal_name) - terminal_room
$	if terminal_start .lt. 0 then terminal_start = 0
$	orig_name = p1 + " " + -
	f$extract(terminal_start,terminal_room,terminal_name)
$ process_name_ready:
$	name		= orig_name
$	parent_pid_status = 1
$	on error then goto kept_error
$	on control_y then goto kept_control_y_early
$	world_restore = "NOWORLD"
$	if f$privilege("WORLD") then world_restore = "WORLD"
$	group_restore = "NOGROUP"
$	if f$privilege("GROUP") then group_restore = "GROUP"
$	priv_restore = world_restore + "," + group_restore
$	privileges_reduced = 1
$	priv_result = f$setprv("NOWORLD,NOGROUP")
$	try		= 0
$
$ 10$:
$	pid 		= 0
$ 20$:
$ 	proc		= f$getjpi ( f$pid ( pid ), "PRCNAM")
$ 	if proc .eqs. name then -
$		goto attach
$ 	if pid .ne. 0 then -
$		goto 20$
$ spawn:
$ 	if .not. privileges_reduced then goto spawn_privileges_restored
$ 	priv_result	= f$setprv ( priv_restore )
$	privileges_reduced = 0
$ spawn_privileges_restored:
$	set noon
$	parent_pid_status = 1
$	if attach_to_emacs_process then gosub set_parent_pid
$	if .not. parent_pid_status then kept_status = parent_pid_status
$	if .not. parent_pid_status then goto quit1
$	set on
$ 	write sys$error spawn_message
$	if p1 .nes. "TPU" then -
$		goto check_emacs
$ 	definee/user	sys$input	sys$command
$	kept_status = $status
$	if .not. kept_status then goto 'quit_label
$ 	spawn	/process="''NAME'" -
    		/nolog'extra_qualifiers' -
    		edit/'p1' 'args'
$	kept_status = $status
$ 	goto 'quit_label
$ check_emacs:
$	if .not. is_emacs then -
$		goto un_kempt
$	definee/user	sys$input	sys$command
$	kept_status = $status
$	if .not. kept_status then goto 'quit_label
$	spawn	/process="''NAME'" -
		/nolog -
		/symbols -
		'extra_qualifiers' run'p1' 'args'
$	kept_status = $status
$	goto 'quit_label
$ un_kempt:
$ ! The editor is unruly - spawn a process and let the user deal with the
$ ! editor himself.
$	spawn	/process="''NAME'" -
		/nolog'extra_qualifiers'
$	kept_status = $status
$	goto 'quit_label
$ attach:
$	if attach_to_emacs_process then goto attach1
$	try		= try + 1
$	try_str		:= 'try'
$	name		= f$extract(0,14-f$length(try_str),orig_name) -
				+ ":" + try_str
$	goto 10$
$ attach1:
$ 	if .not. privileges_reduced then goto attach_privileges_restored
$ 	priv_result	= f$setprv ( priv_restore )
$	privileges_reduced = 0
$ attach_privileges_restored:
$	message_status	= f$environment("message")
$	set noon
$	on control_y then goto kept_control_y_resume
$	set message /nofacility/noidentification/noseverity/notext
$	gosub clear_resume_logicals
$	if .not. clear_status then kept_status = clear_status
$	if .not. clear_status then goto quit
$	! Store arguments in separate logical names.  A flat, space-separated
$	! string cannot preserve both case and argument boundaries.
$	i = 2
$ define_resume_args:
$	if i .gt. last_arg then goto resume_args_defined
$	argument = p'i'
$	a = f$edit(argument,"TRIM,UPCASE")
$	if a .eqs. "-NW" then goto next_resume_arg
$	resume_count = resume_count + 1
$	logical_name = "EMACS_COMMAND_ARG_" + f$string(resume_count)
$	gosub quote_argument
$	if .not. quote_status then goto attach_quote_failure
$	definee/nolog/job 'logical_name' 'quoted_argument'
$	define_status = $status
$	if .not. define_status then goto attach_define_failure
$ next_resume_arg:
$	i = i + 1
$	goto define_resume_args
$ resume_args_defined:
$	if resume_count .eq. 0 then goto no_logical
$	current_directory = f$trnlnm("SYS$DISK") + f$directory()
$	argument = current_directory
$	gosub quote_argument
$	if .not. quote_status then goto attach_quote_failure
$	definee/nolog/job emacs_command_directory 'quoted_argument'
$	define_status = $status
$	if .not. define_status then goto attach_define_failure
$	definee/nolog/job emacs_command_arg_count 'resume_count'
$	define_status = $status
$	if .not. define_status then goto attach_define_failure
$ no_logical:
$	parent_pid_status = 1
$	gosub set_parent_pid
$	if .not. parent_pid_status then kept_status = parent_pid_status
$	if .not. parent_pid_status then goto quit
$ 	write sys$error -
"[Attaching to process ''NAME']"
$ 	definee/user	sys$input	sys$command
$	define_status = $status
$	if .not. define_status then goto attach_define_failure
$ 	attach "''NAME'"
$	attach_status = $status
$	if .not. attach_status then kept_status = attach_status
$	goto quit
$ attach_define_failure:
$	kept_status = define_status
$	goto quit
$ attach_quote_failure:
$	kept_status = quote_status
$	goto quit
$ kept_error:
$	kept_status = $status
$	goto quit1
$ kept_control_y_early:
$	kept_status = %x1000002c
$	goto quit1
$ kept_control_y_resume:
$	kept_status = %x1000002c
$	goto quit
$ quit:
$	set noon
$	gosub clear_resume_logicals
$	if .not. kept_status then goto quit1
$	if .not. clear_status then kept_status = clear_status
$ quit1:
$	set noon
$	if .not. privileges_reduced then goto quit_privileges_restored
$	priv_result = f$setprv(priv_restore)
$	privileges_reduced = 0
$ quit_privileges_restored:
$	gosub restore_parent_pid
$	if .not. kept_status then goto quit_status_ready
$	if .not. parent_restore_status then kept_status = parent_restore_status
$ quit_status_ready:
$	set message 'message_status
$	if kept_status then write sys$error -
"[Attached to DCL in directory ''F$TRNLNM("SYS$DISK")'''F$DIRECTORY()']"
$ quit_not_kept:
$	exit kept_status + 0*f$verify(verify)
$
$ batch:
$	set noon
$	run'p1' 'args'
$	kept_status = $status
$	exit kept_status + 0*f$verify(verify)
$
$ set_parent_pid:
$	! A kept Emacs uses this job logical to find its current foster parent.
$	! Preserve only a single, ordinary supervisor-mode value; otherwise a
$	! temporary definition could mask or destroy state that cannot be rebuilt.
$	parent_pid_status = 1
$	parent_pid_visible_mode = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"USER",,"ACCESS_MODE")
$	if parent_pid_visible_mode .eqs. "USER" then goto parent_pid_invalid
$	parent_pid_saved_mode = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"ACCESS_MODE")
$	if parent_pid_saved_mode .eqs. "" then goto define_parent_pid
$	if parent_pid_saved_mode .nes. "SUPERVISOR" then -
		goto parent_pid_invalid
$	parent_pid_max_index = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"MAX_INDEX")
$	if f$type(parent_pid_max_index) .nes. "INTEGER" then -
		goto parent_pid_invalid
$	if parent_pid_max_index .ne. 0 then goto parent_pid_invalid
$	parent_pid_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"CONCEALED")
$	if parent_pid_attribute .nes. "FALSE" then goto parent_pid_invalid
$	parent_pid_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"TERMINAL")
$	if parent_pid_attribute .nes. "FALSE" then goto parent_pid_invalid
$	parent_pid_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"CONFINE")
$	if parent_pid_attribute .nes. "FALSE" then goto parent_pid_invalid
$	parent_pid_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"CRELOG")
$	if parent_pid_attribute .nes. "FALSE" then goto parent_pid_invalid
$	parent_pid_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"NO_ALIAS")
$	if parent_pid_attribute .nes. "FALSE" then goto parent_pid_invalid
$	parent_pid_was_defined = 1
$	argument = f$trnlnm("EMACS_PARENT_PID","LNM$JOB", -
		0,"SUPERVISOR",,"VALUE")
$	if f$length(argument) .eq. 0 then goto parent_pid_invalid
$	gosub quote_argument
$	if .not. quote_status then parent_pid_status = quote_status
$	if .not. quote_status then return 1
$	parent_pid_saved_value = quoted_argument
$ define_parent_pid:
$	parent_pid_installed_value = f$getjpi("","PID")
$	definee/job/supervisor_mode/nolog EMACS_PARENT_PID -
		'parent_pid_installed_value'
$	parent_pid_status = $status
$	if parent_pid_status then parent_pid_changed = 1
$	return 1
$ parent_pid_invalid:
$	write sys$error -
"%EMACS-E-PARENT, existing EMACS_PARENT_PID cannot be preserved safely"
$	parent_pid_status = %x1000002c
$	return 1
$
$ restore_parent_pid:
$	parent_restore_status = 1
$	if .not. parent_pid_changed then return 1
$	set noon
$	parent_current_visible_mode = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"USER",,"ACCESS_MODE")
$	if parent_current_visible_mode .eqs. "USER" then -
		goto parent_pid_restore_conflict
$	parent_current_mode = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"ACCESS_MODE")
$	if parent_current_mode .nes. "SUPERVISOR" then -
		goto parent_pid_restore_conflict
$	parent_current_max_index = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"MAX_INDEX")
$	if f$type(parent_current_max_index) .nes. "INTEGER" then -
		goto parent_pid_restore_conflict
$	if parent_current_max_index .ne. 0 then goto parent_pid_restore_conflict
$	parent_current_value = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"VALUE")
$	if parent_current_value .nes. parent_pid_installed_value then -
		goto parent_pid_restore_conflict
$	parent_current_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"CONCEALED")
$	if parent_current_attribute .nes. "FALSE" then -
		goto parent_pid_restore_conflict
$	parent_current_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"TERMINAL")
$	if parent_current_attribute .nes. "FALSE" then -
		goto parent_pid_restore_conflict
$	parent_current_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"CONFINE")
$	if parent_current_attribute .nes. "FALSE" then -
		goto parent_pid_restore_conflict
$	parent_current_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"CRELOG")
$	if parent_current_attribute .nes. "FALSE" then -
		goto parent_pid_restore_conflict
$	parent_current_attribute = f$trnlnm("EMACS_PARENT_PID", -
		"LNM$JOB",0,"SUPERVISOR",,"NO_ALIAS")
$	if parent_current_attribute .nes. "FALSE" then -
		goto parent_pid_restore_conflict
$	if .not. parent_pid_was_defined then goto remove_parent_pid
$	definee/job/supervisor_mode/nolog EMACS_PARENT_PID -
		'parent_pid_saved_value'
$	parent_restore_status = $status
$	goto parent_pid_restored
$ remove_parent_pid:
$	deassign/job/supervisor_mode/nolog EMACS_PARENT_PID
$	parent_restore_status = $status
$ parent_pid_restored:
$	if parent_restore_status then parent_pid_changed = 0
$	return 1
$ parent_pid_restore_conflict:
$	write sys$error -
"%EMACS-E-PARENT, EMACS_PARENT_PID changed before it could be restored"
$	parent_restore_status = %x1000002c
$	return 1
$
$ clear_resume_logicals:
$	! COUNT is the commit marker.  Remove every protocol logical both before
$	! publishing a request and after ATTACH returns, including the zero-arg case.
$	clear_status = 1
$	logical_name = "EMACS_COMMAND_ARG_COUNT"
$	gosub clear_one_resume_logical
$	logical_name = "EMACS_COMMAND_DIRECTORY"
$	gosub clear_one_resume_logical
$	i = 1
$ clear_resume_arg_loop:
$	logical_name = "EMACS_COMMAND_ARG_" + f$string(i)
$	gosub clear_one_resume_logical
$	i = i + 1
$	if i .le. 7 then goto clear_resume_arg_loop
$	return 1
$ clear_one_resume_logical:
$	deassign/job/supervisor_mode/nolog 'logical_name'
$	clear_one_status = $status
$	if .not. clear_status then return 1
$	if .not. clear_one_status then clear_status = clear_one_status
$	return 1
$
$ quote_argument:
$	! Return ARGUMENT as a DCL quoted string, doubling embedded quotes.
$	! Two adjacent apostrophes inside that string would force DCL symbol
$	! substitution, so there is no literal representation for that input.
$	quote_status = 1
$	if f$length(argument) .gt. 255 then goto quote_argument_invalid
$	quoted_argument = """"
$	quote_index = 0
$	quote_previous_apostrophe = 0
$ quote_argument_loop:
$	if quote_index .ge. f$length(argument) then goto quote_argument_done
$	quote_character = f$extract(quote_index,1,argument)
$	quote_character_value = f$cvui(0,8,quote_character)
$	if quote_character_value .lt. 32 .or. -
		quote_character_value .eq. 127 then goto quote_argument_invalid
$	! One apostrophe is literal inside a DCL quoted string; a pair is not.
$	quote_apostrophe = quote_character .eqs. "'"
$	if quote_apostrophe .and. quote_previous_apostrophe then -
		goto quote_argument_invalid
$	quote_previous_apostrophe = quote_apostrophe
$	if quote_character .eqs. """" then -
		quoted_argument = quoted_argument + """"
$	quoted_argument = quoted_argument + quote_character
$	quote_index = quote_index + 1
$	goto quote_argument_loop
$ quote_argument_done:
$	quoted_argument = quoted_argument + """"
$	return 1
$ quote_argument_invalid:
$	write sys$error -
"%EMACS-E-BADARG, argument cannot be passed literally and safely through DCL"
$	! SS$_ABORT is even; STS$M_INHIB_MSG leaves the diagnostic above as
$	! the only message while preserving failure through caller cleanup.
$	quote_status = %x1000002c
$	return 1
