/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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
# include  <assert.h>

static void fallback_eval(ivl_expr_t expr)
{
      struct vector_info res = draw_eval_expr(expr, 0);
      fprintf(vvp_out, "    %%pushv/str %u, %u; Cast BOOL/LOGIC to string\n",
	      res.base, res.wid);
      if (res.base > 0)
	    clr_vector(res);
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

static void string_ex_signal(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);

      if (ivl_signal_data_type(sig) == IVL_VT_STRING) {
	    fprintf(vvp_out, "    %%load/str v%p_0;\n", sig);
	    return;
      }

      fallback_eval(expr);
}

void draw_eval_string(ivl_expr_t expr)
{

      switch (ivl_expr_type(expr)) {
	  case IVL_EX_STRING:
	    fprintf(vvp_out, "    %%pushi/str \"%s\";\n", ivl_expr_string(expr));
	    break;

	  case IVL_EX_SIGNAL:
	    string_ex_signal(expr);
	    break;

	  case IVL_EX_CONCAT:
	    string_ex_concat(expr);
	    break;

	  default:
	    fallback_eval(expr);
	    break;
      }
}
