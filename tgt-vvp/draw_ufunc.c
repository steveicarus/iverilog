/*
 * Copyright (c) 2005-2026 Stephen Williams (steve@icarus.com)
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
# include  <stdlib.h>
# include  <assert.h>

static void function_argument_logic(ivl_signal_t port, ivl_expr_t expr)
{
      unsigned ewidth, pwidth;

	/* An unpacked array port is copied straight from the source signal in
	   the send pass, so there is nothing to evaluate onto the stack. */
      if (ivl_signal_dimensions(port) > 0)
	    return;

      ewidth = ivl_expr_width(expr);
      pwidth = ivl_signal_width(port);

      draw_eval_vec4(expr);
      if (ewidth < pwidth)
	    fprintf(vvp_out, "    %%pad/u %u;\n", pwidth);

}

static void function_argument_real(ivl_signal_t port, ivl_expr_t expr)
{
	/* See function_argument_logic: array ports are handled in the send
	   pass. */
      if (ivl_signal_dimensions(port) > 0)
	    return;

      draw_eval_real(expr);
}

static void draw_eval_function_argument(ivl_signal_t port, ivl_expr_t expr)
{
      ivl_variable_type_t dtype = ivl_signal_data_type(port);
      switch (dtype) {
	  case IVL_VT_BOOL:
	      /* For now, treat bit2 variables as bit4 variables. */
	  case IVL_VT_LOGIC:
	    function_argument_logic(port, expr);
	    break;
	  case IVL_VT_REAL:
	    function_argument_real(port, expr);
	    break;
	  case IVL_VT_CLASS:
	    vvp_errors += draw_eval_object(expr);
	    break;
	  case IVL_VT_STRING:
	    draw_eval_string(expr);
	    break;
	  case IVL_VT_DARRAY:
	  case IVL_VT_QUEUE:
	      /* Pass by value: the function gets its own copy. */
	    vvp_errors += draw_eval_object(expr);
	    fprintf(vvp_out, "    %%dup/obj;\n");
	    fprintf(vvp_out, "    %%pop/obj 1, 1;\n");
	    break;
	  default:
	    fprintf(stderr, "XXXX function argument %s type=%d?!\n",
		    ivl_signal_basename(port), dtype);
	    assert(0);
      }
}

static void draw_send_function_argument(ivl_signal_t port, ivl_expr_t expr)
{
      ivl_variable_type_t dtype = ivl_signal_data_type(port);

	/* An unpacked array argument is a memory-to-memory copy rather than a
	   value on the stack, so it is done here in the send pass, reading the
	   source signal directly. */
      if (ivl_signal_dimensions(port) > 0) {
	    vvp_errors += draw_array_copy(port, expr);
	    return;
      }

      switch (dtype) {
	  case IVL_VT_BOOL:
	      /* For now, treat bit2 variables as bit4 variables. */
	  case IVL_VT_LOGIC:
	    fprintf(vvp_out, "    %%store/vec4 v%p_0, 0, %u;\n",
				      port, ivl_signal_width(port));
	    break;
	  case IVL_VT_REAL:
	    fprintf(vvp_out, "    %%store/real v%p_0;\n", port);
	    break;
	  case IVL_VT_CLASS:
	    fprintf(vvp_out, "    %%store/obj v%p_0;\n", port);
	    break;
	  case IVL_VT_STRING:
	    fprintf(vvp_out, "    %%store/str v%p_0;\n", port);
	    break;
	  case IVL_VT_DARRAY:
	  case IVL_VT_QUEUE:
	    fprintf(vvp_out, "    %%store/obj v%p_0;\n", port);
	    break;
	  default:
	    fprintf(stderr, "XXXX function argument %s type=%d?!\n",
		    ivl_signal_basename(port), dtype);
	    assert(0);
      }
}

