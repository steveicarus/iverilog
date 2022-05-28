/*
 * Copyright (c) 2005-2016 Stephen Williams <steve@icarus.com>
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "vvp_net.h"
# include  "compile.h"
# include  <cstring>
# include  <iostream>
# include  <cassert>

vvp_fun_extend_signed::vvp_fun_extend_signed(unsigned wid)
: width_(wid)
{
}

vvp_fun_extend_signed::~vvp_fun_extend_signed()
{
}

void vvp_fun_extend_signed::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                      vvp_context_t)
{
      if (bit.size() >= width_) {
	    port.ptr()->send_vec4(bit, 0);
	    return;
      }

      vvp_vector4_t res (width_);

      for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1)
	    res.set_bit(idx, bit.value(idx));

      vvp_bit4_t pad = bit.size() > 0? bit.value(bit.size()-1) : BIT4_0;
      for (unsigned idx = bit.size() ;  idx < res.size() ;  idx += 1)
	    res.set_bit(idx, pad);

      port.ptr()->send_vec4(res, 0);
}

void vvp_fun_extend_signed::recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
					 unsigned base, unsigned vwid, vvp_context_t ctx)
{
      recv_vec4_pv_(ptr, bit, base, vwid, ctx);
}
