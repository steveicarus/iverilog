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
#ident "$Id: eval_real.c,v 1.10 2003/12/19 01:27:10 steve Exp $"
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
      int max = 8*sizeof(word_alloc_mask);

      while (res < max && (1U << res) & word_alloc_mask)
	    res += 1;

      assert(res < max);
      word_alloc_mask |= 1U << res;
      return res;
}

void clr_word(int res)
{
      int max = 8*sizeof(word_alloc_mask);
      assert(res < max);
      word_alloc_mask &= ~ (1U << res);
}


static int draw_binary_real(ivl_expr_t exp)
{
      int l, r = -1;

      l = draw_eval_real(ivl_expr_oper1(exp));
      r = draw_eval_real(ivl_expr_oper2(exp));

      switch (ivl_expr_opcode(exp)) {

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
	      { struct vector_info res = draw_eval_expr(exp, STUFF_OK_XZ);
		l = allocate_word();
		fprintf(vvp_out, "    %%ix/get %d, %u, %u;\n",
			l, res.base, res.wid);
		fprintf(vvp_out, "    %%cvt/ri %d, %d;\n", l, l);
	        clr_vector(res);
	      }
	      break;

	  default:
	    fprintf(stderr, "XXXX draw_binary_real(%c)\n",
		    ivl_expr_opcode(exp));
	    assert(0);
      }

      if (r >= 0) clr_word(r);

      return l;
}

static int draw_number_real(ivl_expr_t exp)
{
      unsigned int idx;
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
	    fprintf(vvp_out, "    %%loadi/wr %d, %lu, %d; load=%f\n",
		    tmp_word, mant, vexp, ivl_expr_dvalue(exp));
	    fprintf(vvp_out, "    %%add/wr %d, %d;\n", res, tmp_word);
	    clr_word(tmp_word);
      }

      return res;
}

static int draw_sfunc_real(ivl_expr_t exp)
{
      struct vector_info sv;
      int res;

      switch (ivl_expr_value(exp)) {

	  case IVL_VT_REAL:
	    if (ivl_expr_parms(exp) == 0) {
		  res = allocate_word();
		  fprintf(vvp_out, "    %%vpi_func/r \"%s\", %d;\n",
			  ivl_expr_name(exp), res);

	    } else {
		  res = draw_vpi_rfunc_call(exp);
	    }
	    break;

	  case IVL_VT_VECTOR:
	      /* If the value of the sfunc is a vector, then evaluate
		 it as a vector, then convert the result to a real
		 (via an index register) for the result. */
	    sv = draw_eval_expr(exp, 0);
	    clr_vector(sv);
	    
	    res = allocate_word();
	    fprintf(vvp_out, "    %%ix/get %d, %u, %u;\n",
		    res, sv.base, sv.wid);

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
	    if (ivl_expr_value(exp) == IVL_VT_VECTOR) {
		  struct vector_info sv = draw_eval_expr(exp, 0);

		  clr_vector(sv);
		  res = allocate_word();

		  fprintf(vvp_out, "    %%ix/get %d, %u, %u;\n", res,
			  sv.base, sv.wid);

		  fprintf(vvp_out, "    %%cvt/ri %d, %d;\n", res, res);

	    } else {
		  fprintf(stderr, "XXXX Evaluate real expression (%d)\n",
			  ivl_expr_type(exp));
		  fprintf(vvp_out, " ; XXXX Evaluate real expression (%d)\n",
			  ivl_expr_type(exp));
		  return 0;
	    }
	    break;
      }

      return res;
}


/*
 * $Log: eval_real.c,v $
 * Revision 1.10  2003/12/19 01:27:10  steve
 *  Fix various unsigned compare warnings.
 *
 * Revision 1.9  2003/05/25 02:50:08  steve
 *  Add % in real expressions.
 *
 * Revision 1.8  2003/04/23 02:22:47  steve
 *  Fix word register leak.
 *
 * Revision 1.7  2003/03/28 02:33:56  steve
 *  Add support for division of real operands.
 *
 * Revision 1.6  2003/03/15 04:45:18  steve
 *  Allow real-valued vpi functions to have arguments.
 *
 * Revision 1.5  2003/03/08 01:04:01  steve
 *  Excess precision breaks some targets.
 *
 * Revision 1.4  2003/02/07 02:46:16  steve
 *  Handle real value subtract and comparisons.
 *
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

