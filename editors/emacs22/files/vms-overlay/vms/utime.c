/*
 * Copyright (C) 1994 the Free Software Foundation, Inc.
 *
 * Author: Roland B. Roberts (roberts@nsrl.rochester.edu)
 *
 * This file is a part of GNU VMSLIB, the GNU library for porting GNU
 * software to VMS.
 *
 * GNUVMSLIB is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUVMSLIB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNUVMSLIB; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * VMS emulation of unix utime() library call
 *
 * int utime (char *path, struct utimbuf *timep);
 * struct utimbuf { time_t atime, mtime; };
 * 
 * Description
 *   utime() sets a file's access and modification times but otherwise
 *   leaves a time unchanged.
 * 
 * Compatibility
 * + VMS files have no atime field, so the atime field is ignored.
 * + mtime is measured in seconds from 1 Jan 1970, the zero-point
 *   for unix system times.
 * + On VMS, the zero-time is 17 Nov 1858; however, mtime < 0 is *NOT*
 *   allowed; time_t is unsigned. 
 * + utime(path,NULL) will allow the owner or `superuser' to set the file
 *   revision time to the current time.  On VMS, this is interpreted as
 *     1. the current process uic matches the file owner uic, or
 *     2. the current process has SYSPRV and the file protection
 *        allows SYSTEM write access, or
 *     3. the current process had BYPASS privilege.
 *   If none of the above are true, utime fails.
 * + utime(path,&newtime) will set the file revision time as requested
 *   if the current process has write access to the file.  The success
 *   status of utime is determined by the $qio IO$_DEACCESS completion
 *   status.  utime relies let the OS to determine file writeability.
 * 
 * Return codes
 *   0 on sucess
 *  -1 on failure
 *
 * Reasons for failure
 * + Filename contains wildcards; errno = EVMSERR, vaxc$errno = RMS$_FNM
 * + Filename is in unix format and shell$to_vms did not return any 
 *   VMS format filenames; errno = EVMSERR, vaxc$errno = RMS$_FNM
 * + timep is NULL but the caller is not the file owner or `superuser';
 *   errno = EACCES
 * + Any system service failuer; errno = EVMSERR, vaxc$errno = return
 *   status of the failed system service.
 *
 * `Bugs'
 *  + utime cannot set times before 1-JAN-1970 0:00:00.00
 *  + utime ignores the atime field
 *  + utime is not atomic with respect to file access, thus a race
 *    condition results if two processes try to access the same file
 *
 * Author
 *   Roland B. Roberts (roberts@nsrl.rochester.edu)
 *
 * Miscellany
 * + If compiled with GNU C, this routine will use `long long' for
 *   64-bit time arithmetic.  In this case, unix_time_to_vms is a
 *   macro.  If compiled with VAXC, unix_time_to_vms is a function call
 *   and the code for qadd and qmul is included to add and multiple two
 *   64-bit integers.
 * + If compiled with UTIME_DEBUG defined, you will get lots of diagnostics
 *   printed showing the outcome of most of the system service calls
 * + If compiled with UTIME_TEST defined, this file will serve as a standalone
 *   program behaving somewhat like `touch':
 *      $ utime := $disk:[dir]utime
 *      $ utime file [seconds]
 *   where `file' is the name of the file whose revision time you want to
 *   change, and `seconds' is time (in seconds) to set from 1 Jan 1970.  If
 *   seconds is not specified, it defaults to the current time.
 */
#include <rms.h>
#include <atrdef.h>
#include <fibdef.h>
#include <stsdef.h>
#include <prvdef.h>
#include <jpidef.h>
#include <iodef.h>
#include <errno.h>
#include <descrip.h>
#include <string.h>
#include <stdio.h>
#include <unixlib.h>		/* Really only needed on AXP.  */
#include <starlet.h>
#include <lib$routines.h>
#include "utime.h"

#define NOREAD     0x01
#define NOWRITE    0x02
#define NOEXECUTE  0x04
#define NODELETE   0x08

#ifdef UTIME_DEBUG
static int debug = 1;
#else
static int debug = 0;
#endif

#ifdef __GNUC__
/* use native 64-bit arithmetic */
#define unix_time_to_vms(x,y) \
  { unsigned long long reftime, tmptime = (x); \
    $DESCRIPTOR (time,"1-JAN-1970 0:00:00.00"); \
    sys$bintim (&time, &reftime); \
    y = tmptime * 10000000 + reftime; }
