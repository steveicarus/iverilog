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
#ident "$Id: vvm_func.h,v 1.27 2000/03/26 16:55:41 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_signal.h"

/*
 * Implement the unary NOT operator in the verilog way. This takes a
 * vector of a certain width and returns a result of the same width.
 */
extern void vvm_unop_not(vvm_bitset_t&v, const vvm_bitset_t&p);

/*
 * The unary AND is the reduction AND. It returns a single bit.
 */
extern vpip_bit_t vvm_unop_and(const vvm_bitset_t&r);
extern vpip_bit_t vvm_unop_nand(const vvm_bitset_t&r);

extern vpip_bit_t vvm_unop_lnot(const vvm_bitset_t&r);

/*
 * The unary OR is the reduction OR. It returns a single bit.
 */
extern vpip_bit_t vvm_unop_or(const vvm_bitset_t&r);
extern vpip_bit_t vvm_unop_nor(const vvm_bitset_t&r);


/*
 * The unary XOR is the reduction XOR. It returns a single bit.
 */
extern vpip_bit_t vvm_unop_xor(const vvm_bitset_t&r);
extern vpip_bit_t vvm_unop_xnor(const vvm_bitset_t&r);

/*
 * simple-minded unary minus operator (two's complement)
 */
extern void vvm_unop_uminus(vvm_bitset_t&v, const vvm_bitset_t&l);

/*
 * Implement the binary AND operator. This is a bitwise and with all
 * the parameters and the result having the same width.
 */
extern void vvm_binop_and(vvm_bitset_t&v,
			  const vvm_bitset_t&l,
			  const vvm_bitset_t&r);

/*
 * Implement the binary OR operator. This is a bitwise and with all
 * the parameters and the result having the same width.
 */
extern void vvm_binop_or(vvm_bitset_t&v,
			 const vvm_bitset_t&l,
			 const vvm_bitset_t&r);

extern void vvm_binop_nor(vvm_bitset_t&v,
			  const vvm_bitset_t&l,
			  const vvm_bitset_t&r);

/*
 * Implement the binary + operator in the verilog way. This takes
 * vectors of identical width and returns another vector of same width
 * that contains the arithmetic sum. Z values are converted to X.
 */
extern void vvm_binop_plus(vvm_bitset_t&v,
			   const vvm_bitset_t&l,
			   const vvm_bitset_t&r);

/*
 * The binary - operator is turned into + by doing 2's complement
 * arithmetic. l-r == l+~r+1. The "+1" is accomplished by adding in a
 * carry of 1 to the 0 bit position.
 */
extern void vvm_binop_minus(vvm_bitset_t&v,
			    const vvm_bitset_t&l,
			    const vvm_bitset_t&r);

/*
 * The multiply binary operator takes an A and B parameter and returns
 * the result in the vpip_bit_t array. The template form arranges for
 * the right parameters to be passed to the extern form.
 */
extern void vvm_binop_mult(vpip_bit_t*res, unsigned nres,
			   const vpip_bit_t*a, unsigned na,
			   const vpip_bit_t*b, unsigned nb);

inline void vvm_binop_mult(vvm_bitset_t&r, 
			   const vvm_bitset_t&a,
			   const vvm_bitset_t&b)
{
      vvm_binop_mult(r.bits, r.nbits,
		     a.bits, a.nbits,
		     b.bits, b.nbits);
}


/*
 * The binary ^ (xor) operator is a bitwise XOR of equal width inputs
 * to generate the corresponsing output.
 */
extern void vvm_binop_xor(vvm_bitset_t&v,
			  const vvm_bitset_t&l,
			  const vvm_bitset_t&r);

extern void vvm_binop_xnor(vvm_bitset_t&v,
			   const vvm_bitset_t&l,
			   const vvm_bitset_t&r);

/*
 * the binary 'l' operator is a logic left-shift by the number of positions
 * indicated by argument r. r is an unsigned integer, which is represented
 * internally as a 32-bit bitvector.
 */
extern void vvm_binop_shiftl(vvm_bitset_t&v,
			     const vvm_bitset_t&l,
			     const vvm_bitset_t&r);

/*
 * The binary 'r' operator is a logic right-shift by the number of positions
 * indicated by argument r. r is an unsigned integer, which is represented
 * internally by a 32-bit bitvector.
 */
extern void vvm_binop_shiftr(vvm_bitset_t&v,
			     const vvm_bitset_t&l,
			     const vvm_bitset_t&r);

/*
 * Tests for equality are a bit tricky, as they allow for the left and
 * right subexpressions to have different size. The shorter bitset is
 * extended with zeros. Also, if there is Vx or Vz anywhere in either
 * vectors, the result is Vx.
 */
extern vpip_bit_t vvm_binop_eq(const vvm_bitset_t&l, const vvm_bitset_t&r);
extern vpip_bit_t vvm_binop_ne(const vvm_bitset_t&l, const vvm_bitset_t&r);

/*
 * This function return true if all the bits are the same. Even x and
 * z bites are compared for equality.
 */
extern vpip_bit_t vvm_binop_eeq(const vvm_bitset_t&l, const vvm_bitset_t&r);
extern vpip_bit_t vvm_binop_nee(const vvm_bitset_t&l, const vvm_bitset_t&r);


/*
 * This function return true if all the bits are the same. The x and z
 * bits are don't care, s don't make the result false.
 */
extern vpip_bit_t vvm_binop_xeq(const vvm_bitset_t&l, const vvm_bitset_t&r);

/*
 * This function return true if all the bits are the same. The z
 * bits are don't care, so don't make the result false.
 */
extern vpip_bit_t vvm_binop_zeq(const vvm_bitset_t&l, const vvm_bitset_t&r);


extern vpip_bit_t vvm_binop_lt(const vvm_bitset_t&l, const vvm_bitset_t&r);

/*
 * The <= operator takes operands of natural width and returns a
 * single bit. The result is V1 if l <= r, otherwise V0;
 */
extern vpip_bit_t vvm_binop_le(const vvm_bitset_t&l, const vvm_bitset_t&r);

extern vpip_bit_t vvm_binop_gt(const vvm_bitset_t&l, const vvm_bitset_t&r);

extern vpip_bit_t vvm_binop_ge(const vvm_bitset_t&l, const vvm_bitset_t&r);

extern vpip_bit_t vvm_binop_land(const vvm_bitset_t&l, const vvm_bitset_t&r);

extern vpip_bit_t vvm_binop_lor(const vvm_bitset_t&l, const vvm_bitset_t&r);

extern void vvm_ternary(vvm_bitset_t&v, vpip_bit_t c,
			const vvm_bitset_t&t,
			const vvm_bitset_t&f);

/*
 * $Log: vvm_func.h,v $
 * Revision 1.27  2000/03/26 16:55:41  steve
 *  Remove the vvm_bits_t abstract class.
 *
 * Revision 1.26  2000/03/26 16:28:31  steve
 *  vvm_bitset_t is no longer a template.
 *
 * Revision 1.25  2000/03/25 02:43:57  steve
 *  Remove all remain vvm_bitset_t return values,
 *  and disallow vvm_bitset_t copying.
 *
 * Revision 1.24  2000/03/24 02:43:37  steve
 *  vvm_unop and vvm_binop pass result by reference
 *  instead of returning a value.
 *
 * Revision 1.23  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.22  2000/03/16 19:03:04  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
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
