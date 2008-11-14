/*
 * Copyright (c) 2001-2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: eval_expr.c,v 1.110.2.2 2007/02/26 19:51:40 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <assert.h>

static void draw_eval_expr_dest(ivl_expr_t exp, struct vector_info dest,
				int ok_flags);

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

/*
 * The STUFF_OK_XZ bit is true if the output is going to be further
 * processed so that x and z values are equivalent. This may allow for
 * new optimizations.
 */
static struct vector_info draw_eq_immediate(ivl_expr_t exp, unsigned ewid,
					    ivl_expr_t le,
					    ivl_expr_t re,
					    int stuff_ok_flag)
{
      unsigned wid;
      struct vector_info lv;
      unsigned long imm = get_number_immediate(re);

      wid = ivl_expr_width(le);
      lv = draw_eval_expr_wid(le, wid, stuff_ok_flag);

      switch (ivl_expr_opcode(exp)) {
	  case 'E': /* === */
	    fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
		    lv.base, imm, wid);
	    if (lv.base >= 8)
		  clr_vector(lv);
	    lv.base = 6;
	    lv.wid = 1;
	    break;

	  case 'e': /* == */
	      /* If this is a single bit being compared to 1, and the
		 output doesn't care about x vs z, then just return
		 the value itself. */
	    if ((stuff_ok_flag&STUFF_OK_XZ) && (lv.wid == 1) && (imm == 1))
		  break;

	    fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
		    lv.base, imm, wid);
	    if (lv.base >= 8)
		  clr_vector(lv);
	    lv.base = 4;
	    lv.wid = 1;
	    break;

	  case 'N': /* !== */
	    fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
		    lv.base, imm, wid);
	    if (lv.base >= 8)
		  clr_vector(lv);
	    lv.base = 6;
	    lv.wid = 1;
	    fprintf(vvp_out, "    %%inv 6, 1;\n");
	    break;

	  case 'n': /* != */
	      /* If this is a single bit being compared to 0, and the
		 output doesn't care about x vs z, then just return
		 the value itself. */
	    if ((stuff_ok_flag&STUFF_OK_XZ) && (lv.wid == 1) && (imm == 0))
		  break;

	    fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
		    lv.base, imm, wid);
	    if (lv.base >= 8)
		  clr_vector(lv);
	    lv.base = 4;
	    lv.wid = 1;
	    fprintf(vvp_out, "    %%inv 4, 1;\n");
	    break;

	  default:
	    assert(0);
      }

	/* In the special case that 47 bits are ok, and this really is
	   a single bit value, then we are done. */
      if ((lv.wid == 1) && (ewid == 1) && (stuff_ok_flag&STUFF_OK_47))
	    return lv;

	/* Move the result out out the 4-7 bit that the compare
	   uses. This is because that bit may be clobbered by other
	   expressions. */
      if (lv.base < 8) {
	    unsigned base = allocate_vector(ewid);
	    fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	    lv.base = base;
	    lv.wid = ewid;
	    if (ewid > 1)
		  fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, ewid-1);

      } else if (lv.wid < ewid) {
	    unsigned base = allocate_vector(ewid);
	    if (lv.base >= 8)
		  clr_vector(lv);
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n", base,
		    lv.base, lv.wid);
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
		    base+lv.wid, ewid-lv.wid);
	    lv.base = base;
	    lv.wid = ewid;
      }

      return lv;
}

/*
 * This handles the special case that the operands of the comparison
 * are real valued expressions.
 */
static struct vector_info draw_binary_expr_eq_real(ivl_expr_t exp)
{
      struct vector_info res;
      int lword, rword;

      res.base = allocate_vector(1);
      res.wid  = 1;

      lword = draw_eval_real(ivl_expr_oper1(exp));
      rword = draw_eval_real(ivl_expr_oper2(exp));

      clr_word(lword);
      clr_word(rword);

      fprintf(vvp_out, "    %%cmp/wr %d, %d;\n", lword, rword);
      switch (ivl_expr_opcode(exp)) {

	  case 'e':
	    fprintf(vvp_out, "    %%mov %u, 4, 1;\n", res.base);
	    break;

	  case 'n': /* != */
	    fprintf(vvp_out, "    %%mov %u, 4, 1;\n", res.base);
	    fprintf(vvp_out, "    %%inv %u, 1;\n", res.base);
	    break;

	  default:
	    assert(0);
      }

