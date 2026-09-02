$!
$ set symbol/verb/scope=(noglobal,nolocal)
$ move_status = 1
$ if f$search("''p2'") .nes. ""
$  then
$   differences 'p1' 'p2'/output=nla0:
$   diff_status = $status
$!  Saving $STATUS is itself a successful DCL command, so $SEVERITY no
$!  longer necessarily describes DIFFERENCES.  Decode the saved condition
$!  value instead; DIFF-I-DIFFERENCES (severity 3) means the files differ.
$   diff_severity = diff_status .and. 7
$   if diff_status .eq. %X006C8009 then goto unchanged
$   if diff_severity .ne. 3
$    then
$     move_status = diff_status
$     goto done
$    endif
$   set protection=(owner:rwed) 'p2'.*
$  endif
$ copy/nolog 'p1' 'p2'
$ move_status = $status
$ if .not. move_status then goto done
$ delete/nolog 'p1'.*
$ move_status = $status
$ if .not. move_status then goto done
$ purge/nolog/keep=1 'p2'
$ move_status = $status
$ goto done
$unchanged:
$ write sys$output "''p2' is unchanged."
$ delete/nolog 'p1'.*
$ move_status = $status
$done:
$ exit move_status
