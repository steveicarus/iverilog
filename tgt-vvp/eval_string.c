/*
 * Copyright (c) 2012-2013 Stephen Williams (steve@icarus.com)
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
# include  <string.h>
# include  <assert.h>

static void fallback_eval(ivl_expr_t expr)
{
      draw_eval_vec4(expr);
      fprintf(vvp_out, "    %%pushv/str; Cast BOOL/LOGIC to string\n");
}

static void string_ex_concat(ivl_expr_t expr)
{
      unsigned repeat;

      assert(ivl_expr_parms(expr) != 0);
      assert(ivl_expr_repeat(expr) != 0);

	/* Push the first string onto the stack, no matter what. */
      draw_eval_string(ivl_expr_parm(expr,0));

      for (repeat = 0 ; repeat < ivl_expr_repeat(expr) ; repeat += 1) {
	    unsigned idx;
	    for (idx = (repeat==0)? 1 : 0 ; idx < ivl_expr_parms(expr) ; idx += 1) {
		  ivl_expr_t sub = ivl_expr_parm(expr,idx);

		    /* Special case: If operand is a string literal,
		       then concat it using the %concati/str
		       instruction. */
		  if (ivl_expr_type(sub) == IVL_EX_STRING) {
			fprintf(vvp_out, "    %%concati/str \"%s\";\n",
				ivl_expr_string(sub));
			continue;
		  }

		  draw_eval_string(sub);
		  fprintf(vvp_out, "    %%concat/str;\n");
	    }
      }
}

static void string_ex_property(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      unsigned pidx = ivl_expr_property_idx(expr);

      fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
      fprintf(vvp_out, "    %%prop/str %u;\n", pidx);
      fprintf(vvp_out, "    %%pop/obj 1, 0;\n");
}

static void string_ex_signal(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);

      if (ivl_signal_data_type(sig) != IVL_VT_STRING) {
	    fallback_eval(expr);
	    return;
      }

	/* Special Case: If the signal is the return value of the
	   function, then use a different opcode to get the value. */
      if (signal_is_return_value(sig)) {
	    assert(ivl_signal_dimensions(sig) == 0);
	    fprintf(vvp_out, "    %%retload/str 0; Load %s (string_ex_signal)\n",
		    ivl_signal_basename(sig));
	    return;
      }

	/* Simple case: This is a simple variable. Generate a load
	   statement to load the string into the stack. */
      if (ivl_signal_dimensions(sig) == 0) {
	    fprintf(vvp_out, "    %%load/str v%p_0;\n", sig);
	    return;
      }

	/* There is a word select expression, so load the index into a
	   register and load from the array. */
      ivl_expr_t word_ex = ivl_expr_oper1(expr);
      int word_ix = allocate_word();
      draw_eval_expr_into_integer(word_ex, word_ix);
      fprintf(vvp_out, "    %%load/stra v%p, %d;\n", sig, word_ix);
      clr_word(word_ix);
}

static void string_ex_select(ivl_expr_t expr)
{
	/* The sube references the expression to be selected from. */
      ivl_expr_t sube = ivl_expr_oper1(expr);
	/* This is the select expression */
      ivl_expr_t shift= ivl_expr_oper2(expr);

	/* Assume the sub-expression is a signal */
      ivl_signal_t sig = ivl_expr_signal(sube);
      assert(ivl_signal_data_type(sig) == IVL_VT_DARRAY || ivl_signal_data_type(sig) == IVL_VT_QUEUE);

      draw_eval_expr_into_integer(shift, 3);
      fprintf(vvp_out, "    %%load/dar/str v%p_0;\n", sig);
}

static void string_ex_string(ivl_expr_t expr)
{
      const char*val = ivl_expr_string(expr);

	/* Special case: The elaborator converts the string "" to an
	   8-bit zero, which is in turn escaped to the 4-character
	   string \000. Detect this special case and convert it back
	   to an empty string. [Perhaps elaboration should be fixed?] */
      if (ivl_expr_width(expr)==8 && (strcmp(val,"\\000") == 0)) {
	    fprintf(vvp_out, "    %%pushi/str \"\";\n");
	    return;
      }

      fprintf(vvp_out, "    %%pushi/str \"%s\";\n", val);
}

static void string_ex_substr(ivl_expr_t expr)
{
      ivl_expr_t arg;
      unsigned arg1;
      unsigned arg2;
      assert(ivl_expr_parms(expr) == 3);

      arg = ivl_expr_parm(expr,0);
      draw_eval_string(arg);

	/* Evaluate the arguments... */
      arg = ivl_expr_parm(expr, 1);
      arg1 = allocate_word();
      draw_eval_expr_into_integer(arg, arg1);

      arg = ivl_expr_parm(expr, 2);
      arg2 = allocate_word();
      draw_eval_expr_into_integer(arg, arg2);

      fprintf(vvp_out, "    %%substr %u, %u;\n", arg1, arg2);
      clr_word(arg1);
      clr_word(arg2);
}

static void string_ex_pop(ivl_expr_t expr)
{
      const char*fb;
      ivl_expr_t arg;

      if (strcmp(ivl_expr_name(expr), "$ivl_queue_method$pop_back")==0)
	    fb = "b";
      else
	    fb = "f";

      arg = ivl_expr_parm(expr, 0);
      assert(ivl_expr_type(arg) == IVL_EX_SIGNAL);

      fprintf(vvp_out, "    %%qpop/%s/str v%p_0;\n", fb, ivl_expr_signal(arg));
}

static void draw_sfunc_string(ivl_expr_t expr)
{
    assert(ivl_expr_value(expr) == IVL_VT_STRING);
    draw_vpi_sfunc_call(expr);
}

void draw_eval_string(ivl_expr_t expr)
{

      switch (ivl_expr_type(expr)) {
	  case IVL_EX_STRING:
	    string_ex_string(expr);
	    break;

	  case IVL_EX_SIGNAL:
	    string_ex_signal(expr);
	    break;

	  case IVL_EX_CONCAT:
	    string_ex_concat(expr);
	    break;

	  case IVL_EX_PROPERTY:
	    string_ex_property(expr);
	    break;

	  case IVL_EX_SELECT:
	    string_ex_select(expr);
	    break;

	  case IVL_EX_SFUNC:
	    if (strcmp(ivl_expr_name(expr), "$ivl_string_method$substr") == 0)
		  string_ex_substr(expr);
	    else if (strcmp(ivl_expr_name(expr), "$ivl_queue_method$pop_back")==0)
		  string_ex_pop(expr);
	    else if (strcmp(ivl_expr_name(expr), "$ivl_queue_method$pop_front")==0)
		  string_ex_pop(expr);
	    else
		  draw_sfunc_string(expr);
	    break;

	  case IVL_EX_UFUNC:
	    draw_ufunc_string(expr);
	    break;

	  default:
	    fallback_eval(expr);
	    break;
      }
}
