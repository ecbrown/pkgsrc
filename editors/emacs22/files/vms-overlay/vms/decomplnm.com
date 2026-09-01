$ __save_verif = 'f$verify(0)
$! DECOMPLNM.COM -- Decomposes logical names into their value
$! P1 = logical name
$! P2 = global variable to put result in
$! P3 = A comma-separated list of keywords:
$!	DIRS if the logical name is expected to have a directory name as val.
$!	ATTRIBUTES if the caller wants the attributes of the values.
$!	NO_SYSTEM if nothing should be done with logicals which are defined
$!		/SYSTEM.
$! P4 = A global variable which will hold the eventual /NAME attributes
$!	of the logical name.
$
$ proc = f$environment("PROCEDURE")
$ proc_dir = f$parse(proc,,,"NODE")+f$parse(proc,,,"DEVICE")+f$parse(proc,,,"DIRECTORY")
$ p3 = ","+p3+","
$ __debug = f$trnlnm("DEBUG_DECOMPLNM") -
	.or. f$locate(",DEBUG,",p3) .ne. f$length(p3)
$ __verify = f$locate(",VERIFY,",p3) .ne. f$length(p3)
$ if __verify then set verify
$ __lnmdir_p = f$locate(",DIRS,",p3) .ne. f$length(p3)
$ __lnmattr_p = f$locate(",ATTRIBUTES,",p3) .ne. f$length(p3)
$ __lnmdeep_p = f$locate(",DEEP,",p3) .ne. f$length(p3)
$ __lnmnosys_p = f$locate(",NO_SYSTEM,",p3) .ne. f$length(p3)
$ __lnmdevice_p = f$locate(",RETURN_DEVICE,",p3) .ne. f$length(p3)
$
$ if p1 .eqs. "" then goto nothing
$
$ __lnm = p1
$ __lnmresultprefix := __lnm
$ if __debug
$  then
$   sh sym __lnmdir_p
$   sh sym __lnmattr_p
$   sh sym __lnmdeep_p
$   sh sym __lnmnosys_p
$   sh sym __lnmdevice_p
$  endif
$ if p2 .eqs. "" then p2 := __result
$
$ 'p2' :==
$! The default result with a directory is to give back the same string.
$ if __lnmdevice_p then 'p2' == p1
$
$ gosub parse_lnm
$
$ if p4 .nes. "" then 'p4' == __lnm_name_attr
$
$ __i = 0
$ 'p2' :== 
$loop:
$ __i = __i + 1
$ if __i .le. __lnm_n
$  then
$   if __debug
$    then
$     sh sym __lnm_'__i'
$    endif
$   if 'p2' .nes. "" then 'p2' == 'p2' + ","
$   'p2' == 'p2' + __lnm_'__i'
$   goto loop
$  endif
$ goto exit
$
$parse_lnm:
$ if f$type('__lnmresultprefix'_n) .eqs. ""
$  then
$   __parse_lnm_depth = 0
$   '__lnmresultprefix'_n = 0
$   __dirtoadd = ""
$   __lnm_name_attr = ""
$   __lnm_value_attr = ""
$  endif
$
$ __parse_lnm_depth = __parse_lnm_depth + 1
$ __pld = __parse_lnm_depth
$ __node'__pld' :=
$ __lnm'__pld' :=
$ __dirtoadd'__pld' :=
$
$ if __lnmdir_p
$  then
$   __node'__pld' = f$parse(__lnm,,,"NODE","SYNTAX_ONLY")
$   if __node'__pld' .nes. "" 
$    then
$     if __node'__pld' .eqs. f$getsyi("NODENAME") + "::"
$      then
$	__lnm = __lnm - __node'__pld'
$      else
$       '__lnmresultprefix'_1 = __lnm
$	'__lnmresultprefix'_n = 1
$	__parse_lnm_depth = __parse_lnm_depth - 1
$	return
$      endif
$    endif
$   if __lnm - ":" - "[" - "<" .eqs. __lnm
$    then
$     if f$trnlnm(__lnm) .nes. ""
$      then __lnm = __lnm + ":"
$      else goto nothing
$      endif
$    endif
$   __lnm'__pld' = f$element(0,":",__lnm)
$   if __lnm'__pld' .eqs. __lnm
$    then __lnm'__pld' = ""
$    else __lnm'__pld' = __lnm'__pld' + ":"
$    endif
$   __dir'__pld' = f$extract(f$length(__lnm'__pld'),f$length(__lnm),__lnm)
$   if __dir'__pld' - "<" .nes. __dir'__pld' then -
	__dir'__pld' = "[" + (__dir'__pld' - "<" - ">") + "]"
