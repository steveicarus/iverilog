#ifndef __vvm_vvm_func_H
#define __vvm_vvm_func_H
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
#ident "$Id: vvm_func.h,v 1.1 1998/11/09 23:44:11 steve Exp $"
#endif

# include  "vvm.h"

/*
 * Implement the unary NOT operator in the verilog way. This takes a
 * vector of a certain width and returns a result of the same width.
 */
template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_unop_not(const vvm_bitset_t<WIDTH>&p)
{
      vvm_bitset_t<WIDTH> result;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1) switch (p[idx]) {
	  case V0:
	    result[idx] = V1;
	    break;
	  case V1:
	    result[idx] = V0;
	    break;
	  default:
	    result[idx] = Vx;
      }
      return result;
}

/*
 * Implement the binary + operator in the verilog way. This takes
 * vectors of identical width and returns another vector of same width
 * that contains the arithmetic sum. Z values are converted to X.
 */
template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_binop_plus(const vvm_bitset_t<WIDTH>&l,
				   const vvm_bitset_t<WIDTH>&r)
{
      vvm_bitset_t<WIDTH> result;
      vvm_bit_t carry = V0;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    result[idx] = add_with_carry(l[idx], r[idx], carry);

      return result;
}

/*
 * The binary - operator is turned into + by doing 2's complement
 * arithmetic. l-r == l+~r+1. The "+1" is accomplished by adding in a
 * carry of 1 to the 0 bit position.
 */
template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_binop_minus(const vvm_bitset_t<WIDTH>&l,
				   const vvm_bitset_t<WIDTH>&r)
{
      vvm_bitset_t<WIDTH> res;
      res = vvm_unop_not(r);
      vvm_bit_t carry = V1;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    res[idx] = add_with_carry(l[idx], res[idx], carry);

      return res;
}

/*
 * Tests for equality are a bit tricky, as they allow for the left and
 * right subexpressions to have different size. The shorter bitset is
 * extended with zeros. Also, if there is Vx or Vz anywhere in either
 * vectors, the result is Vx.
 */
template <unsigned LW, unsigned RW>
vvm_bitset_t<1> vvm_binop_eq(const vvm_bitset_t<LW>&l,
			     const vvm_bitset_t<RW>&r)
{
      vvm_bitset_t<1> result;
      result[0] = V1;

      if (LW <= RW) {
	    for (unsigned idx = 0 ;  idx < LW ;  idx += 1) {
		  if ((l[idx] == Vx) || (l[idx] == Vz)) {
			result[0] = Vx;
			return result;
		  }
		  if ((r[idx] == Vx) || (r[idx] == Vz)) {
			result[0] = Vx;
			return result;
		  }
		  if (l[idx] != r[idx]) {
			result[0] = V0;
			return result;
		  }
	    }
	    for (unsigned idx = LW ;  idx < RW ;  idx += 1)
		  switch (r[idx]) {
		      case V0:
			break;
		      case V1:
			result[0] = V0;
			return result;
		      case Vx:
		      case Vz:
			result[0] = Vx;
			return result;
		  }
		  
	    return result;
      } else {
	    for (unsigned idx = 0 ;  idx < RW ;  idx += 1) {
		  if ((l[idx] == Vx) || (l[idx] == Vz)) {
			result[0] = Vx;
			return result;
		  }
		  if ((r[idx] == Vx) || (r[idx] == Vz)) {
			result[0] = Vx;
			return result;
		  }
		  if (l[idx] != r[idx]) {
			result[0] = V0;
			return result;
		  }
	    }
	    for (unsigned idx = RW ;  idx < LW ;  idx += 1)
		  switch (l[idx]) {
		      case V0:
			break;
		      case V1:
			result[0] = V0;
			return result;
		      case Vx:
		      case Vz:
			result[0] = Vx;
			return result;
		  }
		  
	    return result;
      }
}

template <unsigned LW, unsigned RW>
vvm_bitset_t<1> vvm_binop_ne(const vvm_bitset_t<LW>&l,
			     const vvm_bitset_t<RW>&r)
{
      vvm_bitset_t<1> result = vvm_binop_eq(l,r);
      result[0] = not(result[0]);
      return result;
}

/*
 * $Log: vvm_func.h,v $
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */
#endif
