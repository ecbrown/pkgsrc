  




$! This is the VMS version of configure.in
$!#### Configuration script for GNU Emacs
$!#### Copyright (C) 1992, 1994, 2004 Free Software Foundation, Inc.
$!#### This script requires autoconf for VMS version 1.9 or later.
$
$!### This file is part of GNU Emacs.
$
$!### GNU Emacs is free software; you can redistribute it and/or modify
$!### it under the terms of the GNU General Public License as published by
$!### the Free Software Foundation; either version 2, or (at your option)
$!### any later version.
$
$!### GNU Emacs is distributed in the hope that it will be useful,
$!### but WITHOUT ANY WARRANTY; without even the implied warranty of
$!### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
$!### GNU General Public License for more details.
$
$!### You should have received a copy of the GNU General Public License
$!### along with GNU Emacs; see the file COPYING.  If not, write to
$!### the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
$
$!### Since Emacs has configuration requirements that autoconf can't
$!### meet, this file is an unholy marriage of custom-baked
$!### configuration code and autoconf macros.
$!###
$!### We use the m4 quoting characters [ ] (as established by the
$!### autoconf system) to include large sections of raw sewage - Oops, I
$!### mean, shell code - in the final configuration script.
$!###
$!### Usage: @configure config_name
$!###
$!### If configure succeeds, it leaves its status in config.status.
$!### If configure fails after disturbing the status quo,
$!### 	config.status is removed.
$
$ set symbol/verb/scope=(noglobal,nolocal)
$
$ progname = f$element(0,";",f$environment("procedure"))
$ progname_fn = f$parse (progname,,,"NAME") + f$parse(progname,,,"TYPE")
$ say := write sys$output
$ sayerr := write sys$error
$
$ silent := no		! For autoconf 1.11
$ no_create := no	! For autoconf 1.11
$
$!### Establish some default values
$ run_in_place="NO"
$ single_tree="NO"
$ force="NO"
$ verbose="NO"
$ with_debug_hack="NO"
$ with_tcpip="YES"
$ startupdir_dev:=sys$common:
$ startupdir_dir:=sys$startup
$ startupdir:=sys$common:[sys$startup]
$ vuelibdir_dev:=sys$common:
$ vuelibdir_dir:=vue$library.user
$ vuelibdir:=sys$common:[vue$library.user]
$ prefix:=sys$sysdevice:[gnu]
$ prefix_dev = f$parse(prefix,,,"NODE") + f$parse(prefix,,,"DEVICE")
$ prefix_dir = f$parse(prefix,,,"DIRECTORY") - "[" - "<" - "]" - ">"
$ exec_prefix = ""
$ exec_prefix_dev = ""
$ exec_prefix_dir = ""
$ bindir_dev="'exec_prefix_dev'"
$ bindir_dir="'exec_prefix_dir'.bin"
$ bindir="'bindir_dev'['bindir_dir']"
$ datadir_dev="'prefix_dev'"
$ datadir_dir="'prefix_dir'.lib"
$ datadir="'datadir_dev'['datadir_dir']"
$ statedir_dev="'prefix_dev'"
$ statedir_dir="'prefix_dir'.lib"
$ statedir="'statedir_dev'['statedir_dir']"
$ libdir_dev="'exec_prefix_dev'"
$ libdir_dir="'exec_prefix_dir'.lib"
$ libdir="'libdir_dev'['libdir_dir']"
$ mandir_dev="'prefix_dev'"
$ mandir_dir="'prefix_dir'.help"
$ mandir="'mandir_dev'['mandir_dir']"
$ infodir_dev="'prefix_dev'"
$ infodir_dir="'prefix_dir'.info"
$ infodir="'infodir_dev'['infodir_dir']"
$ lispdir_dev="'datadir_dev'"
$ lispdir_dir="'datadir_dir'.emacs.'version_us'.lisp"
$ lispdir="'lispdir_dev'['lispdir_dir']"
$ locallisppath="'datadir_dev'['datadir_dir'.emacs.site-lisp]"
$ lisppath="'locallisppath','lispdir'"
$ etcdir_dev="'datadir_dev'"
$ etcdir_dir="'datadir_dir'.emacs.'version_us'.etc"
$ etcdir="'etcdir_dev'['etcdir_dir']"
$ lockdir_dev="'statedir_dev'"
$ lockdir_dir="'statedir_dir'.emacs.lock"
$ lockdir="'lockdir_dev'['lockdir_dir']"
$ archlibdir_dev="'libdir_dev'"
$ archlibdir_dir="'libdir_dir'.emacs.'version_us'.'configuration_us'"
$ archlibdir="'archlibdir_dev'['archlibdir_dir']"
$ docdir_dev="'datadir_dev'"
$ docdir_dir="'datadir_dir'.emacs.'version_us'.etc"
$ docdir="'docdir_dev'['docdir_dir']"
$ vmslibdir_dev="'libdir_dev'"
$ vmslibdir_dir="'libdir_dir'.emacs.vms"
$ vmslibdir="'vmslibdir_dev'['vmslibdir_dir']"
$
$!### These are all the options we have to change the different paths.
$ path_options="| PREFIX | EXEC_PREFIX | BINDIR | LIBDIR | ETCDIR | DATADIR"
$ path_options=path_options+" | ARCHLIBDIR | STATEDIR | MANDIR | INFODIR"
$ path_options=path_options+" | LISPDIR | LOCKDIR | LISPPATH | LOCALLISPPATH"
$ path_options=path_options+" | DOCDIR"
$ path_options=path_options+" | VMSLIBDIR | STARTUPDIR | VUELIBDIR |"
$
$ goto main
$!
$!!!! Usage messages. These are implemented as subroutine you GOSUB to.
$short_usage:
$ say "Usage: @''progname' CONFIGURATION [-OPTION[=VALUE] ...]"
$ say ""
$ say "Set compilation and installation parameters for GNU Emacs, and report."
$ say "CONFIGURATION specifies the machine and operating system to build for."
$ say "If you omit the CONFIGURATION, ''progname_fn' will find out by itself."
$ say "--WITH-X                 Support the X Window System."
$ say "--WITH-X=NO              Don't support X."
$ say "--WITH-X-TOOLKIT         Use an X toolkit."
$ say "--WITH-X-TOOLKIT=NO      Don't use an X toolkit."
$ say "--WITH-GCC               Use GCC to compile Emacs."
$ say "--WITH-GCC=NO            Don't use GCC to compile Emacs."
$ say "--X-INCLUDES=dev:[dir]   Search for X header files in dev:[dir]."
$ say "--X-LIBRARIES=dev:[dir]  Search for X libraries in dev:[dir]."
$ say "--WITH-NETLIB            Use the NETLIB library (requires SOCKETSHR)."
$ say "--WITH-NETLIB=NO         Don't use the NETLIB library."
$ say "--WITH-UCX               Use the Digital UCX TCP/IP kit."
$ say "--WITH-UCX=NO            Don't use the Digital UCX TCP/IP kit."
$ say "--WITH-MULTINET          Use the MultiNet TCP/IP kit."
$ say "--WITH-MULTINET=NO       Don't use the MultiNet TCP/IP kit."
$ say "--WITH-TCPIP             Use the TCP/IP package which can be found."
$ say "--WITH-TCPIP=NO          Don't use any TCP/IP package."
$ say "--WITH-DEBUG-HACK        Use the dirty debug hack.  See [.SRC]DESCRIP.MMS_IN_IN"
$ say "--RUN-IN-PLACE           Use libraries and data files directly out of the "
$ say "                         source tree."
$ say "--SINGLE-TREE=dev:[dir]  Has the effect of creating a directory tree"
$ say "                         at dev:[dir...] which looks like:"
$ say "                           dev:[dir.BIN.configname] (emacs, etags, etc.)"
$ say "                           dev:[dir.BIN.configname.ETC] (movemail, etc.)"
$ say "                           dev:[dir.COMMON.LISP]  (emacs' lisp files)"
$ say "                           dev:[dir.COMMON.SITE-LISP] (local lisp files)"
$ say "                           dev:[dir.COMMON.LIB] (DOC, TUTORIAL, etc.)"
$ say "                           dev:[dir.COMMON.LOCK] (lockfiles)"
$ say "--SRCDIR=dir             Look for the Emacs source files in DIR."
$ say "--PREFIX=dir             Install files below DIR. Defaults to `''prefix''."
$ say "--FORCE                  Forces a reconfiguration."
$ say ""
$ say "You may also specify any of the `path' variables found in descrip.mms_in,"
$ say "including --bindir, --libdir, --etcdir, --infodir, and so on.  This allows"
$ say "you to override a single default location when configuring."
$ say ""
$ say "If successful, ''progname_fn' leaves its status in config.status.  If"
$ say "unsuccessful after disturbing the status quo, it removes config.status."
$ say ""
$ return
$!
$main:
$!#### Option processing.
$
$!### Record all the arguments, so we can save them in config.status.
$ configure_args = p1+" "+p2+" "+p3+" "+p4+" "+p5+" "+p6+" "+p7+" "+p8+" "
$ arguments=f$edit("''p1' ''p2' ''p3' ''p4' ''p5' ''p6' ''p7' ''p8'",-
	"compress,trim")
$
$!### Don't use shift -- that destroys the argument list, which autoconf needs
$!### to produce config.status.  It turns out that "set - ${arguments}" doesn't
$!### work portably.
$!### In configure.in, it ended up using shift in any case, because some shells
$!### cannot expand ${10}.  We don't need it here, because a .COM file will not
$!### receive more than 8 arguments in any case.  And if it did, it would be no
$!### problem handling it. Thus, we don't need the quoted arguments either.
$ args_from_file := NO
$ arg_line = ""
$ index = 0
$ goto while
$arg_file_end:
$ close arg_file
$ args_from_file := NO
$ arg_line = ""
$while:
$ if index .lt. 8 .or. args_from_file
$  then
$   if .not. args_from_file
$    then
$     index = index+1
$     current_arg = p'index'
$    else
$    arg_file_loop1:
$     if arg_line .eqs. ""
$      then
$	read/error=arg_file_end/end=arg_file_end arg_file arg_line
$       goto arg_file_loop1
$      endif
$     current_arg = ""
$     arg_line = f$edit(arg_line,"TRIM")
$    arg_file_loop2:
$! sh sym arg_line
$     if arg_line .nes. ""
$      then
$       i1 = f$locate(" ",arg_line)
$      arg_file_loop3:
$       i2 = f$locate("""",arg_line)
$       if i2 .lt. i1
$        then
$         current_arg = current_arg + f$extract(0,i2,arg_line)
$         arg_line = f$extract(i2,f$length(arg_line)-i2,arg_line)
$         i1 = i1 - i2
$         i2 = f$locate("""",arg_line)
$         current_arg = current_arg + f$extract(0,i2,arg_line)
$         arg_line = f$extract(i2,f$length(arg_line)-i2,arg_line)
$         i1 = i1 - i2
$         if i1 .lt. i1 then goto arg_file_loop2: ! The space was between quotes
$         goto arg_file_loop3:
$        endif
$       current_arg = current_arg + f$extract(0,i1,arg_line)
$       arg_line = f$extract(i1,f$length(arg_line)-i1,arg_line)
$      endif
$     ! This should make sure the argument is treated the same way as a command
$     ! line argument.
$     current_arg := 'current_arg'
$    endif
$!
$! sh sym current_arg
$   if current_arg .nes. ""
$    then
$     if f$extract(0,1,current_arg) .eqs. "+"
$      then
$       open/read/error=arg_file_end arg_file 'f$extract(1,999,current_arg)'
$       args_from_file := YES
$       goto while
$      endif
$     if f$extract(0,1,current_arg) .eqs. "-"
$      then
$	p = current_arg
$      while2:
$	if f$extract(0,1,p) .eqs. "-"
$	 then
$	  p = f$extract(1,999,p)
$	  goto while2
$	 endif
$	opt = f$element(0,"=",p)
$	val = f$element(1,"=",p)
$	if val .eqs. "="
$	 then
$	  !!! If FOO is a boolean argument, --FOO is equivalent to
$	  !!! --FOO=yes.  Otherwise, the value comes from the next
$	  !!! argument - see below.
$	  valomitted = "YES"
$	  val = "YES"
$	 else
$	  valomitted = "NO"
$	 endif
$	tmp = f$element(0,"-",opt)
$	n = 1
$us_loop: ! us = underscore, not United States :-)
$	e = f$element(n,"-",opt)
$	n = n + 1
$	if e .nes. "-"
$	 then
$	  tmp = tmp + "_" + e
$	  goto us_loop
$	 endif
$	opt = f$edit(tmp,"upcase")
$	valu = f$edit(val,"upcase")
$
$	!! Process the option
$	if opt .nes. "WITH_X" .and. opt .nes. "WITH-X10" -
	   .and. opt .nes. "WITH_X11" then goto NOT_WITH_X
$	if f$extract(0,f$length(valu),"YES") .eqs. valu
$	 then
$	  val = "YES"
$	 else
$	  if f$extract(0,f$length(valu),"NO") .eqs. valu
$	   then
$	    val = "NO"
$	   else
$	    sayerr "''progname': the `--''opt'' option is supposed to have a boolean value."
$	    sayerr "Set it to either `yes' or `no'."
$	    gosub short_usage
$	    exit 0
$	   endif
$	 endif
$	'opt'=val
$	goto while
$      NOT_WITH_X:
$	if opt .nes. "WITH_X_TOOLKIT" then -
	goto NOT_WITH_X_TOOLKIT
$	if f$extract(0,f$length(valu),"YES") .eqs. valu
$	 then
$	  val = "athena"
$	 else
$	  if f$extract(0,f$length(valu),"NO") .eqs. valu
$	   then
$	    val = "no"
$	   else
$	    if f$extract(0,f$length(valu),"LUCID") .eqs. valu
$	     then
$	      val = "lucid"
$	     else
$	      if f$extract(0,f$length(valu),"ATHENA") .eqs. valu
$	       then
$	        val = "athena"
$	       else
$!		if f$extract(0,f$length(valu),"MOTIF") .eqs. valu
$!		 then
$!		  val = "motif"
$!		 else
$!		  if f$extract(0,f$length(valu),"OPEN-LOOK") .eqs. valu
$!		   then
$!		    val = "open-look"
$		   else
$!		    sayerr "''progname': the `--''opt'' option is supposed to have a value."
$!		    sayerr "which is `yes', `no', `lucid', `athena', `motif' or `open-look'.
$		    sayerr "''progname': the `--''opt'' option is supposed to have a value."
$		    sayerr "which is `yes', `no', `lucid' or `athena'.
$		    sayerr "Currently, `yes', `athena' and `lucid' are synonyms.'
$		    gosub short_usage
$		    exit 0
$!		   endif
$!		 endif
$	       endif
$	     endif
$	   endif
$	 endif
$	with_x_toolkit=val
$	goto while
$      NOT_WITH_X_TOOLKIT:
$	if opt .nes. "WITH_GCC" .and. opt .nes. "WITH_GNU_CC" then -
	goto NOT_WITH_GCC
$	if f$extract(0,f$length(valu),"YES") .eqs. valu
$	 then
$	  val = "YES"
$	 else
$	  if f$extract(0,f$length(valu),"NO") .eqs. valu
$	   then
$	    val = "NO"
$	   else
$	    sayerr "''progname': the `--''opt'' option is supposed to have a boolean value."
$	    sayerr "Set it to either `yes' or `no'."
$	    gosub short_usage
$	    exit 0
$	   endif
$	 endif
$	with_gcc=val
$	goto while
$      NOT_WITH_GCC:
$	if opt .nes. "WITH_NETLIB" then goto NOT_WITH_NETLIB
$	if f$extract(0,f$length(valu),"YES") .eqs. valu
$	 then
$	  val = "YES"
$	  with_ucx = "NO"
$	  with_multinet = "NO"
$	  with_tcpip = "YES"
$	 else
$	  if f$extract(0,f$length(valu),"NO") .eqs. valu
$	   then
$	    val = "NO"
$	    with_ucx = ""
$	    with_multinet = ""
$	    with_tcpip = "YES"
$	   else
$	    sayerr "''progname': the `--''opt'' option is supposed to have a boolean value."
$	    sayerr "Set it to either `yes' or `no'."
$	    gosub short_usage
$	    exit 0
$	   endif
$	 endif
$	'opt'=val
$	goto while
$      NOT_WITH_NETLIB:
$	if opt .nes. "WITH_UCX" then goto NOT_WITH_UCX
$	if f$extract(0,f$length(valu),"YES") .eqs. valu
$	 then
$	  val = "YES"
$	  with_netlib = "NO"
$	  with_multinet = "NO"
$	  with_tcpip = "YES"
$	 else
$	  if f$extract(0,f$length(valu),"NO") .eqs. valu
$	   then
$	    val = "NO"
$	    with_netlib = ""
$	    with_multinet = ""
$	    with_tcpip = "YES"
$	   else
$	    sayerr "''progname': the `--''opt'' option is supposed to have a boolean value."
$	    sayerr "Set it to either `yes' or `no'."
$	    gosub short_usage
$	    exit 0
$	   endif
$	 endif
$	'opt'=val
$	goto while
$      NOT_WITH_UCX:
$	if opt .nes. "WITH_MULTINET" then goto NOT_WITH_MULTINET
$	if f$extract(0,f$length(valu),"YES") .eqs. valu
$	 then
$	  val = "YES"
$	  with_netlib = "NO"
$	  with_ucx = "NO"
$	  with_tcpip = "YES"
$	 else
$	  if f$extract(0,f$length(valu),"NO") .eqs. valu
$	   then
$	    val = "NO"
$	    with_netlib = ""
$	    with_ucx = ""
$	    with_tcpip = "YES"
$	   else
$	    sayerr "''progname': the `--''opt'' option is supposed to have a boolean value."
$	    sayerr "Set it to either `yes' or `no'."
$	    gosub short_usage
$	    exit 0
$	   endif
$	 endif
$	'opt'=val
$	goto while
$      NOT_WITH_MULTINET:
$	if opt .nes. "WITH_TCPIP" then goto NOT_WITH_TCPIP
$	if f$extract(0,f$length(valu),"YES") .eqs. valu
$	 then
$	  val = "YES"
$	  with_netlib = ""
$	  with_ucx = ""
$	  with_multinet = ""
$	 else
$	  if f$extract(0,f$length(valu),"NO") .eqs. valu
$	   then
$	    val = "NO"
$	    with_netlib = "NO"
$	    with_ucx = "NO"
$	    with_multinet = "NO"
$	   else
$	    sayerr "''progname': the `--''opt'' option is supposed to have a boolean value."
$	    sayerr "Set it to either `yes' or `no'."
$	    gosub short_usage
$	    exit 0
$	   endif
$	 endif
$	'opt'=val
$	goto while
$      NOT_WITH_TCPIP:
$	if opt .nes. "WITH_DEBUG_HACK" then goto NOT_WITH_DEBUG_HACK
$	with_debug_hack = "YES"
$	goto while
$      NOT_WITH_DEBUG_HACK:
$	if opt .nes. "SRCDIR" then goto NOT_SRCDIR
$	if valomitted
$	 then
$	  i = index + 1
$	  if index .eq. 8 .or. (index .lt. 8 .and. p'i' .eqs. "")
$	   then
$	    sayerr "''progname': You must give a value for the `--''opt'' option, as in"
$	    sayerr "    `--''opt'=FOO'."
$	    gosub short_usage
$	    exit 0
$	   endif
$	  index = i
$	  val = current_arg
$	 endif
$	SRCDIR = val
$	goto while
$      NOT_SRCDIR:
$!	## Should this use the "development" file organization?
$	if opt .nes. "RUN_IN_PLACE" then goto NOT_RUN_IN_PLACE
$       single_tree="NO"
$       run_in_place="YES"
$	goto while
$      NOT_RUN_IN_PLACE:
$!	## Should this use the "single tree" file organization?
$       if opt .nes. "SINGLE_TREE" then goto NOT_SINGLE_TREE
$       single_tree="YES"
$       run_in_place="NO"
$	goto while
$      NOT_SINGLE_TREE:
$!	## Has the user specified one of the path_options?
$       if f$locate("| "+opt+" |", path_options) .ge. f$length(path_options) -
	then goto NO_PATH_OPTION
$       if valomitted
$	 then
$	  i = index + 1
$	  if index .eq. 8 .or. (index .lt. 8 .and. p'i' .eqs. "")
$	   then
$           sayerr "''progname': You must give a value for the `--''opt'' option, as in"
$           sayerr "    `--''opt'=FOO'."
$           gosub short_usage
$           exit 0
$	   endif
$	  index = i
$	  val = current_arg
$	 endif
$	if f$locate("PATH",opt) .ne. f$length(opt)
$	 then
$         'opt' = val
$         'opt'_specified = "YES"
$	 else
$         'opt'_dev = f$parse(val,,,"NODE")+f$parse(val,,,"DEVICE")
$         'opt'_dir = f$parse(val,,,"DIRECTORY")-"["-"<"-"]"-">"
$         'opt' = val
$         'opt'_specified = "YES"
$	 endif
$	goto while
$      NO_PATH_OPTION:
$	if opt .nes. "VERBOSE" then goto NOT_VERBOSE
$	VERBOSE = "YES"
$	goto while
$      NOT_VERBOSE:
$	if opt .eqs. "USAGE" .or. opt .eqs. "HELP"
$	 then
$	  gosub short_usage
$	  exit 1
$	 endif
$	if opt .nes. "FORCE" then goto NOT_FORCE
$	force = "YES"
$	goto while
$      NOT_FORCE:
$      else
$	!! Anything not starting with a hyphen we assume is a
$	!! configuration name.
$	!! This really is stupid... We know it is VMS, so we just cancel it.
$	configuration = current_arg
$      endif
$    endif
$   goto while
$  endif
$! done
$
$ if "''configuration'" .eqs. ""
$  then
$   ! Keep the historical Alpha machine settings for Alpha, IA64, and
$   ! unknown architectures.  X86_64 has a compatibility machine header
$   ! of its own so that the configuration name can identify the host.
$   arch_name = f$edit(f$getsyi("ARCH_NAME"),"UPCASE")
$   configuration = "alpha-dec-vms"
$   if arch_name .eqs. "VAX" then configuration = "vax-dec-vms"
$   if arch_name .eqs. "X86_64" then configuration = "x86_64-dec-vms"
$   configuration = configuration + f$extract(1,3,f$getsyi("version")) !! from V5.5-2 we have 5.5
$  endif
$ vms_version = f$extract(3,3,f$element(2,"-",configuration))
$ vms_version_major = f$extract(0,1,vms_version)
$ vms_version_minor = f$extract(2,1,vms_version)
$ vms_version_tmp = vms_version_major+"-"+vms_version_minor
$ configuration_us = configuration
$loop_configuration:
$ p = f$locate(".",configuration_us)
$ if p .nes. f$length(configuration_us)
$  then
$   configuration_us = -
	f$extract(0,p,configuration_us)+"_"+f$extract(p+1,999,configuration_us)
$   goto loop_configuration
$  endif
$ canonical = configuration
$ use_avms_h := no
$ if f$element(0,"-",configuration) .eqs. "alpha" -
     .and. f$extract(0,4,f$element(2,"-",configuration)) .eqs. "avms" then -
	use_avms_h := yes
$
$!#### Decide where the source is.
$ if "''SRCDIR'" .eqs. ""
$  then
$   !+
$   dirname=f$parse("A.;0",progname) - "A.;0"
$   dirname=dirname - ".][000000" - ".><000000" - ".>[000000" - ".]<000000"
$   dirname=dirname - "][" - "><" - ">[" - "]<"
$   if f$element(1,"<",dirname) .nes. "<" then -
	dirname = f$element(0,"<",dirname) + "[" + f$element(1,"<",dirname)
$   if f$element(1,">",dirname) .nes. ">" then -
	dirname = f$element(0,">",dirname) + "]" + f$element(1,">",dirname)
$   if dirname - "[000000." .nes. dirname then dirname = dirname - "000000."
$   !-
$   !! We know this file is in the root of the source
$   if f$search(dirname - "]" + ".SRC]LISP.H") .nes. "" -
	.and. f$search(dirname - "]" + ".LISP]VERSION.EL") .nes. ""
$    then
$     srcdir = dirname
$    else
$     if f$search("[.SRC]LISP.H") .nes. "" -
	.and. f$search("[.LISP]VERSION.EL") .nes. ""
$      then
$       srcdir = f$parse("A.;0") - "A.;0"
$      else
$	if f$search("[-.SRC]LISP.H") .nes. "" -
	.and. f$search("[-.LISP]VERSION.EL") .nes. ""
$	 then
$         srcdir = f$parse("[-]A.;0") - "A.;0"
$	 else
$	  sayerr "''progname': Neither the current directory nor its parent seem to"
$	  sayerr "contain the Emacs sources.  If you do not want to build Emacs in its"
$	  sayerr "source tree, you should run `''progname'' in the directory in which"
$	  sayerr "you wish to build Emacs, using its `--SRCDIR' option to say where the"
$	  sayerr "sources may be found."
$	  gosub short_usage
$	  exit 0
$	 endif
$      endif
$    endif
$  else
$   if f$search(srcdir - "]" + ".SRC]LISP.H") .eqs. "" -
	.or. f$search(srcdir - "]" + ".LISP]VERSION.EL") .eqs. ""
