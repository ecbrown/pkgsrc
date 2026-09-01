/* This file is no longer automatically generated.  */

#define _MALLOC_INTERNAL

/* The malloc headers and source files from the C library follow here.  */

/* Declarations for `malloc' and friends.
   Copyright 1990, 1991, 1992, 1993 Free Software Foundation, Inc.
		  Written May 1989 by Mike Haertel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef _MALLOC_H

#define _MALLOC_H	1

#ifdef _MALLOC_INTERNAL

#if defined(HAVE_CONFIG_H) || defined(emacs)
#include <config.h>
#endif

#if	defined(_LIBC) || defined(STDC_HEADERS) || defined(USG)
#include <string.h>
#else
#ifndef memset
#define	memset(s, zero, n)	bzero ((s), (n))
#endif
#ifndef memcpy
#define	memcpy(d, s, n)		bcopy ((s), (d), (n))
#endif
#endif

#if	defined(__GNU_LIBRARY__) || defined(__STDC__)
#include <limits.h>
#else
#define	CHAR_BIT	8
#endif

#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#endif

#endif	/* _MALLOC_INTERNAL.  */


#ifdef	__cplusplus
extern "C"
{
#endif

#if defined (__cplusplus) || (defined (__STDC__) && __STDC__)
#undef	__P
#define	__P(args)	args
#undef	__const
#define __const		const
#undef	__ptr_t
#define	__ptr_t		void *
#else /* Not C++ or ANSI C.  */
#undef	__P
#define	__P(args)	()
#undef	__const
#define	__const
#undef	__ptr_t
#define	__ptr_t		char *
#endif /* C++ or ANSI C.  */

#ifdef	__STDC__
#include <stddef.h>
#define	__malloc_size_t size_t
#else
typedef int ptrdiff_t;
#define __malloc_size_t unsigned int
#endif /* __STDC__ */

#ifndef	NULL
#define	NULL	0
#endif


/* Allocate SIZE bytes of memory.  */
extern __ptr_t malloc __P ((size_t __size));
/* Re-allocate the previously allocated block
   in __ptr_t, making the new block SIZE bytes long.  */
extern __ptr_t realloc __P ((__ptr_t __ptr, size_t __size));
/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern __ptr_t calloc __P ((size_t __nmemb, size_t __size));
/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free __P ((__ptr_t __ptr));

/* Allocate SIZE bytes allocated to ALIGNMENT bytes.  */
extern __ptr_t memalign __P ((size_t __alignment, size_t __size));

/* Allocate SIZE bytes on a page boundary.  */
extern __ptr_t valloc __P ((size_t __size));

#ifdef _MALLOC_INTERNAL

/* The allocator divides the heap into blocks of fixed size; large
   requests receive one or more whole blocks, and small requests
   receive a fragment of a block.  Fragment sizes are powers of two,
   and all fragments of a block are the same size.  When all the
   fragments in a block have been freed, the block itself is freed.  */
#define INT_BIT		(CHAR_BIT * sizeof(int))
#ifdef VMS
#define BLOCKLOG	12
#else /* !VMS */
#define BLOCKLOG	(INT_BIT > 16 ? 12 : 9)
#endif /* VMS */
#define BLOCKSIZE	(1 << BLOCKLOG)
#define BLOCKIFY(SIZE)	(((SIZE) + BLOCKSIZE - 1) / BLOCKSIZE)

/* Determine the amount of memory spanned by the initial heap table
   (not an absolute limit).  */
#define HEAP		(INT_BIT > 16 ? 4194304 : 65536)

/* Number of contiguous free blocks allowed to build up at the end of
   memory before they will be returned to the system.  */
#define FINAL_FREE_BLOCKS	8

/* Data structure giving per-block information.  */
typedef union
  {
    /* Heap information for a busy block.  */
    struct
      {
	/* Zero for a large block, or positive giving the
	   logarithm to the base two of the fragment size.  */
	int type;
	union
	  {
	    struct
	      {
		__malloc_size_t nfree; /* Free frags in a fragmented block.  */
		__malloc_size_t first; /* First free fragment of the block.  */
	      } frag;
	    /* Size (in blocks) of a large cluster.  */
	    __malloc_size_t size;
	  } info;
      } busy;
    /* Heap information for a free block
       (that may be the first of a free cluster).  */
    struct
      {
	__malloc_size_t size;	/* Size (in blocks) of a free cluster.  */
	__malloc_size_t next;	/* Index of next free cluster.  */
	__malloc_size_t prev;	/* Index of previous free cluster.  */
      } free;
  } malloc_info;

/* Pointer to first block of the heap.  */
extern char *_heapbase;

/* Table indexed by block number giving per-block information.  */
extern malloc_info *_heapinfo;

/* Address to block number and vice versa.  */
#define BLOCK(A)	(((char *) (A) - _heapbase) / BLOCKSIZE + 1)
#define ADDRESS(B)	((__ptr_t) (((B) - 1) * BLOCKSIZE + _heapbase))

/* Current search index for the heap table.  */
extern __malloc_size_t _heapindex;

/* Limit of valid info table indices.  */
extern __malloc_size_t _heaplimit;

/* Doubly linked lists of free fragments.  */
struct list
  {
    struct list *next;
    struct list *prev;
  };

/* Free list headers for each fragment size.  */
extern struct list _fraghead[BLOCKLOG];

/* List of blocks allocated with `memalign' (or `valloc').  */
struct alignlist
  {
    struct alignlist *next;
    __ptr_t aligned;		/* The address that memaligned returned.  */
    __ptr_t exact;		/* The address that malloc returned.  */
  };
extern struct alignlist *_aligned_blocks;

/* Instrumentation.  */
extern __malloc_size_t _chunks_used;
extern __malloc_size_t _bytes_used;
extern __malloc_size_t _chunks_free;
extern __malloc_size_t _bytes_free;

/* Internal version of `free' used in `morecore' (malloc.c). */
extern void _free_internal __P ((__ptr_t __ptr));

#endif /* _MALLOC_INTERNAL.  */

/* Underlying allocation function; successive calls should
   return contiguous pieces of memory.  */
/* It does NOT always return contiguous pieces of memory on VMS. */
extern __ptr_t (*__morecore) __P ((ptrdiff_t __size));

/* Underlying deallocation function. It accepts both a pointer and
   a size to back up. It is implementation dependent what is really
   used. */
extern __ptr_t (*__lesscore) __P ((__ptr_t __ptr, ptrdiff_t __size));

/* Default value of `__morecore'.  */
extern __ptr_t __default_morecore __P ((ptrdiff_t __size));

/* Default value of `__lesscore'.  */
extern __ptr_t __default_lesscore __P ((__ptr_t __ptr, ptrdiff_t __size));

/* If not NULL, this function is called after each time
   `__morecore' is called to increase the data size.  */
extern void (*__after_morecore_hook) __P ((void));

/* If not NULL, this function is called after each time
   `__lesscore' is called to increase the data size.  */
extern void (*__after_lesscore_hook) __P ((void));

/* Nonzero if `malloc' has been called and done its initialization.  */
extern int __malloc_initialized;

__malloc_size_t __malloc_extra_blocks; /* not used? */

/* Hooks for debugging versions.  */
extern void (*__free_hook) __P ((__ptr_t __ptr));
extern __ptr_t (*__malloc_hook) __P ((size_t __size));
extern __ptr_t (*__realloc_hook) __P ((__ptr_t __ptr, size_t __size));

/* Return values for `mprobe': these are the kinds of inconsistencies that
   `mcheck' enables detection of.  */
enum mcheck_status
  {
    MCHECK_DISABLED = -1,	/* Consistency checking is not turned on.  */
    MCHECK_OK,			/* Block is fine.  */
    MCHECK_FREE,		/* Block freed twice.  */
    MCHECK_HEAD,		/* Memory before the block was clobbered.  */
    MCHECK_TAIL			/* Memory after the block was clobbered.  */
  };

/* Activate a standard collection of debugging hooks.  This must be called
   before `malloc' is ever called.  ABORTFUNC is called with an error code
   (see enum above) when an inconsistency is detected.  If ABORTFUNC is
   null, the standard function prints on stderr and then calls `abort'.  */
extern int mcheck __P ((void (*__abortfunc) __P ((enum mcheck_status))));

/* Check for aberrations in a particular malloc'd block.  You must have
   called `mcheck' already.  These are the same checks that `mcheck' does
   when you free or reallocate a block.  */
extern enum mcheck_status mprobe __P ((__ptr_t __ptr));

/* Activate a standard collection of tracing hooks.  */
extern void mtrace __P ((void));
extern void muntrace __P ((void));

/* Statistics available to the user.  */
struct mstats
  {
    __malloc_size_t bytes_total; /* Total size of the heap. */
    __malloc_size_t chunks_used; /* Chunks allocated by the user. */
    __malloc_size_t bytes_used;	/* Byte total of user-allocated chunks. */
    __malloc_size_t chunks_free; /* Chunks in the free list. */
    __malloc_size_t bytes_free;	/* Byte total of chunks in the free list. */
  };

/* Pick up the current statistics. */
extern struct mstats mstats __P ((void));

/* Call WARNFUN with a warning message when memory usage is high.  */
extern void memory_warnings __P ((__ptr_t __start,
				  void (*__warnfun) __P ((const char *))));


/* Relocating allocator.  */

/* Allocate SIZE bytes, and store the address in *HANDLEPTR.  */
extern __ptr_t r_alloc __P ((__ptr_t *__handleptr, size_t __size));

/* Free the storage allocated in HANDLEPTR.  */
extern void r_alloc_free __P ((__ptr_t *__handleptr));

/* Adjust the block at HANDLEPTR to be SIZE bytes long.  */
extern __ptr_t r_re_alloc __P ((__ptr_t *__handleptr, size_t __size));


#ifdef	__cplusplus
}
#endif

