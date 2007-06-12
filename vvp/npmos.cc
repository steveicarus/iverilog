/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: npmos.cc,v 1.14 2007/06/12 02:25:00 steve Exp $"
#endif

# include  "npmos.h"

vvp_fun_pmos_::vvp_fun_pmos_(bool enable_invert)
{
      inv_en_ = enable_invert;
}


void vvp_fun_pmos_::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
	/* Data input is processed throught eh recv_vec8 method,
	   because the strength most be preserved. */
      if (ptr.port() == 0) {
	    vvp_vector8_t tmp = bit;
	    recv_vec8(ptr, tmp);
	    return;
      }

      if (ptr.port() != 1)
	    return;

      en_ = inv_en_? ~bit : bit;
      generate_output_(ptr);
}

void vvp_fun_pmos_::generate_output_(vvp_net_ptr_t ptr)
{
      vvp_vector8_t out (bit_.size());

      for (unsigned idx = 0 ;  idx < out.size() ;  idx += 1) {
	    vvp_bit4_t   b_en  = en_.value(idx);
	    vvp_scalar_t b_bit = bit_.value(idx);

	    switch (b_en) {
		case BIT4_0:
		  out.set_bit(idx, b_bit);
		  break;
		case BIT4_1:
		  out.set_bit(idx, vvp_scalar_t(BIT4_Z,0,0));
		  break;
		default:
		  switch (b_bit.value()) {
		      case BIT4_0:
			b_bit = vvp_scalar_t(BIT4_X,b_bit.strength0(),0);
			break;
		      case BIT4_1:
			b_bit = vvp_scalar_t(BIT4_X,0,b_bit.strength1());
			break;
		      default:
			break;
		  }
		  out.set_bit(idx, b_bit);
		  break;
	    }
      }

      if (out.size() > 0)
	    vvp_send_vec8(ptr.ptr()->out, out);
}


vvp_fun_pmos::vvp_fun_pmos(bool enable_invert)
: vvp_fun_pmos_(enable_invert)
{
}

void vvp_fun_pmos::recv_vec8(vvp_net_ptr_t ptr, vvp_vector8_t bit)
{
      if (ptr.port() == 1) {
	    recv_vec4(ptr, reduce4(bit));
	    return;
      }

      if (ptr.port() != 0)
	    return;

      bit_ = bit;
      generate_output_(ptr);
}

vvp_fun_rpmos::vvp_fun_rpmos(bool enable_invert)
: vvp_fun_pmos_(enable_invert)
{
}

void vvp_fun_rpmos::recv_vec8(vvp_net_ptr_t ptr, vvp_vector8_t bit)
{
      if (ptr.port() == 1) {
	    recv_vec4(ptr, reduce4(bit));
	    return;
      }

      if (ptr.port() != 0)
	    return;

      bit_ = resistive_reduction(bit);
      generate_output_(ptr);
}

/*
 * $Log: npmos.cc,v $
 * Revision 1.14  2007/06/12 02:25:00  steve
 *  Do not propogate until initialized.
 *
 * Revision 1.13  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.12  2005/06/12 15:13:37  steve
 *  Support resistive mos devices.
 *
 * Revision 1.11  2005/06/12 00:44:49  steve
 *  Implement nmos and pmos devices.
 *
 */

