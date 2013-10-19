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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "vvp_priv.h"
# include  <string.h>
# include  <assert.h>

static int eval_darray_new(ivl_expr_t ex)
{
      int errors = 0;
      unsigned size_reg = allocate_word();
      ivl_expr_t size_expr = ivl_expr_oper1(ex);
      ivl_expr_t init_expr = ivl_expr_oper2(ex);
      draw_eval_expr_into_integer(size_expr, size_reg);
      clr_word(size_reg);

	// The new function has a net_type that contains the details
	// of the type.
      ivl_type_t net_type = ivl_expr_net_type(ex);
      assert(net_type);

      ivl_type_t element_type = ivl_type_element(net_type);
      assert(element_type);

      switch (ivl_type_base(element_type)) {
	  case IVL_VT_REAL:
	      // REAL objects are not packable.
	    assert(ivl_type_packed_dimensions(element_type) == 0);
	    fprintf(vvp_out, "    %%new/darray %u, \"r\";\n", size_reg);
	    break;
	  case IVL_VT_STRING:
	      // STRING objects are not packable.
	    assert(ivl_type_packed_dimensions(element_type) == 0);
	    fprintf(vvp_out, "    %%new/darray %u, \"S\";\n", size_reg);
	    break;
	  case IVL_VT_BOOL:
	      // bool objects are vectorable, but for now only support
	      // a single dimensions.
	    assert(ivl_type_packed_dimensions(element_type) == 1);
	    int msb = ivl_type_packed_msb(element_type, 0);
	    int lsb = ivl_type_packed_lsb(element_type, 0);
	    int wid = msb>=lsb? msb - lsb : lsb - msb;
	    wid += 1;

	    fprintf(vvp_out, "    %%new/darray %u, \"sb%d\";\n", size_reg, wid);
	    break;

	  default:
	    assert(0);
	    break;
      }

      if (init_expr && ivl_expr_type(init_expr)==IVL_EX_ARRAY_PATTERN) {
	    int idx;
	    struct vector_info rvec;
	    unsigned wid;
	    switch (ivl_type_base(element_type)) {
		case IVL_VT_BOOL:
		  wid = width_of_packed_type(element_type);
		  for (idx = 0 ; idx < ivl_expr_parms(init_expr) ; idx += 1) {
			rvec = draw_eval_expr_wid(ivl_expr_parm(init_expr,idx),
						    wid, STUFF_OK_XZ);
			fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj 3, %u, %u;\n",
				rvec.base, rvec.wid);
			if (rvec.base >= 4) clr_vector(rvec);
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
		default:
		  fprintf(vvp_out, "; ERROR: Sorry, this type not supported here.\n");
		  errors += 1;
		  break;
	    }
      } else if (init_expr && number_is_immediate(size_expr,32,0)) {
	      /* In this case, there is an init expression, the
		 expression is NOT an array_pattern, and the size
		 expression used to calculate the size of the array is
		 a constant. Generate an unrolled set of assignments. */
	    long idx;
	    long cnt = get_number_immediate(size_expr);
	    struct vector_info rvec;
	    unsigned wid;
	    switch (ivl_type_base(element_type)) {
		case IVL_VT_BOOL:
		  wid = width_of_packed_type(element_type);
		  rvec = draw_eval_expr_wid(init_expr, wid, STUFF_OK_XZ);
		  for (idx = 0 ; idx < cnt ; idx += 1) {
			fprintf(vvp_out, "    %%ix/load 3, %ld, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj 3, %u, %u;\n",
				rvec.base, rvec.wid);
		  }
		  if (rvec.base >= 4) clr_vector(rvec);
		  break;
		case IVL_VT_REAL:
		  draw_eval_real(init_expr);
		  for (idx = 0 ; idx < cnt ; idx += 1) {
			fprintf(vvp_out, "    %%ix/load 3, %ld, 0;\n", idx);
			fprintf(vvp_out, "    %%set/dar/obj/real 3;\n");
		  }
		  fprintf(vvp_out, "    %%pop/real 1;\n");
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
      fprintf(vvp_out, "    %%null;\n");
      return 0;
}

static int eval_object_property(ivl_expr_t expr)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      unsigned pidx = ivl_expr_property_idx(expr);

      fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
      fprintf(vvp_out, "    %%prop/obj %u;\n", pidx);
      return 0;
}

static int eval_object_shallowcopy(ivl_expr_t ex)
{
      ivl_expr_t dest = ivl_expr_oper1(ex);
      ivl_expr_t src  = ivl_expr_oper2(ex);

      draw_eval_object(dest);
      draw_eval_object(src);

	/* The %scopy opcode pops the top of the object stack as the
	   source object, and shallow-copies it to the new top, the
	   destination object. The destination is left on the top of
	   the stack. */
      fprintf(vvp_out, "    %%scopy;\n");

      return 0;
}

static int eval_object_signal(ivl_expr_t ex)
{
      ivl_signal_t sig = ivl_expr_signal(ex);
      fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
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
	    fprintf(vvp_out, "; ERROR: draw_eval_object: Invalid expression type %u\n", ivl_expr_type(ex));
	    return 1;

      }
}