#endif /* malloc.h  */
/* Allocate memory on a page boundary.
   Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#if defined (__GNU_LIBRARY__) || defined (_LIBC)
#include <stddef.h>
#include <sys/cdefs.h>
extern size_t __getpagesize __P ((void));
#else
#include "getpagesize.h"
#define	 __getpagesize()	getpagesize()
#endif

#ifndef	_MALLOC_INTERNAL
#define	_MALLOC_INTERNAL
#include <malloc.h>
#endif

static __malloc_size_t pagesize;

__ptr_t
valloc (size)
     __malloc_size_t size;
{
  if (pagesize == 0)
    pagesize = __getpagesize ();

  return memalign (pagesize, size);
}
/* Memory allocator `malloc'.
   Copyright 1990, 1991, 1992, 1993, 1994 Free Software Foundation
		  Written May 1989 by Mike Haertel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef	_MALLOC_INTERNAL
#define _MALLOC_INTERNAL
#include <malloc.h>
#endif

/* How to really get more memory.  */
__ptr_t (*__morecore) __P ((ptrdiff_t __size)) = __default_morecore;

/* Debugging hook for `malloc'.  */
__ptr_t (*__malloc_hook) __P ((__malloc_size_t __size));

/* Pointer to the base of the first block.  */
char *_heapbase;

/* Block information table.  Allocated with align/__free (not malloc/free).  */
malloc_info *_heapinfo;

/* Number of info entries.  */
static __malloc_size_t heapsize;

/* Search index in the info table.  */
__malloc_size_t _heapindex;

/* Limit of valid info table indices.  */
__malloc_size_t _heaplimit;

/* Free lists for each fragment size.  */
struct list _fraghead[BLOCKLOG];

/* Instrumentation.  */
__malloc_size_t _chunks_used;
__malloc_size_t _bytes_used;
__malloc_size_t _chunks_free;
__malloc_size_t _bytes_free;

/* Are you experienced?  */
int __malloc_initialized;

void (*__after_morecore_hook) __P ((void));
void (*__after_lesscore_hook) __P ((void));

/* Aligned allocation.  */
static __ptr_t align __P ((__malloc_size_t));
static __ptr_t
align (size)
     __malloc_size_t size;
{
  __ptr_t result;
  unsigned long int adj;

  result = (*__morecore) (size);
  adj = (unsigned long int) ((unsigned long int) ((char *) result -
						  (char *) NULL)) % BLOCKSIZE;
  if (adj != 0)
    {
      adj = BLOCKSIZE - adj;
      (void) (*__morecore) (adj);
      result = (char *) result + adj;
    }

  if (__after_morecore_hook)
    (*__after_morecore_hook) ();

  return result;
}

/* Set everything up and remember that we have.  */
static int initialize __P ((void));
static int
initialize ()
{
  heapsize = HEAP / BLOCKSIZE;
  _heapinfo = (malloc_info *) align (heapsize * sizeof (malloc_info));
  if (_heapinfo == NULL)
    return 0;
  memset (_heapinfo, 0, heapsize * sizeof (malloc_info));
  _heapinfo[0].free.size = 0;
  _heapinfo[0].free.next = _heapinfo[0].free.prev = 0;
  _heapindex = 0;
  _heapbase = (char *) _heapinfo;

  /* Account for the _heapinfo block itself in the statistics.  */
  _bytes_used = heapsize * sizeof (malloc_info);
  _chunks_used = 1;

  __malloc_initialized = 1;
  return 1;
}

/* Get neatly aligned memory, initializing or
   growing the heap info table as necessary. */
static __ptr_t morecore __P ((__malloc_size_t));
static __ptr_t
morecore (size)
     __malloc_size_t size;
{
  __ptr_t result;
  malloc_info *newinfo, *oldinfo;
  __malloc_size_t newsize;

  result = align (size);
  if (result == NULL)
    return NULL;

  /* Check if we need to grow the info table.  */
  if ((__malloc_size_t) BLOCK ((char *) result + size) > heapsize)
    {
      newsize = heapsize;
      while ((__malloc_size_t) BLOCK ((char *) result + size) > newsize)
	newsize *= 2;
      newinfo = (malloc_info *) align (newsize * sizeof (malloc_info));
      if (newinfo == NULL)
	{
	  (*__lesscore) (result, size);
	  return NULL;
	}
      memcpy (newinfo, _heapinfo, heapsize * sizeof (malloc_info));
      memset (&newinfo[heapsize], 0,
	      (newsize - heapsize) * sizeof (malloc_info));
      oldinfo = _heapinfo;
      newinfo[BLOCK (oldinfo)].busy.type = 0;
      newinfo[BLOCK (oldinfo)].busy.info.size
	= BLOCKIFY (heapsize * sizeof (malloc_info));
      _heapinfo = newinfo;
      /* Account for the _heapinfo block itself in the statistics.  */
      _bytes_used += newsize * sizeof (malloc_info);
      ++_chunks_used;
      _free_internal (oldinfo);
      heapsize = newsize;
    }

  _heaplimit = BLOCK ((char *) result + size);
  return result;
}

