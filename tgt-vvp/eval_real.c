/*
 * Copyright (c) 2003-2010 Stephen Williams (steve@icarus.com)
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

/*
 * This file includes functions for evaluating REAL expressions.
 */
# include  "vvp_priv.h"
# include  <string.h>
# include  <stdlib.h>
# include  <math.h>
# include  <assert.h>

static unsigned long word_alloc_mask = 0x0f;

int allocate_word()
{
      int res = 4;
      int max = WORD_COUNT;

      while (res < max && (1U << res) & word_alloc_mask)
	    res += 1;

      if (res >= max) {
	    fprintf(stderr, "vvp.tgt error: Thread words exhausted.\n");
	    exit(1);
      }
      word_alloc_mask |= 1U << res;
      return res;
}

void clr_word(int res)
{
      int max = WORD_COUNT;
      assert(res < max);
      word_alloc_mask &= ~ (1U << res);
}

static void warn_about_large_cast(ivl_expr_t expr, unsigned wid)
{
      if (ivl_expr_signed(expr)) {
            if (wid > 64) {
                  fprintf(stderr, "%s:%u: tgt-vvp warning: V0.9 may give "
                                  "incorrect results when casting a signed "
                                  "value greater than 64 bits to a real "
                                  "value.\n",
                                  ivl_expr_file(expr), ivl_expr_lineno(expr));
            }
      } else {
            if (wid > 63) {
                  fprintf(stderr, "%s:%u: tgt-vvp warning: V0.9 may give "
                                  "incorrect results when casting an unsigned "
                                  "value greater than 63 bits to a real "
                                  "value.\n",
                                  ivl_expr_file(expr), ivl_expr_lineno(expr));
            }
      }
}

static int draw_binary_real(ivl_expr_t expr)
{
      int l, r = -1;

      switch (ivl_expr_opcode(expr)) {
	  case 'E':
	  case 'N':
	  case 'l':
	  case 'r':
	  case 'R':
	  case '&':
	  case '|':
	  case '^':
	  case 'A':
	  case 'O':
	  case 'X':
	    /* These should be caught in draw_eval_real(). */
	    assert(0);
      }

      l = draw_eval_real(ivl_expr_oper1(expr));
      r = draw_eval_real(ivl_expr_oper2(expr));

      switch (ivl_expr_opcode(expr)) {

	  case '+':
	    fprintf(vvp_out, "    %%add/wr %d, %d;\n", l, r);
	    break;

	  case '-':
	    fprintf(vvp_out, "    %%sub/wr %d, %d;\n", l, r);
	    break;

	  case '*':
	    fprintf(vvp_out, "    %%mul/wr %d, %d;\n", l, r);
	    break;

	  case '/':
	    fprintf(vvp_out, "    %%div/wr %d, %d;\n", l, r);
	    break;

	  case '%':
	    fprintf(vvp_out, "    %%mod/wr %d, %d;\n", l, r);
	    break;
	  case 'p':
	    fprintf(vvp_out, "    %%pow/wr %d, %d;\n", l, r);
	    break;

	  case 'm': { /* min(l,r) */
		int lab_out = local_count++;
		int lab_r = local_count++;
		  /* If r is NaN, the go out and accept l as result. */
		fprintf(vvp_out, "    %%cmp/wr %d, %d; Is NaN?\n", r, r);
		fprintf(vvp_out, "    %%jmp/0xz T_%d.%d, 4;\n", thread_count,
		        lab_out);
		  /* If l is NaN, the go out and accept r as result. */
		fprintf(vvp_out, "    %%cmp/wr %d, %d; Is NaN?\n", l, l);
		fprintf(vvp_out, "    %%jmp/0xz T_%d.%d, 4;\n", thread_count,
		        lab_r);
		  /* If l <= r then go out. */
		fprintf(vvp_out, "    %%cmp/wr %d, %d;\n", r, l);
		fprintf(vvp_out, "    %%jmp/0xz T_%d.%d, 5;\n", thread_count,
		        lab_out);
		  /* At this point we know we want r as the result. */
		fprintf(vvp_out, "T_%d.%d %%mov/wr %d, %d;\n", thread_count,
		        lab_r, l, r);
		fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_out);
		break;
	  }

	  case 'M': { /* max(l,r) */
		int lab_out = local_count++;
		int lab_r = local_count++;
		  /* If r is NaN, the go out and accept l as result. */
		fprintf(vvp_out, "    %%cmp/wr %d, %d; Is NaN?\n", r, r);
		fprintf(vvp_out, "    %%jmp/0xz T_%d.%d, 4;\n", thread_count,
		        lab_out);
		  /* If l is NaN, the go out and accept r as result. */
		fprintf(vvp_out, "    %%cmp/wr %d, %d; Is NaN?\n", l, l);
		fprintf(vvp_out, "    %%jmp/0xz T_%d.%d, 4;\n", thread_count,
		        lab_r);
		  /* if l >= r then go out. */
		fprintf(vvp_out, "    %%cmp/wr %d, %d;\n", l, r);
		fprintf(vvp_out, "    %%jmp/0xz T_%d.%d, 5;\n", thread_count,
		        lab_out);

		fprintf(vvp_out, "T_%d.%d %%mov/wr %d, %d;\n", thread_count,
		        lab_r, l, r);
		fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_out);
		break;
	  }
	  default:
	    fprintf(stderr, "XXXX draw_binary_real(%c)\n",
		    ivl_expr_opcode(expr));
	    assert(0);
      }

      if (r >= 0) clr_word(r);

      return l;
}

