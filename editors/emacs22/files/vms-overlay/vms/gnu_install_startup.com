$! GNU_INSTALL_STARTUP.COM -- Startup script installer.
$! Copyright (C) 1994 the Free Software Foundation, Inc.
$!
$! Author: Richard Levitte <levitte@e.kth.se>
$!
$! This file is a part of GNU VMSLIB, the GNU library for porting GNU
$! software to VMS.
$!
$! GNU VMSLIB is free software; you can redistribute it and/or
$! modify it under the terms of the GNU General Public License
$! as published by the Free Software Foundation; either version 2
$! of the License, or (at your option) any later version.
$! 
$! GNU VMSLIB is distributed in the hope that it will be useful,
$! but WITHOUT ANY WARRANTY; without even the implied warranty of
$! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
$! GNU General Public License for more details.
$! 
$! You should have received a copy of the GNU General Public License
$! along with GNU VMSLIB; if not, write to the Free Software
$! Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
$
$! GNU_STARTUP.COM installs the startup file for a package.
$!
$! Description:
$!
$! P1 = package name.
$! P2 = version
$! P3 = setup command file (input).  This should be produced by
$!	gnu_generate_script.com
$! P4 = startupdir
$! P5 = NODATABASE if no database should be handled.
$
$ name = f$edit(P1,"lowercase")
$ uname = f$edit(P1,"UPCASE")
$ desc = f$extract(0,1,uname)+f$extract(1,f$length(name),name)
$ version = P2
$ commands = P3
$ startupdir = P4
$ database_p = P5 .nes. "NODATABASE"
$
$ version_us = f$element(0,".",version)
$ i = 0
$loop_version_us:
$ i = i + 1
$ e = f$element(i, ".", version)
$ if e .eqs. "." then goto end_loop_version_us
$ version_us = version_us + "_" + e
$ goto loop_version_us
$end_loop_version_us:
$
$ vmssrcdir = f$parse("A.;0",f$environment("PROCEDURE")) - "A.;0"
$
$ if database_p
$  then
$   @'vmssrcdir'gnu_startup_manager database='startupdir'gnu_startup.com -
newversion='vmssrcdir'gnu_startup_main.com remove='name'-'version' -
insert='commands'
$   purge 'startupdir'gnu_startup.com
$   set file/prot=(w:re) 'startupdir'gnu_startup.com
$   startup_file = startupdir + "GNU_STARTUP"
$   startup_str1 = "	$ @"+startup_file+" "+uname+"-"+version
$   startup_str2 = "	$ @"+startup_file+" *"
$  else
$   copy/protection=(s:rwed,o:rwed,g:re,w:re) -
        'commands' 'startupdir''name'-'version_us'_startup.com
$   purge 'startupdir''name'-'version_us'_startup.com
$   set file/prot=(w:re) 'startupdir''name'-'version_us'_startup.com
$   startup_file = startupdir + uname + "-" + version_us + "_STARTUP"
$   startup_str1 = "	$ @"+startup_file
$   startup_str2 = ""
$  endif
$ 
$ __save_verify = 'f$verify(0)
$ write sys$output ""
$ write sys$output "	Installation of ",desc," done!"
$ write sys$output ""
$ write sys$output "It is recommended that you put the following line in"
$ write sys$output "SYS$MANAGER:SYSTARTUP_VMS.COM:"
$ write sys$output ""
$ if startup_str1 .nes. "" then write sys$output -
	startup_str1," TABLE=/SYSTEM,INSTALL_IMAGE"
$ write sys$output ""
$ if startup_str2 .nes. ""
$  then
$   write sys$output "Or if you have several GNU programs installed this way:"
$   write sys$output ""
$   write sys$output startup_str2," TABLE=/SYSTEM,INSTALL_IMAGE"
$   write sys$output ""
$  endif
$ write sys$output -
"This will create system-wide logical names, and install ",desc," /SHARED."
$ write sys$output "Please look at ",startup_file,".COM."
$ write sys$output ""
$ write sys$output "Also, put this in SYS$MANAGER:SYLOGIN.COM:"
$ write sys$output ""
$ if startup_str1 .nes. "" then write sys$output -
	startup_str1," NOLOGICALS"
$ if startup_str2 .nes. "" then write sys$output -
	"or",startup_str2," NOLOGICALS"
$ write sys$output ""
$ write sys$output "This will create the appropriate command symbols."
$ write sys$output ""
$ a = f$verify(__save_verify)
$ exit
