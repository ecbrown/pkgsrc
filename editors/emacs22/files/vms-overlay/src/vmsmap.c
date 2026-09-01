/* VMS mapping of data and alloc arena for GNU Emacs.
   Copyright (C) 1986, 1987 Free Software Foundation, Inc.
   
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

/* Written by Mukesh Prasad.
   Adapted for Emacs 21 by Thien-Thi Nguyen.
   Minor details added to support I64, Hartmut Becker */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "lisp.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "getpagesize.h"
#include "puresize.h"
#include <rab.h>
#include <fab.h>
#include <rmsdef.h>
#include <secdef.h>
#include <descrip.h>

#include <lib$routines.h>
#include <starlet.h>		/* for sys$crmpsc, etc */

/* RMS block size (bytes) */
#define	BLOCKSIZE	512

/* Maximum number of bytes to be written in one RMS write.
   This must be a multiple of BLOCKSIZE.  */
#define	MAXWRITE	(BLOCKSIZE * 30)


/* This funniness is to ensure that sdata occurs alphabetically BEFORE the
   $DATA psect and that edata occurs after ALL Emacs psects.  This is
   because the VMS linker sorts all psects in a cluster alphabetically
   during the linking, unless you use the cluster_psect command.  Emacs
   uses the cluster command to group all Emacs psects into one cluster;
   this keeps the dumped data separate from any loaded libraries. */

#ifdef __GNUC__
/* We need a large sdata array because otherwise the impure storage will end up
   in low memory, and this will screw up garbage collection (Emacs will not
   be able to tell the difference between a string length and an address).
   This array guarantees that the impure storage is at a sufficiently high
   address so that this problem will not occur. */
char sdata[1] asm("_$$PsectAttributes_NOOVR$$$$$$DATA") = { 'x' };
char edata[1] asm("_$$PsectAttributes_NOOVR$$____DATA") = { 'x' };
#else /* !__GNUC__ */
globaldef {"$$$$DATA"} char sdata[1] = { 'x' }; /* Start of saved data area */
globaldef {"____DATA"} char edata[1] = { 'x' }; /* End of saved data area */
#endif /* !__GNUC__ */

/* It looks like the AXP/OpenVMS linker puts the psects like this:
	$DATA$	containing all initialized data.
	$CODE$	containing the code
	$BSS$	containing uninitialized data.
   This means we have to put two sections in the dump file.  The sdata and
   edata up there find their way into the initialized part of the data.
   We thus need to add a second section to the dump file.  Boy, will this
   look hairy!  */

#ifndef __GNUC__
#ifdef __DECC
globaldef {"$$$$BSS"} char BSS_sdata[1]; /* Start of saved data area */
globaldef {"____BSS"} char BSS_edata[1]; /* End of saved data area */
#endif /* __DECC */
#endif /* !__GNUC__ */


/* Various ways to determine ranges at runtime.  */

/* Arg to a range_func_t is 1 to return the start, otherwise the end.  */
typedef char * (range_func_t) (int);

#define REQUEST_RANGE_START 1
#define REQUEST_RANGE_END   0

#define RANGE_DIFF(fn) \
  ((fn (REQUEST_RANGE_END)) - (fn (REQUEST_RANGE_START)))

/* The following code should probably really reside in sysdep.c, but
   there's a bug in the DEC C v1.3 compiler, concerning globaldef and
   globalref.  */

#define FIRST_PAGE_BYTE(p,ps) \
  ((char *)(((unsigned long)(p)) & ~((unsigned long)(ps) - 1)))

#define LAST_PAGE_BYTE(p,ps) \
  ((char *)((((unsigned long) (p)) & \
	     ~((unsigned long)(ps) - 1)) + ((unsigned long)(ps) - 1)))

#define RANGE_FUNC(name,startexp,endexp)	\
static						\
char *						\
name (int startp)				\
{						\
  return (startp ? (startexp) : (endexp));	\
}

RANGE_FUNC (data_range,
	    FIRST_PAGE_BYTE ((char *) &sdata, getpagesize ()),
	    LAST_PAGE_BYTE ((char *) &edata, getpagesize ()))

