$ __save_verif = 'f$verify(0)
$! DIRFILENAME.COM -- converts foo:[x.y] to foo:[x]y.dir
$!		      converts foo:[x] to foo:[000000]x.dir
$! P1 = directory spec
$! P2 = global variable to put result in
$ __debug = 0
$
$ __node = f$parse(p1,,,"NODE","SYNTAX_ONLY")
$ __dev = f$parse(p1,,,"DEVICE","SYNTAX_ONLY")
$ if p1 - __dev .eqs. p1 then __dev = ""
$ __dir = p1 - __node - __dev
$ __dir = __dir - "[" - "]" - "<" - ">"
$ if __debug
$  then
$   sh sym __node
$   sh sym __dev
$   sh sym __dir
$  endif
$ __i = 0
$ __e = ""
$loop_dir:
$ __olde = __e
$ __e = f$element(__i,".",__dir)
$ if __e .nes. "."
$  then
$   __i = __i + 1
$   goto loop_dir
$  endif
$
$ if __olde .nes. ""
$  then
$   if __dir - ("."+__olde) .nes. __dir
$    then
$     if __dev .nes. "" .or. __dir - ("."+__olde) .nes. ""
$      then __dir = __dir - ("."+__olde)
$      else __dir = __dir - __olde
$      endif
$    else __dir = __dir - __olde
$    endif
$   __dir = "[" + __dir + "]"
$   if __debug then sh sym __dir
$   if __dir .eqs. "[]" then __dir = "[000000]"
$   if __dir .eqs. "[.]" then __dir = ""
$   'p2' == __node+__dev+__dir+__olde+".DIR"
$   if __debug then sh sym 'p2'/global
$  endif
$exit:
$ a = f$verify(__save_verif)
