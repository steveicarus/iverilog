/*
 * Copyright (c) 2003-2024 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * This file includes functions for evaluating REAL expressions.
 */
# include  "vvp_priv.h"
# include  <string.h>
# include  <stdlib.h>
# include  <math.h>
# include  <assert.h>
# include  <stdbool.h>

static unsigned long word_alloc_mask = 0x0f;

int allocate_word(void)
{
      int res = 4;

      while (res < WORD_COUNT && (1U << res) & word_alloc_mask)
	    res += 1;

      if (res >= WORD_COUNT) {
	    fprintf(stderr, "vvp.tgt error: Thread words (%d) exhausted "
	                    "during VVP code generation.\n", WORD_COUNT);
	    exit(1);
      }
      word_alloc_mask |= 1U << res;
      return res;
}

void clr_word(int res)
{
      assert(res < WORD_COUNT);
      word_alloc_mask &= ~ (1U << res);
}

static void draw_binary_real(ivl_expr_t expr)
{
      switch (ivl_expr_opcode(expr)) {
	  case 'E':
	  case 'N':
	  case 'w':
	  case 'W':
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

      draw_eval_real(ivl_expr_oper1(expr));
      draw_eval_real(ivl_expr_oper2(expr));

      switch (ivl_expr_opcode(expr)) {

	  case '+':
	    fprintf(vvp_out, "    %%add/wr;\n");
	    break;

	  case '-':
	    fprintf(vvp_out, "    %%sub/wr;\n");
	    break;

	  case '*':
	    fprintf(vvp_out, "    %%mul/wr;\n");
	    break;

	  case '/':
	    fprintf(vvp_out, "    %%div/wr;\n");
	    break;

	  case '%':
	    fprintf(vvp_out, "    %%mod/wr;\n");
	    break;
	  case 'p':
	    fprintf(vvp_out, "    %%pow/wr;\n");
	    break;

	  case 'm':
	    fprintf(vvp_out, "    %%min/wr;\n");
	    break;

	  case 'M':
	    fprintf(vvp_out, "    %%max/wr;\n");
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt error: draw_binary_real(%c)\n",
		    ivl_expr_opcode(expr));
	    assert(0);
      }
}

static void draw_number_real(ivl_expr_t expr)
{
      unsigned int idx;
      const char*bits = ivl_expr_bits(expr);
      unsigned wid = ivl_expr_width(expr);
      unsigned long mant = 0;
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

      fprintf(vvp_out, "    %%pushi/real %lu, %d; load(num)= %c%lu (wid=%u)\n",
	      mant, vexp, (vexp&0x4000)? '-' : '+', mant, wid);
}

static void draw_property_real(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      unsigned pidx = ivl_expr_property_idx(expr);

      fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
      fprintf(vvp_out, "    %%prop/r %u;\n", pidx);
      fprintf(vvp_out, "    %%pop/obj 1, 0;\n");
}

static void draw_realnum_real(ivl_expr_t expr)
{
      double value = ivl_expr_dvalue(expr);

      double fract;
      int expo, vexp;
      unsigned long mant;
      int sign = 0;

	/* Handle the special case that the value is +-inf. */
      if (isinf(value)) {
	    if (value > 0)
		  fprintf(vvp_out, "    %%pushi/real 0, %d; load=+inf\n",
			  0x3fff);
	    else
		  fprintf(vvp_out, "    %%pushi/real 0, %d; load=-inf\n",
			  0x7fff);
	    return;
      }
	/* Handle the special case that the value is NaN. */
      if (value != value) {
	    fprintf(vvp_out, "    %%pushi/real 1, %d; load=NaN\n",
	            0x3fff);
	    return;
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

      fprintf(vvp_out, "    %%pushi/real %lu, %d; load=%#g\n",
	      mant, vexp, ivl_expr_dvalue(expr));

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
	    fprintf(vvp_out, "    %%pushi/real %lu, %d; load=%#g\n",
		    mant, vexp, ivl_expr_dvalue(expr));
	    fprintf(vvp_out, "    %%add/wr;\n");
      }
}

/*
 * The real value of a logic expression is the integer value of the
 * expression converted to real.
 */
static void draw_real_logic_expr(ivl_expr_t expr)
{
      draw_eval_vec4(expr);
      const char*sign_flag = ivl_expr_signed(expr)? "/s" : "";
      fprintf(vvp_out, "    %%cvt/rv%s;\n", sign_flag);
}

static void draw_select_real(ivl_expr_t expr)
{
	/* The sube references the expression to be selected from. */
      ivl_expr_t sube = ivl_expr_oper1(expr);
	/* This is the select expression */
      ivl_expr_t shift= ivl_expr_oper2(expr);

	/* Assume the sub-expression is a signal */
      ivl_signal_t sig = ivl_expr_signal(sube);
      assert(ivl_signal_data_type(sig) == IVL_VT_DARRAY || ivl_signal_data_type(sig) == IVL_VT_QUEUE);

      draw_eval_expr_into_integer(shift, 3);
      fprintf(vvp_out, "    %%load/dar/r v%p_0;\n", sig);
}

