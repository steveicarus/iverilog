/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: eval_expr.c,v 1.75 2002/09/12 15:49:43 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <assert.h>

struct vector_info draw_eval_expr_wid(ivl_expr_t exp, unsigned wid);

static unsigned char allocation_map[0x10000/8];

static inline int peek_bit(unsigned addr)
{
      unsigned bit = addr % 8;
      addr /= 8;
      return 1 & (allocation_map[addr] >> bit);
}

static inline void set_bit(unsigned addr)
{
      unsigned bit = addr % 8;
      addr /= 8;
      allocation_map[addr] |= (1 << bit);
}

static inline void clr_bit(unsigned addr)
{
      unsigned bit = addr % 8;
      addr /= 8;
      allocation_map[addr] &= ~(1 << bit);
}

/*
 * This clears a vector that was previously allocated by
 * allocate_vector. That is, it unmarks all the bits of the map that
 * represent this vector.
 *
 * If the vector is based in one of 4 constant bit values, then there
 * are no bits to clear. If the vector is based in the 4-8 result
 * area, then someone is broken.
 */
void clr_vector(struct vector_info vec)
{
      unsigned idx;
      if (vec.base < 4)
	    return;
      assert(vec.base >= 8);
      for (idx = 0 ;  idx < vec.wid ;  idx += 1)
	    clr_bit(vec.base + idx);
}

unsigned short allocate_vector(unsigned short wid)
{
      unsigned short base = 8;

      unsigned short idx = 0;
      while (idx < wid) {
	    assert((base + idx) < 0x10000);
	    if (peek_bit(base+idx)) {
		  base = base + idx + 1;
		  idx = 0;

	    } else {
		  idx += 1;
	    }
      }

      for (idx = 0 ;  idx < wid ;  idx += 1)
	    set_bit(base+idx);

      return base;
}

int number_is_unknown(ivl_expr_t ex)
{
      const char*bits;
      unsigned idx;

      assert(ivl_expr_type(ex) == IVL_EX_NUMBER);

      bits = ivl_expr_bits(ex);
      for (idx = 0 ;  idx < ivl_expr_width(ex) ;  idx += 1)
	    if ((bits[idx] != '0') && (bits[idx] != '1'))
		  return 1;

      return 0;
}

/*
 * This function returns TRUE if the number can be represented in a
 * 16bit immediate value. This amounts to looking for non-zero bits
 * above bitX. The maximum size of the immediate may vary, so use
 * lim_wid at the width limit to use.
 */
int number_is_immediate(ivl_expr_t ex, unsigned lim_wid)
{
      const char*bits;
      unsigned idx;

      assert(ivl_expr_type(ex) == IVL_EX_NUMBER);

      bits = ivl_expr_bits(ex);
      for (idx = lim_wid ;  idx < ivl_expr_width(ex) ;  idx += 1)
	    if (bits[idx] != '0')
		  return 0;

      return 1;
}

unsigned long get_number_immediate(ivl_expr_t ex)
{
      unsigned long imm = 0;
      unsigned idx;

      switch (ivl_expr_type(ex)) {
	  case IVL_EX_ULONG:
	    imm = ivl_expr_uvalue(ex);
	    break;

	  case IVL_EX_NUMBER: {
		const char*bits = ivl_expr_bits(ex);
		unsigned nbits = ivl_expr_width(ex);
		for (idx = 0 ; idx < nbits ; idx += 1) switch (bits[idx]){
		    case '0':
		      break;
		    case '1':
		      imm |= 1 << idx;
		      break;
		    default:
		      assert(0);
		}
		break;
	  }

	  default:
	    assert(0);
      }

      return imm;
}

static struct vector_info draw_eq_immediate(ivl_expr_t exp, unsigned ewid,
					    ivl_expr_t le,
					    ivl_expr_t re)
{
      unsigned wid;
      struct vector_info lv;
      unsigned long imm = get_number_immediate(re);

      wid = ivl_expr_width(le);
      lv = draw_eval_expr_wid(le, wid);

      switch (ivl_expr_opcode(exp)) {
	  case 'E': /* === */
	    fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
		    lv.base, imm, wid);
	    clr_vector(lv);
	    lv.base = 6;
	    lv.wid = 1;
	    break;

	  case 'e': /* == */
	    fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
		    lv.base, imm, wid);
	    clr_vector(lv);
	    lv.base = 4;
	    lv.wid = 1;
	    break;

	  case 'N': /* !== */
	    fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
		    lv.base, imm, wid);
	    clr_vector(lv);
	    lv.base = 6;
	    lv.wid = 1;
	    fprintf(vvp_out, "    %%inv 6, 1;\n");
	    break;

	  case 'n': /* != */
	    fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
		    lv.base, imm, wid);
	    clr_vector(lv);
	    lv.base = 4;
	    lv.wid = 1;
	    fprintf(vvp_out, "    %%inv 4, 1;\n");
	    break;

	  default:
	    assert(0);
      }

	/* Move the result out out the 4-7 bit that the compare
	   uses. This is because that bit may be clobbered by other
	   expressions. */
      { unsigned short base = allocate_vector(ewid);
        fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	lv.base = base;
	lv.wid = ewid;
	if (ewid > 1)
	      fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, ewid-1);
      }

      return lv;
}

static struct vector_info draw_binary_expr_eq(ivl_expr_t exp, unsigned ewid)
{
      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      unsigned wid;

      struct vector_info lv;
      struct vector_info rv;

      if ((ivl_expr_type(re) == IVL_EX_ULONG)
	  && (0 == (ivl_expr_uvalue(re) & ~0xffff)))
	    return draw_eq_immediate(exp, ewid, le, re);

      if ((ivl_expr_type(re) == IVL_EX_NUMBER)
	  && (! number_is_unknown(re))
	  && number_is_immediate(re, 16))
	    return draw_eq_immediate(exp, ewid, le, re);

      wid = ivl_expr_width(le);
      if (ivl_expr_width(re) > wid)
	    wid = ivl_expr_width(re);

      lv = draw_eval_expr_wid(le, wid);
      rv = draw_eval_expr_wid(re, wid);

      switch (ivl_expr_opcode(exp)) {
	  case 'E': /* === */
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/u %u, %u, %u;\n", lv.base,
		    rv.base, lv.wid);
	    clr_vector(lv);
	    clr_vector(rv);
	    lv.base = 6;
	    lv.wid = 1;
	    break;