/* Allocate memory from the heap.  */
__ptr_t
malloc (size)
     __malloc_size_t size;
{
  __ptr_t result;
  __malloc_size_t block, blocks, lastblocks, start;
  register __malloc_size_t i;
  struct list *next;

  /* ANSI C allows `malloc (0)' to either return NULL, or to return a
     valid address you can realloc and free (though not dereference).

     It turns out that some extant code (sunrpc, at least Ultrix's version)
     expects `malloc (0)' to return non-NULL and breaks otherwise.
     Be compatible.  */

#if	0
  if (size == 0)
    return NULL;
#endif

  if (__malloc_hook != NULL)
    return (*__malloc_hook) (size);

  if (!__malloc_initialized)
    if (!initialize ())
      return NULL;

  if (size < sizeof (struct list))
    size = sizeof (struct list);

#ifdef SUNOS_LOCALTIME_BUG
  if (size < 16)
    size = 16;
#endif

  /* Determine the allocation policy based on the request size.  */
  if (size <= BLOCKSIZE / 2)
    {
      /* Small allocation to receive a fragment of a block.
	 Determine the logarithm to base two of the fragment size. */
      register __malloc_size_t log = 1;
      --size;
      while ((size /= 2) != 0)
	++log;
      
      /* Look in the fragment lists for a
	 free fragment of the desired size. */
      next = _fraghead[log].next;
      if (next != NULL)
	{
	  /* There are free fragments of this size.
	     Pop a fragment out of the fragment list and return it.
	     Update the block's nfree and first counters. */
	  result = (__ptr_t) next;
	  next->prev->next = next->next;
	  if (next->next != NULL)
	    next->next->prev = next->prev;
	  block = BLOCK (result);
	  if (--_heapinfo[block].busy.info.frag.nfree != 0)
	    _heapinfo[block].busy.info.frag.first = (unsigned long int)
	      ((unsigned long int) ((char *) next->next - (char *) NULL)
	       % BLOCKSIZE) >> log;

	  /* Update the statistics.  */
	  ++_chunks_used;
	  _bytes_used += 1 << log;
	  --_chunks_free;
	  _bytes_free -= 1 << log;
	}
      else
	{
	  /* No free fragments of the desired size, so get a new block
	     and break it into fragments, returning the first.  */
	  result = malloc (BLOCKSIZE);
	  if (result == NULL)
	    return NULL;

	  /* Link all fragments but the first into the free list.  */
	  for (i = 1; i < (__malloc_size_t) (BLOCKSIZE >> log); ++i)
	    {
	      next = (struct list *) ((char *) result + (i << log));
	      next->next = _fraghead[log].next;
	      next->prev = &_fraghead[log];
	      next->prev->next = next;
	      if (next->next != NULL)
		next->next->prev = next;
	    }

	  /* Initialize the nfree and first counters for this block.  */
	  block = BLOCK (result);
	  _heapinfo[block].busy.type = log;
	  _heapinfo[block].busy.info.frag.nfree = i - 1;
	  _heapinfo[block].busy.info.frag.first = i - 1;

	  _chunks_free += (BLOCKSIZE >> log) - 1;
	  _bytes_free += BLOCKSIZE - (1 << log);
	  _bytes_used -= BLOCKSIZE - (1 << log);
	}
    }
  else
    {
      /* Large allocation to receive one or more blocks.
	 Search the free list in a circle starting at the last place visited.
	 If we loop completely around without finding a large enough
	 space we will have to get more memory from the system.  */
      blocks = BLOCKIFY (size);
      start = block = _heapindex;
      while (_heapinfo[block].free.size < blocks)
	{
	  block = _heapinfo[block].free.next;
	  if (block == start)
	    {
	      /* Need to get more from the system.  Check to see if
		 the new core will be contiguous with the final free
		 block; if so we don't need to get as much.
		 However, it is possible for:     

		 (1) small amounts to be contiguous with the final free
		 block, and
		 (2) large amounts to be non-contiguous with the final
		 free block.

		 As a result, we have to check after we call morecore
		 to see where it went....  */
	      __ptr_t newcore;
	      block = _heapinfo[0].free.prev;
	      lastblocks = _heapinfo[block].free.size;
	      if (_heaplimit != 0 && block + lastblocks == _heaplimit &&
		  (*__morecore) (0) == ADDRESS (block + lastblocks) &&
		  ((newcore = morecore ((blocks - lastblocks) * BLOCKSIZE)))
		  != NULL)
		{
		  if (newcore != ADDRESS (block + lastblocks))
		    {
		      /* Oops!  It wasn't contiguous after all. */
		      /* This handles the case when _heapinfo moved
			 during the call to morecore () above.  The
			 only way is to make it an allocated block,
			 to immediatelly free it with _free_internal()  */
		      _heapinfo[BLOCK (newcore)].busy.type = 0;
		      _heapinfo[BLOCK (newcore)].busy.info.size
			= blocks - lastblocks;
		      ++_chunks_used;
		      _bytes_used += (blocks - lastblocks) * BLOCKSIZE;
		      _free_internal (newcore);
		      continue;
		    }
		  else
		    {
		      /* Which block we are extending (the `final free
			 block' referred to above) might have changed, if
			 it got combined with a freed info table.  */
		      block = _heapinfo[0].free.prev;
		      _heapinfo[block].free.size += (blocks - lastblocks);
		      _bytes_free += (blocks - lastblocks) * BLOCKSIZE;
		      continue;
		    }
		}
	      result = morecore (blocks * BLOCKSIZE);
	      if (result == NULL)
		return NULL;
	      block = BLOCK (result);
	      _heapinfo[block].busy.type = 0;
	      _heapinfo[block].busy.info.size = blocks;
	      ++_chunks_used;
	      _bytes_used += blocks * BLOCKSIZE;
	      return result;
	    }
	}

      /* At this point we have found a suitable free list entry.
	 Figure out how to remove what we need from the list. */
      result = ADDRESS (block);
      if (_heapinfo[block].free.size > blocks)
	{
	  /* The block we found has a bit left over,
	     so relink the tail end back into the free list. */
	  _heapinfo[block + blocks].free.size
	    = _heapinfo[block].free.size - blocks;
	  _heapinfo[block + blocks].free.next
	    = _heapinfo[block].free.next;
	  _heapinfo[block + blocks].free.prev
	    = _heapinfo[block].free.prev;
	  _heapinfo[_heapinfo[block].free.prev].free.next
	    = _heapinfo[_heapinfo[block].free.next].free.prev
	    = _heapindex = block + blocks;
	}
      else
	{
	  /* The block exactly matches our requirements,
	     so just remove it from the list. */
	  _heapinfo[_heapinfo[block].free.next].free.prev
	    = _heapinfo[block].free.prev;
	  _heapinfo[_heapinfo[block].free.prev].free.next
	    = _heapindex = _heapinfo[block].free.next;
	  --_chunks_free;
	}

      _heapinfo[block].busy.type = 0;
      _heapinfo[block].busy.info.size = blocks;
      ++_chunks_used;
      _bytes_used += blocks * BLOCKSIZE;
      _bytes_free -= blocks * BLOCKSIZE;
    }

  return result;
}

#ifndef _LIBC

/* On some ANSI C systems, some libc functions call _malloc, _free
   and _realloc.  Make them use the GNU functions.  */

__ptr_t
_malloc (size)
     __malloc_size_t size;
{
  return malloc (size);
}

void
_free (ptr)
     __ptr_t ptr;
{
  free (ptr);
}

__ptr_t
_realloc (ptr, size)
     __ptr_t ptr;
     __malloc_size_t size;
{
  return realloc (ptr, size);
}

#endif
/* Free a block of memory allocated by `malloc'.
   Copyright 1990, 1991, 1992, 1994 Free Software Foundation
		  Written May 1989 by Mike Haertel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef	_MALLOC_INTERNAL
#define _MALLOC_INTERNAL
#include <malloc.h>
#endif

/* How to really get more memory.  */
__ptr_t (*__lesscore) __P ((__ptr_t __ptr,ptrdiff_t __size)) = __default_lesscore;

/* Debugging hook for free.  */
void (*__free_hook) __P ((__ptr_t __ptr));

/* List of blocks allocated by memalign.  */
struct alignlist *_aligned_blocks = NULL;

/* Return memory to the heap.
   Like `free' but don't call a __free_hook if there is one.  */
