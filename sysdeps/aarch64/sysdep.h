/* Copyright (C) 1997-2025 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _AARCH64_SYSDEP_H
#define _AARCH64_SYSDEP_H

#include <sysdeps/generic/sysdep.h>

#ifdef	__ASSEMBLER__

/* CFI directive for return address.  */
#define cfi_negate_ra_state	.cfi_negate_ra_state

/* Syntactic details of assembler.  */

#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

/* Guarded Control Stack support.  */
#define CHKFEAT_X16	hint	40
#define MRS_GCSPR(x)	mrs	x, s3_3_c2_c5_1
#define GCSPOPM(x)	sysl	x, #3, c7, c7, #1
#define GCSSS1(x)	sys	#3, c7, c7, #2, x
#define GCSSS2(x)	sysl	x, #3, c7, c7, #3

/* GNU_PROPERTY_AARCH64_* macros from elf.h for use in asm code.  */
#define FEATURE_1_AND 0xc0000000
#define FEATURE_1_BTI 1
#define FEATURE_1_PAC 2
#define FEATURE_1_GCS 4

/* Add a NT_GNU_PROPERTY_TYPE_0 note.  */
#define GNU_PROPERTY(type, value)	\
  .section .note.gnu.property, "a";	\
  .p2align 3;				\
  .word 4;				\
  .word 16;				\
  .word 5;				\
  .asciz "GNU";				\
  .word type;				\
  .word 4;				\
  .word value;				\
  .word 0;				\
  .text

/* Add GNU property note with the supported features to all asm code
   where sysdep.h is included.  */
GNU_PROPERTY (FEATURE_1_AND, FEATURE_1_BTI|FEATURE_1_PAC|FEATURE_1_GCS)

/* Define an entry point visible from C.  */
#define ENTRY(name)						\
  .globl C_SYMBOL_NAME(name);					\
  .type C_SYMBOL_NAME(name),%function;				\
  .p2align 6;							\
  C_LABEL(name)							\
  cfi_startproc;						\
  bti	c;							\
  CALL_MCOUNT

/* Define an entry point visible from C.  */
#define ENTRY_ALIGN(name, align)				\
  .globl C_SYMBOL_NAME(name);					\
  .type C_SYMBOL_NAME(name),%function;				\
  .p2align align;						\
  C_LABEL(name)							\
  cfi_startproc;						\
  bti	c;							\
  CALL_MCOUNT

/* Define an entry point visible from C with a specified alignment and
   pre-padding with NOPs.  This can be used to ensure that a critical
   loop within a function is cache line aligned.  Note this version
   does not adjust the padding if CALL_MCOUNT is defined. */

#define ENTRY_ALIGN_AND_PAD(name, align, padding)		\
  .globl C_SYMBOL_NAME(name);					\
  .type C_SYMBOL_NAME(name),%function;				\
  .p2align align;						\
  .rep padding - 1; /* -1 for bti c.  */			\
  nop;								\
  .endr;							\
  C_LABEL(name)							\
  cfi_startproc;						\
  bti	c;							\
  CALL_MCOUNT

#undef	END
#define END(name)						\
  cfi_endproc;							\
  ASM_SIZE_DIRECTIVE(name)

/* If compiled for profiling, call `mcount' at the start of each function.  */
#ifdef	PROF
# define CALL_MCOUNT						\
	str	x30, [sp, #-80]!;				\
	cfi_adjust_cfa_offset (80);				\
	cfi_rel_offset (x30, 0);				\
	stp	x0, x1, [sp, #16];				\
	cfi_rel_offset (x0, 16);				\
	cfi_rel_offset (x1, 24);				\
	stp	x2, x3, [sp, #32];				\
	cfi_rel_offset (x2, 32);				\
	cfi_rel_offset (x3, 40);				\
	stp	x4, x5, [sp, #48];				\
	cfi_rel_offset (x4, 48);				\
	cfi_rel_offset (x5, 56);				\
	stp	x6, x7, [sp, #64];				\
	cfi_rel_offset (x6, 64);				\
	cfi_rel_offset (x7, 72);				\
	mov	x0, x30;					\
	bl	mcount;						\
	ldp	x0, x1, [sp, #16];				\
	cfi_restore (x0);					\
	cfi_restore (x1);					\
	ldp	x2, x3, [sp, #32];				\
	cfi_restore (x2);					\
	cfi_restore (x3);					\
	ldp	x4, x5, [sp, #48];				\
	cfi_restore (x4);					\
	cfi_restore (x5);					\
	ldp	x6, x7, [sp, #64];				\
	cfi_restore (x6);					\
	cfi_restore (x7);					\
	ldr	x30, [sp], #80;					\
	cfi_adjust_cfa_offset (-80);				\
	cfi_restore (x30);
#else
# define CALL_MCOUNT		/* Do nothing.  */
#endif

/* Local label name for asm code.  */
#ifndef L
# define L(name)         .L##name
#endif

/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define syscall_error	__syscall_error
#define mcount		_mcount

#endif	/* __ASSEMBLER__ */

#endif  /* _AARCH64_SYSDEP_H */