      return res;
}

static struct vector_info draw_binary_expr_eq(ivl_expr_t exp,
					      unsigned ewid,
					      int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      unsigned wid;

      struct vector_info lv;
      struct vector_info rv;

      if ((ivl_expr_value(le) == IVL_VT_REAL)
	  ||(ivl_expr_value(re) == IVL_VT_REAL))  {
	    return draw_binary_expr_eq_real(exp);
      }

      if ((ivl_expr_type(re) == IVL_EX_ULONG)
	  && (0 == (ivl_expr_uvalue(re) & ~0xffff)))
	    return draw_eq_immediate(exp, ewid, le, re, stuff_ok_flag);

      if ((ivl_expr_type(re) == IVL_EX_NUMBER)
	  && (! number_is_unknown(re))
	  && number_is_immediate(re, 16))
	    return draw_eq_immediate(exp, ewid, le, re, stuff_ok_flag);

      assert(ivl_expr_value(le) == IVL_VT_VECTOR);
      assert(ivl_expr_value(re) == IVL_VT_VECTOR);

      wid = ivl_expr_width(le);
      if (ivl_expr_width(re) > wid)
	    wid = ivl_expr_width(re);

      lv = draw_eval_expr_wid(le, wid, stuff_ok_flag&STUFF_OK_XZ);
      rv = draw_eval_expr_wid(re, wid, stuff_ok_flag&STUFF_OK_XZ);

      switch (ivl_expr_opcode(exp)) {
	  case 'E': /* === */
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/u %u, %u, %u;\n", lv.base,
		    rv.base, lv.wid);
	    if (lv.base >= 8)
		  clr_vector(lv);
	    if (rv.base >= 8)
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

      if ((stuff_ok_flag&STUFF_OK_47) && (wid == 1)) {
	    return lv;
      }

	/* Move the result out out the 4-7 bit that the compare
	   uses. This is because that bit may be clobbered by other
	   expressions. */
      { unsigned base = allocate_vector(ewid);
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


      lv = draw_eval_expr(le, STUFF_OK_XZ);

      if ((lv.base >= 4) && (lv.wid > 1)) {
	    struct vector_info tmp;
	    clr_vector(lv);
	    tmp.base = allocate_vector(1);
	    tmp.wid = 1;
	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n", tmp.base,
		    lv.base, lv.wid);
	    lv = tmp;
      }

      rv = draw_eval_expr(re, STUFF_OK_XZ);
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
      { unsigned base = allocate_vector(wid);
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

      lv = draw_eval_expr(le, STUFF_OK_XZ);

	/* if the left operand has width, then evaluate the single-bit
	   logical equivalent. */
      if ((lv.base >= 4) && (lv.wid > 1)) {
	    struct vector_info tmp;
	    clr_vector(lv);
	    tmp.base = allocate_vector(1);
	    tmp.wid = 1;
	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n", tmp.base,
		    lv.base, lv.wid);
	    lv = tmp;
      }

      rv = draw_eval_expr(re, STUFF_OK_XZ);

	/* if the right operand has width, then evaluate the single-bit
	   logical equivalent. */
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
      { unsigned base = allocate_vector(wid);
        fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	clr_vector(lv);
	lv.base = base;
	lv.wid = wid;
	fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, wid-1);
      }

      return lv;
}

static struct vector_info draw_binary_expr_le_real(ivl_expr_t exp)
{
      struct vector_info res;

      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      int lword = draw_eval_real(le);
      int rword = draw_eval_real(re);

      res.base = allocate_vector(1);
      res.wid  = 1;

      clr_word(lword);
      clr_word(rword);

      switch (ivl_expr_opcode(exp)) {
	  case '<':
	    fprintf(vvp_out, "    %%cmp/wr %d, %d;\n", lword, rword);
	    fprintf(vvp_out, "    %%mov %u, 5, 1;\n", res.base);
	    break;

	  case 'L': /* <= */
	    fprintf(vvp_out, "    %%cmp/wr %d, %d;\n", lword, rword);
	    fprintf(vvp_out, "    %%or 5, 4, 1;\n");
	    fprintf(vvp_out, "    %%mov %u, 5, 1;\n", res.base);
	    break;

	  case '>':
	    fprintf(vvp_out, "    %%cmp/wr %d, %d;\n", rword, lword);
	    fprintf(vvp_out, "    %%mov %u, 5, 1;\n", res.base);
	    break;

	  case 'G': /* >= */
	    fprintf(vvp_out, "    %%cmp/wr %d, %d;\n", rword, lword);
	    fprintf(vvp_out, "    %%or 5, 4, 1;\n");
	    fprintf(vvp_out, "    %%mov %u, 5, 1;\n", res.base);
	    break;

	  default:
	    assert(0);
      }