RANGE_FUNC (BSS_range,
	    FIRST_PAGE_BYTE ((char *) &BSS_sdata, getpagesize ()),
	    LAST_PAGE_BYTE ((char *) &BSS_edata, getpagesize ()))

extern char *initial_vms_brk_start;
extern char *initial_vms_brk_end;
RANGE_FUNC (initial_brk_range,
	    initial_vms_brk_start,
	    initial_vms_brk_end - 1)

extern int *pure;
RANGE_FUNC (pure_range,
	    (char *)(&pure),
	    LAST_PAGE_BYTE ((char *)(&pure) + PURESIZE - 1, getpagesize ()))

#ifdef __ia64
extern void *sbss_start;
extern void *sdata_end;
RANGE_FUNC (sdata_range,
	    sbss_start,
	    LAST_PAGE_BYTE (sdata_end,getpagesize ()))
#endif


typedef int (mapin_func_t) (unsigned int *inadr, unsigned int *retadr,
			    struct FAB *fab, int starting_block,
			    unsigned long p1);

typedef struct
{
  char		*nick;
  range_func_t	*range;
  mapin_func_t	*mapin;
} map_info_t;


static mapin_func_t normal_mapin;

static map_info_t maps [] =
{
  { "DATA", data_range, normal_mapin },
#ifndef __GNUC__
#ifdef __DECC
  { "BSS", BSS_range, normal_mapin },
#endif /* __DECC */
#endif /* !__GNUC__ */
  { "BRK", initial_brk_range, normal_mapin },
  { "PURE", pure_range, normal_mapin }
#ifdef __ia64
  ,
  { "SDATA", sdata_range, normal_mapin }
#endif
};
 
#define MAP_COUNT  (sizeof (maps) / sizeof (map_info_t))

typedef struct
{
  int	        first;		/* number of first RMS block */
  int           count;		/* count of RMS blocks needed */
  char *        start;		/* start address */
  char *        end;		/* end address */
  unsigned int  check;		/* LSFR (for sanity check) */
  unsigned int  flags;		/* sys$crmpsc flags (on mapin) */
} ondisk_map_info_t;

typedef struct
{
  int                ident;		/* major, minor, edit */
  int                map_count;		/* set to MAP_COUNT */
  int                total;		/* count of all RMS blocks needed */
  ondisk_map_info_t  idx[MAP_COUNT];	/* the index */
  char               magic[8];		/* "EMACSMAP" (we're so clever) */
} mapfile_header_t;

#define BLOCKS_NEEDED(start,end) \
  (((char *)(end) - (char *)(start) + 1 + BLOCKSIZE - 1) / BLOCKSIZE)

#define HEADER_BLOCK_COUNT \
  ((sizeof (mapfile_header_t) + BLOCKSIZE - 1) / BLOCKSIZE)

/* Block 0 can never be written because rab$l_bkt == 0 actually means
   to use the "next" block as kept track by an invisible RMS-internal
   counter (the infamous Next Block Pointer, NBP).  Hence this forced
   off-by-one tweak.  */
#define MAP_DATA_START_BLOCK \
  (1 + HEADER_BLOCK_COUNT)


static
int
normal_mapin (unsigned int *inadr, unsigned int *retadr,
	      struct FAB *fab, int starting_block, unsigned long p1)
{
  return sys$crmpsc (inadr, retadr, 0,
		     SEC$M_CRF | SEC$M_WRT,
		     0, 0, 0,
		     fab->fab$l_stv, 0,
		     starting_block, 0, 0);
}


static
unsigned int
compute_checksum (start, end)
     char *start;
     char *end;
{
  unsigned int ans = 0;
  char *p;

  /* The `*p' check avoids wasteful shifting.  */
  for (p = start; p <= end; p++)
    if (*p)
      ans = (ans << 1) ^ *p;

  return ans;
}


/* Fill a `mapfile_header_t' with info from the current session.  */

