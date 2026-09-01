$! Roland Blaine Roberts (roberts@nsrl31.nsrl.rochester.edu)
$! Nuclear Structure Research Laboratory
$! University of Rochester
$! Friday, 17 April 1992 
$!
$! Command procedure to invoke Emacs for VMS MAIL editor.  The logical
$! name MAIL$EDIT should translate to this file.
$!
$! Arguments:
$!   p1 = Input file name.
$!   p2 = Output file name.
$!
$! Logical Names:
$!   MAIL$TRAILER file to be appended to the message
$!
$ args = "-f vms-pmail-setup"
$!
$ ppid = f$getjpi("","MASTER_PID")
$ pima = f$getjpi("''ppid'","IMAGNAME")
$ if (f$locate("PMDF",pima) .ne. f$length(pima)) then goto PEMF
$!
$! VAXmail
$!
$ file :=
$ if p1 .nes. "" then file = f$search(p1)
$ if file .nes. "" then -
     args = "-insert ''file' ''args' -f indicate-mail-reply-text"
$ goto COMMON
$!
$! Pmdf Mail
$!
$PEMF:
$ if (p1 .nes. "") .and. (f$search(p1) .nes. "") then -
    args = " ''args' -f indicate-mail-reply-text"
$!
$COMMON:
$ trailer = f$trnlnm("MAIL$TRAILER")
$ if trailer .nes. "" then -
     args = args + " -f insert-signature"
$ runemacs "-q" 'p2' "''args'"
$ exit
$!
$! Modification history
$! 
$! 21-OCT-1994/Hege: 
$! * check for VAXmail or Pmdf Mail added
$! * replaced "-insert" with "-f delete-other-windows" for Pmdf Mail, as 
$!   "-insert" puts the message replied upon in our buffer - twice 
