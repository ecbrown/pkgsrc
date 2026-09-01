/* Interfaces to system-dependent kernel and library entries.
   Copyright (C) 1985, 1986, 1987, 1988, 1993, 1994, 1995, 1999, 2000, 2001,
                 2002, 2003, 2004, 2005, 2006, 2007, 2008
                 Free Software Foundation, Inc.

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <setjmp.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "lisp.h"
/* Including stdlib.h isn't necessarily enough to get srandom
   declared, e.g. without __USE_XOPEN_EXTENDED with glibc 2.  */
#ifdef HAVE_RANDOM
#if 0 /* It turns out that defining _OSF_SOURCE in osf5-0.h gets
	 random prototyped as returning `int'.  It looks to me as
	 though the best way to DTRT is to prefer the rand48 functions
	 (per libc.info).  -- fx */
extern long int random P_ ((void));
#endif
#if 0 /* Don't prototype srandom; it takes an unsigned argument on
	 some systems, and an unsigned long on others, like FreeBSD
	 4.1.  */
extern void srandom P_ ((unsigned int));
#endif
#endif

#include "blockinput.h"

#ifdef MAC_OS8
#include <sys/param.h>

#ifndef subprocesses
/* Nonzero means delete a process right away if it exits (process.c).  */
static int delete_exited_processes;
#endif
#endif  /* MAC_OS8 */

#ifdef VMS

/* The redefinition of foo to be sys_foo in s/vms.h should
   not apply in this file.  For each such alias, unset it
   and define VMS_NEED_SYS_foo, which is then used to wrap
   the actual definition below.  */

#ifdef tzset
#define VMS_NEED_SYS_TZSET
#endif

#ifdef localtime
#define VMS_NEED_SYS_LOCALTIME
#endif

#ifdef gmtime
#define VMS_NEED_SYS_GMTIME
#endif

#ifdef rename
#define VMS_NEED_SYS_RENAME
#endif

#ifdef execvp
#define VMS_NEED_SYS_EXECVP
#endif

#ifdef system
#define VMS_NEED_SYS_SYSTEM
#endif

#ifdef select
#define VMS_NEED_SYS_SELECT
#endif

#ifdef connect
#define VMS_NEED_SYS_CONNECT
#endif

#ifdef socket
#define VMS_NEED_SYS_SOCKET
#endif

#ifdef malloc
#define VMS_NEED_SYS_MALLOC
#endif

#ifdef free
#define VMS_NEED_SYS_FREE
#endif

#ifdef realloc
#define VMS_NEED_SYS_REALLOC
#endif

#ifdef calloc
#define VMS_NEED_SYS_CALLOC
#endif

#ifdef sbrk
#define VMS_NEED_SYS_SBRK
#endif

#ifdef brk
#define VMS_NEED_SYS_BRK
#endif

#ifdef read
#define VMS_NEED_SYS_READ
#endif

#ifdef write
#define VMS_NEED_SYS_WRITE
#endif

#ifdef creat
#define VMS_NEED_SYS_CREAT
#endif

#ifdef fwrite
#define VMS_NEED_SYS_FWRITE
#endif

#ifdef getuid
#define VMS_NEED_SYS_GETUID
#endif

#ifdef getpwuid
#define VMS_NEED_SYS_GETPWUID
#endif

#ifdef getenv
#define VMS_NEED_SYS_GETENV
#endif

#ifdef access
#define VMS_NEED_SYS_ACCESS
#endif

/* Now pull in the regular stuff.  */
#include <stdlib.h>
#include <libdtdef.h>
#include <lib$routines.h>
#include <starlet.h>
#include <pwd.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

extern char *getwd (char *);

#endif /* VMS */

#ifdef WINDOWSNT
#define read sys_read
#define write sys_write
#include <windows.h>
#ifndef NULL
#define NULL 0
#endif
#endif /* not WINDOWSNT */

#ifdef VMS
#ifndef close
#define sys_close close
#else
#undef close
#endif

#ifndef open
#define sys_open open
#else /* `open' is a macro */
#undef open
#endif /* `open' is a macro */
#endif

/* Does anyone other than VMS need this? */
#ifndef fwrite
#define sys_fwrite fwrite
#else
#undef fwrite
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef HAVE_SETPGID
#if !defined (USG) || defined (BSD_PGRPS)
#undef setpgrp
#define setpgrp setpgid
#endif
#endif

/* Get SI_SRPC_DOMAIN, if it is available.  */
#ifdef HAVE_SYS_SYSTEMINFO_H
#include <sys/systeminfo.h>
#endif

#ifdef MSDOS	/* Demacs 1.1.2 91/10/20 Manabu Higashida, MW Aug 1993 */
#include <dos.h>
#include "dosfns.h"
#include "msdos.h"
#include <sys/param.h>

#if __DJGPP__ > 1
extern int etext;
extern unsigned start __asm__ ("start");
#endif
#endif

#ifndef USE_CRT_DLL
#ifndef errno
extern int errno;
#endif
#endif

#ifdef VMS
#include <rms.h>
#include <ttdef.h>
#include <tt2def.h>
#include <iodef.h>
#include <ssdef.h>
#include <descrip.h>
#include <fibdef.h>
#include <atrdef.h>
#include <ctype.h>
#include <string.h>
#include <perror.h>
#ifdef __GNUC__
#include <sys/file.h>
#else
#include <file.h>
#endif
#undef F_SETFL
#ifndef RAB$C_BID
#include <rab.h>
#endif
#define MAXIOSIZE (32 * PAGESIZE) /* Don't I/O more than 32 blocks at a time */
extern unsigned int input_ef = 0;
static unsigned int input_eflist = 0;
static unsigned int timer_ef = 0;
static unsigned int timer_eflist = 0;
unsigned int process_ef = 0;	/* also used in vmsfns.c */
static unsigned int stop_input = 0;
static unsigned int ast_queued = 0;
static const unsigned __int64 reqidt = 0xdeadbeefdeadbeef;
#endif /* VMS */

#ifndef VMS
#include <sys/file.h>
#endif /* not VMS */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifndef MSDOS
#include <sys/ioctl.h>
#endif

#include "systty.h"

#ifdef VMS /* moved here from systty.h */
static $DESCRIPTOR (input_dsc, "TT");
static int terminator_mask[2] = { 0, 0 };
#endif /* VMS */

#include "syswait.h"

#ifdef BROKEN_TIOCGWINSZ
#undef TIOCGWINSZ
#undef TIOCSWINSZ
#endif

#if defined (USG) || defined (DGUX)
#include <sys/utsname.h>
#ifndef MEMORY_IN_STRING_H
#include <memory.h>
#endif
#if defined (TIOCGWINSZ) || defined (ISC4_0)
#ifdef NEED_SIOCTL
#include <sys/sioctl.h>
#endif
#ifdef NEED_PTEM_H
#include <sys/stream.h>
#include <sys/ptem.h>
#endif
#endif /* TIOCGWINSZ or ISC4_0 */
#endif /* USG or DGUX */

extern int quit_char;

#include "keyboard.h"
#include "frame.h"
#include "window.h"
#include "termhooks.h"
#include "termchar.h"
#include "termopts.h"
#include "dispextern.h"
#include "process.h"

#ifdef WINDOWSNT
#include <direct.h>
/* In process.h which conflicts with the local copy.  */
#define _P_WAIT 0
int _CRTAPI1 _spawnlp (int, const char *, const char *, ...);
int _CRTAPI1 _getpid (void);
extern char *getwd (char *);
#endif

#ifdef NONSYSTEM_DIR_LIBRARY
#include "ndir.h"
#endif /* NONSYSTEM_DIR_LIBRARY */

#include "syssignal.h"
#include "systime.h"
#ifdef HAVE_UTIME_H
#include <utime.h>
#endif

#ifndef HAVE_UTIMES
#ifndef HAVE_STRUCT_UTIMBUF
/* We want to use utime rather than utimes, but we couldn't find the
   structure declaration.  We'll use the traditional one.  */
struct utimbuf {
  long actime;
  long modtime;
};
#endif
#endif

/* LPASS8 is new in 4.3, and makes cbreak mode provide all 8 bits.  */
#ifndef LPASS8
#define LPASS8 0
#endif

#ifdef BSD4_1
#define LNOFLSH 0100000
#endif

static int baud_convert[] =
#ifdef BAUD_CONVERT
  BAUD_CONVERT;
#else
  {
    0, 50, 75, 110, 135, 150, 200, 300, 600, 1200,
    1800, 2400, 4800, 9600, 19200, 38400
  };
#endif

#ifdef HAVE_SPEED_T
#include <termios.h>
#else
#if defined (HAVE_LIBNCURSES) && ! defined (NCURSES_OSPEED_T)
#else
#if defined (HAVE_TERMIOS_H) && defined (GNU_LINUX)
#include <termios.h>
#endif
#endif
#endif

int emacs_ospeed;

/* The file descriptor for Emacs's input terminal.
   Under Unix, this is normally zero except when using X;
   under VMS, we place the input channel number here.  */
int input_fd;

void croak P_ ((char *)) NO_RETURN;

#ifdef AIXHFT
void hft_init ();
void hft_reset ();
#endif

/* Temporary used by `sigblock' when defined in terms of signprocmask.  */

SIGMASKTYPE sigprocmask_set;


#if !defined (HAVE_GET_CURRENT_DIR_NAME) || defined (BROKEN_GET_CURRENT_DIR_NAME)

/* Return the current working directory.  Returns NULL on errors.
   Any other returned value must be freed with free. This is used
   only when get_current_dir_name is not defined on the system.  */
char*
get_current_dir_name ()
{
  char *buf;
  char *pwd;
  struct stat dotstat, pwdstat;
  /* If PWD is accurate, use it instead of calling getwd.  PWD is
     sometimes a nicer name, and using it may avoid a fatal error if a
     parent directory is searchable but not readable.  */
    if ((pwd = getenv ("PWD")) != 0
      && (IS_DIRECTORY_SEP (*pwd) || (*pwd && IS_DEVICE_SEP (pwd[1])))
      && stat (pwd, &pwdstat) == 0
      && stat (".", &dotstat) == 0
      && dotstat.st_ino == pwdstat.st_ino
      && dotstat.st_dev == pwdstat.st_dev
#ifdef MAXPATHLEN
      && strlen (pwd) < MAXPATHLEN
#endif
      )
    {
      buf = (char *) malloc (strlen (pwd) + 1);
      if (!buf)
        return NULL;
      strcpy (buf, pwd);
    }
#ifdef HAVE_GETCWD
  else
    {
      size_t buf_size = 1024;
      buf = (char *) malloc (buf_size);
      if (!buf)
        return NULL;
      for (;;)
        {
          if (getcwd (buf, buf_size) == buf)
            break;
          if (errno != ERANGE)
            {
              int tmp_errno = errno;
              free (buf);
              errno = tmp_errno;
              return NULL;
            }
          buf_size *= 2;
          buf = (char *) realloc (buf, buf_size);
          if (!buf)
            return NULL;
        }
    }
#else
  else
    {
      /* We need MAXPATHLEN here.  */
      buf = (char *) malloc (MAXPATHLEN + 1);
      if (!buf)
        return NULL;
      if (getwd (buf) == NULL)
        {
          int tmp_errno = errno;
          free (buf);
          errno = tmp_errno;
          return NULL;
        }
    }
#endif
  return buf;
}
#endif


/* Specify a different file descriptor for further input operations.  */

void
change_input_fd (fd)
     int fd;
{
  input_fd = fd;
}

/* Discard pending input on descriptor input_fd.  */

static void end_kbd_input (void);
static void queue_kbd_input (void);

void
discard_tty_input ()
{
#ifndef WINDOWSNT
  struct emacs_tty buf;

  if (noninteractive)
    return;

  /* Discarding input is not safe when the input could contain
     replies from the X server.  So don't do it.  */
  if (read_socket_hook)
    return;

#ifdef VMS
  end_kbd_input ();
  SYS$QIOW (0, input_fd, IO$_READVBLK|IO$M_PURGE, input_iosb, 0, 0,
	    &buf.main, 0, 0, terminator_mask, 0, 0);
  queue_kbd_input ();
#else /* not VMS */
#ifdef APOLLO
  {
    int zero = 0;
    ioctl (input_fd, TIOCFLUSH, &zero);
  }
#else /* not Apollo */
#ifdef MSDOS    /* Demacs 1.1.1 91/10/16 HIRANO Satoshi */
  while (dos_keyread () != -1)
    ;
#else /* not MSDOS */
  EMACS_GET_TTY (input_fd, &buf);
  EMACS_SET_TTY (input_fd, &buf, 0);
#endif /* not MSDOS */
#endif /* not Apollo */
#endif /* not VMS */
#endif /* not WINDOWSNT */
}

#ifdef SIGTSTP

/* Arrange for character C to be read as the next input from
   the terminal.  */

void
#ifdef PROTOTYPES
stuff_char (char c)
#else
stuff_char (c)
     char c;
#endif
{
  if (read_socket_hook)
    return;

/* Should perhaps error if in batch mode */
#ifdef TIOCSTI
  ioctl (input_fd, TIOCSTI, &c);
#else /* no TIOCSTI */
  error ("Cannot stuff terminal input characters in this version of Unix");
#endif /* no TIOCSTI */
}

#endif /* SIGTSTP */

void
init_baud_rate ()
{
  if (noninteractive)
    emacs_ospeed = 0;
  else
    {
#ifdef INIT_BAUD_RATE
      INIT_BAUD_RATE ();
#else
#ifdef DOS_NT
    emacs_ospeed = 15;
#else  /* not DOS_NT */
#ifdef VMS
      struct sensemode sg;

      SYS$QIOW (0, input_fd, IO$_SENSEMODE, &sg, 0, 0,
		&sg.class, 12, 0, 0, 0, 0 );
      emacs_ospeed = sg.xmit_baud;
#else /* not VMS */
#ifdef HAVE_TERMIOS
      struct termios sg;

      sg.c_cflag = B9600;
      tcgetattr (input_fd, &sg);
      emacs_ospeed = cfgetospeed (&sg);
#if defined (USE_GETOBAUD) && defined (getobaud)
      /* m88k-motorola-sysv3 needs this (ghazi@noc.rutgers.edu) 9/1/94. */
      if (emacs_ospeed == 0)
        emacs_ospeed = getobaud (sg.c_cflag);
#endif
#else /* neither VMS nor TERMIOS */
#ifdef HAVE_TERMIO
      struct termio sg;

      sg.c_cflag = B9600;
#ifdef HAVE_TCATTR
      tcgetattr (input_fd, &sg);
#else
      ioctl (input_fd, TCGETA, &sg);
#endif
      emacs_ospeed = sg.c_cflag & CBAUD;
#else /* neither VMS nor TERMIOS nor TERMIO */
      struct sgttyb sg;

      sg.sg_ospeed = B9600;
      if (ioctl (input_fd, TIOCGETP, &sg) < 0)
	abort ();
      emacs_ospeed = sg.sg_ospeed;
#endif /* not HAVE_TERMIO */
#endif /* not HAVE_TERMIOS */
#endif /* not VMS */
#endif /* not DOS_NT */
#endif /* not INIT_BAUD_RATE */
    }

  baud_rate = (emacs_ospeed < sizeof baud_convert / sizeof baud_convert[0]
	       ? baud_convert[emacs_ospeed] : 9600);
  if (baud_rate == 0)
    baud_rate = 1200;
}

/*ARGSUSED*/
void
set_exclusive_use (fd)
     int fd;
{
#ifdef FIOCLEX
  ioctl (fd, FIOCLEX, 0);
#endif
  /* Ok to do nothing if this feature does not exist */
}

#ifndef subprocesses

wait_without_blocking ()
{
#ifdef BSD_SYSTEM
  wait3 (0, WNOHANG | WUNTRACED, 0);
#else
  croak ("wait_without_blocking");
#endif
  synch_process_alive = 0;
}

#endif /* not subprocesses */

int wait_debugging;   /* Set nonzero to make following function work under dbx
			 (at least for bsd).  */

SIGTYPE
wait_for_termination_signal ()
{}

#ifndef VMS /* moved to vmsproc.c (like 19.28 --ttn)  */

/* Wait for subprocess with process id `pid' to terminate and
   make sure it will get eliminated (not remain forever as a zombie) */

void
wait_for_termination (pid)
     int pid;
{
  while (1)
    {
#ifdef subprocesses
#ifdef VMS
      int status;

      status = SYS$FORCEX (&pid, 0, 0);
      break;
#else /* not VMS */
#if defined (BSD_SYSTEM) || (defined (HPUX) && !defined (HPUX_5))
      /* Note that kill returns -1 even if the process is just a zombie now.
	 But inevitably a SIGCHLD interrupt should be generated
	 and child_sig will do wait3 and make the process go away. */
      /* There is some indication that there is a bug involved with
	 termination of subprocesses, perhaps involving a kernel bug too,
	 but no idea what it is.  Just as a hunch we signal SIGCHLD to see
	 if that causes the problem to go away or get worse.  */
      sigsetmask (sigmask (SIGCHLD));
      if (0 > kill (pid, 0))
	{
	  sigsetmask (SIGEMPTYMASK);
	  kill (getpid (), SIGCHLD);
	  break;
	}
      if (wait_debugging)
	sleep (1);
      else
	sigpause (SIGEMPTYMASK);
#else /* not BSD_SYSTEM, and not HPUX version >= 6 */
#if defined (UNIPLUS)
      if (0 > kill (pid, 0))
	break;
      wait (0);
#else /* neither BSD_SYSTEM nor UNIPLUS: random sysV */
#ifdef POSIX_SIGNALS    /* would this work for GNU/Linux as well? */
      sigblock (sigmask (SIGCHLD));
      errno = 0;
      if (kill (pid, 0) == -1 && errno == ESRCH)
	{
	  sigunblock (sigmask (SIGCHLD));
	  break;
	}

      sigsuspend (&empty_mask);
#else /* not POSIX_SIGNALS */
#ifdef HAVE_SYSV_SIGPAUSE
      sighold (SIGCHLD);
      if (0 > kill (pid, 0))
	{
	  sigrelse (SIGCHLD);
	  break;
	}
      sigpause (SIGCHLD);
#else /* not HAVE_SYSV_SIGPAUSE */
#ifdef WINDOWSNT
      wait (0);
      break;
#else /* not WINDOWSNT */
      if (0 > kill (pid, 0))
	break;
      /* Using sleep instead of pause avoids timing error.
	 If the inferior dies just before the sleep,
	 we lose just one second.  */
      sleep (1);
#endif /* not WINDOWSNT */
#endif /* not HAVE_SYSV_SIGPAUSE */
#endif /* not POSIX_SIGNALS */
#endif /* not UNIPLUS */
#endif /* not BSD_SYSTEM, and not HPUX version >= 6 */
#endif /* not VMS */
#else /* not subprocesses */
#if __DJGPP__ > 1
      break;
#else /* not __DJGPP__ > 1 */
#ifndef BSD4_1
      if (kill (pid, 0) < 0)
	break;
      wait (0);
#else /* BSD4_1 */
      int status;
      status = wait (0);
      if (status == pid || status == -1)
	break;
#endif /* BSD4_1 */
#endif /* not __DJGPP__ > 1*/
#endif /* not subprocesses */
    }
}
#endif /* not VMS */

#ifdef subprocesses

/*
 *	flush any pending output
 *      (may flush input as well; it does not matter the way we use it)
 */

void
flush_pending_output (channel)
     int channel;
{
#ifdef HAVE_TERMIOS
  /* If we try this, we get hit with SIGTTIN, because
     the child's tty belongs to the child's pgrp. */
#else
#ifdef TCFLSH
  ioctl (channel, TCFLSH, 1);
#else
#ifdef TIOCFLUSH
  int zero = 0;
  /* 3rd arg should be ignored
     but some 4.2 kernels actually want the address of an int
     and nonzero means something different.  */
  ioctl (channel, TIOCFLUSH, &zero);
#endif
#endif
#endif
}

#ifndef VMS
/*  Set up the terminal at the other end of a pseudo-terminal that
    we will be controlling an inferior through.
    It should not echo or do line-editing, since that is done
    in Emacs.  No padding needed for insertion into an Emacs buffer.  */

void
child_setup_tty (out)
     int out;
{
#ifndef DOS_NT
  struct emacs_tty s;

  EMACS_GET_TTY (out, &s);

#if defined (HAVE_TERMIO) || defined (HAVE_TERMIOS)
  s.main.c_oflag |= OPOST;	/* Enable output postprocessing */
  s.main.c_oflag &= ~ONLCR;	/* Disable map of NL to CR-NL on output */
#ifdef NLDLY
  s.main.c_oflag &= ~(NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY);
  				/* No output delays */
#endif
  s.main.c_lflag &= ~ECHO;	/* Disable echo */
  s.main.c_lflag |= ISIG;	/* Enable signals */
#if 0  /* This causes bugs in (for instance) telnet to certain sites.  */
  s.main.c_iflag &= ~ICRNL;	/* Disable map of CR to NL on input */
#ifdef INLCR  /* Just being cautious, since I can't check how
		 widespread INLCR is--rms.  */
  s.main.c_iflag &= ~INLCR;	/* Disable map of NL to CR on input */
#endif
#endif
#ifdef IUCLC
  s.main.c_iflag &= ~IUCLC;	/* Disable downcasing on input.  */
#endif
#ifdef ISTRIP
  s.main.c_iflag &= ~ISTRIP;	/* don't strip 8th bit on input */
#endif
#ifdef OLCUC
  s.main.c_oflag &= ~OLCUC;	/* Disable upcasing on output.  */
#endif
  s.main.c_oflag &= ~TAB3;	/* Disable tab expansion */
  s.main.c_cflag = (s.main.c_cflag & ~CSIZE) | CS8; /* Don't strip 8th bit */
#if 0
  /* Said to be unnecessary:  */
  s.main.c_cc[VMIN] = 1;	/* minimum number of characters to accept  */
  s.main.c_cc[VTIME] = 0;	/* wait forever for at least 1 character  */
#endif

  s.main.c_lflag |= ICANON;	/* Enable erase/kill and eof processing */
  s.main.c_cc[VEOF] = 04;	/* insure that EOF is Control-D */
  s.main.c_cc[VERASE] = CDISABLE;	/* disable erase processing */
  s.main.c_cc[VKILL] = CDISABLE;	/* disable kill processing */

#ifdef HPUX
  s.main.c_cflag = (s.main.c_cflag & ~CBAUD) | B9600; /* baud rate sanity */
#endif /* HPUX */

#ifdef SIGNALS_VIA_CHARACTERS
  /* the QUIT and INTR character are used in process_send_signal
     so set them here to something useful.  */
  if (s.main.c_cc[VQUIT] == CDISABLE)
    s.main.c_cc[VQUIT] = '\\'&037;	/* Control-\ */
  if (s.main.c_cc[VINTR] == CDISABLE)
    s.main.c_cc[VINTR] = 'C'&037;	/* Control-C */
#endif /* not SIGNALS_VIA_CHARACTERS */

#ifdef AIX
/* AIX enhanced edit loses NULs, so disable it */
#ifndef IBMR2AIX
  s.main.c_line = 0;
  s.main.c_iflag &= ~ASCEDIT;
#endif
  /* Also, PTY overloads NUL and BREAK.
     don't ignore break, but don't signal either, so it looks like NUL.  */
  s.main.c_iflag &= ~IGNBRK;
  s.main.c_iflag &= ~BRKINT;
  /* rms: Formerly it set s.main.c_cc[VINTR] to 0377 here
     unconditionally.  Then a SIGNALS_VIA_CHARACTERS conditional
     would force it to 0377.  That looks like duplicated code.  */
#ifndef SIGNALS_VIA_CHARACTERS
  /* QUIT and INTR work better as signals, so disable character forms */
  s.main.c_cc[VQUIT] = CDISABLE;
  s.main.c_cc[VINTR] = CDISABLE;
  s.main.c_lflag &= ~ISIG;
#endif /* no TIOCGPGRP or no TIOCGLTC or no TIOCGETC */
  s.main.c_cc[VEOL] = CDISABLE;
  s.main.c_cflag = (s.main.c_cflag & ~CBAUD) | B9600; /* baud rate sanity */
#endif /* AIX */

#else /* not HAVE_TERMIO */

  s.main.sg_flags &= ~(ECHO | CRMOD | ANYP | ALLDELAY | RAW | LCASE
		       | CBREAK | TANDEM);
  s.main.sg_flags |= LPASS8;
  s.main.sg_erase = 0377;
  s.main.sg_kill = 0377;
  s.lmode = LLITOUT | s.lmode;        /* Don't strip 8th bit */

#endif /* not HAVE_TERMIO */

  EMACS_SET_TTY (out, &s, 0);

#ifdef BSD4_1
  if (interrupt_input)
    reset_sigio ();
#endif /* BSD4_1 */
#ifdef RTU
  {
    int zero = 0;
    ioctl (out, FIOASYNC, &zero);
  }
#endif /* RTU */
#endif /* not DOS_NT */
}
#endif /* not VMS */