static
void
set_header_from_current (header)
     mapfile_header_t *header;
{
  int i;
  ondisk_map_info_t *n, *v;	/* new and venerated */
  int unhandled_overlap = 0;
  int cur_block = MAP_DATA_START_BLOCK;

  memset (header, 0, sizeof *header);
  header->ident = ((EMACS_VERSION_MAJOR << 24)
		   + (EMACS_VERSION_MINOR << 16)
		   + EMACS_VERSION_EDIT);

  for (i = 0; i < MAP_COUNT; i++)
    {
      n = &(header->idx[i]);
      n->first = cur_block;
      n->start = (*maps[i].range) (REQUEST_RANGE_START);
      n->end   = (*maps[i].range) (REQUEST_RANGE_END);
      n->check = compute_checksum (n->start, n->end);
      n->flags = 0;		/* fixme? */
      n->count = BLOCKS_NEEDED (n->start, n->end);

      for (v = &(header->idx[0]); v < n; v++)
	{
	  /* Maps are disjunct.  */
          if (n->start > v->end || n->end < v->start)
	    continue;

	  /* There is some kind of overlap.  */
	  unhandled_overlap = 1;
	}

      cur_block += n->count;
    }

  header->map_count = MAP_COUNT;
  header->total = cur_block;
  {
    char *c = header->magic;

    c[0] = 'E' ; c[1] = 'M' ; c[2] = 'A' ; c[3] = 'C' ; c[4] = 'S';
    c[5] = 'M' ; c[6] = 'A' ; c[7] = 'P' ;
  }

  if (unhandled_overlap)
    {
      fprintf (stderr, "set_header_from_current: unhandled overlap!\n");
      abort ();
    }
}

/* Reject an incompatible or truncated dump before trusting any address or
   block number from it.  Mapping malformed input over Emacs itself is not a
   recoverable validation strategy.  */
static int
validate_mapfile_header (name, expected, actual)
     const char *name;
     const mapfile_header_t *expected;
     const mapfile_header_t *actual;
{
  int i;

  if (memcmp (actual->magic, "EMACSMAP", sizeof actual->magic) != 0)
    {
      fprintf (stderr, "mapin_data: bad magic for %s\n", name);
      return 0;
    }
  if (actual->ident != expected->ident)
    {
      fprintf (stderr, "mapin_data: incompatible Emacs version for %s\n",
	       name);
      return 0;
    }
  if (actual->map_count != MAP_COUNT || actual->total != expected->total)
    {
      fprintf (stderr, "mapin_data: invalid map index for %s\n", name);
      return 0;
    }

  for (i = 0; i < MAP_COUNT; i++)
    {
      const ondisk_map_info_t *want = &expected->idx[i];
      const ondisk_map_info_t *got = &actual->idx[i];

      if (got->start != want->start || got->end != want->end
	  || got->first != want->first || got->count != want->count
	  || got->flags != 0 || got->start > got->end
	  || got->first < MAP_DATA_START_BLOCK || got->count <= 0
	  || got->first > actual->total
	  || got->count > actual->total - got->first)
	{
	  fprintf (stderr, "mapin_data: invalid map %d in %s\n", i, name);
	  fprintf (stderr, "  expected range %p..%p, got %p..%p\n",
		   (void *) want->start, (void *) want->end,
		   (void *) got->start, (void *) got->end);
	  return 0;
	}
    }
  return 1;
}


/* Read header info and memory regions (maps) from disk.
   Return 1 if ok, 0 otherwise.  */