static int draw_number_real(ivl_expr_t expr)
{
      unsigned int idx;
      int res = allocate_word();
      const char*bits = ivl_expr_bits(expr);
      unsigned wid = ivl_expr_width(expr);
      unsigned long mant = 0, mask = -1UL;
      int vexp = 0x1000;

	/* If this is a negative number, then arrange for the 2's
	   complement to be calculated as we scan through the
	   value. Real values are sign-magnitude, and this negation
	   gets us a magnitude. */

      int negate = 0;
      int carry = 0;
      if (ivl_expr_signed(expr) && (bits[wid-1] == '1')) {
	    negate = 1;
	    carry = 1;
      }

      for (idx = 0 ;  idx < wid && idx < IMM_WID ;  idx += 1) {
	    mask <<= 1;
	    int cur_bit = bits[idx] == '1'? 1 : 0;

	    if (negate) {
		  cur_bit ^= 1;
		  cur_bit += carry;
		  carry = (cur_bit >> 1) & 1;
		  cur_bit &= 1;
	    }

	    if (cur_bit) mant |= 1 << idx;
      }

      for ( ; idx < wid ; idx += 1) {
	    if (ivl_expr_signed(expr) && (bits[idx] == bits[IMM_WID-1]))
		  continue;

	    if (bits[idx] == '0')
		  continue;

	    fprintf(stderr, "vvp.tgt error: mantissa doesn't fit!\n");
	    assert(0);
      }

	/* If required, add in a sign bit. */
      if (negate)
	    vexp |= 0x4000;

      fprintf(vvp_out, "    %%loadi/wr %d, %lu, %d; load(num)= %c%lu (wid=%u)\n",
	      res, mant, vexp, (vexp&0x4000)? '-' : '+', mant, wid);
      return res;
}

