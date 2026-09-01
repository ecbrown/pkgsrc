/* Interfaces to subprocesses on VMS.
   Copyright (C) 1988, 1994 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


#include	<ssdef.h>
#include	<iodef.h>
#include	<dcdef.h>
#include	<ttdef.h>
#include	<tt2def.h>
#include	<dvidef.h>
#include	<clidef.h>
#ifndef CLI$M_AUTHPRIV		/* Not defined with VAX C!!!  */
#define CLI$M_AUTHPRIV	128
#endif
#include	<libdef.h>
#include	<descrip.h>
#include	<signal.h>
#include 	<errno.h>
#include        <ctype.h>
#include        <limits.h>
#include        <string.h>
#include        <unixlib.h>
#include	<sys/file.h>

#include	<lib$routines.h>

#include	"config.h"

#include <unistd.h>		/* for chdir */
#include <starlet.h>

/* We need to do the following, or we may get declaration conflicts.  */
#ifdef select
#undef select
#endif
#ifdef connect
#undef connect
#endif

#ifdef HAVE_SOCKETS
#ifdef MULTINET
#include "multinet_root:[multinet.include.vms]inetiodef.h"
#include "multinet_root:[multinet.include.sys]ioctl.h"
extern int socket_errno;
#include <socket.h>  /* for AF_INET */
#endif /* MULTINET */

#if defined(UCX) || defined(NETLIB)
#include <netdb.h>
#include <in.h>
#include <inet.h>
#ifndef NETLIB		/* UCX$INETDEF gives INET$C_TCP and SOCKOPT$M_REUSADDR */
#ifdef EMACS_MULTINET
#include <multinet_root:[multinet.include.vms]ucx$inetdef.h>
#else /* not EMACS_MULTINET */
#include <ucx$inetdef.h>
#endif /* not EMACS_MULTINET */
#endif /* not NETLIB */
#include <socket.h>
/* #include "ucxdef.h"
   struct hostent *dest_host; */
#endif /* UCX */
#ifdef NETLIB
#include "vms_netlib.h"
#endif /* NETLIB */
#endif /* HAVE_SOCKETS */

#include	"lisp.h"
#include	"buffer.h"
#include	"commands.h"
#include	"process.h"
#include	"vmsproc.h"
#include	"systty.h"
#include	"systime.h"
#include        "charset.h"
#include        "coding.h"
#include	"atimer.h"
#include	"sysselect.h"
#include	"syssignal.h"



#define max(a,b) ((a) > (b) ? (a) : (b))


/* Miscellaneous extern declarations on this page.  Ideally, these would be
   replaced by #include-ing the appropriate header; presence here indicates
   possible cleaning up required.  */

extern int max_process_desc;

extern Lisp_Object Vprocess_connection_type;

extern void remove_process (register Lisp_Object proc);
extern Lisp_Object Qprocessp;
extern Lisp_Object Fwindow_width (Lisp_Object window);
extern EMACS_TIME *input_available_clear_time;

#ifdef HAVE_VMS_PTYS
/* Allocate SIZE bytes on a page boundary.  */
extern void * valloc P_ ((size_t __size));
#endif

/* These are based on (some formerly static) vars in process.c.  */

extern Lisp_Object Qrun;
extern int process_tick;
extern SELECT_TYPE input_wait_mask;
extern SELECT_TYPE non_keyboard_wait_mask;

extern Lisp_Object call_process_kill (Lisp_Object fdpid); /* callproc.c */
extern void croak P_ ((char *));


/* Forward declarations on this page.  */

static void finish_ast (VMS_PROC_STUFF *vpr);

static char dcl[] = "*dcl*";	/* subr.el and shell.el use this */


/*
    Event flags and `select' emulation:

    Previously, Event flags were hardcoded to the following:

	0 is never used
	1 is the terminal
	23 is the timer event flag
	24-31 are reserved by VMS

    This is completely idiotic, because hardcoded event flags
    are not supported on VMS. Instead, just consider the above
    to be the index into the vector of VMS_PROC_STUFF below,
    with the following meaning:

	0 keyboard
	1 never used (is stdout on Unix)
	2 never used (is stderr on Unix)

    We'll get the real event flag from inside that structure.  */

/* Event flags are allocated by VMS.  */
static int timer_event_flag = 0;
static int synch_process_event_flag = 0;

static VMS_PROC_STUFF	proc_pool[MAX_VMS_PROC_STUFF];
static VMS_CHAN_STUFF	ch_pool[MAX_VMS_CHAN_STUFF];

#define KEYBOARD_INDEX 0

#define KEYBOARD_EVENT_FLAG		ch_pool[KEYBOARD_INDEX].efnum
#define TIMER_EVENT_FLAG		timer_event_flag
#define SYNCH_PROCESS_EVENT_FLAG	synch_process_event_flag

int
get_kbd_event_flag (void)
{
  return KEYBOARD_EVENT_FLAG;
}

int
get_timer_event_flag (void)
{
  return TIMER_EVENT_FLAG;
}


/* VMS-specific channels.  */

static VMS_CHAN_STUFF *
obtain_vms_channel (void)      /* old name: `get_vms_channel_stuff' */
{
  int i;
  VMS_CHAN_STUFF *vch;

  for (i = 0, vch = ch_pool; i < MAX_VMS_CHAN_STUFF; i++, vch++)
    {
      if (vch->state == IDLE)
	{
	  if (!(LIB$GET_EF (&vch->efnum) & 1))
	    break;
	  if (vch->efnum / 32 != KEYBOARD_EVENT_FLAG / 32)
	    {
	      LIB$FREE_EF (&vch->efnum);
	      break;
	    }
	  vch->state = WORKING;
	  vch->chan = 0;
	  vch->impl = UNKNOWN;
	  memset (&vch->u, 0, sizeof vch->u);
	  sys$clref (vch->efnum);
	  return vch;
	}
    }
  return (VMS_CHAN_STUFF *) 0;
}

static void			/* used to be not static */
release_vms_channel (vch)	/* old name: give_back_vms_channel_stuff */
     VMS_CHAN_STUFF *vch;
{
  vch->state = IDLE;
  vch->chan = 0;
  vch->impl = UNKNOWN;
  memset (&vch->u, 0, sizeof vch->u);
  sys$clref (vch->efnum);
  LIB$FREE_EF (&vch->efnum);
  vch->efnum = -1;
}

static VMS_CHAN_STUFF *		/* used to be not static */
fd_to_vms_channel (fd)		/* old name: get_vms_channel_pointer */
     register int fd;
{
  if (fd >= 0 && fd < MAX_VMS_CHAN_STUFF)
    {
      VMS_CHAN_STUFF *vch = &ch_pool[fd];

      if (vch->state != IDLE)
	return vch;
    }

  return (VMS_CHAN_STUFF *) 0;
}

static int			/* used to be not static */
vms_channel_to_fd (vch)		/* old name: get_vms_channel_handle */
     register VMS_CHAN_STUFF *vch;
{
  register int fd = vch - ch_pool;

  if (fd < 0 || fd >= MAX_VMS_CHAN_STUFF)
    {
      errno = EBADF;
      return -1;
    }
  return fd;
}


/* VMS-specific processes.  */

VMS_PROC_STUFF *
get_vms_process_stuff ()
{
  int i;
  VMS_PROC_STUFF *vpr;

  for (i = 0, vpr = proc_pool; i < MAX_VMS_PROC_STUFF; i++, vpr++)
    {
      if (vpr->process == 0 && vpr->finish_code != -1)
	return vpr;
    }
  return (VMS_PROC_STUFF *) 0;
}

void
give_back_vms_process_stuff (vpr, infd)
     VMS_PROC_STUFF *vpr;
     int *infd;
{
  /* Tell wait_reading_process_input that it needs to wake up and
     look around.  */
  if (input_available_clear_time)
    EMACS_SET_SECS_USECS (*input_available_clear_time, 0, 0);
  vpr->process = 0;
}

VMS_PROC_STUFF *
get_vms_process_pointer (p)
     register struct Lisp_Process *p;
{
  register int			i;
  register VMS_PROC_STUFF	*vpr;

  for (i = 0, vpr = proc_pool; i < MAX_VMS_PROC_STUFF; i++, vpr++)
    {
      if (vpr->process != 0 && vpr->process == p)
	return vpr;
    }
  return (VMS_PROC_STUFF *) 0;
}

void
start_vms_process_read (vpr)
     VMS_PROC_STUFF *vpr;
{
}


/* Emulation of UNIX `select' system call.

   This is a non-comprehensive emulation because (1) not all `select'
   semantics are used; (2) overcoming event flag cluster limitations
   (32 at a time) in a general way is Possibly In The Works but not at
   present worth the trouble.  This basically means we need to ensure
   all flags lie w/in a single cluster (see `init_vmsproc' below), and
   further explains why this function is defined in this file instead
   of sysdep.c along w/ the others.  */

#if 0
#include <stdio.h>		/* for fprintf, stderr */
#define SELECTDEBUG
#endif

/* VMS provides 128 (2^7) event flags, grouped into four clusters of 32
   each.  System routines that take an event flag can set an output "mask"
   which is the single cluster where that event flag resides.  */
#define MKMASK(flag) (1 << ((flag) % 32))

