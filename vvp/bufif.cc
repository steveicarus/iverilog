/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "bufif.h"
# include  "schedule.h"
# include  "statistics.h"
# include  <iostream>
# include  <cassert>

vvp_fun_bufif::vvp_fun_bufif(bool en_invert, bool out_invert,
			     unsigned str0, unsigned str1)
: pol_(en_invert? 1 : 0), inv_(out_invert? 1 : 0)
{
      drive0_ = str0;
      drive1_ = str1;
      count_functors_bufif += 1;
}

void vvp_fun_bufif::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                              vvp_context_t)
{
      switch (ptr.port()) {
	  case 0:
	    bit_ = inv_? ~bit : bit;
	    break;
	  case 1:
	    en_ = pol_? ~bit : bit;
	    break;
	  default:
	    return;
      }

      vvp_vector8_t out (bit.size());

      for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1) {
	    vvp_bit4_t b_en  = en_.value(idx);
	    vvp_bit4_t b_bit = bit_.value(idx);

	    switch (b_en) {
		case BIT4_0:
		  out.set_bit(idx, vvp_scalar_t(BIT4_Z,drive0_,drive1_));
		  break;
		case BIT4_1:
		  if (bit4_is_xz(b_bit))
			out.set_bit(idx, vvp_scalar_t(BIT4_X,drive0_,drive1_));
		  else
			out.set_bit(idx, vvp_scalar_t(b_bit,drive0_,drive1_));
		  break;

		default:
		  switch (b_bit) {
		      case BIT4_0:
			out.set_bit(idx, vvp_scalar_t(BIT4_X,drive0_,0));
			break;
		      case BIT4_1:
			out.set_bit(idx, vvp_scalar_t(BIT4_X,0,drive1_));
			break;
		      default:
			out.set_bit(idx, vvp_scalar_t(BIT4_X,drive0_,drive1_));
			break;
		  }
		  break;
	    }
      }

      vvp_send_vec8(ptr.ptr()->out, out);
}
