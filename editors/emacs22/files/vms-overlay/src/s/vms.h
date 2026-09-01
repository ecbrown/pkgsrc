/* system description header for VMS
   Copyright (C) 1986, 2001, 2002, 2003, 2004, 2005,
                 2006, 2007, 2008  Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.  */

/*
 *	Define symbols to identify the version of Unix this is.
 *	Define all the symbols that apply correctly.
 */

#ifndef VMS		    /* Decus cpp doesn't define this but VAX C does */
#define VMS
#endif /* VMS */
/* Note that this file is used indirectly via vms4-0.h, or some other
   such file.  These other files define a symbol VMS4_0, VMS4_2, etc.  */

/* SYSTEM_TYPE should indicate the kind of system you are using.
 It sets the Lisp variable system-type.  */

#define SYSTEM_TYPE "vax-vms"	/* ttn */
/* #define SYSTEM_TYPE "vms" */	/* ttn */

/* Native OpenVMS support adds logical-name and process Lisp that is not
   present in the stock preload set.  Keep a modest margin so the dumped
   image remains fully purified when those VMS definitions are loaded.  */
#define SYSTEM_PURESIZE_EXTRA 196608

/* NOMULTIPLEJOBS should be defined if your system's shell
 does not have "job control" (the ability to stop a program,
 run some other program, then continue the first one).  */

/* #define NOMULTIPLEJOBS */

/* INTERRUPT_INPUT controls a default for Unix systems.
   VMS uses a separate mechanism.  */

/* #define INTERRUPT_INPUT */

/* Letter to use in finding device name of first pty,
  if system supports pty's.  'a' means it is /dev/ptya0  */

#define FIRST_PTY_LETTER 'a'

/*
 *	Define HAVE_PTYS if the system supports pty devices.
 */

/* #define HAVE_PTYS */

/* Define HAVE_SOCKETS if system supports 4.2-compatible sockets.  */

/* #define HAVE_SOCKETS */

#define HAVE_SELECT

#define HAVE_PTYS

     /* next line is to get signal handlers reinitialized properly after a signal is caught */

#define POSIX_SIGNALS

#if 0 /* ttn */
#if defined(UCX)
#define HAVE_SOCKETS
/* HAVE_GETHOSTNAME is now decided by configure.  */
#endif
#endif /* 0 */

/*
 *	Define NONSYSTEM_DIR_LIBRARY to make Emacs emulate
 *      The 4.2 opendir, etc., library functions.
 */

#define NONSYSTEM_DIR_LIBRARY

/* Define this symbol if your system has the functions bcopy, etc. */

/* #define BSTRING */

/* subprocesses should be defined if you want to
   have code for asynchronous subprocesses
   (as used in M-x compile and M-x shell).
   This is generally OS dependent, and not supported
   under most USG systems. */

#define subprocesses

/* If your system uses COFF (Common Object File Format) then define the
   preprocessor symbol "COFF". */

/* #define COFF */

/* define MAIL_USE_FLOCK if the mailer uses flock
   to interlock access to /usr/spool/mail/$USER.
   The alternative is that a lock file named
   /usr/spool/mail/$USER.lock.  */

/* #define MAIL_USE_FLOCK */

/* Define CLASH_DETECTION if you want lock files to be written
   so that Emacs can tell instantly when you try to modify
   a file that someone else has modified in his Emacs.  */

/* #define CLASH_DETECTION */

/* Define the maximum record length for print strings, if needed. */

#define MAX_PRINT_CHARS 300


/* Here, on a separate page, add any special hacks needed
   to make Emacs work on this system.  For example,
   you might define certain system call names that don't
   exist on your system, or that do different things on
   your system and must be used only through an encapsulation
   (Which you should place, by convention, in sysdep.c).  */

/* VMS `write' adds newline.  Define this to enable compensatory
   buffering in `emacs_write'.  */

#define WRITE_ADDS_NEWLINE

/* In olden days, VMS filenames did not support hyphen (i.e., the "-"
   character).  You can #undef this in vmsX-Y.h for newer versions.  */

