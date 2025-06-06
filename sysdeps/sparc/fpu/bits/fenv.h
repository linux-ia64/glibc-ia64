/* Copyright (C) 1997-2025 Free Software Foundation, Inc.
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

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif

#include <bits/wordsize.h>


/* Define bits representing the exception.  We use the bit positions
   of the appropriate accrued exception bits from the FSR.  */
enum
  {
    FE_INVALID =
#define FE_INVALID	(1 << 9)
      FE_INVALID,
    FE_OVERFLOW =
#define FE_OVERFLOW	(1 << 8)
      FE_OVERFLOW,
    FE_UNDERFLOW =
#define FE_UNDERFLOW	(1 << 7)
      FE_UNDERFLOW,
    FE_DIVBYZERO =
#define FE_DIVBYZERO	(1 << 6)
      FE_DIVBYZERO,
    FE_INEXACT =
#define FE_INEXACT	(1 << 5)
      FE_INEXACT
  };

#define FE_ALL_EXCEPT \
	(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* The Sparc FPU supports all of the four defined rounding modes.  We
   use again the bit positions in the FPU control word as the values
   for the appropriate macros.  */
enum
  {
    FE_TONEAREST =
#define FE_TONEAREST	(0 << 30)
      FE_TONEAREST,
    FE_TOWARDZERO =
#define FE_TOWARDZERO	(1 << 30)
      FE_TOWARDZERO,
    FE_UPWARD =
#define FE_UPWARD	(-0x7fffffff - 1) /* (2 << 30) */
      FE_UPWARD,
    FE_DOWNWARD =
#define FE_DOWNWARD	(-0x40000000) /* (3 << 30) */
      FE_DOWNWARD
  };

#define __FE_ROUND_MASK	(3U << 30)


/* Type representing exception flags.  */
typedef unsigned long int fexcept_t;


/* Type representing floating-point environment.  */
typedef unsigned long int fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((const fenv_t *) -1)

#ifdef __USE_GNU
/* Floating-point environment where none of the exception is masked.  */
# define FE_NOMASK_ENV	((const fenv_t *) -2)
#endif

#if __GLIBC_USE (IEC_60559_BFP_EXT_C23)
/* Type representing floating-point control modes.  */
typedef unsigned long int femode_t;

/* Default floating-point control modes.  */
# define FE_DFL_MODE	((const femode_t *) -1L)
#endif
