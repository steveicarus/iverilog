/*
 * Copyright (c) 2012-2020 Stephen Williams (steve@icarus.com)
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

void darray_new(ivl_type_t element_type, unsigned size_reg)
{
      int wid;
      char*signed_char;
      ivl_variable_type_t type = ivl_type_base(element_type);

      if ((type == IVL_VT_BOOL) || (type == IVL_VT_LOGIC)) {
	    wid = ivl_type_packed_width(element_type);
	    signed_char = ivl_type_signed(element_type) ? "s" : "";
      } else {
	      // REAL or STRING objects are not packable.
	    assert(ivl_type_packed_dimensions(element_type) == 0);
	    wid = 0;
	    signed_char = "";
      }

      switch (type) {
	  case IVL_VT_REAL:
	    fprintf(vvp_out, "    %%new/darray %u, \"r\";\n",
	                     size_reg);
	    break;

	  case IVL_VT_STRING:
	    fprintf(vvp_out, "    %%new/darray %u, \"S\";\n",
	                     size_reg);
	    break;

	  case IVL_VT_BOOL:
	    fprintf(vvp_out, "    %%new/darray %u, \"%sb%d\";\n",
	                     size_reg, signed_char, wid);
	    break;

	  case IVL_VT_LOGIC:
	    fprintf(vvp_out, "    %%new/darray %u, \"%sv%d\";\n",
	                     size_reg, signed_char, wid);
	    break;

	  default:
	    assert(0);
	    break;
      }

      clr_word(size_reg);
}

static int eval_darray_new(ivl_expr_t ex)
{
      int errors = 0;
      unsigned size_reg = allocate_word();
      ivl_expr_t size_expr = ivl_expr_oper1(ex);
      ivl_expr_t init_expr = ivl_expr_oper2(ex);
      draw_eval_expr_into_integer(size_expr, size_reg);

	// The new function has a net_type that contains the details
	// of the type.
      ivl_type_t net_type = ivl_expr_net_type(ex);
      assert(net_type);

      ivl_type_t element_type = ivl_type_element(net_type);
      assert(element_type);

      darray_new(element_type, size_reg);

      if (init_expr && ivl_expr_type(init_expr)==IVL_EX_ARRAY_PATTERN) {
	    unsigned idx;
	    switch (ivl_type_base(element_type)) {
		case IVL_VT_BOOL:
		case IVL_VT_LOGIC:
		  for (idx = 0 ; idx < ivl_expr_parms(init_expr) ; idx += 1) {
			draw_eval_vec4(ivl_expr_parm(init_expr,idx));
			fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/vec4 3;\n");
			fprintf(vvp_out, "    %%pop/vec4 1;\n");
		  }
		  break;
		case IVL_VT_REAL:
		  for (idx = 0 ; idx < ivl_expr_parms(init_expr) ; idx += 1) {
			draw_eval_real(ivl_expr_parm(init_expr,idx));
			fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/real 3;\n");
			fprintf(vvp_out, "    %%pop/real 1;\n");
		  }
		  break;
		case IVL_VT_STRING:
		  for (idx = 0 ; idx < ivl_expr_parms(init_expr) ; idx += 1) {
			draw_eval_string(ivl_expr_parm(init_expr,idx));
			fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/str 3;\n");
			fprintf(vvp_out, "    %%pop/str 1;\n");
		  }
		  break;
		default:
		  fprintf(vvp_out, "; ERROR: Sorry, this type not supported here.\n");
		  errors += 1;
		  break;
	    }
      } else if (init_expr && (ivl_expr_value(init_expr) == IVL_VT_DARRAY)) {
		  ivl_signal_t sig = ivl_expr_signal(init_expr);
		  fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
		  fprintf(vvp_out, "    %%scopy;\n");

      } else if (init_expr && number_is_immediate(size_expr,32,0)) {
	      /* In this case, there is an init expression, the
		 expression is NOT an array_pattern, and the size
		 expression used to calculate the size of the array is
		 a constant. Generate an unrolled set of assignments. */
	    long idx;
	    long cnt = get_number_immediate(size_expr);
	    unsigned wid;
	    switch (ivl_type_base(element_type)) {
		case IVL_VT_BOOL:
		case IVL_VT_LOGIC:
		  wid = ivl_type_packed_width(element_type);
		  for (idx = 0 ; idx < cnt ; idx += 1) {
			draw_eval_vec4(init_expr);
			fprintf(vvp_out, "    %%parti/%c %u, %ld, 6;\n",
                                ivl_expr_signed(init_expr) ? 's' : 'u', wid, idx * wid);
			fprintf(vvp_out, "    %%ix/load 3, %ld, 0;\n", cnt - idx - 1);
			fprintf(vvp_out, "    %%set/dar/obj/vec4 3;\n");
			fprintf(vvp_out, "    %%pop/vec4 1;\n");
		  }
		  break;
		case IVL_VT_REAL:
		  draw_eval_real(init_expr);
		  for (idx = 0 ; idx < cnt ; idx += 1) {
			fprintf(vvp_out, "    %%ix/load 3, %ld, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/real 3;\n");
		  }
		  fprintf(vvp_out, "    %%pop/real 1;\n");
		  break;
		case IVL_VT_STRING:
		  draw_eval_string(init_expr);
		  for (idx = 0 ; idx < cnt ; idx += 1) {
			fprintf(vvp_out, "    %%ix/load 3, %ld, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/str 3;\n");
		  }
		  fprintf(vvp_out, "    %%pop/str 1;\n");
		  break;
		default:
		  fprintf(vvp_out, "; ERROR: Sorry, this type not supported here.\n");
		  errors += 1;
		  break;
	    }

      } else if (init_expr) {
	    fprintf(vvp_out, "; ERROR: Sorry, I don't know how to work with this size expr.\n");
	    errors += 1;
      }

      return errors;
}

