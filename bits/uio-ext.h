/* Operating system-specific extensions to sys/uio.h - generic version.
   Copyright (C) 2017-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _BITS_UIO_EXT_H
#define _BITS_UIO_EXT_H 1

#ifndef _SYS_UIO_H
# error "Never include <bits/uio-ext.h> directly; use <sys/uio.h> instead."
#endif

/* Flags for preadv2/pwritev2.  */
#define RWF_HIPRI	0x00000001 /* High priority request.  */
#define RWF_DSYNC	0x00000002 /* per-IO O_DSYNC.  */
#define RWF_SYNC	0x00000004 /* per-IO O_SYNC.  */
#define RWF_NOWAIT	0x00000008 /* per-IO nonblocking mode.  */
#define RWF_APPEND	0x00000010 /* per-IO O_APPEND.  */

#endif /* sys/uio_ext.h */