int
mapin_data (name)
     char * name;
{
  struct FAB fab;
  struct RAB rab;
  int status, size, i;
  mapfile_header_t bef;
  mapfile_header_t aft;

  set_header_from_current (&bef);

  /* Open map file.  */
  fab = cc$rms_fab;
  fab.fab$b_fac = FAB$M_BIO | FAB$M_GET;
  fab.fab$l_fna = name;
  fab.fab$b_fns = strlen (name);
  status = sys$open (&fab);
  if (status != RMS$_NORMAL)
    {
      printf ("Map file not available: %s\nRunning bare Emacs....\n",
	      name);
      return 0;
    }
  /* Connect the RAB block.  */
  rab = cc$rms_rab;
  rab.rab$l_fab = &fab;
  rab.rab$b_rac = RAB$C_SEQ;
  rab.rab$l_rop = RAB$M_BIO;
  status = sys$connect (&rab);
  if (status != RMS$_NORMAL)
    {
      sys$close (&fab);
      error ("RMS connect to map file %s failed: %s",
	     name, strerror (EVMSERR, status));
      return 0;
    }

  /* Read the header data.  */
  memset (&aft, 0, sizeof aft);
  rab.rab$l_ubf = (char *) &aft;
  rab.rab$w_usz = sizeof (aft);
  rab.rab$l_bkt = 1;		/* see comment for MAP_DATA_START_BLOCK */
  status = sys$read (&rab);
  if (status != RMS$_NORMAL || rab.rab$w_rsz != sizeof aft)
    {
      fprintf (stderr, "mapin_data: truncated header in %s\n", name);
      sys$close (&fab);
      return 0;
    }
  status = sys$close (&fab);
  if (status != RMS$_NORMAL)
    {
      fprintf (stderr, "mapin_data: RMS close failed for %s\n", name);
      return 0;
    }

  if (!validate_mapfile_header (name, &bef, &aft))
    return 0;

  /* All preliminary checks done, time to do the deed.  */
  for (i = 0; i < MAP_COUNT; i++)
    {
      unsigned int inadr[2];
      /* volatile */ unsigned int retadr[2];
      
      fab.fab$l_fop |= FAB$M_UFO;
      status = sys$open (&fab);
      if (status != RMS$_NORMAL)
	lib$stop (status);

      inadr[0] = (unsigned int) aft.idx[i].start;
      inadr[1] = (unsigned int) aft.idx[i].end;

      status = (*maps[i].mapin)(inadr, retadr, &fab,
				aft.idx[i].first,
				aft.ident);
      if (! (status & 1))
	lib$stop (status);

      if (inadr[0] != retadr[0])
	{
	  fprintf (stderr, "start moved on mapin: expected:%p got:%p\n",
		   (void *) inadr[0], (void *) retadr[0]);
	  abort ();
	}	    

      bef.idx[i].check = compute_checksum ((char *) inadr[0],
					   (char *) inadr[1]);
      if (aft.idx[i].check != bef.idx[i].check)
	{
	  fprintf (stderr, "checksum mismatch: expected:%08x got:%08x\n",
		   aft.idx[i].check, bef.idx[i].check);
	  abort ();
	}
    }

  /* cooperate with VMSGMALLOC.C */
  {
#ifdef GNU_MALLOC
    void malloc_clear_hooks ();
    malloc_clear_hooks ();
#endif /* GNU_MALLOC */
  }
  return 1;
}


static int write_map (struct RAB *, ondisk_map_info_t *);
extern int vms_malloc_overflow (void);

/* Write header info and memory regions (maps) to disk.
   Return 1 if ok, 0 otherwise.  */

