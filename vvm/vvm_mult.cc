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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vvm_mult.cc,v 1.8 2002/08/12 01:35:07 steve Exp $"
#endif

# include "config.h"

# include  "vvm_gates.h"
# include  <assert.h>


static void add_into_result(vpip_bit_t *r, unsigned nr,
			    const vpip_bit_t *a, unsigned na) {
      unsigned idx;

      vpip_bit_t carry_bit = St0;

      for (idx = 0; idx < nr && idx < na; idx++) {
	    vpip_bit_t val;

	    val = B_XOR(B_XOR(a[idx], r[idx]), carry_bit);
            carry_bit = B_OR(B_OR(B_AND(a[idx], r[idx]),
                                  B_AND(a[idx], carry_bit)),
                             B_AND(r[idx], carry_bit));
            r[idx] = val;
      }
}

static void vvm_binop_mult_generic(vpip_bit_t*r, unsigned nr,
				   const vpip_bit_t*a, unsigned na,
				   const vpip_bit_t*b, unsigned nb) {
      unsigned idx;

      for (idx = 0 ;  idx < na ;  idx += 1) 
           if (B_ISXZ(a[idx]))
                 goto unknown_result;

      for (idx = 0 ;  idx < nb ;  idx += 1)
	    if (B_ISXZ(b[idx]))
		  goto unknown_result;

	// Start off initialized to 0...
      for (idx = 0 ;  idx < nr ;  idx += 1)
            r[idx] = St0;

      for (idx = 0; idx < nb && idx < nr; idx++)
            if (B_IS1(b[idx]))
		  add_into_result(r+idx, nr-idx, a, na);

      return;

 unknown_result:
      for (unsigned idx= 0 ;  idx < nr ;  idx += 1)
	    r[idx] = StX;
}


void vvm_binop_mult(vpip_bit_t*r, unsigned nr,
		    const vpip_bit_t*a, unsigned na,
		    const vpip_bit_t*b, unsigned nb)
{
      if (nr > 8*sizeof(unsigned long) ||
          na > 8*sizeof(unsigned long) ||
	  nb > 8*sizeof(unsigned long)) {
	      // If we can't use the fast routine, default to 
	      // the slow one...
	    vvm_binop_mult_generic(r, nr, a, na, b, nb);
	    return;
      }

      assert(nr <= 8*sizeof(unsigned long));
      assert(na <= 8*sizeof(unsigned long));
      assert(nb <= 8*sizeof(unsigned long));

      unsigned long av = 0, bv = 0;
      unsigned long rv;

      for (unsigned idx = 0 ;  idx < na ;  idx += 1) {

	    if (B_ISXZ(a[idx]))
		  goto unknown_result;

	    if (B_IS1(a[idx]))
		  av |= 1 << idx;
      }

      for (unsigned idx = 0 ;  idx < nb ;  idx += 1) {

	    if (B_ISXZ(b[idx]))
		  goto unknown_result;

	    if (B_IS1(b[idx]))
		  bv |= 1 << idx;
      }

      rv = av * bv;

      for (unsigned idx = 0 ;  idx < nr ;  idx += 1) {

	    if (rv & 1)
		  r[idx] = St1;
	    else
		  r[idx] = St0;

	    rv >>= 1;
      }

      return;

 unknown_result:
      for (unsigned idx= 0 ;  idx < nr ;  idx += 1)
	    r[idx] = StX;
}

vvm_mult::vvm_mult(unsigned rwid, unsigned awid,
		   unsigned bwid, unsigned swid)
: rwid_(rwid), awid_(awid), bwid_(bwid), swid_(swid)
{
      bits_ = new vpip_bit_t[rwid_+awid_+bwid_+swid_];
      out_ = new vvm_nexus::drive_t[rwid_];

      for (unsigned idx = 0 ;  idx < rwid_+awid_+bwid_+swid_ ;  idx += 1)
	    bits_[idx] = StX;
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
 * Revision 1.8  2002/08/12 01:35:07  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.7  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.6  2000/04/21 02:30:40  steve
 *  Generic multiplier (Chris Lattner)
 *
 * Revision 1.5  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
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