#endif /* subprocesses */

/* Record a signal code and the handler for it.  */
struct save_signal
{
  int code;
  SIGTYPE (*handler) P_ ((int));
};

static void save_signal_handlers P_ ((struct save_signal *));
static void restore_signal_handlers P_ ((struct save_signal *));

/* Suspend the Emacs process; give terminal to its superior.  */

#ifdef VMS
int
#else
void
#endif
sys_suspend ()
{
#ifdef VMS
  /* "Foster" parentage allows emacs to return to a subprocess that attached
     to the current emacs as a cheaper than starting a whole new process.  This
     is set up by KEPTEDITOR.COM.  */
  unsigned long parent_id, foster_parent_id;
  char *fpid_string;

  fpid_string = sys_getenv ("EMACS_PARENT_PID");
  if (fpid_string != NULL)
    {
      sscanf (fpid_string, "%x", &foster_parent_id);
      if (foster_parent_id != 0)
	parent_id = foster_parent_id;
      else
	parent_id = getppid ();
    }
  else
    parent_id = getppid ();

  xfree (fpid_string);		/* On VMS, this was malloc'd */

  if (parent_id && parent_id != 0xffffffff)
    {
      SIGTYPE (*oldsig)() = signal (SIGINT, SIG_IGN);
      int status = LIB$ATTACH (&parent_id) & 1;
      signal (SIGINT, oldsig);
      return status;
    }
  else
    {
      struct {
	int	l;
	char	*a;
      } d_prompt;
      d_prompt.l = sizeof ("Emacs: ");		/* Our special prompt */
      d_prompt.a = "Emacs: ";			/* Just a reminder */
      LIB$SPAWN (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &d_prompt, 0);
      return 1;
    }
  return -1;
#else /* not VMS */
#if defined (SIGTSTP) && !defined (MSDOS)

  {
    int pgrp = EMACS_GETPGRP (0);
    EMACS_KILLPG (pgrp, SIGTSTP);
  }

#else /* No SIGTSTP */
#ifdef USG_JOBCTRL /* If you don't know what this is don't mess with it */
  ptrace (0, 0, 0, 0);		/* set for ptrace - caught by csh */
  kill (getpid (), SIGQUIT);

#else /* No SIGTSTP or USG_JOBCTRL */

/* On a system where suspending is not implemented,
   instead fork a subshell and let it talk directly to the terminal
   while we wait.  */
  sys_subshell ();

#endif /* no USG_JOBCTRL */
#endif /* no SIGTSTP */
#endif /* not VMS */
}

/* Fork a subshell.  */

#ifndef MAC_OS8
void
sys_subshell ()
{
#ifndef VMS
#ifdef DOS_NT	/* Demacs 1.1.2 91/10/20 Manabu Higashida */
  int st;
  char oldwd[MAXPATHLEN+1]; /* Fixed length is safe on MSDOS.  */
#endif
  int pid;
  struct save_signal saved_handlers[5];
  Lisp_Object dir;
  unsigned char *str = 0;
  int len;

  saved_handlers[0].code = SIGINT;
  saved_handlers[1].code = SIGQUIT;
  saved_handlers[2].code = SIGTERM;
#ifdef SIGIO
  saved_handlers[3].code = SIGIO;
  saved_handlers[4].code = 0;
#else
  saved_handlers[3].code = 0;
#endif

  /* Mentioning current_buffer->buffer would mean including buffer.h,
     which somehow wedges the hp compiler.  So instead...  */

  dir = intern ("default-directory");
  if (NILP (Fboundp (dir)))
    goto xyzzy;
  dir = Fsymbol_value (dir);
  if (!STRINGP (dir))
    goto xyzzy;

  dir = expand_and_dir_to_file (Funhandled_file_name_directory (dir), Qnil);
  str = (unsigned char *) alloca (SCHARS (dir) + 2);
  len = SCHARS (dir);
  bcopy (SDATA (dir), str, len);
  if (str[len - 1] != '/') str[len++] = '/';
  str[len] = 0;
 xyzzy:

#ifdef DOS_NT
  pid = 0;
#if __DJGPP__ > 1
  save_signal_handlers (saved_handlers);
  synch_process_alive = 1;
#endif /* __DJGPP__ > 1 */
#else
  pid = vfork ();
  if (pid == -1)
    error ("Can't spawn subshell");
#endif

  if (pid == 0)
    {
      char *sh = 0;

#ifdef DOS_NT    /* MW, Aug 1993 */
      getwd (oldwd);
      if (sh == 0)
	sh = (char *) egetenv ("SUSPEND");	/* KFS, 1994-12-14 */
#endif
      if (sh == 0)
	sh = (char *) egetenv ("SHELL");
      if (sh == 0)
	sh = "sh";

      /* Use our buffer's default directory for the subshell.  */
      if (str)
	chdir ((char *) str);

#ifdef subprocesses
      close_process_descs ();	/* Close Emacs's pipes/ptys */
#endif

#ifdef SET_EMACS_PRIORITY
      {
	extern EMACS_INT emacs_priority;

	if (emacs_priority < 0)
	  nice (-emacs_priority);
      }
#endif

#ifdef MSDOS    /* Demacs 1.1.2 91/10/20 Manabu Higashida */
      {
	char *epwd = getenv ("PWD");
	char old_pwd[MAXPATHLEN+1+4];

	/* If PWD is set, pass it with corrected value.  */
	if (epwd)
	  {
	    strcpy (old_pwd, epwd);
	    if (str[len - 1] == '/')
	      str[len - 1] = '\0';
	    setenv ("PWD", str, 1);
	  }
	st = system (sh);
	chdir (oldwd);
	if (epwd)
	  putenv (old_pwd);	/* restore previous value */
      }
#if 0	/* This is also reported if last command executed in subshell failed, KFS */
      if (st)
	report_file_error ("Can't execute subshell", Fcons (build_string (sh), Qnil));
#endif
#else /* not MSDOS */
#ifdef  WINDOWSNT
      /* Waits for process completion */
      pid = _spawnlp (_P_WAIT, sh, sh, NULL);
      chdir (oldwd);
      if (pid == -1)
	write (1, "Can't execute subshell", 22);
#else   /* not WINDOWSNT */
      execlp (sh, sh, (char *) 0);
      write (1, "Can't execute subshell", 22);
      _exit (1);
#endif  /* not WINDOWSNT */
#endif /* not MSDOS */
    }

  /* Do this now if we did not do it before.  */
#if !defined (MSDOS) || __DJGPP__ == 1
  save_signal_handlers (saved_handlers);
  synch_process_alive = 1;
#endif

#ifndef DOS_NT
  wait_for_termination (pid);
#endif
  restore_signal_handlers (saved_handlers);
  synch_process_alive = 0;
#endif /* !VMS */
}
#endif /* !MAC_OS8 */

static void
save_signal_handlers (saved_handlers)
     struct save_signal *saved_handlers;
{
  while (saved_handlers->code)
    {
      saved_handlers->handler
	= (SIGTYPE (*) P_ ((int))) signal (saved_handlers->code, SIG_IGN);
      saved_handlers++;
    }
}

static void
restore_signal_handlers (saved_handlers)
     struct save_signal *saved_handlers;
{
  while (saved_handlers->code)
    {
      signal (saved_handlers->code, saved_handlers->handler);
      saved_handlers++;
    }
}

#ifdef F_SETFL

int old_fcntl_flags;

void
init_sigio (fd)
     int fd;
{
#ifdef FASYNC
  old_fcntl_flags = fcntl (fd, F_GETFL, 0) & ~FASYNC;
  fcntl (fd, F_SETFL, old_fcntl_flags | FASYNC);
#endif
  interrupts_deferred = 0;
}

static void
reset_sigio ()
{
  unrequest_sigio ();
}

#ifdef FASYNC		/* F_SETFL does not imply existence of FASYNC */

void
request_sigio ()
{
  if (noninteractive || read_socket_hook)
    return;

#ifdef SIGWINCH
  sigunblock (sigmask (SIGWINCH));
#endif
  fcntl (input_fd, F_SETFL, old_fcntl_flags | FASYNC);

  interrupts_deferred = 0;
}

void
unrequest_sigio ()
{
  if (noninteractive || read_socket_hook)
    return;

#ifdef SIGWINCH
  sigblock (sigmask (SIGWINCH));
#endif
  fcntl (input_fd, F_SETFL, old_fcntl_flags);
  interrupts_deferred = 1;
}

#else /* no FASYNC */
#ifdef STRIDE		/* Stride doesn't have FASYNC - use FIOASYNC */

void
request_sigio ()
{
  int on = 1;

  if (noninteractive || read_socket_hook)
    return;

  ioctl (input_fd, FIOASYNC, &on);
  interrupts_deferred = 0;
}

void
unrequest_sigio ()
{
  int off = 0;

  if (noninteractive || read_socket_hook)
    return;

  ioctl (input_fd, FIOASYNC, &off);
  interrupts_deferred = 1;
}

#else /* not FASYNC, not STRIDE */

#ifdef _CX_UX

#include <termios.h>

void
request_sigio ()
{
  int on = 1;
  sigset_t st;

  if (noninteractive || read_socket_hook)
    return;

  sigemptyset (&st);
  sigaddset (&st, SIGIO);
  ioctl (input_fd, FIOASYNC, &on);
  interrupts_deferred = 0;
  sigprocmask (SIG_UNBLOCK, &st, (sigset_t *)0);
}

void
unrequest_sigio ()
{
  int off = 0;

  if (noninteractive || read_socket_hook)
    return;

  ioctl (input_fd, FIOASYNC, &off);
  interrupts_deferred = 1;
}

#else /* ! _CX_UX */
#ifndef MSDOS

void
request_sigio ()
{
  if (noninteractive || read_socket_hook)
    return;

#ifndef VMS
  croak ("request_sigio");
#else
  SYS$SETAST(1);
#endif
}

void
unrequest_sigio ()
{
  if (noninteractive || read_socket_hook)
    return;

#ifndef VMS
  croak ("unrequest_sigio");
#else
  sys$setast(0);
#endif
}

#endif /* MSDOS */
#endif /* _CX_UX */
#endif /* STRIDE */
#endif /* FASYNC */
#endif /* F_SETFL */

/* Saving and restoring the process group of Emacs's terminal.  */

#ifdef BSD_PGRPS

/* The process group of which Emacs was a member when it initially
   started.

   If Emacs was in its own process group (i.e. inherited_pgroup ==
   getpid ()), then we know we're running under a shell with job
   control (Emacs would never be run as part of a pipeline).
   Everything is fine.

   If Emacs was not in its own process group, then we know we're
   running under a shell (or a caller) that doesn't know how to
   separate itself from Emacs (like sh).  Emacs must be in its own
   process group in order to receive SIGIO correctly.  In this
   situation, we put ourselves in our own pgroup, forcibly set the
   tty's pgroup to our pgroup, and make sure to restore and reinstate
   the tty's pgroup just like any other terminal setting.  If
   inherited_group was not the tty's pgroup, then we'll get a
   SIGTTmumble when we try to change the tty's pgroup, and a CONT if
   it goes foreground in the future, which is what should happen.  */
int inherited_pgroup;

/* Split off the foreground process group to Emacs alone.
   When we are in the foreground, but not started in our own process
   group, redirect the TTY to point to our own process group.  We need
   to be in our own process group to receive SIGIO properly.  */
void
narrow_foreground_group ()
{
  int me = getpid ();

  setpgrp (0, inherited_pgroup);
  if (inherited_pgroup != me)
    EMACS_SET_TTY_PGRP (input_fd, &me);
  setpgrp (0, me);
}

/* Set the tty to our original foreground group.  */
void
widen_foreground_group ()
{
  if (inherited_pgroup != getpid ())
    EMACS_SET_TTY_PGRP (input_fd, &inherited_pgroup);
  setpgrp (0, inherited_pgroup);
}

#endif /* BSD_PGRPS */

/* Getting and setting emacs_tty structures.  */

/* Set *TC to the parameters associated with the terminal FD.
   Return zero if all's well, or -1 if we ran into an error we
   couldn't deal with.  */
int
emacs_get_tty (fd, settings)
     int fd;
     struct emacs_tty *settings;
{
  /* Retrieve the primary parameters - baud rate, character size, etcetera.  */
#ifdef HAVE_TCATTR
  /* We have those nifty POSIX tcmumbleattr functions.  */
  bzero (&settings->main, sizeof (settings->main));
  if (tcgetattr (fd, &settings->main) < 0)
    return -1;

#else
#ifdef HAVE_TERMIO
  /* The SYSV-style interface?  */
  if (ioctl (fd, TCGETA, &settings->main) < 0)
    return -1;

#else
#ifdef VMS
  /* Vehemently Monstrous System?  :-)  */
  if (! (SYS$QIOW (0, fd, IO$_SENSEMODE, settings, 0, 0,
		   &settings->main.class, 12, 0, 0, 0, 0)
	 & 1))
    return -1;

#else
#ifndef DOS_NT
  /* I give up - I hope you have the BSD ioctls.  */
  if (ioctl (fd, TIOCGETP, &settings->main) < 0)
    return -1;
#endif /* not DOS_NT */
#endif
#endif
#endif

  /* Suivant - Do we have to get struct ltchars data?  */
#ifdef HAVE_LTCHARS
  if (ioctl (fd, TIOCGLTC, &settings->ltchars) < 0)
    return -1;
#endif

  /* How about a struct tchars and a wordful of lmode bits?  */
#ifdef HAVE_TCHARS
  if (ioctl (fd, TIOCGETC, &settings->tchars) < 0
      || ioctl (fd, TIOCLGET, &settings->lmode) < 0)
    return -1;
#endif

  /* We have survived the tempest.  */
  return 0;
}


/* Set the parameters of the tty on FD according to the contents of
   *SETTINGS.  If FLUSHP is non-zero, we discard input.
   Return 0 if all went well, and -1 if anything failed.  */

int
emacs_set_tty (fd, settings, flushp)
     int fd;
     struct emacs_tty *settings;
     int flushp;
{
  /* Set the primary parameters - baud rate, character size, etcetera.  */
#ifdef HAVE_TCATTR
  int i;
  /* We have those nifty POSIX tcmumbleattr functions.
     William J. Smith <wjs@wiis.wang.com> writes:
     "POSIX 1003.1 defines tcsetattr to return success if it was
     able to perform any of the requested actions, even if some
     of the requested actions could not be performed.
     We must read settings back to ensure tty setup properly.
     AIX requires this to keep tty from hanging occasionally."  */
  /* This make sure that we don't loop indefinitely in here.  */
  for (i = 0 ; i < 10 ; i++)
    if (tcsetattr (fd, flushp ? TCSAFLUSH : TCSADRAIN, &settings->main) < 0)
      {
	if (errno == EINTR)
	  continue;
	else
	  return -1;
      }
    else
      {
	struct termios new;

	bzero (&new, sizeof (new));
	/* Get the current settings, and see if they're what we asked for.  */
	tcgetattr (fd, &new);
	/* We cannot use memcmp on the whole structure here because under
	 * aix386 the termios structure has some reserved field that may
	 * not be filled in.
	 */
	if (   new.c_iflag == settings->main.c_iflag
	    && new.c_oflag == settings->main.c_oflag
	    && new.c_cflag == settings->main.c_cflag
	    && new.c_lflag == settings->main.c_lflag
	    && memcmp (new.c_cc, settings->main.c_cc, NCCS) == 0)
	  break;
	else
	  continue;
      }

#else
#ifdef HAVE_TERMIO
  /* The SYSV-style interface?  */
  if (ioctl (fd, flushp ? TCSETAF : TCSETAW, &settings->main) < 0)
    return -1;

#else
#ifdef VMS
  /* Vehemently Monstrous System?  :-)  */

  if (! (SYS$QIOW (0, fd, IO$_SETMODE,
		   settings,	/* &input_iosb */
		   /* The commented out "&input_iosb" is superstitious
		      reversion to Levitte's work (I could not find
		      any useful information in the change logs
		      pertaining to its use, maybe it was a cut and
		      paste error when code from systty.h was moved
		      here).  --ttn */
		   0, 0,
		   &settings->main.class, 12, 0, 0, 0, 0)
	 & 1))
      return -1;

#else
#ifndef DOS_NT
  /* I give up - I hope you have the BSD ioctls.  */
  if (ioctl (fd, (flushp) ? TIOCSETP : TIOCSETN, &settings->main) < 0)
    return -1;
#endif /* not DOS_NT */

#endif
#endif
#endif

  /* Suivant - Do we have to get struct ltchars data?  */
#ifdef HAVE_LTCHARS
  if (ioctl (fd, TIOCSLTC, &settings->ltchars) < 0)
    return -1;
#endif

  /* How about a struct tchars and a wordful of lmode bits?  */
#ifdef HAVE_TCHARS
  if (ioctl (fd, TIOCSETC, &settings->tchars) < 0
      || ioctl (fd, TIOCLSET, &settings->lmode) < 0)
    return -1;
#endif

  /* We have survived the tempest.  */
  return 0;
}


/* The initial tty mode bits */
struct emacs_tty old_tty;

/* 1 if we have been through init_sys_modes.  */
int term_initted;

/* 1 if outer tty status has been recorded.  */
int old_tty_valid;

#ifdef BSD4_1
/* BSD 4.1 needs to keep track of the lmode bits in order to start
   sigio.  */
int lmode;
#endif

#ifndef F_SETOWN_BUG
#ifdef F_SETOWN
int old_fcntl_owner;
#endif /* F_SETOWN */
#endif /* F_SETOWN_BUG */

/* This may also be defined in stdio,
   but if so, this does no harm,
   and using the same name avoids wasting the other one's space.  */

#ifdef nec_ews_svr4
extern char *_sobuf ;
#else
#if defined (USG) || defined (DGUX)
unsigned char _sobuf[BUFSIZ+8];
#else
char _sobuf[BUFSIZ];
#endif
#endif

#ifdef HAVE_LTCHARS
static struct ltchars new_ltchars = {-1,-1,-1,-1,-1,-1};
#endif
#ifdef HAVE_TCHARS
static struct tchars new_tchars = {-1,-1,-1,-1,-1,-1};
#endif

void sys_access_reinit (void);