int
sys_select (nDesc, rdsc, wdsc, edsc, timeout)
     int nDesc;
     int *rdsc, *wdsc, *edsc;
     EMACS_TIME *timeout;
{
  int nfds = 0, private_rdsc = 0;
  unsigned long mask, readMask, allMask;
  unsigned long saved_ast_flag;

  readMask = 0;
  allMask = MKMASK (SYNCH_PROCESS_EVENT_FLAG);

#ifdef SELECTDEBUG
  fprintf (stderr, "debugging sys_select (nDesc=%d, *rdsc=%08x): BEGIN %d\n",
	   nDesc, rdsc ? *rdsc : 0, KEYBOARD_EVENT_FLAG);
#endif

  if (rdsc)
    {
      private_rdsc = *rdsc;
      *rdsc = 0;
    }

  {
    int i, j = private_rdsc;
    for (i = 0; i < MAX_VMS_CHAN_STUFF; j >>= 1, i++)
      {
	register int k = MKMASK (ch_pool[i].efnum);
	/* Pseudo file descriptor 1 and 2 are just unused placeholders.  */
	if ((ch_pool[i].state != IDLE) && (i != 1) && (i != 2))
	  allMask |= k;
	if (i < nDesc && j & 1)
	  readMask |= k;
      }
  }

#ifdef SELECTDEBUG
  fprintf (stderr, "  We expect these events      : 0x%x\n", readMask);
  fprintf (stderr, "  but we handle these as well : 0x%x\n", allMask);
#endif

  /* Block interrupts and see what is already set.  */
  saved_ast_flag = sys$setast (0);
  sys$readef (KEYBOARD_EVENT_FLAG, &mask);

#ifdef SELECTDEBUG
  fprintf (stderr, "  (saved_ast_flag=0x%x)\n", saved_ast_flag);
  fprintf (stderr, "  Initially, we get this mask : 0x%x\n", mask);
#endif

  if (mask & allMask)
    /* Something already set, keep it.  */
    {
#ifdef SELECTDEBUG
      fprintf (stderr, "  ... and we keep it\n");
#endif
    }
  else
    /* Nothing set, we must wait.  */
    {
      unsigned long secs = timeout ? EMACS_SECS (*timeout) : 0;
      unsigned long usecs = timeout ? EMACS_USECS (*timeout) : 0;

      /* Determine "how long to wait".  */
      if (secs || usecs)
	{
	  /* Apparently `sys$setimr' interprets a "negative daytime"
	     as a time delta from the current time.  The daytime type
	     is a signed quadword (64-bits), so we hardcode "long long
	     int" for now.  The units for a time-delta arg is 100
	     nanoseconds (1e-7 seconds).  */
	  long long int hltw = -10LL * ((1000LL * 1000LL * secs) + usecs);
	  unsigned long waitMask = MKMASK (SYNCH_PROCESS_EVENT_FLAG);

	  /* C RTL uses timer 1 for alarm(), so we use timer 2.
	     (Is this still valid? --ttn)  */

	  sys$cantim (2, 0);
	  sys$clref (TIMER_EVENT_FLAG);
	  sys$setimr (TIMER_EVENT_FLAG, &hltw, 0, 2);
	  waitMask |= allMask | MKMASK (TIMER_EVENT_FLAG);

#ifdef SELECTDEBUG
	  fprintf (stderr, "  Waiting %ds %dus  waitMask : 0x%x\n",
		   secs, usecs, waitMask);
#endif

	  sys$setast (1);
	  sys$wflor (KEYBOARD_EVENT_FLAG, waitMask);
	  sys$setast (0);
	  sys$cantim (2, 0);
	  sys$readef (KEYBOARD_EVENT_FLAG, &mask);

#ifdef SELECTDEBUG
	  fprintf (stderr, "  Eventually, we get this mask: 0x%x\n", mask);
	  if (mask & MKMASK (TIMER_EVENT_FLAG))
	    fprintf (stderr, "    TIMEOUT!!!\n");
#endif
	}
    }

  /* In order to correctly mimic the UNIX `select', we must detect
     ANY event, and return -1 if one occurred, and it wasn't one we
     really are waiting for.  */
  {
    int unexpected_p = (0 != (mask & allMask & ~readMask));

    if ((readMask & MKMASK (KEYBOARD_EVENT_FLAG))
	|| (unexpected_p && (allMask & MKMASK (KEYBOARD_EVENT_FLAG))))
      {
#ifdef SELECTDEBUG
	fprintf (stderr, "  clearing the keyboard event flag\n");
#endif
	sys$clref (KEYBOARD_EVENT_FLAG);
      }

    sys$setast (saved_ast_flag == SS$_WASSET);

    if (unexpected_p)
      {
	errno = EINTR;
#ifdef SELECTDEBUG
	fprintf (stderr, "  returning -1\n");
	fprintf (stderr, "debugging sys_select (): END\n");
#endif
	return -1;
      }
  }

  /* Count number of descriptors that are ready.  Some people might think
     that we need to check if the timer timed out.  There's no real need for
     that, because if that happened (and no OTHER expected event occured),
     mask will be zero, and thus, so will nfds.  */

  mask &= readMask;

  if (rdsc)			/* back to Unix format */
    {
      int i;
      *rdsc = 0;
      nfds = 0;
      for (i = 0; i < MAX_VMS_CHAN_STUFF; i++)
	if (mask & MKMASK (ch_pool[i].efnum))
	  {
	    nfds++;
	    *rdsc |= 1 << i;
	  }
#ifdef SELECTDEBUG
      fprintf (stderr, "  returning %d, with the output mask 0x%x\n",
	       nfds, *rdsc);
#endif
    }
#ifdef SELECTDEBUG
  else
    fprintf (stderr, "  returning %d\n", nfds);
#endif
    
#ifdef SELECTDEBUG
  fprintf (stderr, "debugging sys_select (): END\n");
#endif
  return nfds;
}


/* Channel implementation accessor macros.  */

#define PTY_STRUCT(vch, i)	(&((vch)->u.pty.pty_buffers[i]))
#define PTY_BUF(vch, i)		(&((vch)->u.pty.pty_buffers[i].buf[0]))
#define PTY_LEN(vch, i)		((vch)->u.pty.pty_buffers[i].len)
#define PTY_STAT(vch, i)	((vch)->u.pty.pty_buffers[i].stat)
#define PTY_LASTLEN(vch, i)	((vch)->u.pty.pty_lastlen[i])

#define MBX_BUF(vch)		((vch)->u.mbx.mbx_buffer)
#define MBX_IOSB(vch)		((vch)->u.mbx.iosb)

#define NET_BUF(vch)		((vch)->u.net.net_buffer.dsc$a_pointer)
#define NET_BUF_SIZE(vch)	((vch)->u.net.net_buffer.dsc$w_length)
#define NET_BUF_DSC(vch)	((vch)->u.net.net_buffer)
#define NET_IOSB(vch)		((vch)->u.net.iosb)

#ifdef NETLIB
#define NET_CONTEXT(vch)	((vch)->u.net.net_context)
#define NET_SOCKET(vch)		((vch)->chan)
#else
#define NET_CONTEXT(vch)	((vch)->chan)
#define NET_SOCKET(vch)		NET_CONTEXT (vch)
#endif

#ifdef NETLIB
unsigned int NETLIB_receive_ast (vch)
     VMS_CHAN_STUFF *vch;
{
  SYS$SETEF (vch->efnum);
  return SS$_NORMAL;
}
#endif

/* Start input on the pfd described by the indicated slot.  */
static void
vms_start_input (vch)
     VMS_CHAN_STUFF *vch;
{
  int status;

  {
    VMS_PROC_STUFF *vpr = 0;
    int fd = vms_channel_to_fd (vch);
    int i;
    
    for (i = 0; i < MAX_VMS_PROC_STUFF; i++)
      if (proc_pool[i].process != 0
	  && XINT (proc_pool[i].process->infd) == fd)
	{
	  vpr = &proc_pool[i];
	  break;
	}
	 
    if (vpr == 0 || vpr->process)
      sys$clref (vch->efnum);
  }
   
  if (vch->impl == PTY)
    {
#ifdef HAVE_VMS_PTYS
      status = ptd$read (vch->efnum, vch->chan, 0, vch,
			 PTY_STRUCT (vch, PTY_READBUF), PTYBUF_SIZE);
#endif
    }
  else if (vch->impl == NET)
    {
#ifdef HAVE_SOCKETS
#ifdef MULTINET
      status = SYS$QIO (vch->efnum, NET_CONTEXT (vch), IO$_READVBLK,
			&NET_IOSB (vch), 0, vch, NET_BUF (vch), NETBUFSIZ,
			0, 0, 0, 0);
#endif
#ifdef UCX
      status = SYS$QIO (vch->efnum, NET_CONTEXT (vch), IO$_READVBLK,
			&NET_IOSB (vch), 0, vch, NET_BUF (vch), NETBUFSIZ,
			0, 0, 0, 0);
#endif
#ifdef NETLIB
      status = tcp_receive (&NET_CONTEXT (vch), &NET_BUF_DSC (vch),
			    &NET_IOSB (vch), NETLIB_receive_ast, vch, 0);
#endif
#endif
    }
  else
    {
      status = SYS$QIO (vch->efnum, vch->chan, IO$_READVBLK,
			&MBX_IOSB (vch), 0, vch, MBX_BUF (vch), MSGSIZE,
			0, 0, 0, 0);
    }
  if (! (status & 1))
    LIB$SIGNAL (status);
}

/* functions for reading and writing pfds */

