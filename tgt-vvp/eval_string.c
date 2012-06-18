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


void draw_eval_string(ivl_expr_t expr)
{
      struct vector_info res;

      switch (ivl_expr_type(expr)) {
	  case IVL_EX_STRING:
	    fprintf(vvp_out, "    %%pushi/str \"%s\";\n", ivl_expr_string(expr));
	    break;

	  default:
	    switch (ivl_expr_value(expr)) {

		case IVL_VT_BOOL:
		case IVL_VT_LOGIC:
		  res = draw_eval_expr(expr, 0);
		  fprintf(vvp_out, "    %%pushv/str %u, %u; Cast BOOL/LOGIC to string\n",
			  res.base, res.wid);
		  if (res.base > 0)
			clr_vector(res);
		  break;

		default:
		  assert(0);
		  break;
	    }
	    break;
      }
}