      return res;
}

static struct vector_info draw_binary_expr_le(ivl_expr_t exp,
					      unsigned wid,
					      int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(exp);
      ivl_expr_t re = ivl_expr_oper2(exp);

      struct vector_info lv;
      struct vector_info rv;

      char s_flag = (ivl_expr_signed(le) && ivl_expr_signed(re)) ? 's' : 'u';

      unsigned owid = ivl_expr_width(le);
      if (ivl_expr_width(re) > owid)
	    owid = ivl_expr_width(re);

      if (ivl_expr_value(le) == IVL_VT_REAL)
	    return draw_binary_expr_le_real(exp);

      if (ivl_expr_value(re) == IVL_VT_REAL)
	    return draw_binary_expr_le_real(exp);

      assert(ivl_expr_value(le) == IVL_VT_VECTOR);
      assert(ivl_expr_value(re) == IVL_VT_VECTOR);

      lv = draw_eval_expr_wid(le, owid, STUFF_OK_XZ);
      rv = draw_eval_expr_wid(re, owid, STUFF_OK_XZ);

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

      if ((stuff_ok_flag&STUFF_OK_47) && (wid == 1)) {
	    lv.base = 5;
	    lv.wid = wid;
	    return lv;
      }

	/* Move the result out out the 4-7 bit that the compare
	   uses. This is because that bit may be clobbered by other
	   expressions. */
      { unsigned base = allocate_vector(wid);
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

      lv = draw_eval_expr_wid(le, wid, STUFF_OK_XZ);
      rv = draw_eval_expr_wid(re, wid, STUFF_OK_XZ);

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

	  case 'O': /* NOR (~|) */
	    fprintf(vvp_out, "    %%nor %u, %u, %u;\n",
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
      const char*opcode = "?";

      struct vector_info lv;

	/* Evaluate the expression that is to be shifted. */
      switch (ivl_expr_opcode(exp)) {

	  case 'l': /* << (left shift) */
	    lv = draw_eval_expr_wid(le, wid, 0);

	      /* shifting 0 gets 0. */
	    if (lv.base == 0)
		  return lv;

	    if (lv.base < 4) {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(lv.wid);
		  tmp.wid = lv.wid;
		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, lv.base, lv.wid);
		  lv = tmp;
	    }
	    opcode = "%shiftl";
	    break;

	  case 'r': /* >> (unsigned right shift) */

	      /* with the right shift, there may be high bits that are
		 shifted into the desired width of the expression, so
		 we let the expression size itself, if it is bigger
		 then what is requested of us. */
	    if (wid > ivl_expr_width(le)) {
		  lv = draw_eval_expr_wid(le, wid, 0);
	    } else {
		  lv = draw_eval_expr_wid(le, ivl_expr_width(le), 0);
	    }

	      /* shifting 0 gets 0. */
	    if (lv.base == 0)
		  return lv;

	    if (lv.base < 4) {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(lv.wid);
		  tmp.wid = lv.wid;
		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, lv.base, lv.wid);
		  lv = tmp;
	    }
	    opcode = "%shiftr";
	    break;

	  case 'R': /* >>> (signed right shift) */

	      /* with the right shift, there may be high bits that are
		 shifted into the desired width of the expression, so
		 we let the expression size itself, if it is bigger
		 then what is requested of us. */
	    if (wid > ivl_expr_width(le)) {
		  lv = draw_eval_expr_wid(le, wid, 0);
	    } else {
		  lv = draw_eval_expr_wid(le, ivl_expr_width(le), 0);
	    }

	      /* shifting 0 gets 0. */
	    if (lv.base == 0)
		  return lv;

	      /* Sign extend any constant begets itself, if this
		 expression is signed. */
	    if ((lv.base < 4) && (ivl_expr_signed(exp)))
		  return lv;

	    if (lv.base < 4) {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(lv.wid);
		  tmp.wid = lv.wid;
		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, lv.base, lv.wid);
		  lv = tmp;
	    }

	    if (ivl_expr_signed(exp))
		  opcode = "%shiftr/s";
	    else
		  opcode = "%shiftr";
	    break;

	  default:
	    assert(0);
      }

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
		  rv = draw_eval_expr(re, 0);
		  fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n",
			  rv.base, rv.wid);
		  clr_vector(rv);
		  break;
	    }
      }


      fprintf(vvp_out, "    %s/i0  %u, %u;\n", opcode, lv.base, lv.wid);

      if (lv.base >= 8)
	    save_expression_lookaside(lv.base, exp, lv.wid);

      return lv;
}

