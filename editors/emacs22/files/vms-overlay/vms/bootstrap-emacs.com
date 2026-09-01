$! run bootstrap-emacs.exe
$!
$! first arg always expected to be "-BATCH" or "--BATCH"
$	if 0 ! p1 .eqs. "" .or. (p1 .nes. "-BATCH" .and. p1 .nes. "--BATCH")
$	 then
$	  write sys$output "unexpected p1: ", p1
$	  exit 0
$	 endif
$!
$! do it
$	define/user dbg$input [-.vms]temacs_d.input
$	mcr [-.src]bootstrap-emacs -
		-map [-.src]bootstrap-emacs.dump -
		'p1' 'p2' 'p3' 'p4' 'p5'
$!
$! bye
$	exit 
$!