void
init_sys_modes ()
{
  struct emacs_tty tty;

#ifdef VMS
#if 0
  static int oob_chars[2] = {0, 1 << 7}; /* catch C-g's */
  extern int (*interrupt_signal) ();
#endif
#endif

  Vtty_erase_char = Qnil;

  if (noninteractive)
    return;

#ifdef VMS
  if (!input_ef)
    {
      extern int get_kbd_event_flag (void);
      input_ef = get_kbd_event_flag ();
    }
  SYS$CLREF (input_ef);
  if (!timer_ef)
    {
      extern int get_timer_event_flag (void);
      timer_ef = get_timer_event_flag ();
    }
  SYS$CLREF (timer_ef);
  if (input_ef / 32 != timer_ef / 32)
    croak ("Input and timer event flags in different clusters.");
  timer_eflist = (((unsigned) 1 << (input_ef % 32)) |
		  ((unsigned) 1 << (timer_ef % 32)));
#ifndef VMS4_4
#ifdef VMS_NEED_SYS_ACCESS
  sys_access_reinit ();
#endif
#endif
#endif /* VMS */

#ifdef BSD_PGRPS
  if (! read_socket_hook && EQ (Vwindow_system, Qnil))
    narrow_foreground_group ();
#endif

#ifdef HAVE_WINDOW_SYSTEM
  /* Emacs' window system on MSDOG uses the `internal terminal' and therefore
     needs the initialization code below.  */
  if (!read_socket_hook && EQ (Vwindow_system, Qnil))
#endif
    {
      EMACS_GET_TTY (input_fd, &old_tty);

      old_tty_valid = 1;

      tty = old_tty;

#if defined (HAVE_TERMIO) || defined (HAVE_TERMIOS)
      XSETINT (Vtty_erase_char, old_tty.main.c_cc[VERASE]);

#ifdef DGUX
      /* This allows meta to be sent on 8th bit.  */
      tty.main.c_iflag &= ~INPCK;	/* don't check input for parity */
#endif
      tty.main.c_iflag |= (IGNBRK);	/* Ignore break condition */
      tty.main.c_iflag &= ~ICRNL;	/* Disable map of CR to NL on input */
#ifdef INLCR  /* I'm just being cautious,
		 since I can't check how widespread INLCR is--rms.  */
      tty.main.c_iflag &= ~INLCR;	/* Disable map of NL to CR on input */
#endif
#ifdef ISTRIP
      tty.main.c_iflag &= ~ISTRIP;	/* don't strip 8th bit on input */
#endif
      tty.main.c_lflag &= ~ECHO;	/* Disable echo */
      tty.main.c_lflag &= ~ICANON;	/* Disable erase/kill processing */
#ifdef IEXTEN
      tty.main.c_lflag &= ~IEXTEN;	/* Disable other editing characters.  */
#endif
      tty.main.c_lflag |= ISIG;	/* Enable signals */
      if (flow_control)
	{
	  tty.main.c_iflag |= IXON;	/* Enable start/stop output control */
#ifdef IXANY
	  tty.main.c_iflag &= ~IXANY;
#endif /* IXANY */
	}
      else
	tty.main.c_iflag &= ~IXON;	/* Disable start/stop output control */
      tty.main.c_oflag &= ~ONLCR;	/* Disable map of NL to CR-NL
					   on output */
      tty.main.c_oflag &= ~TAB3;	/* Disable tab expansion */
#ifdef CS8
      if (meta_key)
	{
	  tty.main.c_cflag |= CS8;	/* allow 8th bit on input */
	  tty.main.c_cflag &= ~PARENB;/* Don't check parity */
	}
#endif
      tty.main.c_cc[VINTR] = quit_char;	/* C-g (usually) gives SIGINT */
      /* Set up C-g for both SIGQUIT and SIGINT.
	 We don't know which we will get, but we handle both alike
	 so which one it really gives us does not matter.  */
      tty.main.c_cc[VQUIT] = quit_char;
      tty.main.c_cc[VMIN] = 1;	/* Input should wait for at least 1 char */
      tty.main.c_cc[VTIME] = 0;	/* no matter how long that takes.  */
#ifdef VSWTCH
      tty.main.c_cc[VSWTCH] = CDISABLE;	/* Turn off shell layering use
					   of C-z */
#endif /* VSWTCH */

#if defined (mips) || defined (HAVE_TCATTR)
#ifdef VSUSP
      tty.main.c_cc[VSUSP] = CDISABLE;	/* Turn off mips handling of C-z.  */
#endif /* VSUSP */
#ifdef V_DSUSP
      tty.main.c_cc[V_DSUSP] = CDISABLE; /* Turn off mips handling of C-y.  */
#endif /* V_DSUSP */
#ifdef VDSUSP /* Some systems have VDSUSP, some have V_DSUSP.  */
      tty.main.c_cc[VDSUSP] = CDISABLE;
#endif /* VDSUSP */
#ifdef VLNEXT
      tty.main.c_cc[VLNEXT] = CDISABLE;
#endif /* VLNEXT */
#ifdef VREPRINT
      tty.main.c_cc[VREPRINT] = CDISABLE;
#endif /* VREPRINT */
#ifdef VWERASE
      tty.main.c_cc[VWERASE] = CDISABLE;
#endif /* VWERASE */
#ifdef VDISCARD
      tty.main.c_cc[VDISCARD] = CDISABLE;
#endif /* VDISCARD */

      if (flow_control)
	{
#ifdef VSTART
	  tty.main.c_cc[VSTART] = '\021';
#endif /* VSTART */
#ifdef VSTOP
	  tty.main.c_cc[VSTOP] = '\023';
#endif /* VSTOP */
	}
      else
	{
#ifdef VSTART
	  tty.main.c_cc[VSTART] = CDISABLE;
#endif /* VSTART */
#ifdef VSTOP
	  tty.main.c_cc[VSTOP] = CDISABLE;
#endif /* VSTOP */
	}
#endif /* mips or HAVE_TCATTR */

#ifdef SET_LINE_DISCIPLINE
      /* Need to explicitly request TERMIODISC line discipline or
         Ultrix's termios does not work correctly.  */
      tty.main.c_line = SET_LINE_DISCIPLINE;
#endif
#ifdef AIX
#ifndef IBMR2AIX
      /* AIX enhanced edit loses NULs, so disable it.  */
      tty.main.c_line = 0;
      tty.main.c_iflag &= ~ASCEDIT;
#else
      tty.main.c_cc[VSTRT] = CDISABLE;
      tty.main.c_cc[VSTOP] = CDISABLE;
      tty.main.c_cc[VSUSP] = CDISABLE;
      tty.main.c_cc[VDSUSP] = CDISABLE;
#endif /* IBMR2AIX */
      if (flow_control)
	{
#ifdef VSTART
	  tty.main.c_cc[VSTART] = '\021';
#endif /* VSTART */
#ifdef VSTOP
	  tty.main.c_cc[VSTOP] = '\023';
#endif /* VSTOP */
	}
      /* Also, PTY overloads NUL and BREAK.
	 don't ignore break, but don't signal either, so it looks like NUL.
	 This really serves a purpose only if running in an XTERM window
	 or via TELNET or the like, but does no harm elsewhere.  */
      tty.main.c_iflag &= ~IGNBRK;
      tty.main.c_iflag &= ~BRKINT;
#endif
#else /* if not HAVE_TERMIO */
#ifdef VMS
      tty.main.tt_char |= TT$M_NOECHO;
      if (meta_key)
	tty.main.tt_char |= TT$M_EIGHTBIT;
      if (flow_control)
	tty.main.tt_char |= TT$M_TTSYNC;
      else
	tty.main.tt_char &= ~TT$M_TTSYNC;
      tty.main.tt2_char |= TT2$M_PASTHRU | TT2$M_XON;
#else /* not VMS (BSD, that is) */
#ifndef DOS_NT
      XSETINT (Vtty_erase_char, tty.main.sg_erase);
      tty.main.sg_flags &= ~(ECHO | CRMOD | XTABS);
      if (meta_key)
	tty.main.sg_flags |= ANYP;
      tty.main.sg_flags |= interrupt_input ? RAW : CBREAK;
#endif /* not DOS_NT */
#endif /* not VMS (BSD, that is) */
#endif /* not HAVE_TERMIO */

      /* If going to use CBREAK mode, we must request C-g to interrupt
	 and turn off start and stop chars, etc.  If not going to use
	 CBREAK mode, do this anyway so as to turn off local flow
	 control for user coming over network on 4.2; in this case,
	 only t_stopc and t_startc really matter.  */
#ifndef HAVE_TERMIO
#ifdef HAVE_TCHARS
      /* Note: if not using CBREAK mode, it makes no difference how we
	 set this */
      tty.tchars = new_tchars;
      tty.tchars.t_intrc = quit_char;
      if (flow_control)
	{
	  tty.tchars.t_startc = '\021';
	  tty.tchars.t_stopc = '\023';
	}

      tty.lmode = LDECCTQ | LLITOUT | LPASS8 | LNOFLSH | old_tty.lmode;
#ifdef ultrix
      /* Under Ultrix 4.2a, leaving this out doesn't seem to hurt
	 anything, and leaving it in breaks the meta key.  Go figure.  */
      tty.lmode &= ~LLITOUT;
#endif

#ifdef BSD4_1
      lmode = tty.lmode;
#endif

#endif /* HAVE_TCHARS */
#endif /* not HAVE_TERMIO */

#ifdef HAVE_LTCHARS
      tty.ltchars = new_ltchars;
#endif /* HAVE_LTCHARS */
#ifdef MSDOS	/* Demacs 1.1.2 91/10/20 Manabu Higashida, MW Aug 1993 */
      if (!term_initted)
	internal_terminal_init ();
      dos_ttraw ();
#endif

      EMACS_SET_TTY (input_fd, &tty, 0);

      /* This code added to insure that, if flow-control is not to be used,
	 we have an unlocked terminal at the start. */

#ifdef TCXONC
      if (!flow_control) ioctl (input_fd, TCXONC, 1);
#endif
#ifndef APOLLO
#ifdef TIOCSTART
      if (!flow_control) ioctl (input_fd, TIOCSTART, 0);
#endif
#endif

#if defined (HAVE_TERMIOS) || defined (HPUX9)
#ifdef TCOON
      if (!flow_control) tcflow (input_fd, TCOON);
#endif
#endif

#ifdef AIXHFT
      hft_init ();
#ifdef IBMR2AIX
      {
	/* IBM's HFT device usually thinks a ^J should be LF/CR.  We need it
	   to be only LF.  This is the way that is done. */
	struct termio tty;

	if (ioctl (1, HFTGETID, &tty) != -1)
	  write (1, "\033[20l", 5);
      }
#endif
#endif /* AIXHFT */

#ifdef VMS
/*  Appears to do nothing when in PASTHRU mode.
      SYS$QIOW (0, input_fd, IO$_SETMODE|IO$M_OUTBAND, 0, 0, 0,
		interrupt_signal, oob_chars, 0, 0, 0, 0);
*/
      queue_kbd_input ();
#endif /* VMS */
    }

#ifdef F_SETFL
#ifndef F_SETOWN_BUG
#ifdef F_GETOWN		/* F_SETFL does not imply existence of F_GETOWN */
  if (interrupt_input
      && ! read_socket_hook && EQ (Vwindow_system, Qnil))
    {
      old_fcntl_owner = fcntl (input_fd, F_GETOWN, 0);
      fcntl (input_fd, F_SETOWN, getpid ());
      init_sigio (input_fd);
    }
#endif /* F_GETOWN */
#endif /* F_SETOWN_BUG */
#endif /* F_SETFL */

#ifdef BSD4_1
  if (interrupt_input)
    init_sigio (input_fd);
#endif

#ifdef VMS  /* VMS sometimes has this symbol but lacks setvbuf.  */
#undef _IOFBF
#endif
#ifdef _IOFBF
  /* This symbol is defined on recent USG systems.
     Someone says without this call USG won't really buffer the file
     even with a call to setbuf. */
  setvbuf (stdout, (char *) _sobuf, _IOFBF, sizeof _sobuf);
#else
  setbuf (stdout, (char *) _sobuf);
#endif
#ifdef HAVE_WINDOW_SYSTEM
  /* Emacs' window system on MSDOG uses the `internal terminal' and therefore
     needs the initialization code below.  */
  if (EQ (Vwindow_system, Qnil)
#ifndef WINDOWSNT
      /* When running in tty mode on NT/Win95, we have a read_socket
	 hook, but still need the rest of the initialization code below.  */
      && (! read_socket_hook)
#endif
      )
#endif
    set_terminal_modes ();

  if (!term_initted
      && FRAMEP (Vterminal_frame)
      && FRAME_TERMCAP_P (XFRAME (Vterminal_frame)))
    init_frame_faces (XFRAME (Vterminal_frame));

  if (term_initted && no_redraw_on_reenter)
    {
      if (display_completed)
	direct_output_forward_char (0);
    }
  else
    {
      frame_garbaged = 1;
      if (FRAMEP (Vterminal_frame))
	FRAME_GARBAGED_P (XFRAME (Vterminal_frame)) = 1;
    }

  term_initted = 1;
}

/* Return nonzero if safe to use tabs in output.
   At the time this is called, init_sys_modes has not been done yet.  */

int
tabs_safe_p ()
{
  struct emacs_tty tty;

  EMACS_GET_TTY (input_fd, &tty);
  return EMACS_TTY_TABS_OK (&tty);
}

/* Get terminal size from system.
   Store number of lines into *HEIGHTP and width into *WIDTHP.
   We store 0 if there's no valid information.  */

void
get_frame_size (widthp, heightp)
     int *widthp, *heightp;
{

#ifdef TIOCGWINSZ

  /* BSD-style.  */
  struct winsize size;

  if (ioctl (input_fd, TIOCGWINSZ, &size) == -1)
    *widthp = *heightp = 0;
  else
    {
      *widthp = size.ws_col;
      *heightp = size.ws_row;
    }

#else
#ifdef TIOCGSIZE

  /* SunOS - style.  */
  struct ttysize size;

  if (ioctl (input_fd, TIOCGSIZE, &size) == -1)
    *widthp = *heightp = 0;
  else
    {
      *widthp = size.ts_cols;
      *heightp = size.ts_lines;
    }

#else
#ifdef VMS

  /* Use a fresh channel since the current one may have stale info
     (for example, from prior to a suspend); and to avoid a dependency
     in the init sequence.  */
  int chan;
  struct sensemode tty;

  SYS$ASSIGN (&input_dsc, &chan, 0, 0);
  SYS$QIOW (0, chan, IO$_SENSEMODE, &tty, 0, 0,
            &tty.class, 12, 0, 0, 0, 0);
  SYS$DASSGN (chan);
  *widthp = tty.scr_wid;
  *heightp = tty.scr_len;

#else
#ifdef MSDOS
  *widthp = ScreenCols ();
  *heightp = ScreenRows ();
#else /* system doesn't know size */
  *widthp = 0;
  *heightp = 0;
#endif

#endif /* not VMS */
#endif /* not SunOS-style */
#endif /* not BSD-style */
}

/* Set the logical window size associated with descriptor FD
   to HEIGHT and WIDTH.  This is used mainly with ptys.  */

int
set_window_size (fd, height, width)
     int fd, height, width;
{
#ifdef TIOCSWINSZ

  /* BSD-style.  */
  struct winsize size;
  size.ws_row = height;
  size.ws_col = width;

  if (ioctl (fd, TIOCSWINSZ, &size) == -1)
    return 0; /* error */
  else
    return 1;

#else
#ifdef TIOCSSIZE

  /* SunOS - style.  */
  struct ttysize size;
  size.ts_lines = height;
  size.ts_cols = width;

  if (ioctl (fd, TIOCGSIZE, &size) == -1)
    return 0;
  else
    return 1;
#else
  return -1;
#endif /* not SunOS-style */
#endif /* not BSD-style */
}


/* Prepare the terminal for exiting Emacs; move the cursor to the
   bottom of the frame, turn off interrupt-driven I/O, etc.  */
void
reset_sys_modes ()
{
  struct frame *sf;

  if (noninteractive)
    {
      fflush (stdout);
      return;
    }
  if (!term_initted)
    return;
#ifdef HAVE_WINDOW_SYSTEM
  /* Emacs' window system on MSDOG uses the `internal terminal' and therefore
     needs the clean-up code below.  */
  if (!EQ (Vwindow_system, Qnil)
#ifndef WINDOWSNT
      /* When running in tty mode on NT/Win95, we have a read_socket
	 hook, but still need the rest of the clean-up code below.  */
      || read_socket_hook
#endif
      )
    return;
#endif
  sf = SELECTED_FRAME ();
  cursor_to (FRAME_LINES (sf) - 1, 0);
  clear_end_of_line (FRAME_COLS (sf));
  /* clear_end_of_line may move the cursor */
  cursor_to (FRAME_LINES (sf) - 1, 0);
#if defined (IBMR2AIX) && defined (AIXHFT)
  {
    /* HFT devices normally use ^J as a LF/CR.  We forced it to
       do the LF only.  Now, we need to reset it. */
    struct termio tty;

    if (ioctl (1, HFTGETID, &tty) != -1)
      write (1, "\033[20h", 5);
  }
#endif

  reset_terminal_modes ();
  fflush (stdout);
#ifdef BSD_SYSTEM
#ifndef BSD4_1
  /* Avoid possible loss of output when changing terminal modes.  */
  fsync (fileno (stdout));
#endif
#endif

#ifdef F_SETFL
#ifndef F_SETOWN_BUG
#ifdef F_SETOWN		/* F_SETFL does not imply existence of F_SETOWN */
  if (interrupt_input)
    {
      reset_sigio ();
      fcntl (input_fd, F_SETOWN, old_fcntl_owner);
    }
#endif /* F_SETOWN */
#endif /* F_SETOWN_BUG */
#ifdef O_NDELAY
  fcntl (input_fd, F_SETFL, fcntl (input_fd, F_GETFL, 0) & ~O_NDELAY);
#endif
#endif /* F_SETFL */
#ifdef BSD4_1
  if (interrupt_input)
    reset_sigio ();
#endif /* BSD4_1 */

#ifdef VMS
  /* Disable the AST which would otherwise hang on after exit,
     requiring further input (discarded anyway) before returning to
     the DCL prompt.  */
  end_kbd_input ();
#endif /* VMS */

  if (old_tty_valid)
    while (EMACS_SET_TTY (input_fd, &old_tty, 0) < 0 && errno == EINTR)
      ;

#ifdef MSDOS	/* Demacs 1.1.2 91/10/20 Manabu Higashida */
  dos_ttcooked ();
#endif

#ifdef SET_LINE_DISCIPLINE
  /* Ultrix's termios *ignores* any line discipline except TERMIODISC.
     A different old line discipline is therefore not restored, yet.
     Restore the old line discipline by hand.  */
  ioctl (0, TIOCSETD, &old_tty.main.c_line);
#endif

#ifdef AIXHFT
  hft_reset ();
#endif

#ifdef BSD_PGRPS
  widen_foreground_group ();
#endif
}

#ifdef HAVE_PTYS

/* Set up the proper status flags for use of a pty.  */

void
setup_pty (fd)
     int fd;
{
  /* I'm told that TOICREMOTE does not mean control chars
     "can't be sent" but rather that they don't have
     input-editing or signaling effects.
     That should be good, because we have other ways
     to do those things in Emacs.
     However, telnet mode seems not to work on 4.2.
     So TIOCREMOTE is turned off now. */

  /* Under hp-ux, if TIOCREMOTE is turned on, some calls
     will hang.  In particular, the "timeout" feature (which
     causes a read to return if there is no data available)
     does this.  Also it is known that telnet mode will hang
     in such a way that Emacs must be stopped (perhaps this
     is the same problem).

     If TIOCREMOTE is turned off, then there is a bug in
     hp-ux which sometimes loses data.  Apparently the
     code which blocks the master process when the internal
     buffer fills up does not work.  Other than this,
     though, everything else seems to work fine.

     Since the latter lossage is more benign, we may as well
     lose that way.  -- cph */
#ifdef FIONBIO
#if defined(SYSV_PTYS) || defined(UNIX98_PTYS)
  {
    int on = 1;
    ioctl (fd, FIONBIO, &on);
  }
#endif
#endif
#ifdef IBMRTAIX
  /* On AIX, the parent gets SIGHUP when a pty attached child dies.  So, we */
  /* ignore SIGHUP once we've started a child on a pty.  Note that this may */
  /* cause EMACS not to die when it should, i.e., when its own controlling  */
  /* tty goes away.  I've complained to the AIX developers, and they may    */
  /* change this behavior, but I'm not going to hold my breath.             */
  signal (SIGHUP, SIG_IGN);
#endif
}
#endif /* HAVE_PTYS */

#ifdef VMS

#if 0
#define DEBUGAST
#endif

/* Assigning an input channel is done at the start of Emacs execution.
   This is called each time Emacs is resumed, also, but does nothing
   because input_chain is no longer zero.  */

void
init_vms_input (void)
{
  int status;

  if (input_fd == 0)
    {
      status = SYS$ASSIGN (&input_dsc, &input_fd,
			   0,	/* access control mode: kernel */
			   0);	/* mailbox name */
      if (! (status & 1))
	LIB$STOP (status);
    }
}

#if 0 /* ttn */
/* Deassigning the input channel is done before exiting.  */

void
stop_vms_input (void)
{
  SYS$DASSGN (input_fd);
  return;
}
#endif /* 0 */

static short input_buffer;

/* Request reading one character into the keyboard buffer.
   This is done as soon as the buffer becomes empty.  */

extern void kbd_input_ast (void);

static void
queue_kbd_input (void)
{
  int status;

#ifdef DEBUGAST
  fprintf (stderr, "queue_kbd_input()\n");
#endif

  stop_input = 0;
  status = SYS$QIO (0, input_fd, IO$_READVBLK,
		    &input_iosb, kbd_input_ast, 1,
		    &input_buffer, 1, 0, terminator_mask, 0, 0);
  ast_queued = 1;
}

/* Ast routine that is called when keyboard input comes in
   in accord with the SYS$QIO above.  */

#ifdef DEBUGAST
static int input_count;
extern Lisp_Object Vlast_event_frame;
extern Lisp_Object internal_last_event_frame;
#endif

