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
#ident "$Id: vvm_add_sub.cc,v 1.3 2001/07/25 03:10:50 steve Exp $"
#endif

# include "config.h"

# include  "vvm_gates.h"
# include  <assert.h>

vvm_add_sub::vvm_add_sub(unsigned wid)
: width_(wid), ndir_(St0)
{
      ibits_ = new vpip_bit_t[width_*3];
      ro_ = new vvm_nexus::drive_t[width_];

      c_ = StX;
      for (unsigned idx = 0 ;  idx < width_*3 ;  idx += 1)
	    ibits_[idx] = StX;
}

vvm_add_sub::~vvm_add_sub()
{
      delete[]ro_;
      delete[]ibits_;
}

vvm_nexus::drive_t* vvm_add_sub::config_rout(unsigned idx)
{
      assert(idx < width_);
      return ro_+idx;
}

vvm_nexus::drive_t* vvm_add_sub::config_cout()
{
      return &co_;
}

unsigned vvm_add_sub::key_DataA(unsigned idx) const
{
      assert(idx < width_);
      return idx;
}

unsigned vvm_add_sub::key_DataB(unsigned idx) const
{
      assert(idx < width_);
      return idx+width_;
}

void vvm_add_sub::init_DataA(unsigned idx, vpip_bit_t val)
{
      assert(idx < width_);
      ibits_[idx] = val;
}

void vvm_add_sub::init_DataB(unsigned idx, vpip_bit_t val)
{
      assert(idx < width_);
      ibits_[width_+idx] = val;
}

void vvm_add_sub::init_Add_Sub(unsigned, vpip_bit_t val)
{
      ndir_ = B_NOT(val);
}

void vvm_add_sub::start()
{
      compute_();
}

void vvm_add_sub::take_value(unsigned key, vpip_bit_t val)
{
      if (ibits_[key] == val) return;
      ibits_[key] = val;
      compute_();
}

void vvm_add_sub::compute_()
{
      vpip_bit_t carry = ndir_;

      vpip_bit_t*a = ibits_;
      vpip_bit_t*b = ibits_+width_;
      vpip_bit_t*r = ibits_+2*width_;

      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    vpip_bit_t val;
	    val = add_with_carry(a[idx], B_XOR(b[idx],ndir_), carry);
	    if (val == r[idx]) continue;
	    r[idx] = val;
	    vvm_event*ev = new vvm_out_event(val, ro_+idx);
	    ev->schedule();
      }
      if (carry != c_)
	    (new vvm_out_event(carry, &co_)) -> schedule();
}

/*
 * $Log: vvm_add_sub.cc,v $
 * Revision 1.3  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.2  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.1  2000/03/17 17:25:53  steve
 *  Adder and comparator in nexus style.
 *
 */