int
vms_read_fd (fd, buf, len, translate)
     int fd, len, translate;
     char *buf;
{
  VMS_CHAN_STUFF *vch = fd_to_vms_channel (fd);
  char *chars;
  int nchars;
  unsigned long mask;

  if (!vch || vch->state == IDLE)
    {
      errno = EBADF;
      return -1;
    }

  /* Return now if the channel is draining.  */
  if (vch->state == DRAINING)
    return 0;

  /* Return now if there's nothing to read.  */
  while (sys$readef (KEYBOARD_EVENT_FLAG, &mask),
	 !(mask & (MKMASK (vch->efnum)
		   | MKMASK (SYNCH_PROCESS_EVENT_FLAG))))
    {
      int Atemp = MKMASK (vch->efnum);
      EMACS_TIME timeout;
      EMACS_SET_SECS_USECS (timeout, 100000, 0);
      if (sys_select (MAXDESC, &Atemp, 0, 0, &timeout) < 0)
	return 0;
    }

  if (mask & MKMASK (SYNCH_PROCESS_EVENT_FLAG))
    return 0;

  /* Reading from net streams...  */
  if (vch->impl == NET)
    {
      chars = NET_BUF (vch);
      nchars = NET_IOSB (vch).size;
      if (!(NET_IOSB (vch).status & 1))
	{
	  /* If the connection has gone away, don't consider that an error.
	     Instead, return 0 to mean EOF.  --ttn  */
	  if (NET_IOSB (vch).size == EPIPE)
	    return 0;
	  errno = NET_IOSB (vch).size;
	  vaxc$errno = NET_IOSB (vch).status;
	  return -1;
	}
      NET_IOSB (vch).size = 0;
      /* If nchars == 0 the connection has gone away?  Try returning 0 here so
	 `waiting_for_process_input' will terminate the stream.  */
      if (nchars == 0)
	return 0;
    }

  /* Reading from ptys...  */
  else if (vch->impl == PTY)
    {
      char *p;

      chars = PTY_BUF (vch, PTY_READBUF);
      nchars = PTY_LEN (vch, PTY_READBUF);
      PTY_LEN (vch, PTY_READBUF) = 0;

      /* Remove carriage returns and NULs if translation is on.  */
      if (translate)
	for (p = chars; p < chars + nchars; p++)
	  if (*p == '\r' || *p == '\0')
	    {
	      --nchars;
	      memcpy (p, p+1, nchars - (p-chars));
	      --p;
	    }
    }

  /* Reading from mbxs...  */
  else
    {
      chars = MBX_BUF (vch);
      nchars = MBX_IOSB (vch).size;
      MBX_IOSB (vch).size = 0;

      /* Hack around VMS oddity of sending extraneous CR/LF characters for
	 some of the commands (but not most).  */
      if (translate)
	{
	  if (nchars > 0 && *chars == '\r')
	    {
	      chars++;
	      nchars--;
	    }
	  if (nchars > 0 && chars[nchars - 1] == '\n')
	    nchars--;
	  if (nchars > 0 && chars[nchars - 1] == '\r')
	    nchars--;
      
	  /* Add a newline onto the end.  */
	  chars[nchars++] = '\n';
	}
    }

  /* Copy the data to the output buffer.  */
  if (nchars > len) nchars = len;
  memcpy (buf, chars, nchars);

  /* Queue another read to the channel.  */
  vms_start_input (vch);

  /* We can't just return 0; if we do, `wait_reading_process_input' will
     think that the process has died.  so, do the following to fake it out.  */
  if (nchars == 0)
    {
      nchars = -1;
      errno = EWOULDBLOCK;
    }

  return nchars;
}

#ifdef HAVE_VMS_PTYS