#else
int unix_time_to_vms (time_t, unsigned long *);
#endif

/* Forward declarations.  */
static int qadd (unsigned long *, unsigned long *, unsigned long *);
static int qmul (unsigned long *, unsigned long *, unsigned long *);

#define Dprintf if (debug) printf

#define chkstat(x,y) \
  { int __$$chkstat$x = (x); \
    if (!(__$$chkstat$x & STS$M_SUCCESS)) { \
      if (chan) { sys$dassgn (chan); chan = 0; } \
      vaxc$errno = __$$chkstat$x; \
      errno = EVMSERR; \
      lib$signal ((__$$chkstat$x & 0xFFFFFFF8) | STS$K_INFO); \
      return (y); } }

/* descrip.h doesn't have everything ... */
struct dsc$descriptor_fib
{ unsigned long fib$l_len; struct fibdef *fib$l_addr; };

/* fibdef.h doesn't give us initialized structures like cc$rms_rab, etc. */
#ifdef __GNUC__
#include <gnu_hacks.h>
GLOBALDEF(struct fibdef,cc$fib_init,{0});
#else
globaldef struct fibdef cc$fib_init; /* initialized to zero by default? */
#endif

struct IOSB
{ unsigned short status, count;
  unsigned long devdep; };

static char tryfile[NAM$C_MAXRSS+1];

static int settry (char *file, int ignored)
{
  size_t length = strlen (file);
  if (length > NAM$C_MAXRSS)
    {
      tryfile[0] = '\0';
      return -1;
    }
  memcpy (tryfile, file, length + 1);
  return 0;
}

/* ODS-5 quotes a literal delimiter with an odd number of carets.  Searching
   with strrchr therefore mistakes a quoted ']', '>', or ':' in the leaf name
   for the directory boundary and can update the wrong file specification.  */
static int
vms_char_escaped (start, character)
     const char *start;
     const char *character;
{
  int carets = 0;

  while (character > start && character[-1] == '^')
    {
      carets++;
      character--;
    }
  return carets & 1;
}

static char *
last_vms_separator (name)
     char *name;
{
  char *last = 0;
  char *scan;

  for (scan = name; *scan; scan++)
    if ((*scan == ']' || *scan == '>' || *scan == ':')
	&& !vms_char_escaped (name, scan))
      last = scan;
  return last;
}

struct vstring
{ unsigned short length; char string[NAM$C_MAXRSS+1]; };


