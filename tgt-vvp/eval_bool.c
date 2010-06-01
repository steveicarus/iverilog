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

/*
 * This file includes functions for evaluating REAL expressions.
 */
# include  "vvp_config.h"
# include  "vvp_priv.h"
# include  <string.h>
# include  <stdlib.h>
#ifdef HAVE_STDINT_H
# include  <stdint.h>
#endif

#ifdef HAVE_INTTYPES_H
# ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS 1
# endif
# include  <inttypes.h>
#else
#endif

# include  <math.h>
# include  <assert.h>

/*
 * Evaluate the bool64 the hard way, by evaluating the logic vector
 * and converting it to a bool64.
 */
static int eval_bool64_logic(ivl_expr_t expr)
{
      int res;
      struct vector_info tmp;

      tmp = draw_eval_expr(expr, STUFF_OK_XZ);
      res = allocate_word();
      fprintf(vvp_out, "   %%ix/get %d, %u, %u;\n", res, tmp.base, tmp.wid);
      clr_vector(tmp);

      return res;
}

static int draw_number_bool64(ivl_expr_t expr)
{
      int res;
      const char*bits = ivl_expr_bits(expr);
      uint64_t val = 0;
      unsigned long idx, low, hig;

      for (idx = 0 ;  idx < ivl_expr_width(expr) ;  idx += 1) {
	    if (bits[idx] == '1')
		  val |= 1UL << idx;
      }

      res = allocate_word();
      low = val % UINT64_C(0x100000000);
      hig = val / UINT64_C(0x100000000);
      fprintf(vvp_out, "   %%ix/load %d, %lu, %lu;\n", res, low, hig);
      return res;
}

int draw_eval_bool64(ivl_expr_t expr)
{
      int res;

      switch (ivl_expr_type(expr)) {
	  case IVL_EX_NUMBER:
	    res = draw_number_bool64(expr);
	    break;
	  default:
	    res = eval_bool64_logic(expr);
	    break;
      }

      return res;
}