int
mapout_data (into)
     char * into;
{
  struct FAB fab;
  struct RAB rab;
  int status, i;
  int cur_block;
  mapfile_header_t header;

  if (vms_malloc_overflow ())
    {
      static char buf[512];
      sprintf (buf, "%s\n%s\n%s %d (0x%x).\n",
	       "Out of initial allocation.",
	       "Must rebuild Emacs with more memory (VMS_ALLOCATION_SIZE).",
	       "Set it to at least",
	       RANGE_DIFF (initial_brk_range) + 1,
	       RANGE_DIFF (initial_brk_range) + 1);
      error (buf);
      return 0;
    }

  set_header_from_current (&header);

  /* Create map file.

     These fab$FOO fields are presently unset, inheriting values from the
     `cc$rms_fab': b_acmodes, b_bid, b_bks, b_bln, b_bls, v_chan_mode, l_ctx,
     w_deq, l_dev, l_dna, l_dns, b_fsz, w_gbc, w_ifi, b_journal, v_lnm_mode,
     l_mrn, w_mrs, l_nam, b_rtv, l_sdc, b_shr, l_sts, l_stv, l_xab.

     Some notes from the RMS manual:
     - b_bks (bucket size) only valid for relative and indexed files
     - b_bls (block size) only valid for tapes (not disk files)

     */

  fab = cc$rms_fab;
  fab.fab$b_fac = FAB$M_BIO | FAB$M_PUT;   /* block write access */
  fab.fab$l_fna = into;			   /* filename */
  fab.fab$b_fns = strlen (into);	   /* filename length */
  fab.fab$l_fop = FAB$M_CBT | FAB$M_WCK;   /* contiguous best-try option */
  fab.fab$b_org = FAB$C_SEQ;		   /* sequential organization */
  fab.fab$b_rat = 0;			   /* no record attributes */
  fab.fab$b_rfm = FAB$C_VAR;		   /* variable-length format */
  /* fab.fab$l_alq = sum_blocks; */
  fab.fab$l_alq = 2 + header.total;	   /* block allocation (+ kludge) */
  status = sys$create (&fab);
  if (status != RMS$_NORMAL)
    {
      error ("Could not create map file %s: %s",
	     into, strerror (EVMSERR, status));
      return 0;
    }

  /* Connect the RAB block.  */
  rab = cc$rms_rab;
  rab.rab$l_fab = &fab;
  rab.rab$b_rac = RAB$C_SEQ;
  rab.rab$l_rop = RAB$M_BIO | RAB$M_EOF;
  status = sys$connect (&rab);
  if (status != RMS$_NORMAL)
    {
      sys$close (&fab);
      error ("RMS connect to map file %s failed: %s",
	     into, strerror (EVMSERR, status));
      return 0;
    }
  /* Write the header.  */
  rab.rab$l_rbf = (char *) &header;
  rab.rab$w_rsz = sizeof (header);
  rab.rab$l_bkt = 1;		/* see comment for MAP_DATA_START_BLOCK */
  status = sys$write (&rab);
  if (status != RMS$_NORMAL)
    {
      error ("RMS write (header) to map file %s failed: %s",
	     into, strerror (EVMSERR, status));
      status = sys$close (&fab);
      if (status != RMS$_NORMAL)
	error ("RMS close on map file %s failed: %s",
	       into, strerror (EVMSERR, status));
      return 0;
    }

  /* Write the maps.  */
  for (i = 0; i < MAP_COUNT; i++)
    {
      if (! write_map (&rab, &(header.idx[i])))
	{
	  status = sys$close (&fab);
	  if (status != RMS$_NORMAL)
	    error ("RMS close on map file %s failed: %s",
		   into, strerror (EVMSERR, status));
	  return 0;
	}
    }
  status = sys$close (&fab);
  if (status != RMS$_NORMAL)
    {
      error ("RMS close on map file %s failed: %s",
	     into, strerror (EVMSERR, status));
      return 0;
    }
  return 1;
}

/* Write map data to disk using sys$write.
   Return 1 if ok, 0 if not.  */

static int
write_map (rab, m)
     struct RAB * rab;
     ondisk_map_info_t *m;
{
  int status;
  int cnt = 0;
  char *data = m->start;
  int length =  m->end - m->start + 1;
  int total = length;

  rab->rab$l_bkt = m->first;
  while (length > 0)
    {
      int chunk = length > MAXWRITE ? MAXWRITE : length;

      rab->rab$l_rbf = data;
      rab->rab$w_rsz = chunk;
      status = sys$write (rab, 0, 0);
      if (status != RMS$_NORMAL)
	{
	  fprintf (stderr, "RMS write to map file failed");
	  fprintf (stderr, " with status %%X%0X", status);
	  fprintf (stderr, " (data = %p,", (void *) data);
	  fprintf (stderr, " %d bytes chunk #%d,", MAXWRITE, cnt);
	  fprintf (stderr, " %d bytes out of", length);
	  fprintf (stderr, " %d left to write)\n", total);
	  lib$signal (status);
	  return 0;
	}
      data = &data[chunk];
      cnt++;
      length -= chunk ; /* MAXWRITE; */
      /* This zero does not indicate block-id 0; instead RMS takes it to mean
	 "consult the NBP (next block pointer)".  The equivalent would be to
	 advance it by "+= (MAXWRITE / BLOCKSIZE)" per definition of MAXWRITE.
	 Convenience or obscurity?  You be the judge!  */
      rab->rab$l_bkt = 0;
    }
  return 1;
}

/* vmsmap.c ends here */