	  case 'e': /* == */
	    if (lv.wid != rv.wid) {
		  fprintf(stderr,"internal error: operands of == "
			  " have different widths: %u vs %u\n",
			  lv.wid, rv.wid);
	    }
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/u %u, %u, %u;\n", lv.base,
		    rv.base, lv.wid);
	    clr_vector(lv);
	    clr_vector(rv);
	    lv.base = 4;
	    lv.wid = 1;
	    break;

	  case 'N': /* !== */
	    if (lv.wid != rv.wid) {
		  fprintf(stderr,"internal error: operands of !== "
			  " have different widths: %u vs %u\n",
			  lv.wid, rv.wid);
	    }
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/u %u, %u, %u;\n", lv.base,
		    rv.base, lv.wid);
	    fprintf(vvp_out, "    %%inv 6, 1;\n");

	    clr_vector(lv);
	    clr_vector(rv);
	    lv.base = 6;
	    lv.wid = 1;
	    break;

	  case 'n': /* != */
	    if (lv.wid != rv.wid) {
		  fprintf(stderr,"internal error: operands of != "
			  " have different widths: %u vs %u\n",
			  lv.wid, rv.wid);
	    }
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/u %u, %u, %u;\n", lv.base,
		    rv.base, lv.wid);
	    fprintf(vvp_out, "    %%inv 4, 1;\n");

	    clr_vector(lv);
	    clr_vector(rv);
	    lv.base = 4;
	    lv.wid = 1;
	    break;

	  default:
	    assert(0);
      }

	/* Move the result out out the 4-7 bit that the compare
	   uses. This is because that bit may be clobbered by other
	   expressions. */
      { unsigned short base = allocate_vector(ewid);
        fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	lv.base = base;
	lv.wid = ewid;
	if (ewid > 1)
	      fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, ewid-1);
      }

      return lv;
}

static struct vector_info draw_binary_expr_land(ivl_expr_t exp, unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      struct vector_info lv;
      struct vector_info rv;


      lv = draw_eval_expr(le);

      if ((lv.base >= 4) && (lv.wid > 1)) {
	    struct vector_info tmp;
	    clr_vector(lv);
	    tmp.base = allocate_vector(1);
	    tmp.wid = 1;
	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n", tmp.base,
		    lv.base, lv.wid);
	    lv = tmp;
      }

      rv = draw_eval_expr(re);
      if ((rv.base >= 4) && (rv.wid > 1)) {
	    struct vector_info tmp;
	    clr_vector(rv);
	    tmp.base = allocate_vector(1);
	    tmp.wid = 1;
	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n", tmp.base,
		    rv.base, rv.wid);
	    rv = tmp;
      }

      if (lv.base < 4) {
	    if (rv.base < 4) {
		  unsigned lb = lv.base;
		  unsigned rb = rv.base;

		  if ((lb == 0) || (rb == 0)) {
			lv.base = 0;

		  } else if ((lb == 1) && (rb == 1)) {
			lv.base = 1;
		  } else {
			lv.base = 2;
		  }
		  lv.wid = 1;

	    } else {
		  fprintf(vvp_out, "    %%and %u, %u, 1;\n", rv.base, lv.base);
		  lv = rv;
	    }

      } else {
	    fprintf(vvp_out, "    %%and %u, %u, 1;\n", lv.base, rv.base);
	    clr_vector(rv);
      }

	/* If we only want the single bit result, then we are done. */
      if (wid == 1)
	    return lv;

	/* Write the result into a zero-padded result. */
      { unsigned short base = allocate_vector(wid);
        fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	clr_vector(lv);
	lv.base = base;
	lv.wid = wid;
	fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, wid-1);
      }

      return lv;
}

static struct vector_info draw_binary_expr_lor(ivl_expr_t exp, unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      struct vector_info lv;
      struct vector_info rv;

      lv = draw_eval_expr_wid(le, wid);

	/* if the left operand has width, then evaluate the single-bit
	   logical equivilent. */
      if ((lv.base >= 4) && (lv.wid > 1)) {
	    struct vector_info tmp;
	    clr_vector(lv);
	    tmp.base = allocate_vector(1);
	    tmp.wid = 1;
	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n", tmp.base,
		    lv.base, lv.wid);
	    lv = tmp;
      }

      rv = draw_eval_expr_wid(re, wid);

	/* if the right operand has width, then evaluate the single-bit
	   logical equivilent. */
      if ((rv.base >= 4) && (rv.wid > 1)) {
	    struct vector_info tmp;
	    clr_vector(rv);
	    tmp.base = allocate_vector(1);
	    tmp.wid = 1;
	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n", tmp.base,
		    rv.base, rv.wid);
	    rv = tmp;
      }


      if (lv.base < 4) {
	    if (rv.base < 4) {
		  unsigned lb = lv.base;
		  unsigned rb = rv.base;

		  if ((lb == 0) && (rb == 0)) {
			lv.base = 0;

		  } else if ((lb == 1) || (rb == 1)) {
			lv.base = 1;
		  } else {
			lv.base = 2;
		  }

	    } else {
		  fprintf(vvp_out, "    %%or %u, %u, 1;\n", rv.base, lv.base);
		  lv = rv;
	    }

      } else {
	    fprintf(vvp_out, "    %%or %u, %u, 1;\n", lv.base, rv.base);
	    clr_vector(rv);
      }


	/* If we only want the single bit result, then we are done. */
      if (wid == 1)
	    return lv;

	/* Write the result into a zero-padded result. */
      { unsigned short base = allocate_vector(wid);
        fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	clr_vector(lv);
	lv.base = base;
	lv.wid = wid;
	fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, wid-1);
      }

      return lv;
}

static struct vector_info draw_binary_expr_le(ivl_expr_t exp, unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      struct vector_info lv;
      struct vector_info rv;

      char s_flag = (ivl_expr_signed(le) && ivl_expr_signed(re)) ? 's' : 'u';

      unsigned owid = ivl_expr_width(le);
      if (ivl_expr_width(re) > owid)
	    owid = ivl_expr_width(re);

      lv = draw_eval_expr_wid(le, owid);
      rv = draw_eval_expr_wid(re, owid);

      switch (ivl_expr_opcode(exp)) {
	  case 'G':
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/%c %u, %u, %u;\n", s_flag,
		    rv.base, lv.base, lv.wid);
	    fprintf(vvp_out, "    %%or 5, 4, 1;\n");
	    break;

	  case 'L':
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/%c %u, %u, %u;\n", s_flag,
		    lv.base, rv.base, lv.wid);
	    fprintf(vvp_out, "    %%or 5, 4, 1;\n");
	    break;

	  case '<':
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/%c %u, %u, %u;\n", s_flag,
		    lv.base, rv.base, lv.wid);
	    break;

	  case '>':
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/%c %u, %u, %u;\n", s_flag,
		    rv.base, lv.base, lv.wid);
	    break;

	  default:
	    assert(0);
      }

      clr_vector(lv);
      clr_vector(rv);

	/* Move the result out out the 4-7 bit that the compare
	   uses. This is because that bit may be clobbered by other
	   expressions. */
      { unsigned short base = allocate_vector(wid);
        fprintf(vvp_out, "    %%mov %u, 5, 1;\n", base);
	lv.base = base;
	lv.wid = wid;
	if (wid > 1)
	      fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, wid-1);
      }

      return lv;
}

