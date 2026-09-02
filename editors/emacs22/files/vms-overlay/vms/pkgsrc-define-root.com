$! Define a short concealed logical rooted at P2.
$! P1 = logical name, P2 = native directory specification.
$ save_verify = f$verify(0)
$ save_parse_style = f$getjpi("","PARSE_STYLE_PERM")
$ set process/parse_style=extended
$ root_status = 1
$ on control_y then goto failed
$ on error then goto failed
$ root_name = f$edit(P1,"UPCASE,TRIM")
$ root_spec = f$edit(P2,"TRIM")
$ root_length = f$length(root_spec)
$ if root_name .eqs. "" then goto invalid
$ if f$locate("::",root_spec) .lt. root_length then goto invalid
$ root_colon = f$locate(":",root_spec)
$ if root_colon .eq. 0 .or. root_colon .ge. root_length then goto invalid
$ root_device = f$extract(0,root_colon,root_spec)
$ root_tail = f$extract(root_colon+1,root_length-root_colon-1,root_spec)
$ root_tail_length = f$length(root_tail)
$ if root_tail_length .lt. 3 then goto invalid
$ root_open = f$extract(0,1,root_tail)
$ root_close = f$extract(root_tail_length-1,1,root_tail)
$ if root_open .eqs. "[" .and. root_close .nes. "]" then goto invalid
$ if root_open .eqs. "<" .and. root_close .nes. ">" then goto invalid
$ if root_open .nes. "[" .and. root_open .nes. "<" then goto invalid
$ root_directory = f$extract(1,root_tail_length-2,root_tail)
$ if root_directory .eqs. "" then goto invalid
$ if f$extract(0,1,root_directory) .eqs. "." .or. -
	f$extract(0,1,root_directory) .eqs. "-" then goto invalid
$ if f$extract(f$length(root_directory)-1,1,root_directory) .eqs. "." -
	then goto invalid
$ root_hops = 0
$translate_device:
$ root_hops = root_hops + 1
$ if root_hops .gt. 32 then goto logical_cycle
$ root_max_index = f$trnlnm(root_device,"LNM$FILE_DEV",,,,"MAX_INDEX")
$ if f$string(root_max_index) .eqs. "" then goto physical_device
$ if root_max_index .ne. 0 then goto search_list
$ root_value = f$edit(f$trnlnm(root_device,"LNM$FILE_DEV",0),"TRIM")
$ value_length = f$length(root_value)
$ if value_length .eq. 0 then goto invalid
$ if f$locate("::",root_value) .lt. value_length then goto invalid
$ value_colon = f$locate(":",root_value)
$ if value_colon .eq. value_length
$  then
$   translated_device = root_value
$   translated_tail = ""
$  else
$   if value_colon .eq. 0 then goto invalid
$   translated_device = f$extract(0,value_colon,root_value)
$   translated_tail = f$extract(value_colon+1,value_length-value_colon-1,-
	root_value)
$  endif
$ if translated_device .eqs. "" then goto invalid
$ if translated_tail .nes. ""
$  then
$   tail_length = f$length(translated_tail)
$   if tail_length .lt. 3 then goto invalid
$   tail_open = f$extract(0,1,translated_tail)
$   tail_close = f$extract(tail_length-1,1,translated_tail)
$   if tail_open .eqs. "[" .and. tail_close .nes. "]" then goto invalid
$   if tail_open .eqs. "<" .and. tail_close .nes. ">" then goto invalid
$   if tail_open .nes. "[" .and. tail_open .nes. "<" then goto invalid
$   root_prefix = f$extract(1,tail_length-2,translated_tail)
$   if root_prefix .eqs. "" then goto invalid
$   if f$extract(f$length(root_prefix)-1,1,root_prefix) .nes. "." -
	then goto invalid
$   if f$extract(0,1,root_prefix) .eqs. "." .or. -
	f$extract(0,1,root_prefix) .eqs. "-" then goto invalid
$!  A rooted physical MFD equivalence contributes no directory components.
$   if f$extract(0,7,root_prefix) .eqs. "000000." then -
	root_prefix = f$extract(7,f$length(root_prefix)-7,root_prefix)
$   if root_prefix .nes. ""
$    then
$!    [000000] denotes the current rooted namespace, not a child directory.
$     if root_directory .eqs. "000000"
$      then
$       root_directory = f$extract(0,f$length(root_prefix)-1,root_prefix)
$      else
$       if f$extract(0,7,root_directory) .eqs. "000000."
$        then
$         root_directory = root_prefix + -
		f$extract(7,f$length(root_directory)-7,root_directory)
$        else
$         root_directory = root_prefix + root_directory
$        endif
$      endif
$    endif
$  endif
$ root_device = translated_device
$ goto translate_device
$physical_device:
$ physical_spec = root_device + ":"
$ if .not. f$getdvi(physical_spec,"EXISTS") then goto invalid
$ if .not. f$getdvi(physical_spec,"DIR") then goto invalid
$ if .not. f$getdvi(physical_spec,"MNT") then goto invalid
$ root_translation = physical_spec + "[" + root_directory + ".]"
$ if f$parse(root_translation,,,"DIRECTORY","SYNTAX_ONLY") .eqs. "" -
	then goto invalid
$ define/process/translation=(concealed,terminal) 'root_name' 'root_translation'
$ root_status = $status
$ goto done
$search_list:
$ write sys$error "%PKGSRC-E-SEARCHLIST, device logical ",root_device,-
	" has multiple equivalence names"
$ root_status = 2
$ goto done
$logical_cycle:
$ write sys$error "%PKGSRC-E-LOGICALCYCLE, device logical translation did not terminate"
$ root_status = 2
$ goto done
$invalid:
$ write sys$error "%PKGSRC-E-BADROOT, cannot root ",root_name," at ",root_spec
$ root_status = 2
$ goto done
$failed:
$ root_status = $status
$done:
$ set process/parse_style='save_parse_style'
$ exit root_status + 0*f$verify(save_verify)
