/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: eval_real.c,v 1.3 2003/01/28 04:15:50 steve Exp $"
#endif

/*
 * This file includes functions for evaluating REAL expressions.
 */
# include  "vvp_priv.h"
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <math.h>
# include  <assert.h>

static unsigned long word_alloc_mask = 0x0f;

int allocate_word()
{
      int res = 4;

      while ((1 << res) & word_alloc_mask)
	    res += 1;

      word_alloc_mask |= 1 << res;
      return res;
}

void clr_word(int res)
{
      assert(word_alloc_mask & (1 << res));
      word_alloc_mask &= ~ (1 << res);
}


static int draw_binary_real(ivl_expr_t exp)
{
      int l, r;

      l = draw_eval_real(ivl_expr_oper1(exp));
      r = draw_eval_real(ivl_expr_oper2(exp));

      switch (ivl_expr_opcode(exp)) {

	  case '+':
	    fprintf(vvp_out, "    %%add/wr %d, %d;\n", l, r);
	    clr_word(r);
	    break;

	  case '*':
	    fprintf(vvp_out, "    %%mul/wr %d, %d;\n", l, r);
	    clr_word(r);
	    break;

	  default:
	    assert(0);
      }

      return l;
}

static int draw_number_real(ivl_expr_t exp)
{
      int idx;
      int res = allocate_word();
      const char*bits = ivl_expr_bits(exp);
      unsigned wid = ivl_expr_width(exp);
      unsigned long mant = 0;

      for (idx = 0 ;  idx < wid ;  idx += 1) {
	    if (bits[idx] == '1')
		  mant |= 1 << idx;
      }

      fprintf(vvp_out, "    %%loadi/wr %d, %lu, 4096;\n", res, mant);
      return res;
}

/*
 * Evaluate a real variable expression by loading the real variable
 * into a real thread word.
 */
static int draw_variable_real(ivl_expr_t exp)
{
      int res = allocate_word();
      ivl_variable_t var = ivl_expr_variable(exp);

      fprintf(vvp_out, "    %%load/wr %d, W_%s;\n", res, vvp_word_label(var));
      return res;
}

static int draw_realnum_real(ivl_expr_t exp)
{
      int res = allocate_word();
      double value = ivl_expr_dvalue(exp);

      double fract;
      int expo, vexp;
      unsigned long mant;
      int sign = 0;

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

      fprintf(vvp_out, "    %%loadi/wr %d, %lu, %d; load=%f\n",
	      res, mant, vexp, ivl_expr_dvalue(exp));

	/* Capture the residual bits, if there are any. */
      fract -= floor(fract);
      fract = ldexp(fract, 32);
      mant = fract;
      expo -= 32;

      vexp = expo + 0x1000;
      assert(vexp >= 0);
      assert(vexp < 0x2000);
      vexp += sign;

      if (mant != 0) {
	    int tmp_word = allocate_word();
	    fprintf(vvp_out, "    %%loadi/wr %d, %lu, %d; load=%f\n",
		    tmp_word, mant, vexp, ivl_expr_dvalue(exp));
	    fprintf(vvp_out, "    %%add/wr %d, %d;\n", res, tmp_word);
	    clr_word(tmp_word);
      }

      return res;
}

static int draw_sfunc_real(ivl_expr_t exp)
{
      int res;
      assert(ivl_expr_parms(exp) == 0);
      assert(ivl_expr_value(exp) == IVL_VT_REAL);

      res = allocate_word();
      fprintf(vvp_out, "    %%vpi_func/r \"%s\", %d;\n",
	      ivl_expr_name(exp), res);

      return res;
}

/*
 * The real value of a signal is the integer value of a signal
 * converted to real.
 */
static int draw_signal_real(ivl_expr_t exp)
{
      int res = allocate_word();
      struct vector_info sv = draw_eval_expr(exp, 0);

      fprintf(vvp_out, "    %%ix/get %d, %u, %u;\n", res, sv.base, sv.wid);
      clr_vector(sv);

      fprintf(vvp_out, "    %%cvt/ri %d, %d;\n", res, res);

      return res;
}

int draw_eval_real(ivl_expr_t exp)
{
      int res = 0;

      switch (ivl_expr_type(exp)) {

	  case IVL_EX_BINARY:
	    res = draw_binary_real(exp);
	    break;

	  case IVL_EX_NUMBER:
	    res = draw_number_real(exp);
	    break;

	  case IVL_EX_REALNUM:
	    res = draw_realnum_real(exp);
	    break;

	  case IVL_EX_VARIABLE:
	    res = draw_variable_real(exp);
	    break;

	  case IVL_EX_SFUNC:
	    res = draw_sfunc_real(exp);
	    break;

	  case IVL_EX_SIGNAL:
	    res = draw_signal_real(exp);
	    break;

	  default:
	    fprintf(vvp_out, " ; XXXX Evaluate real expression (%d)\n",
		    ivl_expr_type(exp));
	    return 0;
      }

      return res;
}


/*
 * $Log: eval_real.c,v $
 * Revision 1.3  2003/01/28 04:15:50  steve
 *  Deliver residual bits of real value.
 *
 * Revision 1.2  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.1  2003/01/26 21:16:00  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 */