static void real_ex_pop(ivl_expr_t expr)
{
      const char*fb;
      ivl_expr_t arg;

      if (strcmp(ivl_expr_name(expr), "$ivl_queue_method$pop_back")==0)
            fb = "b";
      else
            fb = "f";

      arg = ivl_expr_parm(expr, 0);
      assert(ivl_expr_type(arg) == IVL_EX_SIGNAL);

      fprintf(vvp_out, "    %%qpop/%s/real v%p_0;\n", fb, ivl_expr_signal(arg));
}

static void draw_sfunc_real(ivl_expr_t expr)
{
      switch (ivl_expr_value(expr)) {

	  case IVL_VT_REAL:
	    if (ivl_expr_parms(expr) == 0) {
		  fprintf(vvp_out, "    %%vpi_func/r %u %u \"%s\" {0 0 0};\n",
			  ivl_file_table_index(ivl_expr_file(expr)),
			  ivl_expr_lineno(expr), ivl_expr_name(expr));

	    } else {
		  draw_vpi_rfunc_call(expr);
	    }
	    break;

	  case IVL_VT_VECTOR:
	      /* If the value of the sfunc is a vector, then evaluate
		 it as a vector, then convert the result to a real
		 (via an index register) for the result. */
	    draw_real_logic_expr(expr);
	    break;

	  default:
	    assert(0);
      }

}

static void draw_signal_real_real(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);

	/* Special Case: If the signal is the return value of the function,
	   then use a different opcode to get the value. */
      if (signal_is_return_value(sig)) {
	    assert(ivl_signal_dimensions(sig) == 0);
	    fprintf(vvp_out, "    %%retload/real 0; Load %s (draw_signal_real_real)\n",
		    ivl_signal_basename(sig));
	    return;
      }


      if (ivl_signal_dimensions(sig) == 0) {
	    fprintf(vvp_out, "    %%load/real v%p_0;\n", sig);
	    return;
      }

      ivl_expr_t word_ex = ivl_expr_oper1(expr);
      int word_ix = allocate_word();
      draw_eval_expr_into_integer(word_ex, word_ix);
      fprintf(vvp_out, "    %%load/ar v%p, %d;\n", sig, word_ix);
      clr_word(word_ix);
}

static void draw_signal_real(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      switch (ivl_signal_data_type(sig)) {
	  case IVL_VT_LOGIC:
	    draw_real_logic_expr(expr);
	    return;
	  case IVL_VT_REAL:
	    draw_signal_real_real(expr);
	    return;
	  default:
	    fprintf(stderr, "vvp.tgt error: signal_data_type=%d\n",
		    ivl_signal_data_type(sig));
	    assert(0);
	    return;
      }
}

/* If we have nested ternary operators they are likely tail recursive.
 * This code is structured to allow this recursion without overflowing
 * the available thread words. */
static void draw_ternary_real(ivl_expr_t expr)
{
      ivl_expr_t cond = ivl_expr_oper1(expr);
      ivl_expr_t true_ex = ivl_expr_oper2(expr);
      ivl_expr_t false_ex = ivl_expr_oper3(expr);

      unsigned lab_true = local_count++;
      unsigned lab_out = local_count++;

      int cond_flag = allocate_flag();

	/* Evaluate the ternary condition. */
      draw_eval_vec4(cond);
      if (ivl_expr_width(cond) > 1)
	    fprintf(vvp_out, "    %%or/r;\n");

      fprintf(vvp_out, "    %%flag_set/vec4 %d;\n", cond_flag);


	/* Evaluate the true expression second. */
      fprintf(vvp_out, "    %%jmp/1  T_%u.%u, %d;\n",
	      thread_count, lab_true, cond_flag);

	/* Evaluate the false expression. */
      draw_eval_real(false_ex);
      fprintf(vvp_out, "    %%jmp/0  T_%u.%u, %d; End of false expr.\n",
              thread_count, lab_out, cond_flag);

	/* If the conditional is undefined then blend the real words. */
      draw_eval_real(true_ex);
      fprintf(vvp_out, "    %%blend/wr;\n");
      fprintf(vvp_out, "    %%jmp  T_%u.%u; End of blend\n",
              thread_count, lab_out);

	/* Evaluate the true expression. */
      fprintf(vvp_out, "T_%u.%u ;\n", thread_count, lab_true);
      draw_eval_real(true_ex);

	/* This is the out label. */
      fprintf(vvp_out, "T_%u.%u ;\n", thread_count, lab_out);

      clr_flag(cond_flag);
}

static void increment(ivl_expr_t e, bool pre)
{
      ivl_signal_t sig = ivl_expr_signal(e);
      fprintf(vvp_out, "    %%load/real v%p_0;\n", sig);
      if (!pre) fprintf(vvp_out, "    %%dup/real;\n");
      fprintf(vvp_out, "    %%pushi/real 1, 0x1000;\n");
      fprintf(vvp_out, "    %%add/wr;\n");
      if ( pre) fprintf(vvp_out, "    %%dup/real;\n");
      fprintf(vvp_out, "    %%store/real v%p_0;\n", sig);
}

