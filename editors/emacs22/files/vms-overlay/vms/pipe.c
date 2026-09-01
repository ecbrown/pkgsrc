/*
 * Copyright (C) 1994 the Free Software Foundation, Inc.
 *
 * Author: Roland B. Roberts (roberts@nsrl.rochester.edu)
 *
 * This file is a part of GNU VMSLIB, the GNU library for porting GNU
 * software to VMS.
 *
 * GNU VMSLIB is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNU VMSLIB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU VMSLIB; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Modification History
 * 13 Sep 94 - RBR
 *    Use event flag one -- zero seems to cause sys$synch to hang.
 * 12 Sep 94 - RBR
 *    All pipes now use event flag zero.
 *    Removed the limit on the number of pipes.
 *    Added members to PIPE structure and memory corruption tests.
 */

/* This won't work with GCC, but it won't cause any problems either.  */
#define MODULE	PIPE
#define VERSION "V1.5"

#ifdef __DECC
#pragma module MODULE VERSION
#else
#ifdef VAXC
#module MODULE VERSION
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <iodef.h>
#include <ssdef.h>
#include <syidef.h>
#include <clidef.h>
#include <stsdef.h>
#include <dvidef.h>
#include <nam.h>
#include <descrip.h>
#include <errno.h>
#include <file.h>
#include <lib$routines.h>
#include <starlet.h>
#include <setjmp.h>
#include "vms-types.h"

/* A linked list of pipes, for internal use only */
struct PIPE
{
  struct PIPE *next;            /* next pipe in the chain */
  struct PIPE *prev;            /* previous pipe in the chain */
  struct PIPE *self;            /* self reference */
  int mode;                     /* pipe I/O mode (read or write) */
  long status;                  /* subprocess completion status */
  struct IOSB iosb;             /* pipe I/O status block */
  FILE *file;                   /* pipe file structure */
  int pid;                      /* pipe process id */
  short chan;                   /* pipe channel */
  jmp_buf jmpbuf;		/* jump buffer, if needed */
  int has_jmpbuf;		/* flag */
};

/* Head of the pipe chain */
static struct PIPE *phead = NULL, *ptail = NULL;

static unsigned char evf = 1;

/*
 * Exit handler for current process, established by popen().
 * Force the current process to wait for the completion of children
 *   which were started via popen().
 * Since
 */
static int
pwait (status)
  int status;
{
  struct IOSB iosb;
  struct PIPE *this;
  int ret = 0;

  this = phead;
  while (this)
    {
      if (this->self != this)
        {
	  ret = -1;
	  break;
	}
      if (!this->iosb.status)
	{
	  fflush (this->file);
	  if (this->mode == O_WRONLY)
	    sys$qio (0, this->chan, IO$_WRITEOF, &iosb,
		     0, 0, 0, 0, 0, 0, 0, 0);
	  fclose (this->file);
	  sys$synch (evf, &this->iosb);
	}
      else
	fclose(this->file);
      sys$dassgn (this->chan);
      this = this->next;
    }
  return ret;
}

/*
 * Close a "pipe" created by popen()
 * Return codes
 * >0  VMS exit status of process
 *  0  success, pipe was closed
 * -1  stream not found in list of pipes
 * -2  memory corruption detected
 */
int
pclose (stream)
  FILE *stream;
{
  struct IOSB iosb;
  struct PIPE *this = phead;
  long child_status;

  while (this && this->self == this && this->file != stream)
    this = this->next;

  /* Pipe not found or failed sanity check */
  if (!this)
    return -1;
  else if (this->self != this)
    return -2;

  /* Flush the I/O buffer and wait for the close to complete */
  if (!this->iosb.status)
    {
      fflush (this->file);
      if (this->mode == O_WRONLY)
	sys$qio (0, this->chan, IO$_WRITEOF, &iosb,
		 0, 0, 0, 0, 0, 0, 0, 0);
      fclose (this->file);
      sys$synch (evf, &this->iosb);
    }
  else
    fclose (this->file);
  sys$dassgn (this->chan);

  /* Remove `this' from the list of pipes and free its storage */
  if (this == ptail)
    ptail = this->prev;
  if (this == phead)
    phead = this->next;
  if (this->prev)
    this->prev->next = this->next;
  if (this->next)
    this->next->prev = this->prev;
  child_status = this->status;
  free (this);

  if (!(child_status & STS$M_SUCCESS))
    return child_status;
  else
    return 0;
}

/*
 * Subprocess AST completion routine
 * Indicate successful completion in the iosb and clear the pid.
 * Note that the channel is *not* deassigned and the file is
 *   *not* closed.
 */
