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
#ident "$Id: resolv.cc,v 1.26 2005/06/22 18:30:12 steve Exp $"
#endif

# include  "resolv.h"
# include  "schedule.h"
# include  "compile.h"
# include  "statistics.h"
# include  <iostream>
# include  <assert.h>


resolv_functor::resolv_functor(vvp_scalar_t hiz_value, const char*debug_l)
: hiz_(hiz_value), debug_label_(debug_l)
{
}

resolv_functor::~resolv_functor()
{
}

void resolv_functor::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      recv_vec8(port, vvp_vector8_t(bit, 6 /* STRONG */));
}

void resolv_functor::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
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

      if (val_[pdx].eeq(bit))
	    return;

      val_[pdx] = bit;

      vvp_vector8_t out (bit);

      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1) {
	    if (idx == pdx)
		  continue;
	    if (val_[idx].size() == 0)
		  continue;
	    out = resolve(out, val_[idx]);
      }

      if (! hiz_.is_hiz()) {
	    for (unsigned idx = 0 ;  idx < out.size() ;  idx += 1) {
		  if (out.value(idx).is_hiz())
			out.set_bit(idx, hiz_);
	    }
      }

      if (debug_label_ && debug_file.is_open())
	    debug_file << "[" << schedule_simtime() << "] "
		       << debug_label_ << ": Resolv out=" << out
		       << " in=" << val_[0] << ", " << val_[1]
		       << ", " << val_[2] << ", " << val_[3] << endl;

      vvp_send_vec8(ptr->out, out);
}


/*
 * $Log: resolv.cc,v $
 */

