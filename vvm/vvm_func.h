#ifndef __vvm_vvm_func_H
#define __vvm_vvm_func_H
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
#ident "$Id: vvm_func.h,v 1.21 2000/03/13 00:02:34 steve Exp $"
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
 * The unary AND is the reduction AND. It returns a single bit.
 */
extern vvm_bitset_t<1> vvm_unop_and(const vvm_bits_t&r);
extern vvm_bitset_t<1> vvm_unop_nand(const vvm_bits_t&r);

/*
 * The unary OR is the reduction OR. It returns a single bit.
 */
extern vvm_bitset_t<1> vvm_unop_or(const vvm_bits_t&r);
extern vvm_bitset_t<1> vvm_unop_nor(const vvm_bits_t&r);


/*
 * The unary XOR is the reduction XOR. It returns a single bit.
 */
extern vvm_bitset_t<1> vvm_unop_xor(const vvm_bits_t&r);
extern vvm_bitset_t<1> vvm_unop_xnor(const vvm_bits_t&r);

//
// simple-minded unary minus operator (two's complement)
//
template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_unop_uminus(const vvm_bitset_t<WIDTH>&l)
{
      vvm_bitset_t<WIDTH> res;
      res = vvm_unop_not(l);
      vpip_bit_t carry = V1;
      for (int i = 0; i < WIDTH; i++)
	    res[i] = add_with_carry(res[i], V0, carry);

      return res;
}

/*
 * Implement the binary AND operator. This is a bitwise and with all
 * the parameters and the result having the same width.
 */
template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_binop_and(const vvm_bitset_t<WIDTH>&l,
				  const vvm_bitset_t<WIDTH>&r)
{
      vvm_bitset_t<WIDTH> result;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    result[idx] = l[idx] & r[idx];

      return result;
}

/*
 * Implement the binary OR operator. This is a bitwise and with all
 * the parameters and the result having the same width.
 */
template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_binop_or(const vvm_bitset_t<WIDTH>&l,
				 const vvm_bitset_t<WIDTH>&r)
{
      vvm_bitset_t<WIDTH> result;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    result[idx] = l[idx] | r[idx];

      return result;
}

template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_binop_nor(const vvm_bitset_t<WIDTH>&l,
				 const vvm_bitset_t<WIDTH>&r)
{
      vvm_bitset_t<WIDTH> result;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    result[idx] = v_not(l[idx] | r[idx]);

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
      vpip_bit_t carry = V0;
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
      vpip_bit_t carry = V1;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    res[idx] = add_with_carry(l[idx], res[idx], carry);

      return res;
}

/*
 * The multiply binary operator takes an A and B parameter and returns
 * the result in the vpip_bit_t array. The template form arranges for
 * the right parameters to be passed to the extern form.
 */
extern void vvm_binop_mult(vpip_bit_t*res, unsigned nres,
			   const vpip_bit_t*a, unsigned na,
			   const vpip_bit_t*b, unsigned nb);

template <unsigned WR, unsigned WA, unsigned WB>
void vvm_binop_mult(vvm_bitset_t<WR>&r, 
		    const vvm_bitset_t<WA>&a,
		    const vvm_bitset_t<WB>&b)
{
      vvm_binop_mult(r.bits, WR, a.bits, WA, b.bits, WB);
}


/*
 * The binary ^ (xor) operator is a bitwise XOR of equal width inputs
 * to generate the corresponsing output.
 */
template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_binop_xor(const vvm_bitset_t<WIDTH>&l,
				  const vvm_bitset_t<WIDTH>&r)
{
      vvm_bitset_t<WIDTH> result;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    result[idx] = l[idx] ^ r[idx];

      return result;
}

template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_binop_xnor(const vvm_bitset_t<WIDTH>&l,
				   const vvm_bitset_t<WIDTH>&r)
{
      vvm_bitset_t<WIDTH> result;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    result[idx] = v_not(l[idx] ^ r[idx]);

      return result;
}

/*
 * the binary 'l' operator is a logic left-shift by the number of positions
 * indicated by argument r. r is an unsigned integer, which is represented
 * internally as a 32-bit bitvector.
 */