void
pdone (this)
  struct PIPE *this;
{
  struct IOSB iosb;

  if (this->self != this)
    return;
  this->iosb.status = 1;
  this->pid  = 0;
  if (this->has_jmpbuf)
    {
      this->has_jmpbuf = 0;
      longjmp (this->jmpbuf, 1);
    }
}

/* A failed popen setup can occur after LIB$SPAWN has installed PDONE as an
   AST.  Do not release THIS until that callback has either already run or
   has observed completion of a requested child deletion.  */
static void
terminate_pipe_process (this)
  struct PIPE *this;
{
  if (!this->iosb.status)
    {
      if (this->pid)
	sys$delprc (&this->pid, 0);
      sys$synch (evf, &this->iosb);
    }
}

int
pipe_set_fd_jmpbuf (fd, jmpbuf)
     int fd;
     jmp_buf jmpbuf;
{
  struct PIPE *this = phead;

  while (this)
    if (fileno (this->file) == fd)
      {
	memcpy (this->jmpbuf, jmpbuf, sizeof (jmp_buf));
	this->has_jmpbuf = 1;
	if (this->pid == 0)
	  {
	    this->has_jmpbuf = 0;
	    longjmp (this->jmpbuf, 1);
	  }
	return 0;
      }
    else
      this = this->next;
  return 1;
}

pipe_unset_fd_jmpbuf (fd)
     int fd;
{
  struct PIPE *this = phead;

  while (this)
    if (fileno (this->file) == fd)
      {
	this->has_jmpbuf = 0;
	return 0;
      }
    else
      this = this->next;
  return 1;
}

/* Exit handler control block for the current process. */
static struct EXHCB pexhcb = { 0, pwait, 1, &pexhcb.exh$l_status, 0 };

struct Vstring
{
  short length;
  char body[NAM$C_MAXRSS+1];
};

/*
 * Emulate a unix popen() call using lib$spawn
 *
 * if mode == "w", lib$spawn uses the mailbox for sys$input
 * if mode == "r", lib$spawn uses the mailbox for sys$output
 *
 * Don't now how to handle both read and write
 *
 * Returns
 *   FILE *  file pointer to the pipe
 *   NULL    indicates an error ocurred, check errno value
 */