static void decrement(ivl_expr_t e, bool pre)
{
      ivl_signal_t sig = ivl_expr_signal(e);
      fprintf(vvp_out, "    %%load/real v%p_0;\n", sig);
      if (!pre) fprintf(vvp_out, "    %%dup/real;\n");
      fprintf(vvp_out, "    %%pushi/real 1, 0x1000;\n");
      fprintf(vvp_out, "    %%sub/wr;\n");
      if ( pre) fprintf(vvp_out, "    %%dup/real;\n");
      fprintf(vvp_out, "    %%store/real v%p_0;\n", sig);
}

static void draw_unary_real(ivl_expr_t expr)
{
      ivl_expr_t sube;

	/* If the opcode is a ~ or a ! then the sub expression must not be
	 * a real expression, so use vector evaluation and then convert
	 * that result to a real value. */
      if ((ivl_expr_opcode(expr) == '~') || (ivl_expr_opcode(expr) == '!')) {
	    draw_real_logic_expr(expr);
	    return;
      }

      sube = ivl_expr_oper1(expr);

      if (ivl_expr_opcode(expr) == 'r') { /* Cast an integer value to a real. */
	    const char *suffix = "";
	    assert(ivl_expr_value(sube) != IVL_VT_REAL);
	    draw_eval_vec4(sube);
	    if (ivl_expr_signed(sube)) suffix = "/s";
	    fprintf(vvp_out, "    %%cvt/rv%s;\n", suffix);
	    return;
      }


      if (ivl_expr_opcode(expr) == '+') {
	    draw_eval_real(sube);
	    return;
      }

      if (ivl_expr_opcode(expr) == '-') {
	    fprintf(vvp_out, "    %%pushi/real 0, 0; load 0.0\n");
	    draw_eval_real(sube);
	    fprintf(vvp_out, "    %%sub/wr;\n");
	    return;
      }

      if (ivl_expr_opcode(expr) == 'm') { /* abs() */
	    draw_eval_real(sube);
	    fprintf(vvp_out, "    %%abs/wr;\n");
	    return;
      }

      if (ivl_expr_opcode(expr) == 'v') { /* Handled in eval_expr.c. */
            fprintf(stderr, "vvp.tgt error: real -> integer cast in real "
                            "context.\n");
	    assert(0);
      }

      switch (ivl_expr_opcode(expr)) {
	  case 'I':
	    increment(sube, true);
	    return;
	  case 'i':
	    increment(sube, false);
	    return;

	  case 'D':
	    decrement(sube, true);
	    return;
	  case 'd':
	    decrement(sube, false);
	    return;
	}

      fprintf(stderr, "vvp.tgt error: unhandled real unary operator: %c.\n",
              ivl_expr_opcode(expr));
      assert(0);
}

void draw_eval_real(ivl_expr_t expr)
{

	/* If this expression/sub-expression is not real then we need
	 * to evaluate it as a bit value and then convert the bit based
	 * result to a real value. This is required to get integer
	 * division to work correctly. */
      if (ivl_expr_value(expr) != IVL_VT_REAL) {
	    draw_real_logic_expr(expr);
	    return;
      }

      switch (ivl_expr_type(expr)) {

	  case IVL_EX_BINARY:
	    draw_binary_real(expr);
	    break;

	  case IVL_EX_NUMBER:
	    draw_number_real(expr);
	    break;

	  case IVL_EX_PROPERTY:
	    draw_property_real(expr);
	    break;

	  case IVL_EX_REALNUM:
	    draw_realnum_real(expr);
	    break;

	  case IVL_EX_SELECT:
	    draw_select_real(expr);
	    break;

	  case IVL_EX_SFUNC:
	    if (strcmp(ivl_expr_name(expr), "$ivl_queue_method$pop_back")==0)
		  real_ex_pop(expr);
	    else if (strcmp(ivl_expr_name(expr), "$ivl_queue_method$pop_front")==0)
		  real_ex_pop(expr);
	    else
		  draw_sfunc_real(expr);
	    break;

	  case IVL_EX_SIGNAL:
	    draw_signal_real(expr);
	    break;

	  case IVL_EX_TERNARY:
	    draw_ternary_real(expr);
	    break;

	  case IVL_EX_UFUNC:
	    draw_ufunc_real(expr);
	    break;

	  case IVL_EX_UNARY:
	    draw_unary_real(expr);
	    break;

	  default:
		fprintf(stderr, "vvp.tgt error: XXXX Evaluate real expression (%d)\n",
			  ivl_expr_type(expr));
		fprintf(vvp_out, " ; XXXX Evaluate real expression (%d)\n",
			  ivl_expr_type(expr));
	    break;
      }

}
