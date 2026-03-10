/* Optimized version of the standard strcmp() function.
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

   Unlike memcmp(), this function is optimized for mismatches within the
   first few characters.

   Partially unrolled pipelined loop is used to get ~1 cycle/character for
   long strings, together with fast return path for early mismatch. */

#include <sysdep.h>
#undef ret

#define s1		in0
#define s2		in1

#define valx1		r15
#define valx2		r16
#define valy1		r17
#define valy2		r18

#define valyf		r19

ENTRY(strcmp)
	alloc	r2 = ar.pfs, 2, 0, 0, 0
	cmp.ne	p6, p7 = r0, r0		// set p6, p7
	// do initial load, nothing to compare yet
	ld1	valx1 = [s1], 1
	ld1	valx2 = [s2], 1
	;;
	// compare first pair / speculatively load second pair
	ld1.s	valy1 = [s1], 1
	ld1.s	valy2 = [s2], 1
	sub	ret0 = valx1, valx2	// prepare early return
	cmp.eq.or p6, p0 = valx1, r0
	cmp.ne.or p6, p0 = valx1, valx2
(p6)	br.ret.sptk.many b0		// do early return if neq or zero
	;;
.loop:	// main loop body
	ld1.s	valx1 = [s1], 1
	ld1.s	valx2 = [s2], 1
	add	valyf = valy1, valy2	// store possible NaT
	cmp.eq.or p6, p0 = valy1, r0
	cmp.ne.or p6, p0 = valy1, valy2
(p6)	br.dptk.many .loopendy
	;;
.cont:
	ld1.s	valy1 = [s1], 1
	ld1.s	valy2 = [s2], 1
	chk.s	valyf, .faulty1		// handle valy1 != 0 and (valy1 or valy2 = NaT)
	cmp.ne.and p7, p0 = valx1, r0
	cmp.eq.and p7, p0 = valx1, valx2
(p7)	br.dpnt.many .loop
	;;
	sub	ret0 = valx1, valx2
	chk.s	valx1, .faultx
	chk.s	valx2, .faultx
	br.ret.sptk.many b0
.loopendy:
	sub	ret0 = valy1, valy2
	chk.s	valyf, .faulty2		// handle valy1 = 0 and valy2 = NaT
	br.ret.sptk.many b0
.faultx:
	adds	s1 = -2, s1
	adds	s2 = -2, s2
	;;
	ld1	valx1 = [s1], 1		// do non-speculative load
	ld1	valx2 = [s2], 1
	cmp.ne	p6, p7 = r0, r0		// set p6, p7
	;;
	br.sptk.many	.cont		// neither load failed, continue
.faulty1:
	adds	s1 = -1, s1
	adds	s2 = -1, s2
	;;
.faulty2:
	adds	s1 = -2, s1
	adds	s2 = -2, s2
	;;
	ld1	valy1 = [s1], 1		// do non-speculative load
	ld1	valy2 = [s2], 1
	cmp.ne	p6, p7 = r0, r0		// set p6, p7
	;;
	br.sptk.many	.loop		// neither load failed, continue
END(strcmp)
libc_hidden_builtin_def (strcmp)
