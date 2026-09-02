$ __save_verify = 'f$verify(0)
$! COPY.COM	-- Copies files, just like the normal DCL command
$!
$! Qualifiers better come last, as this version is quite lazy, and only
$! really checks P1.
$
$ copy := copy
$ proc = f$element(0,";",f$environment("PROCEDURE"))
$ proc_dir = f$parse("A.;0",proc,,,"SYNTAX_ONLY") - "A.;0"
$
$ default = f$environment("DEFAULT")
$ default_node = ""
$ default_dev = ""
$ default_dir = ""
$!DEBUG
$	dummy := 'default_node'
$	dummy := 'default_dev'
$	dummy := 'default_dir'
$!NODEBUG
$ len = f$length(default)
$ node_p = f$locate("::", default) + 2
$ if node_p .lt. len
$  then
$   default_node = f$extract(0,node_p,default)
$   default = f$extract(node_p,len,default)
$   len = len - node_p
$  endif
$ dev_p = f$locate(":", default)
$ if dev_p .lt. len
$  then
$   default_dev = f$extract(0,dev_p,default)
$   default = f$extract(dev_p+1,len,default)
$   len = len - dev_p - 1
$  endif
$ default_dir = default
$!DEBUG
$	dummy := 'default_node'
$	dummy := 'default_dev'
$	dummy := 'default_dir'
$!NODEBUG
$
$ copy_status = 1
$ prefix = ""
$ postfix = ""
$ i = 0
$loop_param1:
$ i = i + 1
$ if i .le. 8 .and. f$extract(0,1,p'i') .eqs. "/"
$  then
$   prefix = prefix + " " + p'i'
$   goto loop_param1
$  endif
$ files = P'i'
$loop_param2:
$ i = i + 1
$ if i .le. 8
$  then
$   postfix = postfix + " " + p'i'
$   goto loop_param2
$  endif
$
$ _node = default_node
$ _dev = default_dev
$ _dir = default_dir
$
$ file_i = 0
$loop_files:
$ file_e = f$element(file_i,",",files)
$!DEBUG
$	dummy := 'file_e'
$!NODEBUG
$ file_i = file_i + 1
$ if file_e .eqs. "" then goto loop_files	! OK, so I'm forgiving...
$ if file_e .nes. ","
$  then
$   __result = ""
$   delete/symbol/local __result
$   @'proc_dir'canonicaldir 'file_e' __result
$   file_e = __result
$
$   len = f$length(file_e)
$   node_p = f$locate("::", file_e) + 2
$   if node_p .lt. len
$    then
$     _node = f$extract(0,node_p,file_e)
$     len = len - node_p
$     file_e = f$extract(node_p,len,file_e)
$    endif
$   dev_p = f$locate(":", file_e)
$   if dev_p .lt. len
$    then
$     _dev = f$extract(0,dev_p,file_e)
$     len = len - dev_p - 1
$     file_e = f$extract(dev_p+1,len,file_e)
$    endif
$   dir_p = f$locate("]", file_e) + 1
$   _tmpdir = ""
$   if dir_p .lt. len
$    then
$     _tmpdir = f$extract(0,dir_p,file_e)
$     len = len - dir_p
$     file_e = f$extract(dir_p,len,file_e)
$    endif
$
$   c1 = f$extract(0,1,_tmpdir)
$   c2 = f$extract(0,2,_tmpdir)
$!DEBUG
$	dummy := 'c1'		!c1
$	dummy := 'c2'		!c2
$	dummy := 'file_e'	!file_e
$!NODEBUG
$   if c2 .eqs. "[." .or. c2 .eqs. "<."
$    then
$     _dir = default_dir - "]" + (_tmpdir - "[")
$    else
$     if c1 .eqs. "[" .or. c1 .eqs. "<" then _dir = _tmpdir
$    endif
$
$   file_e = _dir + file_e
$
$   max_index = ""
$   if _dev .nes. "" then max_index = f$trnlnm(_dev,,,,,"MAX_INDEX")
$!DEBUG
$	dummy := '_node'	!_node
$	dummy := '_dev'		!_dev
$	dummy := 'max_index'	!max_index
$	dummy := 'file_e'	!file_e
$!NODEBUG
$   if _node .eqs. "" .and. f$string(max_index) .nes. ""
$    then
$     dev_i = max_index
$     __result = ""
$     delete/symbol/local __result
$     @'proc_dir'canonicaldir 'file_e' __result
$     file_e = __result
$!DEBUG
$	dummy := 'file_e'	!file_e
$!NODEBUG
$    loop_devs:
$     if dev_i .ge. 0
$      then
$       dev_e = f$trnlnm(_dev,,dev_i)
$       dev_i = dev_i - 1
$!DEBUG
$	dummy := 'dev_e'	!dev_e
$!NODEBUG
$       if dev_e - "]" .nes. dev_e
$        then
$         newfile = (dev_e - "]") + (file_e - "[")
$        else
$	  if f$extract(f$length(dev_e)-1,1,dev_e) .nes. ":" then -
		dev_e = dev_e + ":"
$         newfile = dev_e + file_e
$        endif
$       __result = ""
$       delete/symbol/local __result
$       @'proc_dir'canonicaldir 'newfile' __result
$!DEBUG
$	dummy := '__result'	!__result
$!NODEBUG
$       @'proc' '__result' 'postfix' 'prefix'
$       copy_result = $status
$       if .not. copy_result .and. copy_status then copy_status = copy_result
$       goto loop_devs
$      endif
$     goto loop_files
$    else
$     __result = ""
$     delete/symbol/local __result
$     @'proc_dir'canonicaldir '_node''_dev':'file_e' __result
$    endif
$   if f$search(__result) .nes. ""
$    then
$     copy '_node''_dev':'file_e' 'postfix' 'prefix'
$     copy_result = $status
$     if .not. copy_result .and. copy_status then copy_status = copy_result
$    else
$     write sys$error "%GNUCOPY-W-NOFILE, file ",_node,_dev,":",file_e," does not exist"
$     if copy_status then copy_status = 2
$    endif
$   goto loop_files
$  endif
$ exit copy_status + 0*f$verify(__save_verify)
