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
      unsigned size_reg = allocate_word();
      ivl_expr_t size_expr = ivl_expr_parm(ex, 0);
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

      return 0;
}

static int draw_eval_object_sfunc(ivl_expr_t ex)
{
      const char*name = ivl_expr_name(ex);

      if (strcmp(name, "$ivl_darray_method$new") == 0)
	    return eval_darray_new(ex);

      fprintf(vvp_out, "; ERROR: Invalid system function %s for darray\n", name);
      return 1;
}

int draw_eval_object(ivl_expr_t ex)
{
      switch (ivl_expr_type(ex)) {
	  case IVL_EX_SFUNC:
	    return draw_eval_object_sfunc(ex);

	  default:
	    fprintf(vvp_out, "; ERROR: Invalid expression type %u\n", ivl_expr_type(ex));
	    return 1;

      }
}
