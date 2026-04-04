/* Optimized version of the standard strncmp() function.
   This file is part of the GNU C Library.
   Copyright (C) 2000-2024 Free Software Foundation, Inc.
   Copyright (C) 2024-2026 EPIC Linux project

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

/* Return: the result of the comparison

   Inputs:
        in0:    s1
        in1:    s2
        in2:    n

   Unlike memcmp(), this function is optimized for mismatches within the
   first few characters.

   Partially unrolled pipelined loop is used to get ~1 cycle/character for
   long strings, together with fast return path for early mismatch. */

#include <sysdep.h>
#undef ret

#define sx1		in0
#define s2		in1
#define n		in2

#define valx1		r15
#define valx2		r16
#define valy1		r17
#define valy2		r18
#define sy1		r19
#define tmp		r20
#define sntl		r21
#define one		r22

ENTRY(strncmp)
	alloc	r2 = ar.pfs, 3, 0, 0, 0
	adds	tmp = 1, sx1		// pre-compute sx1 + 1 for both sentinel (sx1 + n + 1) and sy1 (sx1 + 1)
	mov	one = 1
	mov	ret0 = r0
	cmp.eq	p7, p0 = n, r0		// return immediately if n == 0
(p7)	br.ret.spnt.many b0
	;;
	// prologue
	ld1	valx1 = [sx1], 2
	ld1	valx2 = [s2], 1
	cmp.eq	p6, p0 = r0, r0		// set p6
	adds	sy1 = 0, tmp		// sy1  = tmp     = sx1 + 1
	add	sntl = n, tmp		// sntl = tmp + n = sx1 + n + 1
	cmp.eq	p7, p0 = n, one
	;;
	ld1.s	valy1 = [sy1], 2
	ld1.s	valy2 = [s2], 1
	sub	ret0 = valx1, valx2
	cmp.eq.or p7, p0 = valx1, r0
	cmp.ne.or p7, p0 = valx1, valx2
(p7)	br.ret.sptk.many b0		// do early return if neq, zero or n == 1
.balign 32
.loop1:
	ld1.s	valx1 = [sx1], 2
	ld1.s	valx2 = [s2], 1
	cmp.ne.and p6, p0 = valy1, r0
	cmp.ne.and p6, p0 = sy1, sntl
	cmp.eq.and p6, p0 = valy1, valy2
(p6)	br.cond.sptk .loop2
	;;
	sub	ret0 = valy1, valy2
	chk.s	valy1, .faulty
	chk.s	valy2, .faulty
	br.ret.sptk.many b0
.balign 32
.loop2:
	ld1.s	valy1 = [sy1], 2
	ld1.s	valy2 = [s2], 1
	cmp.ne.and p6, p0 = valx1, r0
	cmp.ne.and p6, p0 = sx1, sntl
	cmp.eq.and p6, p0 = valx1, valx2
(p6)	br.cond.sptk .loop1
	;;
	sub	ret0 = valx1, valx2
	chk.s	valx1, .faultx
	chk.s	valx2, .faultx
	br.ret.sptk.many b0
.faultx:
	adds	sx1 = -2, sx1
	adds	sy1 = -2, sy1
	adds	s2 = -2, s2
	;;
	ld1	valx1 = [sx1], 2
	ld1	valx2 = [s2], 1
	cmp.eq	p6, p0 = r0, r0		// set p6
	;;
	br.sptk	.loop2
.faulty:
	adds	sx1 = -2, sx1
	adds	sy1 = -2, sy1
	adds	s2 = -2, s2
	;;
	ld1	valy1 = [sy1], 2
	ld1	valy2 = [s2], 1
	cmp.eq	p6, p0 = r0, r0		// set p6
	;;
	br.sptk	.loop1
END(strncmp)
libc_hidden_builtin_def (strncmp)
