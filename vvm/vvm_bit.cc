/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: vvm_bit.cc,v 1.7 1999/12/02 03:36:01 steve Exp $"
#endif

# include  "vvm.h"
# include  <iostream>

ostream& operator << (ostream&os, vpip_bit_t bit)
{
      switch (bit) {
	  case V0:
	    os << "0";
	    break;
	  case V1:
	    os << "1";
	    break;
	  case Vx:
	    os << "x";
	    break;
	  case Vz:
	    os << "z";
	    break;
      }
      return os;
}

bool posedge(vpip_bit_t from, vpip_bit_t to)
{
      switch (from) {
	  case V1:
	    return false;
	  case V0:
	    return from != to;
	  case Vx:
	  case Vz:
	    return to == V1;
      }
      return false;
}

ostream& operator << (ostream&os, const vvm_bits_t&str)
{
      os << str.get_width() << "b'";
      for (unsigned idx = str.get_width() ;  idx > 0 ;  idx -= 1)
	    os << str.get_bit(idx);

      return os;
}

vvm_bits_t::~vvm_bits_t()
{
}

unsigned vvm_bits_t::as_unsigned() const
{
      unsigned result = 0;
      unsigned width = get_width();
      for (unsigned idx = width ;  idx > 0 ;  idx -= 1) {
	    result <<= 1;
	    switch (get_bit(idx-1)) {
		case V0:
		case Vx:
		case Vz:
		  break;
		case V1:
		  result |= 1;
		  break;
	    }
      }
      return result;
}

vvm_ram_callback::vvm_ram_callback()
{
}

vvm_ram_callback::~vvm_ram_callback()
{
}

vpip_bit_t add_with_carry(vpip_bit_t l, vpip_bit_t r, vpip_bit_t&carry)
{
      unsigned li, ri, ci;
      switch (l) {
	  case V0:
	    li = 0;
	    break;
	  case V1:
	    li = 1;
	    break;
	  default:
	    carry = Vx;
	    return Vx;
      }

      switch (r) {
	  case V0:
	    ri = 0;
	    break;
	  case V1:
	    ri = 1;
	    break;
	  default:
	    carry = Vx;
	    return Vx;
      }

      switch (carry) {
	  case V0:
	    ci = 0;
	    break;
	  case V1:
	    ci = 1;
	    break;
	  default:
	    carry = Vx;
	    return Vx;
      }

      unsigned sum = li + ri + ci;
      carry = (sum & 2)? V1 : V0;
      return (sum & 1)? V1 : V0;
}

/*
 * $Log: vvm_bit.cc,v $
 * Revision 1.7  1999/12/02 03:36:01  steve
 *  shiftl and shiftr take unsized second parameter.
 *
 * Revision 1.6  1999/11/22 00:30:52  steve
 *  Detemplate some and, or and nor methods.
 *
 * Revision 1.5  1999/11/21 00:13:09  steve
 *  Support memories in continuous assignments.
 *
 * Revision 1.4  1999/11/01 02:07:41  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.3  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 * Revision 1.2  1998/11/10 00:48:31  steve
 *  Add support it vvm target for level-sensitive
 *  triggers (i.e. the Verilog wait).
 *  Fix display of $time is format strings.
 *
 * Revision 1.1  1998/11/09 23:44:10  steve
 *  Add vvm library.
 *
 */