static void draw_ufunc_preamble(ivl_expr_t expr)
{
      ivl_scope_t def = ivl_expr_def(expr);
      unsigned idx;

        /* If this is an automatic function, allocate the local storage. */
      if (ivl_scope_is_auto(def)) {
            fprintf(vvp_out, "    %%alloc S_%p;\n", def);
      }

	/* Evaluate the expressions and send the results to the
	   function ports. Do this in two passes - evaluate,
	   then send - this avoids the function input variables
	   being overwritten if the same (non-automatic) function
	   is called in one of the expressions. */

      assert(ivl_expr_parms(expr) == (ivl_scope_ports(def)-1));
      for (idx = 0 ;  idx < ivl_expr_parms(expr) ;  idx += 1) {
	    ivl_signal_t port = ivl_scope_port(def, idx+1);
	    draw_eval_function_argument(port, ivl_expr_parm(expr, idx));
      }
      for (idx = ivl_expr_parms(expr) ;  idx > 0 ;  idx -= 1) {
	    ivl_signal_t port = ivl_scope_port(def, idx);
	    draw_send_function_argument(port, ivl_expr_parm(expr, idx-1));
      }

	/* Call the function. A non-virtual call of a virtual function
	   skips the dispatch to an override at the start of it. */
      const char*entry = (ivl_expr_nonvirtual(expr)
			  && ivl_scope_virtual_root(def) == def) ? "TB" : "TD";
      switch (ivl_expr_value(expr)) {
	  case IVL_VT_VOID:
	    fprintf(vvp_out, "    %%callf/void %s_%p", entry, def);
	    fprintf(vvp_out, ", S_%p; %s\n", def,
		    ivl_scope_name(def));
	    break;
	  case IVL_VT_REAL:
	    fprintf(vvp_out, "    %%callf/real %s_%p", entry, def);
	    fprintf(vvp_out, ", S_%p; %s\n", def,
		    ivl_scope_name(def));
	    break;
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    fprintf(vvp_out, "    %%callf/vec4 %s_%p", entry, def);
	    fprintf(vvp_out, ", S_%p; %s\n", def,
		    ivl_scope_name(def));
	    break;
	  case IVL_VT_STRING:
	    fprintf(vvp_out, "    %%callf/str %s_%p", entry, def);
	    fprintf(vvp_out, ", S_%p; %s\n", def,
		    ivl_scope_name(def));
	    break;
	  case IVL_VT_CLASS:
	  case IVL_VT_DARRAY:
	  case IVL_VT_QUEUE:
	    fprintf(vvp_out, "    %%callf/obj %s_%p", entry, def);
	    fprintf(vvp_out, ", S_%p; %s\n", def,
		    ivl_scope_name(def));
	    break;
	  default:
	    fprintf(vvp_out, "    %%fork TD_%p", def);
	    fprintf(vvp_out, ", S_%p; %s\n", def,
		    ivl_scope_name(def));
	    fprintf(vvp_out, "    %%join;\n");
	    break;
      }
}

static void draw_ufunc_epilogue(ivl_expr_t expr)
{
      ivl_scope_t def = ivl_expr_def(expr);

        /* If this is an automatic function, free the local storage. */
      if (ivl_scope_is_auto(def)) {
            fprintf(vvp_out, "    %%free S_%p;\n", def);
      }
}

/*
 * The call instruction leaves vector, real and string results on their
 * respective evaluation stacks.
 */
void draw_ufunc(ivl_expr_t expr)
{
      draw_ufunc_preamble(expr);
      draw_ufunc_epilogue(expr);
}

void draw_ufunc_object(ivl_expr_t expr)
{
      ivl_scope_t def = ivl_expr_def(expr);
      ivl_signal_t retval = ivl_scope_port(def, 0);

	/* Take in arguments to function and call the function code. */
      draw_ufunc_preamble(expr);

	/* Load the result into the object stack. */
      fprintf(vvp_out, "    %%load/obj v%p_0;\n", retval);

      draw_ufunc_epilogue(expr);
}

/*
 * Copy the value of a port of one method to the port of another.
 */
