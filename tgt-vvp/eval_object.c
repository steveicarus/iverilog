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
# include  <string.h>
# include  <assert.h>

static int eval_darray_new(ivl_expr_t ex)
{
      unsigned size_reg = allocate_word();
      ivl_expr_t size_expr = ivl_expr_parm(ex, 0);
      draw_eval_expr_into_integer(size_expr, size_reg);
      clr_word(size_reg);

	// XXXX: Assume elements are 32bit integers.
      fprintf(vvp_out, "    %%new/darray %u, \"sb32\";\n", size_reg);

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