static struct vector_info draw_binary_expr_logic(ivl_expr_t exp,
						 unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      struct vector_info lv;
      struct vector_info rv;

      lv = draw_eval_expr_wid(le, wid);
      rv = draw_eval_expr_wid(re, wid);

	/* The result goes into the left operand, and that is returned
	   as the result. The instructions do not allow the lv value
	   to be a constant bit, so we either switch the operands, or
	   copy the vector into a new area. */
      if (lv.base < 4) {
	    if (rv.base > 4) {
		  struct vector_info tmp = lv;
		  lv = rv;
		  rv = tmp;

	    } else {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(lv.wid);
		  tmp.wid = lv.wid;
		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, lv.base, tmp.wid);
		  lv = tmp;
	    }
      }

      switch (ivl_expr_opcode(exp)) {

	  case '&':
	    fprintf(vvp_out, "   %%and %u, %u, %u;\n",
		    lv.base, rv.base, wid);
	    break;

	  case '|':
	    fprintf(vvp_out, "   %%or %u, %u, %u;\n",
		    lv.base, rv.base, wid);
	    break;

	  case '^':
	    fprintf(vvp_out, "   %%xor %u, %u, %u;\n",
		    lv.base, rv.base, wid);
	    break;

	  case 'A': /* NAND (~&) */
	    fprintf(vvp_out, "    %%nand %u, %u, %u;\n",
		    lv.base, rv.base, wid);
	    break;

	  case 'X': /* exclusive nor (~^) */
	    fprintf(vvp_out, "    %%xnor %u, %u, %u;\n",
		    lv.base, rv.base, wid);
	    break;

	  default:
	    assert(0);
      }

      clr_vector(rv);
      return lv;
}

/*
 * Draw code to evaluate the << expression. Use the %shiftl/i0
 * or %shiftr/i0 instruction to do the real work of shifting. This
 * means that I can handle both left and right shifts in this
 * function, with the only difference the opcode I generate at the
 * end.
 */
static struct vector_info draw_binary_expr_lrs(ivl_expr_t exp, unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      struct vector_info lv;

	/* Figure out the shift amount and load that into the index
	   register. The value may be a constant, or may need to be
	   evaluated at run time. */
      switch (ivl_expr_type(re)) {

	  case IVL_EX_NUMBER: {
		unsigned shift = 0;
		unsigned idx, nbits = ivl_expr_width(re);
		const char*bits = ivl_expr_bits(re);

		for (idx = 0 ;  idx < nbits ;  idx += 1) switch (bits[idx]) {

		    case '0':
		      break;
		    case '1':
		      assert(idx < (8*sizeof shift));
		      shift |= 1 << idx;
		      break;
		    default:
		      assert(0);
		}

		fprintf(vvp_out, "    %%ix/load 0, %u;\n", shift);
		break;
	  }
	    
	  case IVL_EX_ULONG:
	    fprintf(vvp_out, "    %%ix/load 0, %lu;\n", ivl_expr_uvalue(re));
	    break;

	  default: {
		  struct vector_info rv;
		  rv = draw_eval_expr(re);
		  fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n",
			  rv.base, rv.wid);
		  clr_vector(rv);
		  break;
	    }
      }

      lv = draw_eval_expr_wid(le, wid);

      switch (ivl_expr_opcode(exp)) {

	  case 'l': /* << (left shift) */
	    fprintf(vvp_out, "    %%shiftl/i0  %u, %u;\n", lv.base, lv.wid);
	    break;

	  case 'r': /* >> (unsigned right shift) */
	    fprintf(vvp_out, "    %%shiftr/i0  %u, %u;\n", lv.base, lv.wid);
	    break;

	  default:
	    assert(0);
      }

      return lv;
}

static struct vector_info draw_add_immediate(ivl_expr_t le,
					     ivl_expr_t re,
					     unsigned wid)
{
      struct vector_info lv;
      unsigned long imm;

      lv = draw_eval_expr_wid(le, wid);
      assert(lv.wid == wid);

      imm = get_number_immediate(re);

	/* Now generate enough %addi instructions to add the entire
	   immediate value to the destination. The adds are done 16
	   bits at a time, but 17 bits are done to push the carry into
	   the higher bits if needed. */
      { unsigned base;
        for (base = 0 ;  base < lv.wid ;  base += 16) {
	      unsigned long tmp = imm & 0xffffUL;
	      unsigned add_wid = lv.wid - base;

	      imm >>= 16;

	      fprintf(vvp_out, "    %%addi %u, %lu, %u;\n",
		      lv.base+base, tmp, add_wid);

	      if (imm == 0)
		    break;
	}
      }

      return lv;
}

/*
 * The subi is restricted to imm operands that are <= 16 bits wide.
 */
static struct vector_info draw_sub_immediate(ivl_expr_t le,
					     ivl_expr_t re,
					     unsigned wid)
{
      struct vector_info lv;
      unsigned long imm;

      lv = draw_eval_expr_wid(le, wid);
      assert(lv.wid == wid);

      imm = get_number_immediate(re);
      assert( (imm & ~0xffff) == 0 );

      fprintf(vvp_out, "    %%subi %u, %lu, %u;\n", lv.base, imm, wid);

      return lv;
}

static struct vector_info draw_mul_immediate(ivl_expr_t le,
					     ivl_expr_t re,
					     unsigned wid)
{
      struct vector_info lv;
      unsigned long imm;

      lv = draw_eval_expr_wid(le, wid);
      assert(lv.wid == wid);

      imm = get_number_immediate(re);

      fprintf(vvp_out, "    %%muli %u, %lu, %u;\n", lv.base, imm, lv.wid);

      return lv;
}

