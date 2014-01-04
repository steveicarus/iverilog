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

static void draw_binary_vec4_arith(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      int signed_flag = ivl_expr_signed(le) && ivl_expr_signed(re) ? 1 : 0;
      const char*signed_string = signed_flag? "/s" : "";

      draw_eval_vec4(le, stuff_ok_flag);
      draw_eval_vec4(re, stuff_ok_flag);

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

static void draw_binary_vec4_compare(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      if ((ivl_expr_value(le) == IVL_VT_REAL)
	  || (ivl_expr_value(re) == IVL_VT_REAL)) {
	    draw_binary_vec4_compare_real(expr);
	    return;
      }

      draw_eval_vec4(le, stuff_ok_flag);
      draw_eval_vec4(re, stuff_ok_flag);

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

      draw_eval_vec4(le, stuff_ok_flag);
      draw_eval_vec4(re, stuff_ok_flag);

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
	    fprintf(vvp_out, "    %%shiftrs %u;\n", use_index_reg);
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

static void draw_unary_vec4(ivl_expr_t expr, int stuff_ok_flag)
{
      ivl_expr_t sub = ivl_expr_oper1(expr);

      switch (ivl_expr_opcode(expr)) {
	  case '~':
	    draw_eval_vec4(sub, stuff_ok_flag);
	    fprintf(vvp_out, "    %%inv;\n");
	    break;

	  case '!':
	    draw_eval_vec4(sub, STUFF_OK_XZ);
	    fprintf(vvp_out, "    %%nor/r;\n");
	    break;

	  default:
	    fprintf(stderr, "XXXX Unary operator %c no implemented\n", ivl_expr_opcode(expr));
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

	  case IVL_EX_TERNARY:
	    draw_ternary_vec4(expr, stuff_ok_flag);
	    return;

	  case IVL_EX_UNARY:
	    draw_unary_vec4(expr, stuff_ok_flag);
	    return;

	  default:
	    break;
      }

      fprintf(stderr, "XXXX Evaluate VEC4 expression (%d)\n", ivl_expr_type(expr));
      fprintf(vvp_out, "; XXXX Evaluate VEC4 expression (%d)\n", ivl_expr_type(expr));
}