template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_binop_shiftl(const vvm_bitset_t<WIDTH>&l,
				     const vvm_bits_t&r)
{
      vvm_bitset_t<WIDTH> result;
      vvm_u32 s = r.as_unsigned();
      for (unsigned idx = 0; idx < WIDTH; idx++)
	    result[idx] = (idx < s) ? V0 : l[idx-s];
 
      return result;
}

/*
 * The binary 'r' operator is a logic right-shift by the number of positions
 * indicated by argument r. r is an unsigned integer, which is represented
 * internally by a 32-bit bitvector.
 */
template <unsigned WIDTH>
vvm_bitset_t<WIDTH> vvm_binop_shiftr(const vvm_bitset_t<WIDTH>&l,
				     const vvm_bits_t&r)
{
      vvm_bitset_t<WIDTH> result;
      vvm_u32 s = r.as_unsigned();
      for (unsigned idx = 0; idx < WIDTH; idx++)
	    result[idx] = (idx < (WIDTH-s)) ? l[idx+s] : V0;

      return result;
}

/*
 * Tests for equality are a bit tricky, as they allow for the left and
 * right subexpressions to have different size. The shorter bitset is
 * extended with zeros. Also, if there is Vx or Vz anywhere in either
 * vectors, the result is Vx.
 */
extern vvm_bitset_t<1> vvm_binop_eq(const vvm_bits_t&l, const vvm_bits_t&r);
extern vvm_bitset_t<1> vvm_binop_ne(const vvm_bits_t&l, const vvm_bits_t&r);

/*
 * This function return true if all the bits are the same. Even x and
 * z bites are compared for equality.
 */
extern vvm_bitset_t<1> vvm_binop_eeq(const vvm_bits_t&l, const vvm_bits_t&r);
extern vvm_bitset_t<1> vvm_binop_nee(const vvm_bits_t&l, const vvm_bits_t&r);


/*
 * This function return true if all the bits are the same. The x and z
 * bits are don't care, s don't make the result false.
 */
extern vvm_bitset_t<1> vvm_binop_xeq(const vvm_bits_t&l, const vvm_bits_t&r);

/*
 * This function return true if all the bits are the same. The z
 * bits are don't care, so don't make the result false.
 */
extern vvm_bitset_t<1> vvm_binop_zeq(const vvm_bits_t&l, const vvm_bits_t&r);


extern vvm_bitset_t<1> vvm_binop_lt(const vvm_bits_t&l, const vvm_bits_t&r);

/*
 * The <= operator takes operands of natural width and returns a
 * single bit. The result is V1 if l <= r, otherwise V0;
 */
extern vvm_bitset_t<1> vvm_binop_le(const vvm_bits_t&l, const vvm_bits_t&r);

extern vvm_bitset_t<1> vvm_binop_gt(const vvm_bits_t&l, const vvm_bits_t&r);

extern vvm_bitset_t<1> vvm_binop_ge(const vvm_bits_t&l, const vvm_bits_t&r);

extern vvm_bitset_t<1> vvm_binop_land(const vvm_bits_t&l, const vvm_bits_t&r);

extern vvm_bitset_t<1> vvm_binop_lor(const vvm_bits_t&l, const vvm_bits_t&r);

extern vvm_bitset_t<1> vvm_unop_lnot(const vvm_bits_t&r);

template <unsigned W>
vvm_bitset_t<W> vvm_ternary(vpip_bit_t c, const vvm_bitset_t<W>&t,
			    const vvm_bitset_t<W>&f)
{
      switch (c) {
	  case V0:
	    return f;
	  case V1:
	    return t;
	  default:
	    return f;
      }
}

/*
 * $Log: vvm_func.h,v $
 * Revision 1.21  2000/03/13 00:02:34  steve
 *  Remove unneeded templates.
 *
 * Revision 1.20  2000/02/23 04:43:43  steve
 *  Some compilers do not accept the not symbol.
 *
 * Revision 1.19  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.18  2000/01/13 06:05:46  steve
 *  Add the XNOR operator.
 *
 * Revision 1.17  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.16  1999/12/02 03:36:01  steve
 *  shiftl and shiftr take unsized second parameter.
 */
#endif