$    then
$     sayerr "''progname': The directory specified with the `--SRCDIR' option,"
$     sayerr "`''srcdir'', doesn't seem to contain the Emacs sources.  You should"
$     sayerr "either run the `''progname'' script at the top of the Emacs source"
$     sayerr "tree, or use the `--SRCDIR' option to specify where the Emacs sources"
$     sayerr "are."
$     gosub short_usage
$     exit 0
$    endif
$  endif
$!
$!#### Make srcdir absolute, if it isn't already.  It's important to
$!#### avoid running the path through pwd unnecessary, since pwd can
$!#### give you automounter prefixes, which can go away.
$ save_default = f$environment("DEFAULT")
$ set def 'srcdir'
$ srcdir = f$environment("DEFAULT")
$ set def 'save_default'
$!
$!#### Check if the source directory already has a configured system in it.
$ if f$search (srcdir - "]" + ".SRC]CONFIG.H") .nes. "" .and. .not. force
$  then
$   sayerr "''progname': WARNING: The directory tree `''srcdir'' is being used"
$   sayerr "   as a build directory right now; it has been configured in its own"
$   sayerr "   right.  To  configure in another directory as well, you MUST"
$   sayerr "   now do `make distclean' in ''srcdir',"
$   sayerr "   and then run ''progname' again."
$!#AC_INSTALL_extrasub(LINE_BEGIN+("VPATH"+(((span(" "+ASCII(9))|"")+"="))
$!#$   extrasub="/^VPATH[      ]*=/c\
$!#! vpath %.c $(srcdir)\
$!#! vpath %.h $(srcdir)\
$!#! vpath %.y $(srcdir)\
$!#! vpath %.l $(srcdir)\
$!#! vpath %.s $(srcdir)\
$!#! vpath %.in $(srcdir)'
$   exit 0
$  endif
$!
$!### Make the necessary directories, if they don't exist.
$ dirs = "[.src] [.lib-src] [.tradcpp] [.oldXmenu] [.lwlib] [.lisp] [.etc] [.vms]"
$ diri = 0
$dir_loop:
$ dir = f$element(diri," ",dirs)
$ diri = diri + 1
$ if dir .nes. " "
$  then
$   if f$parse(dir) .eqs. "" then -
      create/directory/version_limit=1/protection=(o:rwed) 'dir'
$   goto dir_loop
$  endif
$! done
$!
$!#### Make TRADCPP.EXE if necessary, and set the symbol tradcpp. (ttn)
$ if "" .eqs. f$search("[.tradcpp]tradcpp.exe")
$  then
$   if "" .eqs. f$search("[.tradcpp]tradcpp.c")
$    then
$     tradcppdir = srcdir - "]" + ".tradcpp]"
$     copy 'tradcppdir'*.* [.tradcpp]
$    endif
$   set def [.tradcpp]
$   mms
$   set def [-]
$  endif
$ tradcpp = f$environment("DEFAULT") - "]" + ".tradcpp]tradcpp.exe"
$ tradcpp := $'tradcpp'
$!
$!#### Given the configuration name, set machfile and opsysfile to the
$!#### names of the m/*.h and s/*.h files we should use.
$
$ machfile_dir = "M"
$ opsysfile_dir = "S"
$ mdir = srcdir  - "]" + ".SRC." + machfile_dir + "]"
$ sdir = srcdir  - "]" + ".SRC." + opsysfile_dir + "]"
$ !! Let's assume they come in rising version order...
$ max_tmp = ""
$ opsystmp = "vms"
$ if use_avms_h then opsystmp = "avms"
$conf_loop:
$ tmp = f$search("''sdir'''opsystmp'%*.H")
$ if tmp .nes. ""
$  then
$   tmp = f$parse(tmp,,,"NAME") - f$edit(opsystmp,"UPCASE")
$   test_major = f$element(0,"-",tmp)
$   test_minor = f$element(1,"-",tmp)
$   dummy := 'tmp		! tmp
$   dummy := 'vms_version_tmp	! vms_version_tmp
$   dummy := 'max_tmp		! max_tmp
$   dummy := 'test_major	! test_major
$   dummy := 'test_minor	! test_minor
$   if tmp .les. vms_version_tmp .and. tmp .ges. max_tmp
$    then
$     max_tmp = tmp
$     machine = f$element(0,"-",configuration)
$     opsys = "''opsystmp'''tmp'"
$     dummy := 'max_tmp		! max_tmp
$    endif
$   goto conf_loop
$  endif
$ say "Guessing the configuration to be ''configuration'"
$ machfile = "''machine'.h"
$ opsysfile = "''opsys'.h"
$!
$
$ ac_DCL_message = f$environment("MESSAGE")
$ ac_DCL_verify = f$verify(0)
$ ! 'f$verify(ac_DCL_verify)
$ ac_DCL_original_directory = f$trnlnm("SYS$DISK")+f$directory()
$ on warning then continue
$ on error then goto ac_bail_out
$ on severe_error then goto ac_bail_out
$ on control_y then goto ac_bail_out
$ goto ac_passed_error_routines
$ac_bail_out:
$ set default 'ac_DCL_original_directory'
$ set message 'ac_DCL_message'
$!#
$ goto after_VMS_DELR
$VMS_DELR: subroutine
$ on control_y then goto VMS_DELR_STOP
$ on warning then goto VMS_DELR_exit
$ _VMS_DELR_def = f$trnlnm("SYS$DISK")+f$directory()
$ if f$parse(p1) .eqs. "" then exit
$ set default 'f$parse(p1,,,"DEVICE")''f$parse(p1,,,"DIRECTORY")'
$VMS_DELR_loop:
$ _fp = f$parse(".DIR",p1)
$ _f = f$search(_fp)
$ if _f .eqs. "" then goto VMS_DELR_loopend
$ call VMS_DELR [.'f$parse(_f,,,"NAME")']*.*
$ goto VMS_DELR_loop
$VMS_DELR_loopend:
$ _fp = f$parse(p1,".;*")
$ if f$search(_fp) .eqs. "" then goto VMS_DELR_exit
$ set noon
$ set file/prot=(S:RWED,O:RWED,G:RWED,W:RWED) '_fp'
$ set on
$ delete/nolog '_fp'
$VMS_DELR_exit:
$ set default '_VMS_DELR_def'
$ exit
$VMS_DELR_STOP:
$ set default '_VMS_DELR_def'
$ stop/id=""
$ exit
$ endsubroutine
$
$after_VMS_DELR:
$!
$ call VMS_DELR "conftest*.*"
$
$ call VMS_DELR "confdefs*.*"
$
$ exit 1 + 0 * f$verify(ac_DCL_verify)
$ac_passed_error_routines:
$
$!# Save the original args if we used an alternate arg parser.
$ if f$type(configure_args) .nes. "" then ac_configure_args = configure_args
$ if f$type(configure_args) .eqs. "" then -
	ac_configure_args = p1+" "+p2+" "+p3+" "+p4+" "+p5+" "+p6+" "+p7+" "+p8
$!# Strip out --no-create and --norecursion so they don't pile up.
$ configure_args = configure_args-" --NO-CREATE"-"--NO-CREATE "-"--NO-CREATE"
$ configure_args = configure_args-" --NORECURSION"-"--NORECURSION "-"--NORECURSION"
$
$ goto ac_prepare_after_subs
$ac_prepare_compile_params:
$ __cflags = ""
$ if f$type(CFLAGS) .nes. "" then __cflags = CFLAGS
$ __defs = ""
$! more thuggery --ttn 2003-11-28
$! if f$type(DEFS) .nes. "" then __defs = "/defin=("+DEFS+")"
$ if f$type(ac_flags) .nes. "" then __cflags = __cflags + ac_flags
$ __incs = ""
$ if f$type(INCS) .nes. "" then __incs = "/include=("+INCS+")"
$ __compiler := cc
$ if f$type(ac_cc) .nes. "" then __compiler := 'ac_cc'
$ if f$type(CC) .nes. "" then __compiler := 'CC'
$ __preprocessor := cc '__cflags'/preprocess=
$ if f$type(ac_cpp) .nes. "" then __preprocessor := 'ac_cpp'
$ if f$type(CPP) .nes. "" then __preprocessor := 'CPP'
$ __extension := c
$ if f$type(ac_ext) .nes. "" then __extension = ac_ext
$ return
$ac_prepare_preprocess:
$ gosub ac_prepare_compile_params
$! '__preprocessor' CONFTEST.OUT '__defs' '__incs' conftest.'__extension'
$ '__preprocessor' CONFTEST.OUT conftest.'__extension'
$ ac_severity = $severity
$ return
$ac_prepare_compile:
$ gosub ac_prepare_compile_params
$ '__compiler' '__cflags' /object=conftest.obj '__defs' '__incs' conftest.'__extension'
$ ac_severity = $severity
$ return
$ac_prepare_link:
$ open/write link_opt conftest.opt
$ libi = 0
$ if f$type(LIBS) .eqs. "" then libs = ""
$ac_prepare_link_loop:
$ lib1 = f$element(libi," ",libs)
$ libi = libi + 1
$ if lib1 .nes. " "
$  then
$   if lib1 .nes. "" then write link_opt lib1
$   goto ac_prepare_link_loop
$  endif
$ close link_opt
$ libc = ","
$ if "''OPTS'" .eqs. "" then libc = ""
$ link 'ldflags' conftest.obj,conftest.opt/opt 'libc' 'OPTS'
$ ac_severity = $severity
$ return
$ac_prepare_after_subs:
$
$ ldflags := /nouserlibrary
$ ac_preprocess := gosub ac_prepare_preprocess
$ ac_compile := gosub ac_prepare_compile
$ ac_link := gosub ac_prepare_link
$
$ call VMS_DELR "conftest*.*"
$
$ if f$search("CONFDEFS.H.*") .nes. "" then delete/nolog confdefs.h.*
$
$ open/write foo confdefs.h
$ close foo
$
$!# A filename unique to this package, relative to the directory that
$!# configure is in, which we can look for to find out if srcdir is correct.
$ ac_unique_file:=lisp.dir
$
$!# Find the source files, if location was not specified.
$ if "''srcdir'" .nes. "" then goto ac_prepare_srcdir_try
$  ac_srcdirdefaulted := yes
$!# Try the directory containing this script, then `..'.
$  ac_prog=f$environment("PROCEDURE")
$  _dummy = "''prog'" ! This is prog, for debugging
$  ac_confdir=f$parse(ac_prog,,,"NODE") + f$parse(ac_prog,,,"DEVICE") + f$parse(ac_prog,,,"DIRECTORY")
$  _dummy = "''ac_confdir'" ! This is confdir, for debugging
$  if ac_confdir .eqs. f$environment("DEFAULT") then ac_confdir := []
$  srcdir=ac_confdir
$  if f$search(srcdir+ac_unique_file) .eqs. "" then srcdir := [-]
$ac_prepare_srcdir_try:
$ if f$search(srcdir+ac_unique_file) .nes. "" then goto ac_prepare_srcdir2
$ if "''ac_srcdirdefaulted'" then -
  $ write sys$error "configure: ","can not find sources in `''ac_confdir'' or `-'."
$ exit 2
$ if .not. "''ac_srcdirdefaulted'" then -
  $ write sys$error "configure: ","can not find sources in `''srcdir''."
$ exit 2
$ exit 0
$ac_prepare_srcdir2:
$!# The following are quite usefull
$ if srcdir .eqs. "[-]" .or. srcdir .eqs. "[]"
$  then
$   srcdir_dev:=
$   srcdir_dir = srcdir - "[" - "]"
$  else
$   srcdir_dir = f$parse(srcdir,,,"DIRECTORY") - "[" - "]" - "<" - ">"
$   srcdir_dev = f$parse(srcdir,,,"DEVICE")
$  endif
$ ac_cc := cc
$ if "''CC'" .nes. "" then ac_cc = CC
$ ac_ext:=c
$ ac_cpp := 'ac_cc' 'CFLAGS' /preprocess=
$ if "''CPP'" .nes. "" then ac_cpp = CPP
$
$
$ 
$
$
$!#### Choose a compiler.
$ cc_specified := NO
$ if f$type(CC) .nes. "" then cc_specified := YES
$
$ if "''WITH_GCC'" .eqs. "YES"
$  then
$   cc := gcc
$   use_gcc := yes
$  else
$   if "''WITH_GCC'" .eqs. "NO"
$    then
$     if f$type(CC) .eqs. ""
$      then
$	cc := cc
$      endif
$    endif
$  endif
 $ if "''CC'" .eqs. ""
$  then ! We are taking a risk here, because we are really trying to start
$	! the program. I hope we don't get caught by this.
$   set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   !# Gasp...
$   ac_dummy = "cc"
$ if .not. silent then write sys$output "checking ","for ''ac_dummy'"
$   ac_dummy
$   ac_status = $status
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$   
$   set on
$   if ac_status .ne. %X038090 ! This is the code for an unknown verb
$    then
$     CC = "cc"
$    endif
$  endif
$ if "''CC'" .eqs. "" then CC="gcc"
$ if "''CC'" .nes. "" then $ if verbose then write sys$output "	","setting CC to ''CC'"
$
$!# Find out if we are using GNU C, under whatever name.
$ _GNU_C := no
$ _DEC_C := no
$ _VAX_C := no
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ type sys$input /out=conftest.c
#ifdef __GNUC__
conftest___ _GNU_C:=yes
#endif
#ifdef __DECC
conftest___ _DEC_C:=yes
#else
#ifdef VAXC
conftest___ _VAX_C:=yes
#endif
#endif
$!#
$ if f$type(CFLAGS) .eqs. "" then CFLAGS = ""
$ ac_preprocess
$ search conftest.out "conftest___" /out=conftest.out-stripped/noheader
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ open/read ac_prog_cc_in conftest.out-stripped
$ac_prog_cc_read_loop:
$ read/end=ac_prog_cc_stop_reading ac_prog_cc_in foo
$ foo = f$extract(12,999,foo) ! Get rid of "conftest___ "
$ 'foo'
$ goto ac_prog_cc_read_loop
$ac_prog_cc_stop_reading:
$ close ac_prog_cc_in
$ if f$type(LIBS) .eqs. "" then LIBS = ""
$ if f$type(OPTS) .eqs. "" then OPTS = ""
$ sys_includes = ""
$ ac_GCC := no
$ if _GNU_C
$  then
$   ac_GCC := yes
$   CFLAGS := 'CFLAGS' /NOCASE_HACK
$   if OPTS .nes. ""
$    then OPTS = OPTS + ","
$    endif
$   OPTS := 'OPTS',GNU_CC:[000000]OPTIONS_SHR.OPT/opt
$! I'm currently not at all sure this is needed...
$   sys_includes := gnu_cc_include:,vaxc$include:,sys$library:
$  else
$   if _DEC_C
$    then
$     CFLAGS := 'CFLAGS'
$    else
$     if LIBS .nes. ""
$      then LIBS = LIBS + " "
$      endif
$     LIBS = LIBS + "SYS$SHARE:VAXCRTL/SHARE"
$     sys_includes := vaxc$include:,sys$library:
$    endif
$   if sys_includes .nes. "" then defi/nolog sys 'sys_includes'
$  endif
$ ac_cflags = CFLAGS
$ set noon
$ delete conftest*.*.*
$ 
$! ttn hack begin
$ if _DEC_C then CFLAGS := 'CFLAGS'/WARNINGS=(DISABLE=(UNDEFESCAP,PTRMISMATCH1,PTRMISMATCH,QUESTCOMPARE,QUESTCOMPARE1))
$! ttn hack end
$
$!#### Some systems specify a CPP to use unless we are using GCC.
$!#### Now that we know whether we are using GCC, we can decide whether
$!#### to use that one.
$ if .NOT. (f$type(NON_GNU_CPP) .eqs. "" .or "''ac_GCC'" .nes. "" -
	.or. f$type(CPP) .nes. "") then CPP = NON_GNU_CPP
$
$!#### Some systems specify a CC to use unless we are using GCC.
$!#### Now that we know whether we are using GCC, we can decide whether
$!#### to use that one.
$ if .NOT. (f$type(NON_GNU_CPP) .eqs. "" .or "''ac_GCC'" .nes. "" -
	.or. cc_specified) then CPP = NON_GNU_CC
$
$ if "''ac_GCC'" .nes. "" .and. f$type(GCC_TEST_OPTIONS) .nes. "" then -
	CC = CC + " " + GCC_TEST_OPTIONS
$
$ if "''ac_GCC'" .eqs. "" .and. f$type(NON_GCC_TEST_OPTIONS) .nes. "" then -
	CC = CC + " " + NON_GCC_TEST_OPTIONS
$
$!#### Some other nice autoconf tests.  If you add a test here which
$!#### should make an entry in src/config.h, don't forget to add an
$!#### #undef clause to src/config.h.in for autoconf to modify.
$
$ if .not. silent then write sys$output "checking ","for ln -s (Will use COPY instead of ln -s)"
$ LN_S:=copy
$
$ if .not. silent then write sys$output "checking ","how to run the C preprocessor"
$ if "''CPP'" .eqs. ""
$  then
$  CPP="''CC' ''CFLAGS'/PREPROCESS="
  $ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <stdio.h>"
$ write conftest_file "Syntax Error"
$ close conftest_file
$ set noon
$ def/user sys$output conftest.err
$ def/user sys$error conftest.err
$ 'ac_cpp' nla0: conftest.'ac_ext'
$ ac_severity = $severity
$ set on
$ ac_err := YES
$ if f$search("conftest.err") .nes. "" then -
	if f$file("conftest.err","EOF") .ne. 0 then -
	ac_err := NO
$! Alternate test for gcc
$ if ac_gcc .and. ac_severity .gt. 1 then ac_err := NO
$ if ac_err
$  then
$!
$  else
$ delete/nolog conftest*.*.*
$ write sys$error "What do I do now?"
$ exit 0
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$  endif
$ if verbose then write sys$output "	","setting CPP to ''CPP'"
$
$! A VMS hack
$ tmp_quotes = """"
$ tmp_quotes = tmp_quotes + tmp_quotes
$ tmp_quotes = tmp_quotes + tmp_quotes
$ tmp_quotes = tmp_quotes + tmp_quotes
$ tmp_quotes = tmp_quotes + tmp_quotes
$ tmp_quotes2 = tmp_quotes + tmp_quotes + tmp_quotes + tmp_quotes + tmp_quotes
$ tmp_quotes = tmp_quotes + tmp_quotes + tmp_quotes
$! tmp_quotes = tmp_quotes + tmp_quotes
$! tmp_quotes = tmp_quotes + tmp_quotes
$! tmp_quotes = tmp_quotes + tmp_quotes
$ INSTALL="@"+(srcdir - "]") + ".VMS]COPY "+tmp_quotes+"/PROT=(S:RWED,O:RWED,G:RE,W:RE)"+tmp_quotes
$ INSTALL_PROGRAM="@"+(srcdir - "]") + ".VMS]COPY "+tmp_quotes+"/PROT=(S:RWED,O:RWED,G:RE,W:RE)"+tmp_quotes
$ INSTALL_DATA="@"+(srcdir - "]") + ".VMS]COPY "+tmp_quotes+"/PROT=(S:RWED,O:RWED,G:RE,W:RE)"+tmp_quotes
$
$ INSTALL_QUOTED="@"+(srcdir - "]") + ".VMS]COPY "+tmp_quotes2+"/PROT=(S:RWED,O:RWED,G:RE,W:RE)"+tmp_quotes2
$ INSTALL_PROGRAM_QUOTED="@"+(srcdir - "]") + ".VMS]COPY "+tmp_quotes2+"/PROT=(S:RWED,O:RWED,G:RE,W:RE)"+tmp_quotes2
$ INSTALL_DATA_QUOTED="@"+(srcdir - "]") + ".VMS]COPY "+tmp_quotes2+"/PROT=(S:RWED,O:RWED,G:RE,W:RE)"+tmp_quotes2
$
$ if f$type(INSTALL) .eqs. "" THEN -
	INSTALL:=copy/prot=(s:rwed,o:rwed,g:re,w:re)
$ if verbose then write sys$output "	","setting INSTALL to ",INSTALL
$ if f$type(INSTALL_PROGRAM) .eqs. "" THEN -
	INSTALL_PROGRAM:=copy/prot=(s:rwed,o:rwed,g:re,w:re)
$ if verbose then write sys$output "	","setting INSTALL_PROGRAM to ",INSTALL_PROGRAM
$ if f$type(INSTALL_DATA) .eqs. "" THEN -
	INSTALL_DATA:=copy/prot=(s:rwed,o:rwed,g:re,w:re)
$ if verbose then write sys$output "	","setting INSTALL_DATA to ",INSTALL_DATA
$
$ if f$type(INSTALL_QUOTED) .eqs. "" THEN -
	INSTALL_QUOTED:=copy/prot=(s:rwed,o:rwed,g:re,w:re)
$ if verbose then write sys$output "	","setting INSTALL_QUOTED to ",INSTALL_QUOTED
$ if f$type(INSTALL_PROGRAM_QUOTED) .eqs. "" THEN -
	INSTALL_PROGRAM_QUOTED:=copy/prot=(s:rwed,o:rwed,g:re,w:re)
$ if verbose then write sys$output "	","setting INSTALL_PROGRAM_QUOTED to ",INSTALL_PROGRAM_QUOTED
$ if f$type(INSTALL_DATA_QUOTED) .eqs. "" THEN -
	INSTALL_DATA_QUOTED:=copy/prot=(s:rwed,o:rwed,g:re,w:re)
$ if verbose then write sys$output "	","setting INSTALL_DATA_QUOTED to ",INSTALL_DATA_QUOTED
$
$
$
$ ac_hh_l = "sys/wait.h sys/timeb.h sys/time.h fcntl.h stdlib.h string.h unistd.h"
$ ac_hh_i = 0
$ac_hh_VMS_LABLE0:
$ ac_hh_hdr_orig = f$element(ac_hh_i," ",ac_hh_l)
$ ac_hh_hdr = "HAVE_" + ac_hh_hdr_orig
$ ac_hh_i = ac_hh_i + 1
$ if ac_hh_hdr_orig .nes. " "
$  then
$   ac_hh_tr_hdr = ""
$   ac_hh_c1 = "."
$   ac_hh_c2 = "/"
$   ac_hh_c = 1
$   ac_hh_j = 0
$ac_hh_VMS_LABLE1:
$   ac_hh_e = f$element(ac_hh_j,ac_hh_c'ac_hh_c',ac_hh_hdr)
$   ac_hh_j = ac_hh_j + 1
$   if ac_hh_e .eqs. ac_hh_c'ac_hh_c'
$    then
$     if ac_hh_c .eq. 1
$      then
$       ac_hh_c = 2
$       ac_hh_hdr = ac_hh_tr_hdr
$       ac_hh_tr_hdr = ""
$       ac_hh_j = 0
$       goto ac_hh_VMS_LABLE1
$      endif
$    else
$     if ac_hh_tr_hdr .nes. "" then -
	ac_hh_tr_hdr = ac_hh_tr_hdr + "_"
$     ac_hh_tr_hdr = ac_hh_tr_hdr + f$edit(ac_hh_e,"UPCASE")
$     goto ac_hh_VMS_LABLE1
$    endif
$ if .not. silent then write sys$output "checking ","for ''ac_hh_hdr_orig'"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <''ac_hh_hdr_orig'>"
$ write conftest_file ""
$ close conftest_file
$ set noon
$ def/user sys$output conftest.err
$ def/user sys$error conftest.err
$ 'ac_cpp' nla0: conftest.'ac_ext'
$ ac_severity = $severity
$ set on
$ ac_err := YES
$ if f$search("conftest.err") .nes. "" then -
	if f$file("conftest.err","EOF") .ne. 0 then -
	ac_err := NO
$! Alternate test for gcc
$ if ac_gcc .and. ac_severity .gt. 1 then ac_err := NO
$ if ac_err
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_'ac_hh_tr_hdr' = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining ''ac_hh_tr_hdr'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define ''ac_hh_tr_hdr' 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """''ac_hh_tr_hdr'""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""''ac_hh_tr_hdr'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""''ac_hh_tr_hdr'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""''ac_hh_tr_hdr'"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$   goto ac_hh_VMS_LABLE0
$  endif
$
$ ac_hh_l = "utime.h"
$ ac_hh_i = 0
$ac_hh_VMS_LABLE2:
$ ac_hh_hdr_orig = f$element(ac_hh_i," ",ac_hh_l)
$ ac_hh_hdr = "HAVE_" + ac_hh_hdr_orig
$ ac_hh_i = ac_hh_i + 1
$ if ac_hh_hdr_orig .nes. " "
$  then
$   ac_hh_tr_hdr = ""
$   ac_hh_c1 = "."
$   ac_hh_c2 = "/"
$   ac_hh_c = 1
$   ac_hh_j = 0
$ac_hh_VMS_LABLE3:
$   ac_hh_e = f$element(ac_hh_j,ac_hh_c'ac_hh_c',ac_hh_hdr)
$   ac_hh_j = ac_hh_j + 1
$   if ac_hh_e .eqs. ac_hh_c'ac_hh_c'
$    then
$     if ac_hh_c .eq. 1
$      then
$       ac_hh_c = 2
$       ac_hh_hdr = ac_hh_tr_hdr
$       ac_hh_tr_hdr = ""
$       ac_hh_j = 0
$       goto ac_hh_VMS_LABLE3
$      endif
$    else
$     if ac_hh_tr_hdr .nes. "" then -
	ac_hh_tr_hdr = ac_hh_tr_hdr + "_"