#include <stdio.h>
void
_free_internal (ptr)
     __ptr_t ptr;
{
  int type;
  __malloc_size_t block, blocks;
  register __malloc_size_t i;
  struct list *prev, *next;

  block = BLOCK (ptr);

#ifdef VMS

  {
    extern __ptr_t last_vms_brk_current_in_fake;
    extern __ptr_t first_vms_brk_current_in_real;

    if ((last_vms_brk_current_in_fake !=0
	 && ptr > last_vms_brk_current_in_fake
	 && (first_vms_brk_current_in_real == 0
	     || ptr < first_vms_brk_current_in_real))
	|| block > heapsize)
      {
	extern void abort __P ((void));
	/* Damn DEC C... */
	extern int fprintf (FILE *__stream, const char *__format, ...);

	fprintf (stderr, "Trying to free memory outside my area");
	abort ();
      }
  }
#endif

  type = _heapinfo[block].busy.type;
  switch (type)
    {
    case 0:
      /* Get as many statistics as early as we can.  */
      --_chunks_used;
      _bytes_used -= _heapinfo[block].busy.info.size * BLOCKSIZE;
      _bytes_free += _heapinfo[block].busy.info.size * BLOCKSIZE;

      /* Find the free cluster previous to this one in the free list.
	 Start searching at the last block referenced; this may benefit
	 programs with locality of allocation.  */
      i = _heapindex;
      if (i > block)
	while (i > block)
	  i = _heapinfo[i].free.prev;
      else
	{
	  do
	    i = _heapinfo[i].free.next;
	  while (i > 0 && i < block);
	  i = _heapinfo[i].free.prev;
	}

      /* Determine how to link this block into the free list.  */
      if (block == i + _heapinfo[i].free.size)
	{
	  /* Coalesce this block with its predecessor.  */
	  _heapinfo[i].free.size += _heapinfo[block].busy.info.size;
	  block = i;
	}
      else
	{
	  /* Really link this block back into the free list.  */
	  _heapinfo[block].free.size = _heapinfo[block].busy.info.size;
	  _heapinfo[block].free.next = _heapinfo[i].free.next;
	  _heapinfo[block].free.prev = i;
	  _heapinfo[i].free.next = block;
	  _heapinfo[_heapinfo[block].free.next].free.prev = block;
	  ++_chunks_free;
	}

      /* Now that the block is linked in, see if we can coalesce it
	 with its successor (by deleting its successor from the list
	 and adding in its size).  */
      if (block + _heapinfo[block].free.size == _heapinfo[block].free.next)
	{
	  _heapinfo[block].free.size
	    += _heapinfo[_heapinfo[block].free.next].free.size;
	  _heapinfo[block].free.next
	    = _heapinfo[_heapinfo[block].free.next].free.next;
	  _heapinfo[_heapinfo[block].free.next].free.prev = block;
	  --_chunks_free;
	}

      /* Now see if we can return stuff to the system.  */
      blocks = _heapinfo[block].free.size;
      if (blocks >= FINAL_FREE_BLOCKS && block + blocks == _heaplimit
	  && (*__morecore) (0) == ADDRESS (block + blocks))
	{
	  register __malloc_size_t bytes = blocks * BLOCKSIZE;
	  _heaplimit -= blocks;
	  (*__lesscore) (ADDRESS(block), bytes);
	  _heapinfo[_heapinfo[block].free.prev].free.next
	    = _heapinfo[block].free.next;
	  _heapinfo[_heapinfo[block].free.next].free.prev
	    = _heapinfo[block].free.prev;
	  block = _heapinfo[block].free.prev;
	  --_chunks_free;
	  _bytes_free -= bytes;
	}

      /* Set the next search to begin at this block.  */
      _heapindex = block;
      break;

    default:
      /* Do some of the statistics.  */
      --_chunks_used;
      _bytes_used -= 1 << type;
      ++_chunks_free;
      _bytes_free += 1 << type;

      /* Get the address of the first free fragment in this block.  */
      prev = (struct list *) ((char *) ADDRESS (block) +
			   (_heapinfo[block].busy.info.frag.first << type));

      if (_heapinfo[block].busy.info.frag.nfree == (BLOCKSIZE >> type) - 1)
	{
	  /* If all fragments of this block are free, remove them
	     from the fragment list and free the whole block.  */
	  next = prev;
	  for (i = 1; i < (__malloc_size_t) (BLOCKSIZE >> type); ++i)
	    next = next->next;
	  prev->prev->next = next;
	  if (next != NULL)
	    next->prev = prev->prev;
	  _heapinfo[block].busy.type = 0;
	  _heapinfo[block].busy.info.size = 1;

	  /* Keep the statistics accurate.  */
	  ++_chunks_used;
	  _bytes_used += BLOCKSIZE;
	  _chunks_free -= BLOCKSIZE >> type;
	  _bytes_free -= BLOCKSIZE;

	  free (ADDRESS (block));
	}
      else if (_heapinfo[block].busy.info.frag.nfree != 0)
	{
	  /* If some fragments of this block are free, link this
	     fragment into the fragment list after the first free
	     fragment of this block. */
	  next = (struct list *) ptr;
	  next->next = prev->next;
	  next->prev = prev;
	  prev->next = next;
	  if (next->next != NULL)
	    next->next->prev = next;
	  ++_heapinfo[block].busy.info.frag.nfree;
	}
      else
	{
	  /* No fragments of this block are free, so link this
	     fragment into the fragment list and announce that
	     it is the first free fragment of this block. */
	  prev = (struct list *) ptr;
	  _heapinfo[block].busy.info.frag.nfree = 1;
	  _heapinfo[block].busy.info.frag.first = (unsigned long int)
	    ((unsigned long int) ((char *) ptr - (char *) NULL)
	     % BLOCKSIZE >> type);
	  prev->next = _fraghead[type].next;
	  prev->prev = &_fraghead[type];
	  prev->prev->next = prev;
	  if (prev->next != NULL)
	    prev->next->prev = prev;
	}
      break;
    }
}

/* Return memory to the heap.  */
void
free (ptr)
     __ptr_t ptr;
{
  register struct alignlist *l;

  if (ptr == NULL)
    return;

  for (l = _aligned_blocks; l != NULL; l = l->next)
    if (l->aligned == ptr)
      {
	l->aligned = NULL;	/* Mark the slot in the list as free.  */
	ptr = l->exact;
	break;
      }

  if (__free_hook != NULL)
    (*__free_hook) (ptr);
  else
    _free_internal (ptr);
}
/* Copyright (C) 1991, 1993, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_MALLOC_INTERNAL
#define _MALLOC_INTERNAL
#include <malloc.h>
#endif

#ifdef _LIBC

#include <ansidecl.h>
#include <gnu-stabs.h>

#undef	cfree

function_alias(cfree, free, void, (ptr),
	       DEFUN(cfree, (ptr), PTR ptr))

#else

void
cfree (ptr)
     __ptr_t ptr;
{
  free (ptr);
}

#endif
/* Change the size of a block allocated by `malloc'.
   Copyright 1990, 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
		     Written May 1989 by Mike Haertel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef	_MALLOC_INTERNAL
#define _MALLOC_INTERNAL
#include <malloc.h>
#endif

#if  (defined (MEMMOVE_MISSING) || !defined(_LIBC) && !defined(STDC_HEADERS) && !defined(USG))

/* Snarfed directly from Emacs src/dispnew.c:
   XXX Should use system bcopy if it handles overlap.  */
#ifndef emacs

/* Like bcopy except never gets confused by overlap.  */

static void
safe_bcopy (from, to, size)
     char *from, *to;
     int size;
{
  if (size <= 0 || from == to)
    return;

  /* If the source and destination don't overlap, then bcopy can
     handle it.  If they do overlap, but the destination is lower in
     memory than the source, we'll assume bcopy can handle that.  */
  if (to < from || from + size <= to)
    bcopy (from, to, size);

  /* Otherwise, we'll copy from the end.  */
  else
    {
      register char *endf = from + size;
      register char *endt = to + size;

      /* If TO - FROM is large, then we should break the copy into
	 nonoverlapping chunks of TO - FROM bytes each.  However, if
	 TO - FROM is small, then the bcopy function call overhead
	 makes this not worth it.  The crossover point could be about
	 anywhere.  Since I don't think the obvious copy loop is too
	 bad, I'm trying to err in its favor.  */
      if (to - from < 64)
	{
	  do
	    *--endt = *--endf;
	  while (endf != from);
	}
      else
	{
	  for (;;)
	    {
	      endt -= (to - from);
	      endf -= (to - from);

	      if (endt < to)
		break;

	      bcopy (endf, endt, to - from);
	    }

	  /* If SIZE wasn't a multiple of TO - FROM, there will be a
	     little left over.  The amount left over is
	     (endt + (to - from)) - to, which is endt - from.  */
	  bcopy (from, to, endt - from);
	}
    }
}
#endif	/* Not emacs.  */

#define memmove(to, from, size) safe_bcopy ((from), (to), (size))

#endif


#define min(A, B) ((A) < (B) ? (A) : (B))

/* Debugging hook for realloc.  */
__ptr_t (*__realloc_hook) __P ((__ptr_t __ptr, __malloc_size_t __size));

/* Resize the given region to the new size, returning a pointer
   to the (possibly moved) region.  This is optimized for speed;
   some benchmarks seem to indicate that greater compactness is
   achieved by unconditionally allocating and copying to a
   new region.  This module has incestuous knowledge of the
   internals of both free and malloc. */