int utime (char *path, struct utimbuf *timep)
{
  struct FAB fab;
  struct NAM nam;
  struct {
#ifdef __GNUC__
    unsigned long long backup, create, expire, revise;
#else
    unsigned long backup[2], create[2], expire[2], revise[2];
#endif
    unsigned long uic;
    union { unsigned short value;
            struct { unsigned system : 4;
                     unsigned owner  : 4;
                     unsigned group  : 4;
                     unsigned world  : 4;
                   } bits;
          } prot;
  } Fat = { 0 };
  struct atrdef atrlst[] = {
    { ATR$S_CREDATE,  ATR$C_CREDATE,  &Fat.create },
    { ATR$S_REVDATE,  ATR$C_REVDATE,  &Fat.revise },
    { ATR$S_EXPDATE,  ATR$C_EXPDATE,  &Fat.expire },
    { ATR$S_BAKDATE,  ATR$C_BAKDATE,  &Fat.backup },
    { ATR$S_FPRO,     ATR$C_FPRO,     &Fat.prot },
    { ATR$S_UIC,      ATR$C_UIC,      &Fat.uic },
    { 0, 0, 0}
  };
  struct fibdef fib;
  struct dsc$descriptor_fib fibdsc = { sizeof(fib), (void *) &fib };
  struct IOSB iosb;
  struct {
    unsigned long uic;
    union prvdef privs;
  } prcinfo;
  struct itmlst {
    short code, length;
    void *bufadr, *retlen; } jpilst[] = {
      { sizeof(prcinfo.uic),   JPI$_UIC,     &prcinfo.uic,   (void *) 0 },
      { sizeof(prcinfo.privs), JPI$_CURPRIV, &prcinfo.privs, (void *) 0 },
      { 0, 0, 0, 0 }
    };
#ifdef __GNUC__
  unsigned long long newtime;
#else
  unsigned long newtime[2];
#endif
  long status;
  short chan = 0;
  char *leaf;

  struct vstring file;
  struct dsc$descriptor_s filedsc = {
    NAM$C_MAXRSS, DSC$K_DTYPE_T, DSC$K_CLASS_S, (void *) file.string };
  struct vstring device;
  struct dsc$descriptor_s devicedsc = {
    NAM$C_MAXRSS, DSC$K_DTYPE_T, DSC$K_CLASS_S, (void *) device.string };
  struct vstring time;
  struct dsc$descriptor_s timedsc = {
    NAM$C_MAXRSS, DSC$K_DTYPE_T, DSC$K_CLASS_S, (void *) time.string };
  struct vstring result;
  struct dsc$descriptor_s resultdsc = {
    NAM$C_MAXRSS, DSC$K_DTYPE_T, DSC$K_CLASS_S, (void *) result.string };

  /*
   * Disallow wildcards in filenames.
   */
  if (strpbrk(path,"*%?")) {
    vaxc$errno = RMS$_FNM;      /* error in file name */
    errno = EVMSERR;
    return (-1);
  }

#ifdef __DECC
#pragma message save 
#pragma message disable macroredef 
#define shell$to_vms decc$to_vms
#pragma message restore 
#endif
  /*
   * Try to convert unix style pathnames to VMS style.
   */
  if (strchr(path,'/')) {
    tryfile[0] = '\0';
    if (!shell$to_vms (path, settry, 0, 0)) {
      vaxc$errno = RMS$_FNM;    /* error in file name */
      errno = EVMSERR;
      return (-1);
    }
    if (!tryfile[0]) {
      vaxc$errno = RMS$_FNM;
      errno = EVMSERR;
      return (-1);
    }
  }
  else if (settry (path, 0) < 0) {
    vaxc$errno = RMS$_FNM;
    errno = EVMSERR;
    return (-1);
  }
  
  /*
   * Allocate and initialize a fab and nam structures.
   */
  fab    = cc$rms_fab;
  nam    = cc$rms_nam;
  
  nam.nam$l_esa = file.string;
  nam.nam$b_ess = NAM$C_MAXRSS;
  nam.nam$l_rsa = result.string;
  nam.nam$b_rss = NAM$C_MAXRSS;
  fab.fab$l_fna = tryfile;
  fab.fab$b_fns = strlen(tryfile);
  fab.fab$l_nam = &nam;
  Dprintf ("utime looking for `%s'\n", tryfile);
  
  /*
   * Validate filespec syntax and device existence by calling sys$parse.
   */
  status = sys$parse (&fab, 0, 0);
  chkstat (status, -1);
  file.string[nam.nam$b_esl] = 0;
  Dprintf ("sys$parse gives nam$l_esa = `%s'\n", nam.nam$l_esa);

  /*
   * Find matching filespec.
   */
  status = sys$search (&fab, 0, 0);
  chkstat (status, -1);
  file.string[nam.nam$b_esl] = 0;
  Dprintf ("sys$search gives nam$l_esa = `%s'\n", nam.nam$l_esa);
  result.string[result.length=nam.nam$b_rsl] = 0;
  Dprintf ("sys$search gives nam$l_rsa = `%s'\n", nam.nam$l_rsa);
  
  /*
   * Get the device name and assign an IO channel.
   */
  strncpy (device.string, nam.nam$l_dev, nam.nam$b_dev);
  devicedsc.dsc$w_length  = nam.nam$b_dev;
  status = sys$assign (&devicedsc, &chan, 0, 0, 0);
  chkstat (status, -1);
  Dprintf ("sys$assign openned channel %d\n", chan);

  /*
   * Initialize the FIB and fill in the directory id field.
   */
  fib = cc$fib_init;
#ifdef __DECC
  fib.fib$w_did[0] = nam.nam$w_did[0];
  fib.fib$w_did[1] = nam.nam$w_did[1];
  fib.fib$w_did[2] = nam.nam$w_did[2];
  fib.fib$l_acctl = 0;
#else
  fib.fib$r_did_overlay.fib$w_did[0]  = nam.nam$w_did[0];
  fib.fib$r_did_overlay.fib$w_did[1]  = nam.nam$w_did[1];
  fib.fib$r_did_overlay.fib$w_did[2]  = nam.nam$w_did[2];
  fib.fib$r_acctl_overlay.fib$l_acctl = 0;
#endif
  fib.fib$l_wcc = 0;
  leaf = last_vms_separator (result.string);
  if (!leaf)
    {
      vaxc$errno = RMS$_FNM;
      errno = EVMSERR;
      sys$dassgn (chan);
      chan = 0;
      return -1;
    }
  strcpy (file.string, leaf + 1);
  filedsc.dsc$w_length = strlen(file.string);
  result.string[result.length=0] = 0;

  /*
   * Open and close the file to fill in the attributes.
   */
  status = sys$qiow (0, chan, IO$_ACCESS|IO$M_ACCESS, &iosb, 0, 0,
                     &fibdsc, &filedsc, &result.length, &resultdsc, &atrlst, 0);
  chkstat (status, -1);
  chkstat (iosb.status, -1);
  result.string[result.length] = 0;
  Dprintf ("sys$qiow accessed file `%s'\n", result.string);
  status = sys$qiow (0, chan, IO$_DEACCESS, &iosb, 0, 0,
                     &fibdsc, 0, 0, 0, &atrlst, 0);
  chkstat (status, -1);
  chkstat (iosb.status, -1);
  Dprintf ("sys$qiow deaccessed file.\n");
  
#ifdef UTIME_DEBUG

#define DPUTPC(x) \
  if (debug && ~x) { \
    int i = x; \
    putchar (':'); \
    if (!(i&NOREAD))    putchar ('R'); \
    if (!(i&NOWRITE))   putchar ('W'); \
    if (!(i&NOEXECUTE)) putchar ('E'); \
    if (!(i&NODELETE))  putchar ('D'); }

  Dprintf ("file protection mask = S");
  DPUTPC (Fat.prot.bits.system);
  Dprintf (", O");
  DPUTPC (Fat.prot.bits.owner);
  Dprintf (", G");
  DPUTPC (Fat.prot.bits.group);
  Dprintf (", W");
  DPUTPC (Fat.prot.bits.world);

  Dprintf ("\nfile owner uic      = 0x%08x\n", Fat.uic);
  Dprintf ("current process uic = 0x%04x%04x\n", getgid(), getuid());

  status = sys$asctim (&time.length, &timedsc, &Fat.create, 0);
  chkstat (status, -1);
  time.string[time.length] = 0;
  Dprintf ("Create time: %s\n", time.string);
  
  status = sys$asctim (&time.length, &timedsc, &Fat.revise, 0);
  chkstat (status, -1);
  time.string[time.length] = 0;
  Dprintf ("Revise time: %s\n", time.string);

  status = sys$asctim (&time.length, &timedsc, &Fat.backup, 0);
  chkstat (status, -1);
  time.string[time.length] = 0;
  Dprintf ("Backup time: %s\n", time.string);

  status = sys$asctim (&time.length, &timedsc, &Fat.expire, 0);
  chkstat (status, -1);
  time.string[time.length] = 0;
  Dprintf ("Expire time: %s\n", time.string);
#endif /* UTIME_DEBUG */

  /*
   * Get the current process information.
   */
  status = sys$getjpiw (0, 0, 0, &jpilst, &iosb, 0, 0);
  chkstat (status, -1);
  chkstat (iosb.status, -1);
  if (!timep) {
    if (!((Fat.uic == prcinfo.uic) ||
#ifdef __GNUC__
          (prcinfo.privs.prv$r_prvdef_bits0.prv$v_sysprv &&
           !(Fat.prot.bits.system & NOWRITE)) ||
          (prcinfo.privs.prv$r_prvdef_bits0.prv$v_bypass)
#else
          (prcinfo.privs.prv$v_sysprv &&
           !(Fat.prot.bits.system & NOWRITE)) ||
          (prcinfo.privs.prv$v_bypass)
#endif
          )) {
      /* Not owner or `superuser' */
      errno = EACCES;
      sys$dassgn (chan);
      chan = 0;
      return (-1);
    }
  }
  else
    unix_time_to_vms (timep->mtime, newtime);
  /*
   * Reopen the file, modify the times and then close.
   */
#ifdef __DECC
  fib.fib$l_acctl = FIB$M_WRITE;
#else
  fib.fib$r_acctl_overlay.fib$l_acctl = FIB$M_WRITE;
#endif
  status = sys$qiow (0, chan, IO$_ACCESS|IO$M_ACCESS, &iosb, 0, 0,
                     &fibdsc, &filedsc, &result.length, &resultdsc, &atrlst, 0);
  chkstat (status, -1);
  chkstat (iosb.status, -1);
  result.string[result.length] = 0;
  Dprintf ("sys$qiow re-accessed file `%s'\n", result.string);
  if (timep) {
#ifdef __GNUC__  
    Fat.revise = newtime;
#else
    Fat.revise[0] = newtime[0];
    Fat.revise[1] = newtime[1];
#endif
  }
  else
    {
      status = sys$gettim(&Fat.revise);
      chkstat (status, -1);
    }
  
#ifdef UTIME_DEBUG
  status = sys$asctim (&time.length, &timedsc, &Fat.revise, 0);
  chkstat (status, -1);
  time.string[time.length] = 0;
  Dprintf ("New revise time: %s\n", time.string);
#endif

  status = sys$qiow (0, chan, IO$_DEACCESS, &iosb, 0, 0,
                     &fibdsc, 0, 0, 0, &atrlst, 0);
  chkstat (status, -1);
  chkstat (iosb.status, -1);
  Dprintf ("sys$qiow deaccessed file.\n");
  /*
   * Deassign the channel and exit.
   */
  status = sys$dassgn (chan);
  chan = 0;
  chkstat (status, -1);
  return 0;
}