$     ac_hh_tr_hdr = ac_hh_tr_hdr + f$edit(ac_hh_e,"UPCASE")
$     goto ac_hh_VMS_LABLE3
$    endif
$ if .not. silent then write sys$output "checking ","for ''ac_hh_hdr_orig'"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <''ac_hh_hdr_orig'>"
$ write conftest_file ""
$ close conftest_file
$ set noon
$ def/user sys$output conftest.err
$ def/user sys$error conftest.err
$ 'ac_cpp' nla0: conftest.'ac_ext'
$ ac_severity = $severity
$ set on
$ ac_err := YES
$ if f$search("conftest.err") .nes. "" then -
	if f$file("conftest.err","EOF") .ne. 0 then -
	ac_err := NO
$! Alternate test for gcc
$ if ac_gcc .and. ac_severity .gt. 1 then ac_err := NO
$ if ac_err
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_'ac_hh_tr_hdr' = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining ''ac_hh_tr_hdr'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define ''ac_hh_tr_hdr' 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """''ac_hh_tr_hdr'""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""''ac_hh_tr_hdr'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""''ac_hh_tr_hdr'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""''ac_hh_tr_hdr'"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$   goto ac_hh_VMS_LABLE2
$  endif
$
$ if .not. silent then write sys$output "checking ","for ANSI C header files"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <stdlib.h>"
$ write conftest_file "#include <stdarg.h>"
$ write conftest_file "#include <string.h>"
$ write conftest_file "#include <float.h>"
$ close conftest_file
$ set noon
$ def/user sys$output conftest.err
$ def/user sys$error conftest.err
$ 'ac_cpp' nla0: conftest.'ac_ext'
$ ac_severity = $severity
$ set on
$ ac_err := YES
$ if f$search("conftest.err") .nes. "" then -
	if f$file("conftest.err","EOF") .ne. 0 then -
	ac_err := NO
$! Alternate test for gcc
$ if ac_gcc .and. ac_severity .gt. 1 then ac_err := NO
$ if ac_err
$  then
$ delete/nolog conftest*.*.*
$!# SunOS 4.x string.h does not declare mem*, contrary to ANSI.
$ open/write conftest_FILE conftest.'ac_ext'
$ write conftest_FILE "#include ""confdefs.h"""
$ write conftest_FILE "#include <string.h>"
$ close conftest_FILE
$ set noon
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ ac_preprocess
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ search/out=nla0: conftest.out "memchr"
$ ac_severity = $severity
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ set on
$ if ac_severity .eq. 1
$  then
$ delete/nolog conftest*.*.*
$!# SGI's /bin/cc from Irix-4.0.5 gets non-ANSI ctype macros unless using -ansi.
$   open/write conftest_file conftest.'ac_ext'
$   write conftest_file "#include ""confdefs.h"""
$   write conftest_file "#include <ctype.h>"
$   write conftest_file "#define ISLOWER(c) ('a' <= (c) && (c) <= 'z')"
$   write conftest_file "#define TOUPPER(c) (ISLOWER(c) ? 'A' + ((c) - 'a') : (c))"
$   write conftest_file "#define XOR(e,f) (((e) && !(f)) || (!(e) && (f)))"
$   write conftest_file "void exit();"
$   write conftest_file "int main () { int i; for (i = 0; i < 256; i++)"
$   write conftest_file "if (XOR (islower (i), ISLOWER (i)) || toupper (i) != TOUPPER (i)) exit(2);"
$   write conftest_file "exit (0); }"
$   write conftest_file ""
$   close conftest_file
$   ac_file_name := conftest.'ac_ext'
$   ac_default_name := conftest.'ac_ext'
$   if ac_file_name .nes. ac_default_name then copy 'ac_file_name 'ac_default_name
$   set noon
$   _tmp = -1 ! Because DCL can't jump into the middle of an IF stmt
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if f$search("conftest.obj") .eqs. "" then goto ac_tp_VMS_LABLE0
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .ne. 1 then goto ac_tp_VMS_LABLE0
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   run conftest
$   ac_status = $status
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   set on
$ ac_tp_VMS_LABLE0:
$   if ac_status .eq. 1 .or. ac_status .eq. 0 ! UNIXoid programs return 0 for success
$    then
$!# ISC 2.0.2 stdlib.h does not declare free, contrary to ANSI.
$ open/write conftest_FILE conftest.'ac_ext'
$ write conftest_FILE "#include ""confdefs.h"""
$ write conftest_FILE "#include <stdlib.h>"
$ close conftest_FILE
$ set noon
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ ac_preprocess
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ search/out=nla0: conftest.out "free"
$ ac_severity = $severity
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ set on
$ if ac_severity .eq. 1
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_STDC_HEADERS = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining STDC_HEADERS"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define STDC_HEADERS 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """STDC_HEADERS""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""STDC_HEADERS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""STDC_HEADERS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""STDC_HEADERS"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$  else
$!
$    endif
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$ if .not. silent then write sys$output "checking ","for whether time.h and sys/time.h may both be included"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <sys/types.h>"
$ write conftest_file "#include <sys/time.h>"
$ write conftest_file "#include <time.h>"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { struct tm *tp;; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_TIME_WITH_SYS_TIME = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining TIME_WITH_SYS_TIME"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define TIME_WITH_SYS_TIME 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """TIME_WITH_SYS_TIME""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""TIME_WITH_SYS_TIME"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""TIME_WITH_SYS_TIME"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""TIME_WITH_SYS_TIME"",""1"");"
$
$ tm_in_sys_time="YES"
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$ if .not. silent then write sys$output "checking ","for sys_siglist declaration in signal.h or unistd.h"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <signal.h>"
$ write conftest_file "/* NetBSD declares sys_siglist in <unistd.h>.  */"
$ write conftest_file "#ifdef HAVE_UNISTD_H"
$ write conftest_file "#include <unistd.h>"
$ write conftest_file "#endif"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { char *msg = *(sys_siglist + 1);; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_SYS_SIGLIST_DECLARED = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining SYS_SIGLIST_DECLARED"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define SYS_SIGLIST_DECLARED 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """SYS_SIGLIST_DECLARED""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""SYS_SIGLIST_DECLARED"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""SYS_SIGLIST_DECLARED"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""SYS_SIGLIST_DECLARED"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$ if .not. silent then write sys$output "checking ","for return type of signal handlers"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <sys/types.h>"
$ write conftest_file "#include <signal.h>"
$ write conftest_file "#ifdef signal"
$ write conftest_file "#undef signal"
$ write conftest_file "#endif"
$ write conftest_file "extern void (*signal ()) ();"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { int i;; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_RETSIGTYPE = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining RETSIGTYPE to be void"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define RETSIGTYPE void"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """RETSIGTYPE""=""void"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""RETSIGTYPE"",""void"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""RETSIGTYPE"",""void"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""RETSIGTYPE"",""void"");"
$
$  else
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_RETSIGTYPE = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining RETSIGTYPE to be int"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define RETSIGTYPE int"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """RETSIGTYPE""=""int"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""RETSIGTYPE"",""int"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""RETSIGTYPE"",""int"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""RETSIGTYPE"",""int"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$ if .not. silent then write sys$output "checking ","for struct tm in time.h"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <sys/types.h>"
$ write conftest_file "#include <time.h>"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { struct tm *tp; tp->tm_sec;; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$!
$  else
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_TM_IN_SYS_TIME = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining TM_IN_SYS_TIME"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define TM_IN_SYS_TIME 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """TM_IN_SYS_TIME""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""TM_IN_SYS_TIME"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""TM_IN_SYS_TIME"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""TM_IN_SYS_TIME"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$ if "''tm_in_sys_time'"
$  then ac_decl = "sys/time.h"
$  else ac_decl = "time.h"
$  endif
$ if .not. silent then write sys$output "checking ","for tm_zone in struct tm"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <sys/types.h>"
$ write conftest_file "#include <''ac_decl'>"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { struct tm tm; tm.tm_zone;; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_TM_ZONE = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_TM_ZONE"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_TM_ZONE 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_TM_ZONE""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_TM_ZONE"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_TM_ZONE"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_TM_ZONE"",""1"");"
$
$  else
$ delete/nolog conftest*.*.*
$ ac_no_tm_zone=1
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$ if "''ac_no_tm_zone'" .nes. ""
$  then
$ if .not. silent then write sys$output "checking ","for tzname"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <time.h>"
$ write conftest_file "#ifndef tzname /* For SGI.  */"
$ write conftest_file "extern char *tzname[]; /* RS6000 and others want it this way.  */"
$ write conftest_file "#endif"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { atoi(*tzname);; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_TZNAME = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_TZNAME"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_TZNAME 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_TZNAME""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_TZNAME"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_TZNAME"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_TZNAME"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$ endif
$
$
$ if .not. silent then write sys$output "checking ","for lack of working const"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { /* Ultrix mips cc rejects this.  */"
$ write conftest_file "typedef int charset[2]; const charset x;"
$ write conftest_file "/* SunOS 4.1.1 cc rejects this.  */"
$ write conftest_file "char const *const *ccp;"
$ write conftest_file "char **p;"
$ write conftest_file "/* AIX XL C 1.02.0.0 rejects this."
$ write conftest_file "   It does not let you subtract one const X* pointer from another in an arm"
$ write conftest_file "   of an if-expression whose if-part is not a constant expression */"
$ write conftest_file "const char *g = ""string"";"
$ write conftest_file "ccp = &g + (g ? g-g : 0);"
$ write conftest_file "/* HPUX 7.0 cc rejects these. */"
$ write conftest_file "++ccp;"
$ write conftest_file "p = (char**) ccp;"
$ write conftest_file "ccp = (char const *const *) p;"
$ write conftest_file "{ /* SCO 3.2v4 cc rejects this.  */"
$ write conftest_file "  char *t;"
$ write conftest_file "  char const *s = 0 ? (char *) 0 : (char const *) 0;"
$ write conftest_file ""
$ write conftest_file "  *t++ = 0;"
$ write conftest_file "}"
$ write conftest_file "{ /* Someone thinks the Sun supposedly-ANSI compiler will reject this.  */"
$ write conftest_file "  int x[] = {25,17};"
$ write conftest_file "  const int *foo = &x[0];"
$ write conftest_file "  ++foo;"
$ write conftest_file "}"
$ write conftest_file "{ /* Sun SC1.0 ANSI compiler rejects this -- but not the above. */"
$ write conftest_file "  typedef const int *iptr;"
$ write conftest_file "  iptr p = 0;"
$ write conftest_file "  ++p;"
$ write conftest_file "}"
$ write conftest_file "{ /* AIX XL C 1.02.0.0 rejects this saying"
$ write conftest_file "     ""k.c"", line 2.27: 1506-025 (S) Operand must be a modifiable lvalue. */"
$ write conftest_file "  struct s { int j; const int *ap[3]; };"
$ write conftest_file "  struct s *b; b->j = 5;"
$ write conftest_file "}"
$ write conftest_file "{ /* ULTRIX-32 V3.1 (Rev 9) vcc rejects this */"
$ write conftest_file "  const int foo = 10;"
$ write conftest_file "}"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$!
$  else
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_const = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining const to be empty"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define const "
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """const""="""""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""const"","""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""const"","""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""const"","""");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$ type sys$input: /out=conftestmake.
all :
	@ write sys$output "$ ac_maketemp:==$(MMS)$(MAKE)"
$
$!set ver
$! MMK is a free MMS clone.  GMAKE is GNU make.
$!TTNHACK: jam preference for mms for now (2003-03-10)
$! ac_makes := mmk,gmake,mms
$ ac_makes := mms,mmk,gmake,mms
$ ac_makes_i = 0
$ SET_MAKE :=
$!$ if f$type(MAKE) .eqs. "" then goto ac_set_make_loop
$!$ if MAKE .nes. "" .and. f$edit(MAKE,"UPCASE") .nes. "$TECO MAKE" then -
$!	ac_makes_i = 999
$!$ if MAKE .nes. "" .and. f$edit(MAKE,"UPCASE") .nes. "$TECO MAKE" then -
$!	SET_MAKE = MAKE
$ac_set_make_loop:
$ ac_make = f$element(ac_makes_i, ",", ac_makes)
$ if ac_make .eqs. "," then goto ac_set_make_loop_end
$ ac_very_dummy := 'ac_make
$ ac_very_dummy := 'ac_makes_i
$ ac_makes_i = ac_makes_i + 1
$ SET_MAKE = ac_make
$ if f$type('SET_MAKE') .nes. "" then SET_MAKE = 'SET_MAKE'
$!# GNU make sometimes prints "make[1]: Entering...", which would confuse us.
$!$ set ver
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$error nl:
$ SET_MAKE -f conftestmake.
$ ac_severity = $severity
$ if f$trnlnm("sys$output",,,,,"ACCESS_MODE") .eqs. "USER" then -
	deassign/user sys$output
$
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if ac_severity .ne. 1
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$error nl:
$   SET_MAKE /description=conftestmake./output=conftest.out
$   ac_severity = $severity
$!$   set nover
$
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$  endif
$!$ set nover
$ if ac_severity .ne. 1 then goto ac_set_make_loop
$ SET_MAKE = ac_make
$ac_set_make_loop_end:
$ if SET_MAKE .nes. "," then SET_MAKE="MAKE="+SET_MAKE
$ if f$edit(SET_MAKE,"UPCASE") .eqs. "MAKE=MMS" then -
	SET_MAKE = SET_MAKE + "/IGNORE=WARNING"
$ ac_very_dummy := 'SET_MAKE
$ if f$search("conftestmake.*") .nes. "" then delete/nolog conftestmake.*;*
$ if f$search("conftest.out") .nes. "" then delete/nolog conftest.out;*
$!set nover
$
$
$ if .not. silent then write sys$output "checking ","for long file names (vms has that!)"
$!#
$ _ac_is_defined_HAVE_LONG_FILE_NAMES = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_LONG_FILE_NAMES"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_LONG_FILE_NAMES 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_LONG_FILE_NAMES""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_LONG_FILE_NAMES"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_LONG_FILE_NAMES"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_LONG_FILE_NAMES"",""1"");"
$
$
$
$
$
$
$!#### Choose a TCP/IP package.
$ say "Checking TCP/IP package."
$ tcpip_package = ""
$ if "''with_ucx'" .eqs. "NO" -
	.and. "''with_netlib'" .eqs. "NO"-
	.and. "''with_multinet'" .eqs. "NO" then -
	with_tcpip = "NO"
$ if "''with_tcpip'" .eqs. "YES"
$  then
$   if "''with_netlib'" .eqs. "YES"
$    then tcpip_package = "NETLIB"
$    else if "''with_ucx'" .eqs. "YES"
$     then tcpip_package = "UCX"
$     else if "''with_multinet'" .eqs. "YES"
$      then tcpip_package = "MultiNet"
$      else
$       say "  No TCP/IP package specified.  Looking for TCP/IP package."
$	tcpip_package = ""
$ if .not. silent then write sys$output "checking ","for socketshr.h"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <socketshr.h>"
$ write conftest_file ""
$ close conftest_file
$ set noon
$ def/user sys$output conftest.err
$ def/user sys$error conftest.err
$ 'ac_cpp' nla0: conftest.'ac_ext'
$ ac_severity = $severity
$ set on
$ ac_err := YES
$ if f$search("conftest.err") .nes. "" then -
	if f$file("conftest.err","EOF") .ne. 0 then -
	ac_err := NO
$! Alternate test for gcc
$ if ac_gcc .and. ac_severity .gt. 1 then ac_err := NO
$ if ac_err
$  then
$ delete/nolog conftest*.*.*
$ tcpip_package = "SOCKETSHR"
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$	if tcpip_package .eqs. "SOCKETSHR" -
	.and. f$search(f$parse("NETLIB_SHR","SYS$SHARE:.EXE")) .nes. "" -
	.and. f$search(f$parse("SOCKETSHR","SYS$SHARE:.EXE")) .nes. ""
$	 then tcpip_package = "NETLIB"
$	 else
$	  tcpip_package = ""
$         if f$search("UCX$LIBRARY:TCPIP$LIB.OLB") .nes. "" -
	  .or. f$search("SYS$LIBRARY:UCX$IPC.OLB") .nes. "" -
	  .or. f$search("SYS$SHARE:UCX$IPC_SHR.EXE") .nes. ""
$          then tcpip_package = "UCX"
$          else
$           if f$trnlnm("multinet") .nes. ""
$            then if f$search("MULTINET:MULTINET_SOCKET_LIBRARY.EXE") .nes. ""
$             then tcpip_package = "MultiNet"
$             endif
$            endif
$          endif
$        endif
$      endif
$     endif
$    endif
$  endif
$ if tcpip_package .nes. ""
$  then
$   if tcpip_package .eqs. "UCX"
$    then
$     netinet_includes := vaxc$include:,sys$library:
$     arpa_includes := vaxc$include:,sys$library:
$    else
$     if tcpip_package .eqs. "NETLIB"
$      then
$       netinet_includes := vaxc$include:,sys$library:
$       arpa_includes := vaxc$include:,sys$library:
$      else
$       if tcpip_package .eqs. "MultiNet"
$        then
$         a = f$trnlnm("multinet_root") - ".]"
$         if "''a'" .eqs. ""
$         then
$          sayerr "Logical MULTINET_ROOT not defined."
$          exit 0
$         endif
$         netinet_includes := 'a'.netinet]
$         arpa_includes := 'a'.arpa]
$        endif
$      endif
$    endif
$   defi netinet 'netinet_includes'
$   defi arpa 'arpa_includes'
$   have_sockets = "YES"
$   say "  Using ''tcpip_package'."
$  else
$   netinet_includes = ""
$   arpa_includes = ""
$   have_sockets = "NO"
$   say "  Using no TCP/IP package."
$  endif
$ 
$!#### Choose a window system.
$ say "checking for specified window system."
$
$ window_system=""
$ if "''with_x'" .eqs. "YES"
$  then window_system = "x11"
$  else if "''with_x'" .eqs. "NO" then window_system = "none"
$  endif
$ if "''with_x11'" .eqs. "YES"
$  then window_system = "x11"
$  else if "''with_x11'" .eqs. "NO" then window_system = "none"
$  endif
$ if "''with_x10'" .eqs. "YES"
$  then
$   say "  There is no such thing as X10 on !"
$   window_system = "none"
$  else if "''with_x10'" .eqs. "NO" then window_system = ""
$  endif
$ if window_system .eqs. ""
$  then
$   say "  No window system specified.  Looking for X11."
$   if f$trnlnm("DECW$INCLUDE") .nes. "" then window_system = "x11"
$  endif
$ if window_system .eqs. "x11" .or. window_system .eqs. ""
$  then
$   if f$type(x_includes) .eqs. "" .and. f$type(x_libraries) .eqs. ""
$    then
$!# If we find X, set shell vars x_includes and x_libraries to the paths.
$ if .not. silent then write sys$output "checking ","for X include and library files"
$ if f$trnlnm("DECW$INCLUDE") .nes. ""
$  then
$   x_includes := DECW$INCLUDE:
$ if verbose then write sys$output "	","X11 headers are in ''x_includes'"
$  endif
$ if f$trnlnm("DECW$INCLUDE") .nes. ""
$  then
$   x_libraries := SYS$LIBRARY:
$ if verbose then write sys$output "	","X11 libraries are in ''x_libraries'"
$  endif
$ defi/nolog x11 'x_includes'
$
$    endif
$   if f$type(x_includes) .nes. "" .or. f$type(x_libraries) .nes. "" then window_system = "x11"
$  endif
$
$ if window_system .eqs. "" then window_system = "none"
$
$ if window_system .eqs. "x11"
$  then
$   have_x_windows = "YES"
$   have_x11 = "YES"
$   say "  Using X11."
$   USE_X_TOOLKIT:=
$   if "''with_x_toolkit'" .eqs. "athena" -
	.or. "''with_x_toolkit'" .eqs. "lucid"
$    then
$     USE_X_TOOLKIT:=LUCID
$     say "  Using Xt toolkit."
$    endif
$   if "''with_x_toolkit'" .eqs. "motif"
$    then
$     USE_X_TOOLKIT:=MOTIF
$     say "  Using Motif toolkit."
$    endif
$   if "''with_x_toolkit'" .eqs. "open-look"
$    then
$     USE_X_TOOLKIT:=OPEN_LOOK
$     say "  Using Open-Look toolkit."
$    endif
$   if USE_X_TOOLKIT .eqs. ""
$    then
$     USE_X_TOOLKIT:=none
$     say "  Using Xlib directly."
$    endif
$  else
$   have_x_windows = "NO"
$   have_x11 = "NO"
$   USE_X_TOOLKIT:=none
$   say "  Using no window system."
$  endif
$ X_TOOLKIT_TYPE=USE_X_TOOLKIT
$
$!### If we're using X11, we should use the X menu package.
$ have_menus = have_x11
$
$!#### Extract some information from the operating system and machine files.
$
$ say "examining the machine- and system-dependent files to find out"
$ say " - which libraries the lib-src programs will want, and"
$ say " - whether the GNU malloc routines are usable"
$
$!### First figure out CFLAGS (which we use for running the compiler here)
$!### and REAL_CFLAGS (which we use for real compilation).
$!### The two are the same except on a few systems, where they are made
$!### different to work around various lossages.  For example,
$!### GCC 2.5 on Linux needs them to be different because it treats -g
$!### as implying static linking.
$
$!### If the CFLAGS env var is specified, we use that value
$!### instead of the default.
$
$!### It's not important that this name contain the PID; you can't run
$!### two configures in the same directory and have anything work
$!### anyway.
$ tempcname="configure-tmp.c" ! Named this way so configure.com* don't get erased
$ open /write out 'tempcname'
$ sout := write out
$ if "''tcpip_package'" .nes. "" then -
	sout "#define ",f$edit(tcpip_package, "UPCASE")
$ sout "#include ""''srcdir_dev'[''srcdir_dir'.src.''opsysfile_dir']''opsysfile'"""
$ sout "#include ""''srcdir_dev'[''srcdir_dir'.src.''machfile_dir']''machfile'"""
$ type sys$input/output=out !This is a piece of dark DCL magic
#ifndef C_SWITCH_X_MACHINE
#define C_SWITCH_X_MACHINE
#endif
configure___ c_switch_x_machine:=C_SWITCH_X_MACHINE
#ifndef C_SWITCH_X_SYSTEM
#define C_SWITCH_X_SYSTEM
#endif
configure___ c_switch_x_system:=C_SWITCH_X_SYSTEM
#ifndef C_SWITCH_SITE
#define C_SWITCH_SITE
#endif
configure___ c_switch_site:=C_SWITCH_SITE
#ifndef C_SWITCH_X_SITE
#define C_SWITCH_X_SITE
#endif
configure___ c_switch_x_site:=C_SWITCH_X_SITE
/* End of  addition */
#ifndef LIBS_MACHINE
#define LIBS_MACHINE
#endif
#ifndef LIBS_SYSTEM
#define LIBS_SYSTEM
#endif
#ifndef C_SWITCH_SYSTEM
#define C_SWITCH_SYSTEM
#endif
#ifndef C_SWITCH_MACHINE
#define C_SWITCH_MACHINE
#endif
configure___ libsrc_libs:=LIBS_MACHINE LIBS_SYSTEM
configure___ c_switch_system:=C_SWITCH_SYSTEM
configure___ c_switch_machine:=C_SWITCH_MACHINE

#ifndef LIB_X11_LIB
#define LIB_X11_LIB sys$share:decw$xlibshr/share
#endif

#ifndef LIBX11_MACHINE
#define LIBX11_MACHINE
#endif

#ifndef LIBX11_SYSTEM
#define LIBX11_SYSTEM
#endif
configure___ LIBX:=LIB_X11_LIB LIBX11_MACHINE LIBX11_SYSTEM

#ifdef UNEXEC
configure___ unexec:=UNEXEC
#else
configure___ unexec:=,vmsmap.obj
#endif

#ifdef SYSTEM_MALLOC
configure___ system_malloc:=yes
#else
configure___ system_malloc:=no
#endif

#ifndef C_DEBUG_SWITCH
#define C_DEBUG_SWITCH /debug
#endif
configure___ c_debug_switch:=C_DEBUG_SWITCH

#ifndef C_OPTIMIZE_SWITCH
#define C_OPTIMIZE_SWITCH 
#endif
configure___ c_optimize_switch:=C_OPTIMIZE_SWITCH

configure___ __tmp2:=
configure___ if f$type(CFLAGS) .nes. "" then __tmp2 = CFLAGS
#ifdef __GNUC__
configure___ __tmp1:=C_DEBUG_SWITCH C_OPTIMIZE_SWITCH
#else
configure___ __tmp1:=C_DEBUG_SWITCH
#endif
configure___ __tmp=__tmp1+" "+__tmp2

#ifdef THIS_IS_CONFIGURE

/* Get the CFLAGS for tests in configure.  */
configure___ CFLAGS = __tmp

#else /* not THIS_IS_CONFIGURE */

/* Get the CFLAGS for real compilation.  */
configure___ REAL_CFLAGS = __tmp

#endif /* not THIS_IS_CONFIGURE */
$ close out
$
$ set noon
$ __tmp:=
$ if f$type(CFLAGS) .nes. "" then __tmp = CFLAGS
$ __compiler := cc
$ if f$type(ac_cc) .nes. "" then __compiler := 'ac_cc'
$ if f$type(CC) .nes. "" then __compiler := 'CC'
$!
$! don't use this
$!
$! '__compiler' '__tmp'/preprocess='tempcname'-i 'tempcname'
$!
$! use this instead
$!
$! the vms version and architecture stuff needs some cleanup, I don't want
$! to go there: I add some more bags, because now this may run on IA64 cpus.
$ if f$getsyi("HW_MODEL").eq. 4096
$  then
$   tradcpp "-DVMS" "-D__ia64" 'tempcname' 'tempcname'-i
$  else
$   tradcpp "-DVMS" 'tempcname' 'tempcname'-i
$ endif
$!
$ search/nohead/output='tempcname'-stripped 'tempcname'-i "configure___"
$ open/read in 'tempcname'-stripped
$read_loop:
$ read/end=stop_reading in foo
$ foo = f$extract(12,999,foo) ! Get rid of "configure___"
$ 'foo'
$ goto read_loop
$stop_reading:
$ close in
$ delete/nolog 'tempcname'*.*
$ if f$type(CFLAGS) .eqs. ""
$  then
$   '__compiler' /preprocess='tempcname'-i /include=('mdir','sdir') -
	'tempcname' /define=THIS_IS_CONFIGURE
$   search/nohead/output='tempcname'-stripped 'tempcname'-i "configure___"
$   open/read in 'tempcname'-stripped
$  read_loop2:
$   read/end=stop_reading2 in foo
$   foo = f$extract(12,999,foo) ! Get rid of "configure___"
$   'foo'
$   goto read_loop2
$  stop_reading2:
$   close in
$   delete/nolog 'tempcname'*.*
$  else
$   REAL_CFLAGS = CFLAGS
$  endif
$ set on
$
$!### Compute the unexec souce name from the object name.
$ UNEXEC_SRC = "$(srcdir)" + f$edit(unexec-",","UPCASE,TRIM") - ".OBJ" + ".C"
$
$!# Do the opsystem or machine files prohibit the use of the GNU malloc?
$!# Assume not, until told otherwise.
$ GNU_MALLOC="YES"
$ if "''system_malloc'" .eqs. ""
$  then
$   gnu_malloc = "NO"
$   gnu_malloc_reason = -
"(The GNU allocators don't work with this system configuration.)"
$  endif
$! if "''rel_alloc'" .eqs. "" then rel_alloc = gnu_malloc
$ rel_alloc = "NO" ! it doesn't work at the moment, and won't probably ever
$ lisp_float_type = "YES"
$
$!#### Add the libraries to LIBS and check for some functions.
$
$
$ ac_flags=C_SWITCH_SYSTEM + " " + C_SWITCH_MACHINE
$ LIBS=libsrc_libs
$
$
$
$!# We change CFLAGS temporarily so that C_SWITCH_X_SITE gets used
$!# for the tests that follow.
$!
$ if "''HAVE_X11'" .eqs. "YES"
$  then
$   if "''C_SWITCH_X_SITE'" .nes. "" then ac_flags=C_SWITCH_X_SITE + " " + ac_flags
$   if "''LIBX'" .nes. "" then LIBS=LIBX + " " + LIBS
$   if "''LD_SWITCH_X_SITE'" .nes. "" then LDFLAGS=LD_SWITCH_X_SITE + " " + LDFLAGS
$ ac_l = "XrmSetDatabase XScreenResourceString "+-
"               XScreenNumberOfScreen XSetWMProtocols "+-
""
$ ac_i = 0
$ac_hf_VMS_LABLE0:
$ ac_func = f$element(ac_i," ",ac_l)
$ ac_i = ac_i + 1
$ if ac_func .nes. " "
$  then
$   if ac_func .nes. ""
$    then
$     !# Translates all lower case characters to upper case
$     ac_tr_func := HAVE_'ac_func'
$ if .not. silent then write sys$output "checking ","for ''ac_func'"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <ctype.h>"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { /* The GNU C library defines this for functions which it implements"
$ write conftest_file "    to always fail with ENOSYS.  Some functions are actually named"
$ write conftest_file "    something starting with __ and the normal name is an alias.  */"
$ write conftest_file "#if defined (__stub_''ac_func') || defined (__stub___''ac_func')"
$ write conftest_file "choke me"
$ write conftest_file "#else"
$ write conftest_file "/* Override any gcc2 internal prototype to avoid an error.  */"
$ write conftest_file "extern char ''ac_func'(); ''ac_func'();"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_'ac_tr_func' = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining ''ac_tr_func'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define ''ac_tr_func' 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """''ac_tr_func'""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""''ac_tr_func'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""''ac_tr_func'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""''ac_tr_func'"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$    endif
$   goto ac_hf_VMS_LABLE0
$  endif 
$
$  endif
$
$ if "''HAVE_X11'" .eqs. "YES"
$  then
$ if .not. silent then write sys$output "checking ","for X11 version"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <X11/Xlib.h>"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { "
$ write conftest_file "#if XlibSpecificationRelease < 6"
$ write conftest_file "fail;"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_X11R6 = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_X11R6"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_X11R6 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_X11R6""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_X11R6"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_X11R6"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_X11R6"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$  endif
$
$ if "''USE_X_TOOLKIT'" .nes. "NONE"
$  then
$ if .not. silent then write sys$output "checking ","for X11 toolkit version >= 6"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <X11/Intrinsic.h>"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { "
$ write conftest_file "#if XtSpecificationRelease < 6"
$ write conftest_file "fail;"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_X11XTR6 = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_X11XTR6"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_X11XTR6 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_X11XTR6""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_X11XTR6"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_X11XTR6"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_X11XTR6"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$   ! The following may be added for the X11XTR6 case in the near future.
$   ! Right now, there's no R6 library for , and I have no idea how
$   ! DEC will name it.
$   type sys$input:/output=conftest2.opt
SYS$SHARE:DECW$XtLIBSHRR5.EXE/SHARE
$   ac_opts = ""
$   if f$type(OPTS) then ac_opts = OPTS
$   OPTS := conftest2.opt/opt
$ if .not. silent then write sys$output "checking ","for X11 toolkit version == 5"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <X11/Intrinsic.h>"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { "
$ write conftest_file "#if XtSpecificationRelease != 5"
$ write conftest_file "fail;"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_X11XTR5 = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_X11XTR5"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_X11XTR5 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_X11XTR5""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_X11XTR5"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_X11XTR5"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_X11XTR5"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$   OPTS = ac_opts
$  endif
$
$!# If netdb.h doesn't declare h_errno, we must declare it by hand.
$ if .not. silent then write sys$output "checking ","for declaration of h_errno in netdb.h"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <netdb.h>"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { "
$ write conftest_file "int"
$ write conftest_file "foo ()"
$ write conftest_file "{"
$ write conftest_file "  return h_errno;"
$ write conftest_file "}"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_H_ERRNO = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_H_ERRNO"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_H_ERRNO 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_H_ERRNO""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_H_ERRNO"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_H_ERRNO"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_H_ERRNO"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$!# The Ultrix 4.2 mips built in alloca declared by alloca.h only works
$!# for constant arguments.  Useless!
$ if .not. silent then write sys$output "checking ","for working alloca.h"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <alloca.h>"
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { char *p = alloca(2 * sizeof(int));; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_ALLOCA_H = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_ALLOCA_H"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_ALLOCA_H 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_ALLOCA_H""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_ALLOCA_H"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_ALLOCA_H"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_ALLOCA_H"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$ if .not. silent then write sys$output "checking ","for alloca"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#ifdef __GNUC__"
$ write conftest_file "#define alloca __builtin_alloca"
$ write conftest_file "#else"
$ write conftest_file "#ifdef __DECC"
$ write conftest_file "#include <builtins.h>"
$ write conftest_file "#define alloca __ALLOCA"
$ write conftest_file "#else"
$ write conftest_file "#if HAVE_ALLOCA_H"
$ write conftest_file "#include <alloca.h>"
$ write conftest_file "#else"
$ write conftest_file "#ifdef _AIX"
$ write conftest_file " #pragma alloca"
$ write conftest_file "#else"
$ write conftest_file "char *alloca ();"
$ write conftest_file "#endif"
$ write conftest_file "#endif"
$ write conftest_file "#endif"
$ write conftest_file "#endif"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { char *p = (char *) alloca(1);; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_ALLOCA = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_ALLOCA"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_ALLOCA 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_ALLOCA""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_ALLOCA"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_ALLOCA"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_ALLOCA"",""1"");"
$
$  else
$ delete/nolog conftest*.*.*
$ ac_alloca_missing=1
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file ""
$ write conftest_file "#if defined(CRAY) && ! defined(CRAY2)"
$ write conftest_file "winnitude"
$ write conftest_file "#else"
$ write conftest_file "lossage"
$ write conftest_file "#endif"
$ write conftest_file ""
$ close conftest_file
$ set noon
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_preprocess
$ search/out=nla0: conftest.out "winnitude"
$ ac_status = $status
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ set on
$ if ac_status .eq. 1
$  then
$ delete/nolog conftest*.*.*
$ if .not. silent then write sys$output "checking ","for _getb67"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <ctype.h>"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { /* The GNU C library defines this for functions which it implements"
$ write conftest_file "    to always fail with ENOSYS.  Some functions are actually named"
$ write conftest_file "    something starting with __ and the normal name is an alias.  */"
$ write conftest_file "#if defined (__stub__getb67) || defined (__stub____getb67)"
$ write conftest_file "choke me"
$ write conftest_file "#else"
$ write conftest_file "#ifndef _getb67 /* It might be implemented as a macro, as with DEC C for AXP */"
$ write conftest_file "/* Override any gcc2 internal prototype to avoid an error.  */"
$ write conftest_file "extern char _getb67();"
$ write conftest_file "#endif"
$ write conftest_file "_getb67();"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_CRAY_STACKSEG_END = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining CRAY_STACKSEG_END to be _getb67"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define CRAY_STACKSEG_END _getb67"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """CRAY_STACKSEG_END""=""_getb67"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""CRAY_STACKSEG_END"",""_getb67"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""CRAY_STACKSEG_END"",""_getb67"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""CRAY_STACKSEG_END"",""_getb67"");"
$
$  else
$ delete/nolog conftest*.*.*
$ if .not. silent then write sys$output "checking ","for GETB67"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <ctype.h>"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { /* The GNU C library defines this for functions which it implements"
$ write conftest_file "    to always fail with ENOSYS.  Some functions are actually named"
$ write conftest_file "    something starting with __ and the normal name is an alias.  */"
$ write conftest_file "#if defined (__stub_GETB67) || defined (__stub___GETB67)"
$ write conftest_file "choke me"
$ write conftest_file "#else"
$ write conftest_file "#ifndef GETB67 /* It might be implemented as a macro, as with DEC C for AXP */"
$ write conftest_file "/* Override any gcc2 internal prototype to avoid an error.  */"
$ write conftest_file "extern char GETB67();"
$ write conftest_file "#endif"
$ write conftest_file "GETB67();"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_CRAY_STACKSEG_END = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining CRAY_STACKSEG_END to be GETB67"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define CRAY_STACKSEG_END GETB67"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """CRAY_STACKSEG_END""=""GETB67"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""CRAY_STACKSEG_END"",""GETB67"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""CRAY_STACKSEG_END"",""GETB67"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""CRAY_STACKSEG_END"",""GETB67"");"
$
$  else
$ delete/nolog conftest*.*.*
$ if .not. silent then write sys$output "checking ","for getb67"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <ctype.h>"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { /* The GNU C library defines this for functions which it implements"
$ write conftest_file "    to always fail with ENOSYS.  Some functions are actually named"
$ write conftest_file "    something starting with __ and the normal name is an alias.  */"
$ write conftest_file "#if defined (__stub_getb67) || defined (__stub___getb67)"
$ write conftest_file "choke me"
$ write conftest_file "#else"
$ write conftest_file "/* Override any gcc2 internal prototype to avoid an error.  */"
$ write conftest_file "extern char getb67(); getb67();"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_CRAY_STACKSEG_END = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining CRAY_STACKSEG_END to be getb67"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define CRAY_STACKSEG_END getb67"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """CRAY_STACKSEG_END""=""getb67"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""CRAY_STACKSEG_END"",""getb67"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""CRAY_STACKSEG_END"",""getb67"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""CRAY_STACKSEG_END"",""getb67"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$ if "''ac_alloca_missing'" .nes. ""
$  then
$   !# The SVR3 libPW and SVR4 libucb both contain incompatible functions
$   !# that cause trouble.  Some versions do not even contain alloca or
$   !# contain a buggy version.  If you still want to use their alloca,
$   !# use ar to extract alloca.o from them instead of compiling alloca.c.
$   ALLOCA=",alloca.obj"
$!#
$ _ac_is_defined_C_ALLOCA = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining C_ALLOCA"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define C_ALLOCA 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """C_ALLOCA""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""C_ALLOCA"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""C_ALLOCA"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""C_ALLOCA"",""1"");"
$
$
$ if .not. silent then write sys$output "checking ","stack direction for C alloca"
$ if .not. silent then write sys$output "checking ","whether cross-compiling"
$!# If we cannot run a trivial program, we must be cross compiling.
$   open/write conftest_file conftest.'ac_ext'
$   write conftest_file "#include ""confdefs.h"""
$   write conftest_file "void exit();main(){exit(0);}"
$   close conftest_file
$   ac_file_name := conftest.'ac_ext'
$   ac_default_name := conftest.'ac_ext'
$   if ac_file_name .nes. ac_default_name then copy 'ac_file_name 'ac_default_name
$   set noon
$   _tmp = -1 ! Because DCL can't jump into the middle of an IF stmt
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if f$search("conftest.obj") .eqs. "" then goto ac_tp_VMS_LABLE1
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .ne. 1 then goto ac_tp_VMS_LABLE1
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   run conftest
$   ac_status = $status
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   set on
$ ac_tp_VMS_LABLE1:
$   if ac_status .eq. 1 .or. ac_status .eq. 0 ! UNIXoid programs return 0 for success
$    then
$!
$  else
$ cross_compiling=1
$    endif
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$
$ if "''cross_compiling'" .nes. ""
$  then
$!#
$ _ac_is_defined_STACK_DIRECTION = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining STACK_DIRECTION to be 0"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define STACK_DIRECTION 0"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """STACK_DIRECTION""=""0"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""STACK_DIRECTION"",""0"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""STACK_DIRECTION"",""0"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""STACK_DIRECTION"",""0"");"
$
$  else
$   open/write conftest_file conftest.'ac_ext'
$   write conftest_file "#include ""confdefs.h"""
$   write conftest_file "find_stack_direction ()"
$   write conftest_file "{"
$   write conftest_file "  static char *addr = 0;"
$   write conftest_file "  auto char dummy;"
$   write conftest_file "  if (addr == 0)"
$   write conftest_file "    {"
$   write conftest_file "      addr = &dummy;"
$   write conftest_file "      return find_stack_direction ();"
$   write conftest_file "    }"
$   write conftest_file "  else"
$   write conftest_file "    return (&dummy > addr) ? 1 : -1;"
$   write conftest_file "}"
$   write conftest_file "void exit();"
$   write conftest_file "main ()"
$   write conftest_file "{"
$   write conftest_file "  exit (find_stack_direction() < 0 ? 2 : 1); /* slightly changed for VMS */"
$   write conftest_file "}"
$   close conftest_file
$   ac_file_name := conftest.'ac_ext'
$   ac_default_name := conftest.'ac_ext'
$   if ac_file_name .nes. ac_default_name then copy 'ac_file_name 'ac_default_name
$   set noon
$   _tmp = -1 ! Because DCL can't jump into the middle of an IF stmt
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if f$search("conftest.obj") .eqs. "" then goto ac_tp_VMS_LABLE2
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .ne. 1 then goto ac_tp_VMS_LABLE2
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   run conftest
$   ac_status = $status
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   set on
$ ac_tp_VMS_LABLE2:
$   if ac_status .eq. 1 .or. ac_status .eq. 0 ! UNIXoid programs return 0 for success
$    then
$!#
$ _ac_is_defined_STACK_DIRECTION = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining STACK_DIRECTION to be 1"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define STACK_DIRECTION 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """STACK_DIRECTION""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""STACK_DIRECTION"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""STACK_DIRECTION"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""STACK_DIRECTION"",""1"");"
$
$  else
$!#
$ _ac_is_defined_STACK_DIRECTION = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining STACK_DIRECTION to be -1"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define STACK_DIRECTION -1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """STACK_DIRECTION""=""-1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""STACK_DIRECTION"",""-1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""STACK_DIRECTION"",""-1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""STACK_DIRECTION"",""-1"");"
$
$    endif
$  endif
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$ endif
$
$
$!# logb and frexp are found in -lm on most systems.
$ ac_l = "gettimeofday gethostname dup2 rename closedir "+-
"               mkdir rmdir random lrand48 bcopy bcmp logb frexp "+-
"               fmod drem ftime res_init setsid strerror fpathconf "+-
"               tzset localtime gmtime getpagesize utimes "+-
""
$ ac_i = 0
$ac_hf_VMS_LABLE1:
$ ac_func = f$element(ac_i," ",ac_l)
$ ac_i = ac_i + 1
$ if ac_func .nes. " "
$  then
$   if ac_func .nes. ""
$    then
$     !# Translates all lower case characters to upper case
$     ac_tr_func := HAVE_'ac_func'
$ if .not. silent then write sys$output "checking ","for ''ac_func'"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <ctype.h>"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { /* The GNU C library defines this for functions which it implements"
$ write conftest_file "    to always fail with ENOSYS.  Some functions are actually named"
$ write conftest_file "    something starting with __ and the normal name is an alias.  */"
$ write conftest_file "#if defined (__stub_''ac_func') || defined (__stub___''ac_func')"
$ write conftest_file "choke me"
$ write conftest_file "#else"
$ write conftest_file "/* Override any gcc2 internal prototype to avoid an error.  */"
$ write conftest_file "extern char ''ac_func'(); ''ac_func'();"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_'ac_tr_func' = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining ''ac_tr_func'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define ''ac_tr_func' 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """''ac_tr_func'""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""''ac_tr_func'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""''ac_tr_func'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""''ac_tr_func'"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$    endif
$   goto ac_hf_VMS_LABLE1
$  endif 
$
$ if .not. silent then write sys$output "checking ","for vfork"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <ctype.h>"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { /* The GNU C library defines this for functions which it implements"
$ write conftest_file "    to always fail with ENOSYS.  Some functions are actually named"
$ write conftest_file "    something starting with __ and the normal name is an alias.  */"
$ write conftest_file "#if defined (__stub_vfork) || defined (__stub___vfork)"
$ write conftest_file "choke me"
$ write conftest_file "#else"
$ write conftest_file "#ifndef vfork /* It might be implemented as a macro, as with DEC C for AXP */"
$ write conftest_file "/* Override any gcc2 internal prototype to avoid an error.  */"
$ write conftest_file "extern char vfork();"
$ write conftest_file "#endif"
$ write conftest_file "vfork();"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_VFORK = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_VFORK"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_VFORK 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_VFORK""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_VFORK"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_VFORK"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_VFORK"",""1"");"
$
$  else
$ delete/nolog conftest*.*.*
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <ctype.h>"
$ write conftest_file "#include <processes.h>"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { /* The GNU C library defines this for functions which it implements"
$ write conftest_file "    to always fail with ENOSYS.  Some functions are actually named"
$ write conftest_file "    something starting with __ and the normal name is an alias.  */"
$ write conftest_file "#if defined (__stub_vfork) || defined (__stub___vfork)"
$ write conftest_file "choke me"
$ write conftest_file "#else"
$ write conftest_file "#ifndef vfork /* It might be implemented as a macro, as with DEC C for AXP */"
$ write conftest_file "/* Override any gcc2 internal prototype to avoid an error.  */"
$ write conftest_file "extern char vfork();"
$ write conftest_file "#endif"
$ write conftest_file "vfork();"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$ delete/nolog conftest*.*.*
$!#
$ _ac_is_defined_HAVE_VFORK = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_VFORK"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_VFORK 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_VFORK""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_VFORK"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_VFORK"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_VFORK"",""1"");"
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$ ok_so_far="YES"
$ if .not. silent then write sys$output "checking ","for socket"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <ctype.h>"
$ write conftest_file ""
$ write conftest_file "int main() { return 1; }"
$ write conftest_file "int t() { /* The GNU C library defines this for functions which it implements"
$ write conftest_file "    to always fail with ENOSYS.  Some functions are actually named"
$ write conftest_file "    something starting with __ and the normal name is an alias.  */"
$ write conftest_file "#if defined (__stub_socket) || defined (__stub___socket)"
$ write conftest_file "choke me"
$ write conftest_file "#else"
$ write conftest_file "#ifndef socket /* It might be implemented as a macro, as with DEC C for AXP */"
$ write conftest_file "/* Override any gcc2 internal prototype to avoid an error.  */"
$ write conftest_file "extern char socket();"
$ write conftest_file "#endif"
$ write conftest_file "socket();"
$ write conftest_file "#endif"
$ write conftest_file "; return 0; }"
$ close conftest_file
$ _ac_cc_flag = "NO"
$!# def/user sys$error nla0:
$!# def/user sys$output nla0:
$ set noon
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$ ac_compile
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$ if f$search("conftest.obj") .nes. ""
$  then
$ __debug = f$trnlnm("DEBUG_CONFIGURE")
$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment("MESSAGE")
$ if .not. __debug then set message/nofacility/noident/noseverity/notext
$ if .not. __debug then def/user sys$output nl:
$ if .not. __debug then def/user sys$error nl:
$   ac_link
$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'
$
$   if ac_severity .eq. 1
$    then
$     _ac_cc_flag = "YES"
$    endif
$  endif
$ set on
$ if _ac_cc_flag
$  then
$!
$  else
$ delete/nolog conftest*.*.*
$ ok_so_far="NO"
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$ if ok_so_far
$  then
$ if .not. silent then write sys$output "checking ","for netinet/in.h"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <netinet/in.h>"
$ write conftest_file ""
$ close conftest_file
$ set noon
$ def/user sys$output conftest.err
$ def/user sys$error conftest.err
$ 'ac_cpp' nla0: conftest.'ac_ext'
$ ac_severity = $severity
$ set on
$ ac_err := YES
$ if f$search("conftest.err") .nes. "" then -
	if f$file("conftest.err","EOF") .ne. 0 then -
	ac_err := NO
$! Alternate test for gcc
$ if ac_gcc .and. ac_severity .gt. 1 then ac_err := NO
$ if ac_err
$  then
$!
$  else
$ delete/nolog conftest*.*.*
$ ok_so_far="NO"
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$  endif
$ if ok_so_far
$  then
$ if .not. silent then write sys$output "checking ","for arpa/inet.h"
$ open/write conftest_file conftest.'ac_ext'
$ write conftest_file "#include ""confdefs.h"""
$ write conftest_file "#include <arpa/inet.h>"
$ write conftest_file ""
$ close conftest_file
$ set noon
$ def/user sys$output conftest.err
$ def/user sys$error conftest.err
$ 'ac_cpp' nla0: conftest.'ac_ext'
$ ac_severity = $severity
$ set on
$ ac_err := YES
$ if f$search("conftest.err") .nes. "" then -
	if f$file("conftest.err","EOF") .ne. 0 then -
	ac_err := NO
$! Alternate test for gcc
$ if ac_gcc .and. ac_severity .gt. 1 then ac_err := NO
$ if ac_err
$  then
$!
$  else
$ delete/nolog conftest*.*.*
$ ok_so_far="NO"
$  endif
$ dummy = f$search("foo.bar.1") ! To prevent f$search bug.
$ if f$search("CONFTEST*.*.*") .nes. "" then delete/nolog conftest*.*.*
$
$
$  endif
$ if ok_so_far
$  then
$!#
$ _ac_is_defined_HAVE_INET_SOCKETS = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_INET_SOCKETS"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_INET_SOCKETS 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_INET_SOCKETS""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_INET_SOCKETS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_INET_SOCKETS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_INET_SOCKETS"",""1"");"
$
$  endif
$
$
$!# Set up the CFLAGS for real compilation, so we can substitute it.
$ CFLAGS=REAL_CFLAGS
$
$!#### Find out which version of Emacs this is.
$ versiontempname = "version-tmp-''pid'.lis"
$ __lispdir = srcdir - "]" + ".LISP]"
$ search '__lispdir'version.el "defconst","emacs-version"/match=and/nohead-
	/output='versiontempname'
$ open/read/error=no_version/end=no_version in 'versiontempname'
$ read/error=no_version/end=no_version in foo
$ close in
$ delete/nolog 'versiontempname'*.*
$ foo = f$element(1,"""",foo)
$ emacs_version_major = f$element(0,".",foo)
$ emacs_version_minor = f$element(1,".",foo)
$ version = emacs_version_major + "." + emacs_version_minor
$ if "''version'" .nes. ""
$  then say "Emacs version is ''version'"
$	version_us = f$element(0,".",version)
$	v_i = 1
$loop_version:
$	v_e = f$element(v_i,".",version)
$	v_i = v_i + 1
$	if v_e .nes. "."
$	 then
$	  version_us = version_us + "_" + v_e
$	 endif
$  else
$no_version:
$   a = srcdir - "]" + ".LISP]"
$   sayerr "''progname': can't find current emacs version in"
$   sayerr "	`''a'VERSION.EL'."
$   exit 0
$  endif
$
$
$!#### Specify what sort of things we'll be editing into descrip.mms and
$!#### config.h.
$!#### Use configuration here uncanonicalized to avoid exceeding size limits.
$
$ srcdir_dev = f$parse(srcdir,,,"DEVICE")
$ srcdir_dir = f$parse(srcdir,,,"DIRECTORY") - "[" - "<" - "]" - ">"




$ EXEEXT = ".exe" !		ttn
$
$!#
$ _ac_is_defined_EMACS_VERSION_MAJOR = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining EMACS_VERSION_MAJOR to be ''emacs_version_major'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define EMACS_VERSION_MAJOR ''emacs_version_major'"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """EMACS_VERSION_MAJOR""=""''emacs_version_major'"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""EMACS_VERSION_MAJOR"",""''emacs_version_major'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""EMACS_VERSION_MAJOR"",""''emacs_version_major'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""EMACS_VERSION_MAJOR"",""''emacs_version_major'"");"
$
$!#
$ _ac_is_defined_EMACS_VERSION_MINOR = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining EMACS_VERSION_MINOR to be ''emacs_version_minor'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define EMACS_VERSION_MINOR ''emacs_version_minor'"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """EMACS_VERSION_MINOR""=""''emacs_version_minor'"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""EMACS_VERSION_MINOR"",""''emacs_version_minor'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""EMACS_VERSION_MINOR"",""''emacs_version_minor'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""EMACS_VERSION_MINOR"",""''emacs_version_minor'"");"
$
$!#
$ _ac_is_defined_EMACS_CONFIGURATION = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining EMACS_CONFIGURATION to be ""''configuration'"""
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define EMACS_CONFIGURATION ""''configuration'"""
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """EMACS_CONFIGURATION""=""""''configuration'"""""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""EMACS_CONFIGURATION"",""""""''configuration'"""""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""EMACS_CONFIGURATION"",""""""''configuration'"""""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""EMACS_CONFIGURATION"",""""""''configuration'"""""");"
$
$!#
$ _ac_is_defined_EMACS_CONFIG_OPTIONS = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining EMACS_CONFIG_OPTIONS to be ""''configure_args'"""
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define EMACS_CONFIG_OPTIONS ""''configure_args'"""
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """EMACS_CONFIG_OPTIONS""=""""''configure_args'"""""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""EMACS_CONFIG_OPTIONS"",""""""''configure_args'"""""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""EMACS_CONFIG_OPTIONS"",""""""''configure_args'"""""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""EMACS_CONFIG_OPTIONS"",""""""''configure_args'"""""");"
$
$!#
$ _ac_is_defined_config_machfile = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining config_machfile to be ""''srcdir_dev'[''srcdir_dir'.src.''machfile_dir']''machfile'"""
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define config_machfile ""''srcdir_dev'[''srcdir_dir'.src.''machfile_dir']''machfile'"""
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """config_machfile""=""""''srcdir_dev'[''srcdir_dir'.src.''machfile_dir']''machfile'"""""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""config_machfile"",""""""''srcdir_dev'[''srcdir_dir'.src.''machfile_dir']''machfile'"""""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""config_machfile"",""""""''srcdir_dev'[''srcdir_dir'.src.''machfile_dir']''machfile'"""""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""config_machfile"",""""""''srcdir_dev'[''srcdir_dir'.src.''machfile_dir']''machfile'"""""");"
$
$!#
$ _ac_is_defined_config_opsysfile = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining config_opsysfile to be ""''srcdir_dev'[''srcdir_dir'.src.''opsysfile_dir']''opsysfile'"""
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define config_opsysfile ""''srcdir_dev'[''srcdir_dir'.src.''opsysfile_dir']''opsysfile'"""
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """config_opsysfile""=""""''srcdir_dev'[''srcdir_dir'.src.''opsysfile_dir']''opsysfile'"""""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""config_opsysfile"",""""""''srcdir_dev'[''srcdir_dir'.src.''opsysfile_dir']''opsysfile'"""""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""config_opsysfile"",""""""''srcdir_dev'[''srcdir_dir'.src.''opsysfile_dir']''opsysfile'"""""");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""config_opsysfile"",""""""''srcdir_dev'[''srcdir_dir'.src.''opsysfile_dir']''opsysfile'"""""");"
$
$!#
$ _ac_is_defined_LD_SWITCH_X_SITE = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining LD_SWITCH_X_SITE to be ''LD_SWITCH_X_SITE'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define LD_SWITCH_X_SITE ''LD_SWITCH_X_SITE'"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """LD_SWITCH_X_SITE""=""''LD_SWITCH_X_SITE'"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""LD_SWITCH_X_SITE"",""''LD_SWITCH_X_SITE'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""LD_SWITCH_X_SITE"",""''LD_SWITCH_X_SITE'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""LD_SWITCH_X_SITE"",""''LD_SWITCH_X_SITE'"");"
$
$!#
$ _ac_is_defined_LD_SWITCH_X_SITE_AUX = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining LD_SWITCH_X_SITE_AUX to be ''LD_SWITCH_X_SITE_AUX'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define LD_SWITCH_X_SITE_AUX ''LD_SWITCH_X_SITE_AUX'"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """LD_SWITCH_X_SITE_AUX""=""''LD_SWITCH_X_SITE_AUX'"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""LD_SWITCH_X_SITE_AUX"",""''LD_SWITCH_X_SITE_AUX'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""LD_SWITCH_X_SITE_AUX"",""''LD_SWITCH_X_SITE_AUX'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""LD_SWITCH_X_SITE_AUX"",""''LD_SWITCH_X_SITE_AUX'"");"
$
$!#
$ _ac_is_defined_C_SWITCH_X_SITE = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining C_SWITCH_X_SITE to be ''C_SWITCH_X_SITE'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define C_SWITCH_X_SITE ''C_SWITCH_X_SITE'"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """C_SWITCH_X_SITE""=""''C_SWITCH_X_SITE'"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""C_SWITCH_X_SITE"",""''C_SWITCH_X_SITE'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""C_SWITCH_X_SITE"",""''C_SWITCH_X_SITE'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""C_SWITCH_X_SITE"",""''C_SWITCH_X_SITE'"");"
$
$!#
$ _ac_is_defined_UNEXEC_SRC = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining UNEXEC_SRC to be ''UNEXEC_SRC'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define UNEXEC_SRC ''UNEXEC_SRC'"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """UNEXEC_SRC""=""''UNEXEC_SRC'"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""UNEXEC_SRC"",""''UNEXEC_SRC'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""UNEXEC_SRC"",""''UNEXEC_SRC'"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""UNEXEC_SRC"",""''UNEXEC_SRC'"");"
$
$ if with_debug_hack
$  then
$!#
$ _ac_is_defined_USE_DEBUG_HACK = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining USE_DEBUG_HACK"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define USE_DEBUG_HACK 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """USE_DEBUG_HACK""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""USE_DEBUG_HACK"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""USE_DEBUG_HACK"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""USE_DEBUG_HACK"",""1"");"
$
$  endif
$
$
$ if "''HAVE_SOCKETS'" .eqs. "YES"
$  then 
$!#
$ _ac_is_defined_HAVE_SOCKETS = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_SOCKETS"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_SOCKETS 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_SOCKETS""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_SOCKETS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_SOCKETS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_SOCKETS"",""1"");"
$
$!#
$ _ac_is_defined_'f$edit(tcpip_package,"UPCASE")' = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining ''f$edit(tcpip_package,""UPCASE"")'"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define ''f$edit(tcpip_package,""UPCASE"")' 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """''f$edit(tcpip_package,""UPCASE"")'""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""''f$edit(tcpip_package,""UPCASE"")'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""''f$edit(tcpip_package,""UPCASE"")'"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""''f$edit(tcpip_package,""UPCASE"")'"",""1"");"
$ 
$  endif
$ if "''HAVE_X_WINDOWS'" .eqs. "YES" 
$  then
   $!#
$ _ac_is_defined_HAVE_X_WINDOWS = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_X_WINDOWS"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_X_WINDOWS 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_X_WINDOWS""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_X_WINDOWS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_X_WINDOWS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_X_WINDOWS"",""1"");"
$ 
$  endif
$ if "''USE_X_TOOLKIT'" .nes. "NONE"
$  then
   $!#
$ _ac_is_defined_USE_X_TOOLKIT = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining USE_X_TOOLKIT"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define USE_X_TOOLKIT 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """USE_X_TOOLKIT""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""USE_X_TOOLKIT"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""USE_X_TOOLKIT"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""USE_X_TOOLKIT"",""1"");"
$ 
$  endif
$ if "''HAVE_X11'" .eqs. "YES" 
$  then
   $!#