static struct vector_info draw_add_immediate(ivl_expr_t le,
					     ivl_expr_t re,
					     unsigned wid)
{
      struct vector_info lv;
      unsigned long imm;

      lv = draw_eval_expr_wid(le, wid, STUFF_OK_XZ);
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
      unsigned tmp;

      lv = draw_eval_expr_wid(le, wid, STUFF_OK_XZ);
      assert(lv.wid == wid);

      imm = get_number_immediate(re);
      assert( (imm & ~0xffff) == 0 );

      switch (lv.base) {
	  case 0:
	  case 1:
	    tmp = allocate_vector(wid);
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n", tmp, lv.base, wid);
	    lv.base = tmp;
	    fprintf(vvp_out, "    %%subi %u, %lu, %u;\n", lv.base, imm, wid);
	    return lv;

	  case 2:
	  case 3:
	    lv.base = 2;
	    return lv;

	  default:
	    fprintf(vvp_out, "    %%subi %u, %lu, %u;\n", lv.base, imm, wid);
      }


      return lv;
}

static struct vector_info draw_mul_immediate(ivl_expr_t le,
					     ivl_expr_t re,
					     unsigned wid)
{
      struct vector_info lv;
      unsigned long imm;

      lv = draw_eval_expr_wid(le, wid, STUFF_OK_XZ);
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

      lv = draw_eval_expr_wid(le, wid, STUFF_OK_XZ);
      rv = draw_eval_expr_wid(re, wid, STUFF_OK_XZ);

      if (lv.wid != wid) {
	    fprintf(stderr, "XXXX ivl_expr_opcode(exp) = %c,"
		    " lv.wid=%u, wid=%u\n", ivl_expr_opcode(exp),
		    lv.wid, wid);
      }

      assert(lv.wid == wid);
      assert(rv.wid == wid);

	/* The arithmetic instructions replace the left operand with
	   the result. If the left operand is a replicated constant,
	   then I need to make a writable copy so that the
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
	    fprintf(vvp_out, "    %%mod%s %u, %u, %u;\n", sign_string,
		    lv.base, rv.base, wid);
	    break;

	  default:
	    assert(0);
      }

      clr_vector(rv);

      return lv;
}

static struct vector_info draw_binary_expr(ivl_expr_t exp,
					   unsigned wid,
					   int stuff_ok_flag)
{
      struct vector_info rv;
      int stuff_ok_used_flag = 0;

      switch (ivl_expr_opcode(exp)) {
	  case 'a': /* && (logical and) */
	    rv = draw_binary_expr_land(exp, wid);
	    break;

	  case 'E': /* === */
	  case 'e': /* == */
	  case 'N': /* !== */
	  case 'n': /* != */
	    rv = draw_binary_expr_eq(exp, wid, stuff_ok_flag);
	    stuff_ok_used_flag = 1;
	    break;

	  case '<':
	  case '>':
	  case 'L': /* <= */
	  case 'G': /* >= */
	    rv = draw_binary_expr_le(exp, wid, stuff_ok_flag);
	    stuff_ok_used_flag = 1;
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
	  case 'R': /* >>> */
	    rv = draw_binary_expr_lrs(exp, wid);
	    break;

	  case 'o': /* || (logical or) */
	    rv = draw_binary_expr_lor(exp, wid);
	    break;

	  case '&':
	  case '|':
	  case '^':
	  case 'A': /* NAND (~&) */
	  case 'O': /* NOR  (~|) */
	  case 'X': /* XNOR (~^) */
	    rv = draw_binary_expr_logic(exp, wid);
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt error: unsupported binary (%c)\n",
		    ivl_expr_opcode(exp));
	    assert(0);
      }

	/* Mark in the lookaside that this value is done. If any OK
	   flags besides the STUFF_OK_47 flag are set, then the result
	   may not be a pure one, so clear the lookaside for the range
	   instead of setting in to the new expression result.

	   The stuff_ok_used_flag tells me if the stuff_ok_flag was
	   even used by anything. If not, then I can ignore it in the
	   following logic. */
      if (rv.base >= 8) {
	    if (stuff_ok_used_flag && (stuff_ok_flag & ~STUFF_OK_47))
		  save_expression_lookaside(rv.base, 0, wid);
	    else
		  save_expression_lookaside(rv.base, exp, wid);
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
      res = draw_eval_expr(sel, 0);
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

      if (res.base >= 8)
	    save_expression_lookaside(res.base, exp, wid);
      return res;
}

