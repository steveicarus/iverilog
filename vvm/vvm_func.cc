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
#ident "$Id: vvm_func.cc,v 1.3 2000/03/24 02:43:37 steve Exp $"
#endif

# include  "vvm_func.h"

vpip_bit_t vvm_unop_and(const vvm_bits_t&r)
{
      vpip_bit_t v = r.get_bit(0);

      for (unsigned idx = 1 ;  idx < r.get_width() ;  idx += 1)
	    v = B_AND(v, r.get_bit(idx));

      return v;
}

vpip_bit_t vvm_unop_nand(const vvm_bits_t&r)
{
      vpip_bit_t v = vvm_unop_and(r);
      return B_NOT(v);
}

vpip_bit_t vvm_unop_lnot(const vvm_bits_t&r)
{
      vpip_bit_t v = vvm_unop_or(r);
      return B_NOT(v);
}

vpip_bit_t vvm_unop_or(const vvm_bits_t&r)
{
      for (unsigned idx = 0 ;  idx < r.get_width() ;  idx += 1) {
	    if (B_IS1(r.get_bit(idx)))
		  return St1;
      }

      return St0;
}

vpip_bit_t vvm_unop_nor(const vvm_bits_t&r)
{
      vpip_bit_t v = vvm_unop_or(r);
      return B_NOT(v);
}

vpip_bit_t vvm_unop_xor(const vvm_bits_t&r)
{
      vpip_bit_t v = St0;

      for (unsigned idx = 0 ;  idx < r.get_width() ;  idx += 1) {
	    if (B_IS1(r.get_bit(idx)))
		  v = B_NOT(v);
      }
      return v;
}

vpip_bit_t vvm_unop_xnor(const vvm_bits_t&r)
{
      vpip_bit_t v = vvm_unop_xor(r);
      return B_NOT(v);
}

vvm_bitset_t<1> vvm_binop_eq(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> result;
      result[0] = St1;
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      if (lwid <= rwid) {
	    for (unsigned idx = 0 ;  idx < lwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx))) {
			result[0] = StX;
			return result;
		  }
		  if (B_ISXZ(r.get_bit(idx))) {
			result[0] = StX;
			return result;
		  }
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }

	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1) {

		  if (B_IS0(r.get_bit(idx)))
			continue;

		  if (B_IS1(r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }

		  result[0] = StX;
		  return result;
	    }
		  
	    return result;

      } else {
	    for (unsigned idx = 0 ;  idx < rwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx))) {
			result[0] = StX;
			return result;
		  }
		  if (B_ISXZ(r.get_bit(idx))) {
			result[0] = StX;
			return result;
		  }
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1) {

		  if (B_IS0(l.get_bit(idx)))
			continue;

		  if (B_IS1(l.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }

		  result[0] = StX;
		  return result;
	    }
		  
	    return result;
      }
}

vvm_bitset_t<1> vvm_binop_ne(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> result = vvm_binop_eq(l,r);
      result[0] = B_NOT(result[0]);
      return result;
}

vvm_bitset_t<1> vvm_binop_eeq(const vvm_bits_t&l,  const vvm_bits_t&r)
{
      vvm_bitset_t<1> result;
      result[0] = St1;
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      if (lwid <= rwid) {
	    for (unsigned idx = 0 ;  idx < lwid ;  idx += 1)
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }

	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  if (! B_IS0(r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
		  
      } else {
	    for (unsigned idx = 0 ;  idx < rwid ;  idx += 1)
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }

	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  if (! B_IS0(l.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
		  
      }

      return result;
}

vvm_bitset_t<1> vvm_binop_nee(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> result = vvm_binop_eeq(l,r);
      result[0] = B_NOT(result[0]);
      return result;
}

vvm_bitset_t<1> vvm_binop_xeq(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> result;
      result[0] = St1;
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      if (lwid <= rwid) {
	    for (unsigned idx = 0 ;  idx < lwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx)))
			continue;
		  if (B_ISXZ(r.get_bit(idx)))
			continue;
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1) {
		  if (B_ISXZ(r.get_bit(idx)))
			continue;
		  if (! B_IS0(r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }
		  
      } else {
	    for (unsigned idx = 0 ;  idx < rwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx)))
			continue;
		  if (B_ISXZ(r.get_bit(idx)))
			continue;
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1) {
		  if (B_ISXZ(l.get_bit(idx)))
			continue;
		  if (! B_IS0(l.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }
      }

      return result;
}