extern void
kbd_input_ast (void)
{
  int old_errno = errno;

#ifdef DEBUGAST
  input_count++;
  fprintf (stderr, "\nDEBUGAST: AST # %d (ast_queued=%d stop_input=%d)\n",
	   input_count, ast_queued, stop_input);
  fprintf (stderr, " iosb offset=%x, stt=%x,%x,%x; char = %d\n",
	   input_iosb.offset, input_iosb.status,
	   input_iosb.termlen, input_iosb.term,
	   input_iosb.offset ? input_buffer : -1);
  sleep (1);
  if (input_count == 25)
    exit (1);
#endif

  ast_queued = 0;

  if (! stop_input)
    {
      extern EMACS_TIME *input_available_clear_time;
      static struct input_event buf[1];
      struct input_event hold_quit;
      register int passes = -1;

      if (input_available_clear_time)
	EMACS_SET_SECS_USECS (*input_available_clear_time, 0, 0);

      EVENT_INIT (hold_quit);
      hold_quit.kind = NO_EVENT;

      while (1)
	{
	  register int c = -1;
	  register int nread = 0;
	  int i;

	  passes++;

	  if (read_socket_hook)
	    {
	      nread = (*read_socket_hook) (0, passes == 0, &hold_quit);
	      if (nread > 0)
		continue;
	      if (hold_quit.kind != NO_EVENT)
		kbd_buffer_store_event (&hold_quit);
	      break;
	    }
	  else
	    {
#if 0
	      if (interrupt_input_blocked)
		{
		  interrupt_input_pending = 1;
		  return -1;
		}

	      interrupt_input_pending = 0;
#endif
	      if (passes == 0)
		{
		  if (input_iosb.offset)
		    c = input_buffer;
		  queue_kbd_input ();
		}
	      if (c >= 0)
		{
		  extern int meta_key;	/* in keyboard.c */

		  nread = 1;
		  EVENT_INIT (*buf);
		  buf->kind = ASCII_KEYSTROKE_EVENT;
		  buf->modifiers = 0;
		  if (meta_key == 1 && (c & 0x80))
		    buf->modifiers = meta_modifier;
		  if (meta_key != 2)
		    c &= ~0x80;

		  buf->code = c;
		  buf->frame_or_window = selected_frame;
		  buf->arg = Qnil;
		}
	    }

	  /* Scan the chars for C-g and store them in kbd_buffer.  */
	  for (i = 0; i < nread; i++)
	    {
#if 0 /* The donor's verbose decoder assumes the Emacs 21 event layout.  */
	      {
		extern Lisp_Object Qexternal_debugging_output;

		switch (buf[i].kind)
		  {
		  case no_event:
		    fprintf (stderr, "DEBUGAST: buf[%d] = no event\n", i);
		    break;
		  case ascii_keystroke:
		    fprintf (stderr,
			     "DEBUGAST: buf[%d] = ASCII keystroke with\n", i);
		    fprintf (stderr, "                 .code            = ");
		    Fprin1 (buf[i].code, Qexternal_debugging_output);
		    fprintf (stderr, "  \n");
		    fprintf (stderr, "                 .frame_or_window = ");
		    Fprin1 (buf[i].frame_or_window,
			    Qexternal_debugging_output);
		    fprintf (stderr, "\n");
		    fprintf (stderr,
			     "                 .timestamp       = %d\n",
			     buf[i].timestamp);
		    break;
		  case non_ascii_keystroke:
		    fprintf (stderr,
			     "DEBUGAST: buf[%d] = keystroke with\n", i);
		    fprintf (stderr, "                 .code            = ");
		    Fprin1 (buf[i].code, Qexternal_debugging_output);
		    fprintf (stderr, "  \n");
		    fprintf (stderr,
			     "                 .modifiers       = %d\n",
			     buf[i].modifiers);
		    fprintf (stderr, "                 .frame_or_window = ");
		    Fprin1 (buf[i].frame_or_window,
			    Qexternal_debugging_output);
		    fprintf (stderr, "\n");
		    fprintf (stderr,
			     "                 .timestamp       = %d\n",
			     buf[i].timestamp);
		    break;
		  case mouse_click:
		    fprintf (stderr,
			     "DEBUGAST: buf[%d] = mouse click with\n", i);
		    fprintf (stderr, "                 .code            = ");
		    Fprin1 (buf[i].code, Qexternal_debugging_output);
		    fprintf (stderr, "  \n");
		    fprintf (stderr,
			     "                 .modifiers       = %d\n",
			     buf[i].modifiers);
		    fprintf (stderr, "                 .frame_or_window = ");
		    Fprin1 (buf[i].frame_or_window,
			    Qexternal_debugging_output);
		    fprintf (stderr, "\n");
		    fprintf (stderr, "                 .x               = ");
		    Fprin1 (buf[i].x, Qexternal_debugging_output);
		    fprintf (stderr, "\n");
		    fprintf (stderr, "                 .y               = ");
		    Fprin1 (buf[i].y, Qexternal_debugging_output);
		    fprintf (stderr, "\n");
		    fprintf (stderr,
			     "                 .timestamp       = %d\n",
			     buf[i].timestamp);
		    break;
		  case scroll_bar_click:
		    fprintf (stderr,
			     "DEBUGAST: buf[%d] = mouse click with\n", i);
		    fprintf (stderr, "                 .code            = ");
		    Fprin1 (buf[i].code, Qexternal_debugging_output);
		    fprintf (stderr, "  \n");
		    fprintf (stderr,
			     "                 .modifiers       = %d\n",
			     buf[i].modifiers);
		    fprintf (stderr, "                 .part            = ");
		    switch (buf[i].part)
		      {
		      case scroll_bar_above_handle:
			fprintf (stderr, "scroll_bar_above_handle\n");
			break;
		      case scroll_bar_handle:
			fprintf (stderr, "scroll_bar_handle\n");
			break;
		      case scroll_bar_below_handle:
			fprintf (stderr, "scroll_bar_below_handle\n");
			break;
		      }
		    fprintf (stderr, "                 .frame_or_window = ");
		    Fprin1 (buf[i].frame_or_window,
			    Qexternal_debugging_output);
		    fprintf (stderr, "\n");
		    fprintf (stderr, "                 .x               = ");
		    Fprin1 (buf[i].x, Qexternal_debugging_output);
		    fprintf (stderr, "\n");
		    fprintf (stderr, "                 .y               = ");
		    Fprin1 (buf[i].y, Qexternal_debugging_output);
		    fprintf (stderr, "\n");
		    fprintf (stderr,
			     "                 .timestamp       = %d\n",
			     buf[i].timestamp);
		    break;
		  case selection_request_event:
		    fprintf (stderr,
			     "DEBUGAST: buf[%d] = selection request event with\n", i);
		    fprintf (stderr,
			     "                 .requestor       = %u\n",
			     SELECTION_EVENT_REQUESTOR (&buf[i]));
		    fprintf (stderr,
			     "                 .selection       = %u\n",
			     SELECTION_EVENT_SELECTION (&buf[i]));
		    fprintf (stderr,
			     "                 .target          = %u\n",
			     SELECTION_EVENT_TARGET (&buf[i]));
		    fprintf (stderr,
			     "                 .property        = %u\n",
			     SELECTION_EVENT_PROPERTY (&buf[i]));
		    fprintf (stderr,
			     "                 .time            = %u\n",
			     SELECTION_EVENT_TIME (&buf[i]));
		    break;
		  case selection_clear_event:
		    fprintf (stderr,
			     "DEBUGAST: buf[%d] = selection clear event with\n", i);
		    fprintf (stderr,
			     "                 .requestor       = %u\n",
			     SELECTION_EVENT_REQUESTOR (&buf[i]));
		    fprintf (stderr,
			     "                 .selection       = %u\n",
			     SELECTION_EVENT_SELECTION (&buf[i]));
		    fprintf (stderr,
			     "                 .target          = %u\n",
			     SELECTION_EVENT_TARGET (&buf[i]));
		    fprintf (stderr,
			     "                 .property        = %u\n",
			     SELECTION_EVENT_PROPERTY (&buf[i]));
		    fprintf (stderr,
			     "                 .time            = %u\n",
			     SELECTION_EVENT_TIME (&buf[i]));
		    break;
		  case delete_window_event:
		    fprintf (stderr,
			     "DEBUGAST: buf[%d] = delete window event with\n",
			     i);
		    fprintf (stderr, "                 .frame_or_window = ");
		    Fprin1 (buf[i].frame_or_window,
			    Qexternal_debugging_output);
		    break;
		  default:
		    fprintf (stderr,
			     "DEBUGAST: buf[%d] = some odd event with\n", i);
		  }
	      }
	      sleep (1);
#endif
	      kbd_buffer_store_event (&buf[i]);
	      /* Don't look at input that follows a C-g too closely.
		 This reduces lossage due to autorepeat on C-g.  */
	      if (buf[i].kind == ASCII_KEYSTROKE_EVENT
		  && buf[i].code == quit_char)
		break;
	    }

	  if (nread <= 0)
	    break;
	}
    }

#ifdef DEBUGAST
  fprintf (stderr, "DEBUGAST: setting event flag %d (queue_next_p=%d)\n",
	   input_ef, queue_next_p);
#endif
  sys$setef (input_ef);

  errno = old_errno;
}

static void
end_kbd_input (void)
{
#ifdef DEBUGAST
  fprintf (stderr, "At end_kbd_input.\n");
  sleep (1);
#endif
  /* Don't wait if suspending from kbd_buffer_store_event!  */
  if (LIB$AST_IN_PROG ())
    {
      SYS$CANCEL (input_fd);
      return;
    }

  if (ast_queued)
    {
      SYS$SETAST (0);
      /* Clear a flag, and tell ast routine above to set it.  */
      SYS$CLREF (input_ef);
      stop_input = 1;
      SYS$CANCEL (input_fd);
      SYS$SETAST (1);
#if 1
      SYS$WAITFR (input_ef);
#endif
      SYS$CLREF (input_ef);
      stop_input = 0;
    }
}

#endif /* VMS */

#ifdef VMS
/* The standard `sleep' routine works some other way
   and it stops working if you have ever quit out of it.
   This one continues to work.  */

void
sys_sleep (timeval)
     int timeval;
{
  int time [2];
  static int zero = 0;
  static int large = -10000000;

  LIB$EMUL (&timeval, &large, &zero, time); 	  /* Convert to VMS format */

  SYS$CANTIM (reqidt, 0);
  if (SYS$SETIMR (timer_ef, time, 0, reqidt) & 1) /* Set timer */
    SYS$WAITFR (timer_ef);	  /* Wait for timer expiry only */
}

void
init_sigio2 (fd)
  int fd;
{
  request_sigio ();
}

void
reset_sigio2 (void)
{
  unrequest_sigio ();
}

void
request_sigio2 (void)
{
  if (noninteractive)
    return;
  SYS$SETAST (1);
}

void
unrequest_sigio2 (void)
{
  if (noninteractive)
    return;
  SYS$SETAST (0);
}

#endif /* VMS */

/* Note that VMS compiler won't accept defined (CANNOT_DUMP).  */
#ifndef CANNOT_DUMP
#define NEED_STARTS
#endif

#ifndef SYSTEM_MALLOC
#ifndef NEED_STARTS
#define NEED_STARTS
#endif
#endif

#ifdef NEED_STARTS
/* Some systems that cannot dump also cannot implement these.  */

/*
 *	Return the address of the start of the text segment prior to
 *	doing an unexec.  After unexec the return value is undefined.
 *	See crt0.c for further explanation and _start.
 *
 */

#if !(defined (__NetBSD__) && defined (__ELF__))
#ifndef HAVE_TEXT_START
char *
start_of_text ()
{
#ifdef TEXT_START
  return ((char *) TEXT_START);
#else
#ifdef GOULD
  extern csrt ();
  return ((char *) csrt);
#else /* not GOULD */
  extern int _start ();
  return ((char *) _start);
#endif /* GOULD */
#endif /* TEXT_START */
}
#endif /* not HAVE_TEXT_START */
#endif

/*
 *	Return the address of the start of the data segment prior to
 *	doing an unexec.  After unexec the return value is undefined.
 *	See crt0.c for further information and definition of data_start.
 *
 *	Apparently, on BSD systems this is etext at startup.  On
 *	USG systems (swapping) this is highly mmu dependent and
 *	is also dependent on whether or not the program is running
 *	with shared text.  Generally there is a (possibly large)
 *	gap between end of text and start of data with shared text.
 *
 *	On Uniplus+ systems with shared text, data starts at a
 *	fixed address.  Each port (from a given oem) is generally
 *	different, and the specific value of the start of data can
 *	be obtained via the UniPlus+ specific "uvar" system call,
 *	however the method outlined in crt0.c seems to be more portable.
 *
 *	Probably what will have to happen when a USG unexec is available,
 *	at least on UniPlus, is temacs will have to be made unshared so
 *	that text and data are contiguous.  Then once loadup is complete,
 *	unexec will produce a shared executable where the data can be
 *	at the normal shared text boundary and the startofdata variable
 *	will be patched by unexec to the correct value.
 *
 */

#ifndef start_of_data
char *
start_of_data ()
{
#ifdef DATA_START
  return ((char *) DATA_START);
#else
#ifdef ORDINARY_LINK
  /*
   * This is a hack.  Since we're not linking crt0.c or pre_crt0.c,
   * data_start isn't defined.  We take the address of environ, which
   * is known to live at or near the start of the system crt0.c, and
   * we don't sweat the handful of bytes that might lose.
   */
  extern char **environ;

  return ((char *) &environ);
#else
  extern int data_start;
  return ((char *) &data_start);
#endif /* ORDINARY_LINK */
#endif /* DATA_START */
}
#endif /* start_of_data */
#endif /* NEED_STARTS (not CANNOT_DUMP or not SYSTEM_MALLOC) */


/* init_system_name sets up the string for the Lisp function
   system-name to return. */

#ifdef BSD4_1
#include <whoami.h>
#endif

extern Lisp_Object Vsystem_name;

#ifndef BSD4_1
#ifndef VMS
#ifdef HAVE_SOCKETS
#include <sys/socket.h>
#include <netdb.h>
#endif /* HAVE_SOCKETS */
#endif /* not VMS */
#endif /* not BSD4_1 */

#ifdef TRY_AGAIN
#ifndef HAVE_H_ERRNO
extern int h_errno;
#endif
#endif /* TRY_AGAIN */

void
init_system_name ()
{
#ifdef BSD4_1
  Vsystem_name = build_string (sysname);
#else
#ifdef VMS
  char *sp, *end;
  if ((sp = egetenv ("SYS$NODE")) == 0)
    Vsystem_name = build_string ("vax-vms");
  else if ((end = index (sp, ':')) == 0)
    Vsystem_name = build_string (sp);
  else
    Vsystem_name = make_string (sp, end - sp);
#else
#ifndef HAVE_GETHOSTNAME
  struct utsname uts;
  uname (&uts);
  Vsystem_name = build_string (uts.nodename);
#else /* HAVE_GETHOSTNAME */
  unsigned int hostname_size = 256;
  char *hostname = (char *) alloca (hostname_size);

  /* Try to get the host name; if the buffer is too short, try
     again.  Apparently, the only indication gethostname gives of
     whether the buffer was large enough is the presence or absence
     of a '\0' in the string.  Eech.  */
  for (;;)
    {
      gethostname (hostname, hostname_size - 1);
      hostname[hostname_size - 1] = '\0';

      /* Was the buffer large enough for the '\0'?  */
      if (strlen (hostname) < hostname_size - 1)
	break;

      hostname_size <<= 1;
      hostname = (char *) alloca (hostname_size);
    }
#ifdef HAVE_SOCKETS
  /* Turn the hostname into the official, fully-qualified hostname.
     Don't do this if we're going to dump; this can confuse system
     libraries on some machines and make the dumped emacs core dump. */
#ifndef CANNOT_DUMP
  if (initialized)
#endif /* not CANNOT_DUMP */
    if (! index (hostname, '.'))
      {
	struct hostent *hp;
	int count;
	for (count = 0;; count++)
	  {
#ifdef TRY_AGAIN
	    h_errno = 0;
#endif
	    hp = gethostbyname (hostname);
#ifdef TRY_AGAIN
	    if (! (hp == 0 && h_errno == TRY_AGAIN))
#endif
	      break;
	    if (count >= 5)
	      break;
	    Fsleep_for (make_number (1), Qnil);
	  }
	if (hp)
	  {
	    char *fqdn = (char *) hp->h_name;
#if 0
	    char *p;
#endif

	    if (!index (fqdn, '.'))
	      {
		/* We still don't have a fully qualified domain name.
		   Try to find one in the list of alternate names */
		char **alias = hp->h_aliases;
		while (*alias
		       && (!index (*alias, '.')
			   || !strcmp (*alias, "localhost.localdomain")))
		  alias++;
		if (*alias)
		  fqdn = *alias;
	      }
	    hostname = fqdn;
#if 0
	    /* Convert the host name to lower case.  */
	    /* Using ctype.h here would introduce a possible locale
	       dependence that is probably wrong for hostnames.  */
	    p = hostname;
	    while (*p)
	      {
		if (*p >= 'A' && *p <= 'Z')
		  *p += 'a' - 'A';
		p++;
	      }
#endif
	  }
      }
#endif /* HAVE_SOCKETS */
  /* We used to try using getdomainname here,
     but NIIBE Yutaka <gniibe@etl.go.jp> says that
     getdomainname gets the NIS/YP domain which often is not the same
     as in Internet domain name.  */
#if 0 /* Turned off because sysinfo is not really likely to return the
	 correct Internet domain.  */
#if (HAVE_SYSINFO && defined (SI_SRPC_DOMAIN))
  if (! index (hostname, '.'))
    {
      /* The hostname is not fully qualified.  Append the domain name.  */

      int hostlen = strlen (hostname);
      int domain_size = 256;

      for (;;)
	{
	  char *domain = (char *) alloca (domain_size + 1);
	  char *fqdn = (char *) alloca (hostlen + 1 + domain_size + 1);
	  int sys_domain_size = sysinfo (SI_SRPC_DOMAIN, domain, domain_size);
	  if (sys_domain_size <= 0)
	    break;
	  if (domain_size < sys_domain_size)
	    {
	      domain_size = sys_domain_size;
	      continue;
	    }
	  strcpy (fqdn, hostname);
	  if (domain[0] == '.')
	    strcpy (fqdn + hostlen, domain);
	  else if (domain[0] != 0)
	    {
	      fqdn[hostlen] = '.';
	      strcpy (fqdn + hostlen + 1, domain);
	    }
	  hostname = fqdn;
	  break;
	}
    }
#endif /* HAVE_SYSINFO && defined (SI_SRPC_DOMAIN) */
#endif /* 0 */
  Vsystem_name = build_string (hostname);
#endif /* HAVE_GETHOSTNAME */
#endif /* VMS */
#endif /* BSD4_1 */
  {
    unsigned char *p;
    for (p = SDATA (Vsystem_name); *p; p++)
      if (*p == ' ' || *p == '\t')
	*p = '-';
  }
}

#ifndef MSDOS
#ifndef VMS
#if !defined (HAVE_SELECT) || defined (BROKEN_SELECT_NON_X)

#include "sysselect.h"
#undef select

#if defined (HAVE_X_WINDOWS) && !defined (HAVE_SELECT)
/* Cause explanatory error message at compile time,
   since the select emulation is not good enough for X.  */
int *x = &x_windows_lose_if_no_select_system_call;
#endif

/* Emulate as much as select as is possible under 4.1 and needed by Gnu Emacs
 * Only checks read descriptors.
 */
/* How long to wait between checking fds in select */
#define SELECT_PAUSE 1
int select_alarmed;

/* For longjmp'ing back to read_input_waiting.  */

jmp_buf read_alarm_throw;

/* Nonzero if the alarm signal should throw back to read_input_waiting.
   The read_socket_hook function sets this to 1 while it is waiting.  */

int read_alarm_should_throw;

SIGTYPE
select_alarm ()
{
  select_alarmed = 1;
#ifdef BSD4_1
  sigrelse (SIGALRM);
#else /* not BSD4_1 */
  signal (SIGALRM, SIG_IGN);
#endif /* not BSD4_1 */
  SIGNAL_THREAD_CHECK (SIGALRM);
  if (read_alarm_should_throw)
    longjmp (read_alarm_throw, 1);
}

#ifndef WINDOWSNT
/* Only rfds are checked.  */
int
sys_select (nfds, rfds, wfds, efds, timeout)
     int nfds;
     SELECT_TYPE *rfds, *wfds, *efds;
     EMACS_TIME *timeout;
{
  int ravail = 0;
  SELECT_TYPE orfds;
  int timeoutval;
  int *local_timeout;
  extern int proc_buffered_char[];
#ifndef subprocesses
  int process_tick = 0, update_tick = 0;
#else
  extern int process_tick, update_tick;
#endif
  unsigned char buf;

#if defined (HAVE_SELECT) && defined (HAVE_X_WINDOWS)
  /* If we're using X, then the native select will work; we only need the
     emulation for non-X usage.  */
  if (!NILP (Vwindow_system))
    return select (nfds, rfds, wfds, efds, timeout);
#endif
  timeoutval = timeout ? EMACS_SECS (*timeout) : 100000;
  local_timeout = &timeoutval;
  FD_ZERO (&orfds);
  if (rfds)
    {
      orfds = *rfds;
      FD_ZERO (rfds);
    }
  if (wfds)
    FD_ZERO (wfds);
  if (efds)
    FD_ZERO (efds);

  /* If we are looking only for the terminal, with no timeout,
     just read it and wait -- that's more efficient.  */
  if (*local_timeout == 100000 && process_tick == update_tick
      && FD_ISSET (0, &orfds))
    {
      int fd;
      for (fd = 1; fd < nfds; ++fd)
	if (FD_ISSET (fd, &orfds))
	  goto hardway;
      if (! detect_input_pending ())
	read_input_waiting ();
      FD_SET (0, rfds);
      return 1;
    }

 hardway:
  /* Once a second, till the timer expires, check all the flagged read
   * descriptors to see if any input is available.  If there is some then
   * set the corresponding bit in the return copy of rfds.
   */
  while (1)
    {
      register int to_check, fd;

      if (rfds)
	{
	  for (to_check = nfds, fd = 0; --to_check >= 0; fd++)
	    {
	      if (FD_ISSET (fd, &orfds))
		{
		  int avail = 0, status = 0;

		  if (fd == 0)
		    avail = detect_input_pending (); /* Special keyboard handler */
		  else
		    {
#ifdef FIONREAD
		      status = ioctl (fd, FIONREAD, &avail);
#else /* no FIONREAD */
		      /* Hoping it will return -1 if nothing available
			 or 0 if all 0 chars requested are read.  */
		      if (proc_buffered_char[fd] >= 0)
			avail = 1;
		      else
			{
			  avail = read (fd, &buf, 1);
			  if (avail > 0)
			    proc_buffered_char[fd] = buf;
			}
#endif /* no FIONREAD */
		    }
		  if (status >= 0 && avail > 0)
		    {
		      FD_SET (fd, rfds);
		      ravail++;
		    }
		}
	    }
	}
      if (*local_timeout == 0 || ravail != 0 || process_tick != update_tick)
	break;

      turn_on_atimers (0);
      signal (SIGALRM, select_alarm);
      select_alarmed = 0;
      alarm (SELECT_PAUSE);

      /* Wait for a SIGALRM (or maybe a SIGTINT) */
      while (select_alarmed == 0 && *local_timeout != 0
	     && process_tick == update_tick)
	{
	  /* If we are interested in terminal input,
	     wait by reading the terminal.
	     That makes instant wakeup for terminal input at least.  */
	  if (FD_ISSET (0, &orfds))
	    {
	      read_input_waiting ();
	      if (detect_input_pending ())
		select_alarmed = 1;
	    }
	  else
	    pause ();
	}
      (*local_timeout) -= SELECT_PAUSE;

      /* Reset the old alarm if there was one.  */
      turn_on_atimers (1);

      if (*local_timeout == 0)  /* Stop on timer being cleared */
	break;
    }
  return ravail;
}
#endif /* not WINDOWSNT */

/* Read keyboard input into the standard buffer,
   waiting for at least one character.  */

void
read_input_waiting ()
{
  int nread, i;
  extern int quit_char;

  if (read_socket_hook)
    {
      struct input_event hold_quit;

      EVENT_INIT (hold_quit);
      hold_quit.kind = NO_EVENT;

      read_alarm_should_throw = 0;
      if (! setjmp (read_alarm_throw))
	nread = (*read_socket_hook) (0, 1, &hold_quit);
      else
	nread = -1;

      if (hold_quit.kind != NO_EVENT)
	kbd_buffer_store_event (&hold_quit);
    }
  else
    {
      struct input_event e;
      char buf[3];
      nread = read (fileno (stdin), buf, 1);
      EVENT_INIT (e);

      /* Scan the chars for C-g and store them in kbd_buffer.  */
      e.kind = ASCII_KEYSTROKE_EVENT;
      e.frame_or_window = selected_frame;
      e.modifiers = 0;
      for (i = 0; i < nread; i++)
	{
	  /* Convert chars > 0177 to meta events if desired.
	     We do this under the same conditions that read_avail_input does.  */
	  if (read_socket_hook == 0)
	    {
	      /* If the user says she has a meta key, then believe her. */
	      if (meta_key == 1 && (buf[i] & 0x80))
		e.modifiers = meta_modifier;
	      if (meta_key != 2)
		buf[i] &= ~0x80;
	    }

	  XSETINT (e.code, buf[i]);
	  kbd_buffer_store_event (&e);
	  /* Don't look at input that follows a C-g too closely.
	     This reduces lossage due to autorepeat on C-g.  */
	  if (buf[i] == quit_char)
	    break;
	}
    }
}

#endif /* not HAVE_SELECT */
#endif /* not VMS */
#endif /* not MSDOS */

#ifdef BSD4_1
void
init_sigio (fd)
     int fd;
{
  if (noninteractive)
    return;
  lmode = LINTRUP | lmode;
  ioctl (fd, TIOCLSET, &lmode);
}

void
reset_sigio ()
{
  if (noninteractive)
    return;
  lmode = ~LINTRUP & lmode;
  ioctl (0, TIOCLSET, &lmode);
}

void
request_sigio ()
{
  if (noninteractive)
    return;
  sigrelse (SIGTINT);

  interrupts_deferred = 0;
}

void
unrequest_sigio ()
{
  if (noninteractive)
    return;
  sighold (SIGTINT);

  interrupts_deferred = 1;
}

/* still inside #ifdef BSD4_1 */
#ifdef subprocesses

int sigheld; /* Mask of held signals */

void
sigholdx (signum)
     int signum;
{
  sigheld |= sigbit (signum);
  sighold (signum);
}

void
sigisheld (signum)
     int signum;
{
  sigheld |= sigbit (signum);
}

void
sigunhold (signum)
     int signum;
{
  sigheld &= ~sigbit (signum);
  sigrelse (signum);
}

void
sigfree ()    /* Free all held signals */
{
  int i;
  for (i = 0; i < NSIG; i++)
    if (sigheld & sigbit (i))
      sigrelse (i);
  sigheld = 0;
}