static int draw_realnum_real(ivl_expr_t expr)
{
      int res = allocate_word();
      double value = ivl_expr_dvalue(expr);

      double fract;
      int expo, vexp;
      unsigned long mant;
      int sign = 0;

	/* Handle the special case that the value is +-inf. */
      if (isinf(value)) {
	    if (value > 0)
		  fprintf(vvp_out, "    %%loadi/wr %d, 0, %d; load=+inf\n",
			  res, 0x3fff);
	    else
		  fprintf(vvp_out, "    %%loadi/wr %d, 0, %d; load=-inf\n",
			  res, 0x7fff);
	    return res;
      }
	/* Handle the special case that the value is NaN. */
      if (value != value) {
	    fprintf(vvp_out, "    %%loadi/wr %d, 1, %d; load=NaN\n",
	            res, 0x3fff);
	    return res;
      }

      if (value < 0) {
	    sign = 0x4000;
	    value *= -1;
      }

      fract = frexp(value, &expo);
      fract = ldexp(fract, 31);
      mant = fract;
      expo -= 31;

      vexp = expo + 0x1000;
      assert(vexp >= 0);
      assert(vexp < 0x2000);
      vexp += sign;

      fprintf(vvp_out, "    %%loadi/wr %d, %lu, %d; load=%#g\n",
	      res, mant, vexp, ivl_expr_dvalue(expr));

	/* Capture the residual bits, if there are any. Note that an
	   IEEE754 mantissa has 52 bits, 31 of which were accounted
	   for already. */
      fract -= floor(fract);
      fract = ldexp(fract, 22);
      mant = fract;
      expo -= 22;

      vexp = expo + 0x1000;
      assert(vexp >= 0);
      assert(vexp < 0x2000);
      vexp += sign;

      if (mant != 0) {
	    int tmp_word = allocate_word();
	    fprintf(vvp_out, "    %%loadi/wr %d, %lu, %d; load=%#g\n",
		    tmp_word, mant, vexp, ivl_expr_dvalue(expr));
	    fprintf(vvp_out, "    %%add/wr %d, %d;\n", res, tmp_word);
	    clr_word(tmp_word);
      }

      return res;
}

static int draw_sfunc_real(ivl_expr_t expr)
{
      struct vector_info sv;
      int res;
      const char*sign_flag = "";

      switch (ivl_expr_value(expr)) {

	  case IVL_VT_REAL:
	    if (ivl_expr_parms(expr) == 0) {
		  res = allocate_word();
		  fprintf(vvp_out, "    %%vpi_func/r %u %u \"%s\", %d;\n",
			  ivl_file_table_index(ivl_expr_file(expr)),
			  ivl_expr_lineno(expr), ivl_expr_name(expr), res);

	    } else {
		  res = draw_vpi_rfunc_call(expr);
	    }
	    break;

	  case IVL_VT_VECTOR:
	      /* If the value of the sfunc is a vector, then evaluate
		 it as a vector, then convert the result to a real
		 (via an index register) for the result. */
	    sv = draw_eval_expr(expr, 0);
	    clr_vector(sv);

	    if (ivl_expr_signed(expr))
		  sign_flag = "/s";

	    res = allocate_word();
	    fprintf(vvp_out, "    %%ix/get%s %d, %u, %u;\n",
		    sign_flag, res, sv.base, sv.wid);

            warn_about_large_cast(expr, sv.wid);
	    fprintf(vvp_out, "    %%cvt/ri %d, %d;\n", res, res);
	    break;

	  default:
	    assert(0);
	    res = -1;
      }

      return res;
}

/*
 * The real value of a signal is the integer value of a signal
 * converted to real.
 */
static int draw_signal_real_logic(ivl_expr_t expr)
{
      int res = allocate_word();
      struct vector_info sv = draw_eval_expr(expr, 0);
      const char*sign_flag = ivl_expr_signed(expr)? "/s" : "";

      fprintf(vvp_out, "    %%ix/get%s %d, %u, %u; logic signal as real\n",
	      sign_flag, res, sv.base, sv.wid);
      clr_vector(sv);

      warn_about_large_cast(expr, sv.wid);
      fprintf(vvp_out, "    %%cvt/ri %d, %d;\n", res, res);

      return res;
}

