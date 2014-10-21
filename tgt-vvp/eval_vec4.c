/*
 * Copyright (c) 2013 Stephen Williams (steve@icarus.com)
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
 * This file includes functions for evaluating VECTOR expressions.
 */
# include  "vvp_priv.h"
# include  <string.h>
# include  <stdlib.h>
# include  <math.h>
# include  <assert.h>
# include  <stdbool.h>

void resize_vec4_wid(ivl_expr_t expr, unsigned wid)
{
      if (ivl_expr_width(expr) == wid)
	    return;

      if (ivl_expr_signed(expr))
	    fprintf(vvp_out, "    %%pad/s %u;\n", wid);
      else
	    fprintf(vvp_out, "    %%pad/u %u;\n", wid);
}

static void draw_binary_vec4_arith(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      unsigned lwid = ivl_expr_width(le);
      unsigned rwid = ivl_expr_width(re);
      unsigned ewid = ivl_expr_width(expr);

      int signed_flag = ivl_expr_signed(le) && ivl_expr_signed(re) ? 1 : 0;
      const char*signed_string = signed_flag? "/s" : "";

	/* All the arithmetic operations handled here require that the
	   operands (and the result) be the same width. We further
	   assume that the core has not given us an operand wider then
	   the expression width. So padd operands as needed. */
      draw_eval_vec4(le, stuff_ok_flag);
      if (lwid != ewid) {
	    fprintf(vvp_out, "    %%pad/%c %u;\n", ivl_expr_signed(le)? 's' : 'u', ewid);
      }
      draw_eval_vec4(re, stuff_ok_flag);
      if (rwid != ewid) {
	    fprintf(vvp_out, "    %%pad/%c %u;\n", ivl_expr_signed(re)? 's' : 'u', ewid);
      }

      switch (ivl_expr_opcode(expr)) {
	  case '+':
	    fprintf(vvp_out, "    %%add;\n");
	    break;
	  case '-':
	    fprintf(vvp_out, "    %%sub;\n");
	    break;
	  case '*':
	    fprintf(vvp_out, "    %%mul;\n");
	    break;
	  case '/':
	    fprintf(vvp_out, "    %%div%s;\n", signed_string);
	    break;
	  case '%':
	    fprintf(vvp_out, "    %%mod%s;\n", signed_string);
	    break;
	  case 'p':
	      /* Note that the power operator is signed if EITHER of
		 the operands is signed. This is different from other
		 arithmetic operators. */
	    if (ivl_expr_signed(le) || ivl_expr_signed(re))
		  signed_string = "/s";
	    fprintf(vvp_out, "    %%pow%s;\n", signed_string);
	    break;

	  default:
	    assert(0);
	    break;
      }
}

static void draw_binary_vec4_bitwise(ivl_expr_t expr, int stuff_ok_flag)
{
      draw_eval_vec4(ivl_expr_oper1(expr), stuff_ok_flag);
      draw_eval_vec4(ivl_expr_oper2(expr), stuff_ok_flag);

      switch (ivl_expr_opcode(expr)) {
	  case '&':
	    fprintf(vvp_out, "    %%and;\n");
	    break;
	  case '|':
	    fprintf(vvp_out, "    %%or;\n");
	    break;
	  case '^':
	    fprintf(vvp_out, "    %%xor;\n");
	    break;
	  case 'A': /* ~& */
	    fprintf(vvp_out, "    %%nand;\n");
	    break;
	  case 'O': /* ~| */
	    fprintf(vvp_out, "    %%nor;\n");
	    break;
	  case 'X': /* ~^ */
	    fprintf(vvp_out, "    %%xnor;\n");
	    break;
	  default:
	    assert(0);
	    break;
      }
}

