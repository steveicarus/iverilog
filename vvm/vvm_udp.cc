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
#ident "$Id: vvm_udp.cc,v 1.1 2000/03/29 04:37:11 steve Exp $"
#endif

# include  "vvm_gates.h"

vvm_udp_comb::vvm_udp_comb(unsigned w, const char*t)
: vvm_1bit_out(0), width_(w), table_(t)
{
      ibits_ = new vpip_bit_t[width_];
}

vvm_udp_comb::~vvm_udp_comb()
{
      delete[]ibits_;
}

void vvm_udp_comb::init_I(unsigned idx, vpip_bit_t val)
{
      assert(idx < width_);
      ibits_[idx] = val;
}

void vvm_udp_comb::take_value(unsigned key, vpip_bit_t val)
{
      if (ibits_[key] == val)
	    return;

      ibits_[key] = val;
      char bit;
      if (B_IS1(val))
	    bit = '1';
      else if (B_IS0(val))
	    bit = '0';
      else
	    bit = 'x';


      for (const char*row = table_ ;  row[0] ;  row += width_+1) {

	    unsigned idx;
	    for (idx = 0 ;  idx < width_ ;  idx += 1)
		  if (row[idx] != bit)
			break;

	    if (idx == width_) switch (row[width_]) {

		case '0':
		  output(St0);
		  return;
		case '1':
		  output(St1);
		  return;
		case 'z':
		  output(HiZ);
		  return;
		default:
		  output(StX);
		  return;
	    }
      }

      output(StX);
}


/*
 * $Log: vvm_udp.cc,v $
 * Revision 1.1  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 */

