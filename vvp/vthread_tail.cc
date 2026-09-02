/*
 * Copyright (c) 2026 Andrew Pullin
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"
# include "codes.h"
# include "vthread.h"
# include "vvp_net.h"

/* Append new handlers without shifting the established hot opcode layout. */
#if defined(__ELF__)
# define VVP_TEXT_TAIL __attribute__((section(".vvp_text_tail")))
#else
# define VVP_TEXT_TAIL
#endif

/* Apply an immediate XOR directly to the vector stack's top value. */
VVP_TEXT_TAIL
bool of_XORI(vthread_t thr, vvp_code_t cp)
{
      const vvp_vector4_t&top = vthread_get_vec4_stack(thr, 0);
	// The public stack accessor is read-only; this opcode deliberately
	// updates the existing top value in place to avoid a pop and push.
      vvp_vector4_t&val = const_cast<vvp_vector4_t&>(top);

      val.xor_immediate(cp->bit_idx[0], cp->bit_idx[1], cp->number);
      return true;
}
