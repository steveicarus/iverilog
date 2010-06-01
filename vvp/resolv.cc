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

# include  "resolv.h"
# include  "schedule.h"
# include  "compile.h"
# include  "statistics.h"
# include  <iostream>
# include  <cassert>


resolv_functor::resolv_functor(vvp_scalar_t hiz_value, const char*debug_l)
: hiz_(hiz_value), debug_label_(debug_l)
{
      count_functors_resolv += 1;
}

resolv_functor::~resolv_functor()
{
}

void resolv_functor::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                               vvp_context_t)
{
      recv_vec8(port, vvp_vector8_t(bit, 6,6 /* STRONG */));
}

void resolv_functor::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				  unsigned base, unsigned wid, unsigned vwid,
                                  vvp_context_t)
{
      assert(bit.size() == wid);
      vvp_vector4_t res (vwid);

      for (unsigned idx = 0 ;  idx < base ;  idx += 1)
	    res.set_bit(idx, BIT4_Z);

      for (unsigned idx = 0 ;  idx < wid  && idx+base < vwid;  idx += 1)
	    res.set_bit(idx+base, bit.value(idx));

      for (unsigned idx = base+wid ;  idx < vwid ;  idx += 1)
	    res.set_bit(idx, BIT4_Z);

      recv_vec4(port, res, 0);
}

void resolv_functor::recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit)
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
	    if (out.size()==0)
		  out = val_[idx];
	    else
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

void resolv_functor::recv_vec8_pv(vvp_net_ptr_t port, const vvp_vector8_t&bit,
				  unsigned base, unsigned wid, unsigned vwid)
{
      assert(bit.size() == wid);
      vvp_vector8_t res (vwid);

      for (unsigned idx = 0 ;  idx < base ;  idx += 1)
	    res.set_bit(idx, vvp_scalar_t());

      for (unsigned idx = 0 ;  idx < wid && idx+base < vwid;  idx += 1)
	    res.set_bit(idx+base, bit.value(idx));

      for (unsigned idx = base+wid ;  idx < vwid ;  idx += 1)
	    res.set_bit(idx, vvp_scalar_t());

      recv_vec8(port, res);
}

resolv_wired_logic::resolv_wired_logic()
{
}

resolv_wired_logic::~resolv_wired_logic()
{
}

void resolv_wired_logic::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                   vvp_context_t)
{
      unsigned pdx = port.port();
      vvp_net_t*ptr = port.ptr();

      if (val_[pdx].eeq(bit))
	    return;

      val_[pdx] = bit;

      vvp_vector4_t out (bit);
      for (unsigned idx = 0 ; idx < 4 ; idx += 1) {
	    if (idx == pdx)
		  continue;
	    if (val_[idx].size() == 0)
		  continue;

	    out = wired_logic_math_(out, val_[idx]);
      }

      vvp_send_vec4(ptr->out, out, 0);
}

vvp_vector4_t resolv_triand::wired_logic_math_(vvp_vector4_t&a, vvp_vector4_t&b)
{
      assert(a.size() == b.size());

      vvp_vector4_t out (a.size());

      for (unsigned idx = 0 ; idx < out.size() ; idx += 1) {
	    vvp_bit4_t abit = a.value(idx);
	    vvp_bit4_t bbit = b.value(idx);
	    if (abit == BIT4_Z) {
		  out.set_bit(idx, bbit);
	    } else if (bbit == BIT4_Z) {
		  out.set_bit(idx, abit);
	    } else if (abit == BIT4_0 || bbit == BIT4_0) {
		  out.set_bit(idx, BIT4_0);
	    } else if (abit == BIT4_X || bbit == BIT4_X) {
		  out.set_bit(idx, BIT4_X);
	    } else {
		  out.set_bit(idx, BIT4_1);
	    }
      }

      return out;
}

vvp_vector4_t resolv_trior::wired_logic_math_(vvp_vector4_t&a, vvp_vector4_t&b)
{
      assert(a.size() == b.size());

      vvp_vector4_t out (a.size());

      for (unsigned idx = 0 ; idx < out.size() ; idx += 1) {
	    vvp_bit4_t abit = a.value(idx);
	    vvp_bit4_t bbit = b.value(idx);
	    if (abit == BIT4_Z) {
		  out.set_bit(idx, bbit);
	    } else if (bbit == BIT4_Z) {
		  out.set_bit(idx, abit);
	    } else if (abit == BIT4_1 || bbit == BIT4_1) {
		  out.set_bit(idx, BIT4_1);
	    } else if (abit == BIT4_X || bbit == BIT4_X) {
		  out.set_bit(idx, BIT4_X);
	    } else {
		  out.set_bit(idx, BIT4_0);
	    }
      }

      return out;
}
