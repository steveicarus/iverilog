/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
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

# include  "vvp_priv.h"
# include  <string.h>
# include  <stdlib.h>
# include  <assert.h>

static void draw_eval_expr_dest(ivl_expr_t expr, struct vector_info dest,
				int ok_flags);
static void draw_signal_dest(ivl_expr_t expr, struct vector_info res,
			     int add_index, long immediate);

int number_is_unknown(ivl_expr_t expr)
{
      const char*bits;
      unsigned idx;

      if (ivl_expr_type(expr) == IVL_EX_ULONG)
	    return 0;

      assert(ivl_expr_type(expr) == IVL_EX_NUMBER);

      bits = ivl_expr_bits(expr);
      for (idx = 0 ;  idx < ivl_expr_width(expr) ;  idx += 1)
	    if ((bits[idx] != '0') && (bits[idx] != '1'))
		  return 1;

      return 0;
}

/*
 * This function returns TRUE if the number can be represented as a
 * lim_wid immediate value. This amounts to verifying that any upper
 * bits are the same. For a negative value we do not support the most
 * negative twos-complement value since it can not be negated. This
 * code generator always emits positive values, hence the negation
 * requirement.
 */
int number_is_immediate(ivl_expr_t expr, unsigned lim_wid, int negative_ok_flag)
{
      const char *bits;
      unsigned nbits = ivl_expr_width(expr);
      char pad_bit = '0';
      unsigned idx;

	/* We can only convert numbers to an immediate value. */
      if (ivl_expr_type(expr) != IVL_EX_NUMBER
	  && ivl_expr_type(expr) != IVL_EX_ULONG
	  && ivl_expr_type(expr) != IVL_EX_DELAY)
	    return 0;

	/* If a negative value is OK, then we really have one less
	 * significant bit because of the sign bit. */
      if (negative_ok_flag) lim_wid -= 1;

	/* This is an unsigned value so it can not have the -2**N problem. */
      if (ivl_expr_type(expr) == IVL_EX_ULONG) {
	    unsigned long imm;
	    if (lim_wid >= 8*sizeof(unsigned long)) return 1;
	      /* At this point we know that lim_wid is smaller than an
	       * unsigned long variable. */
	    imm = ivl_expr_uvalue(expr);
	    if (imm < (1UL << lim_wid)) return 1;
	    else return 0;
      }

	/* This is an unsigned value so it can not have the -2**N problem. */
      if (ivl_expr_type(expr) == IVL_EX_DELAY) {
	    uint64_t imm;
	    if (lim_wid >= 8*sizeof(uint64_t)) return 1;
	      /* At this point we know that lim_wid is smaller than a
	       * uint64_t variable. */
	    imm = ivl_expr_delay_val(expr);
	    if (imm < ((uint64_t)1 << lim_wid)) return 1;
	    else return 0;
      }

      bits = ivl_expr_bits(expr);

      if (ivl_expr_signed(expr) && bits[nbits-1]=='1') pad_bit = '1';

      if (pad_bit == '1' && !negative_ok_flag) return 0;

      for (idx = lim_wid ;  idx < nbits ;  idx += 1)
	    if (bits[idx] != pad_bit) return 0;

	/* If we have a negative number make sure it is not too big. */
      if (pad_bit == '1') {
	    for (idx = 0; idx < lim_wid; idx += 1)
		  if (bits[idx] == '1') return 1;
	    return 0;
      }

      return 1;
}

/*
 * We can return positive or negative values. You must verify that the
 * number is not unknown (number_is_unknown) and is small enough
 * (number_is_immediate).
 */
long get_number_immediate(ivl_expr_t expr)
{
      long imm = 0;
      unsigned idx;

      switch (ivl_expr_type(expr)) {
	  case IVL_EX_ULONG:
	    imm = ivl_expr_uvalue(expr);
	    break;

	  case IVL_EX_NUMBER: {
		const char*bits = ivl_expr_bits(expr);
		unsigned nbits = ivl_expr_width(expr);
		  /* We can not copy more bits than fit into a long. */
		if (nbits > 8*sizeof(long)) nbits = 8*sizeof(long);
		for (idx = 0 ; idx < nbits ; idx += 1) switch (bits[idx]){
		    case '0':
		      break;
		    case '1':
		      imm |= 1L << idx;
		      break;
		    default:
		      assert(0);
		}
		if (ivl_expr_signed(expr) && bits[nbits-1]=='1' &&
		    nbits < 8*sizeof(long)) imm |= -1L << nbits;
		break;
	  }

	  default:
	    assert(0);
      }

      return imm;
}

uint64_t get_number_immediate64(ivl_expr_t expr)
{
      uint64_t imm = 0;
      unsigned idx;

      switch (ivl_expr_type(expr)) {
	  case IVL_EX_ULONG:
	    imm = ivl_expr_uvalue(expr);
	    break;

	  case IVL_EX_NUMBER: {
		const char*bits = ivl_expr_bits(expr);
		unsigned nbits = ivl_expr_width(expr);
		for (idx = 0 ; idx < nbits ; idx += 1) switch (bits[idx]){
		    case '0':
		      break;
		    case '1':
		      assert(idx < 64);
		      imm |= UINT64_C(1) << idx;
		      break;
		    default:
		      assert(0);
		}
		if (ivl_expr_signed(expr) && bits[nbits-1]=='1' && nbits < 64)
		      imm |= (-UINT64_C(1)) << nbits;
		break;
	  }

	  default:
	    assert(0);
      }

      return imm;
}

static void eval_logic_into_integer(ivl_expr_t expr, unsigned ix)
{
      switch (ivl_expr_type(expr)) {

	  case IVL_EX_NUMBER:
	  case IVL_EX_ULONG:
	      {
		    assert(number_is_immediate(expr, IMM_WID, 1));
		    if (number_is_unknown(expr)) {
			    /* We are loading a 'bx so mimic %ix/get. */
			  fprintf(vvp_out, "    %%ix/load %u, 0, 0;\n", ix);
			  fprintf(vvp_out, "    %%mov 4, 1, 1;\n");
			  break;
		    }
		    long imm = get_number_immediate(expr);
		    if (imm >= 0) {
			  fprintf(vvp_out, "    %%ix/load %u, %ld, 0;\n", ix, imm);
		    } else {
			  fprintf(vvp_out, "    %%ix/load %u, 0, 0; loading %ld\n", ix, imm);
			  fprintf(vvp_out, "    %%ix/sub %u, %ld, 0;\n", ix, -imm);
		    }
		      /* This can not have have a X/Z value so clear bit 4. */
		    fprintf(vvp_out, "    %%mov 4, 0, 1;\n");
	      }
	      break;

	  case IVL_EX_SIGNAL: {
		ivl_signal_t sig = ivl_expr_signal(expr);

		unsigned word = 0;
		if (ivl_signal_dimensions(sig) > 0) {
		      ivl_expr_t ixe;
		      const char*type = ivl_expr_signed(expr) ? "/s" : "";

			/* Detect the special case that this is a
			   variable array. In this case, the ix/getv
			   will not work, so do it the hard way. */
		      if (ivl_signal_type(sig) == IVL_SIT_REG) {
			    struct vector_info rv;
			    rv = draw_eval_expr(expr, 0);
			    fprintf(vvp_out, "    %%ix/get%s %u, %u, %u;\n",
				    type, ix, rv.base, rv.wid);
			    clr_vector(rv);
			    break;
		      }

		      ixe = ivl_expr_oper1(expr);
		      if (number_is_immediate(ixe, IMM_WID, 0)) {
		            assert(! number_is_unknown(ixe));
		            word = get_number_immediate(ixe);
		      } else {
		            struct vector_info rv;
		            rv = draw_eval_expr(expr, 0);
		            fprintf(vvp_out, "    %%ix/get%s %u, %u, %u;\n",
		                    type, ix, rv.base, rv.wid);
		            clr_vector(rv);
		            break;
		      }
		}
		const char*type = ivl_signal_signed(sig) ? "/s" : "";
		fprintf(vvp_out, "    %%ix/getv%s %u, v%p_%u;\n", type, ix,
		                 sig, word);
		break;
	  }

	  default: {
		  struct vector_info rv;
		  rv = draw_eval_expr(expr, 0);
		    /* Is this a signed expression? */
		  if (ivl_expr_signed(expr)) {
		      fprintf(vvp_out, "    %%ix/get/s %u, %u, %u;\n",
		                       ix, rv.base, rv.wid);
		  } else {
		      fprintf(vvp_out, "    %%ix/get %u, %u, %u;\n",
		                       ix, rv.base, rv.wid);
		  }
		  clr_vector(rv);
		  break;
	    }
      }
}

/*
 * This function, in addition to setting the value into index 0, sets
 * bit 4 to 1 if the value is unknown.
 */
void draw_eval_expr_into_integer(ivl_expr_t expr, unsigned ix)
{
      int word;

      switch (ivl_expr_value(expr)) {

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    eval_logic_into_integer(expr, ix);
	    break;

	  case IVL_VT_REAL:
	    word = draw_eval_real(expr);
	    clr_word(word);
	    fprintf(vvp_out, "    %%cvt/ir %u, %u;\n", ix, word);
	    break;

	  default:
	    fprintf(stderr, "XXXX ivl_expr_value == %d\n",
		    ivl_expr_value(expr));
	    assert(0);
      }
}

/*
 * The STUFF_OK_XZ bit is true if the output is going to be further
 * processed so that x and z values are equivalent. This may allow for
 * new optimizations.
 */
