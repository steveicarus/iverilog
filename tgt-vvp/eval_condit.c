/*
 * Copyright (c) 2014-2016 Stephen Williams (steve@icarus.com)
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

# include  "vvp_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static int draw_condition_fallback(ivl_expr_t expr)
{
      int use_flag = allocate_flag();

	/* Evaluate the condition expression, including optionally
	   reducing it to a single bit. Put the result into a flag bit
	   for use by all the tests. */
      draw_eval_vec4(expr);
      if (ivl_expr_width(expr) > 1)
	    fprintf(vvp_out, "    %%or/r;\n");

      fprintf(vvp_out, "    %%flag_set/vec4 %d;\n", use_flag);

      return use_flag;
}

static int draw_condition_binary_compare(ivl_expr_t expr)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      if ((ivl_expr_value(le) == IVL_VT_REAL)
	  || (ivl_expr_value(re) == IVL_VT_REAL)) {
	    return draw_condition_fallback(expr);
      }

      if ((ivl_expr_value(le)==IVL_VT_STRING)
	  && (ivl_expr_value(re)==IVL_VT_STRING)) {
	    return draw_condition_fallback(expr);
      }

      if ((ivl_expr_value(le)==IVL_VT_STRING)
	  && (ivl_expr_type(re)==IVL_EX_STRING)) {
	    return draw_condition_fallback(expr);
      }

      if ((ivl_expr_type(le)==IVL_EX_STRING)
	  && (ivl_expr_value(re)==IVL_VT_STRING)) {
	    return draw_condition_fallback(expr);
      }

      if ((ivl_expr_value(le)==IVL_VT_CLASS)
	  && (ivl_expr_value(re)==IVL_VT_CLASS)) {
	    return draw_condition_fallback(expr);
      }

      unsigned use_wid = ivl_expr_width(le);
      if (ivl_expr_width(re) > use_wid)
	    use_wid = ivl_expr_width(re);

	/* If the le is constant, then swap the operands so that we
	   can possibly take advantage of the immediate version of the
	   %cmp instruction. */
      if (ivl_expr_width(le)==use_wid && test_immediate_vec4_ok(le)) {
	    ivl_expr_t tmp = le;
	    le = re;
	    re = tmp;
      }

      draw_eval_vec4(le);
      resize_vec4_wid(le, use_wid);

      char use_opcode = ivl_expr_opcode(expr);


      if (ivl_expr_width(re)==use_wid && test_immediate_vec4_ok(re)) {
	      /* Special case: If the right operand can be handled as
		 an immediate operand, then use that instead. */
	    if (use_opcode=='n' || use_opcode=='N')
		  draw_immediate_vec4(re, "%cmpi/ne");
	    else
		  draw_immediate_vec4(re, "%cmpi/e");
      } else {
	    draw_eval_vec4(re);
	    resize_vec4_wid(re, use_wid);
	    if (use_opcode=='n' || use_opcode=='N')
		  fprintf(vvp_out, "    %%cmp/ne;\n");
	    else
		  fprintf(vvp_out, "    %%cmp/e;\n");
      }

      switch (ivl_expr_opcode(expr)) {
	  case 'n': /* != */
	  case 'e': /* == */
	    return 4;
	    break;
	  case 'N': /* !== */
	  case 'E': /* === */
	    return 6;
	  default:
	    assert(0);
	    return -1;
      }

}

static int draw_condition_binary_real_le(ivl_expr_t expr)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);
      ivl_expr_t tmp;

      char use_opcode = ivl_expr_opcode(expr);

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

      draw_eval_real(le);
      draw_eval_real(re);

      fprintf(vvp_out, "    %%cmp/wr;\n");

      switch (use_opcode) {
	  case '<':
	    return 5;
	  case 'L':
	    fprintf(vvp_out, "    %%flag_or 5, 4;\n");
	    return 5;
	  default:
	    assert(0);
	    return -1;
      }

}