#define NO_HYPHENS_IN_FILENAMES

/* Do you have the sharable library bug?  If you link with a sharable
   library that contains psects with the NOSHR attribute and also refer to
   those psects in your program, the linker give you a private version of
   the psect which is not related to the version used by the sharable
   library.  The end result is that your references to variables in that
   psect have absolutely nothing to do with library references to what is
   supposed to be the same variable.  If you intend to link with the standard
   C library (NOT the sharable one) you can undef this in vmsX-Y.h.  (This
   is NOT fixed in V4.4...)  */

#define SHARABLE_LIB_BUG

/* Partially due to the above mentioned bug and also so that we don't need
   to require that people have a sharable C library, the default for Emacs
   is to link with the non-shared library.  If you want to link with the
   shared library, define this and remake xmakefile and fileio.c. This allows
   us to ship a guaranteed executable image. */

#define LINK_CRTL_SHARE

/* Define this if you want to read the file SYS$SYSTEM:SYSUAF.DAT for user
   information.  If you do use this, you must either make SYSUAF.DAT world
   readable or install Emacs with SYSPRV.  */

/* #define READ_SYSUAF */

/* Traditionally, filenames on VMS are always upper case.  */

#define FILE_SYSTEM_CASE Fupcase

/* These are some special definitions that are needed */
#ifdef NETLIB
/* There is no need to insert netlib_shr/share, since socketshr requires it
   anyway */
#define LIBS_SYSTEM socketshr/share
#endif

#ifdef UCX
#ifdef __DECC
#define LIBS_SYSTEM sys$share:ucx$ipc_shr/share
#else /* for some reason, the VAX C compiled version can't stand
	 the shareable lib */
#define LIBS_SYSTEM sys$library:ucx$ipc/library
#endif
/* begin ttn hack */
#undef LIBS_SYSTEM
#define LIBS_SYSTEM ucx$library:tcpip$lib/library
/* end ttn hack */
#endif

#ifdef MULTINET
#define LIBS_SYSTEM multinet:multinet_socket_library/share
#ifdef __DECC
#define C_SWITCH_SYSTEM /standard=vaxc \
/prefix=(all,except=(setsockopt,ntohs,htonl,htons,getsockname,getpeername,\
gethostbyname,gethostbyaddr,gethostname,inet_ntoa,\
inet_addr,connect,listen,bind,accept,socket))\
/WARN=DIS=(ADDRCONSTEXT,GLOBALEXT,LONGEXTERN)
#endif
#endif

/* This is super weird.  --ttn  */
#define asdf_C_DEBUG_SWITCH  /deb/noopt
#define C_DEBUG_SWITCH /op=(le=5,t=h)

#define LIB_MATH
#define LIB_STANDARD
#define LIB_X11_LIB sys$share:decw$xlibshr/share
/* this used to have a comma --ttn  */
#define UNEXEC vmsmap.obj
#ifdef UNEXEC_SRC
#undef UNEXEC_SRC
#endif
#define UNEXEC_SRC $(srcdir)vmsmap.c
#define UNEXEC_HDRS $(srcdir)getpagesize.h
#define ORDINARY_LINK

#ifndef HAVE_ALLOCA
#define C_ALLOCA
#endif

#ifndef NOT_C_CODE

#ifdef HAVE_ALLOCA
#ifdef __GNUC__
#define alloca __builtin_alloca
#else
#ifdef __DECC
#include <builtins.h>
#define alloca __ALLOCA
#else
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#else
char *alloca ();
#endif
#endif
#endif
#endif

/* On VMS these have a different name.  In the following, the checks
   for pre-processor symbol `emacs' are to distinguish [-] builds (which
   define it) from [-.-.lib-src] builds (which does not define it).  */

#define index strchr
#define rindex strrchr
#define unlink delete

#ifndef _GNUC_
extern double mth$dmod(double, double);
#define drem mth$dmod
#endif

/* Current OpenVMS C RTLs provide these time routines with standard
   prototypes.  The historical VAX C wrappers below in sysdep.c are
   retained for older ports, but must not be selected on this target. */