static struct vector_info draw_binary_expr_arith(ivl_expr_t exp, unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      struct vector_info lv;
      struct vector_info rv;

      const char*sign_string = ivl_expr_signed(exp)? "/s" : "";

      if ((ivl_expr_opcode(exp) == '+')
	  && (ivl_expr_type(re) == IVL_EX_ULONG))
	    return draw_add_immediate(le, re, wid);

      if ((ivl_expr_opcode(exp) == '+')
	  && (ivl_expr_type(re) == IVL_EX_NUMBER)
	  && (! number_is_unknown(re))
	  && number_is_immediate(re, 8*sizeof(unsigned long)))
	    return draw_add_immediate(le, re, wid);

      if ((ivl_expr_opcode(exp) == '-')
	  && (ivl_expr_type(re) == IVL_EX_ULONG))
	    return draw_sub_immediate(le, re, wid);

      if ((ivl_expr_opcode(exp) == '-')
	  && (ivl_expr_type(re) == IVL_EX_NUMBER)
	  && (! number_is_unknown(re))
	  && number_is_immediate(re, 16))
	    return draw_sub_immediate(le, re, wid);

      if ((ivl_expr_opcode(exp) == '*')
	  && (ivl_expr_type(re) == IVL_EX_NUMBER)
	  && (! number_is_unknown(re))
	  && number_is_immediate(re, 16))
	    return draw_mul_immediate(le, re, wid);

      lv = draw_eval_expr_wid(le, wid);
      rv = draw_eval_expr_wid(re, wid);

      assert(lv.wid == wid);
      assert(rv.wid == wid);

	/* The arithmetic instructions replace the left operand with
	   the result. If the left operand is a replicated constant,
	   then I need to make a writeable copy so that the
	   instruction can operate. */
      if (lv.base < 4) {
	    struct vector_info tmp;

	    tmp.base = allocate_vector(wid);
	    tmp.wid = wid;
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n", tmp.base,
		    lv.base, wid);
	    lv = tmp;
      }

      switch (ivl_expr_opcode(exp)) {
	  case '+':
	    fprintf(vvp_out, "    %%add %u, %u, %u;\n", lv.base, rv.base, wid);
	    break;

	  case '-':
	    fprintf(vvp_out, "    %%sub %u, %u, %u;\n", lv.base, rv.base, wid);
	    break;

	  case '*':
	    fprintf(vvp_out, "    %%mul %u, %u, %u;\n", lv.base, rv.base, wid);
	    break;

	  case '/':
	    fprintf(vvp_out, "    %%div%s %u, %u, %u;\n", sign_string,
		    lv.base, rv.base, wid);
	    break;

	  case '%':
	    fprintf(vvp_out, "    %%mod %u, %u, %u;\n", lv.base, rv.base, wid);
	    break;

	  default:
	    assert(0);
      }

      clr_vector(rv);

      return lv;
}

static struct vector_info draw_binary_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info rv;

      switch (ivl_expr_opcode(exp)) {
	  case 'a': /* && (logical and) */
	    rv = draw_binary_expr_land(exp, wid);
	    break;

	  case 'E': /* === */
	  case 'e': /* == */
	  case 'N': /* !== */
	  case 'n': /* != */
	    rv = draw_binary_expr_eq(exp, wid);
	    break;

	  case '<':
	  case '>':
	  case 'L': /* <= */
	  case 'G': /* >= */
	    rv = draw_binary_expr_le(exp, wid);
	    break;

	  case '+':
	  case '-':
	  case '*':
	  case '/':
	  case '%':
	    rv = draw_binary_expr_arith(exp, wid);
	    break;

	  case 'l': /* << */
	  case 'r': /* >> */
	    rv = draw_binary_expr_lrs(exp, wid);
	    break;

	  case 'o': /* || (logical or) */
	    rv = draw_binary_expr_lor(exp, wid);
	    break;

	  case '&':
	  case '|':
	  case '^':
	  case 'A': /* NAND (~&) */
	  case 'X': /* XNOR (~^) */
	    rv = draw_binary_expr_logic(exp, wid);
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt error: unsupported binary (%c)\n",
		    ivl_expr_opcode(exp));
	    assert(0);
      }

      return rv;
}

static struct vector_info draw_bitsel_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info res;
      ivl_signal_t sig = ivl_expr_signal(exp);
      ivl_expr_t sel = ivl_expr_oper1(exp);

	/* Evaluate the bit select expression and save the result into
	   index register 0. */
      res = draw_eval_expr(sel);
      fprintf(vvp_out, "    %%ix/get 0, %u,%u;\n", res.base, res.wid);
      clr_vector(res);

      res.base = allocate_vector(wid);
      res.wid = wid;

      switch (ivl_signal_type(sig)) {
	  case IVL_SIT_TRI:
	  case IVL_SIT_TRI0:
	  case IVL_SIT_TRI1:
	    fprintf(vvp_out, "    %%load/nx %u, V_%s, 0;\n", res.base,
		    vvp_signal_label(sig));
	    break;
	  default:
	    fprintf(vvp_out, "    %%load/x %u, V_%s, 0;\n", res.base,
		    vvp_signal_label(sig));
	    break;
      }

      return res;
}

/*
 * The concatenation operator is evaluated by evaluating each sub-
 * expression, then copying it into the continguous vector of the
 * result. Do this until the result vector is filled.
 */
static struct vector_info draw_concat_expr(ivl_expr_t exp, unsigned wid)
{
      unsigned off, rep;
      struct vector_info res;

	/* Allocate a vector to hold the result. */
      res.base = allocate_vector(wid);
      res.wid = wid;

	/* Get the repeat count. This must be a constant that has been
	   evaluated at compile time. The operands will be repeated to
	   form the result. */
      rep = ivl_expr_repeat(exp);
      off = 0;

      while (rep > 0) {

	      /* Each repeat, evaluate the sub-expressions, from lsb
		 to msb, and copy each into the result vector. The
		 expressions are arranged in the concatenation from
		 MSB to LSB, to go through them backwards.

		 Abort the loop if the result vector gets filled up. */

	    unsigned idx = ivl_expr_parms(exp);
	    while ((idx > 0) && (off < wid)) {
		  ivl_expr_t arg = ivl_expr_parm(exp, idx-1);
		  unsigned awid = ivl_expr_width(arg);

		    /* Evaluate this sub expression. */
		  struct vector_info avec = draw_eval_expr_wid(arg, awid);

		  unsigned trans = awid;
		  if ((off + awid) > wid)
			trans = wid - off;

		  assert(awid == avec.wid);

		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n", res.base+off,
			  avec.base, trans);
		  clr_vector(avec);

		  idx -= 1;
		  off += trans;
		  assert(off <= wid);
	    }
	    rep -= 1;
      }

	/* Pad the result with 0, if necessary. */
      if (off < wid) {
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
		    res.base+off, wid-off);
      }

      return res;
}

/*
 * A number in an expression is made up by copying constant bits into
 * the allocated vector.
 */
static struct vector_info draw_number_expr(ivl_expr_t exp, unsigned wid)
{
      unsigned idx;
      unsigned nwid;
      struct vector_info res;
      const char*bits = ivl_expr_bits(exp);

      res.wid  = wid;

      nwid = wid;
      if (ivl_expr_width(exp) < nwid)
	    nwid = ivl_expr_width(exp);

	/* If all the bits of the number have the same value, then we
	   can use a constant bit. There is no need to allocate wr
	   bits, and there is no need to generate any code. */

      for (idx = 1 ;  idx < nwid ;  idx += 1) {
	    if (bits[idx] != bits[0])
		  break;
      }