int
sigbit (i)
{
  return 1 << (i - 1);
}
#endif /* subprocesses */
#endif /* BSD4_1 */

/* POSIX signals support - DJB */
/* Anyone with POSIX signals should have ANSI C declarations */

#ifdef POSIX_SIGNALS

sigset_t empty_mask, full_mask;

signal_handler_t
sys_signal (int signal_number, signal_handler_t action)
{
  struct sigaction new_action, old_action;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_handler = action;
#if defined (SA_RESTART) && ! defined (BROKEN_SA_RESTART) && !defined(SYNC_INPUT)
  /* Emacs mostly works better with restartable system services. If this
     flag exists, we probably want to turn it on here.
     However, on some systems this resets the timeout of `select'
     which means that `select' never finishes if it keeps getting signals.
     BROKEN_SA_RESTART is defined on those systems.  */
  /* It's not clear why the comment above says "mostly works better".  --Stef
     When SYNC_INPUT is set, we don't want SA_RESTART because we need to poll
     for pending input so we need long-running syscalls to be interrupted
     after a signal that sets the interrupt_input_pending flag.  */
  new_action.sa_flags = SA_RESTART;
#else
  new_action.sa_flags = 0;
#endif
  sigaction (signal_number, &new_action, &old_action);
  return (old_action.sa_handler);
}

#ifndef __GNUC__
/* If we're compiling with GCC, we don't need this function, since it
   can be written as a macro.  */
sigset_t
sys_sigmask (int sig)
{
  static sigset_t mask;
  sigemptyset (&mask);
  sigaddset (&mask, sig);
  return mask;
}
#endif

/* I'd like to have these guys return pointers to the mask storage in here,
   but there'd be trouble if the code was saving multiple masks.  I'll be
   safe and pass the structure.  It normally won't be more than 2 bytes
   anyhow. - DJB */

sigset_t
sys_sigblock (sigset_t new_mask)
{
  sigset_t old_mask;
  sigprocmask (SIG_BLOCK, &new_mask, &old_mask);
  return (old_mask);
}

sigset_t
sys_sigunblock (sigset_t new_mask)
{
  sigset_t old_mask;
  sigprocmask (SIG_UNBLOCK, &new_mask, &old_mask);
  return (old_mask);
}

sigset_t
sys_sigsetmask (sigset_t new_mask)
{
  sigset_t old_mask;
  sigprocmask (SIG_SETMASK, &new_mask, &old_mask);
  return (old_mask);
}

#endif /* POSIX_SIGNALS */

#if !defined HAVE_STRSIGNAL && !HAVE_DECL_SYS_SIGLIST
static char *my_sys_siglist[NSIG];
# ifdef sys_siglist
#  undef sys_siglist
# endif
# define sys_siglist my_sys_siglist
#endif

void
init_signals ()
{
#ifdef POSIX_SIGNALS
  sigemptyset (&empty_mask);
  sigfillset (&full_mask);
#endif

#if !defined HAVE_STRSIGNAL && !HAVE_DECL_SYS_SIGLIST
  if (! initialized)
    {
# ifdef SIGABRT
      sys_siglist[SIGABRT] = "Aborted";
# endif
# ifdef SIGAIO
      sys_siglist[SIGAIO] = "LAN I/O interrupt";
# endif
# ifdef SIGALRM
      sys_siglist[SIGALRM] = "Alarm clock";
# endif
# ifdef SIGBUS
      sys_siglist[SIGBUS] = "Bus error";
# endif
# ifdef SIGCLD
      sys_siglist[SIGCLD] = "Child status changed";
# endif
# ifdef SIGCHLD
      sys_siglist[SIGCHLD] = "Child status changed";
# endif
# ifdef SIGCONT
      sys_siglist[SIGCONT] = "Continued";
# endif
# ifdef SIGDANGER
      sys_siglist[SIGDANGER] = "Swap space dangerously low";
# endif
# ifdef SIGDGNOTIFY
      sys_siglist[SIGDGNOTIFY] = "Notification message in queue";
# endif
# ifdef SIGEMT
      sys_siglist[SIGEMT] = "Emulation trap";
# endif
# ifdef SIGFPE
      sys_siglist[SIGFPE] = "Arithmetic exception";
# endif
# ifdef SIGFREEZE
      sys_siglist[SIGFREEZE] = "SIGFREEZE";
# endif
# ifdef SIGGRANT
      sys_siglist[SIGGRANT] = "Monitor mode granted";
# endif
# ifdef SIGHUP
      sys_siglist[SIGHUP] = "Hangup";
# endif
# ifdef SIGILL
      sys_siglist[SIGILL] = "Illegal instruction";
# endif
# ifdef SIGINT
      sys_siglist[SIGINT] = "Interrupt";
# endif
# ifdef SIGIO
      sys_siglist[SIGIO] = "I/O possible";
# endif
# ifdef SIGIOINT
      sys_siglist[SIGIOINT] = "I/O intervention required";
# endif
# ifdef SIGIOT
      sys_siglist[SIGIOT] = "IOT trap";
# endif
# ifdef SIGKILL
      sys_siglist[SIGKILL] = "Killed";
# endif
# ifdef SIGLOST
      sys_siglist[SIGLOST] = "Resource lost";
# endif
# ifdef SIGLWP
      sys_siglist[SIGLWP] = "SIGLWP";
# endif
# ifdef SIGMSG
      sys_siglist[SIGMSG] = "Monitor mode data available";
# endif
# ifdef SIGPHONE
      sys_siglist[SIGWIND] = "SIGPHONE";
# endif
# ifdef SIGPIPE
      sys_siglist[SIGPIPE] = "Broken pipe";
# endif
# ifdef SIGPOLL
      sys_siglist[SIGPOLL] = "Pollable event occurred";
# endif
# ifdef SIGPROF
      sys_siglist[SIGPROF] = "Profiling timer expired";
# endif
# ifdef SIGPTY
      sys_siglist[SIGPTY] = "PTY I/O interrupt";
# endif
# ifdef SIGPWR
      sys_siglist[SIGPWR] = "Power-fail restart";
# endif
# ifdef SIGQUIT
      sys_siglist[SIGQUIT] = "Quit";
# endif
# ifdef SIGRETRACT
      sys_siglist[SIGRETRACT] = "Need to relinguish monitor mode";
# endif
# ifdef SIGSAK
      sys_siglist[SIGSAK] = "Secure attention";
# endif
# ifdef SIGSEGV
      sys_siglist[SIGSEGV] = "Segmentation violation";
# endif
# ifdef SIGSOUND
      sys_siglist[SIGSOUND] = "Sound completed";
# endif
# ifdef SIGSTOP
      sys_siglist[SIGSTOP] = "Stopped (signal)";
# endif
# ifdef SIGSTP
      sys_siglist[SIGSTP] = "Stopped (user)";
# endif
# ifdef SIGSYS
      sys_siglist[SIGSYS] = "Bad argument to system call";
# endif
# ifdef SIGTERM
      sys_siglist[SIGTERM] = "Terminated";
# endif
# ifdef SIGTHAW
      sys_siglist[SIGTHAW] = "SIGTHAW";
# endif
# ifdef SIGTRAP
      sys_siglist[SIGTRAP] = "Trace/breakpoint trap";
# endif
# ifdef SIGTSTP
      sys_siglist[SIGTSTP] = "Stopped (user)";
# endif
# ifdef SIGTTIN
      sys_siglist[SIGTTIN] = "Stopped (tty input)";
# endif
# ifdef SIGTTOU
      sys_siglist[SIGTTOU] = "Stopped (tty output)";
# endif
# ifdef SIGURG
      sys_siglist[SIGURG] = "Urgent I/O condition";
# endif
# ifdef SIGUSR1
      sys_siglist[SIGUSR1] = "User defined signal 1";
# endif
# ifdef SIGUSR2
      sys_siglist[SIGUSR2] = "User defined signal 2";
# endif
# ifdef SIGVTALRM
      sys_siglist[SIGVTALRM] = "Virtual timer expired";
# endif
# ifdef SIGWAITING
      sys_siglist[SIGWAITING] = "Process's LWPs are blocked";
# endif
# ifdef SIGWINCH
      sys_siglist[SIGWINCH] = "Window size changed";
# endif
# ifdef SIGWIND
      sys_siglist[SIGWIND] = "SIGWIND";
# endif
# ifdef SIGXCPU
      sys_siglist[SIGXCPU] = "CPU time limit exceeded";
# endif
# ifdef SIGXFSZ
      sys_siglist[SIGXFSZ] = "File size limit exceeded";
# endif
    }
#endif /* !defined HAVE_STRSIGNAL && !HAVE_DECL_SYS_SIGLIST */
}

#ifndef HAVE_RANDOM
#ifdef random
#define HAVE_RANDOM
#endif
#endif

/* Figure out how many bits the system's random number generator uses.
   `random' and `lrand48' are assumed to return 31 usable bits.
   BSD `rand' returns a 31 bit value but the low order bits are unusable;
   so we'll shift it and treat it like the 15-bit USG `rand'.  */

#ifndef RAND_BITS
# ifdef HAVE_RANDOM
#  define RAND_BITS 31
# else /* !HAVE_RANDOM */
#  ifdef HAVE_LRAND48
#   define RAND_BITS 31
#   define random lrand48
#  else /* !HAVE_LRAND48 */
#   define RAND_BITS 15
#   if RAND_MAX == 32767
#    define random rand
#   else /* RAND_MAX != 32767 */
#    if RAND_MAX == 2147483647
#     define random() (rand () >> 16)
#    else /* RAND_MAX != 2147483647 */
#     ifdef USG
#      define random rand
#     else
#      define random() (rand () >> 16)
#     endif /* !USG */
#    endif /* RAND_MAX != 2147483647 */
#   endif /* RAND_MAX != 32767 */
#  endif /* !HAVE_LRAND48 */
# endif /* !HAVE_RANDOM */
#endif /* !RAND_BITS */

void
seed_random (arg)
     long arg;
{
#ifdef HAVE_RANDOM
  srandom ((unsigned int)arg);
#else
# ifdef HAVE_LRAND48
  srand48 (arg);
# else
  srand ((unsigned int)arg);
# endif
#endif
}

/*
 * Build a full Emacs-sized word out of whatever we've got.
 * This suffices even for a 64-bit architecture with a 15-bit rand.
 */
long
get_random ()
{
  long val = random ();
#if VALBITS > RAND_BITS
  val = (val << RAND_BITS) ^ random ();
#if VALBITS > 2*RAND_BITS
  val = (val << RAND_BITS) ^ random ();
#if VALBITS > 3*RAND_BITS
  val = (val << RAND_BITS) ^ random ();
#if VALBITS > 4*RAND_BITS
  val = (val << RAND_BITS) ^ random ();
#endif /* need at least 5 */
#endif /* need at least 4 */
#endif /* need at least 3 */
#endif /* need at least 2 */
  return val & ((1L << VALBITS) - 1);
}

#ifdef WRONG_NAME_INSQUE

insque (q,p)
     caddr_t q,p;
{
  _insque (q,p);
}

#endif

#ifdef VMS

#ifdef VMS_NEED_SYS_GETENV
#undef getenv

char *getenv (const char *name);

#define SYS_GETENV_VALUE_SIZE 257

/* Most C callers expect values returned for different environment names to
   remain distinct.  A single translation buffer makes, for example, a saved
   TERMCAP value turn into TERM at the next lookup.  Cache one bounded VMS
   logical translation per name; repeated queries update the existing entry
   instead of leaking another allocation.  */
struct sys_getenv_cache_entry
{
  struct sys_getenv_cache_entry *next;
  char *name;
  char value[SYS_GETENV_VALUE_SIZE];
};

static struct sys_getenv_cache_entry *sys_getenv_cache;

static char *
cache_sys_getenv (name, value, length)
     const char *name, *value;
     size_t length;
{
  struct sys_getenv_cache_entry *entry;

  for (entry = sys_getenv_cache; entry; entry = entry->next)
    if (!strcmp (entry->name, name))
      break;

  if (!entry)
    {
      entry = (struct sys_getenv_cache_entry *) xmalloc (sizeof *entry);
      entry->name = (char *) xmalloc (strlen (name) + 1);
      strcpy (entry->name, name);
      entry->next = sys_getenv_cache;
      sys_getenv_cache = entry;
    }

  memcpy (entry->value, value, length);
  entry->value[length] = '\0';
  return entry->value;
}

/* If any place else asks for the TERM variable,
   allow it to be overridden with the EMACS_TERM variable
   before attempting to translate the logical name TERM.  As a last
   resort, ask for VAX C's special idea of the TERM variable.  */