FILE *
popen (cmd, mode)
  const char *cmd;
  const char *mode;
{
#ifdef __DECC
#pragma message save
#pragma message disable ADDRCONSTEXT
#endif
  int i, status, flags, mbxsize;
  struct IOSB iosb;
  struct dsc$descriptor_s cmddsc, mbxdsc;
  struct Vstring mbxname = { sizeof(mbxname.body) };
  struct itm$list3 mbxlist[2] = {
    { sizeof(mbxname.body)-1, DVI$_DEVNAM, &mbxname.body, (size_t *) &mbxname.length },
    { 0, 0, 0, 0} };
  struct itm$list3 syilist[2] = {
    { sizeof(mbxsize), SYI$_MAXBUF, &mbxsize, (size_t *) 0 },
    { 0, 0, 0, 0} };
  static int noExitHandler = 1;
  struct PIPE *this;
#ifdef __DECC
#pragma message restore
#endif

  if (strcmp (mode, "r") != 0 && strcmp (mode, "w") != 0)
    {
      errno = EINVAL;
      return NULL;
    }
  if (strlen (cmd) > USHRT_MAX)
    {
      errno = E2BIG;
      return NULL;
    }

  /* First allocate space for the new pipe */
  this = (struct PIPE *) calloc (1, sizeof(struct PIPE));
  if (!this)
    {
      errno = ENOMEM;
      return NULL;
    }

  /* Sanity check value */
  this->self = this;

  /* Use the smaller of SYI$_MAXBUF and 2048 for the mailbox size */
  status = sys$getsyiw(0, 0, 0, syilist, &iosb, 0, 0, 0);
  if (!(status & STS$M_SUCCESS) || !(iosb.status & STS$M_SUCCESS))
    {
      vaxc$errno = (status & STS$M_SUCCESS) ? iosb.status : status;
      errno = EVMSERR;
      free (this);
      perror ("popen, $GETSYIW failure for SYI$_MAXBUF");
      return NULL;
    }

  if (mbxsize > 2048)
    mbxsize = 2048;

  status = sys$crembx (0, &this->chan, mbxsize, mbxsize, 0, 0, 0, 0);
  if (!(status & STS$M_SUCCESS))
    {
      vaxc$errno = status;
      errno = EVMSERR;
      free (this);
      perror ("popen, $CREMBX failure");
      return NULL;
    }

  /* Retrieve mailbox name, use for fopen */
  status = sys$getdviw (0, this->chan, 0, &mbxlist, &iosb, 0, 0, 0);
  if (!(status & STS$M_SUCCESS) || !(iosb.status & STS$M_SUCCESS))
    {
      vaxc$errno = (status & STS$M_SUCCESS) ? iosb.status : status;
      errno = EVMSERR;
      sys$dassgn (this->chan);
      free (this);
      perror ("popen, $GETDVIW failure");
      return NULL;
    }

  /* Spawn the command using the mailbox as the name for sys$input */
  mbxname.body[mbxname.length] = 0;
  mbxdsc.dsc$w_length  = mbxname.length;
  mbxdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
  mbxdsc.dsc$b_class   = DSC$K_CLASS_S;
  mbxdsc.dsc$a_pointer = mbxname.body;

  cmddsc.dsc$w_length  = strlen(cmd);
  cmddsc.dsc$b_dtype   = DSC$K_DTYPE_T;
  cmddsc.dsc$b_class   = DSC$K_CLASS_S;
  cmddsc.dsc$a_pointer = (char *)cmd;
  flags = CLI$M_NOWAIT;
  if (strcmp(mode,"w") == 0)
    {
      status = lib$spawn (&cmddsc, &mbxdsc, 0, &flags, 0, &this->pid,
                          &this->status, &evf, &pdone, this->self);
      this->mode = O_WRONLY;
    }
  else
    {
      status = lib$spawn (&cmddsc, 0, &mbxdsc, &flags, 0, &this->pid,
                          &this->status, &evf, &pdone, this->self);
      this->mode = O_RDONLY;
    }
  if (!(status & STS$M_SUCCESS))
    {
      vaxc$errno = status;
      errno = EVMSERR;
      sys$dassgn (this->chan);
      free (this);
      perror("popen, LIB$SPAWN failure");
      return NULL;
    }

  /* Set up an exit handler so the subprocess isn't prematurely killed */
  if (noExitHandler)
    {
      status = sys$dclexh (&pexhcb);
      if (!(status & STS$M_SUCCESS))
        {
          vaxc$errno = status;
          errno = EVMSERR;
          terminate_pipe_process (this);
          sys$dassgn (this->chan);
          free (this);
          perror("popen, $DCLEXH failure");
          return NULL;
        }
      noExitHandler = 0;
    }

  /* Pipes are always binary mode devices */
  if (this->mode == O_WRONLY)
    this->file = fopen (mbxname.body, "w");
  else
    this->file = fopen (mbxname.body, "r");

  /* Paranoia, check for failure again */
  if (!this->file)
    {
      terminate_pipe_process (this);
      sys$dassgn (this->chan);
      free (this);
      perror ("popen, fopen failure");
      return NULL;
    }

  this->has_jmpbuf = 0;

  /* Insert the new pipe into the list of open pipes */
  if (phead)
    {
      ptail->next = this;
      this->prev = ptail;
      ptail = this;
    }
  else
    phead = ptail = this;

  return (this->file);
}


#ifdef TEST_PIPE
int
main (argc, argv)
  int argc;
  char **argv;
{
  FILE *stdpipe;
  char line[512];

  while (1)
    {
      printf ("\nEnter a command to run >> ");
      fgets (line, 511, stdin);
      if (!strlen(line))
        exit (1);
      line[strlen(line)-1] = 0;
      stdpipe = popen (line, "r");
      if (!stdpipe)
        {
          fprintf (stderr, "popen failed.\n");
          exit(44);
        }
#if 1
#if 0
      do {
	if (fgets (line, 511, stdpipe) == NULL)
	  break;
	fputs (line, stdout);
      } while (!feof(stdpipe));
#else
      while (!feof(stdpipe)) {
	if (fgets (line, 511, stdpipe) == NULL)
	  break;
	fputs (line, stdout);
      }
#endif
#else
      {
	int c,i;
	while ((c = fread (line, 1, 511, stdpipe)) != 0) {
	  line[c] = '\0';
	  printf ("\nc: %d\n", c);
#if 0
	  fputs (line, stdout);
#else
	  for (i = 0; i < c; i++)
	    if ((line[i] & 127) > 32)
	      fputc (line[i],stdout);
	    else
	      fprintf (stdout, "\\%03o", line[i]);
#endif
	  if (c < 511)
	    break;
	}
	printf ("\nc: %d\n", c);
      }
#endif
      pclose (stdpipe);
    }
}
#endif