static void draw_binary_vec4_compare_real(ivl_expr_t expr)
{
      draw_eval_real(ivl_expr_oper1(expr));
      draw_eval_real(ivl_expr_oper2(expr));

      switch (ivl_expr_opcode(expr)) {
	  case 'e': /* == */
	    fprintf(vvp_out, "    %%cmp/wr;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    break;
	  case 'n': /* != */
	    fprintf(vvp_out, "    %%cmp/wr;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    fprintf(vvp_out, "    %%inv;\n");
	    break;
	  default:
	    assert(0);
      }
}

static void draw_binary_vec4_compare_string(ivl_expr_t expr)
{
      draw_eval_string(ivl_expr_oper1(expr));
      draw_eval_string(ivl_expr_oper2(expr));

      switch (ivl_expr_opcode(expr)) {
	  case 'e': /* == */
	    fprintf(vvp_out, "    %%cmp/str;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    break;
	  case 'n': /* != */
	    fprintf(vvp_out, "    %%cmp/str;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    fprintf(vvp_out, "    %%inv;\n");
	    break;
	  default:
	    assert(0);
      }
}

static void draw_binary_vec4_compare_class(ivl_expr_t expr)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      if (ivl_expr_type(le) == IVL_EX_NULL) {
	    ivl_expr_t tmp = le;
	    le = re;
	    re = tmp;
      }

	/* Special case: If both operands are null, then the
	   expression has a constant value. */
      if (ivl_expr_type(le)==IVL_EX_NULL && ivl_expr_type(re)==IVL_EX_NULL) {
	    switch (ivl_expr_opcode(expr)) {
		case 'e': /* == */
		  fprintf(vvp_out, "    %%pushi/vec4 1, 0, 1;\n");
		  break;
		case 'n': /* != */
		  fprintf(vvp_out, "    %%pushi/vec4 0, 0, 1;\n");
		  break;
		default:
		  assert(0);
		  break;
	    }
	    return;
      }

      if (ivl_expr_type(re)==IVL_EX_NULL && ivl_expr_type(le)==IVL_EX_SIGNAL) {
	    ivl_signal_t sig= ivl_expr_signal(le);

	    if (ivl_signal_dimensions(sig) == 0) {
		  fprintf(vvp_out, "    %%test_nul v%p_0;\n", sig);
	    } else {
		  ivl_expr_t word_ex = ivl_expr_oper1(le);
		  int word_ix = allocate_word();
		  draw_eval_expr_into_integer(word_ex, word_ix);
		  fprintf(vvp_out, "    %%test_nul/a v%p, %d;\n", sig, word_ix);
		  clr_word(word_ix);
	    }
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    if (ivl_expr_opcode(expr) == 'n')
		  fprintf(vvp_out, "    %%inv;\n");
	    return;
      }

      fprintf(stderr, "SORRY: Compare class handles not implemented\n");
      fprintf(vvp_out, " ; XXXX compare class handles.\n");
      vvp_errors += 1;
}

static void draw_binary_vec4_compare(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      if ((ivl_expr_value(le) == IVL_VT_REAL)
	  || (ivl_expr_value(re) == IVL_VT_REAL)) {
	    draw_binary_vec4_compare_real(expr);
	    return;
      }

      if ((ivl_expr_value(le)==IVL_VT_STRING)
	  && (ivl_expr_value(re)==IVL_VT_STRING)) {
	    draw_binary_vec4_compare_string(expr);
	    return;
      }

      if ((ivl_expr_value(le)==IVL_VT_STRING)
	  && (ivl_expr_type(re)==IVL_EX_STRING)) {
	    draw_binary_vec4_compare_string(expr);
	    return;
      }

      if ((ivl_expr_type(le)==IVL_EX_STRING)
	  && (ivl_expr_value(re)==IVL_VT_STRING)) {
	    draw_binary_vec4_compare_string(expr);
	    return;
      }

      if ((ivl_expr_value(le)==IVL_VT_CLASS)
	  && (ivl_expr_value(re)==IVL_VT_CLASS)) {
	    draw_binary_vec4_compare_class(expr);
	    return;
      }

      unsigned use_wid = ivl_expr_width(le);
      if (ivl_expr_width(re) > use_wid)
	    use_wid = ivl_expr_width(re);

      draw_eval_vec4(le, stuff_ok_flag);
      resize_vec4_wid(le, use_wid);

      draw_eval_vec4(re, stuff_ok_flag);
      resize_vec4_wid(re, use_wid);

      switch (ivl_expr_opcode(expr)) {
	  case 'e': /* == */
	    fprintf(vvp_out, "    %%cmp/u;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    break;
	  case 'n': /* != */
	    fprintf(vvp_out, "    %%cmp/u;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    fprintf(vvp_out, "    %%inv;\n");
	    break;
	  case 'E': /* === */
	    fprintf(vvp_out, "    %%cmp/u;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 6;\n");
	    break;
	  case 'N': /* !== */
	    fprintf(vvp_out, "    %%cmp/u;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 6;\n");
	    fprintf(vvp_out, "    %%inv;\n");
	    break;
	  default:
	    assert(0);
      }
}

static void draw_binary_vec4_land(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

	/* Push the left expression. Reduce it to a single bit if
	   necessary. */
      draw_eval_vec4(le, STUFF_OK_XZ);
      if (ivl_expr_width(le) > 1)
	    fprintf(vvp_out, "    %%or/r;\n");

	/* Now push the right expression. Again, reduce to a single
	   bit if necessasry. */
      draw_eval_vec4(re, STUFF_OK_XZ);
      if (ivl_expr_width(re) > 1)
	    fprintf(vvp_out, "    %%or/r;\n");

      fprintf(vvp_out, "    %%and;\n");

      if (ivl_expr_width(expr) > 1)
	    fprintf(vvp_out, "    %%pad/u %u;\n", ivl_expr_width(expr));
}

static void draw_binary_vec4_le_real(ivl_expr_t expr)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      switch (ivl_expr_opcode(expr)) {
	  case '<':
	    draw_eval_real(le);
	    draw_eval_real(re);
	    fprintf(vvp_out, "    %%cmp/wr;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    break;

	  case 'L': /* <= */
	    draw_eval_real(le);
	    draw_eval_real(re);
	    fprintf(vvp_out, "    %%cmp/wr;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    fprintf(vvp_out, "    %%or;\n");
	    break;

	  case '>':
	    draw_eval_real(re);
	    draw_eval_real(le);
	    fprintf(vvp_out, "    %%cmp/wr;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    break;

	  case 'G': /* >= */
	    draw_eval_real(re);
	    draw_eval_real(le);
	    fprintf(vvp_out, "    %%cmp/wr;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    fprintf(vvp_out, "    %%or;\n");
	    break;

	  default:
	    assert(0);
	    break;
      }
}

static void draw_binary_vec4_le_string(ivl_expr_t expr)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      switch (ivl_expr_opcode(expr)) {
	  case '<':
	    draw_eval_string(le);
	    draw_eval_string(re);
	    fprintf(vvp_out, "    %%cmp/str;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    break;

	  case 'L': /* <= */
	    draw_eval_string(le);
	    draw_eval_string(re);
	    fprintf(vvp_out, "    %%cmp/str;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    fprintf(vvp_out, "    %%or;\n");
	    break;

	  case '>':
	    draw_eval_string(re);
	    draw_eval_string(le);
	    fprintf(vvp_out, "    %%cmp/str;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    break;

	  case 'G': /* >= */
	    draw_eval_string(re);
	    draw_eval_string(le);
	    fprintf(vvp_out, "    %%cmp/str;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    fprintf(vvp_out, "    %%or;\n");
	    break;

	  default:
	    assert(0);
	    break;
      }
}

static void draw_binary_vec4_le(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);
      ivl_expr_t tmp;

      if ((ivl_expr_value(le) == IVL_VT_REAL)
	  || (ivl_expr_value(re) == IVL_VT_REAL)) {
	    draw_binary_vec4_le_real(expr);
	    return;
      }

      char use_opcode = ivl_expr_opcode(expr);
      char s_flag = (ivl_expr_signed(le) && ivl_expr_signed(re)) ? 's' : 'u';

	/* If this is a > or >=, then convert it to < or <= by
	   swapping the operands. Adjust the opcode to match. */
      switch (use_opcode) {
	  case 'G':
	    tmp = le;
	    le = re;
	    re = tmp;
	    use_opcode = 'L';
	    break;
	  case '>':
	    tmp = le;
	    le = re;
	    re = tmp;
	    use_opcode = '<';
	    break;
      }

      if ((ivl_expr_value(le)==IVL_VT_STRING)
	  && (ivl_expr_value(re)==IVL_VT_STRING)) {
	    draw_binary_vec4_le_string(expr);
	    return;
      }

      if ((ivl_expr_value(le)==IVL_VT_STRING)
	  && (ivl_expr_type(re)==IVL_EX_STRING)) {
	    draw_binary_vec4_le_string(expr);
	    return;
      }

      if ((ivl_expr_type(le)==IVL_EX_STRING)
	  && (ivl_expr_value(re)==IVL_VT_STRING)) {
	    draw_binary_vec4_le_string(expr);
	    return;
      }

	/* NOTE: I think I would rather the elaborator handle the
	   operand widths. When that happens, take this code out. */

      unsigned use_wid = ivl_expr_width(le);
      if (ivl_expr_width(re) > use_wid)
	    use_wid = ivl_expr_width(re);

      draw_eval_vec4(le, stuff_ok_flag);
      resize_vec4_wid(le, use_wid);

      draw_eval_vec4(re, stuff_ok_flag);
      resize_vec4_wid(re, use_wid);

      switch (use_opcode) {
	  case 'L':
	    fprintf(vvp_out, "    %%cmp/%c;\n", s_flag);
	    fprintf(vvp_out, "    %%flag_get/vec4 4;\n");
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    fprintf(vvp_out, "    %%or;\n");
	    break;
	  case '<':
	    fprintf(vvp_out, "    %%cmp/%c;\n", s_flag);
	    fprintf(vvp_out, "    %%flag_get/vec4 5;\n");
	    break;
	  default:
	    assert(0);
	    break;
      }
}

static void draw_binary_vec4_lor(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

	/* Push the left expression. Reduce it to a single bit if
	   necessary. */
      draw_eval_vec4(le, STUFF_OK_XZ);
      if (ivl_expr_width(le) > 1)
	    fprintf(vvp_out, "    %%or/r;\n");

	/* Now push the right expression. Again, reduce to a single
	   bit if necessasry. */
      draw_eval_vec4(re, STUFF_OK_XZ);
      if (ivl_expr_width(re) > 1)
	    fprintf(vvp_out, "    %%or/r;\n");

      fprintf(vvp_out, "    %%or;\n");

      if (ivl_expr_width(expr) > 1)
	    fprintf(vvp_out, "    %%pad/u %u;\n", ivl_expr_width(expr));
}

static void draw_binary_vec4_lrs(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

	// Push the left expression onto the stack.
      draw_eval_vec4(le, stuff_ok_flag);

	// Calculate the shift amount into an index register.
      int use_index_reg = allocate_word();
      assert(use_index_reg >= 0);
      draw_eval_expr_into_integer(re, use_index_reg);

	// Emit the actual shift instruction. This will pop the top of
	// the stack and replace it with the result of the shift.
      switch (ivl_expr_opcode(expr)) {
	  case 'l': /* << */
	    fprintf(vvp_out, "    %%shiftl %u;\n", use_index_reg);
	    break;
	  case 'r': /* >> */
	    fprintf(vvp_out, "    %%shiftr %u;\n", use_index_reg);
	    break;
	  case 'R': /* >>> */
	    if (ivl_expr_signed(le))
		  fprintf(vvp_out, "    %%shiftr/s %u;\n", use_index_reg);
	    else
		  fprintf(vvp_out, "    %%shiftr %u;\n", use_index_reg);
	    break;
	  default:
	    assert(0);
	    break;
      }

      clr_word(use_index_reg);
}

static void draw_binary_vec4(ivl_expr_t expr, int stuff_ok_flag)
{
      switch (ivl_expr_opcode(expr)) {
	  case 'a': /* Logical && */
	    draw_binary_vec4_land(expr, stuff_ok_flag);
	    break;

	  case '+':
	  case '-':
	  case '*':
	  case '/':
	  case '%':
	  case 'p': /* ** (power) */
	    draw_binary_vec4_arith(expr, stuff_ok_flag);
	    break;

	  case '&':
	  case '|':
	  case '^':
	  case 'A': /* NAND (~&) */
	  case 'O': /* NOR  (~|) */
	  case 'X': /* exclusive nor (~^) */
	    draw_binary_vec4_bitwise(expr, stuff_ok_flag);
	    break;

	  case 'e': /* == */
	  case 'E': /* === */
	  case 'n': /* !== */
	  case 'N': /* !== */
	    draw_binary_vec4_compare(expr, stuff_ok_flag);
	    break;

	  case 'G': /* >= */
	  case 'L': /* <= */
	  case '>':
	  case '<':
	    draw_binary_vec4_le(expr, stuff_ok_flag);
	    break;

	  case 'l': /* << */
	  case 'r': /* >> */
	  case 'R': /* >>> */
	    draw_binary_vec4_lrs(expr, stuff_ok_flag);
	    break;

	  case 'o': /* || (logical or) */
	    draw_binary_vec4_lor(expr, stuff_ok_flag);
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt error: unsupported binary (%c)\n",
		    ivl_expr_opcode(expr));
	    assert(0);
      }
}

static void draw_concat_vec4(ivl_expr_t expr, int stuff_ok_flag)
{
	/* Repeat the concatenation this many times to make a
	   super-concatenation. */
      unsigned repeat = ivl_expr_repeat(expr);
	/* This is the number of expressions that go into the
	   concatenation. */
      unsigned num_sube = ivl_expr_parms(expr);
      unsigned sub_idx;

      assert(num_sube > 0);

	/* Start with the least-significant bits. */
      draw_eval_vec4(ivl_expr_parm(expr, 0), stuff_ok_flag);

      for (sub_idx = 1 ; sub_idx < num_sube ; sub_idx += 1) {
	      /* Concatenate progressively higher parts. */
	    draw_eval_vec4(ivl_expr_parm(expr, sub_idx), stuff_ok_flag);
	    fprintf(vvp_out, "    %%concat/vec4;\n");
      }

      if (repeat > 1) {
	    fprintf(vvp_out, "    %%replicate %u;\n", repeat);
      }
}

static void draw_number_vec4(ivl_expr_t expr)
{
      unsigned long val0 = 0;
      unsigned long valx = 0;
      unsigned wid = ivl_expr_width(expr);
      const char*bits = ivl_expr_bits(expr);

      int idx;
      int accum = 0;
      int count_pushi = 0;

	/* Scan the literal bits, MSB first. */
      for (idx = 0 ; idx < wid ; idx += 1) {
	    val0 <<= 1;
	    valx <<= 1;
	    switch (bits[wid-idx-1]) {
		case '0':
		  break;
		case '1':
		  val0 |= 1;
		  break;
		case 'x':
		  val0 |= 1;
		  valx |= 1;
		  break;
		case 'z':
		  val0 |= 0;
		  valx |= 1;
		  break;
		default:
		  assert(0);
		  break;
	    }
	    accum += 1;
	    if (accum == 32) {
		  fprintf(vvp_out, "    %%pushi/vec4 %lu, %lu, 32;\n", val0, valx);
		  accum = 0;
		  val0 = 0;
		  valx = 0;
		    /* If there is already at least 1 pushi, then
		       concatenate this result to what we've done
		       already. */
		  if (count_pushi)
			fprintf(vvp_out, "    %%concat/vec4;\n");
		  count_pushi += 1;
	    }
      }

      if (accum) {
	    fprintf(vvp_out, "    %%pushi/vec4 %lu, %lu, %u;\n", val0, valx, accum);
	    if (count_pushi)
		  fprintf(vvp_out, "    %%concat/vec4;\n");
	    count_pushi += 1;
      }
}

static void draw_property_vec4(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      unsigned pidx = ivl_expr_property_idx(expr);

      fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
      fprintf(vvp_out, "    %%prop/v %u;\n", pidx);
      fprintf(vvp_out, "    %%pop/obj 1, 0;\n");
}

static void draw_select_vec4(ivl_expr_t expr)
{
	// This is the sub-expression to part-select.
      ivl_expr_t subexpr = ivl_expr_oper1(expr);
	// This is the base of the part select
      ivl_expr_t base = ivl_expr_oper2(expr);
	// This is the part select width
      unsigned wid = ivl_expr_width(expr);
	// Is the select base expression signed or unsigned?
      char sign_suff = ivl_expr_signed(base)? 's' : 'u';

	// Special Case: If the sub-expression is a STRING, then this
	// is a select from that string.
      if (ivl_expr_value(subexpr)==IVL_VT_STRING) {
	    assert(base);
	    assert(wid==8);
	    draw_eval_string(subexpr);
	    int base_idx = allocate_word();
	    draw_eval_expr_into_integer(base, base_idx);
	    fprintf(vvp_out, "    %%substr/vec4 %d, %u;\n", base_idx, wid);
	    fprintf(vvp_out, "    %%pop/str 1;\n");
	    clr_word(base_idx);
	    return;
      }

      if (ivl_expr_value(subexpr)==IVL_VT_DARRAY) {
	    ivl_signal_t sig = ivl_expr_signal(subexpr);
	    assert(sig);
	    assert(ivl_signal_data_type(sig)==IVL_VT_DARRAY);

	    assert(base);
	    draw_eval_expr_into_integer(base, 3);
	    fprintf(vvp_out, "    %%load/dar/vec4 v%p_0;\n", sig);

	    return;
      }

      draw_eval_vec4(subexpr, 0);
      draw_eval_vec4(base, 0);
      fprintf(vvp_out, "    %%part/%c %u;\n", sign_suff, wid);
}

static void draw_select_pad_vec4(ivl_expr_t expr, int stuff_ok_flag)
{
	// This is the sub-expression to pad/truncate
      ivl_expr_t subexpr = ivl_expr_oper1(expr);
	// This is the target width of the expression
      unsigned wid = ivl_expr_width(expr);

	// Push the sub-expression onto the stack.
      draw_eval_vec4(subexpr, stuff_ok_flag);

	// Special case: The expression is already the correct width,
	// so there is nothing to be done.
      if (wid == ivl_expr_width(subexpr))
	    return;

      if (ivl_expr_signed(expr))
	    fprintf(vvp_out, "    %%pad/s %u;\n", wid);
      else
	    fprintf(vvp_out, "    %%pad/u %u;\n", wid);
}

static void draw_sfunc_vec4(ivl_expr_t expr, int stuff_ok_flag)
{
      unsigned parm_count = ivl_expr_parms(expr);

	/* Special case: If there are no arguments to print, then the
	   %vpi_call statement is easy to draw. */
      if (parm_count == 0) {
	    assert(ivl_expr_value(expr)==IVL_VT_LOGIC
		   || ivl_expr_value(expr)==IVL_VT_BOOL);

	    fprintf(vvp_out, "    %%vpi_func %u %u \"%s\" %u {0 0 0};\n",
		    ivl_file_table_index(ivl_expr_file(expr)),
		    ivl_expr_lineno(expr), ivl_expr_name(expr),
		    ivl_expr_width(expr));
	    return;
      }

      draw_vpi_func_call(expr);
}

static void draw_signal_vec4(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);

	/* Handle the simple case, a signal expression that is a
	   simple vector, no array dimensions. */
      if (ivl_signal_dimensions(sig) == 0) {
	    fprintf(vvp_out, "    %%load/vec4 v%p_0;\n", sig);
	    return;
      }

	/* calculate the array index... */
      int addr_index = allocate_word();
      draw_eval_expr_into_integer(ivl_expr_oper1(expr), addr_index);

      fprintf(vvp_out, "    %%load/vec4a v%p, %d;\n", sig, addr_index);
      clr_word(addr_index);
}

static void draw_string_vec4(ivl_expr_t expr, int stuff_ok_flag)
{
      unsigned wid = ivl_expr_width(expr);
      char*fp = process_octal_codes(ivl_expr_string(expr), wid);
      char*p = fp;

      unsigned long tmp = 0;
      unsigned tmp_wid = 0;
      int push_flag = 0;

      for (unsigned idx = 0 ; idx < wid ; idx += 8) {
	    tmp <<= 8;
	    tmp |= *p;
	    p += 1;
	    tmp_wid += 8;
	    if (tmp_wid == 32) {
		  fprintf(vvp_out, "    %%pushi/vec4 %lu, 0, 32;\n", tmp);
		  tmp = 0;
		  tmp_wid = 0;
		  if (push_flag == 0)
			push_flag += 1;
		  else
			fprintf(vvp_out, "    %%concat/vec4;\n");
	    }
      }

      if (tmp_wid > 0) {
	    fprintf(vvp_out, "    %%pushi/vec4 %lu, 0, %u;\n", tmp, tmp_wid);
	    if (push_flag != 0)
		  fprintf(vvp_out, "    %%concat/vec4;\n");
      }

      free(fp);
}

static void draw_ternary_vec4(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t cond = ivl_expr_oper1(expr);
      ivl_expr_t true_ex = ivl_expr_oper2(expr);
      ivl_expr_t false_ex = ivl_expr_oper3(expr);

      unsigned lab_true  = local_count++;
      unsigned lab_out   = local_count++;

      int use_flag = allocate_flag();

	/* Evaluate the condition expression, including optionally
	   reducing it to a single bit. Put the result into a flag bit
	   for use by all the tests. */
      draw_eval_vec4(cond, STUFF_OK_XZ);
      if (ivl_expr_width(cond) > 1)
	    fprintf(vvp_out, "    %%or/r;\n");
      fprintf(vvp_out, "    %%flag_set/vec4 %d;\n", use_flag);

      fprintf(vvp_out, "    %%jmp/0 T_%u.%u, %d;\n", thread_count, lab_true, use_flag);

	/* If the condition is true or xz (not false), we need the true
	   expression. If the condition is true, then we ONLY need the
	   true expression. */
      draw_eval_vec4(true_ex, stuff_ok_flag);
      fprintf(vvp_out, "    %%jmp/1 T_%u.%u, %d;\n", thread_count, lab_out, use_flag);
      fprintf(vvp_out, "T_%u.%u ; End of true expr.\n", thread_count, lab_true);

	/* If the condition is false or xz (not true), we need the false
	   expression. If the condition is false, then we ONLY need
	   the false expression. */
      draw_eval_vec4(false_ex, stuff_ok_flag);
      fprintf(vvp_out, "    %%jmp/0 T_%u.%u, %d;\n", thread_count, lab_out, use_flag);
      fprintf(vvp_out, " ; End of false expr.\n");

	/* Here, the condition is not true or false, it is xz. Both
	   the true and false expressions have been pushed onto the
	   stack, we just need to blend the bits. */
      fprintf(vvp_out, "    %%blend;\n");
      fprintf(vvp_out, "T_%u.%u;\n", thread_count, lab_out);

      clr_flag(use_flag);
}

static void draw_unary_inc_dec(ivl_expr_t sub, bool incr, bool pre)
{
      ivl_signal_t sig = 0;
      unsigned wid = 0;

      switch (ivl_expr_type(sub)) {
	  case IVL_EX_SELECT: {
		ivl_expr_t e1 = ivl_expr_oper1(sub);
		sig = ivl_expr_signal(e1);
		wid = ivl_expr_width(e1);
		break;
	  }

	  case IVL_EX_SIGNAL:
	    sig = ivl_expr_signal(sub);
	    wid = ivl_expr_width(sub);
	    break;

	  default:
	    assert(0);
	    break;
      }

      draw_eval_vec4(sub, STUFF_OK_XZ);

      const char*cmd = incr? "%add" : "%sub";

      if (pre) {
	      /* prefix means we add the result first, and store the
		 result, as well as leaving a copy on the stack. */
	    fprintf(vvp_out, "    %%pushi/vec4 1, 0, %u;\n", wid);
	    fprintf(vvp_out, "    %s;\n", cmd);
	    fprintf(vvp_out, "    %%dup/vec4;\n");
	    fprintf(vvp_out, "    %%store/vec4 v%p_0, 0, %u;\n", sig, wid);

      } else {
	      /* The post-fix decrement returns the non-decremented
		 version, so there is a slight re-arrange. */
	    fprintf(vvp_out, "    %%dup/vec4;\n");
	    fprintf(vvp_out, "    %%pushi/vec4 1, 0, %u;\n", wid);
	    fprintf(vvp_out, "    %s;\n", cmd);
	    fprintf(vvp_out, "    %%store/vec4 v%p_0, 0, %u;\n", sig, wid);
      }
}

static void draw_unary_vec4(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t sub = ivl_expr_oper1(expr);

      switch (ivl_expr_opcode(expr)) {
	  case '&':
	    draw_eval_vec4(sub, stuff_ok_flag);
	    fprintf(vvp_out, "    %%and/r;\n");
	    break;

	  case '|':
	    draw_eval_vec4(sub, stuff_ok_flag);
	    fprintf(vvp_out, "    %%or/r;\n");
	    break;

	  case '^':
	    draw_eval_vec4(sub, stuff_ok_flag);
	    fprintf(vvp_out, "    %%xor/r;\n");
	    break;

	  case '~':
	    draw_eval_vec4(sub, stuff_ok_flag);
	    fprintf(vvp_out, "    %%inv;\n");
	    break;

	  case '!':
	    draw_eval_vec4(sub, STUFF_OK_XZ);
	    fprintf(vvp_out, "    %%nor/r;\n");
	    break;

	  case '-':
	    draw_eval_vec4(sub, stuff_ok_flag);
	    fprintf(vvp_out, "    %%inv;\n");
	    fprintf(vvp_out, "    %%pushi/vec4 1, 0, %u;\n", ivl_expr_width(sub));
	    fprintf(vvp_out, "    %%add;\n");
	    break;

	  case 'A': /* nand (~&) */
	    draw_eval_vec4(sub, stuff_ok_flag);
	    fprintf(vvp_out, "    %%nand/r;\n");
	    break;

	  case 'D': /* pre-decrement (--x) */
	    draw_unary_inc_dec(sub, false, true);
	    break;

	  case 'd': /* post_decrement (x--) */
	    draw_unary_inc_dec(sub, false, false);
	    break;

	  case 'I': /* pre-increment (++x) */
	    draw_unary_inc_dec(sub, true, true);
	    break;

	  case 'i': /* post-increment (x++) */
	    draw_unary_inc_dec(sub, true, false);
	    break;

	  case 'N': /* nor (~|) */
	    draw_eval_vec4(sub, stuff_ok_flag);
	    fprintf(vvp_out, "    %%nor/r;\n");
	    break;

	  case 'X': /* xnor (~^) */
	    draw_eval_vec4(sub, stuff_ok_flag);
	    fprintf(vvp_out, "    %%xnor/r;\n");
	    break;

	  case 'm': /* abs(m) */
	    draw_eval_vec4(sub, stuff_ok_flag);
	    if (! ivl_expr_signed(sub))
		  break;

	      /* Test if (m) < 0 */
	    fprintf(vvp_out, "    %%dup/vec4;\n");
	    fprintf(vvp_out, "    %%pushi/vec4 0, 0, %u;\n", ivl_expr_width(sub));
	    fprintf(vvp_out, "    %%cmp/s;\n");
	    fprintf(vvp_out, "    %%jmp/0xz T_%u.%u, 5;\n", thread_count, local_count);
	      /* If so, calculate -(m) */
	    fprintf(vvp_out, "    %%inv;\n");
	    fprintf(vvp_out, "    %%pushi/vec4 1, 0, %u;\n", ivl_expr_width(sub));
	    fprintf(vvp_out, "    %%add;\n");
	    fprintf(vvp_out, "T_%u.%u ;\n", thread_count, local_count);
	    local_count += 1;
	    break;

	  case 'v': /* Cast real to vec4 */
	    assert(ivl_expr_value(sub) == IVL_VT_REAL);
	    draw_eval_real(sub);
	    fprintf(vvp_out, "    %%cvt/vr %u;\n", ivl_expr_width(expr));
	    break;

	  case '2': /* Cast expression to bool */
	    switch (ivl_expr_value(sub)) {
		case IVL_VT_LOGIC:
		  draw_eval_vec4(sub, STUFF_OK_XZ);
		  fprintf(vvp_out, "    %%cast2;\n");
		  resize_vec4_wid(sub, ivl_expr_width(expr));
		  break;
		case IVL_VT_BOOL:
		  draw_eval_vec4(sub, 0);
		  break;
		case IVL_VT_REAL:
		  draw_eval_real(sub);
		  fprintf(vvp_out, "    %%cvt/vr %u;\n", ivl_expr_width(expr));
		  break;
		default:
		  assert(0);
		  break;
	    }
	    break;

	  default:
	    fprintf(stderr, "XXXX Unary operator %c not implemented\n", ivl_expr_opcode(expr));
	    break;
      }
}

void draw_eval_vec4(ivl_expr_t expr, int stuff_ok_flag)
{
      switch (ivl_expr_type(expr)) {
	  case IVL_EX_BINARY:
	    draw_binary_vec4(expr, stuff_ok_flag);
	    return;

	  case IVL_EX_CONCAT:
	    draw_concat_vec4(expr, stuff_ok_flag);
	    return;

	  case IVL_EX_NUMBER:
	    draw_number_vec4(expr);
	    return;

	  case IVL_EX_SELECT:
	    if (ivl_expr_oper2(expr)==0)
		  draw_select_pad_vec4(expr, stuff_ok_flag);
	    else
		  draw_select_vec4(expr);
	    return;

	  case IVL_EX_SFUNC:
	    draw_sfunc_vec4(expr, stuff_ok_flag);
	    return;

	  case IVL_EX_SIGNAL:
	    draw_signal_vec4(expr);
	    return;

	  case IVL_EX_STRING:
	    draw_string_vec4(expr, stuff_ok_flag);
	    return;

	  case IVL_EX_TERNARY:
	    draw_ternary_vec4(expr, stuff_ok_flag);
	    return;

	  case IVL_EX_UFUNC:
	    draw_ufunc_vec4(expr);
	    return;

	  case IVL_EX_UNARY:
	    draw_unary_vec4(expr, stuff_ok_flag);
	    return;

	  case IVL_EX_PROPERTY:
	    draw_property_vec4(expr);
	    return;

	  default:
	    break;
      }

      fprintf(stderr, "XXXX Evaluate VEC4 expression (%d)\n", ivl_expr_type(expr));
      fprintf(vvp_out, "; XXXX Evaluate VEC4 expression (%d)\n", ivl_expr_type(expr));
}