char *
sys_getenv (const char *name)
{
  register char *val;
  char buf[SYS_GETENV_VALUE_SIZE];
  struct dsc$descriptor_s equiv
    = {sizeof (buf) - 1, DSC$K_DTYPE_T, DSC$K_CLASS_S, buf};
  struct dsc$descriptor_s d_name
    = {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
  short eqlen;

  if (!strcmp (name, "TERM"))
    {
      val = (char *) getenv ("EMACS_TERM");
      if (val)
	return val;
    }

  d_name.dsc$w_length = strlen (name);
  d_name.dsc$a_pointer = (char *) name;
  if (LIB$SYS_TRNLOG (&d_name, &eqlen, &equiv) == SS$_NORMAL)
    return cache_sys_getenv (name, buf, eqlen);
  return (char *) getenv (name);
}
#define getenv sys_getenv
#endif /* VMS_NEED_SYS_GETENV */

#endif /* VMS */

#ifdef VMS
#ifdef LINK_CRTL_SHARE
#ifdef SHARABLE_LIB_BUG
/* Variables declared noshare and initialized in sharable libraries
   cannot be shared.  The VMS linker incorrectly forces you to use a private
   version which is uninitialized... If not for this "feature", we
   could use the C library definition of sys_nerr and sys_errlist. */
int sys_nerr = 35;
char *sys_errlist[] =
  {
    "error 0",
    "not owner",
    "no such file or directory",
    "no such process",
    "interrupted system call",
    "i/o error",
    "no such device or address",
    "argument list too long",
    "exec format error",
    "bad file number",
    "no child process",
    "no more processes",
    "not enough memory",
    "permission denied",
    "bad address",
    "block device required",
    "mount devices busy",
    "file exists",
    "cross-device link",
    "no such device",
    "not a directory",
    "is a directory",
    "invalid argument",
    "file table overflow",
    "too many open files",
    "not a typewriter",
    "text file busy",
    "file too big",
    "no space left on device",
    "illegal seek",
    "read-only file system",
    "too many links",
    "broken pipe",
    "math argument",
    "result too large",
    "I/O stream empty",
    "vax/vms specific error code nontranslatable error"
  };
#endif /* SHARABLE_LIB_BUG */
#endif /* LINK_CRTL_SHARE */
#endif /* VMS */

#ifndef HAVE_STRERROR
#ifndef WINDOWSNT
char *
strerror (errnum)
     int errnum;
{
  extern char *sys_errlist[];
  extern int sys_nerr;

  if (errnum >= 0 && errnum < sys_nerr)
    return sys_errlist[errnum];
  return (char *) "Unknown error";
}
#endif /* not WINDOWSNT */
#endif /* ! HAVE_STRERROR */

#ifdef WRITE_ADDS_NEWLINE
static void discard_emacs_write_buffer P_ ((int));
#endif

int
emacs_open (path, oflag, mode)
     const char *path;
     int oflag, mode;
{
  register int rtnval;

#ifdef BSD4_1
  if (oflag & O_CREAT)
    return creat (path, mode);
#endif

  while ((rtnval = open (path, oflag, mode)) == -1
	 && (errno == EINTR))
    QUIT;
  return (rtnval);
}

int
emacs_close (fd)
     int fd;
{
  int did_retry = 0;
  register int rtnval;

#ifdef WRITE_ADDS_NEWLINE
  /* Never let an unterminated record survive descriptor reuse.  Normal
     callers flush before closing; error unwind paths deliberately discard.  */
  discard_emacs_write_buffer (fd);
#endif

#if defined (MAC_OSX) && defined (HAVE_CARBON)
  {
    extern int mac_try_close_socket P_ ((int));

    if (mac_try_close_socket (fd))
      return 0;
  }
#endif

  while ((rtnval = close (fd)) == -1
	 && (errno == EINTR))
    did_retry = 1;

  /* If close is interrupted SunOS 4.1 may or may not have closed the
     file descriptor.  If it did the second close will fail with
     errno = EBADF.  That means we have succeeded.  */
  if (rtnval == -1 && did_retry && errno == EBADF)
    return 0;

  return rtnval;
}

int
emacs_read (fildes, buf, nbyte)
     int fildes;
     char *buf;
     unsigned int nbyte;
{
  register int rtnval;

  while ((rtnval = read (fildes, buf, nbyte)) == -1
	 && (errno == EINTR))
    QUIT;
  return (rtnval);
}

#ifdef WRITE_ADDS_NEWLINE	/* It is ridiculously optimistic to expect
				   this will ever change, but we'll be ready
				   anyway.  Optimism is ok in small doses.  */

/* The VMS C RTL turns each write into a record.  Keep an incomplete final
   record per descriptor, rather than in one process-global buffer.  The old
   implementation mixed data written to different files and aborted on a
   record larger than 32K.  */
struct emacs_write_buffer
{
  struct emacs_write_buffer *next;
  int fd;
  size_t valid;
  size_t allocated;
  char *data;
};

static struct emacs_write_buffer *emacs_write_buffers;

static struct emacs_write_buffer *
find_emacs_write_buffer (fd, create)
     int fd, create;
{
  struct emacs_write_buffer *p;

  for (p = emacs_write_buffers; p; p = p->next)
    if (p->fd == fd)
      return p;

  if (!create)
    return 0;

  p = (struct emacs_write_buffer *) malloc (sizeof *p);
  if (!p)
    {
      errno = ENOMEM;
      return 0;
    }
  p->fd = fd;
  p->valid = 0;
  p->allocated = 0;
  p->data = 0;
  p->next = emacs_write_buffers;
  emacs_write_buffers = p;
  return p;
}

static int
grow_emacs_write_buffer (p, needed)
     struct emacs_write_buffer *p;
     size_t needed;
{
  size_t allocated;
  char *data;

  if (needed <= p->allocated)
    return 1;

  allocated = p->allocated ? p->allocated : 4096;
  while (allocated < needed)
    {
      size_t larger = allocated * 2;
      if (larger <= allocated)
	{
	  allocated = needed;
	  break;
	}
      allocated = larger;
    }

  data = (char *) realloc (p->data, allocated);
  if (!data)
    {
      errno = ENOMEM;
      return 0;
    }
  p->data = data;
  p->allocated = allocated;
  return 1;
}

static void
discard_emacs_write_buffer (fd)
     int fd;
{
  struct emacs_write_buffer **link = &emacs_write_buffers;

  while (*link)
    {
      struct emacs_write_buffer *p = *link;
      if (p->fd == fd)
	{
	  *link = p->next;
	  free (p->data);
	  free (p);
	  return;
	}
      link = &p->next;
    }
}

static int
write_complete_vms_record (fd, data, length)
     int fd;
     const char *data;
     size_t length;
{
  int written;

  do
    written = write (fd, data, length);
  while (written < 0 && errno == EINTR);

  if (written < 0)
    return -1;
  if ((size_t) written != length)
    {
      errno = EIO;
      return -1;
    }
  return written;
}

int
emacs_write (fildes, buf, nbyte)
     int fildes;
     const char *buf;
     unsigned int nbyte;
{
  struct emacs_write_buffer *p;
  const char *e;
  size_t complete, trailing, needed;

  /* Flush this descriptor's last incomplete record.  */
  if (0 == nbyte)
    {
      int written;
      p = find_emacs_write_buffer (fildes, 0);
      if (!p)
	return 0;
      written = write_complete_vms_record (fildes, p->data, p->valid);
      discard_emacs_write_buffer (fildes);
      return written;
    }

  e = buf + nbyte;
  while (e > buf && e[-1] != '\n')
    e--;
  complete = e - buf;
  trailing = nbyte - complete;

  p = find_emacs_write_buffer (fildes, trailing != 0);
  if (trailing && !p)
    return -1;

  /* No complete record yet: retain all the new data with any carryover.  */
  if (complete == 0)
    {
      if (p->valid > (size_t) -1 - trailing)
	{
	  errno = ENOMEM;
	  return -1;
	}
      needed = p->valid + trailing;
      if (!grow_emacs_write_buffer (p, needed))
	return -1;
      memcpy (p->data + p->valid, buf, trailing);
      p->valid = needed;
      return nbyte;
    }

  if (p && p->valid > (size_t) -1 - complete - trailing)
    {
      errno = ENOMEM;
      return -1;
    }

  needed = (p ? p->valid : 0) + complete + trailing;
  if (p && !grow_emacs_write_buffer (p, needed))
    return -1;

  if (p && p->valid)
    {
      size_t record_length = p->valid + complete;
      memcpy (p->data + p->valid, buf, complete);
      if (write_complete_vms_record (fildes, p->data, record_length) < 0)
	{
	  discard_emacs_write_buffer (fildes);
	  return -1;
	}
      p->valid = 0;
    }
  else if (complete
	   && write_complete_vms_record (fildes, buf, complete) < 0)
    {
      discard_emacs_write_buffer (fildes);
      return -1;
    }

  if (trailing)
    {
      if (!p)
	{
	  p = find_emacs_write_buffer (fildes, 1);
	  if (!p || !grow_emacs_write_buffer (p, trailing))
	    return -1;
	}
      memcpy (p->data, e, trailing);
      p->valid = trailing;
    }
  else
    discard_emacs_write_buffer (fildes);

  return nbyte;			/* Report the caller's bytes as accepted.  */
}

#else /* ! defined (WRITE_ADDS_NEWLINE) */

int
emacs_write (fildes, buf, nbyte)
     int fildes;
     const char *buf;
     unsigned int nbyte;
{
  register int rtnval, bytes_written;

  bytes_written = 0;

  while (nbyte > 0)
    {
      rtnval = write (fildes, buf, nbyte);

      if (rtnval == -1)
	{
	  if (errno == EINTR)
	    {
#ifdef SYNC_INPUT
	      /* I originally used `QUIT' but that might causes files to
		 be truncated if you hit C-g in the middle of it.  --Stef  */
	      if (interrupt_input_pending)
		handle_async_input ();
#endif
	      continue;
	    }
	  else
	    return (bytes_written ? bytes_written : -1);
	}

      buf += rtnval;
      nbyte -= rtnval;
      bytes_written += rtnval;
    }
  return (bytes_written);
}

#endif /* ! defined (WRITE_ADDS_NEWLINE) */



#ifdef USG
/*
 *	All of the following are for USG.
 *
 *	On USG systems the system calls are INTERRUPTIBLE by signals
 *	that the user program has elected to catch.  Thus the system call
 *	must be retried in these cases.  To handle this without massive
 *	changes in the source code, we remap the standard system call names
 *	to names for our own functions in sysdep.c that do the system call
 *	with retries.  Actually, for portability reasons, it is good
 *	programming practice, as this example shows, to limit all actual
 *	system calls to a single occurrence in the source.  Sure, this
 *	adds an extra level of function call overhead but it is almost
 *	always negligible.   Fred Fish, Unisoft Systems Inc.
 */

/*
 *	Warning, this function may not duplicate 4.2 action properly
 *	under error conditions.
 */

#ifndef MAXPATHLEN
/* In 4.1, param.h fails to define this.  */
#define MAXPATHLEN 1024
#endif

#ifndef HAVE_GETWD

char *
getwd (pathname)
     char *pathname;
{
  char *npath, *spath;
  extern char *getcwd ();

  BLOCK_INPUT;			/* getcwd uses malloc */
  spath = npath = getcwd ((char *) 0, MAXPATHLEN);
  if (spath == 0)
    {
      UNBLOCK_INPUT;
      return spath;
    }
  /* On Altos 3068, getcwd can return @hostname/dir, so discard
     up to first slash.  Should be harmless on other systems.  */
  while (*npath && *npath != '/')
    npath++;
  strcpy (pathname, npath);
  free (spath);			/* getcwd uses malloc */
  UNBLOCK_INPUT;
  return pathname;
}

#endif /* HAVE_GETWD */

/*
 *	Emulate rename using unlink/link.  Note that this is
 *	only partially correct.  Also, doesn't enforce restriction
 *	that files be of same type (regular->regular, dir->dir, etc).
 */

#ifndef HAVE_RENAME

rename (from, to)
     const char *from;
     const char *to;
{
  if (access (from, 0) == 0)
    {
      unlink (to);
      if (link (from, to) == 0)
	if (unlink (from) == 0)
	  return (0);
    }
  return (-1);
}

#endif


#ifdef HPUX
#ifndef HAVE_PERROR

/* HPUX curses library references perror, but as far as we know
   it won't be called.  Anyway this definition will do for now.  */

perror ()
{
}

#endif /* not HAVE_PERROR */
#endif /* HPUX */

#ifndef HAVE_DUP2

/*
 *	Emulate BSD dup2.  First close newd if it already exists.
 *	Then, attempt to dup oldd.  If not successful, call dup2 recursively
 *	until we are, then close the unsuccessful ones.
 */

dup2 (oldd, newd)
     int oldd;
     int newd;
{
  register int fd, ret;

  emacs_close (newd);

#ifdef F_DUPFD
  return fcntl (oldd, F_DUPFD, newd);
#else
  fd = dup (old);
  if (fd == -1)
    return -1;
  if (fd == new)
    return new;
  ret = dup2 (old,new);
  emacs_close (fd);
  return ret;
#endif
}

#endif /* not HAVE_DUP2 */

/*
 *	Gettimeofday.  Simulate as much as possible.  Only accurate
 *	to nearest second.  Emacs doesn't use tzp so ignore it for now.
 *	Only needed when subprocesses are defined.
 */

#ifdef subprocesses
#ifndef VMS
#ifndef HAVE_GETTIMEOFDAY
#ifdef HAVE_TIMEVAL

/* ARGSUSED */
int
gettimeofday (tp, tzp)
     struct timeval *tp;
     struct timezone *tzp;
{
  extern long time ();

  tp->tv_sec = time ((long *)0);
  tp->tv_usec = 0;
  if (tzp != 0)
    tzp->tz_minuteswest = -1;
  return 0;
}

#endif
#endif
#endif
#endif /* subprocess && !HAVE_GETTIMEOFDAY && HAVE_TIMEVAL && !VMS */

/*
 *	This function will go away as soon as all the stubs fixed. (fnf)
 */

void
croak (badfunc)
     char *badfunc;
{
  printf ("%s not yet implemented\r\n", badfunc);
  reset_sys_modes ();
  exit (1);
}

#endif /* USG */

/* Directory routines for systems that don't have them. */

#ifdef SYSV_SYSTEM_DIR

#include <dirent.h>

#if defined (BROKEN_CLOSEDIR) || !defined (HAVE_CLOSEDIR)

int
closedir (dirp)
     register DIR *dirp;              /* stream from opendir */
{
  int rtnval;

  rtnval = emacs_close (dirp->dd_fd);

  /* Some systems (like Solaris) allocate the buffer and the DIR all
     in one block.  Why in the world are we freeing this ourselves
     anyway?  */
#if ! (defined (sun) && defined (USG5_4))
  xfree ((char *) dirp->dd_buf); /* directory block defined in <dirent.h> */
#endif
  xfree ((char *) dirp);

  return rtnval;
}
#endif /* BROKEN_CLOSEDIR or not HAVE_CLOSEDIR */
#endif /* SYSV_SYSTEM_DIR */

#ifdef NONSYSTEM_DIR_LIBRARY

DIR *
opendir (filename)
     char *filename;	/* name of directory */
{
  register DIR *dirp;		/* -> malloc'ed storage */
  register int fd;		/* file descriptor for read */
  struct stat sbuf;		/* result of fstat */

  fd = emacs_open (filename, O_RDONLY, 0);
  if (fd < 0)
    return 0;

  BLOCK_INPUT;
  if (fstat (fd, &sbuf) < 0
      || (sbuf.st_mode & S_IFMT) != S_IFDIR
      || (dirp = (DIR *) xmalloc (sizeof (DIR))) == 0)
    {
      emacs_close (fd);
      UNBLOCK_INPUT;
      return 0;		/* bad luck today */
    }
  UNBLOCK_INPUT;

  dirp->dd_fd = fd;
  dirp->dd_loc = dirp->dd_size = 0;	/* refill needed */

  return dirp;
}

void
closedir (dirp)
     register DIR *dirp;		/* stream from opendir */
{
  emacs_close (dirp->dd_fd);
  xfree ((char *) dirp);
}


#ifndef VMS
#define DIRSIZ	14
struct olddir
  {
    ino_t od_ino; 		/* inode */
    char od_name[DIRSIZ];	/* filename */
  };
#endif /* not VMS */

struct direct dir_static;	/* simulated directory contents */

/* ARGUSED */
struct direct *
readdir (dirp)
     register DIR *dirp;	/* stream from opendir */
{
#ifndef VMS
  register struct olddir *dp;	/* -> directory data */
#else /* VMS */
  register struct dir$_name *dp; /* -> directory data */
  register struct dir$_version *dv; /* -> version data */
#endif /* VMS */

  for (; ;)
    {
      if (dirp->dd_loc >= dirp->dd_size)
	dirp->dd_loc = dirp->dd_size = 0;

      if (dirp->dd_size == 0 	/* refill buffer */
	  && (dirp->dd_size = emacs_read (dirp->dd_fd, dirp->dd_buf, DIRBLKSIZ)) <= 0)
	return 0;

#ifndef VMS
      dp = (struct olddir *) &dirp->dd_buf[dirp->dd_loc];
      dirp->dd_loc += sizeof (struct olddir);

      if (dp->od_ino != 0)	/* not deleted entry */
	{
	  dir_static.d_ino = dp->od_ino;
	  strncpy (dir_static.d_name, dp->od_name, DIRSIZ);
	  dir_static.d_name[DIRSIZ] = '\0';
	  dir_static.d_namlen = strlen (dir_static.d_name);
	  dir_static.d_reclen = sizeof (struct direct)
	    - MAXNAMLEN + 3
	      + dir_static.d_namlen - dir_static.d_namlen % 4;
	  return &dir_static;	/* -> simulated structure */
	}
#else /* VMS */
      dp = (struct dir$_name *) dirp->dd_buf;
      if (dirp->dd_loc == 0)
	dirp->dd_loc = (dp->dir$b_namecount&1) ? dp->dir$b_namecount + 1
	  : dp->dir$b_namecount;
      dv = (struct dir$_version *)&dp->dir$t_name[dirp->dd_loc];
      dir_static.d_ino = dv->dir$w_fid_num;
      dir_static.d_namlen = min (dp->dir$b_namecount, MAXNAMLEN);
      dir_static.d_reclen = sizeof (struct direct)
	- MAXNAMLEN + 3
	  + dir_static.d_namlen - dir_static.d_namlen % 4;
      memcpy (dir_static.d_name, dp->dir$t_name, dir_static.d_namlen);
      dir_static.d_name[dir_static.d_namlen] = '\0';
      dirp->dd_loc = dirp->dd_size; /* only one record at a time */
      return &dir_static;
#endif /* VMS */
    }
}

#ifdef VMS
/* readdirver is just like readdir except it returns all versions of a file
   as separate entries.  */

/* ARGUSED */
struct direct *
readdirver (dirp)
     register DIR *dirp;	/* stream from opendir */
{
  register struct dir$_name *dp; /* -> directory data */
  register struct dir$_version *dv; /* -> version data */

  if (dirp->dd_loc >= dirp->dd_size - sizeof (struct dir$_name))
    dirp->dd_loc = dirp->dd_size = 0;

  if (dirp->dd_size == 0 	/* refill buffer */
      && (dirp->dd_size = sys_read (dirp->dd_fd, dirp->dd_buf, DIRBLKSIZ)) <= 0)
    return 0;

  dp = (struct dir$_name *) dirp->dd_buf;
  if (dirp->dd_loc == 0)
    dirp->dd_loc = dp->dir$b_namecount + (dp->dir$b_namecount & 1);
  dv = (struct dir$_version *) &dp->dir$t_name[dirp->dd_loc];
  dir_static.d_namlen = min (dp->dir$b_namecount, MAXNAMLEN - 7);
  memcpy (dir_static.d_name, dp->dir$t_name, dir_static.d_namlen);
  sprintf (&dir_static.d_name[dir_static.d_namlen], ";%d",
	   dv->dir$w_version);
  dir_static.d_namlen = strlen (dir_static.d_name);
  dir_static.d_ino = dv->dir$w_fid_num;
  dir_static.d_reclen = sizeof (struct direct) - MAXNAMLEN + 3
			+ dir_static.d_namlen - dir_static.d_namlen % 4;
  dirp->dd_loc = ((char *) (++dv) - dp->dir$t_name);
  return &dir_static;
}

#endif /* VMS */

#endif /* NONSYSTEM_DIR_LIBRARY */


int
set_file_times (filename, atime, mtime)
     const char *filename;
     EMACS_TIME atime, mtime;
{
#if defined (HAVE_UTIMES) && !defined (VMS)
  struct timeval tv[2];
  tv[0] = atime;
  tv[1] = mtime;
  return utimes (filename, tv);
#else /* not HAVE_UTIMES */
  struct utimbuf utb;
#ifdef VMS
  utb.atime = EMACS_SECS (atime);
  utb.mtime = EMACS_SECS (mtime);
#else
  utb.actime = EMACS_SECS (atime);
  utb.modtime = EMACS_SECS (mtime);
#endif
  return utime (filename, &utb);
#endif /* not HAVE_UTIMES */
}

/* mkdir and rmdir functions, for systems which don't have them.  */

#ifndef HAVE_MKDIR
/*
 * Written by Robert Rother, Mariah Corporation, August 1985.
 *
 * If you want it, it's yours.  All I ask in return is that if you
 * figure out how to do this in a Bourne Shell script you send me
 * a copy.
 *					sdcsvax!rmr or rmr@uscd
 *
 * Severely hacked over by John Gilmore to make a 4.2BSD compatible
 * subroutine.  11Mar86; hoptoad!gnu
 *
 * Modified by rmtodd@uokmax 6-28-87 -- when making an already existing dir,
 * subroutine didn't return EEXIST.  It does now.
 */

/*
 * Make a directory.
 */
#ifdef MKDIR_PROTOTYPE
MKDIR_PROTOTYPE
#else
int
#ifdef VMS
mkdir2 (dpath, dmode)
#else
mkdir (dpath, dmode)
#endif
     char *dpath;
     int dmode;
#endif
{
  int cpid, status, fd;
  struct stat statbuf;

  if (stat (dpath, &statbuf) == 0)
    {
      errno = EEXIST;		/* Stat worked, so it already exists */
      return -1;
    }

  /* If stat fails for a reason other than non-existence, return error */
  if (errno != ENOENT)
    return -1;

  synch_process_alive = 1;
  switch (cpid = fork ())
    {

    case -1:			/* Error in fork */
      return (-1);		/* Errno is set already */

    case 0:			/* Child process */
      /*
		 * Cheap hack to set mode of new directory.  Since this
		 * child process is going away anyway, we zap its umask.
		 * FIXME, this won't suffice to set SUID, SGID, etc. on this
		 * directory.  Does anybody care?
		 */
      status = umask (0);	/* Get current umask */
      status = umask (status | (0777 & ~dmode));	/* Set for mkdir */
      fd = emacs_open ("/dev/null", O_RDWR, 0);
      if (fd >= 0)
        {
	  dup2 (fd, 0);
	  dup2 (fd, 1);
	  dup2 (fd, 2);
        }
      execl ("/bin/mkdir", "mkdir", dpath, (char *) 0);
      _exit (-1);		/* Can't exec /bin/mkdir */

    default:			/* Parent process */
      wait_for_termination (cpid);
    }

  if (synch_process_death != 0 || synch_process_retcode != 0
      || synch_process_termsig != 0)
    {
      errno = EIO;		/* We don't know why, but */
      return -1;		/* /bin/mkdir failed */
    }

  return 0;
}
#endif /* not HAVE_MKDIR */

#ifndef HAVE_RMDIR
int
rmdir (dpath)
     char *dpath;
{
  int cpid, status, fd;
  struct stat statbuf;

  if (stat (dpath, &statbuf) != 0)
    {
      /* Stat just set errno.  We don't have to */
      return -1;
    }

  synch_process_alive = 1;
  switch (cpid = fork ())
    {

    case -1:			/* Error in fork */
      return (-1);		/* Errno is set already */

    case 0:			/* Child process */
      fd = emacs_open ("/dev/null", O_RDWR, 0);
      if (fd >= 0)
        {
	  dup2 (fd, 0);
	  dup2 (fd, 1);
	  dup2 (fd, 2);
        }
      execl ("/bin/rmdir", "rmdir", dpath, (char *) 0);
      _exit (-1);		/* Can't exec /bin/rmdir */

    default:			/* Parent process */
      wait_for_termination (cpid);
    }

  if (synch_process_death != 0 || synch_process_retcode != 0
      || synch_process_termsig != 0)
    {
      errno = EIO;		/* We don't know why, but */
      return -1;		/* /bin/rmdir failed */
    }

  return 0;
}
#endif /* !HAVE_RMDIR */



/* Functions for VMS */
#ifdef VMS
#include <acldef.h>
#include <chpdef.h>
#include <jpidef.h>

/* Return as a string the VMS error string pertaining to STATUS.
   Reuses the same static buffer each time it is called.  */

char *
vmserrstr (status)
     int status;		/* VMS status code */
{
  int bufadr[2];
  short len;
  static char buf[257];

  bufadr[0] = sizeof buf - 1;
  bufadr[1] = (int) buf;
  if (! (SYS$GETMSG (status, &len, bufadr, 0x1, 0) & 1))
    return "untranslatable VMS error status";
  buf[len] = '\0';
  return buf;
}

#ifdef VMS_NEED_SYS_ACCESS
#undef access
int access (const char *filename, int type);

#include <prvdef.h>
#define ACE$M_WRITE     2
#define ACE$C_KEYID     1

static unsigned short memid, grpid;
static unsigned int uic;

/* Called from init_sys_modes, so it happens not very often
   but at least each time Emacs is loaded.  */
void
sys_access_reinit (void)
{
  uic = 0;
}

/* The fourth argument to sys$setprv used to be of type `pointer to
   union prvdef' but recently (verified w/ 7.3-1) is `unsigned long
   long int'.  Consequently, we also abstract the method of checking
   the privelege (mask vs bit) as well as the privelege "values"
   themselves.

   [Unsolved mystery: When did change this occur?  The headers for old
   systems (s/vms-*.h) still need to be updated. --ttn]  */
#ifdef SETPRV_ARG4_IS_PTRUNION

#define PRVMASK_TYPE		union prvdef
#define SETPRV_ARG4(x)		(x)
#define CHKPRIV(x,bit)		((x).(bit))
#define BYPASS_PRIVELEGE	PRV$V_BYPASS
#define SYSTEM_PREVELEGE	PRV$V_SYSPRV

#else /* !defined (SETPRV_ARG4_IS_PTRUNION) */

#define PRVMASK_TYPE		unsigned long long int
#define SETPRV_ARG4(x)		(&(x))
#define CHKPRIV(x,mask)		(0 != ((mask) & (x)))
#define BYPASS_PRIVELEGE	PRV$M_BYPASS
#define SYSTEM_PRIVELEGE	PRV$M_SYSPRV

#endif /* !defined (SETPRV_ARG4_IS_PTRUNION) */

int
sys_access (const char *filename, int type)
{
  struct FAB fab;
  struct XABPRO xab;
  int status;
  unsigned int aclbuf[60];
  PRVMASK_TYPE prvmask;

  /* Get UIC and GRP values for protection checking.  */
  if (uic == 0)
    {
      status = LIB$GETJPI (&JPI$_UIC, 0, 0, &uic, 0, 0);
      if (! (status & 1))
	return -1;
      memid = uic & 0xFFFF;
      grpid = uic >> 16;
    }

  /* Use the runtime routine for everything except write check.  */
  if (type != 2)
    return access (filename, type);

  /* Read priveleges information into `prvmask'.  */
  status = SYS$SETPRV (0, 0, 0, SETPRV_ARG4 (prvmask));
  if (! (status & 1))
    error ("Unable to determine privileges: %s", vmserrstr (status));

  /* Handle bypass.  */
  if (CHKPRIV (prvmask, BYPASS_PRIVELEGE))
    return 0;

#define WRITEABLE(field)  (! ((xab.xab$w_pro >> field) & XAB$M_NOWRITE))

  fab = cc$rms_fab;
  fab.fab$b_fac = FAB$M_GET;
  fab.fab$l_fna = (char *) filename;
  fab.fab$b_fns = strlen (filename);
  fab.fab$l_xab = &xab;
  xab = cc$rms_xabpro;
  xab.xab$l_aclbuf = aclbuf;
  xab.xab$w_aclsiz = sizeof (aclbuf);
  status = SYS$OPEN (&fab, 0, 0);
  if (! (status & 1))
    return -1;
  SYS$CLOSE (&fab, 0, 0);

  /* Check system access.  */
  if (CHKPRIV (prvmask, SYSTEM_PRIVELEGE) && WRITEABLE (XAB$V_SYS))
    return 0;

  /* Check ACL entries, if any.  */
  if (xab.xab$w_acllen > 0)
    {
      int acl_controlled = 0;
      unsigned int *aclptr = aclbuf;
      unsigned int *aclend = &aclbuf[xab.xab$w_acllen / 4];

      while (*aclptr && aclptr < aclend)
	{
	  int size = (*aclptr & 0xff) / 4;
	  int typecode = (*aclptr >> 8) & 0xff;
	  int i;

	  if (typecode == ACE$C_KEYID)
	    for (i = size - 1; i > 1; i--)
	      if (aclptr[i] == uic)
		{
		  acl_controlled = 1;
		  if (aclptr[1] & ACE$M_WRITE)
		    return 0;	/* Write access through ACL */
		}
	  aclptr = &aclptr[size];
	}
      if (acl_controlled)	/* ACL specified, prohibits write access */
	return -1;
    }

  /* Check normal protection: world-, group- and owner-writeable.  */
  return ((WRITEABLE (XAB$V_WLD)
	   || (WRITEABLE (XAB$V_GRP) &&
	       (unsigned short) (xab.xab$l_uic >> 16) == grpid)
	   || (WRITEABLE (XAB$V_OWN) &&
	       (xab.xab$l_uic & 0xFFFF) == memid))
	  ? 0			/* writeable */
	  : -1);		/* not writeable */

#undef WRITEABLE
}


#define access sys_access
#endif /* VMS_NEED_SYS_ACCESS */

static char vtbuf[NAM$C_MAXRSS+1];

/* translate a vms file spec to a unix path */
char *
sys_translate_vms (vfile)
     char * vfile;
{
  char * p;
  char * targ;

  if (!vfile)
    return 0;

  targ = vtbuf;

  /* leading device or logical name is a root directory */
  if (p = strchr (vfile, ':'))
    {
      *targ++ = '/';
      while (vfile < p)
	*targ++ = *vfile++;
      vfile++;
      *targ++ = '/';
    }
  p = vfile;
  if (*p == '[' || *p == '<')
    {
      while (*++vfile != *p + 2)
	switch (*vfile)
	  {
	  case '.':
	    if (vfile[-1] == *p)
	      *targ++ = '.';
	    *targ++ = '/';
	    break;

	  case '-':
	    *targ++ = '.';
	    *targ++ = '.';
	    break;

	  default:
	    *targ++ = *vfile;
	    break;
	  }
      vfile++;
      *targ++ = '/';
    }
  while (*vfile)
    *targ++ = *vfile++;

  return vtbuf;
}

static char utbuf[NAM$C_MAXRSS+1];

/* Translate a unix path to a VMS file spec.  */
char *
sys_translate_unix (ufile)
     char * ufile;
{
  int slash_seen = 0;
  char *p;
  char * targ;

  if (!ufile)
    return 0;

  targ = utbuf;

  /* Fexpand_file_name can concatenate a native VMS directory with a
     relative Unix-style name before asking us to translate it, for example
       SYS$DISK:[EMACS.LISP]emacs-lisp/byte-run
     Treat the components before the final slash as subdirectories of the
     existing VMS directory.  The old translator interpreted EMACS-LISP as
     a second device name, which made Emacs 22's directory-qualified Lisp
     loads impossible.  */
  {
    char *first_slash = strchr (ufile, '/');
    char *vms_close = 0;
    char *last_slash;
    char *scan;
    char close_char;

    if (first_slash)
      for (scan = ufile; scan < first_slash; scan++)
	if (*scan == ']' || *scan == '>')
	  vms_close = scan;

    if (vms_close)
      {
	char *component = vms_close + 1;
	int appended_directory = 0;

	last_slash = strrchr (component, '/');
	close_char = *vms_close;

	for (scan = ufile; scan < vms_close; scan++)
	  *targ++ = *scan;

	while (component < last_slash)
	  {
	    char *component_end = component;
	    int component_length;

	    while (component_end < last_slash && *component_end != '/')
	      component_end++;
	    component_length = component_end - component;

	    if (component_length != 0
		&& !(component_length == 1 && component[0] == '.'))
	      {
		if (appended_directory || targ == utbuf || targ[-1] != '.')
		  *targ++ = '.';
		if (component_length == 2
		    && component[0] == '.' && component[1] == '.')
		  *targ++ = '-';
		else
		  while (component < component_end)
		    *targ++ = *component++;
		appended_directory = 1;
	      }

	    component = component_end + 1;
	  }

	*targ++ = close_char;
	for (scan = last_slash + 1; *scan; scan++)
	  *targ++ = *scan;
	*targ = '\0';
	return utbuf;
	}
  }

  if (*ufile == '/')
    {
      ufile++;
    }
  else if (*ufile != '\0'
	   && index (&ufile[1], '/')
	   && strncmp (ufile, "./", 2) != 0)
    {
      /* A path without a leading slash is relative; its first component is
	 a directory, not a device name.  */
      slash_seen = 1;
      *targ++ = '[';
      *targ++ = '.';
    }

  while (*ufile)
    {
      switch (*ufile)
	{
	case '/':
	  if (slash_seen)
	    if (index (&ufile[1], '/'))
	      *targ++ = '.';
	    else
	      *targ++ = ']';
	  else
	    {
	      *targ++ = ':';
	      if (index (&ufile[1], '/'))
		*targ++ = '[';
	      slash_seen = 1;
	    }
	  break;

	case '.':
	  if (strncmp (ufile, "./", 2) == 0)
	    {
	      if (!slash_seen)
		{
		  *targ++ = '[';
		  slash_seen = 1;
		}
	      ufile++;		/* skip the dot */
	      if (index (&ufile[1], '/'))
		*targ++ = '.';
	      else
		*targ++ = ']';
	    }
	  else if (strncmp (ufile, "../", 3) == 0)
	    {
	      if (!slash_seen)
		{
		  *targ++ = '[';
		  slash_seen = 1;
		}
	      *targ++ = '-';
	      ufile += 2;	/* skip the dots */
	      if (index (&ufile[1], '/'))
		*targ++ = '.';
	      else
		*targ++ = ']';
	    }
	  else
	    *targ++ = *ufile;
	  break;

	default:
	  *targ++ = *ufile;
	  break;
	}
      ufile++;
    }
  *targ = '\0';

  return utbuf;
}

char *
getwd (pathname)
     char *pathname;
{
  char *ptr, *val;
#ifndef VMS
  extern char *getcwd ();
#endif

#define MAXPATHLEN 1024

  ptr = xmalloc (MAXPATHLEN);
  val = getcwd (ptr, MAXPATHLEN);
  if (val == 0)
    {
      xfree (ptr);
      return val;
    }
  strcpy (pathname, ptr);
  xfree (ptr);

 return pathname;
}

#ifdef VMS_NEED_SYS_GETUID
#undef getuid
uid_t getuid (void);
unsigned
sys_getuid ()
{
  return (getgid () << 16) | getuid ();
}
#define getuid sys_getuid
#endif /* VMS_NEED_SYS_GETUID */

#ifdef VMS_NEED_SYS_READ
#undef read
int read (int file_desc, void *buffer, int nbytes);
int
sys_read  (fildes, buf, nbyte)
     int fildes;
#ifdef VMS
     void *buf;
#else
     char *buf;
#endif
     unsigned int nbyte;
{
  return read (fildes, buf, (nbyte < MAXIOSIZE ? nbyte : MAXIOSIZE));
}
#define read sys_read
#endif /* VMS_NEED_SYS_READ */

#if 0
int
sys_write (fildes, buf, nbyte)
     int fildes;
     char *buf;
     unsigned int nbyte;
{
  register int nwrote, rtnval = 0;

  while (nbyte > MAXIOSIZE && (nwrote = write (fildes, buf, MAXIOSIZE)) > 0) {
    nbyte -= nwrote;
    buf += nwrote;
    rtnval += nwrote;
  }
  if (nwrote < 0)
    return rtnval ? rtnval : -1;
  if ((nwrote = write (fildes, buf, nbyte)) < 0)
    return rtnval ? rtnval : -1;
  return (rtnval + nwrote);
}
#endif /* 0 */

/*
 *	VAX/VMS VAX C RTL really loses. It insists that records
 *      end with a newline (carriage return) character, and if they
 *	don't it adds one (nice of it isn't it!)
 *
 *	Thus we do this stupidity below.
 */

#ifdef VMS_NEED_SYS_WRITE
#undef write
int write (int __file_desc, const void *__buffer, int __nbytes);
int
sys_write (int fildes, const void *buf, unsigned int nbytes)
{
  register const char *p;
  register const char *e;
  int sum = 0;
  struct stat st;

  fstat (fildes, &st);
  p = buf;
  while (nbytes > 0)
    {
      int len, retval;

      /* Handle fixed-length files with carriage control.  */
      if (st.st_fab_rfm == FAB$C_FIX
	  && ((st.st_fab_rat & (FAB$M_FTN | FAB$M_CR)) != 0))
	{
	  len = st.st_fab_mrs;
	  retval = write (fildes, p, min (len, nbytes));
	  if (retval != len)
	    return -1;
	  retval++;	/* This skips the implied carriage control */
	}
      else
	{
	  e =  p + min (MAXIOSIZE, nbytes) - 1;
	  while (*e != '\n' && e > p) e--;
	  if (p == e)		/* Ok.. so here we add a newline... sigh. */
	    e = p + min (MAXIOSIZE, nbytes) - 1;
	  len = e + 1 - p;
	  retval = write (fildes, p, len);
	  if (retval != len)
	    return -1;
	}
      p += retval;
      sum += retval;
      nbytes -= retval;
    }
  return sum;
}
#define write sys_write
#endif /* VMS_NEED_SYS_WRITE */

/* Create file NEW copying its attributes from file OLD.  If
   OLD is 0 or does not exist, create based on the value of
   vms_stmlf_recfm. */

/* Protection value the file should ultimately have.
   Set by creat_copy_attrs, and use by rename_sans_version.  */
static unsigned short int fab_final_pro;

int
creat_copy_attrs (old, new)
     char *old, *new;
{
  struct FAB fab = cc$rms_fab;
  struct XABPRO xabpro;
  char aclbuf[256];	/* Choice of size is arbitrary.  See below. */
  extern int vms_stmlf_recfm;

  if (old)
    {
      fab.fab$b_fac = FAB$M_GET;
      fab.fab$l_fna = old;
      fab.fab$b_fns = strlen (old);
      fab.fab$l_xab = (char *) &xabpro;
      xabpro = cc$rms_xabpro;
      xabpro.xab$l_aclbuf = aclbuf;
      xabpro.xab$w_aclsiz = sizeof aclbuf;
      /* Call $OPEN to fill in the fab & xabpro fields. */
      if (SYS$OPEN (&fab, 0, 0) & 1)
	{
	  SYS$CLOSE (&fab, 0, 0);
	  fab.fab$l_alq = 0;	/* zero the allocation quantity */
	  if (xabpro.xab$w_acllen > 0)
	    {
	      if (xabpro.xab$w_acllen > sizeof aclbuf)
		/* If the acl buffer was too short, redo open with longer one.
		   Wouldn't need to do this if there were some system imposed
		   limit on the size of an ACL, but I can't find any such. */
		{
		  xabpro.xab$l_aclbuf = (char *) alloca (xabpro.xab$w_acllen);
		  xabpro.xab$w_aclsiz = xabpro.xab$w_acllen;
		  if (SYS$OPEN (&fab, 0, 0) & 1)
		    SYS$CLOSE (&fab, 0, 0);
		  else
		    old = 0;
		}
	    }
	  else
	    xabpro.xab$l_aclbuf = 0;
	}
      else
	old = 0;
    }
  fab.fab$l_fna = new;
  fab.fab$b_fns = strlen (new);
  if (!old)
    {
      fab.fab$l_xab = 0;
      fab.fab$b_rfm = vms_stmlf_recfm ? FAB$C_STMLF : FAB$C_VAR;
      fab.fab$b_rat = FAB$M_CR;
    }

  /* Set the file protections such that we will be able to manipulate
     this file.  Once we are done writing and renaming it, we will set
     the protections back.  */
  if (old)
    fab_final_pro = xabpro.xab$w_pro;
  else
    SYS$SETDFPROT (0, &fab_final_pro);
  xabpro.xab$w_pro &= 0xff0f; /* set O:rewd for now. This is set back later. */

  /* Create the new file with either default attrs or attrs copied
     from old file. */
  if (!(SYS$CREATE (&fab, 0, 0) & 1))
    return -1;
  SYS$CLOSE (&fab, 0, 0);
  /* As this is a "replacement" for creat, return a file descriptor
     opened for writing. */
  return open (new, O_WRONLY);
}

#ifdef VMS /* VMS_NEED_SYS_CREAT */
/* #undef creat */
#include <varargs.h>
#ifdef __GNUC__
#ifndef va_count
#define va_count(X) ((X) = *(((int *) &(va_alist)) - 1))
#endif
#endif /* __GNUC__ */

int
sys_creat (name, mode, rfd)
     char *name;
     int mode, rfd;		/* rfd < 0 means use original */
{
  int fd;			/* Our new file descriptor */
  struct stat st_buf;
  char rfm[12];
  char rat[15];
  char mrs[13];
  char fsz[13];
  extern int vms_stmlf_recfm;

  if (rfd >= 0)
    {
      /* Use information from the related file descriptor to set record
	 format of the newly created file. */
      fstat (rfd, &st_buf);
      switch (st_buf.st_fab_rfm)
	{
	case FAB$C_FIX:
	  strcpy (rfm, "rfm = fix");
	  sprintf (mrs, "mrs = %d", st_buf.st_fab_mrs);
	  strcpy (rat, "rat = ");
	  if (st_buf.st_fab_rat & FAB$M_CR)
	    strcat (rat, "cr");
	  else if (st_buf.st_fab_rat & FAB$M_FTN)
	    strcat (rat, "ftn");
	  else if (st_buf.st_fab_rat & FAB$M_PRN)
	    strcat (rat, "prn");
	  if (st_buf.st_fab_rat & FAB$M_BLK)
	    if (st_buf.st_fab_rat & (FAB$M_CR|FAB$M_FTN|FAB$M_PRN))
	      strcat (rat, ", blk");
	    else
	      strcat (rat, "blk");
	  return creat (name, 0, rfm, rat, mrs);

	case FAB$C_VFC:
	  strcpy (rfm, "rfm = vfc");
	  sprintf (fsz, "fsz = %d", st_buf.st_fab_fsz);
	  strcpy (rat, "rat = ");
	  if (st_buf.st_fab_rat & FAB$M_CR)
	    strcat (rat, "cr");
	  else if (st_buf.st_fab_rat & FAB$M_FTN)
	    strcat (rat, "ftn");
	  else if (st_buf.st_fab_rat & FAB$M_PRN)
	    strcat (rat, "prn");
	  if (st_buf.st_fab_rat & FAB$M_BLK)
	    if (st_buf.st_fab_rat & (FAB$M_CR|FAB$M_FTN|FAB$M_PRN))
	      strcat (rat, ", blk");
	    else
	      strcat (rat, "blk");
	  return creat (name, 0, rfm, rat, fsz);

	case FAB$C_STM:
	  strcpy (rfm, "rfm = stm");
	  break;

	case FAB$C_STMCR:
	  strcpy (rfm, "rfm = stmcr");
	  break;

	case FAB$C_STMLF:
	  strcpy (rfm, "rfm = stmlf");
	  break;

	case FAB$C_UDF:
	  strcpy (rfm, "rfm = udf");
	  break;

	case FAB$C_VAR:
	  strcpy (rfm, "rfm = var");
	  break;
	}
      strcpy (rat, "rat = ");
      if (st_buf.st_fab_rat & FAB$M_CR)
	strcat (rat, "cr");
      else if (st_buf.st_fab_rat & FAB$M_FTN)
	strcat (rat, "ftn");
      else if (st_buf.st_fab_rat & FAB$M_PRN)
	strcat (rat, "prn");
      if (st_buf.st_fab_rat & FAB$M_BLK)
	if (st_buf.st_fab_rat & (FAB$M_CR|FAB$M_FTN|FAB$M_PRN))
	  strcat (rat, ", blk");
	else
	  strcat (rat, "blk");
    }
  else
    {
      strcpy (rfm, vms_stmlf_recfm ? "rfm = stmlf" : "rfm=var");
      strcpy (rat, "rat=cr");
    }
  /* Until the VAX C RTL fixes the many bugs with modes, always use
     mode 0 to get the user's default protection. */
  fd = creat (name, 0, rfm, rat);
  if (fd < 0 && errno == EEXIST)
    {
      if (unlink (name) < 0)
	report_file_error ("delete", build_string (name));
      fd = creat (name, 0, rfm, rat);
    }
  return fd;
}
#define creat sys_creat
#endif /* VMS_NEED_SYS_CREAT */

#ifdef VMS_NEED_SYS_FWRITE
/* fwrite to stdout is S L O W.  Speed it up by using fputc...*/
size_t
sys_fwrite (const void *ptr, size_t size, size_t num, FILE *fp)
{
  const unsigned char *p = ptr;
  size_t tot = num * size;

  while (tot--)
    fputc (*p++, fp);
  return num;
}
#endif /* VMS_NEED_SYS_FWRITE */

/* Define this symbol to actually read SYSUAF.DAT.  This requires either
   SYSPRV or a readable SYSUAF.DAT. */

#ifdef READ_SYSUAF
/*
 * getuaf.c
 *
 * Routine to read the VMS User Authorization File and return
 * a specific user's record.
 */

static struct UAF retuaf;

struct UAF *
get_uaf_name (uname)
     char * uname;
{
  register status;
  struct FAB uaf_fab;
  struct RAB uaf_rab;

  uaf_fab = cc$rms_fab;
  uaf_rab = cc$rms_rab;
  /* initialize fab fields */
  uaf_fab.fab$l_fna = "SYS$SYSTEM:SYSUAF.DAT";
  uaf_fab.fab$b_fns = 21;
  uaf_fab.fab$b_fac = FAB$M_GET;
  uaf_fab.fab$b_org = FAB$C_IDX;
  uaf_fab.fab$b_shr = FAB$M_GET|FAB$M_PUT|FAB$M_UPD|FAB$M_DEL;
  /* initialize rab fields */
  uaf_rab.rab$l_fab = &uaf_fab;
  /* open the User Authorization File */
  status = SYS$OPEN (&uaf_fab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  status = SYS$CONNECT (&uaf_rab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  /* read the requested record - index is in uname */
  uaf_rab.rab$l_kbf = uname;
  uaf_rab.rab$b_ksz = strlen (uname);
  uaf_rab.rab$b_rac = RAB$C_KEY;
  uaf_rab.rab$l_ubf = (char *)&retuaf;
  uaf_rab.rab$w_usz = sizeof retuaf;
  status = SYS$GET (&uaf_rab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  /* close the User Authorization File */
  status = SYS$DISCONNECT (&uaf_rab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  status = SYS$CLOSE (&uaf_fab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  return &retuaf;
}

struct UAF *
get_uaf_uic (uic)
     unsigned long uic;
{
  register status;
  struct FAB uaf_fab;
  struct RAB uaf_rab;

  uaf_fab = cc$rms_fab;
  uaf_rab = cc$rms_rab;
  /* initialize fab fields */
  uaf_fab.fab$l_fna = "SYS$SYSTEM:SYSUAF.DAT";
  uaf_fab.fab$b_fns = 21;
  uaf_fab.fab$b_fac = FAB$M_GET;
  uaf_fab.fab$b_org = FAB$C_IDX;
  uaf_fab.fab$b_shr = FAB$M_GET|FAB$M_PUT|FAB$M_UPD|FAB$M_DEL;
  /* initialize rab fields */
  uaf_rab.rab$l_fab = &uaf_fab;
  /* open the User Authorization File */
  status = SYS$OPEN (&uaf_fab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  status = SYS$CONNECT (&uaf_rab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  /* read the requested record - index is in uic */
  uaf_rab.rab$b_krf = 1;	/* 1st alternate key */
  uaf_rab.rab$l_kbf = (char *) &uic;
  uaf_rab.rab$b_ksz = sizeof uic;
  uaf_rab.rab$b_rac = RAB$C_KEY;
  uaf_rab.rab$l_ubf = (char *)&retuaf;
  uaf_rab.rab$w_usz = sizeof retuaf;
  status = SYS$GET (&uaf_rab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  /* close the User Authorization File */
  status = SYS$DISCONNECT (&uaf_rab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  status = SYS$CLOSE (&uaf_fab);
  if (!(status&1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  return &retuaf;
}

static struct passwd retpw;

struct passwd *
cnv_uaf_pw (up)
     struct UAF * up;
{
  char * ptr;

  /* copy these out first because if the username is 32 chars, the next
     section will overwrite the first byte of the UIC */
  retpw.pw_uid = up->uaf$w_mem;
  retpw.pw_gid = up->uaf$w_grp;

  /* I suppose this is not the best style, to possibly overwrite one
     byte beyond the end of the field, but what the heck... */
  ptr = &up->uaf$t_username[UAF$S_USERNAME];
  while (ptr[-1] == ' ')
    ptr--;
  *ptr = '\0';
  strcpy (retpw.pw_name, up->uaf$t_username);

  /* the rest of these are counted ascii strings */
  strncpy (retpw.pw_gecos, &up->uaf$t_owner[1], up->uaf$t_owner[0]);
  retpw.pw_gecos[up->uaf$t_owner[0]] = '\0';
  strncpy (retpw.pw_dir, &up->uaf$t_defdev[1], up->uaf$t_defdev[0]);
  retpw.pw_dir[up->uaf$t_defdev[0]] = '\0';
  strncat (retpw.pw_dir, &up->uaf$t_defdir[1], up->uaf$t_defdir[0]);
  retpw.pw_dir[up->uaf$t_defdev[0] + up->uaf$t_defdir[0]] = '\0';
  strncpy (retpw.pw_shell, &up->uaf$t_defcli[1], up->uaf$t_defcli[0]);
  retpw.pw_shell[up->uaf$t_defcli[0]] = '\0';

  return &retpw;
}
#else /* not READ_SYSUAF */
static struct passwd retpw;
#endif /* not READ_SYSUAF */

/* struct passwd * */
/* getpwnam (name) */
/*      char * name; */
/* { */
/* #ifdef READ_SYSUAF */
/*   struct UAF *up; */
/* #else */
/*   char * user; */
/*   char * dir; */
/*   unsigned char * full; */
/* #endif /\* READ_SYSUAF *\/ */
/*   char *ptr = name; */

/*   while (*ptr) */
/*     { */
/*       if ('a' <= *ptr && *ptr <= 'z') */
/* 	*ptr -= 040; */
/*       ptr++; */
/*     } */
/* #ifdef READ_SYSUAF */
/*   if (!(up = get_uaf_name (name))) */
/*     return 0; */
/*   return cnv_uaf_pw (up); */
/* #else */
/*   if (strcmp (name, sys_getenv ("USER")) == 0) */
/*     { */
/*       retpw.pw_uid = getuid (); */
/*       retpw.pw_gid = getgid (); */
/*       strcpy (retpw.pw_name, name); */
/*       if (full = egetenv ("FULLNAME")) */
/* 	strcpy (retpw.pw_gecos, full); */
/*       else */
/* 	*retpw.pw_gecos = '\0'; */
/*       strcpy (retpw.pw_dir, egetenv ("HOME")); */
/*       *retpw.pw_shell = '\0'; */
/*       return &retpw; */
/*     } */
/*   else */
/*     return 0; */
/* #endif /\* not READ_SYSUAF *\/ */
/* } */

#ifdef VMS_NEED_SYS_GETPWUID
#undef getpwuid
struct passwd *
sys_getpwuid (uid)		/* ttn (added sys_ prefix) */
     uid_t uid;
{
#ifdef READ_SYSUAF
  struct UAF * up;

  if (!(up = get_uaf_uic (uid)))
    return 0;
  return cnv_uaf_pw (up);
#else
  if (uid == sys_getuid ())
    return getpwnam (egetenv ("USER"));
  else
    return 0;
#endif /* not READ_SYSUAF */
}
#define getpwuid sys_getpwuid
#endif /* VMS_NEED_SYS_GETPWUID */

/* return total address space available to the current process.  This is
   the sum of the current p0 size, p1 size and free page table entries
   available. */
int
sys_vlimit (ig1, ig2)
     int ig1, ig2;		/* ignored */
{
  int item_code;
  unsigned long free_pages;
  unsigned long frep0va;
  unsigned long frep1va;
  register status;

  item_code = JPI$_FREPTECNT;
  if (((status = LIB$GETJPI (&item_code, 0, 0, &free_pages)) & 1) == 0)
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
  free_pages *= 8192; /* was 512; */

  item_code = JPI$_FREP0VA;
  if (((status = LIB$GETJPI (&item_code, 0, 0, &frep0va)) & 1) == 0)
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
  item_code = JPI$_FREP1VA;
  if (((status = LIB$GETJPI (&item_code, 0, 0, &frep1va)) & 1) == 0)
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }

  return free_pages + frep0va + (0x7fffffff - frep1va);
}

int
ulimit ()
{
  return 0;
}

int
execvp2 ()
{
  error ("execvp system call not implemented");
  return -1;
}

int
sys_execvp (const char *__file, __char_ptr_const_ptr32 __argv)
{
  fprintf (stderr, "sys_execvp not implemented --ttn\n");
  abort();
#if 0
  decc$execvp(__file,__argv);
#endif /* 0 */
  return(0); /* mainly to trick compiler */
}

int
rename (const char *from, const char *to)
{
  int status;
  struct FAB from_fab = cc$rms_fab, to_fab = cc$rms_fab;
  struct NAM from_nam = cc$rms_nam, to_nam = cc$rms_nam;
  char from_esn[NAM$C_MAXRSS];
  char to_esn[NAM$C_MAXRSS];

  from_fab.fab$l_fna = (char *) from;
  from_fab.fab$b_fns = strlen (from);
  from_fab.fab$l_nam = &from_nam;
  from_fab.fab$l_fop = FAB$M_NAM;

  from_nam.nam$l_esa = from_esn;
  from_nam.nam$b_ess = sizeof from_esn;

  to_fab.fab$l_fna = (char *) to;
  to_fab.fab$b_fns = strlen (to);
  to_fab.fab$l_nam = &to_nam;
  to_fab.fab$l_fop = FAB$M_NAM;

  to_nam.nam$l_esa = to_esn;
  to_nam.nam$b_ess = sizeof to_esn;

  status = SYS$RENAME (&from_fab, 0, 0, &to_fab);

  if (status & 1)
    return 0;
  else
    {
      if (status == RMS$_DEV)
	errno = EXDEV;
      else
	errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
}

/* This function renames a file like `rename', but it strips
   the version number from the "to" filename, such that the "to" file is
   will always be a new version.  It also sets the file protection once it is
   finished.  The protection that we will use is stored in fab_final_pro,
   and was set when we did a creat_copy_attrs to create the file that we
   are renaming.

   We could use the chmod function, but Eunichs uses 3 bits per user category
   to describe the protection, and VMS uses 4 (write and delete are separate
   bits).  To maintain portability, the VMS implementation of `chmod' wires
   the W and D bits together.  */


static struct fibdef fib;	/* We need this initialized to zero */
char vms_file_written[NAM$C_MAXRSS];

int
rename_sans_version (from, to)
     char *from, *to;
{
  short int chan;
  int stat;
  short int iosb[4];
  int status;
  struct FAB to_fab = cc$rms_fab;
  struct NAM to_nam = cc$rms_nam;
  struct dsc$descriptor fib_d ={sizeof (fib),0,0,(char*) &fib};
  struct dsc$descriptor fib_attr[2]
    = {{sizeof (fab_final_pro),ATR$C_FPRO,0,(char*) &fab_final_pro},{0,0,0,0}};
  char to_esn[NAM$C_MAXRSS];

  $DESCRIPTOR (disk,to_esn);

  to_fab.fab$l_fna = to;
  to_fab.fab$b_fns = strlen (to);
  to_fab.fab$l_nam = &to_nam;
  to_fab.fab$l_fop = FAB$M_NAM;

  to_nam.nam$l_esa = to_esn;
  to_nam.nam$b_ess = sizeof to_esn;

  status = SYS$PARSE (&to_fab, 0, 0); /* figure out the full file name */

  if (to_nam.nam$l_fnb && NAM$M_EXP_VER)
    *(to_nam.nam$l_ver) = '\0';

  stat = rename (from, to_esn);
  if (stat < 0)
    return stat;

  strcpy (vms_file_written, to_esn);

  to_fab.fab$l_fna = vms_file_written; /* this points to the versionless name */
  to_fab.fab$b_fns = strlen (vms_file_written);

  /* Now set the file protection to the correct value */
  SYS$OPEN (&to_fab, 0, 0);	/* This fills in the nam$w_fid fields */

  /* Copy these fields into the fib */
/*
  fib.fib$r_fid_overlay.fib$w_fid[0] = to_nam.nam$w_fid[0];
  fib.fib$r_fid_overlay.fib$w_fid[1] = to_nam.nam$w_fid[1];
  fib.fib$r_fid_overlay.fib$w_fid[2] = to_nam.nam$w_fid[2];
*/
  fib.fib$w_fid[0] = to_nam.nam$w_fid[0];
  fib.fib$w_fid[1] = to_nam.nam$w_fid[1];
  fib.fib$w_fid[2] = to_nam.nam$w_fid[2];

  SYS$CLOSE (&to_fab, 0, 0);

  stat = SYS$ASSIGN (&disk, &chan, 0, 0); /* open a channel to the disk */
  if (!stat)
    LIB$SIGNAL (stat);
  stat = SYS$QIOW (0, chan, IO$_MODIFY, iosb, 0, 0, &fib_d,
		   0, 0, 0, &fib_attr, 0);
  if (!stat)
    LIB$SIGNAL (stat);
  stat = SYS$DASSGN (chan);
  if (!stat)
    LIB$SIGNAL (stat);
  strcpy (vms_file_written, to_esn); /* We will write this to the terminal*/
  return 0;
}

/* int */
/* link (file, new) */
/*      char * file, * new; */
/* { */
/*   register status; */
/*   struct FAB fab; */
/*   struct NAM nam; */
/*   unsigned short fid[3]; */
/*   char esa[NAM$C_MAXRSS]; */

/*   fab = cc$rms_fab; */
/*   fab.fab$l_fop = FAB$M_OFP; */
/*   fab.fab$l_fna = file; */
/*   fab.fab$b_fns = strlen (file); */
/*   fab.fab$l_nam = &nam; */

/*   nam = cc$rms_nam; */
/*   nam.nam$l_esa = esa; */
/*   nam.nam$b_ess = NAM$C_MAXRSS; */

/*   status = SYS$PARSE (&fab); */
/*   if ((status & 1) == 0) */
/*     { */
/*       errno = EVMSERR; */
/*       vaxc$errno = status; */
/*       return -1; */
/*     } */
/*   status = SYS$SEARCH (&fab); */
/*   if ((status & 1) == 0) */
/*     { */
/*       errno = EVMSERR; */
/*       vaxc$errno = status; */
/*       return -1; */
/*     } */

/*   fid[0] = nam.nam$w_fid[0]; */
/*   fid[1] = nam.nam$w_fid[1]; */
/*   fid[2] = nam.nam$w_fid[2]; */

/*   fab.fab$l_fna = new; */
/*   fab.fab$b_fns = strlen (new); */

/*   status = SYS$PARSE (&fab); */
/*   if ((status & 1) == 0) */
/*     { */
/*       errno = EVMSERR; */
/*       vaxc$errno = status; */
/*       return -1; */
/*     } */

/*   nam.nam$w_fid[0] = fid[0]; */
/*   nam.nam$w_fid[1] = fid[1]; */
/*   nam.nam$w_fid[2] = fid[2]; */

/*   nam.nam$l_esa = nam.nam$l_name; */
/*   nam.nam$b_esl = nam.nam$b_name + nam.nam$b_type + nam.nam$b_ver; */

/*   status = SYS$ENTER (&fab); */
/*   if ((status & 1) == 0) */
/*     { */
/*       errno = EVMSERR; */
/*       vaxc$errno = status; */
/*       return -1; */
/*     } */

/*   return 0; */
/* } */

void
croak (badfunc)
     char *badfunc;
{
  printf ("%s not yet implemented\r\n", badfunc);
  reset_sys_modes ();
  exit (EXIT_FAILURE);
}

#endif /* VMS */

#ifdef AIXHFT

/* Called from init_sys_modes.  */
void
hft_init ()
{
  int junk;

  /* If we're not on an HFT we shouldn't do any of this.  We determine
     if we are on an HFT by trying to get an HFT error code.  If this
     call fails, we're not on an HFT. */
#ifdef IBMR2AIX
  if (ioctl (0, HFQERROR, &junk) < 0)
    return;
#else /* not IBMR2AIX */
  if (ioctl (0, HFQEIO, 0) < 0)
    return;
#endif /* not IBMR2AIX */

  /* On AIX the default hft keyboard mapping uses backspace rather than delete
     as the rubout key's ASCII code.  Here this is changed.  The bug is that
     there's no way to determine the old mapping, so in reset_sys_modes
     we need to assume that the normal map had been present.  Of course, this
     code also doesn't help if on a terminal emulator which doesn't understand
     HFT VTD's.  */
  {
    struct hfbuf buf;
    struct hfkeymap keymap;

    buf.hf_bufp = (char *)&keymap;
    buf.hf_buflen = sizeof (keymap);
    keymap.hf_nkeys = 2;
    keymap.hfkey[0].hf_kpos = 15;
    keymap.hfkey[0].hf_kstate = HFMAPCHAR | HFSHFNONE;
#ifdef IBMR2AIX
    keymap.hfkey[0].hf_keyidh = '<';
#else /* not IBMR2AIX */
    keymap.hfkey[0].hf_page = '<';
#endif /* not IBMR2AIX */
    keymap.hfkey[0].hf_char = 127;
    keymap.hfkey[1].hf_kpos = 15;
    keymap.hfkey[1].hf_kstate = HFMAPCHAR | HFSHFSHFT;
#ifdef IBMR2AIX
    keymap.hfkey[1].hf_keyidh = '<';
#else /* not IBMR2AIX */
    keymap.hfkey[1].hf_page = '<';
#endif /* not IBMR2AIX */
    keymap.hfkey[1].hf_char = 127;
    hftctl (0, HFSKBD, &buf);
  }
  /* The HFT system on AIX doesn't optimize for scrolling, so it's really ugly
     at times.  */
  line_ins_del_ok = char_ins_del_ok = 0;
}

/* Reset the rubout key to backspace.  */

void
hft_reset ()
{
  struct hfbuf buf;
  struct hfkeymap keymap;
  int junk;

#ifdef IBMR2AIX
  if (ioctl (0, HFQERROR, &junk) < 0)
    return;
#else /* not IBMR2AIX */
  if (ioctl (0, HFQEIO, 0) < 0)
    return;
#endif /* not IBMR2AIX */

  buf.hf_bufp = (char *)&keymap;
  buf.hf_buflen = sizeof (keymap);
  keymap.hf_nkeys = 2;
  keymap.hfkey[0].hf_kpos = 15;
  keymap.hfkey[0].hf_kstate = HFMAPCHAR | HFSHFNONE;
#ifdef IBMR2AIX
  keymap.hfkey[0].hf_keyidh = '<';
#else /* not IBMR2AIX */
  keymap.hfkey[0].hf_page = '<';
#endif /* not IBMR2AIX */
  keymap.hfkey[0].hf_char = 8;
  keymap.hfkey[1].hf_kpos = 15;
  keymap.hfkey[1].hf_kstate = HFMAPCHAR | HFSHFSHFT;
#ifdef IBMR2AIX
  keymap.hfkey[1].hf_keyidh = '<';
#else /* not IBMR2AIX */
  keymap.hfkey[1].hf_page = '<';
#endif /* not IBMR2AIX */
  keymap.hfkey[1].hf_char = 8;
  hftctl (0, HFSKBD, &buf);
}

#endif /* AIXHFT */

#ifdef USE_DL_STUBS

/* These are included on Sunos 4.1 when we do not use shared libraries.
   X11 libraries may refer to these functions but (we hope) do not
   actually call them.  */

void *
dlopen ()
{
  return 0;
}

void *
dlsym ()
{
  return 0;
}

int
dlclose ()
{
  return -1;
}

#endif /* USE_DL_STUBS */

#ifndef BSTRING

#ifndef bzero

void
bzero (b, length)
     register char *b;
     register int length;
{
#ifdef VMS
  short zero = 0;
  long max_str = 65535;

  while (length > max_str) {
    (void) LIB$MOVC5 (&zero, &zero, &zero, &max_str, b);
    length -= max_str;
    b += max_str;
  }
  max_str = length;
  (void) LIB$MOVC5 (&zero, &zero, &zero, &max_str, b);
#else
  while (length-- > 0)
    *b++ = 0;
#endif /* not VMS */
}

#endif /* no bzero */
#endif /* BSTRING */

#if (!defined (BSTRING) && !defined (bcopy)) || defined (NEED_BCOPY)
#undef bcopy

/* Saying `void' requires a declaration, above, where bcopy is used
   and that declaration causes pain for systems where bcopy is a macro.  */
bcopy (b1, b2, length)
     register char *b1;
     register char *b2;
     register int length;
{
#ifdef VMS
  long max_str = 65535;

  while (length > max_str) {
    (void) LIB$MOVC3 (&max_str, b1, b2);
    length -= max_str;
    b1 += max_str;
    b2 += max_str;
  }
  max_str = length;
  (void) LIB$MOVC3 (&length, b1, b2);
#else
  while (length-- > 0)
    *b2++ = *b1++;
#endif /* not VMS */
}
#endif /* (!defined (BSTRING) && !defined (bcopy)) || defined (NEED_BCOPY) */

#ifndef BSTRING
#ifndef bcmp
int
#ifdef VMS
bcmp2 (b1, b2, length)	/* This could be a macro! */
#else
bcmp (b1, b2, length)   /* This could be a macro! */
#endif
     register char *b1;
     register char *b2;
     register int length;
{
#ifdef VMS
  struct dsc$descriptor_s src1 = {length, DSC$K_DTYPE_T, DSC$K_CLASS_S, b1};
  struct dsc$descriptor_s src2 = {length, DSC$K_DTYPE_T, DSC$K_CLASS_S, b2};

  return STR$COMPARE (&src1, &src2);
#else
  while (length-- > 0)
    if (*b1++ != *b2++)
      return 1;

  return 0;
#endif /* not VMS */
}
#endif /* no bcmp */
#endif /* not BSTRING */

#ifndef HAVE_STRSIGNAL
char *
strsignal (signo)
     int signo;
{
  char *signame = 0;

  if (0 <= signo && signo < NSIG)
    /* Cast to suppress warning if the table has const char *.  */
    signame = (char *) sys_siglist[signo];

  return signame;
}
#endif /* HAVE_STRSIGNAL */

#ifdef VMS
fork ()
{
  return vfork ();
}
#endif /* VMS */


/* Time support for VMS on this page.  */
#ifdef VMS

#include <time.h>

time_t
emacs_vms_time (void)
{
  return time ((long *) 0);
}

#ifdef __CRTL_VER
#if __CRTL_VER < 70000000
extern long timezone;
extern int daylight;
extern char *tzname[2];
#endif /* for __CRTL_VER >= 70000000 they were declared in time.h */
#else /* ! __CRTL_VER */
extern long timezone;
extern int daylight;
extern char *tzname[2];
#endif /* ! __CRTL_VER */

#ifdef VMS_NEED_SYS_TZSET
void sys_tzset (void);
#endif
#ifdef VMS_NEED_SYS_LOCALTIME
struct tm *sys_localtime (const time_t *clock);
#endif
#ifdef VMS_NEED_SYS_GMTIME
struct tm *sys_gmtime (const time_t *clock);
#endif

static char tzname_default[20] = "";
static char tzname_dst[20] = "";

//static char *tzname[2] = { tzname_default, tzname_dst };

static long internal_daylight = 0;
static char daylight_set = 0;

static long
read_time (const char *nptr, const char **endptr,
	   int sign_allowed_p)
{
  int t;

  *endptr = nptr;

  /* This routine trusts the user very much, and does no checks!
      The only exception is this: */
  if (!sign_allowed_p && (*nptr == '-' || *nptr == '+'))
    return 0;

  t = strtol(*endptr, endptr, 10) * 3600;
  if (**endptr != ':' || **endptr == '+' || **endptr == '-')
    return t;
  (*endptr)++;

  t = t + strtol(*endptr, endptr, 10) * 60;
  if (**endptr != ':' || **endptr == '+' || **endptr == '-')
    return t;
  (*endptr)++;

  return t + strtol(*endptr, endptr, 10);
}

static void
read_dst_time (const char *nptr, const char **endptr,
	       int *m, int *n, int *d,
	       int *leap_p)
{
  time_t bintim = time(0);
  struct tm *lc = localtime(&bintim);

  *leap_p = 1;
  *m = 0;			/* When m and n are 0, a Julian */
  *n = 0;			/* date has been inserted in d */

  switch(*nptr)
    {
    case 'M':
      {
	/* This routine counts on the user to have specified "Mm.n.d",
	   where 1 <= n <= 5, 1 <= m <= 12, 0 <= d <= 6 */

	*m = strtol(++nptr, endptr, 10);
	(*endptr)++;		/* Skip the dot */
	*n = strtol(*endptr, endptr, 10);
	(*endptr)++;		/* Skip the dot */
	*d = strtol(*endptr, endptr, 10);

	return;
      }
    case 'J':
      *leap_p = 0;		/* Never count with leap years */
    default: /* trust the user to have inserted a number! */
      *d = strtol(++nptr, endptr, 10);
      return;
    }
}

struct vms_vectim
{
  short year, month, day, hour, minute, second, centi_second;
};
static void
find_dst_time (int m, int n, long d,
	       int hour, int minute, int second,
	       int leap_p,
	       long vms_internal_time[2])
{
  long status = SYS$GETTIM(vms_internal_time);
  struct vms_vectim vms_vectime;
  status = SYS$NUMTIM(&vms_vectime, vms_internal_time);

  if (m == 0 && n == 0)
    {
      long tmp_vms_internal_time[2][2];
      long day_of_year;
      long tmp_operation = LIB$K_DAY_OF_YEAR;

      status = LIB$CVT_FROM_INTERNAL_TIME(&tmp_operation, &day_of_year,
					  vms_internal_time);

      vms_vectime.month = 2;
      vms_vectime.day = 29;
      status = LIB$CVT_VECTIM(&vms_vectime, tmp_vms_internal_time[0]);
      if (status & 1) /* This is a leap year */
	{
	  if (!leap_p && d > 59)
	    d ++;		/* If we don't count with 29th Feb,
				   and this is a leap year, count up,
				   to make day 60 really become the
				   1st March. */
	}
      /* 1st January, at midnight */
      vms_vectime.month = 1;
      vms_vectime.day = 1;
      vms_vectime.hour = hour;
      vms_vectime.minute = minute;
      vms_vectime.second = second;
      vms_vectime.centi_second = 0;
      status = LIB$CVT_VECTIM(&vms_vectime, tmp_vms_internal_time[0]);
      tmp_operation = LIB$K_DELTA_DAYS;
      status = LIB$CVT_TO_INTERNAL_TIME(&tmp_operation, &d,
					tmp_vms_internal_time[1]);
      /* now, tmp_vms_interval_time[0] contains 1st Jan, 00:00:00,
	 and  tmp_vms_interval_time[1] contains delta time +d days.
	 Let's just add them together */
      status = LIB$ADD_TIMES(tmp_vms_internal_time[0],
			     tmp_vms_internal_time[1],
			     vms_internal_time);
    }
  else
    {
      long tmp_vms_internal_time[2];
      long day_of_week;
      long tmp_operation = LIB$K_DAY_OF_YEAR;

      if (d == 0) /* 0 is Sunday, which isn't compatible with VMS,
		     where day_of_week is 1 -- 7, and 1 is Monday */
	{
	  d = 7; /* So a simple conversion is required */
	}
      vms_vectime.month = m;
      vms_vectime.day = 1;
      vms_vectime.hour = hour;
      vms_vectime.minute = minute;
      vms_vectime.second = second;
      vms_vectime.centi_second = 0;
      status = LIB$CVT_VECTIM(&vms_vectime, tmp_vms_internal_time);
      tmp_operation = LIB$K_DAY_OF_WEEK;
      status = LIB$CVT_FROM_INTERNAL_TIME(&tmp_operation, &day_of_week,
					  tmp_vms_internal_time);
      d -= day_of_week;
      if (d < 0)
	{
	  d += 7;
	}
      vms_vectime.day += (n-1)*7 + d;
      status = LIB$CVT_VECTIM(&vms_vectime, vms_internal_time);
      if (!(status & 1))
	{
	  vms_vectime.day -= 7;	/* n was probably 5 */
	  status = LIB$CVT_VECTIM(&vms_vectime, vms_internal_time);
	}
    }
}

static int
cmp_vms_internal_times (long vms_internal_time1[2],
			long vms_internal_time2[2])
{
  if (vms_internal_time1[1] < vms_internal_time2[1])
    return -1;
  else
    if (vms_internal_time1[1] > vms_internal_time2[1])
      return 1;

  if (vms_internal_time1[0] < vms_internal_time2[0])
    return -1;
  else
    if (vms_internal_time1[0] > vms_internal_time2[0])
      return 1;

  return 0;
}

/* -------------------------- Global routines ------------------------------ */

#ifdef VMS_NEED_SYS_TZSET
#undef tzset
void
sys_tzset (void)
{
  char *TZ;
  char *p, *q;

  if (daylight_set)
    return;

  daylight = 0;

  if ((TZ = getenv("TZ")) == 0)
    return;

  p = TZ;
  q = tzname[0];

  while(*p != '\0'
	&& (*p <'0' || *p > '9') && *p != '-' && *p != '+' && *p != ',')
    *q++ = *p++;
  *q = '\0';

  /* This is special for VMS, so I don't care if it doesn't exist anywhere
     else */

  timezone = read_time (p, &p, 1);

  q = tzname[1];

  while(*p != '\0'
	&& (*p <'0' || *p > '9') && *p != '-' && *p != '+' && *p != ',')
    *q++ = *p++;
  *q = '\0';

  internal_daylight = ((*p != '-' && *p != '+' && !(*p >='0' && *p <= '9'))
		       ? timezone - 3600
		       : read_time (p, &p, 1));

  if (*p == ',')
    {
      int start_m;
      int start_n;
      int start_d;
      int start_leap_p;
      int start_hour=2, start_minute=0, start_second=0;

      p++;
      read_dst_time (p, &p, &start_m, &start_n, &start_d, &start_leap_p);
      if (*p == '/')
	{
	  long tmp = read_time (++p, &p, 0);
	  start_hour = tmp / 3600;
	  start_minute = (tmp % 3600) / 60;
	  start_second = tmp % 60;
	}
      if (*p == ',')
	{
	  int end_m;
	  int end_n;
	  int end_d;
	  int end_leap_p;
	  int end_hour=2, end_minute=0, end_second=0;

	  p++;
	  read_dst_time (p, &p, &end_m, &end_n, &end_d, &end_leap_p);
	  if (*p == '/')
	    {
	      long tmp = read_time (++p, &p, 0);
	      end_hour = tmp / 3600;
	      end_minute = (tmp % 3600) / 60;
	      end_second = tmp % 60;
	    }
	  {
	    long vms_internal_time[3][2];
	    find_dst_time (start_m, start_n, start_d,
			   start_hour, start_minute, start_second,
			   start_leap_p,
			   vms_internal_time[0]);
	    SYS$GETTIM (&vms_internal_time[1]);
	    find_dst_time (end_m, end_n, end_d,
			   end_hour, end_minute, end_second,
			   end_leap_p,
			   vms_internal_time[2]);
	    if (cmp_vms_internal_times (vms_internal_time[0],
				        vms_internal_time[1]) < 0
		&& cmp_vms_internal_times (vms_internal_time[1],
					   vms_internal_time[2]) < 0)
	      daylight = 1;
	  }
	}
    }
}
#define tzset sys_tzset
#endif /* VMS_NEED_SYS_TZSET */

#ifdef VMS_NEED_SYS_LOCALTIME
#undef localtime
struct tm *
sys_localtime (const time_t *clock)
{
  struct tm *tmp = localtime (clock);

  tzset ();
  tmp->tm_isdst = daylight;

  return tmp;
}
#define localtime sys_localtime
#endif /* VMS_NEED_SYS_LOCALTIME */

#ifdef VMS_NEED_SYS_GMTIME
#undef gmtime
struct tm *
sys_gmtime (const time_t *clock)
{
  static struct tm gmt;
  struct vms_vectim tmp_vectime;
  long vms_internal_time[3][2];
  long tmp_operation = LIB$K_DELTA_SECONDS;
  long status;
  long tmp_offset;
  char tmp_o_sign;

  tzset ();

  if (daylight)
    tmp_offset = internal_daylight;
  else
    tmp_offset = timezone;

  if (tmp_offset < 0)
    {
      tmp_o_sign = -1;
      tmp_offset = -tmp_offset;
    }
  else
    tmp_o_sign = 1;

  status = LIB$CVT_TO_INTERNAL_TIME (&tmp_operation, &tmp_offset,
				     vms_internal_time[1]);
  status = SYS$GETTIM (vms_internal_time[0]);
  status = (*((tmp_o_sign < 0) ? LIB$SUB_TIMES : LIB$ADD_TIMES)
	    (vms_internal_time[0],
	     vms_internal_time[1],
	     vms_internal_time[2]));

  status = SYS$NUMTIM (&tmp_vectime, vms_internal_time[2]);
  gmt.tm_sec = tmp_vectime.second;
  gmt.tm_min = tmp_vectime.minute;
  gmt.tm_hour = tmp_vectime.hour;
  gmt.tm_mday = tmp_vectime.day;
  gmt.tm_mon = tmp_vectime.month - 1;
  gmt.tm_year = tmp_vectime.year - 1900;

  tmp_operation = LIB$K_DAY_OF_WEEK;
  status = LIB$CVT_FROM_INTERNAL_TIME (&tmp_operation,
				       &gmt.tm_wday,
				       vms_internal_time[2]);
  if (gmt.tm_wday == 7)
    gmt.tm_wday = 0;

  tmp_operation = LIB$K_DAY_OF_YEAR;
  status = LIB$CVT_FROM_INTERNAL_TIME (&tmp_operation,
				       &gmt.tm_yday,
				       vms_internal_time[2]);
  gmt.tm_yday--;
  gmt.tm_isdst = daylight;

  return &gmt;
}
#define gmtime sys_gmtime
#endif /* VMS_NEED_SYS_GMTIME */

#endif /* VMS */

/* sysdep.c ends here */

/* arch-tag: edb43589-4e09-4544-b325-978b5b121dcf
   (do not change this comment) */
