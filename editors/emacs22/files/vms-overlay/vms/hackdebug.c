/*
 * Copyright (C) 1994 the Free Software Foundation, Inc.
 *
 * Author: Richard Levitte <levitte@e.kth.se>
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

static char hackdebug_version[] = "VMS hackdebug version 1.1";

#ifdef __DECC
#define close decc$close
#define lseek decc$lseek
#define open decc$open
#if __DECC_VER < 50200000
/* For some reason, this one IS declared in stdio.h with DEC C >5.0 */
#define read decc$read
#endif
#define write decc$write
#endif

/* Now this is probably one of the dirtiest hacks you've ever seen */

/* For now, this is a VERY ugly hack, with everything hardcoded...
   What more do we need? */

#include <unixio.h>
#include <file.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef __DECC
#if __VMS_VER < 70000000
/* Of course, with VMS <7.0, read() is STILL not in the library! */
#define read decc$read
#endif
#endif

#define FLAGS (O_RDWR|O_EXCL)
#define MODE (0755)

#define BLOCK_SIZE (512)

#ifdef VAX
#define IMAGE_OFFSET (0x20)
#define mask (~0x01)
#define value (0x01)
#else
#define IMAGE_OFFSET (0x50)
#define mask (~0x01)
#define value (0x01)
#endif

static char buffer[BLOCK_SIZE];
static char original[BLOCK_SIZE];

static int
read_block (fd, destination, length)
     int fd;
     char *destination;
     int length;
{
  int offset = 0;

  while (offset < length)
    {
      int count = read (fd, destination + offset, length - offset);
      if (count < 0)
	{
	  if (errno == EINTR)
	    continue;
	  return -1;
	}
      if (count == 0)
	{
	  errno = EIO;
	  return -1;
	}
      offset += count;
    }
  return 0;
}

static int
write_block (fd, source, length)
     int fd;
     const char *source;
     int length;
{
  int offset = 0;

  while (offset < length)
    {
      int count = write (fd, source + offset, length - offset);
      if (count < 0)
	{
	  if (errno == EINTR)
	    continue;
	  return -1;
	}
      if (count == 0)
	{
	  errno = EIO;
	  return -1;
	}
      offset += count;
    }
  return 0;
}

int main (argc, argv)
     int argc;
     char *argv[];
{
#define IMAGE argv[1]
  int fd;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: hackdebug image\n");
      return 4;
    }

  fd = open (IMAGE, FLAGS, MODE);

  if (fd == -1)
    {
      perror ("Could not open TEMACS.EXE");
      return 4; /* This is fatal */
    }

  if (read_block (fd, buffer, BLOCK_SIZE) != 0)
    {
      perror ("Could not read TEMACS.EXE image header");
      close (fd);
      return 4;
    }
  memcpy (original, buffer, sizeof buffer);

  buffer[IMAGE_OFFSET] &= mask;

  if (lseek (fd, 0, SEEK_SET) != 0
      || write_block (fd, buffer, BLOCK_SIZE) != 0)
    {
      int saved_errno = errno;

      /* A short write can leave an unusable image.  Restore the header on a
         best-effort basis before reporting the failed build step.  */
      if (lseek (fd, 0, SEEK_SET) == 0)
	(void) write_block (fd, original, BLOCK_SIZE);
      (void) close (fd);
      errno = saved_errno;
      perror ("Could not update TEMACS.EXE image header");
      return 4;
    }

  if (close (fd) != 0)
    {
      perror ("Could not close TEMACS.EXE");
      return 4;
    }

  return 1;
}
