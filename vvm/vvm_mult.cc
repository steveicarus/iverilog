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
#ident "$Id: vvm_mult.cc,v 1.4 2000/03/17 03:05:13 steve Exp $"
#endif

# include  "vvm_gates.h"
# include  <assert.h>

void vvm_binop_mult(vpip_bit_t*r, unsigned nr,
		    const vpip_bit_t*a, unsigned na,
		    const vpip_bit_t*b, unsigned nb)
{
      assert(nr <= 8*sizeof(unsigned long));
      assert(na <= 8*sizeof(unsigned long));
      assert(nb <= 8*sizeof(unsigned long));

      unsigned long av = 0, bv = 0;
      unsigned long rv;

      for (unsigned idx = 0 ;  idx < na ;  idx += 1) switch (a[idx]) {

	  case V0:
	    break;
	  case V1:
	    av |= 1 << idx;
	    break;
	  default:
	    goto unknown_result;
      }

      for (unsigned idx = 0 ;  idx < nb ;  idx += 1) switch (b[idx]) {

	  case V0:
	    break;
	  case V1:
	    bv |= 1 << idx;
	    break;
	  default:
	    goto unknown_result;
      }

      rv = av * bv;

      for (unsigned idx = 0 ;  idx < nr ;  idx += 1) {

	    if (rv & 1)
		  r[idx] = V1;
	    else
		  r[idx] = V0;

	    rv >>= 1;
      }

      return;

 unknown_result:
      for (unsigned idx= 0 ;  idx < nr ;  idx += 1)
	    r[idx] = Vx;
}

vvm_mult::vvm_mult(unsigned rwid, unsigned awid,
		   unsigned bwid, unsigned swid)
: rwid_(rwid), awid_(awid), bwid_(bwid), swid_(swid)
{
      bits_ = new vpip_bit_t[rwid_+awid_+bwid_+swid_];
      out_ = new vvm_nexus::drive_t[rwid_];

      for (unsigned idx = 0 ;  idx < rwid_+awid_+bwid_+swid_ ;  idx += 1)
	    bits_[idx] = Vx;
}

vvm_mult::~vvm_mult()
{
      delete[]bits_;
      delete[]out_;
}

vvm_nexus::drive_t* vvm_mult::config_rout(unsigned idx)
{
      assert(idx < rwid_);
      return out_+idx;
}

unsigned vvm_mult::key_DataA(unsigned idx) const
{
      assert(idx < awid_);
      return rwid_ + idx;
}

void vvm_mult::init_DataA(unsigned idx, vpip_bit_t val)
{
      assert(idx < awid_);
      bits_[rwid_+idx] = val;
}

unsigned vvm_mult::key_DataB(unsigned idx) const
{
      assert(idx < bwid_);
      return rwid_+awid_+idx;
}

void vvm_mult::init_DataB(unsigned idx, vpip_bit_t val)
{
      assert(idx < bwid_);
      bits_[rwid_+awid_+idx] = val;
}

void vvm_mult::take_value(unsigned key, vpip_bit_t val)
{
      bits_[key] = val;
      vvm_binop_mult(bits_, rwid_,
		     bits_+rwid_, awid_,
		     bits_+rwid_+awid_, bwid_);
      for (unsigned idx = 0 ;  idx < rwid_ ;  idx += 1)
	    out_[idx].set_value(bits_[idx]);
}

/*
 * $Log: vvm_mult.cc,v $
 * Revision 1.4  2000/03/17 03:05:13  steve
 *  Update vvm_mult to nexus style.
 *
 * Revision 1.3  2000/03/04 01:13:54  steve
 *  Simpler implementation of multiplication.
 *
 * Revision 1.2  2000/02/23 02:56:57  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  2000/01/13 03:35:36  steve
 *  Multiplication all the way to simulation.
 *
 */