static int eval_class_new(ivl_expr_t ex)
{
      ivl_type_t class_type = ivl_expr_net_type(ex);
      fprintf(vvp_out, "    %%new/cobj C%p;\n", class_type);
      return 0;
}

static int eval_object_null(ivl_expr_t ex)
{
      (void)ex; /* Parameter is not used. */
      fprintf(vvp_out, "    %%null;\n");
      return 0;
}

static int eval_object_property(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      unsigned pidx = ivl_expr_property_idx(expr);

      int idx = 0;
      ivl_expr_t idx_expr = 0;

	/* If there is an array index expression, then this is an
	   array'ed property, and we need to calculate the index for
	   the expression. */
      if ( (idx_expr = ivl_expr_oper1(expr)) ) {
	    idx = allocate_word();
	    draw_eval_expr_into_integer(idx_expr, idx);
      }

      fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
      fprintf(vvp_out, "    %%prop/obj %u, %d; eval_object_property\n", pidx, idx);
      fprintf(vvp_out, "    %%pop/obj 1, 1;\n");

      if (idx != 0) clr_word(idx);
      return 0;
}

static int eval_object_shallowcopy(ivl_expr_t ex)
{
      int errors = 0;
      ivl_expr_t dest = ivl_expr_oper1(ex);
      ivl_expr_t src  = ivl_expr_oper2(ex);

      errors += draw_eval_object(dest);
      errors += draw_eval_object(src);

	/* The %scopy opcode pops the top of the object stack as the
	   source object, and shallow-copies it to the new top, the
	   destination object. The destination is left on the top of
	   the stack. */
      fprintf(vvp_out, "    %%scopy;\n");

      return errors;
}

static int eval_object_signal(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);

	/* Simple case: This is a simple variable. Generate a load
	   statement to load the string into the stack. */
      if (ivl_signal_dimensions(sig) == 0) {
	    fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
	    return 0;
      }

	/* There is a word select expression, so load the index into a
	   register and load from the array. */
      ivl_expr_t word_ex = ivl_expr_oper1(expr);
      int word_ix = allocate_word();
      draw_eval_expr_into_integer(word_ex, word_ix);
      fprintf(vvp_out, "    %%load/obja v%p, %d;\n", sig, word_ix);
      clr_word(word_ix);

      return 0;
}

static int eval_object_ufunc(ivl_expr_t ex)
{
      draw_ufunc_object(ex);
      return 0;
}

int draw_eval_object(ivl_expr_t ex)
{
      switch (ivl_expr_type(ex)) {

	  case IVL_EX_NEW:
	    switch (ivl_expr_value(ex)) {
		case IVL_VT_CLASS:
		  return eval_class_new(ex);
		case IVL_VT_DARRAY:
		  return eval_darray_new(ex);
		default:
		  fprintf(vvp_out, "; ERROR: draw_eval_object: Invalid type (%d) for <new>\n",
			  ivl_expr_value(ex));
		  return 0;
	    }

	  case IVL_EX_NULL:
	    return eval_object_null(ex);

	  case IVL_EX_PROPERTY:
	    return eval_object_property(ex);

	  case IVL_EX_SHALLOWCOPY:
	    return eval_object_shallowcopy(ex);

	  case IVL_EX_SIGNAL:
	    return eval_object_signal(ex);

	  case IVL_EX_UFUNC:
	    return eval_object_ufunc(ex);

	  default:
	    fprintf(vvp_out, "; ERROR: draw_eval_object: Invalid expression type %d\n", ivl_expr_type(ex));
	    return 1;

      }
}
