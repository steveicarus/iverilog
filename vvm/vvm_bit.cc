/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_bit.cc,v 1.11 2000/03/26 16:55:41 steve Exp $"
#endif

# include  "vvm.h"
# include  <iostream>

ostream& b_output (ostream&os, vpip_bit_t bit)
{
      if (B_IS0(bit)) {
	    os << "0";
	    return os;
      }

      if (B_IS1(bit)) {
	    os << "1";
	    return os;
      }

      if (B_ISX(bit)) {
	    os << "x";
	    return os;
      }

      if (B_ISZ(bit)) {
	    os << "z";
	    return os;
      }

      return os;
}

bool posedge(vpip_bit_t from, vpip_bit_t to)
{
      if (B_IS1(from))
	    return false;

      if (B_ISX(from) || B_ISZ(from))
	    return B_IS1(to);

      if (B_IS0(from))
	    return ! B_IS0(to);

      return false;
}

vpip_bit_t add_with_carry(vpip_bit_t l, vpip_bit_t r, vpip_bit_t&carry)
{
      unsigned li, ri, ci;

      if (B_IS1(l)) {
	    li = 1;
      } else if (B_IS0(l)) {
	    li = 0;
      } else {
	    carry = StX;
	    return StX;
      }

      if (B_IS1(r)) {
	    ri = 1;
      } else if (B_IS0(r)) {
	    ri = 0;
      } else {
	    carry = StX;
	    return StX;
      }

      if (B_IS1(carry)) {
	    ci = 1;
      } else if (B_IS0(carry)) {
	    ci = 0;
      } else {
	    carry = StX;
	    return StX;
      }

      unsigned sum = li + ri + ci;
      carry = (sum & 2)? St1 : St0;
      return (sum & 1)? St1 : St0;
}

/*
 * $Log: vvm_bit.cc,v $
 * Revision 1.11  2000/03/26 16:55:41  steve
 *  Remove the vvm_bits_t abstract class.
 *
 * Revision 1.10  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.9  2000/03/16 19:03:04  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 * Revision 1.8  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.7  1999/12/02 03:36:01  steve
 *  shiftl and shiftr take unsized second parameter.
 *
 * Revision 1.6  1999/11/22 00:30:52  steve
 *  Detemplate some and, or and nor methods.
 *
 * Revision 1.5  1999/11/21 00:13:09  steve
 *  Support memories in continuous assignments.
 */