      if (idx >= res.wid) {
	    switch (bits[0]) {
		case '0':
		  res.base = 0;
		  break;
		case '1':
		  res.base = 1;
		  break;
		case 'x':
		  res.base = 2;
		  break;
		case 'z':
		  res.base = 3;
		  break;
	    }
	    return res;
      }

	/* The number value needs to be represented as an allocated
	   vector. Allocate the vector and use %mov instructions to
	   load the constant bit values. */
      res.base = allocate_vector(wid);

      idx = 0;
      while (idx < nwid) {
	    unsigned cnt;
	    char src = '?';
	    switch (bits[idx]) {
		case '0':
		  src = '0';
		  break;
		case '1':
		  src = '1';
		  break;
		case 'x':
		  src = '2';
		  break;
		case 'z':
		  src = '3';
		  break;
	    }

	    for (cnt = 1 ;  idx+cnt < wid ;  cnt += 1)
		  if (bits[idx+cnt] != bits[idx])
			break;

	    fprintf(vvp_out, "    %%mov %u, %c, %u;\n",
		    res.base+idx, src, cnt);

	    idx += cnt;
      }

	/* Pad the number up to the expression width. */
      if (idx < wid)
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n", res.base+idx, wid-idx);

      return res;
}

/*
 * A string in an expression is made up by copying constant bits into
 * the allocated vector.
 */
static struct vector_info draw_string_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info res;
      const char *p = ivl_expr_string(exp);
      unsigned ewid, nwid;
      unsigned bit = 0, idx;

      res.wid  = wid;
      nwid = wid;
      ewid = ivl_expr_width(exp);
      if (ewid < nwid)
	    nwid = ewid;

      p += (ewid / 8) - 1;

	/* The string needs to be represented as an allocated
	   vector. Allocate the vector and use %mov instructions to
	   load the constant bit values. */
      res.base = allocate_vector(wid);

      idx = 0;
      while (idx < nwid) {
	    unsigned this_bit = ((*p) & (1 << bit)) ? 1 : 0;

	    fprintf(vvp_out, "    %%mov %u, %d, 1;\n",
		    res.base+idx, this_bit);

	    bit++;
	    if (bit == 8) {
		  bit = 0;
		  p--;
	    }

	    idx++;
      }

	/* Pad the number up to the expression width. */
      if (idx < wid)
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n", res.base+idx, wid-idx);

      return res;
}

/*
 * Evaluating a signal expression means loading the bits of the signal
 * into the thread bits. Remember to account for the part select by
 * offsetting the read from the lsi (least significant index) of the
 * signal.
 */
static struct vector_info draw_signal_expr(ivl_expr_t exp, unsigned wid)
{
      unsigned idx;
      unsigned lsi = ivl_expr_lsi(exp);
      unsigned swid = ivl_expr_width(exp);
      ivl_signal_t sig = ivl_expr_signal(exp);
      struct vector_info res;

      if (swid > wid)
	    swid = wid;

      res.base = allocate_vector(wid);
      res.wid  = wid;

      for (idx = 0 ;  idx < swid ;  idx += 1)
	    fprintf(vvp_out, "    %%load  %u, V_%s[%u];\n",
		    res.base+idx, vvp_signal_label(sig), idx+lsi);

	/* Pad the signal value with zeros. */
      if (swid < wid)
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
		    res.base+swid, wid-swid);
      return res;
}

void draw_memory_index_expr(ivl_memory_t mem, ivl_expr_t ae)
{
      int root = ivl_memory_root(mem);
      unsigned width = ivl_memory_width(mem);
      width = (width+3) & ~3;

      switch (ivl_expr_type(ae)) {
	  case IVL_EX_NUMBER: {
		unsigned nbits = ivl_expr_width(ae);
		const char*bits = ivl_expr_bits(ae);
		unsigned long v = 0;
		int unknown_flag = 0;
		unsigned idx;
		for (idx = 0 ;  idx < nbits ;  idx += 1) 
		      switch (bits[idx]) {
			  case '0':
			    break;
			  case '1':
			    assert(idx < (8*sizeof v));
			    v |= 1 << idx;
			    break;
			  default:
			    v = ~0UL;
			    unknown_flag = 1;
			    break;
		      }
		fprintf(vvp_out, "    %%ix/load 3, %lu;\n", (v-root)*width);
		fprintf(vvp_out, "    %%mov 4, %c, 1;\n",
			unknown_flag?'1':'0');
		break;
	  }
	  case IVL_EX_ULONG: {
		unsigned v = ivl_expr_uvalue(ae); 
		fprintf(vvp_out, "    %%ix/load 3, %u;\n", (v-root)*width);
		fprintf(vvp_out, "    %%mov 4, 0, 1;\n");
		break;
	  }
	  default: {
		struct vector_info addr = draw_eval_expr(ae);
		fprintf(vvp_out, "    %%ix/get 3, %u, %u;\n",
			addr.base, addr.wid);
		clr_vector(addr);
		if (root>0)
		      fprintf(vvp_out, "    %%ix/sub 3, %u;\n", root);
		if (width>1)
		      fprintf(vvp_out, "    %%ix/mul 3, %u;\n", width);
		break;
	  }
      }
}

static struct vector_info draw_memory_expr(ivl_expr_t exp, unsigned wid)
{
      unsigned swid = ivl_expr_width(exp);
      ivl_memory_t mem = ivl_expr_memory(exp);
      struct vector_info res;
      unsigned idx;

      draw_memory_index_expr(mem, ivl_expr_oper1(exp));

      if (swid > wid)
	    swid = wid;
      
      res.base = allocate_vector(wid);
      res.wid  = wid;

      for (idx = 0 ;  idx < swid ;  idx += 1) {
	    if (idx)
		  fprintf(vvp_out, "    %%ix/add 3, 1;\n");
	    fprintf(vvp_out, "    %%load/m  %u, M_%s;\n",
		    res.base+idx, vvp_memory_label(mem));
      }

	/* Pad the signal value with zeros. */
      if (swid < wid)
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
		    res.base+swid, wid-swid);

      return res;
}

static struct vector_info draw_select_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info subv, shiv, res;
      ivl_expr_t sube  = ivl_expr_oper1(exp);
      ivl_expr_t shift = ivl_expr_oper2(exp);

	/* Evaluate the sub-expression. */
      subv = draw_eval_expr(sube);

	/* Any bit select of a constant zero is another constant zero,
	   so short circuit and return the value we know. */
      if (subv.base == 0) {
	    subv.wid = wid;
	    return subv;
      }

	/* Evaluate the bit select base expression and store the
	   result into index register 0. */
      shiv = draw_eval_expr(shift);
      fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", shiv.base, shiv.wid);
      clr_vector(shiv);

	/* If the subv result is a magic constant, then make a copy in
	   writeable vector space and work from there instead. */
      if (subv.base < 4) {
	    res.base = allocate_vector(subv.wid);
	    res.wid = wid;
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n", res.base,
		    subv.base, res.wid);
	    subv = res;
      }

      fprintf(vvp_out, "    %%shiftr/i0 %u, %u;\n", subv.base, subv.wid);

      assert(subv.wid >= wid);
      if (subv.wid > wid) {
	    res.base = subv.base;
	    res.wid = wid;

	    subv.base += wid;
	    clr_vector(subv);

      } else if (subv.wid == wid) {
	    res = subv;
      }

      return res;
}