static void draw_virtual_copy(ivl_signal_t from, ivl_signal_t to)
{
      switch (ivl_signal_data_type(to)) {
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    fprintf(vvp_out, "    %%load/vec4 v%p_0;\n", from);
	    fprintf(vvp_out, "    %%store/vec4 v%p_0, 0, %u;\n", to,
		    ivl_signal_width(to));
	    break;
	  case IVL_VT_REAL:
	    fprintf(vvp_out, "    %%load/real v%p_0;\n", from);
	    fprintf(vvp_out, "    %%store/real v%p_0;\n", to);
	    break;
	  case IVL_VT_STRING:
	    fprintf(vvp_out, "    %%load/str v%p_0;\n", from);
	    fprintf(vvp_out, "    %%store/str v%p_0;\n", to);
	    break;
	  default:
	    fprintf(vvp_out, "    %%load/obj v%p_0;\n", from);
	    fprintf(vvp_out, "    %%store/obj v%p_0;\n", to);
	    break;
      }
}

/*
 * Draw the code that a call of the virtual method root runs for an
 * object whose class overrides it with impl. The caller has put the
 * arguments in the variables of root and started its code, which jumped
 * here. Pass the arguments on to impl, call it and return its results as
 * the results of root. The variables of impl are read after the call
 * with the context of impl as the read context, see do_join().
 */
int draw_virtual_adapter(ivl_scope_t root, ivl_scope_t impl)
{
      unsigned idx;
      int is_func = ivl_scope_type(impl) == IVL_SCT_FUNCTION;
	/* The ports of a function start with the return value. */
      unsigned first = is_func ? 1 : 0;

      fprintf(vvp_out, "TA_%p_%p ; %s -> %s\n", root, impl,
	      ivl_scope_name(root), ivl_scope_name(impl));
      fprintf(vvp_out, "    %%alloc S_%p;\n", impl);

      assert(ivl_scope_ports(root) == ivl_scope_ports(impl));
      for (idx = first ; idx < ivl_scope_ports(root) ; idx += 1) {
	    ivl_signal_t from = ivl_scope_port(root, idx);
	    ivl_signal_t to = ivl_scope_port(impl, idx);
	    if (ivl_signal_port(to) != IVL_SIP_OUTPUT)
		  draw_virtual_copy(from, to);
      }

      if (!is_func) {
	    fprintf(vvp_out, "    %%fork TD_%p, S_%p;\n", impl, impl);
	    fprintf(vvp_out, "    %%join;\n");
	    for (idx = first ; idx < ivl_scope_ports(root) ; idx += 1) {
		  ivl_signal_t from = ivl_scope_port(impl, idx);
		  ivl_signal_t to = ivl_scope_port(root, idx);
		  if (ivl_signal_port(to) != IVL_SIP_INPUT)
			draw_virtual_copy(from, to);
	    }
      } else {
	    ivl_signal_t rret = ivl_scope_port(root, 0);
	    ivl_signal_t iret = ivl_scope_port(impl, 0);

	    switch (ivl_scope_func_type(impl)) {
		case IVL_VT_VOID:
		  fprintf(vvp_out, "    %%callf/void TD_%p, S_%p;\n", impl, impl);
		  break;
		case IVL_VT_REAL:
		  fprintf(vvp_out, "    %%callf/real TD_%p, S_%p;\n", impl, impl);
		  fprintf(vvp_out, "    %%ret/real 0;\n");
		  break;
		case IVL_VT_BOOL:
		case IVL_VT_LOGIC:
		  fprintf(vvp_out, "    %%callf/vec4 TD_%p, S_%p;\n", impl, impl);
		  fprintf(vvp_out, "    %%ret/vec4 0, 0, %u;\n",
			  ivl_scope_func_width(impl));
		  break;
		case IVL_VT_STRING:
		  fprintf(vvp_out, "    %%callf/str TD_%p, S_%p;\n", impl, impl);
		  fprintf(vvp_out, "    %%ret/str 0;\n");
		  break;
		default:
		  fprintf(vvp_out, "    %%callf/obj TD_%p, S_%p;\n", impl, impl);
		  draw_virtual_copy(iret, rret);
		  break;
	    }
      }

      fprintf(vvp_out, "    %%free S_%p;\n", impl);
      fprintf(vvp_out, "    %%end;\n");

      return 0;
}