$   if __dirtoadd .nes. ""
$    then
$     if __dir'__pld' .eqs. ""
$      then
$       __dirtoadd'__pld' = __dirtoadd
$      else
$       __dirtoadd = __dirtoadd - "[000000." - "[000000]" - "[" - "]"
$       if __dirtoadd .nes. "" then __dirtoadd = "." + __dirtoadd
$       __dirtoadd'__pld' = __dir'__pld' - ".]" + __dirtoadd + "]"
$      endif
$    else
$     __dirtoadd'__pld' = __dir'__pld'
$   endif
$   if __debug
$    then
$     sh sym __node'__pld'
$     sh sym __lnm'__pld'
$     sh sym __dir'__pld'
$     sh sym __dirtoadd'__pld'
$     wait 0:0:1
$    endif
$  else
$   __lnm'__pld' = __lnm
$  endif
$ if __pld .eq. 1 .and. f$trnlnm(__lnm1 - ":") .eqs. "" then goto nothing
$
$ if __lnmnosys_p -
	.and. f$trnlnm(__lnm'__pld' - ":",,,,,"TABLE_NAME") .eqs. "LNM$SYSTEM_TABLE"
$  then
$   __lnm_value_n'__pld' = ""
$   if __pld .eq. 1
$    then
$     __node'__pld' = ""
$     __lnm'__pld' = ""
$     __dirtoadd'__pld' = ""
$     __lnm_value_attr = ""
$    endif
$  else
$   __lnm_value_n'__pld' = f$trnlnm(__lnm'__pld' - ":",,,,,"MAX_INDEX")
$  endif
$ __lnm_value_attr'__pld' = ""
$ if __lnm_value_n'__pld' .nes. ""
$  then
$   if __lnmattr_p .and. __pld .eq. 1
$    then
$     __lnm_name_attr = ""
$     __confined = f$trnlnm(__lnm'__pld' - ":",,,,,"CONFINE")
$     __no_alias = f$trnlnm(__lnm'__pld' - ":",,,,,"NO_ALIAS")
$     if __confined then __lnm_name_attr = __lnm_name_attr + "CONFINE,"
$     if __no_alias then __lnm_name_attr = __lnm_name_attr + ",NO_ALIAS"
$     if __lnm_name_attr .nes. "" then -
	  __lnm_name_attr = "/NAME=(" + (__lnm_name_attr - ",") + ")"
$     if __debug then sh sym __lnm_name_attr
$    endif
$   __lnm_value_n'__pld' = __lnm_value_n'__pld' + 1
$   __i'__pld' = 0
$  loop_lnm:
$   if __i'__pld' .lt. __lnm_value_n'__pld' .and. __lnmdir_p
$    then
$     if __lnmattr_p .and. __pld .eq. 1
$      then
$       __lnm_value_attr = ""
$	__concealed = f$trnlnm(__lnm'__pld' - ":",,__i'__pld',,,"CONCEALED")
$	__terminal = f$trnlnm(__lnm'__pld' - ":",,__i'__pld',,,"TERMINAL")
$       if __concealed then -
		__lnm_value_attr = __lnm_value_attr + "CONCEAL,"
$       if __terminal then -
		__lnm_value_attr = __lnm_value_attr + ",TERMINAL"
$       if __lnm_value_attr .nes. "" then -
		__lnm_value_attr = -
			"/TRANS=(" + (__lnm_value_attr - ",") + ")"
$       if __debug
$	 then
$	  sh sym __lnm'__pld'
$	  sh sym __i'__pld'
$	  sh sym __lnm_value_attr
$	 endif
$      endif
$     __lnm_value'__pld' = f$trnlnm(__lnm'__pld' - ":",,__i'__pld')
$     __lnm = __lnm_value'__pld'
$     __dirtoadd = __dirtoadd'__pld'
$     gosub parse_lnm
$     __pld = __parse_lnm_depth
$     __i'__pld' = __i'__pld' + 1
$     goto loop_lnm
$    endif
$  else
$   '__lnmresultprefix'_n = '__lnmresultprefix'_n + 1
$   tmp = '__lnmresultprefix'_n
$   '__lnmresultprefix'_'tmp' = __node'__pld'+__lnm'__pld'+__dirtoadd'__pld'+__lnm_value_attr
$   if __debug then -
	sh sym '__lnmresultprefix'_'tmp'
$  endif
$ __parse_lnm_depth = __parse_lnm_depth - 1
$ return
$
$nothing:
$! 'p2' == ""
$ if p4 .nes. "" then 'p4' == ""
$exit:
$ a = f$verify(__save_verif)