__ptr_t
realloc (ptr, size)
     __ptr_t ptr;
     __malloc_size_t size;
{
  __ptr_t result;
  int type;
  __malloc_size_t block, blocks, oldlimit;

  if (size == 0)
    {
      free (ptr);
      return malloc (0);
    }
  else if (ptr == NULL)
    return malloc (size);

  if (__realloc_hook != NULL)
    return (*__realloc_hook) (ptr, size);

  block = BLOCK (ptr);

  type = _heapinfo[block].busy.type;
  switch (type)
    {
    case 0:
      /* Maybe reallocate a large block to a small fragment.  */
      if (size <= BLOCKSIZE / 2)
	{
	  result = malloc (size);
	  if (result != NULL)
	    {
	      memcpy (result, ptr, size);
	      _free_internal (ptr);
	      return result;
	    }
	}

      /* The new size is a large allocation as well;
	 see if we can hold it in place. */
      blocks = BLOCKIFY (size);
      if (blocks < _heapinfo[block].busy.info.size)
	{
	  /* The new size is smaller; return
	     excess memory to the free list. */
	  _heapinfo[block + blocks].busy.type = 0;
	  _heapinfo[block + blocks].busy.info.size
	    = _heapinfo[block].busy.info.size - blocks;
	  _heapinfo[block].busy.info.size = blocks;
	  /* We have just created a new chunk by splitting a chunk in two.
	     Now we will free this chunk; increment the statistics counter
	     so it doesn't become wrong when _free_internal decrements it.  */
	  ++_chunks_used;
	  _free_internal (ADDRESS (block + blocks));
	  result = ptr;
	}
      else if (blocks == _heapinfo[block].busy.info.size)
	/* No size change necessary.  */
	result = ptr;
      else
	{
	  /* Won't fit, so allocate a new region that will.
	     Free the old region first in case there is sufficient
	     adjacent free space to grow without moving. */
	  blocks = _heapinfo[block].busy.info.size;
	  /* Prevent free from actually returning memory to the system.  */
	  oldlimit = _heaplimit;
	  _heaplimit = 0;
	  _free_internal (ptr);
	  _heaplimit = oldlimit;
	  result = malloc (size);
	  if (result == NULL)
	    {
	      /* Now we're really in trouble.  We have to unfree
		 the thing we just freed.  Unfortunately it might
		 have been coalesced with its neighbors.  */
	      if (_heapindex == block)
	        (void) malloc (blocks * BLOCKSIZE);
	      else
		{
		  __ptr_t previous = malloc ((block - _heapindex) * BLOCKSIZE);
		  (void) malloc (blocks * BLOCKSIZE);
		  _free_internal (previous);
		}
	      return NULL;
	    }
	  if (ptr != result)
	    memmove (result, ptr, blocks * BLOCKSIZE);
	}
      break;

    default:
      /* Old size is a fragment; type is logarithm
	 to base two of the fragment size.  */
      if (size > (__malloc_size_t) (1 << (type - 1)) &&
	  size <= (__malloc_size_t) (1 << type))
	/* The new size is the same kind of fragment.  */
	result = ptr;
      else
	{
	  /* The new size is different; allocate a new space,
	     and copy the lesser of the new size and the old. */
	  result = malloc (size);
	  if (result == NULL)
	    return NULL;
	  memcpy (result, ptr, min (size, (__malloc_size_t) 1 << type));
	  free (ptr);
	}
      break;
    }

  return result;
}
/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef	_MALLOC_INTERNAL
#define	_MALLOC_INTERNAL
#include <malloc.h>
#endif

/* Allocate an array of NMEMB elements each SIZE bytes long.
   The entire array is initialized to zeros.  */
__ptr_t
calloc (nmemb, size)
     register __malloc_size_t nmemb;
     register __malloc_size_t size;
{
  register __ptr_t result = malloc (nmemb * size);

  if (result != NULL)
    (void) memset (result, 0, nmemb * size);

  return result;
}
/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef	_MALLOC_INTERNAL
#define	_MALLOC_INTERNAL
#include <malloc.h>
#endif

#ifndef	__GNU_LIBRARY__
#define	__sbrk	sbrk
#ifdef VMS
#define __brk brk
#endif
#endif

#ifdef __GNU_LIBRARY__
/* It is best not to declare this and cast its result on foreign operating
   systems with potentially hostile include files.  */
extern __ptr_t __sbrk __P ((int increment));
#endif

#ifndef NULL
#define NULL 0
#endif

#ifdef VMS
//#define DEBUG_VMSGMALLOC
/* We use sys$expreg and friends, so we need the starlet definitions.  */

#include <starlet.h>

/* Unfortunatelly, the VAX C sbrk() is buggy. For example, it returns
   memory in 512 byte chunks (not a bug, but there's more), AND it
   adds an extra 512 byte chunk if you ask for a multiple of 512
   bytes (you ask for 512 bytes, you get 1024 bytes...). And also,
   the VAX C sbrk does not handle negative increments...
   There's a similar problem with brk(). Even if you set the break
   to an even page boundary, it gives you one extra page... */

/* -1 if this is the first time, otherwise
   bit 0 set if 'increment' needs adjustment
   bit 1 set if the value to brk() needs adjustment.  */
static signed char vms_brk_info_fetched = -1;

static __ptr_t vms_brk_start = 0;
static __ptr_t vms_brk_end = 0;
static __ptr_t vms_brk_current = 0;

static __ptr_t vms_real_heap_start = 0;
static __ptr_t vms_fake_heap_end = 0;
/* SOME DEBUGGING VARS */
__ptr_t last_vms_brk_current_in_fake = 0;
__ptr_t first_vms_brk_current_in_real = 0;
/* THAT'S IT! */

#define VMSGMALLOC_LOWLIM
#ifdef VMSGMALLOC_LOWLIM
static __ptr_t vms_brk_lowlim = 0; /* We may not deallocate memory below this
				    address.  */
#endif
#endif

/* Allocate INCREMENT more bytes of data space,
   and return the start of data space, or NULL on errors.
   If INCREMENT is negative, shrink data space.  */
__ptr_t
__default_morecore (increment)
#ifdef __STDC__
     ptrdiff_t increment;
#else
     int increment;