/*
 * The concatenation operator is evaluated by evaluating each sub-
 * expression, then copying it into the contiguous vector of the
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

		  unsigned trans;
		  struct vector_info avec;

		    /* Try to locate the subexpression in the
		       lookaside map. */
		  avec.base = allocate_vector_exp(arg, awid);
		  avec.wid = awid;

		  trans = awid;
		  if ((off + awid) > wid)
			trans = wid - off;

		  if (avec.base != 0) {
			assert(awid == avec.wid);

			fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
				res.base+off,
				avec.base, trans);
			clr_vector(avec);

		  } else {
			struct vector_info dest;

			dest.base = res.base+off;
			dest.wid  = trans;
			draw_eval_expr_dest(arg, dest, 0);
		  }


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

	/* Save the accumulated result in the lookaside map. */
      if (res.base >= 8)
	    save_expression_lookaside(res.base, exp, wid);

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
      if (idx < wid) {
	    if (ivl_expr_signed(exp) && bits[nwid-1] == '1')
		  fprintf(vvp_out, "    %%mov %u, 1, %u;\n",
			  res.base+idx, wid-idx);

	    else if (bits[nwid-1] == 'x')
		  fprintf(vvp_out, "    %%mov %u, 2, %u;\n",
			  res.base+idx, wid-idx);

	    else if (bits[nwid-1] == 'z')
		  fprintf(vvp_out, "    %%mov %u, 3, %u;\n",
			  res.base+idx, wid-idx);

	    else
		  fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
			  res.base+idx, wid-idx);
      }

      if (res.base >= 8)
	    save_expression_lookaside(res.base, exp, wid);

      return res;
}

/*
 * The PAD expression takes a smaller node and pads it out to a larger
 * value. It will zero extend or sign extend depending on the
 * signedness of the expression.
 */
static struct vector_info draw_pad_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info subv;
      struct vector_info res;

      subv = draw_eval_expr(ivl_expr_oper1(exp), 0);
      if (wid <= subv.wid) {
	    if (subv.base >= 8)
		  save_expression_lookaside(subv.base, exp, subv.wid);
	    res.base = subv.base;
	    res.wid = wid;
	    return res;
      }

      res.base = allocate_vector(wid);
      res.wid = wid;

      fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
	      res.base, subv.base, subv.wid);

      assert(wid > subv.wid);

      if (ivl_expr_signed(exp)) {
	    unsigned idx;
	    for (idx = subv.wid ;  idx < res.wid ;  idx += 1)
		  fprintf(vvp_out, "    %%mov %u, %u, 1;\n",
			  res.base+idx, subv.base+subv.wid-1);
      } else {
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
		    res.base+subv.wid, res.wid - subv.wid);
      }

      save_expression_lookaside(res.base, exp, wid);
      return res;
}

static struct vector_info draw_realnum_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info res;
      double val = ivl_expr_dvalue(exp);

      res.base = allocate_vector(wid);
      res.wid = wid;

      fprintf(vvp_out, "    ; XXXX draw_realnum_expr(%f, %u)\n", val, wid);

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

      if (res.base >= 8)
	    save_expression_lookaside(res.base, exp, wid);

      return res;
}

/*
 * Evaluating a signal expression means loading the bits of the signal
 * into the thread bits. Remember to account for the part select by
 * offsetting the read from the lsi (least significant index) of the
 * signal.
 */