static int draw_signal_real_real(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      int res = allocate_word();

      if (ivl_signal_dimensions(sig) == 0) {
	    fprintf(vvp_out, "    %%load/wr %d, v%p_0;\n", res, sig);
	    return res;
      }

      ivl_expr_t word_ex = ivl_expr_oper1(expr);
      int word_ix = allocate_word();
      draw_eval_expr_into_integer(word_ex, word_ix);
      fprintf(vvp_out, "    %%load/ar %d, v%p, %d;\n", res, sig, word_ix);
      clr_word(word_ix);
      return res;
}

static int draw_signal_real(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      switch (ivl_signal_data_type(sig)) {
	  case IVL_VT_LOGIC:
	    return draw_signal_real_logic(expr);
	  case IVL_VT_REAL:
	    return draw_signal_real_real(expr);
	  default:
	    fprintf(stderr, "vvp.tgt error: signal_data_type=%d\n",
		    ivl_signal_data_type(sig));
	    assert(0);
	    return -1;
      }
}

/* If we have nested ternary operators they are likely tail recursive.
 * This code is structured to allow this recursion without overflowing
 * the available thread words. */
static int draw_ternary_real(ivl_expr_t expr)
{
      ivl_expr_t cond = ivl_expr_oper1(expr);
      ivl_expr_t true_ex = ivl_expr_oper2(expr);
      ivl_expr_t false_ex = ivl_expr_oper3(expr);

      struct vector_info tst;

      unsigned lab_true = local_count++;
      unsigned lab_move = local_count++;
      unsigned lab_out = local_count++;

      int tru, fal, res;

	/* Evaluate the ternary condition. */
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

	/* Evaluate the true expression second. */
      fprintf(vvp_out, "    %%jmp/1  T_%d.%d, %u;\n",
	      thread_count, lab_true, tst.base);

	/* Evaluate the false expression and copy it to the result word. */
      fal = draw_eval_real(false_ex);
      res = allocate_word();
      fprintf(vvp_out, "    %%mov/wr %d, %d;\n", res, fal);
      clr_word(fal);
      fprintf(vvp_out, "    %%jmp/0  T_%d.%d, %u; End of false expr.\n",
              thread_count, lab_out, tst.base);

	/* Evaluate the true expression. */
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_true);
      tru = draw_eval_real(true_ex);
      fprintf(vvp_out, "    %%jmp/1  T_%d.%d, %u; End of true expr.\n",
              thread_count, lab_move, tst.base);

	/* If the conditional is undefined then blend the real words. */
      fprintf(vvp_out, "    %%blend/wr %d, %d;\n", res, tru);
      fprintf(vvp_out, "    %%jmp  T_%d.%d; End of blend\n",
              thread_count, lab_out);

	/* If we only need the true result then copy it to the result word. */
      fprintf(vvp_out, "T_%d.%d ; Move true result.\n",
              thread_count, lab_move);
      fprintf(vvp_out, "    %%mov/wr %d, %d;\n", res, tru);
      clr_word(tru);

	/* This is the out label. */
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, lab_out);

      clr_vector(tst);

      return res;
}