$ _ac_is_defined_HAVE_X11 = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_X11"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_X11 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_X11""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_X11"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_X11"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_X11"",""1"");"
$ 
$  endif
$ if "''HAVE_XFREE386'" .eqs. "YES" 
$  then
   $!#
$ _ac_is_defined_HAVE_XFREE386 = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_XFREE386"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_XFREE386 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_XFREE386""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_XFREE386"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_XFREE386"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_XFREE386"",""1"");"
$ 
$  endif
$ if "''HAVE_MENUS'" .eqs. "YES" 
$  then
   $!#
$ _ac_is_defined_HAVE_MENUS = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining HAVE_MENUS"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define HAVE_MENUS 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """HAVE_MENUS""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""HAVE_MENUS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""HAVE_MENUS"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""HAVE_MENUS"",""1"");"
$ 
$  endif
$ if "''GNU_MALLOC'" .eqs. "YES" 
$  then
   $!#
$ _ac_is_defined_GNU_MALLOC = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining GNU_MALLOC"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define GNU_MALLOC 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """GNU_MALLOC""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""GNU_MALLOC"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""GNU_MALLOC"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""GNU_MALLOC"",""1"");"
$ 
$  endif
$ if "''REL_ALLOC'" .eqs. "YES" 
$  then
   $!#