static struct vector_info draw_ternary_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info res, tmp;

      unsigned lab_true, lab_false, lab_out;
      ivl_expr_t cond = ivl_expr_oper1(exp);
      ivl_expr_t true_ex = ivl_expr_oper2(exp);
      ivl_expr_t false_ex = ivl_expr_oper3(exp);

      lab_true  = local_count++;
      lab_false = local_count++;
      lab_out = local_count++;

      tmp = draw_eval_expr(cond);
      clr_vector(tmp);

      if ((tmp.base >= 4) && (tmp.wid > 1)) {
	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n",
		    tmp.base, tmp.base, tmp.wid);
	    tmp.wid = 1;
      }

      res.base = allocate_vector(wid);
      res.wid  = wid;

      fprintf(vvp_out, "    %%jmp/1  T_%d.%d, %u;\n",
	      thread_count, lab_true, tmp.base);
      fprintf(vvp_out, "    %%jmp/0  T_%d.%d, %u;\n",
	      thread_count, lab_false, tmp.base);

	/* Ambiguous case. Evaluate both true and false expressions,
	   and use %blend to merge them. */

      tmp = draw_eval_expr_wid(true_ex, wid);
      fprintf(vvp_out, "    %%mov  %u, %u, %u;\n", res.base, tmp.base, wid);
      clr_vector(tmp);

      tmp = draw_eval_expr_wid(false_ex, wid);
      fprintf(vvp_out, "    %%blend  %u, %u, %u;\n", res.base, tmp.base, wid);
      fprintf(vvp_out, "    %%jmp  T_%d.%d;\n", thread_count, lab_out);
      if (tmp.base >= 8)
	    clr_vector(tmp);

	/* This is the true case. Just evaluate the true expression. */
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_true);

      tmp = draw_eval_expr_wid(true_ex, wid);
      fprintf(vvp_out, "    %%mov  %u, %u, %u;\n", res.base, tmp.base, wid);
      fprintf(vvp_out, "    %%jmp  T_%d.%d;\n", thread_count, lab_out);
      if (tmp.base >= 8)
	    clr_vector(tmp);


	/* This is the false case. Just evaluate the false expression. */
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_false);

      tmp = draw_eval_expr_wid(false_ex, wid);
      fprintf(vvp_out, "    %%mov  %u, %u, %u;\n", res.base, tmp.base, wid);
      if (tmp.base >= 8)
	    clr_vector(tmp);

	/* This is the out label. */
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_out);

      return res;
}

static struct vector_info draw_sfunc_expr(ivl_expr_t exp, unsigned wid)
{
      unsigned idx;
      struct vector_info *vec = 0x0;
      unsigned int vecs= 0;
      unsigned int veci= 0;

      unsigned parm_count = ivl_expr_parms(exp);
      struct vector_info res;


	/* If the function has no parameters, then use this short-form
	   to draw the statement. */
      if (parm_count == 0) {
	    res.base = allocate_vector(wid);
	    res.wid  = wid;
	    fprintf(vvp_out, "    %%vpi_func \"%s\", %u, %u;\n",
		    ivl_expr_name(exp), res.base, res.wid);
	    return res;

      }

	/* Evaluate any expressions that need to be evaluated by the
	   run-time before passed to the system task. The results are
	   stored in the vec array. */
      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    ivl_expr_t expr = ivl_expr_parm(exp, idx);
	    
	    switch (ivl_expr_type(expr)) {
		case IVL_EX_NONE:
		case IVL_EX_NUMBER:
		case IVL_EX_SIGNAL:
		case IVL_EX_STRING:
		case IVL_EX_SCOPE:
		case IVL_EX_SFUNC:
		  continue;
		case IVL_EX_MEMORY:
		  if (!ivl_expr_oper1(expr)) {
			continue;
		  }
		default:
		  break;
	    }

