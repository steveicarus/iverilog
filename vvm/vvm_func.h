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
#ident "$Id: vvm_func.h,v 1.24 2000/03/24 02:43:37 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_signal.h"

/*
 * Implement the unary NOT operator in the verilog way. This takes a
 * vector of a certain width and returns a result of the same width.
 */
template <unsigned WIDTH>
void vvm_unop_not(vvm_bitset_t<WIDTH>&v, const vvm_bitset_t<WIDTH>&p)
{
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    v[idx] = B_NOT(p[idx]);
}

/*
 * The unary AND is the reduction AND. It returns a single bit.
 */
extern vpip_bit_t vvm_unop_and(const vvm_bits_t&r);
extern vpip_bit_t vvm_unop_nand(const vvm_bits_t&r);

extern vpip_bit_t vvm_unop_lnot(const vvm_bits_t&r);

/*
 * The unary OR is the reduction OR. It returns a single bit.
 */
extern vpip_bit_t vvm_unop_or(const vvm_bits_t&r);
extern vpip_bit_t vvm_unop_nor(const vvm_bits_t&r);


/*
 * The unary XOR is the reduction XOR. It returns a single bit.
 */
extern vpip_bit_t vvm_unop_xor(const vvm_bits_t&r);
extern vpip_bit_t vvm_unop_xnor(const vvm_bits_t&r);

//
// simple-minded unary minus operator (two's complement)
//
template <unsigned WIDTH>
void vvm_unop_uminus(vvm_bitset_t<WIDTH>&v, const vvm_bitset_t<WIDTH>&l)
{
      vvm_unop_not(v, l);
      vpip_bit_t carry = St1;
      for (int i = 0; i < WIDTH; i++)
	    v[i] = add_with_carry(v[i], St0, carry);

}

/*
 * Implement the binary AND operator. This is a bitwise and with all
 * the parameters and the result having the same width.
 */
template <unsigned WIDTH>
void vvm_binop_and(vvm_bitset_t<WIDTH>&v,
		   const vvm_bitset_t<WIDTH>&l,
		   const vvm_bitset_t<WIDTH>&r)
{
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    v[idx] = B_AND(l[idx], r[idx]);
}

/*
 * Implement the binary OR operator. This is a bitwise and with all
 * the parameters and the result having the same width.
 */
template <unsigned WIDTH>
void vvm_binop_or(vvm_bitset_t<WIDTH>&v,
		  const vvm_bitset_t<WIDTH>&l,
		  const vvm_bitset_t<WIDTH>&r)
{
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    v[idx] = B_OR(l[idx], r[idx]);
}

template <unsigned WIDTH>
void vvm_binop_nor(vvm_bitset_t<WIDTH>&v,
		   const vvm_bitset_t<WIDTH>&l,
		   const vvm_bitset_t<WIDTH>&r)
{
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    v[idx] = B_NOT(B_OR(l[idx], r[idx]));
}

/*
 * Implement the binary + operator in the verilog way. This takes
 * vectors of identical width and returns another vector of same width
 * that contains the arithmetic sum. Z values are converted to X.
 */
template <unsigned WIDTH>
void vvm_binop_plus(vvm_bitset_t<WIDTH>&v,
		    const vvm_bitset_t<WIDTH>&l,
		    const vvm_bitset_t<WIDTH>&r)
{
      vpip_bit_t carry = St0;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    v[idx] = add_with_carry(l[idx], r[idx], carry);
}

/*
 * The binary - operator is turned into + by doing 2's complement
 * arithmetic. l-r == l+~r+1. The "+1" is accomplished by adding in a
 * carry of 1 to the 0 bit position.
 */
template <unsigned WIDTH>
void vvm_binop_minus(vvm_bitset_t<WIDTH>&v,
		     const vvm_bitset_t<WIDTH>&l,
		     const vvm_bitset_t<WIDTH>&r)
{
      vvm_unop_not(v, r);
      vpip_bit_t carry = St1;
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    v[idx] = add_with_carry(l[idx], v[idx], carry);
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
void vvm_binop_xor(vvm_bitset_t<WIDTH>&v,
		   const vvm_bitset_t<WIDTH>&l,
		   const vvm_bitset_t<WIDTH>&r)
{
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    v[idx] = B_XOR(l[idx], r[idx]);
}

template <unsigned WIDTH>
void vvm_binop_xnor(vvm_bitset_t<WIDTH>&v,
		    const vvm_bitset_t<WIDTH>&l,
		    const vvm_bitset_t<WIDTH>&r)
{
      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
	    v[idx] = B_NOT(B_XOR(l[idx], r[idx]));
}

/*
 * the binary 'l' operator is a logic left-shift by the number of positions
 * indicated by argument r. r is an unsigned integer, which is represented
 * internally as a 32-bit bitvector.
 */
template <unsigned WIDTH>
void vvm_binop_shiftl(vvm_bitset_t<WIDTH>&v,
		      const vvm_bitset_t<WIDTH>&l,
		      const vvm_bits_t&r)
{
      vvm_u32 s = r.as_unsigned();
      for (unsigned idx = 0; idx < WIDTH; idx++)
	    v[idx] = (idx < s) ? St0 : l[idx-s];
}

/*
 * The binary 'r' operator is a logic right-shift by the number of positions
 * indicated by argument r. r is an unsigned integer, which is represented
 * internally by a 32-bit bitvector.
 */
template <unsigned WIDTH>
void vvm_binop_shiftr(vvm_bitset_t<WIDTH>&v,
		      const vvm_bitset_t<WIDTH>&l,
		      const vvm_bits_t&r)
{
      vvm_u32 s = r.as_unsigned();
      for (unsigned idx = 0; idx < WIDTH; idx++)
	    v[idx] = (idx < (WIDTH-s)) ? l[idx+s] : St0;
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

template <unsigned W>
void vvm_ternary(vvm_bitset_t<W>&v, vpip_bit_t c,
		 const vvm_bitset_t<W>&t,
		 const vvm_bitset_t<W>&f)
{
      if (B_IS0(c)) {
	    for (unsigned idx = 0 ;  idx < W ;  idx += 1)
		  v[idx] = f[idx];
	    return;
      }
      if (B_IS1(c)) {
	    for (unsigned idx = 0 ;  idx < W ;  idx += 1)
		  v[idx] = t[idx];
	    return;
      }

      for (unsigned idx = 0 ;  idx < W ;  idx += 1) {
	    if (B_EQ(t[idx], f[idx]))
		  v[idx] = t[idx];
	    else
		  v[idx] = StX;
      }
}

/*
 * $Log: vvm_func.h,v $
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
