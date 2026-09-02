/* vmsproc.h */

/* Copyright (C) 1988, 1994, 2003 Free Software Foundation, Inc.

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

/* Written by Richard Levitte. (???)
   Adapted for OpenVMS 7.3-1 by Thien-Thi Nguyen <ttn@gnu.org>.  */


struct mbx_iosb
{
  unsigned short status;
  unsigned short size;
  unsigned int   pid;
};


/* Define the structure of the buffers used for communicating with the
   pty driver.  Each buffer is exactly one page long.  */

#define PTYBUF_SIZE (PAGESIZE - 2 * sizeof (short))
#define PTY_BUFFER_COUNT 2
#define PTY_READBUF 0

struct ptybuf
{
  short stat;
  short len;
  char buf[PTYBUF_SIZE];
};

typedef struct
{
  enum { IDLE, WORKING, DRAINING }	state;
  int					efnum;
  unsigned short			chan;
  enum { UNKNOWN, PTY, NET, KBD }	impl;
  union
  {
    struct
    {
      int pty_lastlen[PTY_BUFFER_COUNT];
      struct ptybuf *pty_buffers;
    } pty;

    struct
    {
      char *mbx_buffer;
      struct mbx_iosb iosb;
    } mbx;

    struct
    {
      struct dsc$descriptor net_buffer;
#ifdef NETLIB
      void *net_context;
#endif
      /* char *net_buffer; */
      struct mbx_iosb iosb;
    } net;
  } u;
} VMS_CHAN_STUFF;

#define	MAX_EVENT_FLAGS		23
#define	MAX_VMS_CHAN_STUFF	32

typedef struct
{
  struct Lisp_Process  *process;	/* 0 means not active */
  int			finish_code;
} VMS_PROC_STUFF;

#define	MAX_VMS_PROC_STUFF 32

#define MSGSIZE 1024            /* Maximum size for mailbox operations */
#define NETBUFSIZ 1024		/* Maximum size for network operations */

extern VMS_PROC_STUFF *get_vms_process_stuff P_ ((void));
extern VMS_PROC_STUFF *get_vms_process_pointer P_ ((struct Lisp_Process *));
extern void give_back_vms_process_stuff P_ ((VMS_PROC_STUFF *, int *));
extern int vms_write_fd P_ ((int, char *, int, int));
extern void start_vms_process_read P_ ((VMS_PROC_STUFF *));
extern int vms_read_fd P_ ((int, char *, int, int));
#ifdef HAVE_SOCKETS
extern int vms_net_chan P_ ((int, int *));
#endif

/* vmsproc.h ends here */