	    vec = (struct vector_info *)
		  realloc(vec, (vecs+1)*sizeof(struct vector_info));
	    vec[vecs] = draw_eval_expr(expr);
	    vecs++;
      }

	/* Start the complicated expression. */

      res.base = allocate_vector(wid);
      res.wid  = wid;
      fprintf(vvp_out, "    %%vpi_func \"%s\", %u, %u",
	      ivl_expr_name(exp), res.base, res.wid);

	/* Now draw all the parameters to the function. */
      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    ivl_expr_t expr = ivl_expr_parm(exp, idx);

	    switch (ivl_expr_type(expr)) {
		case IVL_EX_NONE:
		  fprintf(vvp_out, ", \" \"");
		  continue;

		case IVL_EX_NUMBER: {
		      unsigned bit, wid = ivl_expr_width(expr);
		      const char*bits = ivl_expr_bits(expr);

		      fprintf(vvp_out, ", %u'b", wid);
		      for (bit = wid ;  bit > 0 ;  bit -= 1)
			    fputc(bits[bit-1], vvp_out);
		      continue;
		}

		case IVL_EX_SIGNAL:
		  fprintf(vvp_out, ", V_%s", 
			  vvp_signal_label(ivl_expr_signal(expr)));
		  continue;

		case IVL_EX_STRING:
		  fprintf(vvp_out, ", \"%s\"", 
			  ivl_expr_string(expr));
		  continue;

		case IVL_EX_SCOPE:
		  fprintf(vvp_out, ", S_%s",
			  vvp_mangle_id(ivl_scope_name(ivl_expr_scope(expr))));
		  continue;

		case IVL_EX_SFUNC:
		  if (strcmp("$time", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $time");
		  else if (strcmp("$stime", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $stime");
		  else
			fprintf(vvp_out, ", ?");
		  continue;
		  
		case IVL_EX_MEMORY:
		  if (!ivl_expr_oper1(expr)) {
			fprintf(vvp_out, ", M_%s", 
				vvp_memory_label(ivl_expr_memory(expr)));
			continue;
		  }
		  break;

		default:
		  break;
	    }
	    
	    fprintf(vvp_out, ", T<%u,%u>", 
		    vec[veci].base, 
		    vec[veci].wid);
	    veci++;
      }
      
      assert(veci == vecs);

      if (vecs) {
	    for (idx = 0; idx < vecs; idx++)
		  clr_vector(vec[idx]);
	    free(vec);
      }

      fprintf(vvp_out, ";\n");


      return res;
}

/*
 * A call to a user defined function generates a result that is the
 * result of this expression.
 *
 * The result of the function is placed by the function execution into
 * a signal within the scope of the function that also has a basename
 * the same as the function. The ivl_target API handled the result
 * mapping already, and we get the name of the result signal as
 * parameter 0 of the function definition.
 */

static struct vector_info draw_ufunc_expr(ivl_expr_t exp, unsigned wid)
{
      unsigned idx;
      unsigned swid = ivl_expr_width(exp);
      ivl_scope_t def = ivl_expr_def(exp);
      ivl_signal_t retval = ivl_scope_port(def, 0);
      struct vector_info res;

	/* evaluate the expressions and send the results to the
	   function ports. */

      assert(ivl_expr_parms(exp) == (ivl_scope_ports(def)-1));
      for (idx = 0 ;  idx < ivl_expr_parms(exp) ;  idx += 1) {
	    ivl_signal_t port = ivl_scope_port(def, idx+1);
	    unsigned pin, bit;

	    res = draw_eval_expr_wid(ivl_expr_parm(exp, idx),
				     ivl_signal_pins(port));
	    bit = res.base;
	    assert(res.wid <= ivl_signal_pins(port));
	    for (pin = 0 ;  pin < res.wid ;  pin += 1) {
		  fprintf(vvp_out, "    %%set V_%s[%u], %u;\n",
			  vvp_signal_label(port),
			  pin, bit);
		  if (bit >= 4)
			bit += 1;
	    }

	    clr_vector(res);
      }


	/* Call the function */
      fprintf(vvp_out, "    %%fork TD_%s", vvp_mangle_id(ivl_scope_name(def)));
      fprintf(vvp_out, ", S_%s;\n", vvp_mangle_id(ivl_scope_name(def)));
      fprintf(vvp_out, "    %%join;\n");

	/* The return value is in a signal that has the name of the
	   expression. Load that into the thread and return the
	   vector result. */

      res.base = allocate_vector(wid);
      res.wid  = wid;

      { unsigned load_wid = swid;
        if (load_wid > ivl_signal_pins(retval))
	      load_wid = ivl_signal_pins(retval);

	for (idx = 0 ;  idx < load_wid ;  idx += 1)
	      fprintf(vvp_out, "    %%load  %u, V_%s[%u];\n",
		      res.base+idx,
		      vvp_signal_label(retval), idx);

	if (load_wid < swid)
	      fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
		      res.base+load_wid, swid-load_wid);
      }

	/* Pad the signal value with zeros. */
      if (swid < wid)
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
		    res.base+swid, wid-swid);

      return res;
}

static struct vector_info draw_unary_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info res;
      ivl_expr_t sub = ivl_expr_oper1(exp);
      const char *rop = 0;
      int inv = 0;

      switch (ivl_expr_opcode(exp)) {
	  case '&': rop = "and";  break;
	  case '|': rop = "or";   break;
	  case '^': rop = "xor";  break;
	  case 'A': rop = "nand"; break;
	  case 'N': rop = "nor";  break;
	  case 'X': rop = "xnor"; break;
      }

      switch (ivl_expr_opcode(exp)) {
	  case '~':
	    res = draw_eval_expr_wid(sub, wid);
	    switch (res.base) {
		case 0:
		  res.base = 1;
		  break;
		case 1:
		  res.base = 0;
		  break;
		case 2:
		case 3:
		  res.base = 2;
		  break;
		default:
		  fprintf(vvp_out, "    %%inv %u, %u;\n", res.base, res.wid);
		  break;
	    }
	    break;

	  case '-':
	      /* Unary minus is implemented by generating the 2's
		 complement of the number. That is the 1's complement
		 (bitwise invert) with a 1 added in. Note that the
		 %sub subtracts -1 (1111...) to get %add of +1. */
	    res = draw_eval_expr_wid(sub, wid);
	    switch (res.base) {
		case 0:
		  res.base = 0;
		  break;
		case 1:
		  res.base = 1;
		  break;
		case 2:
		case 3:
		  res.base = 2;
		  break;
		default:
		  fprintf(vvp_out, "    %%inv %u, %u;\n", res.base, res.wid);
		  fprintf(vvp_out, "    %%sub %u, 1, %u;\n",res.base,res.wid);
		  break;
	    }
	    break;

	  case '!':
	    res = draw_eval_expr(sub);
	    if (res.wid > 1) {
		    /* a ! on a vector is implemented with a reduction
		       nor. Generate the result into the first bit of
		       the input vector and free the rest of the
		       vector. */
		  struct vector_info tmp;
		  assert(res.base >= 4);
		  tmp.base = res.base+1;
		  tmp.wid = res.wid - 1;
		  fprintf(vvp_out, "    %%nor/r %u, %u, %u;\n",
			  res.base, res.base, res.wid);
		  clr_vector(tmp);
		  res.wid = 1;
	    } else switch (res.base) {
		case 0:
		  res.base = 1;
		  break;
		case 1:
		  res.base = 0;
		  break;
		case 2:
		case 3:
		  res.base = 2;
		  break;
		default:
		  fprintf(vvp_out, "    %%inv %u, 1;\n", res.base);
		  break;
	    }

	      /* If the result needs to be bigger then the calculated
		 value, then write it into a padded vector. */
	    if (res.wid < wid) {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(wid);
		  tmp.wid = wid;
		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, res.base, res.wid);
		  fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
			  tmp.base+res.wid, tmp.wid-res.wid);
		  clr_vector(res);
		  res = tmp;
	    }
	    break;

	  case 'N':
	  case 'A':
	  case 'X':
	    inv = 1;
	  case '&':
	  case '|':
	  case '^':
	    res = draw_eval_expr(sub);
	    if (res.wid > 1) {
		  struct vector_info tmp;
		    /* If the previous result is in the constant area
		       (and is a vector) then copy it out into some
		       temporary space. */
		  if (res.base < 4) {
			tmp.base = allocate_vector(res.wid);
			tmp.wid = res.wid;
			fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
				tmp.base, res.base, res.wid);
			res = tmp;
		  }

		  tmp.base = res.base+1;
		  tmp.wid = res.wid - 1;
		  fprintf(vvp_out, "    %%%s/r %u, %u, %u;\n",
			  rop,
			  res.base, res.base, res.wid);
		  clr_vector(tmp);
		  res.wid = 1;
	    } else if (inv) {
		  assert(res.base >= 4);
		  fprintf(vvp_out, "    %%inv %u, 1;\n", res.base);
	    }

	      /* If the result needs to be bigger then the calculated
		 value, then write it into a passed vector. */
	    if (res.wid < wid) {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(wid);
		  tmp.wid = wid;
		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, res.base, res.wid);
		  fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
			  tmp.base+res.wid, tmp.wid-res.wid);
		  clr_vector(res);
		  res = tmp;
	    }
	    break;

	  default:
	    fprintf(stderr, "vvp error: unhandled unary: %c\n",
		    ivl_expr_opcode(exp));
	    assert(0);
      }

      return res;
}

