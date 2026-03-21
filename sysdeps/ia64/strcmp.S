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
        in0:    sx1
        in1:    sx2

   Unlike memcmp(), this function is optimized for mismatches within the
   first few characters.

   Partially unrolled pipelined loop is used to get ~1 cycle/character for
   long strings, together with fast return path for early mismatch. */

#include <sysdep.h>
#undef ret

// registers used in prologue and initial loop
#define align1		r21
#define align2		r22
#define cnt1		r23
#define cnt2		r24

// registers used in initial and final loop
#define sx1		in0
#define sx2		in1
#define sy1		r29
#define sy2		r30

// registers used in kernel loop
#define sk1		r21	/* same as align1 */
#define sk2		r22	/* same as align2 */
#define valz1		r23	/* same as cnt1 */
#define valz2		r24	/* same as cnt2 */
#define zindx		r25
#define zindy		r26
#define zindz		r27
#define zind0		r28

// registers used in all loops
#define valx1		r15
#define valx2		r16
#define valy1		r17
#define valy2		r18

ENTRY(strcmp)
// -- Prologue: figure out alignment ---
	alloc	r2 = ar.pfs, 2, 0, 0, 0
	and	align1 = 0x7, sx1	// sx1 % 8
	and	align2 = 0x7, sx2	// sx2 % 8
	adds	cnt1 = 10, sx1
	adds	cnt2 = 10, sx2
	adds	sy1 = 1, sx1		// set sy1 for counting to compare with cnt sentinel
	;;
	// do initial load, relevant for both branches
	ld1	valx1 = [sx1], 2
	ld1	valx2 = [sx2], 2
	adds	sy2 = -9, cnt2		// set remaining sy2, use cnt2 to avoid conflict
	cmp.eq	p6, p7 = r0, r0		// set p6, p7
	// check for alignment here
	cmp.ne	p4, p0 = align1, align2
(p4)	br.spnt.many	.final		// misaligned, jump directly to final loop
	;;
// --- Compare head one by one until strings align ---
	// compare first pair / speculatively load second pair
	// note: init never ends here, so we can skip cnt1 check
	ld1.s	valy1 = [sy1], 2
	ld1.s	valy2 = [sy2], 2
	sub	ret0 = valx1, valx2	// prepare early return
	cmp.eq.or p7, p0 = valx1, r0
	cmp.ne.or p7, p0 = valx1, valx2
(p7)	br.ret.sptk.many b0		// do early return if neq or zero
	;;
	// set cnt1 and cnt2 sentinels
	sub	cnt1 = cnt1, align1	// cntx = sx1 + (8 - align1) + 2
	sub	cnt2 = cnt2, align2	// cnty = sx2 + (8 - align2) + 2
	cmp.eq	p4, p0 = r0, r0		// set p4 for final loop
	;;
.balign 32
.initloop1:	// main loop body
	ld1.s	valx1 = [sx1], 2
	ld1.s	valx2 = [sx2], 2
	cmp.ne.and p6, p0 = valy1, r0
	cmp.eq.and p6, p0 = valy1, valy2
	cmp.ne.and p6, p0 = sy1, cnt1	// check for end of initial loop
(p6)	br.sptk .initloop2
	;;
	sub	ret0 = valy1, valy2
	cmp.eq p4, p5 = sy1, cnt1
(p4)	br.sptk.many	.kernel
	chk.s	valy1, .initfaulty
	chk.s	valy2, .initfaulty
(p5)	br.ret.sptk.many b0
.balign 32
.initloop2:
	ld1.s	valy1 = [sy1], 2
	ld1.s	valy2 = [sy2], 2
	cmp.ne.and p6, p0 = valx1, r0
	cmp.eq.and p6, p0 = valx1, valx2
	cmp.ne.and p6, p0 = sx1, cnt1	// check for end of initial loop
(p6)	br.sptk .initloop1
	;;
	sub	ret0 = valx1, valx2
	cmp.eq p4, p5 = sx1, cnt1
(p4)	br.sptk	.kernel
	chk.s	valx1, .initfaultx
	chk.s	valx2, .initfaultx
(p5)	br.ret.sptk.many b0
.initfaultx:
	adds	sx1 = -2, sx1
	adds	sx2 = -2, sx2
	adds	sy1 = -2, sy1
	adds	sy2 = -2, sy2		// rewind
	;;
	ld1	valx1 = [sx1], 2	// do non-speculative load
	ld1	valx2 = [sx2], 2
	cmp.eq	p6, p0 = r0, r0		// set p6
	;;
	br.sptk	.initloop2	// neither load failed, continue
.initfaulty:
	adds	sx1 = -2, sx1
	adds	sx2 = -2, sx2
	adds	sy1 = -2, sy1
	adds	sy2 = -2, sy2		// rewind
	;;
	ld1	valy1 = [sx1], 2	// do non-speculative load
	ld1	valy2 = [sx2], 2
	cmp.eq	p6, p0 = r0, r0		// set p6
	;;
	br.sptk	.initloop1	// neither load failed, continue