#ifndef __GNUC__
/*
 * Convert a 32-bit unix time (zero at 1-JAN-1970 0:00:00.00) to
 * the equivalent VMS 64-bit time
 */
int unix_time_to_vms (time_t unixtime, unsigned long *vmstime)
{
  unsigned long reftime[2];
  $DESCRIPTOR (time,"1-JAN-1970 0:00:00.00");
  
  vmstime[0] = unixtime;
  vmstime[1] = 0;
  reftime[0] = 10000000;
  reftime[1] = 0;
  qmul (vmstime, vmstime, reftime);
  sys$bintim (&time, &reftime);
  qadd (vmstime, vmstime, reftime);
  return (1);
}

/*
 * qadd() --- Add two VAX quadwords (64-bit integers)
 * Return codes
 *   0 on success
 *  -1 on overflow (think of it as a carry flag)
 */
static int qadd (unsigned long *r, unsigned long *a1, unsigned long *a2)
{
  /* VAX low-order bits are on the right */
  unsigned long z[4];
  z[0] = (a1[0] & 0x0000ffff) + (a2[0] & 0x0000ffff);
  z[1] = (a1[0] >> 16) + (a2[0] >> 16) + (z[0] >> 16);
  z[2] = (a1[1] & 0x0000ffff) + (a2[1] & 0x0000ffff) + (z[1] >> 16);
  z[3] = (a1[1] >> 16) + (a2[1] >> 16) + (z[2] >> 16);
  r[0] = (z[0] & 0x0000ffff) | (z[1] << 16);
  r[1] = (z[2] & 0x0000ffff) | (z[3] << 16);
  if (z[3] >> 16)
    return -1;
  else
    return 0;
}

