/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vvm_ff.cc,v 1.3 2001/07/25 03:10:50 steve Exp $"
#endif

# include "config.h"

# include  "vvm_gates.h"

vvm_ff::vvm_ff(unsigned w)
: width_(w)
{
      out_ = new vvm_nexus::drive_t[width_];
      bits_ = new vpip_bit_t[width_*2];
      for (unsigned idx = 0 ;  idx < width_*2 ;  idx += 1)
	    bits_[idx] = StX;
}

vvm_ff::~vvm_ff()
{
      delete[]bits_;
      delete[]out_;
}

vvm_nexus::drive_t* vvm_ff::config_rout(unsigned idx)
{
      assert(idx < width_);
      return out_+idx;
}

unsigned vvm_ff::key_Data(unsigned idx) const
{
      assert(idx < width_);
      return idx+width_;
}

unsigned vvm_ff::key_Clock() const
{
      return 0;
}

void vvm_ff::init_Data(unsigned idx, vpip_bit_t val)
{
      assert(idx < width_);
      bits_[idx+width_] = val;
}

void vvm_ff::init_Clock(unsigned, vpip_bit_t val)
{
      clk_ = val;
}

void vvm_ff::take_value(unsigned key, vpip_bit_t val)
{
      if (key == 0) {
	    if (val == clk_)
		  return;
	    bool flag = posedge(clk_, val);
	    clk_ = val;
	    if (flag)
		  latch_();
	    return;
      }

      bits_[key] = val;
}

void vvm_ff::latch_()
{
      vpip_bit_t*q = bits_;
      vpip_bit_t*d = bits_+width_;

      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    if (q[idx] == d[idx])
		  continue;

	    q[idx] = d[idx];
	    out_[idx].set_value(q[idx]);
      }
}

/*
 * $Log: vvm_ff.cc,v $
 * Revision 1.3  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.2  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.1  2000/03/18 23:22:37  steve
 *  Update the FF device to nexus style.
 *
 */