#endif
{
  __ptr_t result;
#ifdef VMS
  __ptr_t temp;
  ptrdiff_t increment_page_bytes;
  __ptr_t save_vms_brk_current = vms_brk_current;

  if (vms_brk_info_fetched < 0)
    {
      /* We know that on VMS (Open or not), __sbrk returns chunks
	 with the size being a multiple of the local pagesize.
	 So the code below effectivelly gives us one page.  */
      vms_real_heap_start = (__ptr_t) __sbrk (1);
      //      fprintf (stderr, "  vms_real_heap_start starts at 0x%X\n", vms_real_heap_start);
      if (vms_brk_start == 0)
	vms_brk_current = vms_brk_start = vms_real_heap_start;
      else
	vms_fake_heap_end = vms_brk_end;
      pagesize = ((char *) __sbrk (0) - (char *) vms_real_heap_start);

      /* At least with VAX C, __sbrk does an off-by-one error.
	 So when you ask for exactly one page, you get two.  Newer
	 implementations, or DEC C, might have corrected that "error".
	 We store our knowledge about it in vms_brk_info_fetched.  */
      temp = (__ptr_t) __sbrk (pagesize);
      vms_brk_end = (__ptr_t) __sbrk (0);
      if ( ((char*)vms_brk_end - (char*)temp) == 2*pagesize)
	vms_brk_info_fetched = 1;
      else
	vms_brk_info_fetched = 0;

      /* At least on VAX C, if you point exactly at a page boundary,
	 the break will really be set at the next page boundary.
	 Knowledge about this is also stored in vms_brk_info_fetched.  */
      vms_brk_end = __brk ((unsigned long int)vms_real_heap_start);
      if (vms_brk_end != vms_real_heap_start)
	vms_brk_info_fetched |= 2;
    }

  increment_page_bytes  = (increment + pagesize - 1) & ~(pagesize - 1);

#ifdef DEBUG_VMSGMALLOC
      fprintf (stderr, "$ morecore: increment = 0x%X, increment_page_bytes = 0x%X\n",
	       increment, increment_page_bytes);
#endif

  if (increment < 0)
    return NULL;

  if (increment > 0)
    {
      unsigned long status;
      __ptr_t tmpadr[2][2];
#ifdef DEBUG_VMSGMALLOC
      fprintf (stderr, "$ vms_brk_current = 0x%p, vms_brk_end = 0x%p\n",
	       vms_brk_current, vms_brk_end);
#endif
      result = vms_brk_current;

      temp = (__ptr_t) (((unsigned long)vms_brk_current + pagesize - 1)
			& ~(pagesize - 1));
      tmpadr[0][0] = temp;
      tmpadr[0][1] = (char*)temp + increment_page_bytes - 1;
      tmpadr[1][0] = tmpadr[1][1] = 0;

#ifdef DEBUG_VMSGMALLOC
      fprintf (stderr, "  preliminary setting tmpadr[0:1][0:1] to:\n");
      fprintf (stderr, "    {{ %p, %p },\n", tmpadr[0][0], tmpadr[0][1]);
      fprintf (stderr, "     { %p, %p }}\n", tmpadr[1][0], tmpadr[1][1]);
#endif

      if (tmpadr[0][1] < vms_fake_heap_end)
	{
#ifdef DEBUG_VMSGMALLOC
	  fprintf (stderr, "  tmpadr[0][1] < vms_fake_heap_end {%p}, so:\n",
		   vms_fake_heap_end);
#endif
	  tmpadr[1][0] = tmpadr[0][0];
	  tmpadr[1][1] = tmpadr[0][1];
#ifdef DEBUG_VMSGMALLOC
	  fprintf (stderr, "    {{ %p, %p },\n", tmpadr[0][0], tmpadr[0][1]);
	  fprintf (stderr, "     { %p, %p }}\n", tmpadr[1][0], tmpadr[1][1]);
#endif
	}
      else
	{
	  if (tmpadr[0][1] >= vms_fake_heap_end
	      && tmpadr[0][0] < vms_real_heap_start)
	    {
#ifdef DEBUG_VMSGMALLOC
	      fprintf (stderr, "  tmpadr[0][1] >= vms_fake_heap_end {%p}",
		       vms_fake_heap_end);
	      fprintf (stderr,
		       "  and tmpadr[0][0] < vms_real_heap_start {%p}, so:\n",
		       vms_real_heap_start);
#endif
	      tmpadr[0][0] = __sbrk (0);
	      //	      fprintf(stderr, "!  __sbrk(0) gave me %p.\n", tmpadr[0][0]);
	      /* This seems incredible, but I've seen this happen...  RL  */
              if (tmpadr[0][0] < vms_real_heap_start)
                {
		  //		  extern void abort __P ((void));
		  //                  fprintf (stderr, "!  __sbrk(0) gave me %p, which is in the middle of the code!\n", tmpadr[0][0]);
		  // This happens when the sharable images that Emacs is linked against change
		  // (e.g. OS upgrade, TCP/IP stack upgrade, etc.)
		  // The best I can do here is to reset vms_real_heap_start to be the new value.
		  
		  //lib$signal(1132);
		  vms_real_heap_start = tmpadr[0][0];
		}
	      tmpadr[0][1] = (char*)tmpadr[0][0] + increment_page_bytes - 1;
#ifdef DEBUG_VMSGMALLOC
	      fprintf (stderr, "    {{ %p, %p },\n",
		       tmpadr[0][0], tmpadr[0][1]);
	      fprintf (stderr, "     { %p, %p }}\n",
		       tmpadr[1][0], tmpadr[1][1]);
#endif
	    }
	  if (tmpadr[0][1] < vms_brk_end
	      || (__ptr_t) __sbrk (0) == vms_brk_end)
	    {
	      status = sys$cretva (tmpadr[0], tmpadr[1], 0);

#ifdef DEBUG_VMSGMALLOC
	      fprintf (stderr, "  sys$cretva ({0x%p, 0x%p}, >{0x%p, 0x%p}, 0) -> 0x%X\n",
		       tmpadr[0][0], tmpadr[0][1], tmpadr[1][0], tmpadr[1][1],
		       status);
#endif
	      /* The following are the possibilities:

		 tmpadr[1][0] == vms_brk_current,
		 tmpadr[1][1] >= vms_brk_current+i

		 In this case, we're done.

		 tmpadr[1][0] > vms_brk_current,
		 tmpadr[1][1] == vms_brk_current+i

		 In this case, use expreg IF vms_brk_current+i == vms_brk_end

		 tmpadr[1][0] == vms_brk_current,
		 tmpadr[1][1] < vms_brk_current+i
		 tmpadr[1][0] > vms_brk_current,
		 tmpadr[1][1] < vms_brk_current+i

		 In these cases, use expreg.  */
	     
	      if (tmpadr[1][0] == (void*)-1
		  || (char*)tmpadr[1][1] - (char*)tmpadr[1][0] + 1 < increment_page_bytes)
		{
		  status = sys$deltva (tmpadr[1], 0, 0);
#ifdef DEBUG_VMSGMALLOC
		  fprintf (stderr, "  sys$deltva ({0x%p, 0x%p}, 0) -> 0x%X\n",
			   tmpadr[1][0], tmpadr[1][1], status);
#endif

		  if ((char*)tmpadr[1][1] + 1 < (char*)temp + increment_page_bytes)
		    tmpadr[1][0] = tmpadr[1][1] = 0;
		  else
		    {
		      tmpadr[0][0] = tmpadr[1][0];
		      tmpadr[0][1] = (char*)tmpadr[0][0] + increment_page_bytes - 1;
		      status = sys$cretva (tmpadr[0], tmpadr[1], 0);
#ifdef DEBUG_VMSGMALLOC
		      fprintf (stderr, "  sys$cretva ({0x%p, 0x%p}, >{0x%p, 0x%p}, 0) -> 0x%X\n",
			       tmpadr[0][0], tmpadr[0][1],
			       tmpadr[1][0], tmpadr[1][1],
			       status);
#endif
		      if (tmpadr[1][0] == (void*)-1
			  || tmpadr[0][0] != tmpadr[1][0]
			  || tmpadr[0][1] != tmpadr[1][1])
			{
			  status = sys$deltva (tmpadr[1], 0, 0);
#ifdef DEBUG_VMSGMALLOC
			  fprintf (stderr, "  sys$deltva ({0x%p, 0x%p}, 0) -> 0x%X\n",
				   tmpadr[1][0], tmpadr[1][1], status);
#endif
			  tmpadr[1][0] = tmpadr[1][1] = 0;
			}
		    }
		}
	    }
	}
      if (tmpadr[1][1] == 0)
	{
	  status = sys$expreg ((increment + 511)/512, tmpadr[1], 0, 0);
	  result = tmpadr[1][0];
#ifdef DEBUG_VMSGMALLOC
	  fprintf (stderr, "+ sys$expreg (%d pagelets (%d bytes), expanded the region with {0x%p, 0x%p}, 0, 0) -> 0x%X\n",
		   (increment + 511)/512, increment+511, tmpadr[1][0], tmpadr[1][1], status);
#endif
	}
      if (tmpadr[1][0] == (void*)-1
	  || (char*)tmpadr[1][1] - (char*)tmpadr[1][0] + 1 < increment)
	return NULL;
      if (result >= vms_fake_heap_end && result < vms_real_heap_start)
	result = tmpadr[1][0];
      vms_brk_current = (char*)result + increment;
      if ((char *) vms_brk_end < (char *) tmpadr[1][1] + 1)
	vms_brk_end = (__ptr_t) ((char *) tmpadr[1][1] + 1);
#ifdef DEBUG_VMSGMALLOC
      fprintf (stderr, "  result = 0x%p\n", result);
      fprintf (stderr, "  vms_brk_current = 0x%p, vms_brk_end = 0x%p\n",
	       vms_brk_current, vms_brk_end);
#endif
      if (save_vms_brk_current <= vms_fake_heap_end
	  && result >= vms_real_heap_start)
	{
	  last_vms_brk_current_in_fake = save_vms_brk_current;
	  first_vms_brk_current_in_real = result;
	}
      return result;
    }

  /* OK, so the user wanted to check where the heap limit is. Let's
     see if the system thinks it is where we think it is. */
  temp = vms_brk_current;
#ifdef DEBUG_VMSGMALLOC
  fprintf (stderr, "$ ");
#endif
  if (temp >= vms_real_heap_start && temp == vms_brk_end)
    {
      temp = (__ptr_t) __sbrk (0);
#ifdef DEBUG_VMSGMALLOC
      fprintf (stderr, "__sbrk(0) -> 0x%p\n$ ", temp);
#endif
      if (temp != vms_brk_end)
	{
	  /* the value has changed.
	     Let's trust the system and modify our value */
	  vms_brk_current = vms_brk_end = temp;
	}
    }
#ifdef DEBUG_VMSGMALLOC
  fprintf (stderr, "vms_brk_current = 0x%p, vms_brk_end = 0x%p\n",
	   vms_brk_current, vms_brk_end);
#endif
  if (save_vms_brk_current <= vms_fake_heap_end
      && vms_brk_current >= vms_real_heap_start)
    {
      last_vms_brk_current_in_fake = save_vms_brk_current;
      first_vms_brk_current_in_real = vms_brk_current;
    }
  return vms_brk_current;

#else /* not VMS */
  result = (__ptr_t) __sbrk ((int) increment);
  if (result == (__ptr_t) -1)
    return NULL;
  return result;
#endif /* VMS */
}

__ptr_t
__default_lesscore (ptr, size)
     __ptr_t ptr;
     ptrdiff_t size;
{
#ifdef VMS
  if (vms_brk_end != 0)
    {
      __ptr_t temp = (__ptr_t) sbrk (0);
      vms_brk_current = ptr;
      if (vms_brk_current < vms_brk_start)
	vms_brk_current = vms_brk_start;
#ifdef DEBUG_VMSGMALLOC
      fprintf (stderr, "* vms_brk_current = 0x%p, vms_brk_end = 0x%p, sbrk(0) = 0x%p, ptr = 0x%p\n",
	       vms_brk_current, vms_brk_end, temp, ptr);
#endif
      if (vms_brk_end == temp || temp == vms_real_heap_start)
	{
	  vms_brk_end = (__ptr_t) (((unsigned long) vms_brk_current + pagesize - 1)
				  & ~(pagesize - 1));
#ifdef VMSGMALLOC_LOWLIM
	  if (vms_brk_end < vms_brk_lowlim)
	    vms_brk_end = vms_brk_lowlim;
	  else
#endif
	    vms_brk_end = (char*)vms_brk_end - ((vms_brk_info_fetched >> 1) & 1);
#ifdef DEBUG_VMSGMALLOC
	  fprintf (stderr, "  end of memory moved from 0x%p to ", temp);
#endif
	  if (vms_brk_end >= vms_real_heap_start)
	    vms_brk_end = __brk ((unsigned long int)vms_brk_end);
#ifdef DEBUG_VMSGMALLOC
	  fprintf (stderr, "  0x%p (should be %p).\n", sbrk (0),
		   vms_brk_end);
#endif
	}
      else
	{
	  /* It may be dangerous to try to deallocate below this value,
	     so set the limit for deallocation.  */
#ifdef DEBUG_VMSGMALLOC
	  fprintf (stderr, "  setting vms_brk_current = 0x%X, vms_brk_end = 0x%p\n", vms_brk_current, vms_brk_end);
#endif
	}
    }

  return vms_brk_current;
#else /* not VMS */
  __default_morecore (-size);
#endif
}