// --- Kernel: compare 8 characters at once ---
.kernel:
	// rewind
	adds	sk1 = -2, cnt1
	adds	sk2 = -2, cnt2
	cmp.eq	p6, p7 = r0, r0		// set p6, p7
	;;
	// first prologue, only load values
	ld8	valx1 = [sk1], 8
	ld8	valx2 = [sk2], 8
	mov	zind0 = 8		// default value of czx1.l not found
	;;
	// second prologue, load and zero index
	ld8.s	valy1 = [sk1], 8
	ld8.s	valy2 = [sk2], 8
	czx1.l	zindx = valx1
	;;
.balign 32
.kernelloop1:
	ld8.s	valz1 = [sk1], 8
	ld8.s	valz2 = [sk2], 8
	czx1.l	zindy = valy1
	cmp.eq.and p6, p0 = zindx, zind0
	cmp.eq.and p6, p0 = valx1, valx2
(p6)	br.sptk .kernelloop2
	;;
	mov	valz1 = valx1
	mov	valz2 = valx2
	br.sptk	.exitpath
	;;
.balign 32
.kernelloop2:
	ld8.s	valx1 = [sk1], 8
	ld8.s	valx2 = [sk2], 8
	czx1.l	zindz = valz1
	cmp.eq.and p6, p0 = zindy, zind0
	cmp.eq.and p6, p0 = valy1, valy2
(p6)	br.sptk .kernelloop3
	;;
	mov	valz1 = valy1
	mov	valz2 = valy2
	br.sptk .exitpath
	;;
.balign 32
.kernelloop3:
	ld8.s	valy1 = [sk1], 8
	ld8.s	valy2 = [sk2], 8
	czx1.l	zindx = valx1
	cmp.eq.and p6, p0 = zindz, zind0
	cmp.eq.and p6, p0 = valz1, valz2
(p6)	br.sptk .kernelloop1
	;;
.balign 32
.exitpath:
	// exit path
	adds	sx1 = -22, sk1
	adds	sx2 = -22, sk2
	adds	sy1 = -23, sk1
	adds	sy2 = -23, sk2
	extr.u	valx1 = valz1, 0, 8	// extract first character (little endian)
	extr.u	valx2 = valz2, 0, 8
	;;
// --- Compare tail of string one by one ---
.balign 32
.final:
	// compare first pair / speculatively load second pair
	ld1.s	valy1 = [sy1], 2
	ld1.s	valy2 = [sy2], 2
	sub	ret0 = valx1, valx2	// prepare early return
	cmp.eq.or p7, p0 = valx1, r0
	cmp.ne.or p7, p0 = valx1, valx2
(p7)	br.ret.sptk.many b0		// do early return if neq or zero
	;;
.balign 32
.finalloop1:	// main loop body
	ld1.s	valx1 = [sx1], 2
	ld1.s	valx2 = [sx2], 2
	cmp.ne.and p4, p0 = valy1, r0
	cmp.eq.and p4, p0 = valy1, valy2
(p4)	br.sptk .finalloop2
	;;
	sub	ret0 = valy1, valy2
	chk.s	valy1, .finalfaulty
	chk.s	valy2, .finalfaulty
	br.ret.sptk.many b0
.balign 32
.finalloop2:
	ld1.s	valy1 = [sy1], 2
	ld1.s	valy2 = [sy2], 2
	cmp.ne.and p4, p0 = valx1, r0
	cmp.eq.and p4, p0 = valx1, valx2
(p4)	br.sptk .finalloop1
	;;
	sub	ret0 = valx1, valx2
	chk.s	valx1, .finalfaultx
	chk.s	valx2, .finalfaultx
	br.ret.sptk.many b0
.finalfaultx:
	adds	sx1 = -2, sx1
	adds	sx2 = -2, sx2
	adds	sy1 = -2, sy1
	adds	sy2 = -2, sy2		// rewind
	;;
	ld1	valx1 = [sx1], 2	// do non-speculative load
	ld1	valx2 = [sx2], 2
	cmp.eq	p4, p0 = r0, r0		// set p4
	;;
	br.sptk.many	.finalloop2	// neither load failed, continue
.finalfaulty:
	adds	sx1 = -2, sx1
	adds	sx2 = -2, sx2
	adds	sy1 = -2, sy1
	adds	sy2 = -2, sy2		// rewind
	;;
	ld1	valy1 = [sx1], 2	// do non-speculative load
	ld1	valy2 = [sx2], 2
	cmp.eq	p4, p0 = r0, r0		// set p4
	;;
	br.sptk.many	.finalloop1	// neither load failed, continue
END(strcmp)
libc_hidden_builtin_def (strcmp)