/* On later versions of VMS these exist in the C run time library, but
   we are using our own implementations.  Hide their names to avoid
   linker errors.  */
#define rename sys_rename
#define execvp sys_execvp
#define system sys_system

#define select sys_select

#define vlimit sys_vlimit	/* ttn */

#if defined(UCX) || defined(NETLIB)
/* connect() and socket() are defined in the UCX socket library, but they
   won't return a channel number we could use for SYS$QIO, so we redefine
   them.  */

/* Trying without these with TCPIP instead of UCX?  */
#define connect sys_connect
#define socket sys_socket
#endif


#ifndef GNU_MALLOC
/* Hide these names so that we don't get linker errors.  */
#define malloc sys_malloc
#define free sys_free
#define realloc sys_realloc
#define calloc sys_calloc

/* Don't use the standard brk and sbrk.  */
#define sbrk sys_sbrk
#define brk sys_brk
#endif

/* On VMS we want to avoid reading and writing very large amounts of
   data at once, so we redefine read and write here.  */

#ifdef emacs
#define read sys_read
#define write sys_write
#endif

#if 0 /* Don't do this.  Use sys_creat explicitly when needed.
	 Otherwise, horrible confusion occurs.  */
/* sys_creat just calls the real creat with additional args of
   "rfm=var", "rat=cr" to get "normal" VMS files... */
#define creat sys_creat
#endif /* 0 */

/* fwrite forces an RMS PUT on every call.  This is abysmally slow, so
   we emulate fwrite with fputc, which forces buffering and is much
   faster!  */
#define fwrite sys_fwrite

/* getuid only returns the member number, which is not unique on most VMS
   systems.  We emulate it with (getgid()<<16 | getuid()).  */
/* Actually, on VMS 7.3-2 you can make getuid return the true user id */
#define getuid sys_getuid

/* getpwuid needs to be aware of our shenanegans w/ getuid.  */
#define getpwuid sys_getpwuid

/* If user asks for TERM, check first for EMACS_TERM.  */
#ifdef emacs
#define getenv sys_getenv
#endif

/* Standard C abort is less useful than it should be.  */
#ifdef emacs
#define abort sys_abort
#endif

/* Case conflicts with C library fread.  */
#define Fread F_read

/* Case conflicts with C library srandom.  */
#define Srandom S_random

/* Cause initialization of vmsfns.c to be run.  */
#define SYMS_SYSTEM syms_of_vmsfns ()

/* VAXCRTL access doesn't deal with SYSPRV very well (among other oddities).
   Here, we use $CHKPRO to really determine access.  */
#define access sys_access

#define	PAGESIZE	512

#define	_longjmp	longjmp
#define	_setjmp		setjmp

globalref char sdata[];
#define DATA_START (((int) sdata + 511) & ~511)
#define TEXT_START 512

/* Baud-rate values from tty status are not standard.  */

#define BAUD_CONVERT  \
{ 0, 50, 75, 110, 134, 150, 300, 600, 1200, 1800, \
  2000, 2400, 3600, 4800, 7200, 9600, 19200 }

/* Stdio FILE type has extra indirect on VMS, so must alter this macro.  */

#define PENDING_OUTPUT_COUNT(FILE) ((*(FILE))->_ptr - (*(FILE))->_base)

#define NULL_DEVICE "NLA0:"

/*#define TERMCAP_NAME "emacs_library:[etc]termcap.dat"*/

#define EXEC_SUFFIXES ".exe:.com"

/* Case conflict with Xlib XFree () */
#define xfree emacs_xfree

/* What separator do we use in paths?  */
#define SEPCHAR ','

/* What separator do we use between directory components?  This is
   unused because '.' may occur also in the non-directory part, for
   example "temacs.exe".  */
/* #define DIRECTORY_SEP '.' */

#endif /* no NOT_C_CODE */

/* arch-tag: 76bc2b70-46d1-4334-8f12-955c0d0ca6d4
   (do not change this comment) */