static struct vector_info draw_eq_immediate(ivl_expr_t expr, unsigned ewid,
					    ivl_expr_t le,
					    ivl_expr_t re,
					    int stuff_ok_flag)
{
      unsigned wid;
      struct vector_info lv;
      unsigned long imm;
      assert(number_is_immediate(re, IMM_WID, 0));
      assert(! number_is_unknown(re));
      imm = get_number_immediate(re);

      wid = ivl_expr_width(le);
      lv = draw_eval_expr_wid(le, wid, stuff_ok_flag);

      switch (ivl_expr_opcode(expr)) {
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
	    if (base == 0) {
		  fprintf(stderr, "%s:%u: vvp.tgt error: "
			  "Unable to allocate %u thread bits "
			  "for result of equality compare.\n",
			  ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
		  vvp_errors += 1;
	    }

	    fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	    lv.base = base;
	    lv.wid = ewid;
	    if (ewid > 1)
		  fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, ewid-1);

      } else if (lv.wid < ewid) {
	    unsigned base = allocate_vector(ewid);
	    if (base == 0) {
		  fprintf(stderr, "%s:%u: vvp.tgt error: "
			  "Unable to allocate %u thread bits "
			  "for result of equality compare.\n",
			  ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
		  vvp_errors += 1;
	    }

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
static struct vector_info draw_binary_expr_eq_real(ivl_expr_t expr)
{
      struct vector_info res;
      int lword, rword;

      res.base = allocate_vector(1);
      res.wid  = 1;
      assert(res.base);

      lword = draw_eval_real(ivl_expr_oper1(expr));
      rword = draw_eval_real(ivl_expr_oper2(expr));

      clr_word(lword);
      clr_word(rword);

      fprintf(vvp_out, "    %%cmp/wr %d, %d;\n", lword, rword);
      switch (ivl_expr_opcode(expr)) {

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

static struct vector_info draw_binary_expr_eq(ivl_expr_t expr,
					      unsigned ewid,
					      int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      unsigned wid;

      struct vector_info lv;
      struct vector_info rv;

      if ((ivl_expr_value(le) == IVL_VT_REAL)
	  ||(ivl_expr_value(re) == IVL_VT_REAL))  {
	    return draw_binary_expr_eq_real(expr);
      }

      if (number_is_immediate(re,16,0) && !number_is_unknown(re))
	    return draw_eq_immediate(expr, ewid, le, re, stuff_ok_flag);

      assert(ivl_expr_value(le) == IVL_VT_LOGIC
	     || ivl_expr_value(le) == IVL_VT_BOOL);
      assert(ivl_expr_value(re) == IVL_VT_LOGIC
	     || ivl_expr_value(re) == IVL_VT_BOOL);

      wid = ivl_expr_width(le);
      if (ivl_expr_width(re) > wid)
	    wid = ivl_expr_width(re);

      lv = draw_eval_expr_wid(le, wid, stuff_ok_flag&~STUFF_OK_47);
      rv = draw_eval_expr_wid(re, wid, stuff_ok_flag&~STUFF_OK_47);

      switch (ivl_expr_opcode(expr)) {
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
		  fprintf(stderr,"vvp.tgt error: operands of == "
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
		  fprintf(stderr,"vvp.tgt error: operands of !== "
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
		  fprintf(stderr,"vvp.tgt error: operands of != "
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

      if ((stuff_ok_flag&STUFF_OK_47) && (ewid == 1)) {
	    return lv;
      }

	/* Move the result out out the 4-7 bit that the compare
	   uses. This is because that bit may be clobbered by other
	   expressions. */
      { unsigned base = allocate_vector(ewid);
	if (base == 0) {
	      fprintf(stderr, "%s:%u: vvp.tgt error: "
		      "Unable to allocate %u thread bits "
		      "for result of equality compare.\n",
		      ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	    vvp_errors += 1;
	}

        fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	lv.base = base;
	lv.wid = ewid;
	if (ewid > 1)
	      fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, ewid-1);
      }

      return lv;
}

static struct vector_info draw_binary_expr_land(ivl_expr_t expr, unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      struct vector_info lv;
      struct vector_info rv;


      lv = draw_eval_expr(le, STUFF_OK_XZ);

      if ((lv.base >= 4) && (lv.wid > 1)) {
	    struct vector_info tmp;
	    clr_vector(lv);
	    tmp.base = allocate_vector(1);
	    tmp.wid = 1;
	    assert(tmp.base);
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
	    assert(tmp.base);
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
	if (base == 0) {
	      fprintf(stderr, "%s:%u: vvp.tgt error: "
		      "Unable to allocate %u thread bits "
		      "for result of padded logical AND.\n",
		      ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	      vvp_errors += 1;
	}

        fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	clr_vector(lv);
	lv.base = base;
	lv.wid = wid;
	fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, wid-1);
      }

      return lv;
}

static struct vector_info draw_binary_expr_lor(ivl_expr_t expr, unsigned wid,
					       int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

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
	    assert(tmp.base);
	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n", tmp.base,
		    lv.base, lv.wid);
	    lv = tmp;
      }

	/* The right expression may be left in registers 4-7 because
	   I'll be using it immediately. */
      rv = draw_eval_expr(re, STUFF_OK_XZ|STUFF_OK_47);

	/* if the right operand has width, then evaluate the single-bit
	   logical equivalent. */
      if ((rv.base >= 4) && (rv.wid > 1)) {
	    struct vector_info tmp;
	    clr_vector(rv);
	    tmp.base = allocate_vector(1);
	    tmp.wid = 1;
	    assert(tmp.base);
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

	    } else if (lv.base==0) {
		  lv = rv;
	    } else {
		  fprintf(vvp_out, "    %%or %u, %u, 1;\n", rv.base, lv.base);
		  lv = rv;
	    }

      } else if (rv.base == 0) {
	    ; /* Just return lv. */
      } else {
	    if (rv.base >= 8 && lv.base < 8 && !(stuff_ok_flag&STUFF_OK_47)) {
		    /* If STUFF_OK_47 is false, and rv is not in the
		       47 area (and lv is) then plan to or the result
		       into the rv instead. This can case a %mov later. */
		  struct vector_info tmp = lv;
		  lv = rv;
		  rv = tmp;
	    }
	    fprintf(vvp_out, "    %%or %u, %u, 1;\n", lv.base, rv.base);
	    if (rv.base >= 8) clr_vector(rv);
      }

      if (wid==1 && (lv.base<4 || lv.base>=8 || (stuff_ok_flag&STUFF_OK_47)))
	    return lv;

	/* If we only want the single bit result, then we are done. */
      if (wid == 1) {
	    if (lv.base >= 4 && lv.base < 8) {
		  unsigned tmp = allocate_vector(1);
		  fprintf(vvp_out, "   %%mov %u, %u, 1;\n", tmp, lv.base);
		  lv.base = tmp;
	    }
	    return lv;
      }

	/* Write the result into a zero-padded result. */
      { unsigned base = allocate_vector(wid);
	if (base == 0) {
	      fprintf(stderr, "%s:%u: vvp.tgt error: "
		      "Unable to allocate %u thread bits "
		      "for result of padded logical OR.\n",
		      ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	      vvp_errors += 1;
	}

        fprintf(vvp_out, "    %%mov %u, %u, 1;\n", base, lv.base);
	if (lv.base >= 8) clr_vector(lv);
	lv.base = base;
	lv.wid = wid;
	fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, wid-1);
      }

      return lv;
}

static struct vector_info draw_binary_expr_le_real(ivl_expr_t expr)
{
      struct vector_info res;

      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      int lword = draw_eval_real(le);
      int rword = draw_eval_real(re);

      res.base = allocate_vector(1);
      res.wid  = 1;

      assert(res.base);

      clr_word(lword);
      clr_word(rword);

      switch (ivl_expr_opcode(expr)) {
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

static struct vector_info draw_binary_expr_le_bool(ivl_expr_t expr,
						   unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      int lw, rw;
      struct vector_info tmp;

      char s_flag = (ivl_expr_signed(le) && ivl_expr_signed(re)) ? 's' : 'u';

      assert(ivl_expr_value(le) == IVL_VT_BOOL);
      assert(ivl_expr_value(re) == IVL_VT_BOOL);

      lw = draw_eval_bool64(le);
      rw = draw_eval_bool64(re);

      switch (ivl_expr_opcode(expr)) {
	  case 'G':
	    fprintf(vvp_out, "    %%cmp/w%c %u, %u;\n", s_flag, rw, lw);
	    fprintf(vvp_out, "    %%or 5, 4, 1;\n");
	    break;

	  case 'L':
	    fprintf(vvp_out, "    %%cmp/w%c %u, %u;\n", s_flag, lw, rw);
	    fprintf(vvp_out, "    %%or 5, 4, 1;\n");
	    break;

	  case '<':
	    fprintf(vvp_out, "    %%cmp/w%c %u, %u;\n", s_flag, lw, rw);
	    break;

	  case '>':
	    fprintf(vvp_out, "    %%cmp/w%c %u, %u;\n", s_flag, rw, lw);
	    break;

	  default:
	    assert(0);
      }

      clr_word(lw);
      clr_word(rw);

	/* Move the result out out the 4-7 bit that the compare
	   uses. This is because that bit may be clobbered by other
	   expressions. */
      { unsigned base = allocate_vector(wid);
	if (base == 0) {
	      fprintf(stderr, "%s:%u: vvp.tgt error: "
		      "Unable to allocate %u thread bits "
		      "for result of padded inequality compare.\n",
		      ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	      vvp_errors += 1;
	}

        fprintf(vvp_out, "    %%mov %u, 5, 1;\n", base);
	tmp.base = base;
	tmp.wid = wid;
	if (wid > 1)
	      fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, wid-1);
      }

      return tmp;
}

static struct vector_info draw_binary_expr_le(ivl_expr_t expr,
					      unsigned wid,
					      int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      struct vector_info lv;
      struct vector_info rv;

      char s_flag = (ivl_expr_signed(le) && ivl_expr_signed(re)) ? 's' : 'u';

      unsigned owid = ivl_expr_width(le);
      if (ivl_expr_width(re) > owid)
	    owid = ivl_expr_width(re);

      if (ivl_expr_value(le) == IVL_VT_REAL)
	    return draw_binary_expr_le_real(expr);

      if (ivl_expr_value(re) == IVL_VT_REAL)
	    return draw_binary_expr_le_real(expr);

	/* Detect the special case that we can do this with integers. */
      if (ivl_expr_value(le) == IVL_VT_BOOL
	  && ivl_expr_value(re) == IVL_VT_BOOL
	  && owid < 64) {
	    return draw_binary_expr_le_bool(expr, wid);
      }

      assert(ivl_expr_value(le) == IVL_VT_LOGIC
	     || ivl_expr_value(le) == IVL_VT_BOOL);
      assert(ivl_expr_value(re) == IVL_VT_LOGIC
	     || ivl_expr_value(re) == IVL_VT_BOOL);

      lv.wid = 0;  lv.base=0;
      rv.wid = 0;  rv.base=0;

      switch (ivl_expr_opcode(expr)) {
	  case 'G':
	    rv = draw_eval_expr_wid(re, owid, STUFF_OK_XZ);
	    if (number_is_immediate(le,16,0) && !number_is_unknown(le)) {
		  long imm = get_number_immediate(le);
		  assert(imm >= 0);
		  fprintf(vvp_out, "   %%cmpi/%c %u, %ld, %u;\n", s_flag,
			  rv.base, imm, rv.wid);
	    } else {
		  lv = draw_eval_expr_wid(le, owid, STUFF_OK_XZ);
		  assert(lv.wid == rv.wid);
		  fprintf(vvp_out, "    %%cmp/%c %u, %u, %u;\n", s_flag,
			  rv.base, lv.base, lv.wid);
	    }
	    fprintf(vvp_out, "    %%or 5, 4, 1;\n");
	    break;

	  case 'L':
	    lv = draw_eval_expr_wid(le, owid, STUFF_OK_XZ);
	    if (number_is_immediate(re,16,0) && !number_is_unknown(re)) {
		  long imm = get_number_immediate(re);
		  assert(imm >= 0);
		  fprintf(vvp_out, "   %%cmpi/%c %u, %ld, %u;\n", s_flag,
			  lv.base, imm, lv.wid);
	    } else {
		  rv = draw_eval_expr_wid(re, owid, STUFF_OK_XZ);
		  assert(lv.wid == rv.wid);
		  fprintf(vvp_out, "    %%cmp/%c %u, %u, %u;\n", s_flag,
			  lv.base, rv.base, lv.wid);
	    }
	    fprintf(vvp_out, "    %%or 5, 4, 1;\n");
	    break;

	  case '<':
	    lv = draw_eval_expr_wid(le, owid, STUFF_OK_XZ);
	    if (number_is_immediate(re,16,0) && !number_is_unknown(re)) {
		  long imm = get_number_immediate(re);
		  assert(imm >= 0);
		  fprintf(vvp_out, "   %%cmpi/%c %u, %ld, %u;\n", s_flag,
			  lv.base, imm, lv.wid);
	    } else {
		  rv = draw_eval_expr_wid(re, owid, STUFF_OK_XZ);
		  assert(lv.wid == rv.wid);
		  fprintf(vvp_out, "    %%cmp/%c %u, %u, %u;\n", s_flag,
			  lv.base, rv.base, lv.wid);
	    }
	    break;

	  case '>':
	    rv = draw_eval_expr_wid(re, owid, STUFF_OK_XZ);
	    if (number_is_immediate(le,16,0) && !number_is_unknown(le)) {
		  long imm = get_number_immediate(le);
		  assert(imm >= 0);
		  fprintf(vvp_out, "   %%cmpi/%c %u, %ld, %u;\n", s_flag,
			  rv.base, imm, rv.wid);
	    } else {
		  lv = draw_eval_expr_wid(le, owid, STUFF_OK_XZ);
		  assert(lv.wid == rv.wid);
		  fprintf(vvp_out, "    %%cmp/%c %u, %u, %u;\n", s_flag,
			  rv.base, lv.base, lv.wid);
	    }
	    break;

	  default:
	    assert(0);
      }

      if (lv.wid > 0) clr_vector(lv);
      if (rv.wid > 0) clr_vector(rv);

      if ((stuff_ok_flag&STUFF_OK_47) && (wid == 1)) {
	    lv.base = 5;
	    lv.wid = wid;
	    return lv;
      }

	/* Move the result out out the 4-7 bit that the compare
	   uses. This is because that bit may be clobbered by other
	   expressions. */
      { unsigned base = allocate_vector(wid);
	if (base == 0) {
	      fprintf(stderr, "%s:%u: vvp.tgt error: "
		      "Unable to allocate %u thread bits "
		      "for result of padded inequality compare.\n",
		      ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	      vvp_errors += 1;
	}

        fprintf(vvp_out, "    %%mov %u, 5, 1;\n", base);
	lv.base = base;
	lv.wid = wid;
	if (wid > 1)
	      fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base+1, wid-1);
      }

      return lv;
}

static struct vector_info draw_logic_immediate(ivl_expr_t expr,
					       ivl_expr_t le,
					       ivl_expr_t re,
					       unsigned wid)
{
      struct vector_info lv = draw_eval_expr_wid(le, wid, STUFF_OK_XZ);
      assert(number_is_immediate(re, IMM_WID, 0));
      assert(! number_is_unknown(re));
      unsigned long imm = get_number_immediate(re);

      assert(lv.base >= 4);

      switch (ivl_expr_opcode(expr)) {

	  case '&':
	    fprintf(vvp_out, "   %%andi %u, %lu, %u;\n", lv.base, imm, lv.wid);
	    break;

	  default:
	    assert(0);
	    break;
      }

      return lv;
}

static struct vector_info draw_binary_expr_logic(ivl_expr_t expr,
						 unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);
      struct vector_info lv;
      struct vector_info rv;

      if (ivl_expr_opcode(expr) == '&') {
	    if (number_is_immediate(re, IMM_WID, 0) && !number_is_unknown(re))
		  return draw_logic_immediate(expr, le, re, wid);
	    if (number_is_immediate(le, IMM_WID, 0) && !number_is_unknown(le))
		  return draw_logic_immediate(expr, re, le, wid);
      }

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
		  if (tmp.base == 0) {
			fprintf(stderr, "%s:%u: vvp.tgt error: "
				"Unable to allocate %u thread bits "
				"for result of binary logic.\n",
				ivl_expr_file(expr), ivl_expr_lineno(expr),
				wid);
			vvp_errors += 1;
		  }

		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, lv.base, tmp.wid);
		  lv = tmp;
	    }
      }

      switch (ivl_expr_opcode(expr)) {

	  case '&':
	    fprintf(vvp_out, "    %%and %u, %u, %u;\n",
		    lv.base, rv.base, wid);
	    break;

	  case '|':
	    fprintf(vvp_out, "    %%or %u, %u, %u;\n",
		    lv.base, rv.base, wid);
	    break;

	  case '^':
	    fprintf(vvp_out, "    %%xor %u, %u, %u;\n",
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
static struct vector_info draw_binary_expr_lrs(ivl_expr_t expr, unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);
      const char*opcode = "?";

      struct vector_info lv;

	/* Evaluate the expression that is to be shifted. */
      switch (ivl_expr_opcode(expr)) {

	  case 'l': /* << (left shift) */
	    lv = draw_eval_expr_wid(le, wid, 0);

	      /* Shifting 0 gets 0, if we can be sure the shift value
                 contains no 'x' or 'z' bits. */
	    if ((lv.base == 0) && (ivl_expr_value(re) == IVL_VT_BOOL))
		  return lv;

	    if (lv.base < 4) {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(lv.wid);
		  tmp.wid = lv.wid;
		  if (tmp.base == 0) {
			fprintf(stderr, "%s:%u: vvp.tgt error: "
				"Unable to allocate %u thread bits "
				"for result of left shift (<<).\n",
				ivl_expr_file(expr), ivl_expr_lineno(expr),
				wid);
			vvp_errors += 1;
		  }

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

	      /* Shifting 0 gets 0, if we can be sure the shift value
                 contains no 'x' or 'z' bits. */
	    if ((lv.base == 0) && (ivl_expr_value(re) == IVL_VT_BOOL))
		  return lv;

	    if (lv.base < 4) {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(lv.wid);
		  tmp.wid = lv.wid;
		  if (tmp.base == 0) {
			fprintf(stderr, "%s:%u: vvp.tgt error: "
				"Unable to allocate %u thread bits "
				"for result of right shift (>>).\n",
				ivl_expr_file(expr), ivl_expr_lineno(expr),
				lv.wid);
			vvp_errors += 1;
		  }

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

	      /* Shifting 0 gets 0, if we can be sure the shift value
                 contains no 'x' or 'z' bits. */
	    if ((lv.base == 0) && (ivl_expr_value(re) == IVL_VT_BOOL))
		  return lv;

	      /* Similarly, sign extending any constant bit begets itself,
                 if this expression is signed. */
	    if ((lv.base < 4) && ivl_expr_signed(expr)
                && (ivl_expr_value(re) == IVL_VT_BOOL))
		  return lv;

	    if (lv.base < 4) {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(lv.wid);
		  tmp.wid = lv.wid;
		  if (tmp.base == 0) {
			fprintf(stderr, "%s:%u: vvp.tgt error: "
				"Unable to allocate %u thread bits "
				"for result of right shift (>>>).\n",
				ivl_expr_file(expr), ivl_expr_lineno(expr),
				lv.wid);
			vvp_errors += 1;
		  }

		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, lv.base, lv.wid);
		  lv = tmp;
	    }

	    if (ivl_expr_signed(expr))
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
      eval_logic_into_integer(re,0);

      fprintf(vvp_out, "    %s/i0  %u, %u;\n", opcode, lv.base, lv.wid);

      if (lv.base >= 8)
	    save_expression_lookaside(lv.base, expr, lv.wid);

      return lv;
}

static struct vector_info draw_load_add_immediate(ivl_expr_t le,
						  ivl_expr_t re,
						  unsigned wid,
						  int signed_flag)
{
      struct vector_info lv;
      long imm;
      assert(! number_is_unknown(re));
      assert(number_is_immediate(re, IMM_WID, 1));
      imm = get_number_immediate(re);
      lv.base = allocate_vector(wid);
      lv.wid = wid;
      if (lv.base == 0) {
	    fprintf(stderr, "%s:%u: vvp.tgt error: "
		    "Unable to allocate %u thread bits "
		    "for result of addition.\n",
		    ivl_expr_file(le), ivl_expr_lineno(le), wid);
	    vvp_errors += 1;
      }

	/* Load the signal value with a %load that adds the index
	   register to the value being loaded. */
      draw_signal_dest(le, lv, signed_flag, imm);

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

      assert(! number_is_unknown(re));
      assert(number_is_immediate(re, IMM_WID, 0));
      imm = get_number_immediate(re);

	/* This shouldn't generally happen, because the elaborator
	   should take care of simple constant propagation like this,
	   but it doesn't have to and it is easy to catch here. */
      if (imm == 0)
	    return lv;

      switch (lv.base) {
	  case 0: /* Left expression is 0. */
	    lv.base = allocate_vector(wid);
	    if (lv.base == 0) {
		  fprintf(stderr, "%s:%u: vvp.tgt error: "
			  "Unable to allocate %u thread bits "
			  "for result of addition.\n",
			  ivl_expr_file(re), ivl_expr_lineno(re), wid);
		  vvp_errors += 1;
	    }
	    fprintf(vvp_out, "    %%movi %u, %lu %u;\n", lv.base, imm, wid);
	    break;

	  case 1: /* Left expression is 1...1 (i.e. -1) */
	    imm -= 1;
	    if (imm == 0) {
		  lv.base = 0;
	    } else {
		  lv.base = allocate_vector(wid);
		  if (lv.base == 0) {
			fprintf(stderr, "%s:%u: vvp.tgt error: "
				"Unable to allocate %u thread bits "
				"for result of addition.\n",
				ivl_expr_file(re), ivl_expr_lineno(re), wid);
			vvp_errors += 1;
		  }
		  fprintf(vvp_out, "    %%movi %u, %lu %u;\n", lv.base, imm, wid);
	    }
	    break;

	  case 2: /* Left expression is 'bx or 'bz */
	  case 3:
	    lv.base = 2;
	    break;

	  default: /* The regular case. */
	    fprintf(vvp_out, "    %%addi %u, %lu, %u;\n", lv.base, imm, wid);
	    break;
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

      assert(! number_is_unknown(re));
      assert(number_is_immediate(re, IMM_WID, 0));
      imm = get_number_immediate(re);
      if (imm == 0)
	    return lv;

      switch (lv.base) {
	  case 0:
	  case 1:
	    tmp = allocate_vector(wid);
	    if (tmp == 0) {
		  fprintf(stderr, "%s:%u: vvp.tgt error: "
			  "Unable to allocate %u thread bits "
			  "for result of subtraction.\n",
			  ivl_expr_file(le), ivl_expr_lineno(le), wid);
		  vvp_errors += 1;
	    }

	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n", tmp, lv.base, wid);
	    lv.base = tmp;
	    fprintf(vvp_out, "    %%subi %u, %lu, %u;\n", lv.base, imm, wid);
	    break;

	  case 2:
	  case 3:
	    lv.base = 2;
	    break;

	  default:
	    fprintf(vvp_out, "    %%subi %u, %lu, %u;\n", lv.base, imm, wid);
	    break;
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

      assert(! number_is_unknown(re));
      assert(number_is_immediate(re, IMM_WID, 0));
      imm = get_number_immediate(re);
      if (imm == 0)
	    return lv;

      fprintf(vvp_out, "    %%muli %u, %lu, %u;\n", lv.base, imm, lv.wid);

      return lv;
}

static struct vector_info draw_binary_expr_arith(ivl_expr_t expr, unsigned wid)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      struct vector_info lv;
      struct vector_info rv;

      int signed_flag = ivl_expr_signed(le) && ivl_expr_signed(re) ? 1 : 0;
      const char*sign_string = signed_flag ? "/s" : "";


      if ((ivl_expr_opcode(expr) == '+')
	  && (ivl_expr_type(le) == IVL_EX_SIGNAL)
	  && (ivl_expr_type(re) == IVL_EX_ULONG)
	  && number_is_immediate(re, IMM_WID, 1))
	    return draw_load_add_immediate(le, re, wid, signed_flag);

      if ((ivl_expr_opcode(expr) == '+')
	  && (ivl_expr_type(le) == IVL_EX_SIGNAL)
	  && (ivl_expr_type(re) == IVL_EX_NUMBER)
	  && (! number_is_unknown(re))
	  && number_is_immediate(re, IMM_WID, 1))
	    return draw_load_add_immediate(le, re, wid, signed_flag);

      if ((ivl_expr_opcode(expr) == '+')
	  && (ivl_expr_type(re) == IVL_EX_SIGNAL)
	  && (ivl_expr_type(le) == IVL_EX_ULONG)
	  && number_is_immediate(re, IMM_WID, 1))
	    return draw_load_add_immediate(re, le, wid, signed_flag);

      if ((ivl_expr_opcode(expr) == '+')
	  && (ivl_expr_type(re) == IVL_EX_SIGNAL)
	  && (ivl_expr_type(le) == IVL_EX_NUMBER)
	  && (! number_is_unknown(le))
	  && number_is_immediate(le, IMM_WID, 1))
	    return draw_load_add_immediate(re, le, wid, signed_flag);

      if ((ivl_expr_opcode(expr) == '+')
	  && (ivl_expr_type(re) == IVL_EX_ULONG)
	  && number_is_immediate(re, IMM_WID, 0))
	    return draw_add_immediate(le, re, wid);

      if ((ivl_expr_opcode(expr) == '+')
	  && (ivl_expr_type(re) == IVL_EX_NUMBER)
	  && (! number_is_unknown(re))
	  && number_is_immediate(re, IMM_WID, 0))
	    return draw_add_immediate(le, re, wid);

      if ((ivl_expr_opcode(expr) == '-')
	  && (ivl_expr_type(re) == IVL_EX_ULONG)
	  && number_is_immediate(re, IMM_WID, 0))
	    return draw_sub_immediate(le, re, wid);

      if ((ivl_expr_opcode(expr) == '-')
	  && (ivl_expr_type(re) == IVL_EX_NUMBER)
	  && (! number_is_unknown(re))
	  && number_is_immediate(re, IMM_WID, 0))
	    return draw_sub_immediate(le, re, wid);

      if ((ivl_expr_opcode(expr) == '*')
	  && (ivl_expr_type(re) == IVL_EX_NUMBER)
	  && (! number_is_unknown(re))
	  && number_is_immediate(re, IMM_WID, 0))
	    return draw_mul_immediate(le, re, wid);

      lv = draw_eval_expr_wid(le, wid, STUFF_OK_XZ);
      rv = draw_eval_expr_wid(re, wid, STUFF_OK_XZ|STUFF_OK_RO);

      if (lv.wid != wid) {
	    fprintf(stderr, "XXXX ivl_expr_opcode(expr) = %c,"
		    " lv.wid=%u, wid=%u\n", ivl_expr_opcode(expr),
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
	    if (tmp.base == 0) {
		  fprintf(stderr, "%s:%u: vvp.tgt error: "
			  "Unable to allocate %u thread bits "
			  "for result of arithmetic expression.\n",
			  ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
		  vvp_errors += 1;
	    }

	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n", tmp.base,
		    lv.base, wid);
	    lv = tmp;
      }

      switch (ivl_expr_opcode(expr)) {
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

	  case 'p':
	    if (ivl_expr_signed(le) || ivl_expr_signed(re)) {
		  fprintf(vvp_out, "    %%pow/s %u, %u, %u;\n",
		          lv.base, rv.base, wid);
	    } else {
		  fprintf(vvp_out, "    %%pow %u, %u, %u;\n",
		          lv.base, rv.base, wid);
	    }
	    break;

	  default:
	    assert(0);
      }

      clr_vector(rv);

      return lv;
}

static struct vector_info draw_binary_expr(ivl_expr_t expr,
					   unsigned wid,
					   int stuff_ok_flag)
{
      struct vector_info rv;
      int stuff_ok_used_flag = 0;

      switch (ivl_expr_opcode(expr)) {
	  case 'a': /* && (logical and) */
	    rv = draw_binary_expr_land(expr, wid);
	    break;

	  case 'E': /* === */
	  case 'e': /* == */
	  case 'N': /* !== */
	  case 'n': /* != */
	    rv = draw_binary_expr_eq(expr, wid, stuff_ok_flag);
	    stuff_ok_used_flag = 1;
	    break;

	  case '<':
	  case '>':
	  case 'L': /* <= */
	  case 'G': /* >= */
	    rv = draw_binary_expr_le(expr, wid, stuff_ok_flag);
	    stuff_ok_used_flag = 1;
	    break;

	  case '+':
	  case '-':
	  case '*':
	  case '/':
	  case '%':
	  case 'p':
	    rv = draw_binary_expr_arith(expr, wid);
	    break;

	  case 'l': /* << */
	  case 'r': /* >> */
	  case 'R': /* >>> */
	    rv = draw_binary_expr_lrs(expr, wid);
	    break;

	  case 'o': /* || (logical or) */
	    rv = draw_binary_expr_lor(expr, wid, stuff_ok_flag);
	    stuff_ok_used_flag = 1;
	    break;

	  case '&':
	  case '|':
	  case '^':
	  case 'A': /* NAND (~&) */
	  case 'O': /* NOR  (~|) */
	  case 'X': /* XNOR (~^) */
	    rv = draw_binary_expr_logic(expr, wid);
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt error: unsupported binary (%c)\n",
		    ivl_expr_opcode(expr));
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
		  save_expression_lookaside(rv.base, expr, wid);
      }

      return rv;
}

/*
 * The concatenation operator is evaluated by evaluating each sub-
 * expression, then copying it into the contiguous vector of the
 * result. Do this until the result vector is filled.
 */
static struct vector_info draw_concat_expr(ivl_expr_t expr, unsigned wid,
					   int stuff_ok_flag)
{
      unsigned off, rep, expr_wid, concat_wid, num_sube, idx;
      struct vector_info res;

      int alloc_exclusive = (stuff_ok_flag&STUFF_OK_RO) ? 0 : 1;

	/* Find out how wide the base concatenation expression is. */
      num_sube = ivl_expr_parms(expr);
      expr_wid = 0;
      for (idx = 0 ; idx < num_sube; idx += 1) {
	    expr_wid += ivl_expr_width(ivl_expr_parm(expr, idx));
      }

	/* Get the repeat count. This must be a constant that has been
	   evaluated at compile time. The operands will be repeated to
	   form the result. */
      rep = ivl_expr_repeat(expr);

	/* Allocate a vector to hold the result. */
      if (rep == 0) {
	      /* If the replication is zero we need to allocate temporary
	       * space to build the concatenation. */
	    res.base = allocate_vector(expr_wid);
	    res.wid = expr_wid;
      } else {
	    res.base = allocate_vector(wid);
	    res.wid = wid;
      }
      if (res.base == 0) {
	    fprintf(stderr, "%s:%u: vvp.tgt error: "
		    "Unable to allocate %u thread bits "
		    "for result of concatenation.\n",
		    ivl_expr_file(expr), ivl_expr_lineno(expr),
		    rep ? wid : expr_wid);
	    vvp_errors += 1;
      }

	/* If the result is the right size we can just build this in place. */
      concat_wid = rep*expr_wid;
      if (concat_wid <= wid) {
	    off = 0;
	      /* Evaluate the base expression. */
	    for (idx = num_sube; idx > 0; idx -= 1) {
		  ivl_expr_t arg = ivl_expr_parm(expr, idx-1);
		  unsigned awid = ivl_expr_width(arg);
		  struct vector_info avec;

		  assert(awid+off <= expr_wid);

		    /* Try to locate the subexpression in the
		     * lookaside map and use it when available. */
		  avec.base = allocate_vector_exp(arg, awid, alloc_exclusive);
		  avec.wid = awid;

		  if (avec.base != 0) {
			assert(awid == avec.wid);
			fprintf(vvp_out, "    %%mov %u, %u, %u; Reuse "
			                 "calculated expression.\n",
			                 res.base+off, avec.base, awid);
			clr_vector(avec);
		  } else {
			struct vector_info dest;

			dest.base = res.base+off;
			dest.wid  = awid;
			draw_eval_expr_dest(arg, dest, 0);
		  }

		  off += awid;
	    }

	      /* Now repeat the expression as needed. */
	    if (rep != 0) {
		  rep -= 1;
	    } else {
		    /* Clear the temporary space and return nothing.
		     * This will be caught in draw_eval_expr_dest()
		     * and dropped. */
		  clr_vector(res);
		  res.base = 0;
		  res.wid = 0;
	    }
	    while (rep > 0) {
		  fprintf(vvp_out, "    %%mov %u, %u, %u; Repetition %u\n",
		                   res.base+expr_wid*rep, res.base, expr_wid,
		                   rep+1);
		  rep -= 1;
	    }

	      /* Pad the expression when needed. */
	    if (wid > concat_wid) {
		    /* We can get a signed concatenation with $signed({...}). */
		  if (ivl_expr_signed(expr)) {
			unsigned base = res.base+concat_wid-1;
			for (idx = 1; idx <= wid-concat_wid; idx += 1) {
			      fprintf(vvp_out, "    %%mov %u, %u, 1;\n",
			                       base+idx, base);
			}
		  } else {
			fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
			                 res.base+concat_wid, wid-concat_wid);
		  }
	    }
      } else {
	      /* The concatenation is too big for the result so draw it
	       * at full width and then copy the bits that are needed. */
	    struct vector_info full_res;
	    full_res = draw_concat_expr(expr, concat_wid, stuff_ok_flag);
	    assert(full_res.base);

	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n", res.base,
	                     full_res.base, wid);
	    clr_vector(full_res);
      }

	/* Save the accumulated result in the lookaside map. */
      if (res.base >= 8)
	    save_expression_lookaside(res.base, expr, wid);

      return res;
}

/*
 * A number in an expression is made up by copying constant bits into
 * the allocated vector.
 */
static struct vector_info draw_number_expr(ivl_expr_t expr, unsigned wid)
{
      unsigned idx;
      unsigned nwid;
      struct vector_info res;
      const char*bits = ivl_expr_bits(expr);
      unsigned long val;
      unsigned val_bits;
      unsigned val_addr;

      res.wid  = wid;

      nwid = wid;
      if (ivl_expr_width(expr) < nwid)
	    nwid = ivl_expr_width(expr);

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
		default:
		  assert(0);
		  res.base = 0;
	    }
	    return res;
      }

	/* The number value needs to be represented as an allocated
	   vector. Allocate the vector and use %mov instructions to
	   load the constant bit values. */
      res.base = allocate_vector(wid);
      if (res.base == 0) {
	    fprintf(stderr, "%s:%u: vvp.tgt error: "
		    "Unable to allocate %u thread bits "
		    "for number value.\n",
		    ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	    vvp_errors += 1;
      }

	/* Detect the special case that the entire number fits in an
	   immediate. In this case we generate a single %movi
	  instruction. */
      if ((!number_is_unknown(expr)) && number_is_immediate(expr, IMM_WID,0)) {
	    unsigned long val2 = get_number_immediate(expr);
	    fprintf(vvp_out, "    %%movi %u, %lu, %u;\n", res.base, val2, wid);
	    return res;
      }

	/* Use %movi as much as possible to build the value into the
	   destination. Use the %mov to handle the remaining general
	   bits. */
      idx = 0;
      val = 0;
      val_bits = 0;
      val_addr = res.base;
      while (idx < nwid) {
	    char src = 0;
	    switch (bits[idx]) {
		case '0':
		  val_bits += 1;
		  break;
		case '1':
		  val |= 1UL << val_bits;
		  val_bits += 1;
		  break;
		case 'x':
		  src = '2';
		  break;
		case 'z':
		  src = '3';
		  break;
	    }

	    if (val_bits >= IMM_WID
		|| (val_bits>0 && src != 0)
		|| (val_bits>0 && idx+1==nwid)) {
		  fprintf(vvp_out, "    %%movi %u, %lu, %u;\n",
			  val_addr, val, val_bits);
		  val_addr += val_bits;
		  val_bits = 0;
		  val = 0;
	    }

	    if (src != 0) {
		  unsigned cnt;

		  assert(val_bits == 0);
		  for (cnt = 1 ; idx+cnt < nwid ; cnt += 1)
			if (bits[idx+cnt] != bits[idx])
			      break;

		  fprintf(vvp_out, "    %%mov %u, %c, %u;\n", val_addr, src, cnt);
		  val_addr += cnt;
		  idx += cnt-1;
	    }

	    idx += 1;
      }

	/* Pad the number up to the expression width. */
      if (idx < wid) {
	    if (ivl_expr_signed(expr) && bits[nwid-1] == '1')
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
	    save_expression_lookaside(res.base, expr, wid);

      return res;
}

/*
 * This little helper function generates the instructions to pad a
 * vector in place. It is assumed that the calling function has set up
 * the first sub_width bits of the dest vector, and the signed_flag is
 * true if the extension is to be signed.
 */
static void pad_in_place(struct vector_info dest, unsigned sub_width, int signed_flag)
{
      if (signed_flag) {
	    unsigned idx;
	    for (idx = sub_width ;  idx < dest.wid ;  idx += 1)
			fprintf(vvp_out, "    %%mov %u, %u, 1;\n",
				dest.base+idx, dest.base+sub_width-1);
      } else {
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n",
		    dest.base+sub_width, dest.wid - sub_width);
      }
}

/*
 * The PAD expression takes a smaller node and pads it out to a larger
 * value. It will zero extend or sign extend depending on the
 * signedness of the expression.
 */
static struct vector_info draw_pad_expr(ivl_expr_t expr, unsigned wid)
{
      struct vector_info subv;
      struct vector_info res;

      ivl_expr_t subexpr = ivl_expr_oper1(expr);

	/* If the sub-expression is at least as wide as the target
	   width, then instead of pad, we truncate. Evaluate the
	   expression and return that expression with the width
	   reduced to what we want. */
      if (wid <= ivl_expr_width(subexpr)) {
	    subv = draw_eval_expr(subexpr, 0);
	    assert(subv.wid >= wid);
	    res.base = subv.base;
	    res.wid = wid;
	    if (subv.base >= 8)
		  save_expression_lookaside(subv.base, expr, subv.wid);
	    return res;
      }

	/* So now we know that the subexpression is smaller then the
	   desired result (the usual case) so we build the
	   result. Evaluate the subexpression into the target buffer,
	   then pad it as appropriate. */
      res.base = allocate_vector(wid);
      if (res.base == 0) {
	    fprintf(stderr, "%s:%u: vvp.tgt error: "
		    "Unable to allocate %u thread bits "
		    "to pad expression.\n",
		    ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	    vvp_errors += 1;
      }

      res.wid = wid;

      subv.base = res.base;
      subv.wid = ivl_expr_width(subexpr);
      draw_eval_expr_dest(subexpr, subv, 0);

      pad_in_place(res, subv.wid, ivl_expr_signed(expr));

      save_expression_lookaside(res.base, expr, wid);
      return res;
}

static struct vector_info draw_realnum_expr(ivl_expr_t expr, unsigned wid)
{
      struct vector_info res;
      double val = ivl_expr_dvalue(expr);
      long ival = val;
      assert(wid <= 8*sizeof(long));

      unsigned addr, run, idx;
      int bit;

      fprintf(vvp_out, "; draw_realnum_expr(%f, %u) as %ld\n",
	      val, wid, ival);

      res.base = allocate_vector(wid);
      res.wid = wid;
      assert(res.base);

      addr = res.base;
      run = 1;
      bit = ival & 1;
      ival >>= 1;

      for (idx = 1 ;  idx < wid ;  idx += 1) {
	    int next_bit = ival & 1;
	    ival >>= 1;

	    if (next_bit == bit) {
		  run += 1;
		  continue;
	    }

	    fprintf(vvp_out, "    %%mov %u, %d, %u;\n", addr, bit, run);
	    addr += run;
	    run = 1;
	    bit = next_bit;
      }

      fprintf(vvp_out, "    %%mov %u, %d, %u;\n", addr, bit, run);


      return res;
}

static char *process_octal_codes(const char *in, unsigned width)
{
      unsigned idx = 0;
      unsigned ridx = 0;
      unsigned str_len = strlen(in);
      char *out = (char *)malloc(str_len+1);

        /* If we do not have any octal codes just return the input. */
      if (width/8 == str_len) {
	    strcpy(out, in);
	    return out;
      }

      while (ridx < str_len) {
	      /* An octal constant always has three digits. */
	    if (in[ridx] == '\\') {
		  out[idx] = (in[ridx+1]-'0')*64 + (in[ridx+2]-'0')*8 +
		            (in[ridx+3]-'0');
		  idx += 1;
		  ridx += 4;
	    } else {
		  out[idx] = in[ridx];
		  idx += 1;
		  ridx += 1;
	    }
      }
      out[idx] = '\0';

      return out;
}

/*
 * A string in an expression is made up by copying constant bits into
 * the allocated vector.
 */
static struct vector_info draw_string_expr(ivl_expr_t expr, unsigned wid)
{
      struct vector_info res;
      char *p, *fp;
      unsigned ewid, nwid;
      unsigned idx;

      res.wid  = wid;
      nwid = wid;
      ewid = ivl_expr_width(expr);
      if (ewid < nwid)
	    nwid = ewid;

        /* Our string may have embedded \xxx sequences so they need
           to be removed before we proceed. Returns a new string. */
      fp = process_octal_codes(ivl_expr_string(expr), ewid);
      p = fp;

      p += (ewid / 8) - 1;

	/* The string needs to be represented as an allocated
	   vector. Allocate the vector and use %mov instructions to
	   load the constant bit values. */
      res.base = allocate_vector(wid);
      if (res.base == 0) {
	    fprintf(stderr, "%s:%u: vvp.tgt error: "
		    "Unable to allocate %u thread bits "
		    "for string value.\n",
		    ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	    vvp_errors += 1;
      }

	/* Since this is a string, we know that all the bits are
	   defined and each character represents exactly 8 bits. Use
	   the %movi instruction to more efficiently move the string
	   around. */
      idx = 0;
      while (idx < nwid) {
	    unsigned bits;
	    unsigned trans = IMM_WID;
	    if (nwid-idx < trans)
		  trans = nwid-idx;

	    bits = *p;
	    p -= 1;
	    if (trans > 8) {
		  bits |= *p << 8;
		  p -= 1;
		  if (trans > 16) {
			bits |= *p << 16;
			p -= 1;
			if (trans > 24) {
			      bits |= *p << 24;
			      p -= 1;
			}
		  }
	    }
	    fprintf(vvp_out, "    %%movi %u, %u, %u;\n", res.base+idx,bits,trans);

	    idx += trans;
      }

	/* Pad the number up to the expression width. */
      if (idx < wid)
	    fprintf(vvp_out, "    %%mov %u, 0, %u;\n", res.base+idx, wid-idx);

      if (res.base >= 8)
	    save_expression_lookaside(res.base, expr, wid);

      free(fp);
      return res;
}

/*
* This function is given an expression, the preallocated vector
* result, and the swid that is filled in so far. The caller has
* already calculated the load swid bits of exp into the beginning of
* the res vector. This function just calculates the pad to fill out
* the res area.
*/
void pad_expr_in_place(ivl_expr_t expr, struct vector_info res, unsigned swid)
{
      if (res.wid <= swid)
	    return;

      if (ivl_expr_signed(expr)) {
	    unsigned idx;
	    for (idx = swid ;  idx < res.wid ;  idx += 1)
		  fprintf(vvp_out, "    %%mov %u, %u, 1;\n",
			  res.base+idx, res.base+swid-1);

      } else {
	    unsigned base = res.base+swid;
	    unsigned count = res.wid-swid;
	      /* The %movi is faster for larger widths, but for very
		 small counts, the %mov is faster. */
	    if (count > 4)
		  fprintf(vvp_out, "    %%movi %u, 0, %u;\n", base, count);
	    else
		  fprintf(vvp_out, "    %%mov %u, 0, %u;\n", base, count);
      }
}

/*
 * Evaluating a signal expression means loading the bits of the signal
 * into the thread bits. Remember to account for the part select by
 * offsetting the read from the lsi (least significant index) of the
 * signal.
 *
 * If the add_index is 0, then generate a %load/vp0 to add the
 * word0 value to the loaded value before storing it into the
 * destination. If the add_index is 1, then generate a %load/vp0/s to
 * do a signed load.
 */
static void draw_signal_dest(ivl_expr_t expr, struct vector_info res,
			     int add_index, long immediate)
{
      unsigned swid = ivl_expr_width(expr);
      ivl_signal_t sig = ivl_expr_signal(expr);

      unsigned word = 0;

      if (swid > res.wid)
	    swid = res.wid;

	/* If this is an access to an array, handle that by emitting a
	   load/av instruction. */
      if (ivl_signal_dimensions(sig) > 0) {
	    ivl_expr_t ix = ivl_expr_oper1(expr);

	    draw_eval_expr_into_integer(ix, 3);
	    if (add_index < 0) {
		  fprintf(vvp_out, "    %%load/av %u, v%p, %u;\n",
			  res.base, sig, swid);
		  pad_expr_in_place(expr, res, swid);
	    } else {
		  const char*sign_flag = (add_index>0)? "/s" : "";

		    /* Add an immediate value to an array value. */
		  fprintf(vvp_out, "    %%ix/load 0, %lu, 0;\n", immediate);
		  fprintf(vvp_out, "    %%load/avp0%s %u, v%p, %u;\n",
			  sign_flag, res.base, sig, res.wid);
	    }
	    return;
      }


      if (ivl_signal_data_type(sig) == IVL_VT_REAL) {
	    int tmp;

	    assert(add_index < 0);
	    tmp = allocate_word();
	    fprintf(vvp_out, "    %%load/wr %d, v%p_%u;\n", tmp, sig, word);
	    fprintf(vvp_out, "    %%cvt/vr %u, %d, %u;\n", res.base, tmp, res.wid);
	    clr_word(tmp);

      } else if (add_index >= 0) {

	    const char*sign_flag = add_index==1? "/s" : "";

	      /* If this is a REG (a variable) then I can do a vector read. */
	    if (immediate >= 0) {
		  fprintf(vvp_out, "    %%ix/load 0, %lu, 0;\n", immediate);
	    } else {
		  fprintf(vvp_out, "    %%ix/load 0, 0, 0; immediate=%ld\n", immediate);
		  fprintf(vvp_out, "    %%ix/sub 0, %ld, 0;\n", -immediate);
	    }
	    fprintf(vvp_out, "    %%load/vp0%s %u, v%p_%u, %u;\n", sign_flag,
		    res.base, sig,word, res.wid);
	    swid = res.wid;

      } else {

	      /* If this is a REG (a variable) then I can do a vector read. */
	    fprintf(vvp_out, "    %%load/v %u, v%p_%u, %u;\n",
		    res.base, sig, word, swid);

      }

      pad_expr_in_place(expr, res, swid);
}

static struct vector_info draw_signal_expr(ivl_expr_t expr, unsigned wid,
					   int stuff_ok_flag)
{
      struct vector_info res;

      int alloc_exclusive = (stuff_ok_flag&STUFF_OK_RO) ? 0 : 1;

	/* Already in the vector lookaside? */
      res.base = allocate_vector_exp(expr, wid, alloc_exclusive);
      res.wid = wid;
      if (res.base != 0) {
	    fprintf(vvp_out, "; Reuse signal base=%u wid=%u from lookaside.\n",
		    res.base, res.wid);
	    return res;
      }

      res.base = allocate_vector(wid);
      res.wid  = wid;
      if (res.base == 0) {
	    fprintf(stderr, "%s:%u: vvp.tgt error: "
		    "Unable to allocate %u thread bits "
		    "to load variable/wire.\n",
		    ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	    vvp_errors += 1;
      }

      save_expression_lookaside(res.base, expr, wid);

      draw_signal_dest(expr, res, -1, 0L);
      return res;
}

static struct vector_info draw_select_array(ivl_expr_t sube,
					    ivl_expr_t bit_idx,
					    unsigned bit_width,
					    unsigned wid)
{
      unsigned idx, label;
      ivl_signal_t sig = ivl_expr_signal(sube);
	/* unsigned sig_wid = ivl_expr_width(sube); */
      ivl_expr_t ix = ivl_expr_oper1(sube);

      struct vector_info shiv;
      struct vector_info res;

      shiv = draw_eval_expr(bit_idx, STUFF_OK_XZ|STUFF_OK_RO);
      draw_eval_expr_into_integer(ix, 3);
      label = local_count++;
	/* We can safely skip the bit index load below if the array word
	 * index is undefined. We need to do this so that the bit index
	 * load does not reset bit 4 to zero by loading a defined value. */
      fprintf(vvp_out, "    %%jmp/1 T_%d.%d, 4;\n", thread_count, label);
      if (ivl_expr_signed(bit_idx)) {
	    fprintf(vvp_out, "    %%ix/get/s 0, %u, %u;\n", shiv.base,
	                                                    shiv.wid);
      } else {
	    fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", shiv.base, shiv.wid);
      }
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, label);
      if (shiv.base >= 8)
	    clr_vector(shiv);

      res.base = allocate_vector(wid);
      res.wid = wid;
      if (res.base == 0) {
	    fprintf(stderr, "%s:%u: vvp.tgt error: "
		    "Unable to allocate %u thread bits "
		    "to hold selected value.\n",
		    ivl_expr_file(sube), ivl_expr_lineno(sube), wid);
	    vvp_errors += 1;
      }

      for (idx = 0 ;  idx < wid ;  idx += 1) {
	    fprintf(vvp_out, "    %%load/avx.p %u, v%p, 0;\n", res.base+idx, sig);
      }

      return res;
}

static struct vector_info draw_select_signal(ivl_expr_t expr,
					     ivl_expr_t sube,
					     ivl_expr_t bit_idx,
					     unsigned bit_wid,
					     unsigned wid)
{
      ivl_signal_t sig = ivl_expr_signal(sube);
      struct vector_info res;

	/* Use this word of the signal. */
      unsigned use_word = 0;
      unsigned use_wid, lab_x, lab_end;

	/* If this is an access to an array, try to get the index as a
	   constant. If it is (and the array is not a reg array then
	   this reduces to a signal access and we stay here. If it is
	   not constant, then give up and do an array index in front
	   of this part select. */

      if (ivl_signal_dimensions(sig) > 0) {
	    ivl_expr_t ix = ivl_expr_oper1(sube);

	    if (ivl_signal_type(sig)==IVL_SIT_REG
		|| !number_is_immediate(ix, IMM_WID, 0))
		  return draw_select_array(sube, bit_idx, bit_wid, wid);

	      /* The index is constant, so we can return to direct
	         readout with the specific word selected. */
	    assert(! number_is_unknown(ix));
	    use_word = get_number_immediate(ix);
      }

	/* Try the special case that the part is at the beginning of
	   the signal (or the entire width). Just load the early bits
	   in one go. */
      if (number_is_immediate(bit_idx, IMM_WID, 0)
	  && !number_is_unknown(bit_idx)
	  && get_number_immediate(bit_idx) == 0
	  && (ivl_expr_width(sube) >= bit_wid)) {

	    res.base = allocate_vector(wid);
	    res.wid = wid;
	    assert(res.base);
	    fprintf(vvp_out, "    %%load/v %u, v%p_%u, %u; Only need %u of %u bits\n",
		    res.base, sig, use_word, bit_wid, bit_wid, ivl_expr_width(sube));

	    save_signal_lookaside(res.base, sig, use_word, bit_wid);
	      /* Pad the part select to the desired width. Because of
	         $signed() this may be signed or unsigned (default). */
	    pad_expr_in_place(expr, res, bit_wid);
	    return res;
      }

	/* Alas, do it the hard way. */

      draw_eval_expr_into_integer(bit_idx, 1);

      res.base = allocate_vector(wid);
      res.wid = wid;
      assert(res.base);

      lab_x = local_count++;
      lab_end = local_count++;

	/* If the index is 'bx then we just return 'bx. */
      fprintf(vvp_out, "    %%jmp/1 T_%d.%d, 4;\n", thread_count, lab_x);

      use_wid = res.wid;
      if (use_wid > bit_wid)
	    use_wid = bit_wid;

      fprintf(vvp_out, "    %%load/x1p %u, v%p_%u, %u;\n",
	      res.base, sig, use_word, use_wid);
	/* Pad the part select to the desired width. Because of
	   $signed() this may be signed or unsigned (default). */
      pad_expr_in_place(expr, res, use_wid);

      fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count, lab_end);
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_x);
      fprintf(vvp_out, "    %%mov %u, 2, %u;\n", res.base, res.wid);
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_end);

      return res;
}

static void draw_select_signal_dest(ivl_expr_t expr,
				    ivl_expr_t sube,
				    ivl_expr_t bit_idx,
				    struct vector_info dest,
				    int stuff_ok_flag)
{
      struct vector_info tmp;
      ivl_signal_t sig = ivl_expr_signal(sube);

	/* Special case: If the operand is a signal (not an array) and
	   the part select is coming from the LSB, and the part select
	   is no larger then the signal itself, then we can load the
	   value in place, directly. */
      if ((ivl_signal_dimensions(sig) == 0)
	  && (ivl_expr_width(sube) >= dest.wid)
	  && number_is_immediate(bit_idx, IMM_WID, 0)
	  && ! number_is_unknown(bit_idx)
	  && get_number_immediate(bit_idx) == 0) {
	    unsigned use_word = 0;
	    fprintf(vvp_out, "    %%load/v %u, v%p_%u, %u; Select %u out of %u bits\n",
		    dest.base, sig, use_word, dest.wid,
		    dest.wid, ivl_expr_width(sube));
	    return;
      }

	/* Fallback, just draw the expression and copy the result into
	   the destination. */
      tmp = draw_select_signal(expr, sube, bit_idx, dest.wid, dest.wid);
      assert(tmp.wid == dest.wid);

      fprintf(vvp_out, "    %%mov %u, %u, %u; Move signal select into place\n",
	      dest.base, tmp.base, dest.wid);

      if (tmp.base >= 8) {
	    save_expression_lookaside(tmp.base, sube, tmp.wid);
	    clr_vector(tmp);
      }
}

static struct vector_info draw_select_expr(ivl_expr_t expr, unsigned wid,
					   int stuff_ok_flag)
{
      struct vector_info subv, shiv, res;
      ivl_expr_t sube  = ivl_expr_oper1(expr);
      ivl_expr_t shift = ivl_expr_oper2(expr);

      int alloc_exclusive = (stuff_ok_flag&STUFF_OK_RO)? 0 : 1;
      int cmp;
      unsigned lab_l, lab_end;

      res.wid = wid;

	/* First look for the self expression in the lookaside, and
	   allocate that if possible. If I find it, then immediately
	   return that. */
      if ( (res.base = allocate_vector_exp(expr, wid, alloc_exclusive)) != 0) {
	    fprintf(vvp_out, "; Reuse base=%u wid=%u from lookaside.\n",
		    res.base, wid);
	    return res;
      }

      if (ivl_expr_type(sube) == IVL_EX_SIGNAL) {
	    res = draw_select_signal(expr, sube, shift, ivl_expr_width(expr),
	                             wid);
	    fprintf(vvp_out, "; Save base=%u wid=%u in lookaside.\n",
		    res.base, wid);
	    save_expression_lookaside(res.base, expr, wid);
	    return res;
      }

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

	/* Detect and handle the special case that the shift is a
	   constant 0. Skip the shift, and return the subexpression
	   with the width trimmed down to the desired width. */
      if (shiv.base == 0) {
	    assert(subv.wid >= wid);
	    res.base = subv.base;
	    return res;
      }

      if (ivl_expr_signed(shift)) {
	    fprintf(vvp_out, "    %%ix/get/s 0, %u, %u;\n", shiv.base,
	                                                    shiv.wid);
      } else {
	    fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", shiv.base, shiv.wid);
      }
      clr_vector(shiv);

	/* If the subv result is a magic constant, then make a copy in
	   writable vector space and work from there instead. */
      if (subv.base < 4) {
	    res.base = allocate_vector(subv.wid);
	    res.wid = subv.wid;
	    assert(res.base);
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n", res.base,
		    subv.base, res.wid);
	    subv = res;
      }

      lab_l = local_count++;
      lab_end = local_count++;
	/* If we have an undefined index then just produce a 'bx result. */
      fprintf(vvp_out, "    %%jmp/1  T_%d.%d, 4;\n", thread_count, lab_l);

      cmp = allocate_word();
      assert(subv.wid >= wid);
	/* Determine if we need to shift a 'bx into the top. */
      fprintf(vvp_out, "    %%ix/load %u, %u, 0;\n", cmp, subv.wid - wid);
      fprintf(vvp_out, "    %%cmp/ws %u, 0;\n", cmp);
      clr_word(cmp);
      fprintf(vvp_out, "    %%jmp/1  T_%d.%d, 5;\n", thread_count, lab_l);
	/* Clear the cmp bit if the two values are equal. */
      fprintf(vvp_out, "    %%mov 4, 0, 1;\n");
      fprintf(vvp_out, "    %%shiftr/i0 %u, %u;\n", subv.base, subv.wid);
      fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count, lab_end);
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_l);
	/* Multiply by -1. */
      fprintf(vvp_out, "    %%ix/mul 0, %u, %u;\n", 0xFFFFFFFF, 0xFFFFFFFF);
      fprintf(vvp_out, "    %%shiftl/i0 %u, %u;\n", subv.base, subv.wid);
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_end);

      if (subv.wid > wid) {
	    res.base = subv.base;
	    res.wid = wid;

	    subv.base += wid;
	    subv.wid  -= wid;
	    clr_vector(subv);

      } else {
	    assert(subv.wid == wid);
	    res = subv;
      }

      if (res.base >= 8) {
	    fprintf(vvp_out, "; Save expression base=%u wid=%u in lookaside\n",
		    res.base, wid);
	    save_expression_lookaside(res.base, expr, wid);
      }

      return res;
}

static void draw_select_expr_dest(ivl_expr_t expr, struct vector_info dest,
				  int stuff_ok_flag)
{
      struct vector_info tmp;

      ivl_expr_t sube = ivl_expr_oper1(expr);
      ivl_expr_t shift= ivl_expr_oper2(expr);

	/* If the shift expression is not present, then this is really
	   a pad expression, and that can be handled pretty
	   easily. Evaluate the subexpression into the destination,
	   then pad in place. */
      if (shift == 0) {
	    struct vector_info subv;
	    subv.base = dest.base;
	    subv.wid = ivl_expr_width(sube);
	    if (subv.wid > dest.wid)
		  subv.wid = dest.wid;

	    draw_eval_expr_dest(sube, subv, stuff_ok_flag);

	    pad_in_place(dest, subv.wid, ivl_expr_signed(expr));
	    return;
      }

      if (ivl_expr_type(sube) == IVL_EX_SIGNAL) {
	    draw_select_signal_dest(expr, sube, shift, dest, stuff_ok_flag);
	    return;
      }

	/* Fallback, is to draw the expression by width, and mov it to
	   the required dest. */
      tmp = draw_select_expr(expr, dest.wid, stuff_ok_flag);
      assert(tmp.wid == dest.wid);

      fprintf(vvp_out, "    %%mov %u, %u, %u; Move select into place\n",
	      dest.base, tmp.base, dest.wid);

      if (tmp.base >= 8) {
	    save_expression_lookaside(tmp.base, expr, tmp.wid);
	    clr_vector(tmp);
      }
}

static struct vector_info draw_ternary_expr(ivl_expr_t expr, unsigned wid)
{
      struct vector_info res, tru, fal, tst;

      unsigned lab_true, lab_false, lab_out;
      ivl_expr_t cond = ivl_expr_oper1(expr);
      ivl_expr_t true_ex = ivl_expr_oper2(expr);
      ivl_expr_t false_ex = ivl_expr_oper3(expr);

      lab_true  = local_count++;
      lab_false = local_count++;
      lab_out = local_count++;

	/* Evaluate the condition expression, and if necessary reduce
	   it to a single bit. */
      tst = draw_eval_expr(cond, STUFF_OK_XZ|STUFF_OK_RO);
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
	    assert(tmp.base);
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
	    save_expression_lookaside(res.base, expr, wid);

      return res;
}

static struct vector_info draw_sfunc_expr(ivl_expr_t expr, unsigned wid)
{
      unsigned parm_count = ivl_expr_parms(expr);
      struct vector_info res;


	/* If the function has no parameters, then use this short-form
	   to draw the statement. */
      if (parm_count == 0) {
	    assert(ivl_expr_value(expr) == IVL_VT_LOGIC
		   || ivl_expr_value(expr) == IVL_VT_BOOL);
	    res.base = allocate_vector(wid);
	    res.wid  = wid;
	    assert(res.base);
	    fprintf(vvp_out, "    %%vpi_func %u %u \"%s\", %u, %u;\n",
		    ivl_file_table_index(ivl_expr_file(expr)),
		    ivl_expr_lineno(expr), ivl_expr_name(expr),
		    res.base, res.wid);
	    return res;

      }

      res = draw_vpi_func_call(expr, wid);

	/* New basic block starts after VPI calls. */
      clear_expression_lookaside();

      return res;
}

static struct vector_info draw_unary_expr(ivl_expr_t expr, unsigned wid)
{
      struct vector_info res;
      ivl_expr_t sub = ivl_expr_oper1(expr);
      const char *rop = 0;
      int inv = 0;

      switch (ivl_expr_opcode(expr)) {
	  case '&': rop = "and";  break;
	  case '|': rop = "or";   break;
	  case '^': rop = "xor";  break;
	  case 'A': rop = "nand"; break;
	  case 'N': rop = "nor";  break;
	  case 'X': rop = "xnor"; break;
      }

      switch (ivl_expr_opcode(expr)) {
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
		  if (tmp.base == 0) {
			fprintf(stderr, "%s:%u: vvp.tgt error: "
				"Unable to allocate %u thread bits "
				"to pad unary expression result.\n",
				ivl_expr_file(expr), ivl_expr_lineno(expr),
				wid);
			vvp_errors += 1;
		  }

		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, res.base, res.wid);
		  fprintf(vvp_out, "    %%movi %u, 0, %u;\n",
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
			assert(res.base);
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
	    } else {
		    /* We need to convert a 1'bz to 1'bx. */
		  assert(res.base >= 4);
		  fprintf(vvp_out, "    %%inv %u, 1;\n", res.base);
		  fprintf(vvp_out, "    %%inv %u, 1;\n", res.base);
	    }

	      /* If the result needs to be bigger then the calculated
		 value, then write it into a passed vector. */
	    if (res.wid < wid) {
		  struct vector_info tmp;
		  tmp.base = allocate_vector(wid);
		  tmp.wid = wid;
		  assert(res.base);
		  fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
			  tmp.base, res.base, res.wid);
		  fprintf(vvp_out, "    %%movi %u, 0, %u;\n",
			  tmp.base+res.wid, tmp.wid-res.wid);
		  clr_vector(res);
		  res = tmp;
	    }
	    break;

	  case 'm': /* abs() */
	    res = draw_eval_expr_wid(sub, wid, 0);

	    if (!ivl_expr_signed(sub))
		  break;

	    if (res.base == 0 || res.base == 2 || res.base == 3)
		  break;

	      /* Handle the special case of a -1 constant. Make the
	         result a 1. */
	    if (res.base == 1) {
		  res.base = allocate_vector(wid);
		  fprintf(vvp_out, "    %%movi %d, 1, %u;\n",
			  res.base, res.wid);
		  break;
	    }

	    fprintf(vvp_out, "    %%cmpi/s %d, 0, %u;\n", res.base, res.wid);
	    fprintf(vvp_out, "    %%jmp/0xz T_%u.%u, 5;\n", thread_count, local_count);
	    fprintf(vvp_out, "    %%inv %d, %u;\n", res.base, res.wid);
	    fprintf(vvp_out, "    %%addi %d, 1, %u;\n", res.base, res.wid);
	    fprintf(vvp_out, "T_%u.%u ;\n", thread_count, local_count);
	    local_count += 1;
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt error: unhandled unary operator: %c\n",
		    ivl_expr_opcode(expr));
	    assert(0);
      }

      if (res.base >= 8)
	    save_expression_lookaside(res.base, expr, wid);

      return res;
}