static int
vms_write_pty (vch, buf, len, translate)
     VMS_CHAN_STUFF *vch;
     char *buf;
     int len, translate;
{
  int i, status;

  /* We can't write more than PTYBUF_SIZE characters at once.  */
  if (len > PTYBUF_SIZE)
    len = PTYBUF_SIZE;

  /* Find a free buffer.  */
  for (i = 0; i < PTY_BUFFER_COUNT; i++)
    if (i != PTY_READBUF && PTY_STAT (vch, i) != 0)
      break;

  /* If we couldn't find one, return an error status with EWOULDBLOCK.  */
  if (i >= PTY_BUFFER_COUNT)
    {
      errno = EWOULDBLOCK;
      return -1;
    }

  /* If the previous write resulted in a data overrun error, requeue that
     write, and return an EWOULDBLOCK error.  */
  if (PTY_STAT (vch, i) == SS$_DATAOVERUN)
    {
      int j;

      /* The number of characters that the last request tried to write
	 is in `PTY_LASTLEN (vch, i)'.  The number of characters that were
	 actually written is in `PTY_LEN (vch, i)'.  */

      len = PTY_LASTLEN (vch, i) - PTY_LEN (vch, i);
      for (j = 0; j < len; j++)
	PTY_BUF (vch, i)[j] = PTY_BUF (vch, i)[j + PTY_LEN (vch, i)];
      PTY_LASTLEN (vch, i) = len;
      PTY_STAT (vch, i) = SS$_NORMAL;
      if (len)
	status = ptd$write (vch->chan, 0, 0, PTY_STRUCT (vch, i), len, 0, 0);
      else
	status = SS$_NORMAL;
      if (! (status & 1))
	{
	  errno = EVMSERR;
	  vaxc$errno = status;
	  return -1;
	}
      errno = EWOULDBLOCK;
      return -1;
    }

  /* Copy the data to the pty buffer.  */
  memcpy (PTY_BUF (vch, i), buf, len);

  if (translate)
    {
      /* If the buffer consists of the single character ^D, change it to ^Z.
	 Also, translate newlines to carriage-returns.  */
      if (len == 1 && PTY_BUF (vch, i)[0] == '\004')
	PTY_BUF (vch, i)[0] = '\032';
      else
	{
	  char *p;
	  for (p = PTY_BUF (vch, i); p < PTY_BUF (vch, i) + len; p++)
	    if (*p == '\n')
	      *p = '\r';
	}
    }


  /* Queue the write.  */
  PTY_STAT (vch, i) = SS$_NORMAL;
  PTY_LASTLEN (vch, i) = len;
  status = (len
	    ? ptd$write (vch->chan, 0, 0, PTY_STRUCT (vch, i), len, 0, 0)
	    : SS$_NORMAL);
  if (! (status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }

  return len;
}

#endif

static int
vms_write_mbx (vch, buf, len, translate)
     VMS_CHAN_STUFF *vch;
     char *buf;
     int len, translate;
{
  int status, oldrwm;
  int xlen = len;

  /* Turn off resource-wait mode to prevent blocking on a full mbx.  */
  oldrwm = sys$setrwm (1);

  /* As a special hack, if the buffer consists of the single character ^D,
     write EOF to the mailbox.  */

  if (len == 1 && buf[0] == '\004' && translate)
    status = SYS$QIOW (0, vch->chan, IO$_WRITEOF | IO$M_NOW,
		       0, 0, 0, buf, xlen, 0, 0, 0, 0);
  else
    {
      /* Strip trailing newlines if translation is on.  */
      if (xlen > 0 && buf[xlen-1] == '\n' && translate)
	--xlen;
      status = SYS$QIOW (0, vch->chan, IO$_WRITEVBLK | IO$M_NOW,
			 0, 0, 0, buf, xlen, 0, 0, 0, 0);
    }

  /* Restore the previous state of resource-waiting.  */
  if (oldrwm == SS$_WASCLR)
    sys$setrwm (0);

  if (! (status & 1))
    {
      if (status == SS$_MBFULL)
	errno = EWOULDBLOCK;
      else
	{
	  errno = EVMSERR;
	  vaxc$errno = status;
	}
      
      return -1;
    }

  return len;
}

#ifdef HAVE_SOCKETS

static int
vms_write_net (vch, buf, len)
     VMS_CHAN_STUFF *vch;
     char *buf;
     int len;
{
  int status;
  int dum_0 = 0, dum_1 = 1;
  short iosb[4];

  /* Do the write.  */
#ifdef UCX
  status = SYS$QIOW (0, NET_CONTEXT (vch), IO$_WRITEVBLK, iosb, 0, 0, buf, len,
		     0, 0, 0, 0);
#endif
#ifdef NETLIB
  {
    struct dsc$descriptor tmpstr;

    /* Descriptor lengths are 16-bit.  Report a partial write rather than
       silently wrapping the byte count.  */
    if (len > USHRT_MAX)
      len = USHRT_MAX;
    tmpstr.dsc$b_dtype = DSC$K_DTYPE_T;
    tmpstr.dsc$b_class = DSC$K_CLASS_S;
    tmpstr.dsc$a_pointer = buf;
    tmpstr.dsc$w_length = len;

    status = tcp_send (&NET_CONTEXT (vch), &tmpstr, 2, iosb, 0, 0);
  }
#endif
#if defined(UCX) || defined(NETLIB)
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
  if (!(iosb[0] & 1))
    {
      errno = iosb[1];
      vaxc$errno = iosb[0];
      return -1;
    }
  /* We shall return how many bytes were actually returned.  */
  status = iosb[1];
#endif

#ifdef MULTINET
  {
    /* Turn on nonblocking mode.  */
    if (socket_ioctl (NET_CONTEXT (vch), FIONBIO, &dum_1) != 0)
      {
	errno = socket_errno;
	return -1;
      }

    /* Do the write.  */
    status = socket_write (NET_CONTEXT (vch), buf, len);
    if (status == -1) errno = socket_errno;

    /* Change back to blocking mode so reads will work properly.  */
    if (socket_ioctl (NET_CONTEXT (vch), FIONBIO, &dum_0) != 0)
      {
	errno = socket_errno;
	return -1;
      }
  }
#endif /* MULTINET */

  return status;
}

#endif

int
vms_write_fd (fd, buf, len, translate)
     int fd, len, translate;
     char *buf;
{
  VMS_CHAN_STUFF *vch = fd_to_vms_channel (fd);

  if (!vch || vch->state == IDLE)
    {
      errno = EBADF;
      return -1;
    }

#ifdef HAVE_VMS_PTYS
  if (vch->impl == PTY)
    return vms_write_pty (vch, buf, len, translate);
#endif

#ifdef HAVE_SOCKETS
  if (vch->impl == NET)
    return vms_write_net (vch, buf, len);
#endif

  return vms_write_mbx (vch, buf, len, translate);
}

/* Close a pfd and free its buffers.  */

#ifdef HAVE_SOCKETS
#ifdef NETLIB
static int socket_close (int net_chan);
static void *netlib_context_for_handle (int net_chan);
static void netlib_update_context_for_handle (int net_chan, void *context);
#define SOCKET_CLOSE(n)  socket_close (n)
#elif defined(UCX)
static int socket_close (int net_chan);
#define SOCKET_CLOSE(n)  socket_close (n)
#elif defined(MULTINET)
#define SOCKET_CLOSE(n)  socket_close (n)
#else
#define SOCKET_CLOSE(n)  close (n)
#endif
#endif /* HAVE_SOCKETS */

int
vms_close_fd (fd)
     int fd;
{
  VMS_CHAN_STUFF *vch = fd_to_vms_channel (fd);

  if (!vch || vch->state == IDLE)
    {
      errno = EBADF;
      return -1;
    }

#ifdef HAVE_VMS_PTYS
  if (vch->impl == PTY)
    {
      ptd$delete (vch->chan);
      free (vch->u.pty.pty_buffers);
    }
  else
#endif
#ifdef HAVE_SOCKETS
  if (vch->impl == NET)
    {
#ifdef NETLIB
      /* NETLIB may replace its opaque context during an I/O operation.  Keep
         the socket table in step before socket_close disconnects it.  */
      netlib_update_context_for_handle (NET_SOCKET (vch), NET_CONTEXT (vch));
#endif
      SOCKET_CLOSE (NET_SOCKET (vch));
      if (NET_BUF (vch))
	{
	  free (NET_BUF (vch));
	  NET_BUF (vch) = 0;
	}
    }
  else
#endif
    {
      SYS$DASSGN (vch->chan);
      if (MBX_BUF (vch))
	{
	  free (MBX_BUF (vch));
	  MBX_BUF (vch) = 0;
	}
    }
  release_vms_channel (vch);
  FD_CLR (fd, &input_wait_mask);

  return 0;
}

/* functions for creating pfds */

/* Create a temporary mailbox and return the channel in CHAN.  'buffer_factor'
   is used to allow sending messages asynchronously till some point.  */

static int
create_mbx (chan, buffer_factor)
     int *chan;
     int buffer_factor;
{
  int status;

  status = sys$crembx (0, chan, MSGSIZE, MSGSIZE * buffer_factor, 0, 0, 0);
  if (! (status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return 0;
    }
  return 1;
}                             /* create_mbx */

static void
abandon_mbx_channel (vch)
     VMS_CHAN_STUFF *vch;
{
  if (!vch)
    return;
  if (vch->chan)
    SYS$DASSGN (vch->chan);
  if (MBX_BUF (vch))
    free (MBX_BUF (vch));
  release_vms_channel (vch);
}

static void
vms_get_device_name (fd, dsc)
     int fd;
     struct dsc$descriptor_s *dsc;
#define FUNC_NAME "vms_get_device_name"
{
  int status;
  short retlen;
  VMS_CHAN_STUFF *vch = fd_to_vms_channel (fd);
  int dum_DVI$_DEVNAM = DVI$_DEVNAM;

  if (vch == 0)
    abort ();

  if (vch->state == IDLE)
    abort ();

  status = lib$getdvi (&dum_DVI$_DEVNAM, &vch->chan, 0, 0, dsc,
		       &retlen);
  if (! (status & 1))
    LIB$SIGNAL (status);

  dsc->dsc$w_length = retlen;
}
#undef FUNC_NAME

static int
vms_pipe (fds)
     int fds[2];
{
  VMS_CHAN_STUFF *vch[2];

  /* Allocate VMS_CHAN_STUFF for two free pseudo-fds; store their indices in
     fds.  If it wasn't possible to allocate them, return an error status.  */
  if ((vch[0] = obtain_vms_channel ()) == 0)
    {
      errno = ENFILE;
      return -1;
    }
  fds[0] = vms_channel_to_fd (vch[0]);

  if ((vch[1] = obtain_vms_channel ()) == 0)
    {
      release_vms_channel (vch[0]);
      errno = ENFILE;
      return -1;
    }
  fds[1] = vms_channel_to_fd (vch[1]);

  errno = EACCES;

  /* Create the input mailbox.  */
  sys$clref (vch[1]->efnum);
  if (! create_mbx (&vch[1]->chan, 2))
    {
      abandon_mbx_channel (vch[1]);
      release_vms_channel (vch[0]);
      return -1;
    }

  /* Create the output mailbox.  */
  MBX_BUF (vch[0]) = (char *) malloc (MSGSIZE + 1);
  if (!MBX_BUF (vch[0]))
    {
      errno = ENOMEM;
      abandon_mbx_channel (vch[1]);
      release_vms_channel (vch[0]);
      return -1;
    }
  sys$clref (vch[0]->efnum);
  if (! create_mbx (&vch[0]->chan, 1))
    {
      abandon_mbx_channel (vch[1]);
      abandon_mbx_channel (vch[0]);
      return -1;
    }

  FD_SET (fds[0], &input_wait_mask);
  vms_start_input (vch[0]);

  errno = 0;			/* done! */
  return 0;
}

#ifdef HAVE_VMS_PTYS

int
vms_make_pty (fds)
     int fds[2];
{
  int i, status;
  VMS_CHAN_STUFF *vch;
  struct ptybuf *addarr[2];
  struct			/* sizeof must be 12, 16 or 20 bytes */
  {
    char class;
    char type;
    unsigned short scr_wid;
    unsigned long tt_char : 24, scr_len : 8;
    unsigned long tt2_char;
  } term_mode;

  /* allocate VMS_CHAN_STUFF for a free pseudo-fds;
     store its index in fds. If it wasn't possible to allocate
     them, return an error status.  */
  if ((vch = obtain_vms_channel ()) == 0)
    {
      errno = ENFILE;
      return -1;
    }
  fds[0] = fds[1] = vms_channel_to_fd (vch);

  vch->u.pty.pty_buffers = valloc (PTY_BUFFER_COUNT * PAGESIZE);
  if (vch->u.pty.pty_buffers == 0)
    {
      errno = ENOMEM;
      release_vms_channel (vch);
      return -1;
    }

  /* Mark buffers as not busy.  */
  for (i = 0; i < PTY_BUFFER_COUNT; i++)
    PTY_STAT (vch, i) = 1;

#if 0
  /* Get the current terminal characteristics.  */
  SYS$QIOW (0, input_chan, IO$_SENSEMODE, 0, 0, 0,
	    &term_mode, sizeof (term_mode), 0, 0, 0, 0);

  /* Use those characteristics for the new pty, with the exception
     of pasthru.  */
  term_mode.tt2_char &= ~TT2$M_PASTHRU;
#endif
  term_mode.class = DC$_TERM;
  term_mode.type = TT$_UNKNOWN;
  /* A batch-mode frame can report the minimal width of 10 columns.  DCL
     then truncates ordinary command output to that artificial PTY width.  */
  term_mode.scr_wid = max (80, XINT (Fwindow_width (Qnil)));
  term_mode.scr_len = 255;
  term_mode.tt_char = TT$M_ESCAPE | TT$M_LOWER | TT$M_MECHFORM | TT$M_NOECHO |
    TT$M_EIGHTBIT;
  /*
   * RBR - I've had problems with ALTYPEAHD, even when $getsyi reports its
   * size as 2048.
   */
  /* term_mode.tt2_char = TT2$M_ALTYPEAHD; */
  term_mode.tt2_char = 0;

  /* Create the pty.  */
  addarr[0] = vch->u.pty.pty_buffers;
  addarr[1] = addarr[0] + PTY_BUFFER_COUNT;
  addarr[1] = (char *) addarr[1] - 1;
  status = ptd$create (&vch->chan,
		       0,		/* access mode */
		       &term_mode,
		       sizeof (term_mode),
		       0, 0, 0,		/* last-ch-deass-ast, parm, accmode */
		       addarr);
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      free (vch->u.pty.pty_buffers);
      vch->u.pty.pty_buffers = 0;
      release_vms_channel (vch);
      return -1;
    }

  /* Finish initializing and start the input.  */
  vch->state = WORKING;
  vch->impl = PTY;
  sys$clref (vch->efnum);
  vms_start_input (vch);

  return 0;
}

#endif /* HAVE_VMS_PTYS */

#ifdef HAVE_SOCKETS

int
vms_net_chan (vms_chan, fds)
     int vms_chan;
     int fds[2];
{
  VMS_CHAN_STUFF *vch;

  /* Allocate VMS_CHAN_STUFF for a free pseudo-fds; store its index in fds.
     If it wasn't possible to allocate them, return an error status.  */
  if ((vch = obtain_vms_channel ()) == 0)
    {
      errno = ENFILE;
      SOCKET_CLOSE (vms_chan);
      return -1;
    }
  fds[0] = fds[1] = vms_channel_to_fd (vch);

  vch->state = WORKING;
  vch->impl = NET;
  vch->chan = vms_chan;
#ifdef NETLIB
  NET_CONTEXT (vch) = netlib_context_for_handle (vms_chan);
  if (!NET_CONTEXT (vch))
    {
      SOCKET_CLOSE (vms_chan);
      release_vms_channel (vch);
      return -1;
    }
#endif

  NET_BUF (vch) = (char *) malloc (NETBUFSIZ + 1);
  if (!NET_BUF (vch))
    {
      errno = ENOMEM;
      SOCKET_CLOSE (vms_chan);
      release_vms_channel (vch);
      return -1;
    }
  NET_BUF_SIZE (vch) = NETBUFSIZ;
  NET_BUF_DSC (vch).dsc$b_dtype = DSC$K_DTYPE_T;
  NET_BUF_DSC (vch).dsc$b_class = DSC$K_CLASS_S;
  sys$clref (vch->efnum);

  vms_start_input (vch);

  return 0;			/* done! */
}

#if defined(UCX) || defined(NETLIB)
/* We need socket routines that handle VMS I/O channels directly.
   Unfortunatelly, the VAX C socket library routines return
   handles to its internal file structure array, which is not
   really the same...  */
/* Most of the following is picked from the Example A-4 in the
   DEC TCP/IP Services for VMS Programming Manual.  */

struct itlst {
  int lgth;
  struct sockaddr_in *hst;
};

struct itlst_1 {
  int lgth;
  char *rmt_adrs;
  int *retlth;
};

struct itlst_3 {
  int lgth;
  struct sockaddr_in *hst;
  int *retlth;
};

struct socket_structure {
#ifdef NETLIB
  void *net_chan;
  int protocol;
#else
  int net_chan;
#endif
  int inet_family;
  char inuse:1;
  char connected:1;
} socket_structure[MAXDESC];

static struct sockaddr_in prototype_sockaddr;
#ifdef NETLIB
static void *
netlib_context_for_handle (net_chan)
     int net_chan;
{
  int index = net_chan - 1;

  if (index < 0 || index >= MAXDESC || !socket_structure[index].inuse)
    {
      errno = EBADF;
      return 0;
    }
  return socket_structure[index].net_chan;
}

static void
netlib_update_context_for_handle (net_chan, context)
     int net_chan;
     void *context;
{
  int index = net_chan - 1;

  if (index >= 0 && index < MAXDESC && socket_structure[index].inuse)
    socket_structure[index].net_chan = context;
}
#endif
#endif /* UCX || NETLIB */

#ifdef MULTINET
socket (af, type, protocol)
    int af, type, protocol;
{
  int status, i;
  long net_chan;
  short iosb[4];
  
  $DESCRIPTOR (multinet_template, "INET0:");

  status = SYS$ASSIGN (&multinet_template, &net_chan, 0, 0);
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
  
  
  status = SYS$QIOW (0, net_chan, IO$_SOCKET, iosb, 0, 0,
		     AF_INET, type,0,0,0,0);
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      SYS$DASSGN (net_chan);
      return -1;
    }
  if (!(iosb[0] & 1))
    {
      errno = iosb[1];
      if (errno == 0)
	errno = EVMSERR;
      vaxc$errno = iosb[0];
      SYS$DASSGN (net_chan);
      return -1;
    }
  return net_chan;
}