static int draw_condition_binary_le(ivl_expr_t expr)
{
      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);
      ivl_expr_t tmp;

      if ((ivl_expr_value(le) == IVL_VT_REAL)
	  || (ivl_expr_value(re) == IVL_VT_REAL)) {
	    return draw_condition_binary_real_le(expr);
      }

      if ((ivl_expr_value(le)==IVL_VT_STRING)
	  && (ivl_expr_value(re)==IVL_VT_STRING)) {
	    return draw_condition_fallback(expr);
      }

      if ((ivl_expr_value(le)==IVL_VT_STRING)
	  && (ivl_expr_type(re)==IVL_EX_STRING)) {
	    return draw_condition_fallback(expr);
      }

      if ((ivl_expr_type(le)==IVL_EX_STRING)
	  && (ivl_expr_value(re)==IVL_VT_STRING)) {
	    return draw_condition_fallback(expr);
      }

      char use_opcode = ivl_expr_opcode(expr);
      char s_flag = (ivl_expr_signed(le) && ivl_expr_signed(re)) ? 's' : 'u';

      if (test_immediate_vec4_ok(le) && !test_immediate_vec4_ok(re)) {
	    tmp = le;
	    le = re;
	    re = tmp;

	    switch (use_opcode) {
		case 'G':
		  use_opcode = 'L';
		  break;
		case 'L':
		  use_opcode = 'G';
		  break;
		case '>':
		  use_opcode = '<';
		  break;
		case '<':
		  use_opcode = '>';
		  break;
		default:
		  assert(0);
		  break;
	    }

      } else if (!test_immediate_vec4_ok(re)) {
	      /* If this is a > or >=, then convert it to < or <= by
		 swapping the operands. Adjust the opcode to match. Do
		 this because the instruction really only supports <
		 and <= and we can avoid a %flag_inv instruction. */
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
      }

	/* NOTE: I think I would rather the elaborator handle the
	   operand widths. When that happens, take this code out. */

      unsigned use_wid = ivl_expr_width(le);
      if (ivl_expr_width(re) > use_wid)
	    use_wid = ivl_expr_width(re);

      draw_eval_vec4(le);
      resize_vec4_wid(le, use_wid);

      if (ivl_expr_width(re)==use_wid && test_immediate_vec4_ok(re)) {
	      /* Special case: If the right operand can be handled as
		 an immediate operand, then use that instead. */
	    char opcode[8];
	    snprintf(opcode, sizeof opcode, "%%cmpi/%c", s_flag);
	    draw_immediate_vec4(re, opcode);

      } else {
	    draw_eval_vec4(re);
	    resize_vec4_wid(re, use_wid);

	    fprintf(vvp_out, "    %%cmp/%c;\n", s_flag);
      }

      switch (use_opcode) {
	  case '>':
	    fprintf(vvp_out, "    %%flag_or 5, 4; GT is !LE\n");
	    fprintf(vvp_out, "    %%flag_inv 5;\n");
	    return 5;
	  case 'G':
	    fprintf(vvp_out, "    %%flag_inv 5; GE is !LT\n");
	    return 5;
	  case '<':
	    return 5;
	  case 'L':
	    fprintf(vvp_out, "    %%flag_or 5, 4;\n");
	    return 5;
	  default:
	    assert(0);
	    return -1;
      }
}

static int draw_condition_binary_lor(ivl_expr_t expr)
{
      unsigned label_out = local_count++;

      ivl_expr_t le = ivl_expr_oper1(expr);
      ivl_expr_t re = ivl_expr_oper2(expr);

      int lx = draw_eval_condition(le);
      int tmp_flag = lx;

      /* Short circuit right hand side if necessary */
      fprintf(vvp_out, "    %%jmp/1 T_%u.%u, %d;\n", thread_count, label_out, lx);

      if (lx < 8) {
	  tmp_flag = allocate_flag();
	  fprintf(vvp_out, "    %%flag_mov %d, %d;\n", tmp_flag, lx);
      }

      int rx = draw_eval_condition(re);

      /*
       * The flag needs to be in the same position regardless of whether the
       * right side is short-cicuited or not.
       */
      if (lx == tmp_flag) {
	  fprintf(vvp_out, "    %%flag_or %d, %d;\n", lx, rx);
      } else {
	  fprintf(vvp_out, "    %%flag_or %d, %d;\n", rx, tmp_flag);
	  if (lx != rx)
	      fprintf(vvp_out, "    %%flag_mov %d, %d;\n", lx, rx);
	  clr_flag(tmp_flag);
      }
      fprintf(vvp_out, "T_%u.%u;\n", thread_count, label_out);
      clr_flag(rx);
      return lx;
}

static int draw_condition_binary(ivl_expr_t expr)
{
      switch (ivl_expr_opcode(expr)) {
	  case 'e': /* == */
	  case 'E': /* === */
	  case 'n': /* != */
	  case 'N': /* !== */
	    return draw_condition_binary_compare(expr);
	  case '<':
	  case '>':
	  case 'L': /* <= */
	  case 'G': /* >= */
	    return draw_condition_binary_le(expr);
	  case 'o': /* Logical or (||) */
	    return draw_condition_binary_lor(expr);
	  default:
	    return draw_condition_fallback(expr);
      }
}

int draw_eval_condition(ivl_expr_t expr)
{
      switch (ivl_expr_type(expr)) {
	  case IVL_EX_BINARY:
	    return draw_condition_binary(expr);
	  default:
	    return draw_condition_fallback(expr);
      }
}