#ifdef VMS
#if defined(emacs) && defined(VMS)
/* Dumping of Emacs on VMS does not include the heap!
   So let's make a huge array from which initial data will be
   allocated.

   VMS_ALLOCATION_SIZE is the amount of memory we preallocate.
   We don't want it to be too large, because it only gives a larger
   dump file. The way to check how much is really used is to
   make VMS_ALLOCATION_SIZE very large, to link Emacs with the
   debugger, run Emacs, check how much was allocated. Then set
   VMS_ALLOCATION_SIZE to something suitable, recompile gmalloc,
   relink Emacs, and you should be off.

   N.B. This is experimental, but it worked quite fine on Emacs 18.
*/
#ifndef VMS_ALLOCATION_SIZE
/* Emacs 22's preload set exceeds the 2.34 MiB inherited from the Emacs 21
   port.  Leave room for both the terminal and DECwindows dumps, and keep
   the reservation aligned to the 8 KiB OpenVMS page size.  */
#define VMS_ALLOCATION_SIZE 4194304
#endif

// #define USE_INITIAL_BUFFER
#ifdef USE_INITIAL_BUFFER
#ifdef __GNUC__
char initial_buffer[VMS_ALLOCATION_SIZE] asm("_$PsectAttributes_NOOVR$$HEAPBUFFER") = "";
#else
globaldef {"HEAPBUFFER"} char initial_buffer[VMS_ALLOCATION_SIZE] = "";
#endif
#endif

__ptr_t initial_vms_brk_start = 0;
__ptr_t initial_vms_brk_end = 0;

void
__vms_allocate_initial_buffer ()
{
#ifdef USE_INITIAL_BUFFER
  vms_brk_start = initial_buffer;
  vms_brk_end = initial_buffer + VMS_ALLOCATION_SIZE;
#else
  __default_morecore (VMS_ALLOCATION_SIZE);
#endif
  vms_brk_current = vms_brk_start;
  initial_vms_brk_start = vms_brk_start;
  initial_vms_brk_end = vms_brk_end;
#ifdef VMSGMALLOC_LOWLIM
  vms_brk_lowlim = vms_brk_end;
#endif
  #ifdef DEBUG_VMSGMALLOC
  fprintf (stderr, "Initial buffer spans %p - %p\n",
	   initial_vms_brk_start,
	   initial_vms_brk_end);
  #endif
}

int
vms_malloc_overflow ()
{
  return
    initial_vms_brk_start != vms_brk_start
#if 0  /* Incredibly stupid of me.  Other parts of the system may have
	  allocated things on the heap, out of my control...  RL  */
    || initial_vms_brk_end != vms_brk_end;
#else  /* Instead, I should have done the following from the very beginning
	  since that's all I know *I* have allocated, and that's all
	  I need to save...  RL  */
    || initial_vms_brk_end <= vms_brk_current;
#endif
}

__ptr_t
start_of_initial_brk_area ()
{
  return initial_vms_brk_start;
}

__ptr_t
end_of_initial_brk_area ()
{
  return (char*)initial_vms_brk_end - 1;
}
#endif /* defined(emacs) && defined(VMS) */

__ptr_t
start_of_brk_area ()
{
  return vms_brk_start;
}

__ptr_t
current_end_of_brk_area ()
{
  return (char*)vms_brk_current - 1;
}

__ptr_t
end_of_brk_area ()
{
  return (char*)vms_brk_end - 1;
}

__ptr_t
start_of_real_heap ()
{
  return vms_real_heap_start;
}

#ifdef DEBUG_VMSGMALLOC
#undef DEBUG_VMSGMALLOC
#endif
#endif
/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_MALLOC_INTERNAL
#define _MALLOC_INTERNAL
#include <malloc.h>
#endif

__ptr_t
memalign (alignment, size)
     __malloc_size_t alignment;
     __malloc_size_t size;
{
  __ptr_t result;
  unsigned long int adj;

  size = ((size + alignment - 1) / alignment) * alignment;

  result = malloc (size);
  if (result == NULL)
    return NULL;
  adj = (unsigned long int) ((unsigned long int) ((char *) result -
						  (char *) NULL)) % alignment;
  if (adj != 0)
    {
      struct alignlist *l;
      for (l = _aligned_blocks; l != NULL; l = l->next)
	if (l->aligned == NULL)
	  /* This slot is free.  Use it.  */
	  break;
      if (l == NULL)
	{
	  l = (struct alignlist *) malloc (sizeof (struct alignlist));
	  if (l == NULL)
	    {
	      free (result);
	      return NULL;
	    }
	  l->next = _aligned_blocks;
	  _aligned_blocks = l;
	}
      l->exact = result;
      result = l->aligned = (char*) result + alignment - adj;
    }

  return result;
}
/* Standard debugging hooks for `malloc'.
   Copyright 1990, 1991, 1992 Free Software Foundation
   Written May 1989 by Mike Haertel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef	_MALLOC_INTERNAL
#define	_MALLOC_INTERNAL
#include <malloc.h>
#endif

/* Old hook values.  */
static void (*old_free_hook) __P ((__ptr_t ptr));
static __ptr_t (*old_malloc_hook) __P ((size_t size));
static __ptr_t (*old_realloc_hook) __P ((__ptr_t ptr, size_t size));

/* Function to call when something awful happens.  */
/* static void (*abortfunc) __P ((void)); */
static void (*abortfunc) __P ((enum mcheck_status));

/* Arbitrary magical numbers.  */
#define MAGICWORD	0xfedabeeb
#define MAGICBYTE	((char) 0xd7)

struct hdr
  {
    size_t size;		/* Exact size requested by user.  */
    unsigned long int magic;	/* Magic number to check header integrity.  */
  };

static void checkhdr __P ((__const struct hdr *));
static void
checkhdr (hdr)
     __const struct hdr *hdr;
{
  if (hdr->magic != MAGICWORD || ((char *) &hdr[1])[hdr->size] != MAGICBYTE)
    (*abortfunc) (MCHECK_HEAD);
}

static void freehook __P ((__ptr_t));
static void
freehook (ptr)
     __ptr_t ptr;
{
  struct hdr *hdr = ((struct hdr *) ptr) - 1;
  checkhdr (hdr);
  hdr->magic = 0;
  __free_hook = old_free_hook;
  free (hdr);
  __free_hook = freehook;
}

static __ptr_t mallochook __P ((size_t));
static __ptr_t
mallochook (size)
     size_t size;
{
  struct hdr *hdr;

  __malloc_hook = old_malloc_hook;
  hdr = (struct hdr *) malloc (sizeof (struct hdr) + size + 1);
  __malloc_hook = mallochook;
  if (hdr == NULL)
    return NULL;

  hdr->size = size;
  hdr->magic = MAGICWORD;
  ((char *) &hdr[1])[size] = MAGICBYTE;
  return (__ptr_t) (hdr + 1);
}

static __ptr_t reallochook __P ((__ptr_t, size_t));
static __ptr_t
reallochook (ptr, size)
     __ptr_t ptr;
     size_t size;
{
  struct hdr *hdr = ((struct hdr *) ptr) - 1;

  checkhdr (hdr);
  __free_hook = old_free_hook;
  __malloc_hook = old_malloc_hook;
  __realloc_hook = old_realloc_hook;
  hdr = (struct hdr *) realloc ((__ptr_t) hdr, sizeof (struct hdr) + size + 1);
  __free_hook = freehook;
  __malloc_hook = mallochook;
  __realloc_hook = reallochook;
  if (hdr == NULL)
    return NULL;

  hdr->size = size;
  ((char *) &hdr[1])[size] = MAGICBYTE;
  return (__ptr_t) (hdr + 1);
}

