/* strlen dispatch for RTLD and non-multiarch build
   Copyright (C) 2021-2025 Free Software Foundation, Inc.
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

#define STRLEN	strlen

#define DEFAULT_IMPL_V1	"multiarch/strlen-sse2.S"
#define DEFAULT_IMPL_V3	"multiarch/strlen-avx2.S"
#define DEFAULT_IMPL_V4	"multiarch/strlen-evex.S"

#include "isa-default-impl.h"

libc_hidden_builtin_def (strlen)