/*
 * qmul() --- Multiply two VAX quadwords (64-bit integers)
 * Return codes
 *   0 on success
 *  -1 on overflow
 * In the event of an overflow, the 64-bit result contains only the
 * low order bits; the overflow part is lost.
 */
static int qmul (unsigned long *r, unsigned long *m1, unsigned long *m2) 
{       
  int i, j;
  unsigned long a[4], b[4], c[8];

  /* split into small pieces so we can multiply w/o overflow */
  a[0] = m1[0] & 0x0000ffff;
  a[1] = m1[0] >> 16;
  a[2] = m1[1] & 0x0000ffff;
  a[3] = m1[1] >> 16;

  b[0] = m2[0] & 0x0000ffff;
  b[1] = m2[0] >> 16;
  b[2] = m2[1] & 0x0000ffff;
  b[3] = m2[1] >> 16;

  for (i = 0; i < 8; i++)
    c[i] = 0;
  /* do multiply */
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
      c[i+j] += a[i] * b[j];
  /* do carry */
  for (i = 0; i < 7; i++)
    if ((j=c[i]>>16))
      c[i+1] += j, c[i] &= 0x0000ffff;
  /* squeeze back into quadword */
  for (i = 0; i < 4; i++)
    c[i] = c[2*i] | (c[2*i+1]<<16);
  /* set return value */
  r[0] = c[0];
  r[1] = c[1];
  /* check for overflow */
  if (c[3] || c[4])
    return -1;
  else
    return 0;
}
#endif /* !__GNUC__ */

#ifdef UTIME_TEST

#include <stdlib.h>
main (int argc, char **argv)
{
  struct utimbuf newtime;
  if (argc == 3) {
    newtime.atime = newtime.mtime = atol(argv[2]);
    utime (argv[1], &newtime);
  }
  else
    utime (argv[1], NULL);
  exit (1);
}
#endif