static void
mabort (status)
     enum mcheck_status status;
{
  const char *msg;
  switch (status)
    {
    case MCHECK_OK:
      msg = "memory is consistent, library is buggy";
      break;
    case MCHECK_HEAD:
      msg = "memory clobbered before allocated block";
      break;
    case MCHECK_TAIL:
      msg = "memory clobbered past end of allocated block";
      break;
    case MCHECK_FREE:
      msg = "block freed twice";
      break;
    default:
      msg = "bogus mcheck_status, library is buggy";
      break;
    }
#ifdef __GNU_LIBRARY__
  __libc_fatal (msg);
#else
  fprintf (stderr, "mcheck: %s\n", msg);
  fflush (stderr);
  abort ();
#endif
}

int
mcheck (func)
     void (*func) __P ((enum mcheck_status));
{
  static int mcheck_used = 0;

  abortfunc = (func != NULL) ? func : &mabort;

  /* These hooks may not be safely inserted if malloc is already in use.  */
  if (!__malloc_initialized && !mcheck_used)
    {
      old_free_hook = __free_hook;
      __free_hook = freehook;
      old_malloc_hook = __malloc_hook;
      __malloc_hook = mallochook;
      old_realloc_hook = __realloc_hook;
      __realloc_hook = reallochook;
      mcheck_used = 1;
    }

  return mcheck_used ? 0 : -1;
}
/* More debugging hooks for `malloc'.
   Copyright (C) 1991, 1992 Free Software Foundation, Inc.
		 Written April 2, 1991 by John Gilmore of Cygnus Support.
		 Based on mcheck.c by Mike Haertel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef	_MALLOC_INTERNAL
#define	_MALLOC_INTERNAL
#include <malloc.h>
#endif

#include <stdio.h>

#ifndef	__GNU_LIBRARY__
extern char *getenv ();
#else
#include <stdlib.h>
#endif

static FILE *mallstream;
static char mallenv[]= "MALLOC_TRACE";
static char mallbuf[BUFSIZ];	/* Buffer for the output.  */

/* Address to breakpoint on accesses to... */
__ptr_t mallwatch;

/* Old hook values.  */
static __ptr_t (*tr_old_morecore) __P ((ptrdiff_t increment));
static __ptr_t (*tr_old_lesscore) __P ((__ptr_t ptr, ptrdiff_t increment));
static void (*tr_old_free_hook) __P ((__ptr_t ptr));
static __ptr_t (*tr_old_malloc_hook) __P ((size_t size));
static __ptr_t (*tr_old_realloc_hook) __P ((__ptr_t ptr, size_t size));

/* This function is called when the block being alloc'd, realloc'd, or
   freed has an address matching the variable "mallwatch".  In a debugger,
   set "mallwatch" to the address of interest, then put a breakpoint on
   tr_break.  */

void tr_break __P ((void));
void
tr_break ()
{
}

static void tr_freehook __P ((__ptr_t));
static void
tr_freehook (ptr)
     __ptr_t ptr;
{
  fprintf (mallstream, "- %p\n", ptr);	/* Be sure to print it first.  */
  if (ptr == mallwatch)
    tr_break ();
  __free_hook = tr_old_free_hook;
  free (ptr);
  __free_hook = tr_freehook;
}

static __ptr_t tr_morecore __P ((ptrdiff_t));
static __ptr_t
tr_morecore (increment)
     ptrdiff_t increment;
{
  __ptr_t p;

  __morecore = tr_old_morecore;
  p = (__ptr_t) (*__morecore) (increment);
  __morecore = tr_morecore;

#ifdef VMS
  fprintf (mallstream, "$ %p %d (%p, %p) (%p, %p)\n", p, increment,
	   vms_fake_heap_end, vms_real_heap_start,
	   last_vms_brk_current_in_fake, first_vms_brk_current_in_real);
#else
  fprintf (mallstream, "$ %p %d\n", p, increment);
#endif

  return p;
}

static __ptr_t tr_lesscore __P ((__ptr_t, ptrdiff_t));
static __ptr_t
tr_lesscore (ptr, increment)
     __ptr_t ptr;
     ptrdiff_t increment;
{
  __ptr_t p;

  __lesscore = tr_old_lesscore;
  p = (__ptr_t) (*__lesscore) (ptr, increment);
  __lesscore = tr_lesscore;

  fprintf (mallstream, "* %p (%p, %d)\n", p, ptr, increment);

  return p;
}

static __ptr_t tr_mallochook __P ((size_t));
static __ptr_t
tr_mallochook (size)
     size_t size;
{
  __ptr_t hdr;

  __malloc_hook = tr_old_malloc_hook;
  hdr = (__ptr_t) malloc (size);
  __malloc_hook = tr_mallochook;

  /* We could be printing a NULL here; that's OK.  */
  fprintf (mallstream, "+ %p %d\n", hdr, size);

  if (hdr == mallwatch)
    tr_break ();

  return hdr;
}

static __ptr_t tr_reallochook __P ((__ptr_t, size_t));
static __ptr_t
tr_reallochook (ptr, size)
     __ptr_t ptr;
     size_t size;
{
  __ptr_t hdr;

  if (ptr == mallwatch)
    tr_break ();

  __free_hook = tr_old_free_hook;
  __malloc_hook = tr_old_malloc_hook;
  __realloc_hook = tr_old_realloc_hook;
  hdr = (__ptr_t) realloc (ptr, size);
  __free_hook = tr_freehook;
  __malloc_hook = tr_mallochook;
  __realloc_hook = tr_reallochook;
  if (hdr == NULL)
    /* Failed realloc.  */
    fprintf (mallstream, "! %p %d\n", ptr, size);
  else
    fprintf (mallstream, "< %p\n> %p %d\n", ptr, hdr, size);

  if (hdr == mallwatch)
    tr_break ();

  return hdr;
}

/* We enable tracing if either the environment variable MALLOC_TRACE
   is set, or if the variable mallwatch has been patched to an address
   that the debugging user wants us to stop on.  When patching mallwatch,
   don't forget to set a breakpoint on tr_break!  */

void
mtrace ()
{
  char *mallfile;

  mallfile = getenv (mallenv);
  if (mallfile != NULL || mallwatch != NULL)
    {
      mallstream = fopen (mallfile != NULL ? mallfile : "/dev/null", "w");
      if (mallstream != NULL)
	{
	  /* Be sure it doesn't malloc its buffer!  */
	  setbuf (mallstream, mallbuf);
	  fprintf (mallstream, "= Start\n");
	  tr_old_morecore = __morecore;
	  __morecore = tr_morecore;
	  tr_old_lesscore = __lesscore;
	  __lesscore = tr_lesscore;
	  tr_old_free_hook = __free_hook;
	  __free_hook = tr_freehook;
	  tr_old_malloc_hook = __malloc_hook;
	  __malloc_hook = tr_mallochook;
	  tr_old_realloc_hook = __realloc_hook;
	  __realloc_hook = tr_reallochook;
	}
    }
}
/* Access the statistics maintained by `malloc'.
   Copyright 1990, 1991, 1992 Free Software Foundation
		  Written May 1989 by Mike Haertel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef	_MALLOC_INTERNAL
#define _MALLOC_INTERNAL
#include <malloc.h>
#endif

struct mstats
mstats ()
{
  struct mstats result;

  result.bytes_total = (char *) (*__morecore) (0) - _heapbase;
  result.chunks_used = _chunks_used;
  result.bytes_used = _bytes_used;
  result.chunks_free = _chunks_free;
  result.bytes_free = _bytes_free;
  return result;
}

/* VMS specialities */

#ifdef VMS
void malloc_clear_hooks ()
{
  __free_hook = 0;
  __malloc_hook = 0;
  __realloc_hook = 0;
  __after_morecore_hook = 0;
  __after_lesscore_hook = 0;

  __morecore = __default_morecore;
  __lesscore = __default_lesscore;
#ifdef DEBUG_VMSGMALLOC
#ifdef sbkr
#undef sbrk
#endif
  printf ("sbrk(0) returns %p\n", sbrk (0));
  printf ("vms_fake_heap_end = %p, vms_real_heap_start = %p\n",
	  vms_fake_heap_end, vms_real_heap_start);
#endif
}
  
#if 0
void print_map ()
{
  unsigned long block;
  struct list ourfraghead[BLOCKLOG] = _fraghead;
  printf ("_heaplimit = %d\n", _heaplimit);
  printf ("_heapbase = %p\n", _heapbase);
  printf ("_heapinfo = %p\n", _heapinfo);

  block = 0  /* ; block < _heaplimit; block++) */
  do
    {
    } while 1;
}
#endif

void print_brk ()
{
#ifdef sbrk
#undef sbrk
#endif
  printf ("sbrk (0) = %p\n", sbrk (0));
}
#endif /* VMS */