/*
 * Sometimes we know ahead of time where we want the expression value
 * to go. In that case, call this function. It will check to see if
 * the expression can be preplaced, and if so it will evaluate it in
 * place.
 */
static void draw_eval_expr_dest(ivl_expr_t expr, struct vector_info dest,
				int stuff_ok_flag)
{
      struct vector_info tmp;

      switch (ivl_expr_type(expr)) {

	  case IVL_EX_SIGNAL:
	    draw_signal_dest(expr, dest, -1, 0L);
	    return;

	  case IVL_EX_SELECT:
	    draw_select_expr_dest(expr, dest, stuff_ok_flag);
	    return;

	  default:
	    break;
      }

	/* Fallback, is to draw the expression by width, and move it to
	   the required dest. */
      tmp = draw_eval_expr_wid(expr, dest.wid, stuff_ok_flag);
      assert(tmp.wid == dest.wid);

	/* If the replication is 0 we can have a zero width, so skip it. */
      if (dest.wid) fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
                                     dest.base, tmp.base, dest.wid);

      if (tmp.base >= 8)
	    save_expression_lookaside(tmp.base, expr, tmp.wid);

      clr_vector(tmp);
}

struct vector_info draw_eval_expr_wid(ivl_expr_t expr, unsigned wid,
				      int stuff_ok_flag)
{
      struct vector_info res;

      switch (ivl_expr_type(expr)) {
	  default:
	  case IVL_EX_NONE:
	    fprintf(stderr, "%s:%u:  vvp.tgt error: unhandled expr. type: "
	            "%u at %s:%d\n", ivl_expr_file(expr), ivl_expr_lineno(expr),
                    ivl_expr_type(expr), __FILE__, __LINE__);
	    exit(1);
	    res.base = 0;
	    res.wid = 0;
	    break;
	  case IVL_EX_EVENT:
	    fprintf(stderr, "%s:%u: vvp.tgt error: A named event is not "
	                    "handled in this context (expression).\n",
	                    ivl_expr_file(expr), ivl_expr_lineno(expr));
	    exit(1);
	    break;

	  case IVL_EX_STRING:
	    res = draw_string_expr(expr, wid);
	    break;

	  case IVL_EX_BINARY:
	    res = draw_binary_expr(expr, wid, stuff_ok_flag);
	    break;

	  case IVL_EX_CONCAT:
	    res = draw_concat_expr(expr, wid, stuff_ok_flag);
	    break;

	  case IVL_EX_NUMBER:
	    res = draw_number_expr(expr, wid);
	    break;

	  case IVL_EX_REALNUM:
	    res = draw_realnum_expr(expr, wid);
	    break;

	  case IVL_EX_SELECT:
	    if (ivl_expr_oper2(expr) == 0)
		  res = draw_pad_expr(expr, wid);
	    else
		  res = draw_select_expr(expr, wid, stuff_ok_flag);
	    break;

	  case IVL_EX_SIGNAL:
	    res = draw_signal_expr(expr, wid, stuff_ok_flag);
	    break;

	  case IVL_EX_TERNARY:
	    res = draw_ternary_expr(expr, wid);
	    break;

	  case IVL_EX_SFUNC:
	    res = draw_sfunc_expr(expr, wid);
	    break;

	  case IVL_EX_UFUNC:
	    res = draw_ufunc_expr(expr, wid);
	    break;

	  case IVL_EX_UNARY:
	    res = draw_unary_expr(expr, wid);
	    break;
      }

      return res;
}

struct vector_info draw_eval_expr(ivl_expr_t expr, int stuff_ok_flag)
{
      return draw_eval_expr_wid(expr, ivl_expr_width(expr), stuff_ok_flag);
}
