$!
$ set symbol/verb/scope=(noglobal,nolocal)
$ if f$search("''p2'") .nes. ""
$  then
$   diff 'p1' 'p2'/output=nla0:
$   if $severity .eq. 3
$    then
$     set prot=(owner:red,group:red,world:red) 'p2'.*
$     delete/nolog 'p2'.*
$     copy/nolog 'p1' 'p2'
$     delete/nolog 'p1'.*
$    else
$     write sys$output "''p2' is unchanged."
$     delete/nolog 'p1'.*
$    endif
$  else
$   copy/nolog 'p1' 'p2'
$   delete/nolog 'p1'.*
$  endif
$ exit
