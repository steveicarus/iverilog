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
#ident "$Id: vvm_bit.cc,v 1.2 1998/11/10 00:48:31 steve Exp $"
#endif

# include  "vvm.h"

ostream& operator << (ostream&os, vvm_bit_t bit)
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

vvm_bit_t add_with_carry(vvm_bit_t l, vvm_bit_t r, vvm_bit_t&carry)
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
 * Revision 1.2  1998/11/10 00:48:31  steve
 *  Add support it vvm target for level-sensitive
 *  triggers (i.e. the Verilog wait).
 *  Fix display of $time is format strings.
 *
 * Revision 1.1  1998/11/09 23:44:10  steve
 *  Add vvm library.
 *
 */

