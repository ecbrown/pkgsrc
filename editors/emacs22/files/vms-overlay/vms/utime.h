/*
 * Copyright © 1994 the Free Software Foundation, Inc.
 *
 * Author: Richard Levitte <levitte@e.kth.se>
 *
 * This file is a part of GNUVMSLIB, the GNU library for porting GNU
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

#ifndef __UTIME_H
#define __UTIME_H

#include <sys/types.h>

struct utimbuf
{
  time_t atime;
  time_t mtime;
};

int utime (char *path, struct utimbuf *timep);

#endif /* not __UTIME_H */