struct vector_info draw_eval_expr_wid(ivl_expr_t exp, unsigned wid)
{
      struct vector_info res;

      switch (ivl_expr_type(exp)) {
	  default:
	    fprintf(stderr, "vvp error: unhandled expr type: %u\n",
	    ivl_expr_type(exp));
	  case IVL_EX_NONE:
	    assert(0);
	    res.base = 0;
	    res.wid = 0;
	    break;

	  case IVL_EX_STRING:
	    res = draw_string_expr(exp, wid);
	    break;

	  case IVL_EX_BINARY:
	    res = draw_binary_expr(exp, wid);
	    break;

	  case IVL_EX_BITSEL:
	    res = draw_bitsel_expr(exp, wid);
	    break;

	  case IVL_EX_CONCAT:
	    res = draw_concat_expr(exp, wid);
	    break;

	  case IVL_EX_NUMBER:
	    res = draw_number_expr(exp, wid);
	    break;

	  case IVL_EX_SELECT:
	    res = draw_select_expr(exp, wid);
	    break;

	  case IVL_EX_SIGNAL:
	    res = draw_signal_expr(exp, wid);
	    break;

	  case IVL_EX_TERNARY:
	    res = draw_ternary_expr(exp, wid);
	    break;

	  case IVL_EX_MEMORY:
	    res = draw_memory_expr(exp, wid);
	    break;

	  case IVL_EX_SFUNC:
	    res = draw_sfunc_expr(exp, wid);
	    break;

	  case IVL_EX_UFUNC:
	    res = draw_ufunc_expr(exp, wid);
	    break;

	  case IVL_EX_UNARY:
	    res = draw_unary_expr(exp, wid);
	    break;
      }

      return res;
}

struct vector_info draw_eval_expr(ivl_expr_t exp)
{
      return draw_eval_expr_wid(exp, ivl_expr_width(exp));
}

/*
 * $Log: eval_expr.c,v $
 * Revision 1.75  2002/09/12 15:49:43  steve
 *  Add support for binary nand operator.
 *
 * Revision 1.74  2002/09/01 01:42:34  steve
 *  Fix leaking vthread bits in ?: eval.
 *
 * Revision 1.73  2002/08/28 18:38:07  steve
 *  Add the %subi instruction, and use it where possible.
 *
 * Revision 1.72  2002/08/28 17:15:35  steve
 *  Generate %load/nx for indexed load of nets.
 *
 * Revision 1.71  2002/08/27 05:39:57  steve
 *  Fix l-value indexing of memories and vectors so that
 *  an unknown (x) index causes so cell to be addresses.
 *
 *  Fix tangling of label identifiers in the fork-join
 *  code generator.
 *
 * Revision 1.70  2002/08/22 03:38:40  steve
 *  Fix behavioral eval of x?a:b expressions.
 *
 * Revision 1.69  2002/08/12 01:35:03  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.68  2002/08/05 04:18:45  steve
 *  Store only the base name of memories.
 *
 * Revision 1.67  2002/08/04 18:28:15  steve
 *  Do not use hierarchical names of memories to
 *  generate vvp labels. -tdll target does not
 *  used hierarchical name string to look up the
 *  memory objects in the design.
 *
 * Revision 1.66  2002/08/03 22:30:48  steve
 *  Eliminate use of ivl_signal_name for signal labels.
 *
 * Revision 1.65  2002/07/12 18:10:45  steve
 *  Use all bits of ?: condit expression.
 *
 * Revision 1.64  2002/07/01 00:52:47  steve
 *  Carry can propagate to the otp in addi.
 *
 * Revision 1.63  2002/06/02 18:57:17  steve
 *  Generate %cmpi/u where appropriate.
 *
 * Revision 1.62  2002/05/31 20:04:57  steve
 *   Generate %muli instructions when possible.
 *
 * Revision 1.61  2002/05/30 01:57:23  steve
 *  Use addi with wide immediate values.
 *
 * Revision 1.60  2002/05/29 16:29:34  steve
 *  Add %addi, which is faster to simulate.
 *
 * Revision 1.59  2002/05/07 03:49:58  steve
 *  Handle x case of unary ! properly.
 *
 * Revision 1.58  2002/04/22 02:41:30  steve
 *  Reduce the while loop expression if needed.
 *
 * Revision 1.57  2002/04/14 18:41:34  steve
 *  Support signed integer division.
 *
 * Revision 1.56  2002/02/03 05:53:00  steve
 *  Fix parameter bit select check for magic constants.
 *
 * Revision 1.55  2002/01/28 00:52:42  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.54  2002/01/11 05:23:05  steve
 *  Handle certain special cases of stime.
 *
 * Revision 1.53  2001/11/19 04:25:46  steve
 *  Handle padding out of logical values.
 *
 * Revision 1.52  2001/10/24 05:06:54  steve
 *  The ! expression returns 0 to x and z values.
 *
 * Revision 1.51  2001/10/18 16:41:49  steve
 *  Evaluate string expressions (Philip Blundell)
 *
 * Revision 1.50  2001/10/16 01:27:17  steve
 *  Generate %div instructions for binary /.
 *
 * Revision 1.49  2001/10/14 03:24:35  steve
 *  Handle constant bits in arithmetic expressions.
 *
 * Revision 1.48  2001/10/10 04:47:43  steve
 *  Support vectors as operands to logical and.
 *
 * Revision 1.47  2001/09/29 04:37:44  steve
 *  Generate code for unary minus (PR#272)
 *
 * Revision 1.46  2001/09/29 01:53:22  steve
 *  Fix the size of unsized constant operants to compare (PR#274)
 *
 * Revision 1.45  2001/09/20 03:46:38  steve
 *  Handle short l-values to concatenation.
 *
 * Revision 1.44  2001/09/15 18:27:04  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.43  2001/08/31 01:37:56  steve
 *  Handle update in place of repeat constants.
 *
 * Revision 1.42  2001/08/23 02:54:15  steve
 *  Handle wide assignment to narrow return value.
 *
 * Revision 1.41  2001/08/03 17:06:10  steve
 *  More detailed messages about unsupported things.
 *
 * Revision 1.40  2001/07/27 04:51:45  steve
 *  Handle part select expressions as variants of
 *  NetESignal/IVL_EX_SIGNAL objects, instead of
 *  creating new and useless temporary signals.
 *
 * Revision 1.39  2001/07/27 02:41:56  steve
 *  Fix binding of dangling function ports. do not elide them.
 */

