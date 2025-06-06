/* Dynamic loading of the libgcc unwinder.  ia64 customization.
   Copyright (C) 2021-2024 Free Software Foundation, Inc.
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

#ifndef _ARCH_UNWIND_LINK_H

#define UNWIND_LINK_GETIP 1
#define UNWIND_LINK_FRAME_STATE_FOR 1
#define UNWIND_LINK_FRAME_ADJUSTMENT 0
#define UNWIND_LINK_EXTRA_FIELDS \
  __typeof (_Unwind_GetBSP) *ptr__Unwind_GetBSP;
#define UNWIND_LINK_EXTRA_INIT                               \
  local.ptr__Unwind_GetBSP                                   \
    = __libc_dlsym (local_libgcc_handle, "_Unwind_GetBSP");  \
  assert (local.ptr__Unwind_GetBSP != NULL);                 \
  PTR_MANGLE (local.ptr__Unwind_GetBSP);

#endif /* _ARCH_UNWIND_LINK_H */