static void draw_signal_dest(ivl_expr_t exp, struct vector_info res)
{
      unsigned idx;
      unsigned lsi = ivl_expr_lsi(exp);
      unsigned swid = ivl_expr_width(exp);
      ivl_signal_t sig = ivl_expr_signal(exp);

      if (swid > res.wid)
	    swid = res.wid;

      if (ivl_signal_type(sig) == IVL_SIT_REG) {
	      /* If this is a REG (a variable) then I can do a vector
		 read. */
	    fprintf(vvp_out, "    %%load/v %u, V_%s[%u], %u;\n",
		    res.base, vvp_signal_label(sig), lsi, swid);

      } else {
	      /* Vector reads of nets do not in general work because
		 they are not really functors but references to
		 scattered functors. So generate an array of loads. */
	    for (idx = 0 ;  idx < swid ;  idx += 1) {
		  fprintf(vvp_out, "    %%load  %u, V_%s[%u];\n",
			  res.base+idx, vvp_signal_label(sig), idx+lsi);
	    }
      }


	/* Pad the signal value with zeros. */
      if (swid < res.wid) {

	    if (ivl_expr_signed(exp)) {
		  for (idx = swid ;  idx < res.wid ;  idx += 1)
			fprintf(vvp_out, "    %%mov %u, %u, 1;\n",
				res.base+idx, res.base+swid-1);

	    } else {
		  fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
			  res.base+swid, res.wid-swid);
	    }
      }
}

static struct vector_info draw_signal_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info res;

	/* Already in the vector lookaside? */
      res.base = allocate_vector_exp(exp, wid);
      res.wid = wid;
      if (res.base != 0)
	    return res;

      res.base = allocate_vector(wid);
      res.wid  = wid;
      save_expression_lookaside(res.base, exp, wid);

      draw_signal_dest(exp, res);
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
		struct vector_info addr = draw_eval_expr(ae, 0);
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

      if (res.base >= 8)
	    save_expression_lookaside(res.base, exp, wid);

      return res;
}

static struct vector_info draw_select_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info subv, shiv, res;
      ivl_expr_t sube  = ivl_expr_oper1(exp);
      ivl_expr_t shift = ivl_expr_oper2(exp);

	/* Evaluate the sub-expression. */
      subv = draw_eval_expr(sube, 0);

	/* Any bit select of a constant zero is another constant zero,
	   so short circuit and return the value we know. */
      if (subv.base == 0) {
	    subv.wid = wid;
	    return subv;
      }

	/* Evaluate the bit select base expression and store the
	   result into index register 0. */
      shiv = draw_eval_expr(shift, STUFF_OK_XZ);
      fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", shiv.base, shiv.wid);
      clr_vector(shiv);

	/* If the subv result is a magic constant, then make a copy in
	   writable vector space and work from there instead. */
      if (subv.base < 4) {
	    res.base = allocate_vector(subv.wid);
	    res.wid = subv.wid;
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

      if (res.base >= 8)
	    save_expression_lookaside(res.base, exp, wid);

      return res;
}

static struct vector_info draw_ternary_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info res, tru, fal, tst;

      unsigned lab_true, lab_false, lab_out;
      ivl_expr_t cond = ivl_expr_oper1(exp);
      ivl_expr_t true_ex = ivl_expr_oper2(exp);
      ivl_expr_t false_ex = ivl_expr_oper3(exp);

      lab_true  = local_count++;
      lab_false = local_count++;
      lab_out = local_count++;

	/* Evaluate the condition expression, and if necessary reduce
	   it to a single bit. */
      tst = draw_eval_expr(cond, STUFF_OK_XZ);
      if ((tst.base >= 4) && (tst.wid > 1)) {
	    struct vector_info tmp;

	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n",
		    tst.base, tst.base, tst.wid);

	    tmp = tst;
	    tmp.base += 1;
	    tmp.wid -= 1;
	    clr_vector(tmp);

	    tst.wid = 1;
      }

      fprintf(vvp_out, "    %%jmp/0  T_%d.%d, %u;\n",
	      thread_count, lab_true, tst.base);

      tru = draw_eval_expr_wid(true_ex, wid, 0);

	/* The true result must be in a writable register, because the
	   blend might want to manipulate it. */
      if (tru.base < 4) {
	    struct vector_info tmp;
	    tmp.base = allocate_vector(wid);
	    tmp.wid = wid;
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
		    tmp.base, tru.base, wid);
	    tru = tmp;
      }

      fprintf(vvp_out, "    %%jmp/1  T_%d.%d, %u;\n",
	      thread_count, lab_out, tst.base);

      clear_expression_lookaside();

      fprintf(vvp_out, "T_%d.%d ; End of true expr.\n",
	      thread_count, lab_true);

      fal = draw_eval_expr_wid(false_ex, wid, 0);

      fprintf(vvp_out, "    %%jmp/0  T_%d.%d, %u;\n",
	      thread_count, lab_false, tst.base);

      fprintf(vvp_out, " ; End of false expr.\n");

      clr_vector(tst);
      clr_vector(fal);

      fprintf(vvp_out, "    %%blend  %u, %u, %u; Condition unknown.\n",
	      tru.base, fal.base, wid);
      fprintf(vvp_out, "    %%jmp  T_%d.%d;\n",
	      thread_count, lab_out);

      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_false);
      fprintf(vvp_out, "    %%mov %u, %u, %u; Return false value\n",
	      tru.base, fal.base, wid);

	/* This is the out label. */
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_out);
      clear_expression_lookaside();

      res = tru;

      if (res.base >= 8)
	    save_expression_lookaside(res.base, exp, wid);

      return res;
}

