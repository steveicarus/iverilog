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
#ident "$Id: vvm_idiv.cc,v 1.1 2000/04/01 21:40:23 steve Exp $"
#endif

# include  "vvm_func.h"
# include  "vvm_gates.h"
# include  <assert.h>

void vvm_binop_idiv(vvm_bitset_t&v, const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      assert(v.get_width() <= 8*sizeof(unsigned long));
      assert(l.get_width() <= 8*sizeof(unsigned long));
      assert(r.get_width() <= 8*sizeof(unsigned long));

      unsigned long lv = 0, rv = 0;
      unsigned long vv;

      for (unsigned idx = 0 ;  idx < l.get_width() ;  idx += 1) {

	    if (B_ISXZ(l[idx]))
		  goto unknown_result;

	    if (B_IS1(l[idx]))
		  lv |= 1 << idx;
      }

      for (unsigned idx = 0 ;  idx < r.get_width() ;  idx += 1) {

	    if (B_ISXZ(r[idx]))
		  goto unknown_result;

	    if (B_IS1(r[idx]))
		  rv |= 1 << idx;
      }

      if (rv == 0)
	    goto unknown_result;

      vv = lv / rv;

      for (unsigned idx = 0 ;  idx < v.get_width() ;  idx += 1) {

	    if (vv & 1)
		  v[idx] = St1;
	    else
		  v[idx] = St0;

	    vv >>= 1;
      }

      return;

 unknown_result:
      for (unsigned idx= 0 ;  idx < v.get_width() ;  idx += 1)
	    v[idx] = StX;
}


vvm_idiv::vvm_idiv(unsigned rwid, unsigned awid, unsigned bwid)
: rwid_(rwid), awid_(awid), bwid_(bwid)
{
      bits_ = new vpip_bit_t[rwid_+awid_+bwid_];
      for (unsigned idx = 0 ;  idx < rwid_+awid_+bwid_ ;  idx += 1)
	    bits_[idx] = StX;

      out_ = new vvm_nexus::drive_t[rwid];
}

vvm_idiv::~vvm_idiv()
{
      delete[]out_;
      delete[]bits_;
}

void vvm_idiv::init_DataA(unsigned idx, vpip_bit_t val)
{
      assert(idx < awid_);
      bits_[rwid_+idx] = val;
}

void vvm_idiv::init_DataB(unsigned idx, vpip_bit_t val)
{
      assert(idx < bwid_);
      bits_[rwid_+awid_+idx] = val;
}

vvm_nexus::drive_t* vvm_idiv::config_rout(unsigned idx)
{
      assert(idx < rwid_);
      return out_+idx;
}

unsigned vvm_idiv::key_DataA(unsigned idx) const
{
      assert(idx < awid_);
      return rwid_+idx;
}

unsigned vvm_idiv::key_DataB(unsigned idx) const
{
      assert(idx < bwid_);
      return rwid_+awid_+idx;
}

void vvm_idiv::take_value(unsigned key, vpip_bit_t val)
{
      if (B_EQ(bits_[key], val)) {
	    bits_[key] = val;
	    return;
      }

      bits_[key] = val;

      vvm_bitset_t r (bits_, rwid_);
      vvm_bitset_t a (bits_+rwid_, awid_);
      vvm_bitset_t b (bits_+rwid_+awid_, bwid_);
      vvm_binop_idiv(r, a, b);

      for (unsigned idx = 0 ;  idx < rwid_ ;  idx += 1)
	    out_[idx].set_value(bits_[idx]);

}      

/*
 * $Log: vvm_idiv.cc,v $
 * Revision 1.1  2000/04/01 21:40:23  steve
 *  Add support for integer division.
 *
 */