vvm_bitset_t<1> vvm_binop_zeq(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> result;
      result[0] = St1;
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      if (lwid <= rwid) {
	    for (unsigned idx = 0 ;  idx < lwid ;  idx += 1) {
		  if (B_ISZ(l.get_bit(idx)) || B_ISZ(r.get_bit(idx)))
			continue;
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1) {
		  if (B_ISZ(r.get_bit(idx)))
			continue;
		  if (! B_IS0(r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }
		  
      } else {
	    for (unsigned idx = 0 ;  idx < rwid ;  idx += 1) {
		  if (B_ISZ(l.get_bit(idx)) || B_ISZ(r.get_bit(idx)))
			continue;
		  if (! B_EQ(l.get_bit(idx), r.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1) {
		  if (B_ISZ(l.get_bit(idx)))
			continue;
		  if (! B_IS0(l.get_bit(idx))) {
			result[0] = St0;
			return result;
		  }
	    }
      }

      return result;
}

vvm_bitset_t<1> vvm_binop_lt(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> result;
      result[0] = St0;
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();

      const unsigned common = (lwid < rwid)? lwid : rwid;

      for (unsigned idx = 0 ;  idx < common ;  idx += 1)
	    result[0] = less_with_cascade(l.get_bit(idx),
					  r.get_bit(idx),
					  result[0]);

      if (lwid > rwid) {
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  result[0] = less_with_cascade(l.get_bit(idx),
						St0,
						result[0]);
      } else {
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  result[0] = less_with_cascade(St0,
						r.get_bit(idx),
						result[0]);
      }

      return result;
}

vvm_bitset_t<1> vvm_binop_le(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> result;
      result[0] = St1;
      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();
      const unsigned common = (lwid < rwid)? lwid : rwid;

      for (unsigned idx = 0 ;  idx < common ;  idx += 1)
	    result[0] = less_with_cascade(l.get_bit(idx),
					  r.get_bit(idx),
					  result[0]);

      if (lwid > rwid) {
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  result[0] = less_with_cascade(l.get_bit(idx),
						St0,
						result[0]);
      } else {
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  result[0] = less_with_cascade(St0,
						r.get_bit(idx),
						result[0]);
      }

      return result;
}

vvm_bitset_t<1> vvm_binop_gt(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> result;
      result[0] = St0;

      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();
      const unsigned common = (lwid < rwid)? lwid : rwid;

      for (unsigned idx = 0 ;  idx < common ;  idx += 1)
	    result[0] = greater_with_cascade(l.get_bit(idx),
					     r.get_bit(idx),
					     result[0]);

      if (lwid > rwid) {
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  result[0] = greater_with_cascade(l.get_bit(idx),
						   St0,
						   result[0]);
      } else {
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  result[0] = greater_with_cascade(St0,
						   r.get_bit(idx),
						   result[0]);
      }

      return result;
}

vvm_bitset_t<1> vvm_binop_ge(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> result;
      result[0] = St1;

      const unsigned lwid = l.get_width();
      const unsigned rwid = r.get_width();
      const unsigned common = (lwid < rwid)? lwid : rwid;

      for (unsigned idx = 0 ;  idx < common ;  idx += 1)
	    result[0] = greater_with_cascade(l.get_bit(idx),
					     r.get_bit(idx),
					     result[0]);

      if (lwid > rwid) {
	    for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
		  result[0] = greater_with_cascade(l.get_bit(idx),
						   St0,
						   result[0]);
      } else {
	    for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
		  result[0] = greater_with_cascade(St0,
						   r.get_bit(idx),
						   result[0]);
      }

      return result;
}

vvm_bitset_t<1> vvm_binop_land(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> res1;
      res1[0] = vvm_unop_or(l);
      vvm_bitset_t<1> res2;
      res2[0] = vvm_unop_or(r);
      res1[0] = B_AND(res1[0], res2[0]);
      return res1;
}

vvm_bitset_t<1> vvm_binop_lor(const vvm_bits_t&l, const vvm_bits_t&r)
{
      vvm_bitset_t<1> res1;
      res1[0] = vvm_unop_or(l);
      vvm_bitset_t<1> res2;
      res2[0] = vvm_unop_or(r);
      res1[0] = B_OR(res1[0], res2[0]);
      return res1;
}


/*
 * $Log: vvm_func.cc,v $
 * Revision 1.3  2000/03/24 02:43:37  steve
 *  vvm_unop and vvm_binop pass result by reference
 *  instead of returning a value.
 *
 * Revision 1.2  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.1  2000/03/13 00:02:34  steve
 *  Remove unneeded templates.
 *
 */