static struct vector_info draw_sfunc_expr(ivl_expr_t exp, unsigned wid)
{
      unsigned parm_count = ivl_expr_parms(exp);
      struct vector_info res;


	/* If the function has no parameters, then use this short-form
	   to draw the statement. */
      if (parm_count == 0) {
	    assert(ivl_expr_value(exp) == IVL_VT_VECTOR);
	    res.base = allocate_vector(wid);
	    res.wid  = wid;
	    fprintf(vvp_out, "    %%vpi_func \"%s\", %u, %u;\n",
		    ivl_expr_name(exp), res.base, res.wid);
	    return res;

      }

      res = draw_vpi_func_call(exp, wid);

	/* New basic block starts after VPI calls. */
      clear_expression_lookaside();

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
				     ivl_signal_pins(port), 0);
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
      fprintf(vvp_out, ", S_%p;\n", def);
      fprintf(vvp_out, "    %%join;\n");

	/* Fresh basic block starts after the join. */
      clear_expression_lookaside();

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

static struct vector_info draw_ulong_expr(ivl_expr_t exp, unsigned wid)
{
      unsigned long idx;
      struct vector_info res;
      unsigned long uval = ivl_expr_uvalue(exp);

      if (uval == 0) {
	    res.wid = wid;
	    res.base = 0;
	    return res;
      }

      res.base = allocate_vector(wid);
      res.wid = wid;

      idx = 0;
      while (idx < wid) {
	    unsigned long cnt;

	    int bit = 1 & (uval >> idx);
	    for (cnt = 1 ;  idx+cnt < wid ;  cnt += 1) {
		  int tmp = 1 & (uval >> (idx+cnt));
		  if (tmp != bit)
			break;
	    }

	    fprintf(vvp_out, "   %%mov %u, %d, %lu;\n",
		    res.base+idx, bit, cnt);
	    idx += cnt;
      }

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
	    res = draw_eval_expr_wid(sub, wid, STUFF_OK_XZ);
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
	    res = draw_eval_expr_wid(sub, wid, STUFF_OK_XZ);
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
		  fprintf(vvp_out, "    %%addi %u, 1, %u;\n",res.base,res.wid);
		  break;
	    }
	    break;

	  case '!':
	    res = draw_eval_expr(sub, STUFF_OK_XZ);
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
	    res = draw_eval_expr(sub, 0);
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

      if (res.base >= 8)
	    save_expression_lookaside(res.base, exp, wid);

      return res;
}

static struct vector_info draw_variable_expr(ivl_expr_t exp, unsigned wid)
{
      struct vector_info res;
      ivl_variable_t var = ivl_expr_variable(exp);
      fprintf(vvp_out, "    ; XXXX Read variable %s\n",
	      ivl_variable_name(var));

      res.base = 0;
      res.wid = wid;
      return res;
}

/*
 * Sometimes we know ahead of time where we want the expression value
 * to go. In that case, call this function. It will check to see if
 * the expression can be preplaced, and if so it will evaluate it in
 * place.
 */