static int draw_unary_real(ivl_expr_t expr)
{
      ivl_expr_t sube;
      int sub;

	/* If the opcode is a ~ then the sub expression must not be a
	 * real expression, so use vector evaluation and then convert
	 * that result to a real value. */
      if (ivl_expr_opcode(expr) == '~') {
	    struct vector_info vi;
	    int res;
	    const char*sign_flag;

	    vi = draw_eval_expr(expr, STUFF_OK_XZ);
	    res = allocate_word();
	    sign_flag = ivl_expr_signed(expr)? "/s" : "";
	    fprintf(vvp_out, "    %%ix/get%s %d, %u, %u;\n",
		    sign_flag, res, vi.base, vi.wid);

            warn_about_large_cast(expr, vi.wid);
	    fprintf(vvp_out, "    %%cvt/ri %d, %d;\n", res, res);

	    clr_vector(vi);
	    return res;
      }

      if (ivl_expr_opcode(expr) == '!') {
	    struct vector_info vi;
	    int res;
	    const char*sign_flag;

	    vi = draw_eval_expr(expr, STUFF_OK_XZ);
	    res = allocate_word();
	    sign_flag = ivl_expr_signed(expr)? "/s" : "";
	    fprintf(vvp_out, "    %%ix/get%s %d, %u, %u;\n",
		    sign_flag, res, vi.base, vi.wid);

            warn_about_large_cast(expr, vi.wid);
	    fprintf(vvp_out, "    %%cvt/ri %d, %d;\n", res, res);

	    clr_vector(vi);
	    return res;
      }

      sube = ivl_expr_oper1(expr);
      sub = draw_eval_real(sube);

      if (ivl_expr_opcode(expr) == '+')
	    return sub;

      if (ivl_expr_opcode(expr) == '-') {
	    int res = allocate_word();
	    fprintf(vvp_out, "    %%loadi/wr %d, 0, 0; load 0.0\n", res);
	    fprintf(vvp_out, "    %%sub/wr %d, %d;\n", res, sub);

	    clr_word(sub);
	    return res;
      }

      if (ivl_expr_opcode(expr) == 'm') { /* abs(sube) */
	    fprintf(vvp_out, "    %%abs/wr %d, %d;\n", sub, sub);
	    return sub;
      }

      fprintf(stderr, "vvp.tgt error: unhandled real unary operator: %c.\n",
              ivl_expr_opcode(expr));
      assert(0);
}

int draw_eval_real(ivl_expr_t expr)
{
      int res = 0;

	/* If this expression/sub-expression is not real then we need
	 * to evaluate it as a bit value and then convert the bit based
	 * result to a real value. This is required to get integer
	 * division to work correctly. */
      if (ivl_expr_value(expr) != IVL_VT_REAL) {
	    struct vector_info vi;
	    const char*sign_flag;

	    vi = draw_eval_expr(expr, STUFF_OK_XZ);
	    res = allocate_word();
	    sign_flag = ivl_expr_signed(expr)? "/s" : "";
	    fprintf(vvp_out, "    %%ix/get%s %d, %u, %u;\n", sign_flag, res,
	            vi.base, vi.wid);

            warn_about_large_cast(expr, vi.wid);
	    fprintf(vvp_out, "    %%cvt/ri %d, %d;\n", res, res);

	    clr_vector(vi);
	    return res;
      }

      switch (ivl_expr_type(expr)) {

	  case IVL_EX_BINARY:
	    res = draw_binary_real(expr);
	    break;

	  case IVL_EX_NUMBER:
	    res = draw_number_real(expr);
	    break;

	  case IVL_EX_REALNUM:
	    res = draw_realnum_real(expr);
	    break;

	  case IVL_EX_SFUNC:
	    res = draw_sfunc_real(expr);
	    break;

	  case IVL_EX_SIGNAL:
	    res = draw_signal_real(expr);
	    break;

	  case IVL_EX_TERNARY:
	    res = draw_ternary_real(expr);
	    break;

	  case IVL_EX_UFUNC:
	    res = draw_ufunc_real(expr);
	    break;

	  case IVL_EX_UNARY:
	    res = draw_unary_real(expr);
	    break;

	  default:
	    if (ivl_expr_value(expr) == IVL_VT_VECTOR) {
		  struct vector_info sv = draw_eval_expr(expr, 0);
		  const char*sign_flag = ivl_expr_signed(expr)? "/s" : "";

		  clr_vector(sv);
		  res = allocate_word();

		  fprintf(vvp_out, "    %%ix/get%s %d, %u, %u;\n",
			  sign_flag, res, sv.base, sv.wid);

                  warn_about_large_cast(expr, sv.wid);
		  fprintf(vvp_out, "    %%cvt/ri %d, %d;\n", res, res);

	    } else {
		  fprintf(stderr, "XXXX Evaluate real expression (%d)\n",
			  ivl_expr_type(expr));
		  fprintf(vvp_out, " ; XXXX Evaluate real expression (%d)\n",
			  ivl_expr_type(expr));
		  return 0;
	    }
	    break;
      }

      return res;
}
