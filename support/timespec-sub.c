/* Subtract two struct timespec values.
   Copyright (C) 2011-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library and is also part of gnulib.
   Patches to this file should be submitted to both projects.

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

/* Return the difference between two timespec values A and B.  On
   overflow, return an extremal value.  This assumes 0 <= tv_nsec <
   TIMESPEC_HZ.  */

#include "timespec.h"

#include "intprops.h"

struct timespec
timespec_sub (struct timespec a, struct timespec b)
{
  time_t rs = a.tv_sec;
  time_t bs = b.tv_sec;
  int ns = a.tv_nsec - b.tv_nsec;
  int rns = ns;

  if (ns < 0)
    {
      rns = ns + TIMESPEC_HZ;
      time_t bs1;
      if (!INT_ADD_WRAPV (bs, 1, &bs1))
        bs = bs1;
      else if (- TYPE_SIGNED (time_t) < rs)
        rs--;
      else
        goto low_overflow;
    }

  if (INT_SUBTRACT_WRAPV (rs, bs, &rs))
    {
      if (0 < bs)
        {
        low_overflow:
          rs = TYPE_MINIMUM (time_t);
          rns = 0;
        }
      else
        {
          rs = TYPE_MAXIMUM (time_t);
          rns = TIMESPEC_HZ - 1;
        }
    }

  return (struct timespec) { .tv_sec = rs, .tv_nsec = rns };
}
