/*
 * Copyright (c) 2001-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: resolv.cc,v 1.22 2005/03/12 04:27:43 steve Exp $"
#endif

# include  "resolv.h"
# include  "schedule.h"
# include  "statistics.h"
# include  <stdio.h>
# include  <assert.h>


resolv_functor::resolv_functor(vvp_scalar_t hiz_value)
: hiz_(hiz_value)
{
}

resolv_functor::~resolv_functor()
{
}

void resolv_functor::recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit)
{
      recv_vec8(port, vvp_vector8_t(bit, 6 /* STRONG */));
}

void resolv_functor::recv_vec4_pv(vvp_net_ptr_t port, vvp_vector4_t bit,
				  unsigned base, unsigned wid, unsigned vwid)
{
      assert(bit.size() == wid);
      vvp_vector4_t res (vwid);

      for (unsigned idx = 0 ;  idx < base ;  idx += 1)
	    res.set_bit(idx, BIT4_Z);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
	    res.set_bit(idx+base, bit.value(idx));

      for (unsigned idx = base+wid ;  idx < vwid ;  idx += 1)
	    res.set_bit(idx, BIT4_Z);

      recv_vec4(port, res);
}

void resolv_functor::recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit)
{
      unsigned pdx = port.port();
      vvp_net_t*ptr = port.ptr();

      val_[pdx] = bit;

      vvp_vector8_t out (bit);

      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1) {
	    if (val_[idx].size() == 0)
		  continue;
	    if (idx == pdx)
		  continue;

	    out = resolve(out, val_[idx]);
      }

      if (! hiz_.is_hiz()) {
	    for (unsigned idx = 0 ;  idx < out.size() ;  idx += 1) {
		  if (out.value(idx).is_hiz())
			out.set_bit(idx, hiz_);
	    }
      }

      vvp_send_vec8(ptr->out, out);
}


/*
 * $Log: resolv.cc,v $
 * Revision 1.22  2005/03/12 04:27:43  steve
 *  Implement VPI access to signal strengths,
 *  Fix resolution of ambiguous drive pairs,
 *  Fix spelling of scalar.
 *
 * Revision 1.21  2005/02/13 05:26:30  steve
 *  tri0 and tri1 resolvers must replace HiZ with 0/1 after resolution.
 *
 * Revision 1.20  2005/01/09 20:11:16  steve
 *  Add the .part/pv node and related functionality.
 *
 * Revision 1.19  2004/12/31 06:00:06  steve
 *  Implement .resolv functors, and stub signals recv_vec8 method.
 *
 * Revision 1.18  2004/12/11 02:31:30  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */

