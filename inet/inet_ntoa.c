/* Convert Inet number to ASCII representation.
   Copyright (C) 1997-2025 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

/* The interface of this function is completely stupid, it requires a
   static buffer.  We relax this a bit in that we allow one buffer for
   each thread.  */
static __thread char buffer[INET_ADDRSTRLEN];


char *
inet_ntoa (struct in_addr in)
{
  __inet_ntop (AF_INET, &in, buffer, sizeof buffer);
  return buffer;
}
