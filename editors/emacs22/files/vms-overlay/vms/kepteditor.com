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
$ !
$ !
$ ! Modified by Richard Levitte to behave a little differently when
$ ! called on an X-windows machine, and also to recognize the -nw
$ ! and the -batch option.
$
$	args = f$edit(p2 + " " + p3 + " " + p4 + " " + p5 + " " + p6 + -
		" " + p7 + " " + p8, "LOWERCASE")
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
$	if p1 .eqs. "EMACS" then -
	orig_name	= "Emacs " + f$trnlnm ( "TT" ) - ":"
$	if p1 .eqs. "TEMACS" then -
	orig_name	= "TEmacs " + f$trnlnm ( "TT" ) - ":"
$	if p1 .eqs. "TEMACS_D" then -
	orig_name	= "TEmacs_D " + f$trnlnm ( "TT" ) - ":"
$	if is_emacs then -
	orig_name	= p1 + " " + f$trnlnm ( "TT" ) - ":"
$	name		= orig_name
$	priv_list	= f$setprv ("NOWORLD, NOGROUP")
$	try		= 0
$
$	! If we look up an Emacs process, it will want to know what it's
$	! current "foster" parent is.
$	if attach_to_emacs_process then -
	definee/job/nolog EMACS_PARENT_PID 'f$getjpi("","PID")'
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
$ 	priv_list	= f$setprv ( priv_list )
$ 	write sys$error spawn_message
$	if p1 .nes. "TPU" then -
$		goto check_emacs
$ 	definee/user	sys$input	sys$command
$ 	spawn	/process="''NAME'" -
    		/nolog'extra_qualifiers' -
    		edit/'p1' 'args'
$ 	goto 'quit_label
$ check_emacs:
$	if .not. is_emacs then -
$		goto un_kempt
$	definee/user	sys$input	sys$command
$	spawn	/process="''NAME'" -
		/nolog -
		/symbols -
		'extra_qualifiers' run'p1' 'args'
$	goto 'quit_label
$ un_kempt:
$ ! The editor is unruly - spawn a process and let the user deal with the
$ ! editor himself.
$	spawn	/process="''NAME'" -
		/nolog'extra_qualifiers'
$	goto 'quit_label
$ attach:
$	if attach_to_emacs_process then goto attach1
$	try		= try + 1
$	try_str		:= 'try'
$	name		= f$extract(0,14-f$length(try_str),orig_name) -
				+ ":" + try_str
$	goto 10$
$ attach1:
$ 	priv_list	= f$setprv ( priv_list )
$	message_status	= f$environment("message")
$	set noon
$	on control_y then goto quit
$	if a .eqs. "-NW" then args = args - f$edit(p'i',"lowercase")
$	set message /nofacility/noidentification/noseverity/notext
$	if f$edit(args,"COLLAPSE") .eqs. "" then goto no_logical
$	! We have to include the current directory as first item in the value
$	definee/nolog/job emacs_command_args -
	       "''f$trnlnm(""SYS$DISK"")'''f$directory()' ''args'"
$ no_logical:
$ 	write sys$error -
"[Attaching to process ''NAME']"
$ 	definee/user	sys$input	sys$command
$ 	attach "''NAME'"
$ quit:
$	set noon
$	if f$edit(args,"COLLAPSE") .eqs. "" then goto quit1
$	deassign/job emacs_command_args
$ quit1:
$	set message 'message_status
$ 	write sys$error -
"[Attached to DCL in directory ''F$TRNLNM("SYS$DISK")'''F$DIRECTORY()']"
$ quit_not_kept:
$	exit 1 + 0*f$verify(verify)
$
$ batch:
$	run'p1' 'args'