connect (net_chan, name, namelen)
    int net_chan, namelen;
    struct sockaddr *name;
{
  int status, i;
  short iosb[4];
  
  struct sockaddr_in *name_in = (struct sockaddr_in *)name;

  status = SYS$QIOW (0, net_chan, IO$_CONNECT, iosb, 0, 0,
		     name_in, namelen, 0, 0, 0, 0);
  
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
  if (!(iosb[0] & 1))
    {
      errno = iosb[1];
      if (errno == 0)
	errno = EVMSERR;
      vaxc$errno = iosb[0];
      return -1;
    }
  return 0;
}
#endif

#ifdef UCX

#ifndef UCX$C_DSC_ALL
#define UCX$C_DSC_ALL 2
#endif

socket (af, type, protocol)
    int af, type, protocol;
{
  int status, i;
  long net_chan;
  short sck_parm[2];
  short iosb[4];
  struct sockaddr_in local_host = prototype_sockaddr;
  struct itlst lhst_adrs;
  struct itlst_1 lsck_adrs;
  int l_retlen;
  char local_hostaddr[16];
  $DESCRIPTOR (ucx_template, "BG:");

  /* Initialize the parameters */
  sck_parm[0] = INET$C_TCP;
  sck_parm[1] = type;

  /* Itlst for local IP address */
  lhst_adrs.lgth= sizeof (local_host);
  lhst_adrs.hst=  &local_host;
  lsck_adrs.lgth=     16;
  lsck_adrs.rmt_adrs= &local_hostaddr;
  lsck_adrs.retlth=   &l_retlen;

  local_host.sin_family = af;
  local_host.sin_port = 0;
  local_host.sin_addr.s_addr = 0;

  for (i = 0; i < MAXDESC; i++)
    if (!socket_structure[i].inuse)
      break;
  if (i == MAXDESC)
    {
      errno = ENFILE;
      return -1;
    }

  status = SYS$ASSIGN (&ucx_template, &net_chan, 0, 0);
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
  socket_structure[i].inet_family = af;
  socket_structure[i].net_chan = net_chan;
  socket_structure[i].connected = 0;
  status = SYS$QIOW (0, net_chan, IO$_SETMODE, iosb, 0, 0,
		     &sck_parm, 0x01000000|SOCKOPT$M_REUSEADDR,
		     &lhst_adrs, 0, 0, 0);
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      SYS$DASSGN (net_chan);
      socket_structure[i].inuse = 0;
      return -1;
    }
  if (!(iosb[0] & 1))
    {
      errno = iosb[1];
      if (errno == 0)
	errno = EVMSERR;
      vaxc$errno = iosb[0];
      SYS$DASSGN (net_chan);
      socket_structure[i].inuse = 0;
      return -1;
    }
  socket_structure[i].inuse = 1;
  return net_chan;
}

sys_connect (net_chan, name, namelen)
    int net_chan, namelen;
    struct sockaddr *name;
{
  int status, i;
  short iosb[4];
  struct sockaddr_in remote_host = prototype_sockaddr;
  struct sockaddr_in *name_in = (struct sockaddr_in *)name;
  struct itlst rhst_adrs;
  struct itlst_1 rsck_adrs;
  int r_retlen;
  char remote_hostaddr[16];

  rhst_adrs.lgth= sizeof (*name_in);
  rhst_adrs.hst=  name_in;
  rsck_adrs.lgth=     16;
  rsck_adrs.rmt_adrs= &remote_hostaddr;
  rsck_adrs.retlth=   &r_retlen;

  for (i = 0; i < MAXDESC; i++)
    if (socket_structure[i].inuse
	&& socket_structure[i].net_chan == net_chan)
      {
        if (socket_structure[i].connected)
          {
            errno = EISCONN;
	    return -1;
	  }
	break;
      }

  if (i == MAXDESC)
    {
      errno = EBADF;
      return -1;
    }

  status = SYS$QIOW (0, net_chan, IO$_ACCESS, iosb, 0, 0,
		     0, 0, &rhst_adrs, 0, 0, 0);
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
  if (!(iosb[0] & 1))
    {
      errno = iosb[1];
      if (errno == 0)
	errno = EVMSERR;
      vaxc$errno = iosb[0];
      return -1;
    }
  socket_structure[i].connected = 1;
  return 0;
}