$ _ac_is_defined_REL_ALLOC = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining REL_ALLOC"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define REL_ALLOC 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """REL_ALLOC""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""REL_ALLOC"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""REL_ALLOC"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""REL_ALLOC"",""1"");"
$ 
$  endif
$ if "''LISP_FLOAT_TYPE'" .eqs. "YES" 
$  then
   $!#
$ _ac_is_defined_LISP_FLOAT_TYPE = 1
$ if f$type(DEFS_COUNTER) .eqs. "" then DEFS_COUNTER = 0
$ if f$type(DEFS) .nes. "" then DEFS = DEFS + ","
$ if f$type(DEFS) .eqs. "" then DEFS:=
$ DEFS_COUNTER = DEFS_COUNTER + 1
$ if "''verbose'" then write sys$output -
"	defining LISP_FLOAT_TYPE"
$ open/append ac_define_file confdefs.h
$ write ac_define_file "#define LISP_FLOAT_TYPE 1"
$ close ac_define_file
$ DEFS'DEFS_COUNTER' = """LISP_FLOAT_TYPE""=""1"""
$! actually, no --ttn
$! actually, yes --ttn
$! really, no -- i insist! --ttn 2003-11-27
$! DEFS = DEFS + DEFS'DEFS_COUNTER'
$ if "''ac_sed_defs_COUNTER'" .eqs. "" then ac_sed_defs_COUNTER = 0
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_d(""LISP_FLOAT_TYPE"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_u(""LISP_FLOAT_TYPE"",""1"");"
$ ac_sed_defs_COUNTER = ac_sed_defs_COUNTER + 1
$ ac_sed_defs'ac_sed_defs_COUNTER' = -
	"ac_e(""LISP_FLOAT_TYPE"",""1"");"
$ 
$  endif
$
$!# ====================== Developer's configuration =======================
$
$!# The following assignments make sense if you're running Emacs on a single
$!# machine, one version at a time, and  you want changes to the lisp and etc
$!# directories in the source tree to show up immediately in your working
$!# environment.  It saves a great deal of disk space by not duplicating the
$!# lisp and etc directories.
$ 
$ if "''run_in_place'"
$  then
$   if .not. "''lispdir_specified'"
$    then
$     lispdir_dev="'srcdir_dev'"
$     lispdir_dir="'srcdir_dir'.lisp"
$     lispdir="'lispdir_dev'['lispdir_dir']"
$    endif
$   if .not. "''locallisp_specified'"
$    then
$     locallisppath="'srcdir_dev'['srcdir_dir'.site-lisp]"
$    endif
$   if .not. "''etcdir_specified'"
$    then
$     etcdir_dev="'srcdir_dev'"
$     etcdir_dir="'srcdir_dir'.etc"
$     etcdir="'etcdir_dev'['etcdir_dir']"
$    endif
$   if .not. "''lockdir_specified'"
$    then
$     lockdir_dev="'srcdir_dev'"
$     lockdir_dir="'srcdir_dir'.lock"
$     lockdir="'lockdir_dev'['lockdir_dir']"
$    endif
$!   if .not. "''archlibdir_specified'"
$!    then
$!     archlibdir_dev="'srcdir_dev'"
$!     archlibdir_dir="'srcdir_dir'.lib-src"
$!     archlibdir="'archlibdir_dev'['archlibdir_dir']"
$!    endif
$   if .not. "''docdir_specified'"
$    then
$     docdir_dev="'srcdir_dev'"
$     docdir_dir="'srcdir_dir'.etc"
$     docdir="'docdir_dev'['docdir_dir']"
$    endif
$   if .not. "''vmslibdir_specified'"
$    then
$     vmslibdir_dev="'srcdir_dev'"
$     vmslibdir_dir="'srcdir_dir'.vms"
$     vmslibdir="'vmslibdir_dev'['vmslibdir_dir']"
$    endif
$   if .not. "''infodir_specified'"
$    then
$     infodir_dev="'srcdir_dev'"
$     infodir_dir="'srcdir_dir'.info"
$     infodir="'infodir_dev'['infodir_dir']"
$    endif
$  else if "''single_tree'"
$   then
$    if .not. "''exec_prefix_specified'" 
$     then
$      exec_prefix_dev = "'prefix_dev'"
$      exec_prefix_dir = "'prefix_dir'"
$      exec_prefix="'prefix'"
$     endif
$    if .not. "''bindir_specified'"
$     then
$      bindir_dev="'exec_prefix_dev'"
$      bindir_dir="'exec_prefix_dir'.bin.'configuration_us'"
$      bindir="'bindir_dev'['bindir_dir']"
$     endif
$    if .not. "''datadir_specified'"
$     then
$      datadir_dev="'prefix_dev'"
$      datadir_dir="'prefix_dir'.common"
$      datadir="'datadir_dev'['datadir_dir']"
$     endif
$    if .not. "''statedir_specified'"
$     then
$      statedir_dev="'prefix_dev'"
$      statedir_dir="'prefix_dir'.common"
$      statedir="'statedir_dev'['statedir_dir']"
$     endif
$    if .not. "''libdir_specified'"
$     then
$      libdir_dev="'bindir_dev'"
$      libdir_dir="'bindir_dir'"
$      libdir="'libdir_dev'['libdir_dir']"
$     endif
$    if .not. "''lispdir_specified'"
$     then
$      lispdir_dev="'prefix_dev'"
$      lispdir_dir="'prefix_dir'.common.lisp"
$      lispdir="'lispdir_dev'['lispdir_dir']"
$     endif
$    if .not. "''locallisppath_specified'"
$     then
$      locallisppath="'prefix_dev'['prefix_dir'.common.site-lisp]"
$     endif
$    if .not. "''lockdir_specified'"
$     then
$      lockdir_dev="'prefix_dev'"
$      lockdir_dir="'prefix_dir'.common.lock"
$      lockdir="'lockdir_dev'['lockdir_dir']"
$     endif
$    if .not. "''archlibdir_specified'"
$     then
$      archlibdir_dev="'libdir_dev'"
$      archlibdir_dir="'libdir_dir'.etc"
$      archlibdir="'archlibdir_dev'['archlibdir_dir']"
$     endif
$    if .not. "''etcdir_specified'"
$     then
$      etcdir_dev="'prefix_dev'"
$      etcdir_dir="'prefix_dir'.common.data"
$      etcdir="'etcdir_dev'['etcdir_dir']"
$     endif
$    if .not. "''docdir_specified'"
$     then
$      docdir_dev="'prefix_dev'"
$      docdir_dir="'prefix_dir'.common.data"
$      docdir="'docdir_dev'['docdir_dir']"
$     endif
$    if .not. "''vmslibdir_specified'"
$     then
$      vmslibdir_dev="'libdir_dev'"
$      vmslibdir_dir="'libdir_dir'.vms"
$      vmslibdir="'vmslibdir_dev'['vmslibdir_dir']"
$     endif
$    if .not. "''vuelibdir_specified'"
$     then
$      vuelibdir_dev="'prefix_dev'"
$      vuelibdir_dir="'prefix_dir'.vms"
$      vuelibdir="'vuelibdir_dev'['vuelibdir_dir']"
$     endif
$    if .not. "''startupdir_specified'"
$     then
$      startupdir_dev="'prefix_dev'"
$      startupdir_dir="'prefix_dir'.vms"
$      startupdir="'startupdir_dev'['startupdir_dir']"
$     endif
$   endif
$ endif
$
$!#### Report on what we decided to do.
$ say ""
$ say ""
$ say "Configured for `''canonical''."
$ say ""
$ say "  Where should the build process find the source code?    ''srcdir'"
$ say "  What operating system and machine description files should Emacs use?"
$ say "        `''opsysfile'' and `''machfile''"
$ say "  What compiler should emacs be built with?               ''CC' ''CFLAGS'"
$ say "  Should Emacs use the GNU version of malloc?             ''GNU_MALLOC'''GNU_MALLOC_reason'"
$ say "  Should Emacs use the relocating allocator for buffers?  ''REL_ALLOC'"
$ say "  What window system should Emacs use?                    ''window_system'"
$ say "  What toolkit should Emacs use?                          ''USE_X_TOOLKIT'"
$ if "''x_includes'" .nes. "" then -
say "  Where do we find X Windows header files?                ''x_includes'"
$ if "''x_libraries'" .nes. "" then -
say "  Where do we find X Windows libraries?                   ''x_libraries'"
$ say ""
$ say ""
$
$!# Remove any trailing slashes in these variables.
$!test -n "${prefix}" &&
$!  prefix=`echo "${prefix}" | sed 's,\([^/]\)/*$,\1,'`
$!test -n "${exec_prefix}" &&
$!  exec_prefix=`echo "${exec_prefix}" | sed 's,\([^/]\)/*$,\1,'`
$
$!# The preferred way to propogate these variables is regular @ substitutions.
$ if "''prefix'" .nes. ""
$  then
$   prefix_dir = f$parse(prefix,,,"DIRECTORY") - "[" - "]" - "<" - ">"
$   prefix_dev = f$parse(prefix,,,"DEVICE")
$  else
$   prefix_dev := sys$sysdevice:
$   prefix_dir := gnu
$   prefix := sys$sysdevice:[gnu]
$  endif
$ if "''exec_prefix'" .nes. ""
$  then
$   exec_prefix := 'exec_prefix'
$   exec_prefix_dir = f$parse(exec_prefix,,,"DIRECTORY")-"[" - "]" - "<" - ">"
$   exec_prefix_dev = f$parse(exec_prefix,,,"DEVICE")
$  else
$   exec_prefix_dev = prefix_dev
$   exec_prefix_dir = prefix_dir
$   exec_prefix = prefix
$  endif
$ ac_prsub_counter = 0
$ if "''prefix_dev'" .nes. ""
$  then
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""prefix_dev""""+ ((p_0_or_more_spc +""""="""")@r1))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r = 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""prefix_dev""""+ ((p_0_or_more_spc +""""="""")@r1)+scan(""""""""))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r <> 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$! This is a piece of deep DCL magic.
$   ac_prsub_support'ac_prsub_counter'="__ac_prsub_tmp:='prefix_dev'"
$   ac_prsub'ac_prsub_counter'="res := """"prefix_dev"""" + """"=""""+""""'"+"'__ac_prsub_tmp'"""";"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="erase(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="copy_text(res);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$  endif
$!#
$
$ if "''prefix_dir'" .nes. ""
$  then
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""prefix_dir""""+ ((p_0_or_more_spc +""""="""")@r1))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r = 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""prefix_dir""""+ ((p_0_or_more_spc +""""="""")@r1)+scan(""""""""))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r <> 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$! This is a piece of deep DCL magic.
$   ac_prsub_support'ac_prsub_counter'="__ac_prsub_tmp:='prefix_dir'"
$   ac_prsub'ac_prsub_counter'="res := """"prefix_dir"""" + """"=""""+""""'"+"'__ac_prsub_tmp'"""";"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="erase(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="copy_text(res);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$  endif
$!#
$
$ if "''prefix'" .nes. ""
$  then
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""prefix""""+ ((p_0_or_more_spc +""""="""")@r1))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r = 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""prefix""""+ ((p_0_or_more_spc +""""="""")@r1)+scan(""""""""))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r <> 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$! This is a piece of deep DCL magic.
$   ac_prsub_support'ac_prsub_counter'="__ac_prsub_tmp:='prefix'"
$   ac_prsub'ac_prsub_counter'="res := """"prefix"""" + """"=""""+""""'"+"'__ac_prsub_tmp'"""";"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="erase(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="copy_text(res);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$  endif
$!#
$
$ if "''exec_prefix_dev'" .nes. ""
$  then
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""exec_prefix_dev""""+ ((p_0_or_more_spc +""""="""")@r1))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r = 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""exec_prefix_dev""""+ ((p_0_or_more_spc +""""="""")@r1)+scan(""""""""))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r <> 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$! This is a piece of deep DCL magic.
$   ac_prsub_support'ac_prsub_counter'="__ac_prsub_tmp:='exec_prefix_dev'"
$   ac_prsub'ac_prsub_counter'="res := """"exec_prefix_dev"""" + """"=""""+""""'"+"'__ac_prsub_tmp'"""";"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="erase(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="copy_text(res);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$  endif
$!#
$
$ if "''exec_prefix_dir'" .nes. ""
$  then
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""exec_prefix_dir""""+ ((p_0_or_more_spc +""""="""")@r1))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r = 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""exec_prefix_dir""""+ ((p_0_or_more_spc +""""="""")@r1)+scan(""""""""))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r <> 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$! This is a piece of deep DCL magic.
$   ac_prsub_support'ac_prsub_counter'="__ac_prsub_tmp:='exec_prefix_dir'"
$   ac_prsub'ac_prsub_counter'="res := """"exec_prefix_dir"""" + """"=""""+""""'"+"'__ac_prsub_tmp'"""";"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="erase(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="copy_text(res);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$  endif
$!#
$
$ if "''exec_prefix'" .nes. ""
$  then
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""exec_prefix""""+ ((p_0_or_more_spc +""""="""")@r1))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r = 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(beginning_of (main_buffer));"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'= "r := search_quietly(LINE_BEGIN+(""""exec_prefix""""+ ((p_0_or_more_spc +""""="""")@r1)+scan(""""""""))@r0+LINE_END,FORWARD);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="if (r <> 0)"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="then"
$   ac_prsub_counter = ac_prsub_counter + 1
$! This is a piece of deep DCL magic.
$   ac_prsub_support'ac_prsub_counter'="__ac_prsub_tmp:='exec_prefix'"
$   ac_prsub'ac_prsub_counter'="res := """"exec_prefix"""" + """"=""""+""""'"+"'__ac_prsub_tmp'"""";"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="erase(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="position(r0);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="copy_text(res);"
$   ac_prsub_counter = ac_prsub_counter + 1
$   ac_prsub'ac_prsub_counter'="endif;"
$  endif
$!#
$
$
$ set noon
$ if f$search("CONFIG.STATUS.*") .nes. "" then purge/nolog config.status
$ set on
$! trap 'rm -f config.status; exit 1' 1 2 15
$ write sys$output "creating config.status"
$ open/write config_status_file config.status
$ write config_status_file "$! Generated automatically by CONFIGURE.COM."
$ write config_status_file "$! Run this file to recreate the current configuration."
$ write config_status_file "$! This directory was configured as follows,"
$ write config_status_file "$! on host "+f$getsyi("NODENAME")+":"
$ write config_status_file "$!"
$ __tmp_p = f$env("PROCEDURE")
$ __tmp_p = f$parse(__tmp_p,,,"DEVICE")+f$parse(__tmp_p,,,"DIRECTORY")+ -
	f$parse(__tmp_p,,,"NAME")+f$parse(__tmp_p,,,"TYPE")
$ write config_status_file "$! @"+__tmp_p+" "+configure_args
$ write config_status_file "$"
$ write config_status_file "$ set symbol/verb/scope=(noglobal,nolocal)"
$ write config_status_file "$"
$ write config_status_file "$ ac_cs_usage = ""Usage: config.status [--recheck] [--version] [--help]"
$ write config_status_file "$ ac_will_recheck := no"
$ write config_status_file "$ ac_recheck_extra_args = """""
$ write config_status_file "$ ac_count = 8"
$ write config_status_file "$config_status_loop1:"
$ write config_status_file "$ if ac_count .gt. 0
$ write config_status_file "$  then
$ write config_status_file "$   if p'ac_count' .eqs. """""
$ write config_status_file "$    then"
$ write config_status_file "$     ac_count = ac_count - 1"
$ write config_status_file "$     goto config_status_loop1"
$ write config_status_file "$    endif"
$ write config_status_file "$  endif"
$ write config_status_file "$ i = 0"
$ write config_status_file "$config_status_loop2:"
$ write config_status_file "$ i = i + 1"
$ write config_status_file "$ if i .le. ac_count"
$ write config_status_file "$  then"
$ write config_status_file "$   if p'i' .eqs. ""-RECHECK"" -"
$ write config_status_file "	.or. p'i' .eqs. f$extract(0,f$length(p'i'),""--RECHECK"")"
$ write config_status_file "$    then"
$ write config_status_file "$     ac_will_recheck := yes"
$ write config_status_file "$     goto config_status_loop2"
$ write config_status_file "$    endif"
$ write config_status_file "$   if f$extract(0,f$locate(""="",p'i'),p'i') .eqs. f$extract(0,f$locate(""="",p'i'),""--NEW-ARGS"")"
$ write config_status_file "$    then"
$ write config_status_file "$     tmp = f$extract(f$locate(""="",p'i'),999,p'i')"
$ write config_status_file "$     if tmp .nes. """" then ac_recheck_extra_args = ac_recheck_extra_args + "" "" + (tmp - ""="")"
$ write config_status_file "$     goto config_status_loop2"
$ write config_status_file "$    endif"
$ write config_status_file "$   if p'i' .eqs. ""-VERSION"" -"
$ write config_status_file "	.or. p'i' .eqs. f$extract(0,f$length(p'i'),""--VERSION"")"
$ write config_status_file "$    then"
$ write config_status_file "$     write sys$output ""config.status generated by autoconf version 1.11"""
$ write config_status_file "$     exit 1"
$ write config_status_file "$    endif"
$ write config_status_file "$   if p'i' .eqs. ""-HELP"" -"
$ write config_status_file "	.or. p'i' .eqs. f$extract(0,f$length(p'i'),""--HELP"")"
$ write config_status_file "$    then"
$ write config_status_file "$     write sys$output ac_cs_usage"
$ write config_status_file "$     exit $status"
$ write config_status_file "$    endif"
$ write config_status_file "$   if p'i' .nes. """"
$ write config_status_file "$    then
$ write config_status_file "$     write sys$error ac_cs_usage"
$ write config_status_file "$     exit 0"
$ write config_status_file "$    endif"
$ write config_status_file "$   goto config_status_loop2"
$ write config_status_file "$  endif"
$ write config_status_file "$"
$ write config_status_file "$ if ac_will_recheck"
$ write config_status_file "$  then"
$ write config_status_file "$   write sys$output ""@"+__tmp_p+" "+configure_args+""",ac_recheck_extra_args"
$ write config_status_file "$   @"+__tmp_p+" "+f$edit(configure_args,"TRIM")+"'ac_recheck_extra_args'"
$ write config_status_file "$   exit $status"
$ write config_status_file "$  endif"
$ write config_status_file "$"
$ write config_status_file "$ set noon"
$ write config_status_file "$! _tmp = f$parse(""[.src]config.h"","";*"",,,""SYNTAX_ONLY"")
$ write config_status_file "$! if f$search(_tmp) .nes. """" then delete/nolog '_tmp'"
$ write config_status_file "$ if f$search(""CONFTEST*.*.*"") .nes. """" then delete/nolog conftest*.*.*"
$ write config_status_file "$ if f$search(""CONFDEFS*.*.*"") .nes. """" then delete/nolog confdefs*.*.*"
$ write config_status_file "$ set on"
$ write config_status_file "$ LDFLAGS=""''LDFLAGS'""" ! Experimental
$ write config_status_file "$ CC=""''CC'""" ! Experimental
$ write config_status_file "$ sys_includes=""''sys_includes'""" ! Experimental
$ write config_status_file "$ OPTS=""''OPTS'""" ! Experimental
$ write config_status_file "$ LIBS=""''LIBS'""" ! Experimental
$ write config_status_file "$ CFLAGS=""''CFLAGS'""" ! Experimental
$ write config_status_file "$ LN_S=""''LN_S'""" ! Experimental
$ write config_status_file "$ CPP=""''CPP'""" ! Experimental
$ write config_status_file "$ INSTALL=""''INSTALL'""" ! Experimental
$ write config_status_file "$ INSTALL_PROGRAM=""''INSTALL_PROGRAM'""" ! Experimental
$ write config_status_file "$ INSTALL_DATA=""''INSTALL_DATA'""" ! Experimental
$ write config_status_file "$ INSTALL_QUOTED=""''INSTALL_QUOTED'""" ! Experimental
$ write config_status_file "$ INSTALL_PROGRAM_QUOTED=""''INSTALL_PROGRAM_QUOTED'""" ! Experimental
$ write config_status_file "$ INSTALL_DATA_QUOTED=""''INSTALL_DATA_QUOTED'""" ! Experimental
$ write config_status_file "$ SET_MAKE=""''SET_MAKE'""" ! Experimental
$ write config_status_file "$ netinet_includes=""''netinet_includes'""" ! Experimental
$ write config_status_file "$ arpa_includes=""''arpa_includes'""" ! Experimental
$ write config_status_file "$ x_includes=""''x_includes'""" ! Experimental
$ write config_status_file "$ ALLOCA=""''ALLOCA'""" ! Experimental
$ write config_status_file "$ version=""''version'""" ! Experimental
$ write config_status_file "$ version_us=""''version_us'""" ! Experimental
$ write config_status_file "$ configuration=""''configuration'""" ! Experimental
$ write config_status_file "$ configuration_us=""''configuration_us'""" ! Experimental
$ write config_status_file "$ srcdir_dev=""''srcdir_dev'""" ! Experimental
$ write config_status_file "$ srcdir_dir=""''srcdir_dir'""" ! Experimental
$ write config_status_file "$ srcdir=""''srcdir'""" ! Experimental
$ write config_status_file "$ libsrc_libs=""''libsrc_libs'""" ! Experimental
$ write config_status_file "$ LD_SWITCH_X_SITE=""''LD_SWITCH_X_SITE'""" ! Experimental
$ write config_status_file "$ C_SWITCH_MACHINE=""''C_SWITCH_MACHINE'""" ! Experimental
$ write config_status_file "$ C_SWITCH_X_MACHINE=""''C_SWITCH_X_MACHINE'""" ! Experimental
$ write config_status_file "$ C_SWITCH_SYSTEM=""''C_SWITCH_SYSTEM'""" ! Experimental
$ write config_status_file "$ C_SWITCH_X_SYSTEM=""''C_SWITCH_X_SYSTEM'""" ! Experimental
$ write config_status_file "$ C_SWITCH_SITE=""''C_SWITCH_SITE'""" ! Experimental
$ write config_status_file "$ C_SWITCH_X_SITE=""''C_SWITCH_X_SITE'""" ! Experimental
$ write config_status_file "$ C_DEBUG_SWITCH=""''C_DEBUG_SWITCH'""" ! Experimental
$ write config_status_file "$ X_TOOLKIT_TYPE=""''X_TOOLKIT_TYPE'""" ! Experimental
$ write config_status_file "$ startupdir_dev=""''startupdir_dev'""" ! Experimental
$ write config_status_file "$ startupdir_dir=""''startupdir_dir'""" ! Experimental
$ write config_status_file "$ startupdir=""''startupdir'""" ! Experimental
$ write config_status_file "$ vuelibdir_dev=""''vuelibdir_dev'""" ! Experimental
$ write config_status_file "$ vuelibdir_dir=""''vuelibdir_dir'""" ! Experimental
$ write config_status_file "$ vuelibdir=""''vuelibdir'""" ! Experimental
$ write config_status_file "$ prefix_dev=""''prefix_dev'""" ! Experimental
$ write config_status_file "$ prefix_dir=""''prefix_dir'""" ! Experimental
$ write config_status_file "$ prefix=""''prefix'""" ! Experimental
$ write config_status_file "$ exec_prefix_dev=""''exec_prefix_dev'""" ! Experimental
$ write config_status_file "$ exec_prefix_dir=""''exec_prefix_dir'""" ! Experimental
$ write config_status_file "$ exec_prefix=""''exec_prefix'""" ! Experimental
$ write config_status_file "$ bindir_dev=""''bindir_dev'""" ! Experimental
$ write config_status_file "$ bindir_dir=""''bindir_dir'""" ! Experimental
$ write config_status_file "$ bindir=""''bindir'""" ! Experimental
$ write config_status_file "$ datadir_dev=""''datadir_dev'""" ! Experimental
$ write config_status_file "$ datadir_dir=""''datadir_dir'""" ! Experimental
$ write config_status_file "$ datadir=""''datadir'""" ! Experimental
$ write config_status_file "$ statedir_dev=""''statedir_dev'""" ! Experimental
$ write config_status_file "$ statedir_dir=""''statedir_dir'""" ! Experimental
$ write config_status_file "$ statedir=""''statedir'""" ! Experimental
$ write config_status_file "$ libdir_dev=""''libdir_dev'""" ! Experimental
$ write config_status_file "$ libdir_dir=""''libdir_dir'""" ! Experimental
$ write config_status_file "$ libdir=""''libdir'""" ! Experimental
$ write config_status_file "$ mandir_dev=""''mandir_dev'""" ! Experimental
$ write config_status_file "$ mandir_dir=""''mandir_dir'""" ! Experimental
$ write config_status_file "$ mandir=""''mandir'""" ! Experimental
$ write config_status_file "$ infodir_dev=""''infodir_dev'""" ! Experimental
$ write config_status_file "$ infodir_dir=""''infodir_dir'""" ! Experimental
$ write config_status_file "$ infodir=""''infodir'""" ! Experimental
$ write config_status_file "$ lispdir_dev=""''lispdir_dev'""" ! Experimental
$ write config_status_file "$ lispdir_dir=""''lispdir_dir'""" ! Experimental
$ write config_status_file "$ lispdir_file=""''lispdir_file'""" ! Experimental
$ write config_status_file "$ lispdir=""''lispdir'""" ! Experimental
$ write config_status_file "$ locallisppath=""''locallisppath'""" ! Experimental
$ write config_status_file "$ lisppath=""''lisppath'""" ! Experimental
$ write config_status_file "$ etcdir_dev=""''etcdir_dev'""" ! Experimental
$ write config_status_file "$ etcdir_dir=""''etcdir_dir'""" ! Experimental
$ write config_status_file "$ etcdir_file=""''etcdir_file'""" ! Experimental
$ write config_status_file "$ etcdir=""''etcdir'""" ! Experimental
$ write config_status_file "$ lockdir_dev=""''lockdir_dev'""" ! Experimental
$ write config_status_file "$ lockdir_dir=""''lockdir_dir'""" ! Experimental
$ write config_status_file "$ lockdir_file=""''lockdir_file'""" ! Experimental
$ write config_status_file "$ lockdir=""''lockdir'""" ! Experimental
$ write config_status_file "$ archlibdir_dev=""''archlibdir_dev'""" ! Experimental
$ write config_status_file "$ archlibdir_dir=""''archlibdir_dir'""" ! Experimental
$ write config_status_file "$ archlibdir=""''archlibdir'""" ! Experimental
$ write config_status_file "$ docdir_dev=""''docdir_dev'""" ! Experimental
$ write config_status_file "$ docdir_dir=""''docdir_dir'""" ! Experimental
$ write config_status_file "$ docdir=""''docdir'""" ! Experimental
$ write config_status_file "$ vmslibdir_dev=""''vmslibdir_dev'""" ! Experimental
$ write config_status_file "$ vmslibdir_dir=""''vmslibdir_dir'""" ! Experimental
$ write config_status_file "$ vmslibdir=""''vmslibdir'""" ! Experimental
$ write config_status_file "$ machfile=""''machfile'""" ! Experimental
$ write config_status_file "$ machfile_dir=""''machfile_dir'""" ! Experimental
$ write config_status_file "$ opsysfile=""''opsysfile'""" ! Experimental
$ write config_status_file "$ opsysfile_dir=""''opsysfile_dir'""" ! Experimental
$ write config_status_file "$ CPPFLAGS=""''CPPFLAGS'""" ! Experimental
$ write config_status_file "$ LIBOBJS=""''LIBOBJS'""" ! Experimental
$ write config_status_file "$ EXEEXT=""''EXEEXT'""" ! Experimental
$ write config_status_file "$ LIBSOUND=""''LIBSOUND'""" ! Experimental
$ write config_status_file "$ GETLOADAVG_LIBS=""''GETLOADAVG_LIBS'""" ! Experimental
$ write config_status_file "$ top_srcdir_dev=""''top_srcdir_dev'""" ! Experimental
$ write config_status_file "$ top_srcdir_dir=""''top_srcdir_dir'""" ! Experimental
$ write config_status_file "$ top_srcdir=""''top_srcdir'""" ! Experimental
$ ac_i = 0
$ac_prsub_loop:
$ ac_i = ac_i + 1
$ if f$type(ac_prsub'ac_i') .nes. ""
$  then"
$   if f$type(ac_prsub_support'ac_i') .nes. ""
$    then
$     ac_tmp = ac_prsub_support'ac_i'
$     write config_status_file "$   ",ac_tmp
$    endif
$   ac_tmp = ac_prsub'ac_i'
$   write config_status_file "$   ac_prsub''ac_i'=""",ac_tmp,""""
$   goto ac_prsub_loop
$  endif
$
$ ac_i = 0
$ac_vpsub_loop:
$ ac_i = ac_i + 1
$ if f$type(ac_vpsub'ac_i') .nes. ""
$  then"
$   if f$type(ac_vpsub_support'ac_i') .nes. ""
$    then
$     ac_tmp = ac_vpsub_support'ac_i'
$     write config_status_file "$   ",ac_tmp
$    endif
$   ac_tmp = ac_vpsub'ac_i'
$   write config_status_file "$   ac_vpsub''ac_i'=""",ac_tmp,""""
$   goto ac_vpsub_loop
$  endif
$
$ ac_i = 0
$ac_extrasub_loop:
$ ac_i = ac_i + 1
$ if f$type(ac_extrasub'ac_i') .nes. ""
$  then"
$   ac_tmp = ac_extrasub'ac_i'
$   write config_status_file "$   ac_extrasub''ac_i'=""''ac_tmp'"""
$   goto ac_extrasub_loop
$  endif
$ write config_status_file "$ LDFLAGS := 'LDFLAGS'"
$ write config_status_file "$ CC := 'CC'"
$ write config_status_file "$ sys_includes := 'sys_includes'"
$ write config_status_file "$ OPTS := 'OPTS'"
$ write config_status_file "$ LIBS := 'LIBS'"
$ write config_status_file "$ CFLAGS := 'CFLAGS'"
$ write config_status_file "$ LN_S := 'LN_S'"
$ write config_status_file "$ CPP := 'CPP'"
$ write config_status_file "$ INSTALL := 'INSTALL'"
$ write config_status_file "$ INSTALL_PROGRAM := 'INSTALL_PROGRAM'"
$ write config_status_file "$ INSTALL_DATA := 'INSTALL_DATA'"
$ write config_status_file "$ INSTALL_QUOTED := 'INSTALL_QUOTED'"
$ write config_status_file "$ INSTALL_PROGRAM_QUOTED := 'INSTALL_PROGRAM_QUOTED'"
$ write config_status_file "$ INSTALL_DATA_QUOTED := 'INSTALL_DATA_QUOTED'"
$ write config_status_file "$ SET_MAKE := 'SET_MAKE'"
$ write config_status_file "$ netinet_includes := 'netinet_includes'"
$ write config_status_file "$ arpa_includes := 'arpa_includes'"
$ write config_status_file "$ x_includes := 'x_includes'"
$ write config_status_file "$ ALLOCA := 'ALLOCA'"
$ write config_status_file "$ version := 'version'"
$ write config_status_file "$ version_us := 'version_us'"
$ write config_status_file "$ configuration := 'configuration'"
$ write config_status_file "$ configuration_us := 'configuration_us'"
$ write config_status_file "$ srcdir_dev := 'srcdir_dev'"
$ write config_status_file "$ srcdir_dir := 'srcdir_dir'"
$ write config_status_file "$ srcdir := 'srcdir'"
$ write config_status_file "$ libsrc_libs := 'libsrc_libs'"
$ write config_status_file "$ LD_SWITCH_X_SITE := 'LD_SWITCH_X_SITE'"
$ write config_status_file "$ C_SWITCH_MACHINE := 'C_SWITCH_MACHINE'"
$ write config_status_file "$ C_SWITCH_X_MACHINE := 'C_SWITCH_X_MACHINE'"
$ write config_status_file "$ C_SWITCH_SYSTEM := 'C_SWITCH_SYSTEM'"
$ write config_status_file "$ C_SWITCH_X_SYSTEM := 'C_SWITCH_X_SYSTEM'"
$ write config_status_file "$ C_SWITCH_SITE := 'C_SWITCH_SITE'"
$ write config_status_file "$ C_SWITCH_X_SITE := 'C_SWITCH_X_SITE'"
$ write config_status_file "$ C_DEBUG_SWITCH := 'C_DEBUG_SWITCH'"
$ write config_status_file "$ X_TOOLKIT_TYPE := 'X_TOOLKIT_TYPE'"
$ write config_status_file "$ startupdir_dev := 'startupdir_dev'"
$ write config_status_file "$ startupdir_dir := 'startupdir_dir'"
$ write config_status_file "$ startupdir := 'startupdir'"
$ write config_status_file "$ vuelibdir_dev := 'vuelibdir_dev'"
$ write config_status_file "$ vuelibdir_dir := 'vuelibdir_dir'"
$ write config_status_file "$ vuelibdir := 'vuelibdir'"
$ write config_status_file "$ prefix_dev := 'prefix_dev'"
$ write config_status_file "$ prefix_dir := 'prefix_dir'"
$ write config_status_file "$ prefix := 'prefix'"
$ write config_status_file "$ exec_prefix_dev := 'exec_prefix_dev'"
$ write config_status_file "$ exec_prefix_dir := 'exec_prefix_dir'"
$ write config_status_file "$ exec_prefix := 'exec_prefix'"
$ write config_status_file "$ bindir_dev := 'bindir_dev'"
$ write config_status_file "$ bindir_dir := 'bindir_dir'"
$ write config_status_file "$ bindir := 'bindir'"
$ write config_status_file "$ datadir_dev := 'datadir_dev'"
$ write config_status_file "$ datadir_dir := 'datadir_dir'"
$ write config_status_file "$ datadir := 'datadir'"
$ write config_status_file "$ statedir_dev := 'statedir_dev'"
$ write config_status_file "$ statedir_dir := 'statedir_dir'"
$ write config_status_file "$ statedir := 'statedir'"
$ write config_status_file "$ libdir_dev := 'libdir_dev'"
$ write config_status_file "$ libdir_dir := 'libdir_dir'"
$ write config_status_file "$ libdir := 'libdir'"
$ write config_status_file "$ mandir_dev := 'mandir_dev'"
$ write config_status_file "$ mandir_dir := 'mandir_dir'"
$ write config_status_file "$ mandir := 'mandir'"
$ write config_status_file "$ infodir_dev := 'infodir_dev'"
$ write config_status_file "$ infodir_dir := 'infodir_dir'"
$ write config_status_file "$ infodir := 'infodir'"
$ write config_status_file "$ lispdir_dev := 'lispdir_dev'"
$ write config_status_file "$ lispdir_dir := 'lispdir_dir'"
$ write config_status_file "$ lispdir_file := 'lispdir_file'"
$ write config_status_file "$ lispdir := 'lispdir'"
$ write config_status_file "$ locallisppath := 'locallisppath'"
$ write config_status_file "$ lisppath := 'lisppath'"
$ write config_status_file "$ etcdir_dev := 'etcdir_dev'"
$ write config_status_file "$ etcdir_dir := 'etcdir_dir'"
$ write config_status_file "$ etcdir_file := 'etcdir_file'"
$ write config_status_file "$ etcdir := 'etcdir'"
$ write config_status_file "$ lockdir_dev := 'lockdir_dev'"
$ write config_status_file "$ lockdir_dir := 'lockdir_dir'"
$ write config_status_file "$ lockdir_file := 'lockdir_file'"
$ write config_status_file "$ lockdir := 'lockdir'"
$ write config_status_file "$ archlibdir_dev := 'archlibdir_dev'"
$ write config_status_file "$ archlibdir_dir := 'archlibdir_dir'"
$ write config_status_file "$ archlibdir := 'archlibdir'"
$ write config_status_file "$ docdir_dev := 'docdir_dev'"
$ write config_status_file "$ docdir_dir := 'docdir_dir'"
$ write config_status_file "$ docdir := 'docdir'"
$ write config_status_file "$ vmslibdir_dev := 'vmslibdir_dev'"
$ write config_status_file "$ vmslibdir_dir := 'vmslibdir_dir'"
$ write config_status_file "$ vmslibdir := 'vmslibdir'"
$ write config_status_file "$ machfile := 'machfile'"
$ write config_status_file "$ machfile_dir := 'machfile_dir'"
$ write config_status_file "$ opsysfile := 'opsysfile'"
$ write config_status_file "$ opsysfile_dir := 'opsysfile_dir'"
$ write config_status_file "$ CPPFLAGS := 'CPPFLAGS'"
$ write config_status_file "$ LIBOBJS := 'LIBOBJS'"
$ write config_status_file "$ EXEEXT := 'EXEEXT'"
$ write config_status_file "$ LIBSOUND := 'LIBSOUND'"
$ write config_status_file "$ GETLOADAVG_LIBS := 'GETLOADAVG_LIBS'"
$ write config_status_file "$ top_srcdir_dev := 'top_srcdir_dev'"
$ write config_status_file "$ top_srcdir_dir := 'top_srcdir_dir'"
$ write config_status_file "$ top_srcdir := 'top_srcdir'"
$ write config_status_file "$"
$ write config_status_file "$ ac_given_srcdir=srcdir"
$ write config_status_file "$ ac_given_srcdir_dev=srcdir_dev"
$ write config_status_file "$ ac_given_srcdir_dir=srcdir_dir"
$ write config_status_file "$"
$ write config_status_file "$ if f$type(CONFIG_FILES) .eqs. """" then CONFIG_FILES = ""[-]"" +-"
$ write config_status_file "	"" descrip.mms [.lib-src]descrip.mms_in [.oldXMenu]descrip.mms"" +-"
$ write config_status_file "	"" [.lwlib]descrip.mms [.src]descrip.mms_in [.vms]descrip.mms"" +-"
$ write config_status_file "	"" [.vms]emacs_vue.com"" +-"
$ write config_status_file "	"""""
$ write config_status_file "$! CONFIG_FILE_i = -1"
$ write config_status_file "$ CONFIG_FILE_i = 0"
$ write config_status_file "$config_status_loop3:"
$ write config_status_file "$! if CONFIG_FILE_i .eq. -1"
$ write config_status_file "$!  then"
$ write config_status_file "$!   ac_file = ""[-]"""
$ write config_status_file "$!  else"
$ write config_status_file "$   ac_file = f$element(CONFIG_FILE_i,"" "",CONFIG_FILES)"
$ write config_status_file "$!  endif"
$ write config_status_file "$ dummy = ""'"+"'ac_file'""" ! debug
$ write config_status_file "$ CONFIG_FILE_i = CONFIG_FILE_i + 1"
$ write config_status_file "$ if ac_file .eqs. ""[-]"" then goto config_status_loop3"
$ write config_status_file "$ if ac_file .nes. "" """
$ write config_status_file "$  then"
$ write config_status_file "$   ac_dir:="
$ write config_status_file "$   ac_dir_tmp = ac_file"
$ write config_status_file "$  ac_loop_dir1:"
$ write config_status_file "$   ac_dir_e = f$element(0,""]"",ac_dir_tmp)"
$ write config_status_file "$   if ac_dir_e .eqs. ac_dir_tmp then goto ac_loop_dir2"
$ write config_status_file "$   ac_dir_e = ac_dir_e + ""]"""
$ write config_status_file "$   ac_dir = ac_dir + ac_dir_e"
$ write config_status_file "$   ac_dir_tmp = ac_dir_tmp - ac_dir_e"
$ write config_status_file "$   goto ac_loop_dir1
$ write config_status_file "$  ac_loop_dir2:"
$ write config_status_file "$   ac_dir_e = f$element(0,"">"",ac_dir_tmp)"
$ write config_status_file "$   if ac_dir_e .eqs. ac_dir_tmp then goto ac_end_dir"
$ write config_status_file "$   ac_dir_e = ac_dir_e + "">"""
$ write config_status_file "$   ac_dir = ac_dir + ac_dir_e"
$ write config_status_file "$   ac_dir_tmp = ac_dir_tmp - ac_dir_e"
$ write config_status_file "$   goto ac_loop_dir2"
$ write config_status_file "$  ac_end_dir:"
$ write config_status_file "$   if ac_dir .nes. ac_file .and. ac_dir .nes. """" -"
$ write config_status_file "	.and. ac_dir .nes. ""[]"""
$ write config_status_file "$    then"
$ write config_status_file "$     !# The file is in a subdirectory"
$ write config_status_file "$     set noon"
$ write config_status_file "$     if f$parse(ac_dir) .eqs. """" then create/dir 'ac_dir'"
$ write config_status_file "$     set on"
$ write config_status_file "$     ac_dir_tmp = ac_dir - ""[."" - ""<."" - ""["" - ""<"" - ""]"" - "">"""
$ write config_status_file "$     ac_dir_suffix:=.'ac_dir_tmp'"
$ write config_status_file "$    else"
$ write config_status_file "$     ac_dir_suffix:="
$ write config_status_file "$    endif"
$ write config_status_file "$"
$ write config_status_file "$   ac_dashes:="
$ write config_status_file "$   ac_dir_tmp=ac_dir_suffix"
$ write config_status_file "$  ac_loop_dashes:"
$ write config_status_file "$   ac_dir_e = f$element(1,""."",ac_dir_tmp)"
$ write config_status_file "$   if ac_dir_e .eqs. ""."" then goto ac_end_dashes"
$ write config_status_file "$   ac_dashes = ac_dashes + ""-"""
$ write config_status_file "$   ac_dir_e = "".""+ac_dir_e"
$ write config_status_file "$   ac_dir_tmp = ac_dir_tmp - ac_dir_e"
$ write config_status_file "$   goto ac_loop_dashes"
$ write config_status_file "$  ac_end_dashes:"
$ write config_status_file "$   if ac_given_srcdir .eqs. ""[]"""
$ write config_status_file "$    then"
$ write config_status_file "$     srcdir := []"
$ write config_status_file "$     srcdir_dev :="
$ write config_status_file "$     srcdir_dir :="
$ write config_status_file "$     if ac_dir_suffix .eqs. """"
$ write config_status_file "$      then"
$ write config_status_file "$       top_srcdir := []"
$ write config_status_file "$       top_srcdir_dev :="
$ write config_status_file "$       top_srcdir_dir :="
$ write config_status_file "$      else"
$ write config_status_file "$       top_srcdir := ['ac_dashes']"
$ write config_status_file "$       top_srcdir_dev :="
$ write config_status_file "$       top_srcdir_dir = ac_dashes"
$ write config_status_file "$      endif"
$ write config_status_file "$    else"
$ write config_status_file "$     if f$extract(0,2,ac_given_srcdir_dir) .eqs. ""[."""
$ write config_status_file "$      then !# Relative path."
$ write config_status_file "$       if ac_dashes .nes. """" then ac_dashes = ac_dashes + ""."""
$ write config_status_file "$       srcdir_dir := 'ac_dashes'"+"'ac_given_srcdir_dir'"+"'ac_dir_suffix'"
$ write config_status_file "$       srcdir_dev := 'ac_given_srcdir_dev'"
$ write config_status_file "$       srcdir := 'srcdir_dev'['srcdir_dir']"
$ write config_status_file "$       top_srcdir_dev := 'ac_given_srcdir_dev'"
$ write config_status_file "$       top_srcdir_dir := 'ac_dashes'"+"'ac_given_srcdir_dir'"
$ write config_status_file "$       top_srcdir := 'top_srcdir_dev'['top_srcdir_dir']"
$ write config_status_file "$      else"
$ write config_status_file "$       srcdir_dir := 'ac_given_srcdir_dir'"+"'ac_dir_suffix'"
$ write config_status_file "$       srcdir_dev := 'ac_given_srcdir_dev'"
$ write config_status_file "$       srcdir := 'srcdir_dev'['srcdir_dir']"
$ write config_status_file "$       top_srcdir_dev := 'ac_given_srcdir_dev'"
$ write config_status_file "$       top_srcdir_dir := 'ac_given_srcdir_dir'"
$ write config_status_file "$       top_srcdir := 'top_srcdir_dev'['top_srcdir_dir']"
$ write config_status_file "$      endif"
$ write config_status_file "$    endif"
$ write config_status_file "$"
$ write config_status_file "$   write sys$output ""creating '"+"'ac_file'"""
$ write config_status_file "$   set noon"
$ write config_status_file "$   ac_tmp = f$parse(ac_file,""*.*;*"")"
$ write config_status_file "$!   if f$search(ac_tmp) .nes. """" then delete/nolog 'ac_tmp'"
$ write config_status_file "$   ac_input_file_type = f$parse(ac_file,,,""TYPE"")"
$ write config_status_file "$   ac_input_file := 'f$element(0,"";"",ac_file)'"
$ write config_status_file "$   ac_input_file_len = f$length(ac_input_file)-f$length(ac_input_file_type)"
$ write config_status_file "$   !sh sym ac_input_file*"
$ write config_status_file "$   if f$extract(ac_input_file_len,f$length(ac_input_file_type),ac_input_file) .nes. ac_input_file_type then ac_input_file_len = f$length(ac_input_file)"
$ write config_status_file "$   ac_input_file = f$extract(0,ac_input_file_len,ac_input_file)"
$ write config_status_file "$   ac_input_file_dir = f$extract(0,ac_input_file_len-f$length(f$parse(ac_input_file,,,""NAME"")),ac_input_file)"
$ write config_status_file "$   ac_input_file = ac_input_file-ac_input_file_dir"
$ write config_status_file "$   !sh sym ac_input_file*"
$ write config_status_file "$   !set verify"
$ write config_status_file "$   comment_str=""Generated automatically from ""+ac_input_file+ac_input_file_type+"" by configure."""
$ write config_status_file "$   open/write config_status_dest 'ac_file'"
$ write config_status_file "$   if ac_input_file_type .eqs. "".C"" -"
$ write config_status_file "      .or. ac_input_file_type .eqs. "".H"" -"
$ write config_status_file "      .or. ac_input_file_type .eqs. "".CC"" -"
$ write config_status_file "      .or. ac_input_file_type .eqs. "".MAR"""
$ write config_status_file "$    then"
$ write config_status_file "$     write config_status_dest ""/* "",comment_str,"" */"""
$ write config_status_file "$    else"
$ write config_status_file "$     if ac_input_file_type .eqs. "".COM"""
$ write config_status_file "$      then"
$ write config_status_file "$       write config_status_dest ""$! "",comment_str"
$ write config_status_file "$      else
$ write config_status_file "$       write config_status_dest ""# "",comment_str"
$ write config_status_file "$      endif"
$ write config_status_file "$    endif"
$ write config_status_file "$
$ write config_status_file "$   open/write config_status_tpu conftest.tpu"
$ write config_status_file "$   type sys$input:/out=config_status_tpu"
$ write config_status_file "procedure TPU_substitute(pat,value)"
$ write config_status_file "	local lr;"
$ write config_status_file "	position (beginning_of (main_buffer));"
$ write config_status_file "	loop"
$ write config_status_file "		lr := search_quietly (pat, FORWARD);"
$ write config_status_file "		EXITIF lr = 0;"
$ write config_status_file "		erase (lr);"
$ write config_status_file "		position (lr);"
$ write config_status_file "		copy_text (value);"
$ write config_status_file "		position (end_of (lr));"
$ write config_status_file "	endloop;"
$ write config_status_file "endprocedure;"
$ write config_status_file "! This is the main thing."
$ write config_status_file "input_file := GET_INFO (COMMAND_LINE, ""file_name"");"
$ write config_status_file "main_buffer := CREATE_BUFFER (""main"", input_file);"
$ write config_status_file "p_0_or_more_spc := (span("" ""+ASCII(9)) | """");"
$ write config_status_file "position (beginning_of (main_buffer));"
$ write config_status_file "! Here it is time to put the calls."
$ write config_status_file "$   !set noverify"
$ write config_status_file "$   ac_i = 0"
$ write config_status_file "$ac_prsub_write_loop:"
$ write config_status_file "$   ac_i = ac_i + 1"
$ write config_status_file "$   if f$type(ac_prsub'ac_i') .nes. ""
$ write config_status_file "$    then
$ write config_status_file "$     ac_tmp = ac_prsub'ac_i'"
$ write config_status_file "$     write config_status_tpu ac_tmp"
$ write config_status_file "$     goto ac_prsub_write_loop"
$ write config_status_file "$    endif"
$ write config_status_file "$   ac_i = 0"
$ write config_status_file "$ac_vpsub_write_loop:"
$ write config_status_file "$   ac_i = ac_i + 1"
$ write config_status_file "$   if f$type(ac_vpsub'ac_i') .nes. ""
$ write config_status_file "$    then
$ write config_status_file "$     ac_tmp = ac_vpsub'ac_i'"
$ write config_status_file "$     write config_status_tpu ac_tmp"
$ write config_status_file "$     goto ac_vpsub_write_loop"
$ write config_status_file "$    endif"
$ write config_status_file "$   ac_i = 0"
$ write config_status_file "$ac_extrasub_write_loop:"
$ write config_status_file "$   ac_i = ac_i + 1"
$ write config_status_file "$   if f$type(ac_extrasub'ac_i') .nes. ""
$ write config_status_file "$    then
$ write config_status_file "$     ac_tmp = ac_extrasub'ac_i'"
$ write config_status_file "$     write config_status_tpu ac_tmp"
$ write config_status_file "$     goto ac_extrasub_write_loop"
$ write config_status_file "$    endif"
$ write config_status_file "$ __config_status_tmp := 'LDFLAGS'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""LDFLAGS""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'CC'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""CC""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'sys_includes'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""sys_includes""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'OPTS'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""OPTS""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'LIBS'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""LIBS""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'CFLAGS'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""CFLAGS""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'LN_S'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""LN_S""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'CPP'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""CPP""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'INSTALL'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""INSTALL""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'INSTALL_PROGRAM'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""INSTALL_PROGRAM""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'INSTALL_DATA'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""INSTALL_DATA""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'INSTALL_QUOTED'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""INSTALL_QUOTED""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'INSTALL_PROGRAM_QUOTED'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""INSTALL_PROGRAM_QUOTED""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'INSTALL_DATA_QUOTED'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""INSTALL_DATA_QUOTED""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'SET_MAKE'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""SET_MAKE""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'netinet_includes'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""netinet_includes""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'arpa_includes'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""arpa_includes""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'x_includes'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""x_includes""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'ALLOCA'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""ALLOCA""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'version'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""version""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'version_us'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""version_us""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'configuration'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""configuration""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'configuration_us'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""configuration_us""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'srcdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""srcdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'srcdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""srcdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'srcdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""srcdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'libsrc_libs'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""libsrc_libs""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'LD_SWITCH_X_SITE'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""LD_SWITCH_X_SITE""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'C_SWITCH_MACHINE'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""C_SWITCH_MACHINE""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'C_SWITCH_X_MACHINE'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""C_SWITCH_X_MACHINE""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'C_SWITCH_SYSTEM'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""C_SWITCH_SYSTEM""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'C_SWITCH_X_SYSTEM'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""C_SWITCH_X_SYSTEM""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'C_SWITCH_SITE'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""C_SWITCH_SITE""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'C_SWITCH_X_SITE'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""C_SWITCH_X_SITE""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'C_DEBUG_SWITCH'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""C_DEBUG_SWITCH""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'X_TOOLKIT_TYPE'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""X_TOOLKIT_TYPE""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'startupdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""startupdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'startupdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""startupdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'startupdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""startupdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'vuelibdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""vuelibdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'vuelibdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""vuelibdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'vuelibdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""vuelibdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'prefix_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""prefix_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'prefix_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""prefix_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'prefix'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""prefix""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'exec_prefix_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""exec_prefix_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'exec_prefix_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""exec_prefix_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'exec_prefix'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""exec_prefix""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'bindir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""bindir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'bindir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""bindir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'bindir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""bindir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'datadir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""datadir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'datadir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""datadir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'datadir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""datadir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'statedir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""statedir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'statedir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""statedir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'statedir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""statedir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'libdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""libdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'libdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""libdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'libdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""libdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'mandir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""mandir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'mandir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""mandir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'mandir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""mandir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'infodir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""infodir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'infodir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""infodir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'infodir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""infodir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'lispdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""lispdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'lispdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""lispdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'lispdir_file'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""lispdir_file""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'lispdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""lispdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'locallisppath'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""locallisppath""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'lisppath'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""lisppath""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'etcdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""etcdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'etcdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""etcdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'etcdir_file'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""etcdir_file""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'etcdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""etcdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'lockdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""lockdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'lockdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""lockdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'lockdir_file'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""lockdir_file""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'lockdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""lockdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'archlibdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""archlibdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'archlibdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""archlibdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'archlibdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""archlibdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'docdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""docdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'docdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""docdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'docdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""docdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'vmslibdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""vmslibdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'vmslibdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""vmslibdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'vmslibdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""vmslibdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'machfile'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""machfile""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'machfile_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""machfile_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'opsysfile'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""opsysfile""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'opsysfile_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""opsysfile_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'CPPFLAGS'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""CPPFLAGS""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'LIBOBJS'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""LIBOBJS""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'EXEEXT'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""EXEEXT""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'LIBSOUND'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""LIBSOUND""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'GETLOADAVG_LIBS'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""GETLOADAVG_LIBS""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'top_srcdir_dev'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""top_srcdir_dev""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'top_srcdir_dir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""top_srcdir_dir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ __config_status_tmp := 'top_srcdir'"
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@""""+""""top_srcdir""""+""""@"""","""""",__config_status_tmp,"""""");""" ! Experimental
$ write config_status_file "$ write config_status_tpu ""TPU_substitute(""""@DEFS@"""",""""""""""""HAVE_CONFIG_H"""""""""""");"""
$ write config_status_file "$   !set verify"
$ write config_status_file "$   type sys$input:/out=config_status_tpu"
$ write config_status_file "! Now let's save it all"
$ write config_status_file "WRITE_FILE(main_buffer, GET_INFO(COMMAND_LINE, ""output_file""));"
$ write config_status_file "quit;"
$ write config_status_file "$   close config_status_tpu"
$ write config_status_file "$   if ac_input_file_type .eqs. ""."""
$ write config_status_file "$    then ac_input_file = ac_input_file + "".IN"""
$ write config_status_file "$    else ac_input_file = ac_input_file + ac_input_file_type + ""_IN"""
$ write config_status_file "$    endif"
$ write config_status_file "$   save_def = f$environment(""DEFAULT"")"
$ write config_status_file "$   if ac_given_srcdir .nes. """" then set default 'ac_given_srcdir'"
$ write config_status_file "$   if ac_input_file_dir .nes. """" then set default 'ac_input_file_dir'"
$ write config_status_file "$   ac_input_file_dir = f$environment(""DEFAULT"")"
$ write config_status_file "$   set default 'save_def'
$ write config_status_file "$ __debug = f$trnlnm(""DEBUG_CONFIGURE"")"
$ write config_status_file "$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. __debug then set message/nofacility/noident/noseverity/notext"
$ write config_status_file "$ if .not. __debug then def/user sys$output nl:"
$ write config_status_file "$ if .not. __debug then def/user sys$error nl:"
$ write config_status_file "$   edit/tpu/nosection/command=conftest.tpu/nodisplay -"
$ write config_status_file "	'ac_input_file_dir'"+"'ac_input_file' /out=config_status_dest"
$ write config_status_file "$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'"
$ write config_status_file "$"
$ write config_status_file "$   close config_status_dest"
$ write config_status_file "$   set noon"
$ write config_status_file "$   if f$search(""conftest.tpu"") then delete/nolog conftest.tpu.*"
$ write config_status_file "$! Description files are often check for dependencies, so don't check
$ write config_status_file "$! if they have changed.  Scripts on the ofter hand...
$ write config_status_file "$!$   if f$parse(file,,,""TYPE"") .eqs. "".COM"" -"
$ write config_status_file "$!	.and. f$search(file) .nes. """" then -"
$ write config_status_file "$!	diff conftest.out 'file'/out=nla0:"
$ write config_status_file "$!$   if $status .eq. %X06c8009 ! code for no change"
$ write config_status_file "$!$    then"
$ write config_status_file "$!$     ! The file exists and we would not be changing it."
$ write config_status_file "$!$     write sys$output ""'"+"'file' is unchanged."""
$ write config_status_file "$!$    else"
$ write config_status_file "$!$     copy conftest.out 'file'"
$ write config_status_file "$!$     purge/nolog 'file'"
$ write config_status_file "$!$    endif"
$ write config_status_file "$!$   if f$search(""conftest.out.*"") .nes. """" then delete/nolog conftest.out.*"
$ write config_status_file "$   set on"
$ write config_status_file "$   goto config_status_loop3"
$ write config_status_file "$  endif"
$ write config_status_file "$ if f$search(""conftest.tpu"") .nes. """" then delete/nolog conftest.tpu.*"
$ write config_status_file "$ type sys$input: /out=conftest.tpu"
$ write config_status_file "! ac_d sets the value in ""#define NAME VALUE"" lines."
$ write config_status_file "! ac_u turns ""#undef NAME"" with trailing blanks into ""#define NAME VALUE""."
$ write config_status_file "! ac_e turns ""#undef NAME"" without trailing blanks into ""#define NAME VALUE""."
$ write config_status_file ""
$ write config_status_file "procedure ac_d(nam,value)"
$ write config_status_file "	local p0, r, r0, r1, r2, r3, res;"
$ write config_status_file "	position (beginning_of (main_buffer));"
$ write config_status_file "	p0 := line_begin + (((((p_empty_or_spaces @ r1) + ""#"" + (p_empty_or_spaces @ r2)) @ r10) + ""def"+"ine"" + (p_spaces @ r3) + nam + (p_spaces @ r4) + p_nospaces) @ r0) + line_end;"
$ write config_status_file "	loop"
$ write config_status_file "		r := search_quietly (p0, FORWARD);"
$ write config_status_file "		EXITIF r = 0;"
$ write config_status_file "		res := sub"+"str (r10, 1) + ""def"+"ine"" + sub"+"str (r3, 1) + nam + sub"+"str(r4, 1) + value;"
$ write config_status_file "		erase (r0);"
$ write config_status_file "		position (r0);"
$ write config_status_file "		copy_text (res);"
$ write config_status_file "		position (end_of (r0));"
$ write config_status_file "	endloop;"
$ write config_status_file "endprocedure;"
$ write config_status_file ""
$ write config_status_file "procedure ac_u(nam,value)"
$ write config_status_file "	local p0, r, r0, r1, r2, r3, res;"
$ write config_status_file "	position (beginning_of (main_buffer));"
$ write config_status_file "	p0 := line_begin + (((((p_empty_or_spaces @ r1) + ""#"" + (p_empty_or_spaces @ r2)) @ r10) + ""undef"" + (p_spaces @ r3) + nam + (p_space @ r4)) @ r0);"
$ write config_status_file "	loop"
$ write config_status_file "		r := search_quietly (p0, FORWARD);"
$ write config_status_file "		EXITIF r = 0;"
$ write config_status_file "		res := sub"+"str (r10, 1) + ""def"+"ine"" + sub"+"str (r3, 1) + nam + "" "" + value + sub"+"str (r4, 1);"
$ write config_status_file "		erase (r0);"
$ write config_status_file "		position (r0);"
$ write config_status_file "		copy_text (res);"
$ write config_status_file "		position (end_of (r0));"
$ write config_status_file "	endloop;"
$ write config_status_file "endprocedure;"
$ write config_status_file ""
$ write config_status_file "procedure ac_e(nam,value)"
$ write config_status_file "	local p0, r, r0, r1, r2, r3, res;"
$ write config_status_file "	position (beginning_of (main_buffer));"
$ write config_status_file "	p0 := line_begin + (((((p_empty_or_spaces @ r1) + ""#"" + (p_empty_or_spaces @ r2)) @ r10) + ""undef"" + (p_spaces @ r3) + nam) @ r0) + line_end;"
$ write config_status_file "	loop"
$ write config_status_file "		r := search_quietly (p0, FORWARD);"
$ write config_status_file "		EXITIF r = 0;"
$ write config_status_file "		res := sub"+"str (r10, 1) + ""def"+"ine"" + sub"+"str (r3, 1) + nam + "" "" + value;"
$ write config_status_file "		erase (r0);"
$ write config_status_file "		position (r0);"
$ write config_status_file "		copy_text (res);"
$ write config_status_file "		position (end_of (r0));"
$ write config_status_file "	endloop;"
$ write config_status_file "endprocedure;"
$ write config_status_file ""
$ write config_status_file "procedure comment_undefs()"
$ write config_status_file "	local p0, r, r0, res;"
$ write config_status_file "	position (beginning_of (main_buffer));"
$ write config_status_file "	p0 := line_begin + ((p_empty_or_spaces + ""#"" + p_empty_or_spaces + ""undef"" + p_spaces + p_any_name) @ r0);"
$ write config_status_file "	loop"
$ write config_status_file "		r := search_quietly (p0, FORWARD);"
$ write config_status_file "		EXITIF r = 0;"
$ write config_status_file "		res := ""/* "" + sub"+"str (r0, 1) + "" */"";"
$ write config_status_file "		erase (r0);"
$ write config_status_file "		position (r0);"
$ write config_status_file "		copy_text (res);"
$ write config_status_file "		position (end_of (r0));"
$ write config_status_file "	endloop;"
$ write config_status_file "endprocedure;"
$ write config_status_file ""
$ write config_status_file "p_empty_or_spaces := (span ("" "" + ascii (9)) | """");"
$ write config_status_file "p_before_keyword := (span ("" "" + ascii (9)) | """") + ""#"" + (span ("" "" + ascii (9)) | """");"
$ write config_status_file "p_space := ("" "" | ascii (9));"
$ write config_status_file "p_spaces := span ("" "" + ascii (9));"
$ write config_status_file "p_nospaces := scan ("" "" + ascii (9));"
$ write config_status_file "p_any_name := any(""abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"") +"
$ write config_status_file "	span(""abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789"");"
$ write config_status_file ""
$ write config_status_file "! This is the main thing."
$ write config_status_file "ac_input_file := GET_INFO (COMMAND_LINE, ""file_name"");"
$ write config_status_file "main_buffer := CREATE_BUFFER (""main"", ac_input_file);"
$ write config_status_file "position (beginning_of (main_buffer));"
$ write config_status_file "! Here it is time to put the calls."
$ ac_i = 0
$ac_sed_defs_write_loop:
$ ac_i = ac_i + 1
$ if f$type(ac_sed_defs'ac_i') .nes. ""
$  then"
$   write config_status_file ac_sed_defs'ac_i'
$   goto ac_sed_defs_write_loop
$  endif
$ write config_status_file "!# This command replaces #undef's with comments.  This is necessary, for"
$ write config_status_file "!# example, in the case of _POSIX_SOURCE, which is predefined and required"
$ write config_status_file "!# on some systems where configure will not decide to define it in"
$ write config_status_file "!#"+-
" [[.src]config.h]."
$ write config_status_file "comment_undefs();"
$ write config_status_file "!# Now let's save it all"
$ write config_status_file "WRITE_FILE(main_buffer, GET_INFO(COMMAND_LINE, ""output_file""));"
$ write config_status_file "quit;"
$ write config_status_file "$!"
$ write config_status_file "$ if f$type(CONFIG_HEADERS) .eqs. """" then CONFIG_HEADERS=""[.src]config.h"""
$ write config_status_file "$ CONFIG_HEADER_i = 0"
$ write config_status_file "$config_status_loop4:"
$ write config_status_file "$ ac_file = f$element(CONFIG_HEADER_i,"" "",CONFIG_HEADERS)"
$ write config_status_file "$ CONFIG_HEADER_i = CONFIG_HEADER_i + 1"
$ write config_status_file "$ if ac_file .nes. "" """
$ write config_status_file "$  then"
$ write config_status_file "$   write sys$output ""creating "",ac_file"
$ write config_status_file "$   set noon"
$ write config_status_file "$   if f$search(""conftest.h.*"") .nes. """" then delete/nolog conftest.h.*"
$ write config_status_file "$   set on"
$ write config_status_file "$   open/write config_status_tmp conftest.h"
$ write config_status_file "$   write config_status_tmp ""/* '"+"'ac_file'.  Generated automatically by configure.  */"""
$ write config_status_file "$   _tmp = f$parse(ac_file,"".;*"")"
$ write config_status_file "$   ac_input_file = f$parse(ac_file,,,""NAME"")"
$ write config_status_file "$   if f$parse(ac_file,,,""TYPE"") .eqs. ""."""
$ write config_status_file "$    then ac_input_file = ac_input_file + "".IN"""
$ write config_status_file "$    else ac_input_file = ac_input_file + f$parse(ac_file,,,""TYPE"") + ""_IN"""
$ write config_status_file "$    endif"
$ write config_status_file "$   save_def = f$environment(""DEFAULT"")"
$ write config_status_file "$   set def 'ac_given_srcdir'"
$ write config_status_file "$!   set def 'f$parse(ac_file,,,""DEVICE"")'"+"'f$parse(ac_file,,,""DIRECTORY"")"
$ write config_status_file "$   set def 'f$parse(ac_file,,,""DIRECTORY"")'"
$ write config_status_file "$   ac_input_dir = f$environment(""DEFAULT"")"
$ write config_status_file "$   set def 'save_def'"
$ write config_status_file "$!   set noon"
$ write config_status_file "$ __debug = f$trnlnm(""DEBUG_CONFIGURE"")"
$ write config_status_file "$ if .not. __debug then __save_mesg_VMS_LABLE0 = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. __debug then set message/nofacility/noident/noseverity/notext"
$ write config_status_file "$ if .not. __debug then def/user sys$output nl:"
$ write config_status_file "$ if .not. __debug then def/user sys$error nl:"
$ write config_status_file "$   edit/tpu/nosection/nodisplay/command=conftest.tpu/out=config_status_tmp 'ac_input_dir'"+"'ac_input_file'"
$ write config_status_file "$ if .not. __debug then set message '__save_mesg_VMS_LABLE0'"
$ write config_status_file "$"
$ write config_status_file "$   close config_status_tmp"
$ write config_status_file "$   set noon"
$ write config_status_file "$   if f$search(""conftest.tpu.*"") .nes. """" then delete/nolog conftest.tpu.*"
$ write config_status_file "$   if f$search(ac_file) .nes. """" then -"
$ write config_status_file "	diff conftest.h 'ac_file'/out=nla0:"
$ write config_status_file "$   if $status .eq. %X06c8009 ! code for no change"
$ write config_status_file "$    then"
$ write config_status_file "$     ! The file exists and we would not be changing it."
$ write config_status_file "$     write sys$output ""'"+"'ac_file' is unchanged."""
$ write config_status_file "$    else"
$ write config_status_file "$     _tmp = f$parse(ac_file,""*.*;*"")"
$ write config_status_file "$     if f$search(_tmp) .nes. """" then delete/nolog '_tmp'"
$ write config_status_file "$     copy conftest.h 'ac_file'"
$ write config_status_file "$     purge/nolog 'ac_file'"
$ write config_status_file "$    endif"
$ write config_status_file "$   if f$search(""conftest.h.*"") .nes. """" then delete/nolog conftest.h.*"
$ write config_status_file "$   goto config_status_loop4
$ write config_status_file "$  endif
$ write config_status_file "$ set on"
$ write config_status_file "$"
$
$ write config_status_file "$"
$ write config_status_file "$ srcdir=ac_given_srcdir"
$ write config_status_file "$ srcdir_dev=ac_given_srcdir_dev"
$ write config_status_file "$ srcdir_dir=ac_given_srcdir_dir"
$ write config_status_file "$"
$ write config_status_file "$"
$ write config_status_file "$!# Build [.src]descrip.mms from 'srcdir_dev:['srcdir_dir'.src]descrip.mms_in."
$ write config_status_file "$!# This must be done after [.src]config.h is built, since we rely on that"
$ write config_status_file "$!# file."
$ write config_status_file "$"
$ write config_status_file "$! changequote(,)dnl The horror, the horror."
$ write config_status_file "$! # Now get this: Some word that is part of the ${srcdir} directory name"
$ write config_status_file "$! # or the ${configuration} value might, just might, happen to be an"
$ write config_status_file "$! # identifier like `sun4' or `i386' or something, and be predefined by"
$ write config_status_file "$! # the C preprocessor to some helpful value like 1, or maybe the empty"
$ write config_status_file "$! # string.  Needless to say consequent macro substitutions are less"
$ write config_status_file "$! # than conducive to the makefile finding the correct directory."
$ write config_status_file "$! undefs=""`echo $top_srcdir $configuration $canonical |"
$ write config_status_file "$! sed -e 's/[^a-zA-Z0-9_]/ /g' -e 's/^/ /' -e 's/  *$//' \"
$ write config_status_file "$!     -e 's/  */ -U/g' -e 's/-U[0-9][^ ]*//g' \"
$ write config_status_file "$! `"""
$ write config_status_file "$! changequote([,])dnl"
$ write config_status_file "$"
$ write config_status_file "$ save_default = f$environment(""DEFAULT"")"
$ write config_status_file "$!"
$ write config_status_file "$!-------------------------------------------------------"
$ write config_status_file "$ write sys$output ""creating [.lib-src]descrip.mms"""
$ write config_status_file "$ set default [.lib-src]"
$ write config_status_file "$ open/write foo junk1.tpu"
$ write config_status_file "$ write foo ""input_file := GET_INFO (COMMAND_LINE, """"file_name"""");"""
$ write config_status_file "$ write foo ""main_buffer := CREATE_BUFFER (""""main"""", input_file);"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""my_range := SEARCH_QUIETLY (""""start of cpp stuff"""", FORWARD);"""
$ write config_status_file "$ write foo ""IF my_range <> 0"
$ write config_status_file "$ write foo "" THEN"
$ write config_status_file "$ write foo ""  POSITION (BEGINNING_OF (my_range));"""
$ write config_status_file "$ write foo ""  MOVE_VERTICAL (1);"""
$ write config_status_file "$ write foo ""  POSITION (LINE_BEGIN);"
$ write config_status_file "$ write foo ""  mark1 := MARK (NONE);"""
$ write config_status_file "$ write foo ""  POSITION (END_OF (main_buffer));"""
$ write config_status_file "$ write foo ""  mark2 := MARK (NONE);"""
$ write config_status_file "$ write foo ""  range1 := CREATE_RANGE (mark1, mark2, NONE);"""
$ write config_status_file "$ write foo ""  ERASE (range1);"""
$ write config_status_file "$ write foo "" ENDIF;"
$ write config_status_file "$ write foo ""WRITE_FILE (main_buffer, get_info (command_line, """"output_file""""));"""
$ write config_status_file "$ write foo ""QUIT;"""
$ write config_status_file "$ close foo"
$ write config_status_file "$ set noon"
$ write config_status_file "$ save_mesg = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then set message/nofacility/noseverity/noident/notext"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$ edit/tpu/nosection/command=junk1.tpu/nodisplay/out=junk1.c -"
$ write config_status_file "	descrip.mms_in"
$ write config_status_file "$ set message 'save_mesg'"
$ write config_status_file "$!"
$ write config_status_file "$ open/write foo junk.tpu"
$ write config_status_file "$ write foo ""input_file := GET_INFO (COMMAND_LINE, """"file_name"""");"""
$ write config_status_file "$ write foo ""main_buffer := CREATE_BUFFER (""""main"""", input_file);"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""mark1 := MARK (NONE);"""
$ write config_status_file "$ write foo ""my_range := SEARCH_QUIETLY (""""start of cpp stuff"""", FORWARD);"""
$ write config_status_file "$ write foo ""IF my_range <> 0"
$ write config_status_file "$ write foo "" THEN"
$ write config_status_file "$ write foo ""  POSITION (BEGINNING_OF (my_range));"""
$ write config_status_file "$ write foo ""  POSITION (LINE_END);"
$ write config_status_file "$ write foo ""  mark2 := MARK (NONE);"""
$ write config_status_file "$ write foo ""  range1 := CREATE_RANGE (mark1, mark2, NONE);"""
$ write config_status_file "$ write foo ""  ERASE (range1);"""
$ write config_status_file "$ write foo "" ENDIF;"
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""LOOP"""
$ write config_status_file "$ write foo ""  my_range := SEARCH_QUIETLY (((""""/**/#"""" + ((unanchor + """""""")@r1))@r2) + LINE_END, FORWARD);"""
$ write config_status_file "$ write foo ""  EXITIF my_range = 0;"""
$ write config_status_file "$ write foo ""  repl := """"/* """" + SUB""+""STR (r1, 1) + """" */"""";"""
$ write config_status_file "$ write foo ""  ERASE (r2);"""
$ write config_status_file "$ write foo ""  POSITION (r2);"""
$ write config_status_file "$ write foo ""  COPY_TEXT (repl);"""
$ write config_status_file "$ write foo ""  POSITION (END_OF (my_range));"""
$ write config_status_file "$ write foo ""ENDLOOP;"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""WRITE_FILE (main_buffer, get_info (command_line, """"output_file""""));"""
$ write config_status_file "$ write foo ""QUIT;"""
$ write config_status_file "$ close foo"
$ write config_status_file "$ set noon"
$ write config_status_file "$ save_mesg = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then set message/nofacility/noseverity/noident/notext"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$ edit/tpu/nosection/command=junk.tpu/nodisplay/out=junk.c -"
$ write config_status_file "	descrip.mms_in"
$ write config_status_file "$ set message 'save_mesg'"
$ write config_status_file "$!"
$ write config_status_file "$ save_mesg = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then set message/nofacility/noseverity/noident/notext"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$!"
$ write config_status_file "$! don't do this"
$ write config_status_file "$!"
$ write config_status_file "$! CPP junk.i /include=(sys$disk:[],sys$disk:[-.src],'srcdir_dev'['srcdir_dir'.src]) -"
$ write config_status_file "$!	'CPPFLAGS' junk.c"
$ write config_status_file "$!"
$ write config_status_file "$! instead do this"
$ write config_status_file "$!"
$ write config_status_file "$ tradcpp := $sys$disk:[-.tradcpp]tradcpp.exe"
$ if f$getsyi("HW_MODEL").eq. 4096
$  then
$   write config_status_file "$ tradcpp ""-DVMS"" ""-D__ia64"" -"
$  else
$   write config_status_file "$ tradcpp ""-DVMS"" -"
$ endif
$ write config_status_file "          ""-I"" f$environment(""DEFAULT"") -"
$ write config_status_file "          ""-I"" [-.src] -"
$ write config_status_file "          ""-I"" 'srcdir_dev'['srcdir_dir'.src] -"
$ write config_status_file "          junk.c junk.i"
$ write config_status_file "$!"
$ write config_status_file "$ set message 'save_mesg'"
$ write config_status_file "$ set on"
$ write config_status_file "$ junkname := junk.cpp"
$ write config_status_file "$ if f$search(junkname) .eqs. """" then junkname := junk.i"
$ write config_status_file "$ open/write foo junk2.tpu"
$ write config_status_file "$ write foo ""input_file := GET_INFO (COMMAND_LINE, """"file_name"""");"""
$ write config_status_file "$ write foo ""main_buffer := CREATE_BUFFER (""""main"""", input_file);"""
$ write config_status_file "$ write foo ""p_spcs := (SPAN("""" """") | """""""");"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""my_pattern := LINE_BEGIN + ("""" """"@r);"
$ write config_status_file "$ write foo ""LOOP"""
$ write config_status_file "$ write foo ""  my_range := SEARCH_QUIETLY (my_pattern, FORWARD);"""
$ write config_status_file "$ write foo ""  EXITIF my_range = 0;"""
$ write config_status_file "$ write foo ""  ERASE (r); POSITION (r); COPY_TEXT (ASCII(9));"""
$ write config_status_file "$ write foo ""  POSITION (END_OF (my_range));"""
$ write config_status_file "$ write foo ""ENDLOOP;"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""my_pattern := LINE_BEGIN + (""""#""""@r);"
$ write config_status_file "$ write foo ""LOOP"""
$ write config_status_file "$ write foo ""  my_range := SEARCH_QUIETLY (my_pattern, FORWARD);"""
$ write config_status_file "$ write foo ""  EXITIF my_range = 0;"""
$ write config_status_file "$ write foo ""  POSITION (r); ERASE_LINE;"
$ write config_status_file "$ write foo ""  POSITION (END_OF (my_range));"""
$ write config_status_file "$ write foo ""ENDLOOP;"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""my_pattern := LINE_BEGIN + ("""""""" | ANY ("""" 	""""))@r + LINE_END;"""
$ write config_status_file "$ write foo ""LOOP"""
$ write config_status_file "$ write foo ""  my_range := SEARCH_QUIETLY (my_pattern, FORWARD);"""
$ write config_status_file "$ write foo ""  EXITIF my_range = 0;"""
$ write config_status_file "$ write foo ""  POSITION (r); ERASE_LINE;"""
$ write config_status_file "$ write foo ""  POSITION (END_OF (my_range));"""
$ write config_status_file "$ write foo ""ENDLOOP;"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""WRITE_FILE (main_buffer, get_info (command_line, """"output_file""""));"""
$ write config_status_file "$ write foo ""QUIT;"""
$ write config_status_file "$ close foo"
$ write config_status_file "$ set noon"
$ write config_status_file "$ save_mesg = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then set message/nofacility/noseverity/noident/notext"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$ edit/tpu/nosection/command=junk2.tpu/nodisplay/out=junk2.c -"
$ write config_status_file "	'junkname'"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$ copy junk1.c+junk2.c descrip.new"
$ write config_status_file "$ set message 'save_mesg'"
$ write config_status_file "$ if f$search(""descrip.mms"") .nes. """" then -"
$ write config_status_file "	set file/prot=(s:rwed,o:rwed) descrip.mms.*"
$ write config_status_file "$ rename descrip.new .mms"
$ write config_status_file "$ set file/prot=(s:r,o:r,g:r,w:r) descrip.mms"
$ write config_status_file "$ purge/nolog descrip.mms"
$ write config_status_file "$ set on"
$ write config_status_file "$ junknames = junkname+"";*"""
$ write config_status_file "$ delete junk*.c.*,junk%.tpu.*,junk.tpu.*,'junknames'"
$ write config_status_file "$!"
$ write config_status_file "$!-------------------------------------------------------"
$ write config_status_file "$ write sys$output ""creating [.src]descrip.mms"""
$ write config_status_file "$ set default [-.src]"
$ write config_status_file "$ open/write foo junk1.tpu"
$ write config_status_file "$ write foo ""input_file := GET_INFO (COMMAND_LINE, """"file_name"""");"""
$ write config_status_file "$ write foo ""main_buffer := CREATE_BUFFER (""""main"""", input_file);"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""my_range := SEARCH_QUIETLY (""""start of cpp stuff"""", FORWARD);"""
$ write config_status_file "$ write foo ""IF my_range <> 0"
$ write config_status_file "$ write foo "" THEN"
$ write config_status_file "$ write foo ""  POSITION (BEGINNING_OF (my_range));"""
$ write config_status_file "$ write foo ""  MOVE_VERTICAL (1);"""
$ write config_status_file "$ write foo ""  POSITION (LINE_BEGIN);"
$ write config_status_file "$ write foo ""  mark1 := MARK (NONE);"""
$ write config_status_file "$ write foo ""  POSITION (END_OF (main_buffer));"""
$ write config_status_file "$ write foo ""  mark2 := MARK (NONE);"""
$ write config_status_file "$ write foo ""  range1 := CREATE_RANGE (mark1, mark2, NONE);"""
$ write config_status_file "$ write foo ""  ERASE (range1);"""
$ write config_status_file "$ write foo "" ENDIF;"
$ write config_status_file "$ write foo ""WRITE_FILE (main_buffer, get_info (command_line, """"output_file""""));"""
$ write config_status_file "$ write foo ""QUIT;"""
$ write config_status_file "$ close foo"
$ write config_status_file "$ set noon"
$ write config_status_file "$ save_mesg = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then set message/nofacility/noseverity/noident/notext"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$ edit/tpu/nosection/command=junk1.tpu/nodisplay/out=junk1.c -"
$ write config_status_file "	descrip.mms_in"
$ write config_status_file "$ set message 'save_mesg'"
$ write config_status_file "$!"
$ write config_status_file "$ open/write foo junk.tpu"
$ write config_status_file "$ write foo ""input_file := GET_INFO (COMMAND_LINE, """"file_name"""");"""
$ write config_status_file "$ write foo ""main_buffer := CREATE_BUFFER (""""main"""", input_file);"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""mark1 := MARK (NONE);"""
$ write config_status_file "$ write foo ""my_range := SEARCH_QUIETLY (""""start of cpp stuff"""", FORWARD);"""
$ write config_status_file "$ write foo ""IF my_range <> 0"
$ write config_status_file "$ write foo "" THEN"
$ write config_status_file "$ write foo ""  POSITION (BEGINNING_OF (my_range));"""
$ write config_status_file "$ write foo ""  POSITION (LINE_END);"
$ write config_status_file "$ write foo ""  mark2 := MARK (NONE);"""
$ write config_status_file "$ write foo ""  range1 := CREATE_RANGE (mark1, mark2, NONE);"""
$ write config_status_file "$ write foo ""  ERASE (range1);"""
$ write config_status_file "$ write foo "" ENDIF;"
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""LOOP"""
$ write config_status_file "$ write foo ""  my_range := SEARCH_QUIETLY (((""""/**/#"""" + ((unanchor + """""""")@r1))@r2) + LINE_END, FORWARD);"""
$ write config_status_file "$ write foo ""  EXITIF my_range = 0;"""
$ write config_status_file "$ write foo ""  repl := """"/* """" + SUB""+""STR (r1, 1) + """" */"""";"""
$ write config_status_file "$ write foo ""  ERASE (r2);"""
$ write config_status_file "$ write foo ""  POSITION (r2);"""
$ write config_status_file "$ write foo ""  COPY_TEXT (repl);"""
$ write config_status_file "$ write foo ""  POSITION (END_OF (my_range));"""
$ write config_status_file "$ write foo ""ENDLOOP;"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""WRITE_FILE (main_buffer, get_info (command_line, """"output_file""""));"""
$ write config_status_file "$ write foo ""QUIT;"""
$ write config_status_file "$ close foo"
$ write config_status_file "$ set noon"
$ write config_status_file "$ save_mesg = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then set message/nofacility/noseverity/noident/notext"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$ edit/tpu/nosection/command=junk.tpu/nodisplay/out=junk.c -"
$ write config_status_file "	descrip.mms_in"
$ write config_status_file "$ set message 'save_mesg'"
$ write config_status_file "$!"
$ write config_status_file "$ save_mesg = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then set message/nofacility/noseverity/noident/notext"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$!"
$ write config_status_file "$! don't do this"
$ write config_status_file "$!"
$ write config_status_file "$! CPP junk.i /include=(sys$disk:[],sys$disk:[-.src],'srcdir_dev'['srcdir_dir'.src]) -"
$ write config_status_file "$!	'CPPFLAGS' junk.c"
$ write config_status_file "$!"
$ write config_status_file "$! instead do this"
$ write config_status_file "$!"
$ write config_status_file "$ tradcpp := $sys$disk:[-.tradcpp]tradcpp.exe"
$ if f$getsyi("HW_MODEL").eq. 4096
$  then
$   write config_status_file "$ tradcpp ""-DVMS"" ""-D__ia64"" -"
$  else
$   write config_status_file "$ tradcpp ""-DVMS"" -"
$ endif
$ write config_status_file "          ""-I"" f$environment(""DEFAULT"") -"
$ write config_status_file "          ""-I"" [-.src] -"
$ write config_status_file "          ""-I"" 'srcdir_dev'['srcdir_dir'.src] -"
$ write config_status_file "          junk.c junk.i"
$ write config_status_file "$!"
$ write config_status_file "$ set message 'save_mesg'"
$ write config_status_file "$ set on"
$ write config_status_file "$ junkname := junk.cpp"
$ write config_status_file "$ if f$search(junkname) .eqs. """" then junkname := junk.i"
$ write config_status_file "$ open/write foo junk2.tpu"
$ write config_status_file "$ write foo ""input_file := GET_INFO (COMMAND_LINE, """"file_name"""");"""
$ write config_status_file "$ write foo ""main_buffer := CREATE_BUFFER (""""main"""", input_file);"""
$ write config_status_file "$ write foo ""p_spcs := (SPAN("""" """") | """""""");"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""my_pattern := LINE_BEGIN + ("""" """"@r);"
$ write config_status_file "$ write foo ""LOOP"""
$ write config_status_file "$ write foo ""  my_range := SEARCH_QUIETLY (my_pattern, FORWARD);"""
$ write config_status_file "$ write foo ""  EXITIF my_range = 0;"""
$ write config_status_file "$ write foo ""  ERASE (r); POSITION (r); COPY_TEXT (ASCII(9));"""
$ write config_status_file "$ write foo ""  POSITION (END_OF (my_range));"""
$ write config_status_file "$ write foo ""ENDLOOP;"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""my_pattern := LINE_BEGIN + (""""#""""@r);"
$ write config_status_file "$ write foo ""LOOP"""
$ write config_status_file "$ write foo ""  my_range := SEARCH_QUIETLY (my_pattern, FORWARD);"""
$ write config_status_file "$ write foo ""  EXITIF my_range = 0;"""
$ write config_status_file "$ write foo ""  POSITION (r); ERASE_LINE;"
$ write config_status_file "$ write foo ""  POSITION (END_OF (my_range));"""
$ write config_status_file "$ write foo ""ENDLOOP;"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""POSITION (BEGINNING_OF (main_buffer));"""
$ write config_status_file "$ write foo ""my_pattern := LINE_BEGIN + ("""""""" | ANY ("""" 	""""))@r + LINE_END;"""
$ write config_status_file "$ write foo ""LOOP"""
$ write config_status_file "$ write foo ""  my_range := SEARCH_QUIETLY (my_pattern, FORWARD);"""
$ write config_status_file "$ write foo ""  EXITIF my_range = 0;"""
$ write config_status_file "$ write foo ""  POSITION (r); ERASE_LINE;"""
$ write config_status_file "$ write foo ""  POSITION (END_OF (my_range));"""
$ write config_status_file "$ write foo ""ENDLOOP;"""
$ write config_status_file "$ write foo ""!"""
$ write config_status_file "$ write foo ""WRITE_FILE (main_buffer, get_info (command_line, """"output_file""""));"""
$ write config_status_file "$ write foo ""QUIT;"""
$ write config_status_file "$ close foo"
$ write config_status_file "$ set noon"
$ write config_status_file "$ save_mesg = f$environment(""MESSAGE"")"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then set message/nofacility/noseverity/noident/notext"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$ edit/tpu/nosection/command=junk2.tpu/nodisplay/out=junk2.c -"
$ write config_status_file "	'junkname'"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$output nl:"
$ write config_status_file "$ if .not. f$trnlnm(""DEBUG_CONFIGURE"") then define/user sys$error nl:"
$ write config_status_file "$ copy junk1.c+junk2.c descrip.new"
$ write config_status_file "$ set message 'save_mesg'"
$ write config_status_file "$ if f$search(""descrip.mms"") .nes. """" then -"
$ write config_status_file "	set file/prot=(s:rwed,o:rwed) descrip.mms.*"
$ write config_status_file "$ rename descrip.new .mms"
$ write config_status_file "$ set file/prot=(s:r,o:r,g:r,w:r) descrip.mms"
$ write config_status_file "$ purge/nolog descrip.mms"
$ write config_status_file "$ set on"
$ write config_status_file "$ junknames = junkname+"";*"""
$ write config_status_file "$ delete junk*.c.*,junk%.tpu.*,junk.tpu.*,'junknames'"
$ write config_status_file "$!"
$ write config_status_file "$!-------------------------------------------------------"
$ write config_status_file "$ set default [-.vms]"
$ write config_status_file "$ files_to_copy := testemacs.com"
$ write config_status_file "$ set noon"
$ write config_status_file "$ file_i = 0"
$ write config_status_file "$ac_emacs_loop_files_to_copy:"
$ write config_status_file "$ file_e = f$element(file_i,"","",files_to_copy)"
$ write config_status_file "$ file_i = file_i + 1"
$ write config_status_file "$ if file_e .eqs. """" then goto ac_emacs_loop_files_to_copy"
$ write config_status_file "$ if file_e .nes. "","""
$ write config_status_file "$  then"
$ write config_status_file "$   if f$search(file_e) .nes. """" then -"
$ write config_status_file "	diff 'file_e' 'srcdir_dev'['srcdir_dir'.vms]'file_e'/out=nla0:"
$ write config_status_file "$   if $status .ne. %X06c8009 ! code for no change"
$ write config_status_file "$    then"
$ write config_status_file "$     write sys$output ""copying [.vms]"",file_e"
$ write config_status_file "$     copy 'srcdir_dev'['srcdir_dir'.vms]'file_e' sys$disk:[]"
$ write config_status_file "$    endif"
$ write config_status_file "$   goto ac_emacs_loop_files_to_copy"
$ write config_status_file "$  endif"
$ write config_status_file "$ set on"
$ write config_status_file "$!"
$ write config_status_file "$ set def [-]"
$ write config_status_file "$ set def 'save_default'"
$ write config_status_file "$"
$ write config_status_file "$ exit"
$ close config_status_file
$ set file/prot=(s:rwed,o:rwed,g:re,w:re) config.status
$ if .not. no_create then @config.status
$
