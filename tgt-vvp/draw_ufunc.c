/*
 * Copyright (c) 2005-2010 Stephen Williams (steve@icarus.com)
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

static void function_argument_logic(ivl_signal_t port, ivl_expr_t expr)
{
      struct vector_info res;
      unsigned ewidth, pwidth;

	/* ports cannot be arrays. */
      assert(ivl_signal_dimensions(port) == 0);

      ewidth = ivl_expr_width(expr);
      pwidth = ivl_signal_width(port);
	/* Just like a normal assignment the function arguments need to
	 * be evaluated at either their width or the argument width if
	 * it is larger. */
      if (ewidth < pwidth) ewidth = pwidth;
      res = draw_eval_expr_wid(expr, ewidth, 0);

	/* We could have extra bits so only select the ones we need. */
      fprintf(vvp_out, "    %%set/v v%p_0, %u, %u;\n", port, res.base, pwidth);

      clr_vector(res);
}

static void function_argument_real(ivl_signal_t port, ivl_expr_t expr)
{
      int res = draw_eval_real(expr);

	/* ports cannot be arrays. */
      assert(ivl_signal_dimensions(port) == 0);

      fprintf(vvp_out, "   %%set/wr v%p_0, %d;\n", port, res);
      clr_word(res);
}

static void draw_function_argument(ivl_signal_t port, ivl_expr_t expr)
{
      ivl_variable_type_t dtype = ivl_signal_data_type(port);
      switch (dtype) {
	  case IVL_VT_LOGIC:
	    function_argument_logic(port, expr);
	    break;
	  case IVL_VT_REAL:
	    function_argument_real(port, expr);
	    break;
	  default:
	    fprintf(stderr, "XXXX function argument %s type=%d?!\n",
		    ivl_signal_basename(port), dtype);
	    assert(0);
      }
}

/*
 * A call to a user defined function generates a result that is the
 * result of this expression.
 *
 * The result of the function is placed by the function execution into
 * a signal within the scope of the function that also has a basename
 * the same as the function. The ivl_target API handled the result
 * mapping already, and we get the name of the result signal as
 * parameter 0 of the function definition.
 */

struct vector_info draw_ufunc_expr(ivl_expr_t expr, unsigned wid)
{
      unsigned idx;
      unsigned swid = ivl_expr_width(expr);
      ivl_scope_t def = ivl_expr_def(expr);
      ivl_signal_t retval = ivl_scope_port(def, 0);
      struct vector_info res;
      unsigned load_wid;

        /* If this is an automatic function, allocate the local storage. */
      if (ivl_scope_is_auto(def)) {
            fprintf(vvp_out, "    %%alloc S_%p;\n", def);
      }

	/* evaluate the expressions and send the results to the
	   function ports. */

      assert(ivl_expr_parms(expr) == (ivl_scope_ports(def)-1));
      for (idx = 0 ;  idx < ivl_expr_parms(expr) ;  idx += 1) {
	    ivl_signal_t port = ivl_scope_port(def, idx+1);
	    draw_function_argument(port, ivl_expr_parm(expr, idx));
      }


	/* Call the function */
      fprintf(vvp_out, "    %%fork TD_%s", vvp_mangle_id(ivl_scope_name(def)));
      fprintf(vvp_out, ", S_%p;\n", def);
      fprintf(vvp_out, "    %%join;\n");

	/* Fresh basic block starts after the join. */
      clear_expression_lookaside();

	/* The return value is in a signal that has the name of the
	   expression. Load that into the thread and return the
	   vector result. */

      res.base = allocate_vector(wid);
      res.wid  = wid;
      if (res.base == 0) {
	    fprintf(stderr, "%s:%u: vvp.tgt error: "
		    "Unable to allocate %u thread bits for function result.\n",
		    ivl_expr_file(expr), ivl_expr_lineno(expr), wid);
	    vvp_errors += 1;
	    return res;
      }

      assert(res.base != 0);

      load_wid = swid;
      if (load_wid > ivl_signal_width(retval))
	    load_wid = ivl_signal_width(retval);

      assert(ivl_signal_dimensions(retval) == 0);
      fprintf(vvp_out, "    %%load/v  %u, v%p_0, %u;\n",
	      res.base, retval, load_wid);

	/* Pad the signal value with zeros. */
      if (load_wid < wid)
	    pad_expr_in_place(expr, res, swid);

        /* If this is an automatic function, free the local storage. */
      if (ivl_scope_is_auto(def)) {
            fprintf(vvp_out, "    %%free S_%p;\n", def);
      }

      return res;
}

int draw_ufunc_real(ivl_expr_t expr)
{
      ivl_scope_t def = ivl_expr_def(expr);
      ivl_signal_t retval = ivl_scope_port(def, 0);
      int res = 0;
      unsigned idx;

        /* If this is an automatic function, allocate the local storage. */
      if (ivl_scope_is_auto(def)) {
            fprintf(vvp_out, "    %%alloc S_%p;\n", def);
      }

      assert(ivl_expr_parms(expr) == (ivl_scope_ports(def)-1));
      for (idx = 0 ;  idx < ivl_expr_parms(expr) ;  idx += 1) {
	    ivl_signal_t port = ivl_scope_port(def, idx+1);
	    draw_function_argument(port, ivl_expr_parm(expr, idx));
      }


	/* Call the function */
      fprintf(vvp_out, "   %%fork TD_%s", vvp_mangle_id(ivl_scope_name(def)));
      fprintf(vvp_out, ", S_%p;\n", def);
      fprintf(vvp_out, "   %%join;\n");

	/* Return value signal cannot be an array. */
      assert(ivl_signal_dimensions(retval) == 0);

	/* Load the result into a word. */
      res = allocate_word();
      fprintf(vvp_out, "  %%load/wr %d, v%p_0;\n", res, retval);

        /* If this is an automatic function, free the local storage. */
      if (ivl_scope_is_auto(def)) {
            fprintf(vvp_out, "    %%free S_%p;\n", def);
      }

      return res;
}