static int
socket_close (net_chan)
    int net_chan;
{
  int i;

  for (i = 0; i < MAXDESC; i++)
    if (socket_structure[i].inuse &&
        socket_structure[i].net_chan == net_chan)
      break;
  if (i == MAXDESC)
    {
      errno = EBADF;
      return -1;
    }
	
  SYS$QIOW (0, net_chan, IO$_DEACCESS|IO$M_SHUTDOWN, 0, 0, 0,
	    0, 0, 0, UCX$C_DSC_ALL, 0, 0);
  SYS$QIOW (0, net_chan, IO$_DEACCESS, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  SYS$DASSGN (net_chan);
  socket_structure[i].inuse = 0;
  socket_structure[i].connected = 0;
  return 0;
}
#endif /* UCX */

#ifdef NETLIB
socket (af, type, protocol)
     int af, type, protocol;
{
  void *net_chan = 0;
  int status, i;

  for (i = 0; i < MAXDESC; i++)
    if (!socket_structure[i].inuse)
      break;
  if (i == MAXDESC)
    {
      errno = ENFILE;
      return -1;
    }

  switch (protocol)
    {
    case IPPROTO_IP:
      switch (type)
	{
	case SOCK_STREAM:
	  protocol = IPPROTO_TCP;
	  break;
	case SOCK_DGRAM:
	default:
	  protocol = IPPROTO_UDP;
	  break;
	}
      break;
    case IPPROTO_TCP:
    case IPPROTO_UDP:
      break;
    default:
      errno = EPROTONOSUPPORT;
      return -1;
    }

  status = net_assign (&net_chan);
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
  socket_structure[i].inet_family = af;
  socket_structure[i].protocol = protocol;
  socket_structure[i].net_chan = net_chan;
  socket_structure[i].connected = 0;
  socket_structure[i].inuse = 1;
  /* POSIX socket handles are ints.  Keep the pointer-sized NETLIB context
     in socket_structure and return a stable table handle instead.  */
  return i + 1;
}

sys_connect (net_chan, name, namelen)
     int net_chan, namelen;
     struct sockaddr *name;
{
  int status, i;
  void *context;
  struct sockaddr_in remote_host = prototype_sockaddr;
  struct sockaddr_in *name_in = (struct sockaddr_in *)name;

  i = net_chan - 1;
  if (i < 0 || i >= MAXDESC || !socket_structure[i].inuse)
    {
      errno = EBADF;
      return -1;
    }
  if (socket_structure[i].connected)
    {
      errno = EISCONN;
      return -1;
    }

  if (socket_structure[i].inet_family != AF_INET)
    {
      errno = EAFNOSUPPORT;
      return -1;
    }

  context = socket_structure[i].net_chan;

  switch (socket_structure[i].protocol)
    {
    case IPPROTO_TCP:
      status = net_bind (&context, 1, ntohs (name_in->sin_port), 1, 1);
      if (status & 1)
	status = tcp_connect_addr (&context, &(name_in->sin_addr.s_addr),
				   ntohs (name_in->sin_port));
      break;
    case IPPROTO_UDP:
      status = net_bind (&context, 2, ntohs (name_in->sin_port), 0, 1);
      break;
    }
  if (!(status & 1))
    {
      vaxc$errno = status;
      errno = EVMSERR;
      return -1;
    }
  socket_structure[i].net_chan = context;
  socket_structure[i].connected = 1;
  return 0;
}

static int
socket_close (net_chan)
    int net_chan;
{
  int i = net_chan - 1;
  void *context;

  if (i < 0 || i >= MAXDESC || !socket_structure[i].inuse)
    {
      errno = EBADF;
      return -1;
    }

  context = socket_structure[i].net_chan;
  if (socket_structure[i].connected)
    tcp_disconnect (&context);
  net_deassign (&context);

  socket_structure[i].net_chan = 0;
  socket_structure[i].inuse = 0;
  socket_structure[i].connected = 0;
  return 0;
}
#endif /* NETLIB */
#endif /* HAVE_SOCKETS */

static int
VMSgetwd (buf, size)
     char *buf;
     size_t size;
{
  char curdir[256];
  char *getenv ();
  char *s;
  short len;
  int status;
  size_t disk_len;
  struct
  {
    int	  l;
    char *a;
  } d;

  if (size == 0)
    {
      errno = EINVAL;
      return -1;
    }

  s = getenv ("SYS$DISK");
  disk_len = s ? strlen (s) : 0;

  d.l = 255;
  d.a = curdir;
  status = sys$setddir (0, &len, &d);
  if (!(status & 1))
    {
      errno = EVMSERR;
      vaxc$errno = status;
      return -1;
    }
  if (disk_len + len >= size)
    {
      errno = ENAMETOOLONG;
      return -1;
    }

  if (disk_len)
    memcpy (buf, s, disk_len);
  memcpy (buf + disk_len, curdir, len);
  buf[disk_len + len] = '\0';
  return 0;
}


void
wait_for_termination (pid)
     int pid;
{
  sys$waitfr (SYNCH_PROCESS_EVENT_FLAG);
  sys$clref (SYNCH_PROCESS_EVENT_FLAG); 
}

static Lisp_Object
free_vms_process_string (value)
     Lisp_Object value;
{
  void *pointer = XSAVE_VALUE (value)->pointer;
  if (pointer)
    free (pointer);
  XSAVE_VALUE (value)->pointer = 0;
  return Qnil;
}

static Lisp_Object
restore_vms_async_state (ignored)
     Lisp_Object ignored;
{
  turn_on_atimers (1);
  sys$setast (1);
  return Qnil;
}

/* Return the newly-allocated concatenation (with intervening spaces) of
   ARGV.  DCL parses a command line before an image sees it, so quote every
   foreign-command argument after argv[0] and double embedded quotes.  A
   complete command supplied to *dcl* -c is instead passed through verbatim.  */

static char *
argv_to_line (argv, quote_arguments)
     unsigned char **argv;
     int quote_arguments;
{
  int i;
  size_t total = 1;
  char *line, *out;

  for (i = 0; argv[i] != 0; i++)
    {
      const unsigned char *p;
      size_t extra = strlen (argv[i]) + (i != 0);
      if (quote_arguments && i != 0)
	{
	  extra += 2;
	  for (p = argv[i]; *p; p++)
	    if (*p == '"')
	      extra++;
	}
      if (extra > (size_t) -1 - total)
	memory_full ();
      total += extra;
    }

  if (total - 1 > USHRT_MAX)
    error ("OpenVMS command line too long");

  line = (char *) xmalloc (total);
  out = line;
  for (i = 0; argv[i] != 0; i++)
    {
      const unsigned char *p = argv[i];
      if (i != 0)
	*out++ = ' ';
      if (quote_arguments && i != 0)
	{
	  *out++ = '"';
	  while (*p)
	    {
	      if (*p == '"')
		*out++ = '"';
	      *out++ = *p++;
	    }
	  *out++ = '"';
	}
      else
	while (*p)
	  *out++ = *p++;
    }
  *out = '\0';
  return line;
}

static int
vms_image_suffix_p (name, length, suffix)
     const unsigned char *name;
     int length;
     const char *suffix;
{
  int end = length;
  int i, suffix_length = strlen (suffix);

  for (i = length; i > 0; i--)
    if (name[i - 1] == ';')
      {
	end = i - 1;
	break;
      }
  if (end < suffix_length)
    return 0;
  for (i = 0; i < suffix_length; i++)
    if (toupper ((unsigned char) name[end - suffix_length + i])
	!= toupper ((unsigned char) suffix[i]))
      return 0;
  return 1;
}

/* Return a newly allocated command fragment for PATH.  A resolved .EXE
   image (including an explicit ;version) needs MCR, while a .COM procedure
   needs @.  Otherwise PATH is presumed to be a DCL verb or symbol.  */

static char *
hack_vms_program_name (path)
     const char *path;
{
  Lisp_Object lpath = Qnil, name = Qnil, suffixes = Qnil;
  struct gcpro gcpro1, gcpro2, gcpro3;
  const char *pathrest = strchr (path, ' ');
  unsigned char *tem;
  char *result;
  size_t pathrestlen = pathrest ? strlen (pathrest) : 0;
  size_t command_length = pathrest ? pathrest - path : strlen (path);
  const char *prefix = 0;
  size_t prefix_length = 0;

  tem = alloca (command_length + 1);
  memcpy (tem, path, command_length);
  tem[command_length] = '\0';

  GCPRO3 (lpath, name, suffixes);
  name = build_string (tem);
  suffixes = Fcons (build_string (".COM"), Qnil);
  suffixes = Fcons (build_string (".EXE"), suffixes);
  openp (Vexec_path, name, suffixes, &lpath, make_number (X_OK));

  if (!NILP (lpath))
    {
      unsigned char *resolved = XSTRING (lpath)->data;
      int resolved_length = XSTRING (lpath)->size;

      if (vms_image_suffix_p (resolved, resolved_length, ".EXE"))
	{
	  prefix = "MCR ";
	  prefix_length = 4;
	}
      else if (vms_image_suffix_p (resolved, resolved_length, ".COM"))
	{
	  prefix = "@";
	  prefix_length = 1;
	}

      if (prefix)
	{
	  result = (char *) xmalloc (prefix_length + resolved_length
				     + pathrestlen + 1);
	  memcpy (result, prefix, prefix_length);
	  memcpy (result + prefix_length, resolved, resolved_length);
	  if (pathrestlen)
	    memcpy (result + prefix_length + resolved_length,
		    pathrest, pathrestlen);
	  result[prefix_length + resolved_length + pathrestlen] = '\0';
	  UNGCPRO;
	  return result;
	}
    }

  /* An explicit versioned image can be executable even when OPENP cannot
     canonicalize it (for example, when it is outside exec-path).  */
  if (vms_image_suffix_p (tem, command_length, ".EXE"))
    {
      prefix = "MCR ";
      prefix_length = 4;
    }
  else if (vms_image_suffix_p (tem, command_length, ".COM"))
    {
      prefix = "@";
      prefix_length = 1;
    }
  if (prefix)
    {
      result = (char *) xmalloc (prefix_length + strlen (path) + 1);
      memcpy (result, prefix, prefix_length);
      strcpy (result + prefix_length, path);
      UNGCPRO;
      return result;
    }

  result = (char *) xmalloc (strlen (path) + 1);
  strcpy (result, path);
  UNGCPRO;
  return result;
}

void
create_process (process, new_argv, current_dir)
     Lisp_Object process;
     char **new_argv;
     Lisp_Object current_dir;
{
  int inchannel, outchannel;
  int count = SPECPDL_INDEX ();
  int argc;
  pid_t pid;
  int fd[2];
  char old_dir[512];
  int status;
  int spawn_flags = CLI$M_NOWAIT;
  int pty_p = 0;
  char *hacked_program = 0;
  char **command_argv;
  char in_dev_name[65];
  char out_dev_name[65];
  $DESCRIPTOR (din, in_dev_name);
  $DESCRIPTOR (dout, out_dev_name);
  struct dsc$descriptor_s dcmd;
  VMS_PROC_STUFF *vpr;

  record_unwind_protect (restore_vms_async_state, Qnil);
  turn_on_atimers (0);

  /* Create the I/O channels either ptys or mailboxes.  */
  status = -1;
#ifdef HAVE_VMS_PTYS
  if (EQ (Vprocess_connection_type, Qt))
    {
      status = vms_make_pty (fd);
      if (status >= 0)
	pty_p = 1;
    }
#endif

  if (status < 0)
    {
      if (vms_pipe (fd) < 0)
	error ("Can't create mailboxes");
    }

  vpr = get_vms_process_pointer (XPROCESS (process));
  if (vpr == 0)
    {
      PROCESS_CLOSE_FD (fd[0]);
      if (fd[1] != fd[0])
	PROCESS_CLOSE_FD (fd[1]);
      error ("make_process () didn't make a process.");
    }

  vpr->process = XPROCESS (process);

  inchannel = fd[0];
  outchannel = fd[1];

  /* Record ownership before any Lisp operation can unwind.  The caller's
     start_process_unwind will then close both channels on every error.  */
  chan_process[inchannel] = process;
  XSETINT (XPROCESS (process)->infd, inchannel);
  XSETINT (XPROCESS (process)->outfd, outchannel);
  XPROCESS (process)->pty_flag = (pty_p ? Qt : Qnil);
  XPROCESS (process)->status = Qrun;
  setup_process_coding_systems (process);

  for (argc = 0; new_argv[argc]; argc++)
    ;
  command_argv = alloca ((argc + 1) * sizeof *command_argv);
  memcpy (command_argv, new_argv, (argc + 1) * sizeof *command_argv);

  dcmd.dsc$b_dtype = DSC$K_DTYPE_T;
  dcmd.dsc$b_class = DSC$K_CLASS_S;
  dcmd.dsc$a_pointer = 0;
  dcmd.dsc$w_length = 0;
  if (strcmp (*command_argv, dcl) == 0)
    {
	if (command_argv[1] != 0 && strcmp (command_argv[1], "-c") == 0
	    && command_argv[2] != 0)
	{
	  hacked_program = hack_vms_program_name (command_argv[2]);
	  record_unwind_protect (free_vms_process_string,
				 make_save_value (hacked_program, 0));
	  command_argv[2] = hacked_program;
	  dcmd.dsc$a_pointer
	    = argv_to_line ((unsigned char **) command_argv + 2, 0);
	  dcmd.dsc$w_length = strlen (dcmd.dsc$a_pointer);
	}
    }
  else
    {
      hacked_program = hack_vms_program_name (command_argv[0]);
      record_unwind_protect (free_vms_process_string,
			     make_save_value (hacked_program, 0));
      command_argv[0] = hacked_program;
      dcmd.dsc$a_pointer
	= argv_to_line ((unsigned char **) command_argv, 1);
      dcmd.dsc$w_length = strlen (dcmd.dsc$a_pointer);
    }
  if (dcmd.dsc$a_pointer)
    record_unwind_protect (free_vms_process_string,
			   make_save_value (dcmd.dsc$a_pointer, 0));

  /* Delay interrupts until we have a chance to store
     the new fork's pid in its process structure.
     (No need for VMS.)  */

  FD_SET (inchannel, &input_wait_mask);
  FD_SET (inchannel, &non_keyboard_wait_mask);
  if (inchannel > max_process_desc)
    max_process_desc = inchannel;

  /* Until we store the proper pid, enable sigchld_handler
     to recognize an unknown pid as standing for this process.
     It is very important not to let this `marker' value stay
     in the table after this function has returned; if it does
     it might cause call-process to hang and subsequent asynchronous
     processes to get their return values scrambled.  */
  XPROCESS (process)->pid = (pid_t) -1;

  /* Spawn the subprocess.  */
  vms_get_device_name (fd[0], &din);
  vms_get_device_name (fd[1], &dout);

  /* Switch current directory so that the child inherits it.
     Save our place first so we can restore afterwards.  */
  message ("Creating subprocess...");
  if (VMSgetwd (old_dir, sizeof old_dir) < 0)
    report_file_error ("Reading current directory", Qnil);
  if (chdir (XSTRING (current_dir)->data) < 0)
    report_file_error ("Setting current directory",
		       Fcons (current_dir, Qnil));

  /* Delay AST delivery until the new pid is in its process structure.  */
  sys$setast (0);
  do {
    spawn_flags ^= CLI$M_AUTHPRIV;
    vpr->finish_code = -1;
    
    /* Scott Snyder suggests I flip din and dout in this call... done.  */
    status = lib$spawn (&dcmd, &dout, &din, &spawn_flags, 0, &pid,
			&vpr->finish_code,
			0,			/* 8bit-event-flag addr */
			finish_ast, vpr);
  }
  while (status == LIB$_INVARG && (spawn_flags & CLI$M_AUTHPRIV));

  if (chdir (old_dir) < 0)
    report_file_error ("Restoring current directory", Qnil);

  if (!(status & 1))
    {
      char *msg = strerror (EVMSERR, status);
      /* No completion AST will arrive to make this pool slot reusable.  */
      vpr->finish_code = 0;
      if (msg && *msg)
	error ("Unable to spawn subprocess: %s", msg);
      else
	error ("Unable to spawn subprocess");
    }

  /* We only keep the low 24 bits of the pid, because the high 8 bits
     are hopefully the same for all processes on one machine.
     --- Richard Levitte */
  XPROCESS (process)->pid = (pid_t) (pid & 0xFFFFFF);

  /* Free command storage and restore timers/AST delivery before invoking
     Lisp-visible messaging.  */
  unbind_to (count, Qnil);
  message ("Creating subprocess...done");
}

/* This used to be called `child_sig' but we renamed it because its
   sole use is the AST to be called upon process completion (per the
   `lib$spawn' call in `create_process').  It mimics `sigchld_handler'
   for the process-exiting context, however, to maintain consistency
   of interface to the event-loop internals.  */
static void
finish_ast (vpr)
     VMS_PROC_STUFF *vpr;
{
  register struct Lisp_Process *p = vpr->process;
  if (p)
    {
      VMS_CHAN_STUFF *vch = fd_to_vms_channel (XINT (p->infd));

      /* Special-case the normal exit code.  */
      if ((vpr->finish_code & SS$_NORMAL) == SS$_NORMAL)
	vpr->finish_code = 0;
      p->raw_status = vpr->finish_code;
      p->raw_status_new = 1;
      XSETINT (p->tick, ++process_tick);

      /* Stop waiting for the process output.  Do not immediately make
	 the channel idle as that introduces a race condition in
	 `wait_reading_process_input'.  Instead, allow it to drain.  */
      vch->state = DRAINING;
      FD_CLR (XINT (p->infd), &input_wait_mask);
      FD_CLR (XINT (p->infd), &non_keyboard_wait_mask);

      /* Tell wait_reading_process_input that it needs to wake up and
	 look around.  */
      if (input_available_clear_time)
	EMACS_SET_SECS_USECS (*input_available_clear_time, 0, 0);

      /* Why is this necessary?  --ttn */
      sys$setef (vch->efnum);
    }
}


/* Synchronous subprocess (call-process).  */

static int call_process_exited;

static void
call_process_ast ()
{
  synch_process_alive = 0;
  sys$setef (SYNCH_PROCESS_EVENT_FLAG);
}

static int
call_process_check_end ()
{
  long mask;
  EMACS_TIME timeout;
  SELECT_TYPE Atemp = input_wait_mask;
  EMACS_SET_SECS_USECS (timeout, /*100000*/ 0, 0);
  sys_select (MAXDESC, &Atemp, 0, 0, &timeout); /* to avoid constant looping */
  sys$readef (KEYBOARD_EVENT_FLAG, &mask);
  return mask & MKMASK (SYNCH_PROCESS_EVENT_FLAG);
}

static Lisp_Object
call_process_cleanup (fdpid)
     Lisp_Object fdpid;
{
  register pid_t pid = ((getpid () & 0xff000000)
			| (pid_t) XFASTINT (Fcdr (fdpid)));

  if (call_process_exited)
    {
      synch_process_alive = 0;
      PROCESS_CLOSE_FD (XFASTINT (Fcar (fdpid)));
      return Qnil;
    }

  if (EMACS_KILLPG (pid, SIGINT) == 0)
    {
      int count = SPECPDL_INDEX ();
      record_unwind_protect (call_process_kill, fdpid);
      message1 ("Waiting for process to die...(type C-g again to kill it instantly)");
      immediate_quit = 1;
      QUIT;
      wait_for_termination (pid);
      immediate_quit = 0;
      specpdl_ptr = specpdl + count; /* Discard the unwind protect.  */
      message1 ("Waiting for process to die...done");
    }
  synch_process_alive = 0;
  PROCESS_CLOSE_FD (XFASTINT (Fcar (fdpid)));
  return Qnil;
}

/* Clean resources acquired before LIB$SPAWN has transferred ownership to
   the ordinary call_process_cleanup handler.  The cons is (input-fd .
   output-pseudo-fd); either slot is set back to -1 as ownership moves.  */
static Lisp_Object
call_process_setup_cleanup (state)
     Lisp_Object state;
{
  int filefd = XFASTINT (XCAR (state));
  int outputfd = XFASTINT (XCDR (state));

  if (filefd >= 0)
    close (filefd);
  if (outputfd >= 0)
    PROCESS_CLOSE_FD (outputfd);
  /* -2 means LIB$SPAWN succeeded and the ordinary synchronous cleanup now
     owns the child.  Every earlier unwind must clear the global state.  */
  if (outputfd != -2)
    synch_process_alive = 0;
  return restore_vms_async_state (Qnil);
}

DEFUN ("call-process", Fcall_process, Scall_process, 1, MANY, 0,
       doc: /* Call PROGRAM synchronously in a separate process.
The program's input comes from file INFILE (nil means `NLA0:').
Insert output in BUFFER before point; t means the current buffer;
nil means discard it; 0 means discard it and do not wait;
1 means report output lines as messages.
Fourth arg DISPLAY non-nil means redisplay BUFFER as output is inserted.
Remaining arguments are strings passed as command arguments to PROGRAM.
If BUFFER is 0, return immediately with value nil.
Otherwise wait for PROGRAM to terminate and return its OpenVMS status.
If you quit, kill the process with SIGINT, or SIGKILL if you quit again.

usage: (call-process PROGRAM &optional INFILE BUFFER DISPLAY &rest ARGS)  */)
  (nargs, args)
     int nargs;
     register Lisp_Object *args;
{
  Lisp_Object infile = Qnil, buffer = Qnil, current_dir = Qnil;
  Lisp_Object display = Qnil, setup_state = Qnil;
  struct gcpro gcpro1, gcpro2, gcpro3, gcpro4, gcpro5;
  int give_messages = 0;
  int fd[2] = {-1, -1};
  int filefd = -1;
  pid_t pid;			/* Doesn't really matter, and it takes
				   away a stupid warning below, in the
				   call to lib$spawn().  RL  */

  char buf[1024];
  char *hacked_program = 0;
  int count = SPECPDL_INDEX ();
  register unsigned char **new_argv
    = (unsigned char **) alloca ((max (2, nargs - 2)) * sizeof (char *));
  struct buffer *old = current_buffer;
  char out_dev_name[65];
  $DESCRIPTOR (dout, out_dev_name);
  CHECK_STRING (args[0]);
  GCPRO5 (infile, buffer, current_dir, display, setup_state);

#ifndef subprocesses
  /* Without asynchronous processes we cannot have BUFFER == 0.  */
  if (nargs >= 3 && XTYPE (args[2]) == Lisp_Int)
    error ("Operating system cannot handle asynchronous subprocesses");
#endif /* subprocesses */

  setup_state = Fcons (make_number (-1), make_number (-1));
  record_unwind_protect (call_process_setup_cleanup, setup_state);
  turn_on_atimers (0);
  sys$setast (0);
  
  if (nargs >= 2 && ! NILP (args[1]))
    {
      infile = Fexpand_file_name (args[1], current_buffer->directory);
      /* If the file name doesn't have an extension, add a period.  */
      {
	unsigned char *p = strrchr (XSTRING (infile)->data, ']');
	if (p == 0)
	  p = strrchr (XSTRING (infile)->data, ':');
	if (p == 0)
	  p = XSTRING (infile)->data;
	
	if (strchr (p, '.') == 0)
	  infile = concat2 (infile, build_string ("."));
      }
      CHECK_STRING (infile);
    }
  else
    infile = build_string (NULL_DEVICE);

  if (nargs >= 3)
    {
      register Lisp_Object tem;

      buffer = tem = args[2];
      if (XTYPE (tem) == Lisp_Int && XFASTINT (tem) == 1)
	{
	  buffer = Qnil;
	  give_messages = 1;
	}
      else
	if (!(EQ (tem, Qnil)
	      || EQ (tem, Qt)
	      || (INTEGERP (tem) && XFASTINT (tem) == 0)))
	  {
	    buffer = Fget_buffer (tem);
	    CHECK_BUFFER (buffer);
	  }
    }
  else 
    buffer = Qnil;

  /* Make sure that the child will be able to chdir to the current
     buffer's current directory, or its unhandled equivalent.  We
     can't just have the child check for an error when it does the
     chdir, since it's in a vfork.

     We have to GCPRO around this because Fexpand_file_name,
     Funhandled_file_name_directory, and Ffile_accessible_directory_p
     might call a file name handling function.  The argument list is
     protected by the caller, so all we really have to worry about is
     buffer.  */
  {
    struct gcpro gcpro1, gcpro2, gcpro3;
    char* tempy;
    current_dir = current_buffer->directory;
    tempy = XSTRING(current_dir)->data;
    GCPRO3 (infile, buffer, current_dir);

    current_dir = Funhandled_file_name_directory (current_dir);
    tempy = XSTRING(current_dir)->data;
    if (NILP (Ffile_accessible_directory_p
	      (expand_and_dir_to_file (current_dir, Qnil))))
      report_file_error ("Setting current directory",
			 Fcons (current_buffer->directory, Qnil));

    UNGCPRO;
  }

  display = nargs >= 4 ? args[3] : Qnil;

  filefd = open (XSTRING (infile)->data, O_RDONLY, 0);
  if (filefd < 0)
    {
      report_file_error ("Opening process input file", Fcons (infile, Qnil));
    }
  XSETCAR (setup_state, make_number (filefd));
  {
    int start = 4;
    int out = 0;
    CHECK_STRING (args[0]);
    if (strcmp (XSTRING (args[0])->data, dcl) == 0)
      {
	new_argv[out++] = XSTRING (args[0])->data;
	if (start < nargs)
	  {
	    CHECK_STRING (args[start]);
	    if (strcmp (XSTRING (args[start])->data, "-c") == 0)
	      {
		new_argv[out++] = XSTRING (args[start++])->data;
		if (start >= nargs)
		  error ("Missing command after *dcl* -c");
		CHECK_STRING (args[start]);
		hacked_program
		  = hack_vms_program_name (XSTRING (args[start++])->data);
		record_unwind_protect (free_vms_process_string,
				       make_save_value (hacked_program, 0));
		new_argv[out++] = hacked_program;
	      }
	  }
      }
    else
      {
	hacked_program = hack_vms_program_name (XSTRING (args[0])->data);
	record_unwind_protect (free_vms_process_string,
			       make_save_value (hacked_program, 0));
	new_argv[out++] = hacked_program;
      }
    for (; start < nargs; start++)
      {
	CHECK_STRING (args[start]);
	new_argv[out++] = XSTRING (args[start])->data;
      }
    new_argv[out] = 0;
  }


  if (XTYPE (buffer) == Lisp_Int)
    {
      dout.dsc$b_dtype = DSC$K_DTYPE_T;
      dout.dsc$b_class = DSC$K_CLASS_S;
      dout.dsc$a_pointer = "NLA0:";
      dout.dsc$w_length = strlen (dout.dsc$a_pointer);
      fd[0] = -1;
    }
  else
    {
      if (vms_pipe (fd) < 0)
	error ("can't create mailboxes");
      XSETCDR (setup_state, make_number (fd[0]));
      PROCESS_CLOSE_FD (fd[1]);
      fd[1] = -1;
      vms_get_device_name (fd[0], &dout);
    }

  {
    struct dsc$descriptor_s dcmd, din;
    int spawn_flags = CLI$M_NOWAIT;
    int status;
    int wait_for_child = XTYPE (buffer) != Lisp_Int;
    char oldDir[512];

    if (close (filefd) < 0)
      report_file_error ("Closing process input file", Fcons (infile, Qnil));
    filefd = -1;
    XSETCAR (setup_state, make_number (-1));

    din.dsc$b_dtype = DSC$K_DTYPE_T;
    din.dsc$b_class = DSC$K_CLASS_S;
    din.dsc$a_pointer = (char *) XSTRING (infile)->data;
    din.dsc$w_length = strlen (XSTRING (infile)->data);

    dcmd.dsc$b_dtype = DSC$K_DTYPE_T;
    dcmd.dsc$b_class = DSC$K_CLASS_S;
    dcmd.dsc$a_pointer = 0;
    dcmd.dsc$w_length = 0;
    if (strcmp (*new_argv, dcl) == 0)
      {
	if (new_argv[1] && strcmp (new_argv[1], "-c") == 0
	    && new_argv[2])
	  {
	    dcmd.dsc$a_pointer = argv_to_line (new_argv + 2, 0);
	    dcmd.dsc$w_length = strlen (dcmd.dsc$a_pointer);
	  }
      }
    else
      {
	dcmd.dsc$a_pointer = argv_to_line (new_argv, 1);
	dcmd.dsc$w_length = strlen (dcmd.dsc$a_pointer);
      }
    if (dcmd.dsc$a_pointer)
      record_unwind_protect (free_vms_process_string,
			     make_save_value (dcmd.dsc$a_pointer, 0));

    synch_process_alive = wait_for_child;
    synch_process_death = 0;
    synch_process_termsig = 0;
    synch_process_retcode = 0;
    if (wait_for_child)
      sys$clref (SYNCH_PROCESS_EVENT_FLAG);
    /* On VMS we need to change the current directory
       of the parent process before forking so that
       the child inherit that directory.  We remember
       where we were before changing.  */
    if (VMSgetwd (oldDir, sizeof oldDir) < 0)
      report_file_error ("Reading current directory", Qnil);
    if (chdir (XSTRING (current_dir)->data) < 0)
      report_file_error ("Setting current directory",
			 Fcons (current_dir, Qnil));
    
    do {
      spawn_flags = spawn_flags ^ CLI$M_AUTHPRIV;
    
      status = lib$spawn (&dcmd, &din, &dout, &spawn_flags, 0, &pid,
			  wait_for_child ? &synch_process_retcode : 0, 0,
			  wait_for_child ? call_process_ast : 0, 0);
    }
    while (status == LIB$_INVARG && (spawn_flags & CLI$M_AUTHPRIV));

    
    if (chdir (oldDir) < 0)
      report_file_error ("Restoring current directory", Qnil);
    
    if (!(status & 1))
      {
	char *msg = strerror (EVMSERR, status);
	if (msg != 0)
	  error ("Unable to spawn subprocess: %s", msg);
	else
	  error ("Unable to spawn subprocess");
      }

    /* The running process and the normal cleanup handler now own fd[0].
       Preserve synch_process_alive while setup unwind-protects are removed.  */
    XSETCDR (setup_state, make_number (-2));
    unbind_to (count, Qnil);
  }

  if (XTYPE (buffer) == Lisp_Int)
    {
      synch_process_alive = 0;
      if (fd[0] >= 0)
	PROCESS_CLOSE_FD (fd[0]);
#ifndef subprocesses
      /* If Emacs has been built with asynchronous subprocess support,
	 we don't need to do this, I think because it will then have
	 the facilities for handling SIGCHLD.  */
      wait_without_blocking ();
#endif /* subprocesses */
      UNGCPRO;
      return Qnil;
    }

  /* Enable sending signal if user quits below.  */
  call_process_exited = 0;

  record_unwind_protect (call_process_cleanup,
			 Fcons (make_number (fd[0]), make_number (pid)));

  if (BUFFERP (buffer))
    Fset_buffer (buffer);

  immediate_quit = 1;
  QUIT;

  {
    register int nread;
    int status = 0;
    int first = 1;

#define tmp_read vms_read_fd

    /* The fourth argument "1" is for VMS.  On other systems, it will just
       be ignored.  --  Richard Levitte */
    while ((nread = tmp_read (fd[0], buf, sizeof buf, 1)) >= 0
	   || errno == EWOULDBLOCK)
      {
#if 0
	printf ("nread = %d, errno = %d\n", nread, errno);
#endif
	if (nread == 0 && call_process_check_end ())
	  break;
	if (nread < 0)
	  continue;		/* EWOULDBLOCK */

	immediate_quit = 0;
#if 0
	if (nread)
	  printf ("buf = \"%*.*s\"\n", nread, nread, buf);
#endif
	if (!NILP (buffer))
	  insert (buf, nread);
	else if (give_messages && nread > 1 && !status)
	  /* Output first output line as message line if 
	     output not sent to buffer.  */
          {
            status = 0;

	    /* read () will most probably return several lines.
	       At least on VMS, the first is the most interesting, so
	       let's skip the rest...
	       This should really be fdone a better way...
	       -- Richard Levitte */
	    while (status <= nread && buf[status] > 31) status++;
            if (status)
              {
		buf[status] = 0;
		message ("%s", buf);
              }
	  }
	if (!NILP (display) && INTERACTIVE)
	  {
	    if (first)
	      prepare_menu_bars ();
	    first = 0;
	    redisplay_preserve_echo_area (19);
	  }
	immediate_quit = 1;
	QUIT;
      }
#if 0
    printf ("terminating: nread = %d, errno = %d\n", nread, errno);
#endif

#undef tmp_read
  }

  /* Wait for it to terminate, unless it already has.  */
  wait_for_termination (pid);

  PROCESS_CLOSE_FD (fd[0]);       /* need to close output mailbox */

  immediate_quit = 0;

  set_buffer_internal (old);

  /* Don't kill any children that the subprocess may have left behind
     when exiting.  */
  call_process_exited = 1;

  unbind_to (count, Qnil);

  UNGCPRO;
  if (synch_process_death)
    return build_string (synch_process_death);
  return make_number (synch_process_retcode);
}


void
init_vmsproc (void)
{
  int i;
  unsigned int status;
  VMS_CHAN_STUFF *vch;
  VMS_PROC_STUFF *vpr;
  int last_event_flag = 0;

  for (vch = &ch_pool[0], i = 0; i < MAX_VMS_CHAN_STUFF; vch++, i++)
    {
      vch->state = IDLE;
      vch->efnum = -1;
      vch->chan = 0;
    }

  ch_pool[1].state = WORKING;	/* stdout */
  ch_pool[2].state = WORKING;	/* stderr */

  status = LIB$GET_EF (&SYNCH_PROCESS_EVENT_FLAG);
  if (!(status & 1))
    abort ();
  sys$clref (SYNCH_PROCESS_EVENT_FLAG);

  status = LIB$GET_EF (&TIMER_EVENT_FLAG);
  if (!(status & 1))
    abort ();
  if (SYNCH_PROCESS_EVENT_FLAG / 32 != TIMER_EVENT_FLAG / 32)
    croak ("Synch process and timer event flags in different clusters.");
  sys$clref (TIMER_EVENT_FLAG);

  status = LIB$GET_EF (&KEYBOARD_EVENT_FLAG);
  if (!(status & 1))
    abort ();
  if (TIMER_EVENT_FLAG / 32 != KEYBOARD_EVENT_FLAG / 32)
    croak ("Timer and keyboard event flags in different clusters.");
  sys$clref (KEYBOARD_EVENT_FLAG);
  ch_pool[KEYBOARD_INDEX].state = WORKING;	/* stdin */

  for (vpr = proc_pool, i = 0; i < MAX_VMS_PROC_STUFF; i++, vpr++)
    {
      vpr->process = 0;
      vpr->finish_code = 0;
    }

}

void
syms_of_vmsproc (void)
{
  defsubr (&Scall_process);
}

/* vmsproc.c ends here */