static void draw_eval_expr_dest(ivl_expr_t exp, struct vector_info dest,
				int stuff_ok_flag)
{
      struct vector_info tmp;

      switch (ivl_expr_type(exp)) {

	  case IVL_EX_SIGNAL:
	    draw_signal_dest(exp, dest);
	    return;

	  default:
	    break;
      }

	/* Fallback, is to draw the expression by width, and mov it to
	   the required dest. */
      tmp = draw_eval_expr_wid(exp, dest.wid, stuff_ok_flag);
      assert(tmp.wid == dest.wid);

      fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
	      dest.base, tmp.base, dest.wid);

      if (tmp.base >= 8)
	    save_expression_lookaside(tmp.base, exp, tmp.wid);

      clr_vector(tmp);
}

struct vector_info draw_eval_expr_wid(ivl_expr_t exp, unsigned wid,
				      int stuff_ok_flag)
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
	    res = draw_binary_expr(exp, wid, stuff_ok_flag);
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

	  case IVL_EX_REALNUM:
	    res = draw_realnum_expr(exp, wid);
	    break;

	  case IVL_EX_SELECT:
	    if (ivl_expr_oper2(exp) == 0)
		  res = draw_pad_expr(exp, wid);
	    else
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

	  case IVL_EX_ULONG:
	    res = draw_ulong_expr(exp, wid);
	    break;

	  case IVL_EX_UNARY:
	    res = draw_unary_expr(exp, wid);
	    break;

	  case IVL_EX_VARIABLE:
	    res = draw_variable_expr(exp, wid);
	    break;
      }

      return res;
}

struct vector_info draw_eval_expr(ivl_expr_t exp, int stuff_ok_flag)
{
      return draw_eval_expr_wid(exp, ivl_expr_width(exp), stuff_ok_flag);
}

/*
 * $Log: eval_expr.c,v $
 * Revision 1.110.2.2  2007/02/26 19:51:40  steve
 *  Spelling fixes (larry doolittle)
 *
 * Revision 1.110.2.1  2006/03/12 07:34:20  steve
 *  Fix the memsynth1 case.
 *
 * Revision 1.110  2004/10/04 01:10:57  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.109  2004/09/10 00:14:31  steve
 *  Relaxed width constraint on pad_expression output.
 *
 * Revision 1.108  2004/06/30 03:07:32  steve
 *  Watch out for real compared to constant. Handle as real.
 *
 * Revision 1.107  2004/06/19 16:17:37  steve
 *  Generate signed modulus if appropriate.
 *
 * Revision 1.106  2003/10/01 17:44:20  steve
 *  Slightly more efficient unary minus.
 *
 * Revision 1.105  2003/09/24 20:46:20  steve
 *  Clear expression lookaside after true cause of ternary.
 *
 * Revision 1.104  2003/08/03 03:53:38  steve
 *  Subtract from constant values.
 *
 * Revision 1.103  2003/07/26 03:34:43  steve
 *  Start handling pad of expressions in code generators.
 *
 * Revision 1.102  2003/06/18 03:55:19  steve
 *  Add arithmetic shift operators.
 *
 * Revision 1.101  2003/06/17 19:17:42  steve
 *  Remove short int restrictions from vvp opcodes.
 *
 * Revision 1.100  2003/06/16 22:14:15  steve
 *  Fix fprintf warning.
 *
 * Revision 1.99  2003/06/15 22:49:32  steve
 *  More efficient code for ternary expressions.
 *
 * Revision 1.98  2003/06/14 22:18:54  steve
 *  Sign extend signed vectors.
 *
 * Revision 1.97  2003/06/13 19:10:20  steve
 *  Handle assign of real to vector.
 *
 * Revision 1.96  2003/06/11 02:23:45  steve
 *  Proper pad of signed constants.
 *
 * Revision 1.95  2003/05/10 02:38:49  steve
 *  Proper width handling of || expressions.
 *
 * Revision 1.94  2003/03/25 02:15:48  steve
 *  Use hash code for scope labels.
 *
 * Revision 1.93  2003/03/15 04:45:18  steve
 *  Allow real-valued vpi functions to have arguments.
 *
 * Revision 1.92  2003/02/28 20:21:13  steve
 *  Merge vpi_call and vpi_func draw functions.
 *
 * Revision 1.91  2003/02/07 02:46:16  steve
 *  Handle real value subtract and comparisons.
 *
 * Revision 1.90  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.89  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.88  2002/12/20 01:11:14  steve
 *  Evaluate shift index after shift operand because
 *  the chift operand may use the index register itself.
 *
 * Revision 1.87  2002/12/19 23:11:29  steve
 *  Keep bit select subexpression width if it is constant.
 */

