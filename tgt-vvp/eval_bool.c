/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: eval_bool.c,v 1.1 2005/09/14 02:53:15 steve Exp $"

/*
 * This file includes functions for evaluating REAL expressions.
 */
# include  "config.h"
# include  "vvp_priv.h"
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <stdint.h>
# include  <math.h>
# include  <assert.h>

/*
 * Evaluate the bool64 the hard way, by evaluating the logic vector
 * and converting it to a bool64.
 */
static int eval_bool64_logic(ivl_expr_t exp)
{
      int res;
      struct vector_info tmp;

      tmp = draw_eval_expr(exp, STUFF_OK_XZ);
      res = allocate_word();
      fprintf(vvp_out, "   %%ix/get %d, %u, %u;\n", res, tmp.base, tmp.wid);
      clr_vector(tmp);

      return res;
}

static int draw_number_bool64(ivl_expr_t exp)
{
      int res;
      const char*bits = ivl_expr_bits(exp);
      uint64_t val = 0;
      unsigned long idx;

      for (idx = 0 ;  idx < ivl_expr_width(exp) ;  idx += 1) {
	    if (bits[idx] == '1')
		  val |= 1UL << idx;
      }

      res = allocate_word();
      fprintf(vvp_out, "   %%ix/load %d, %lu;\n", res, val);
      return res;
}

int draw_eval_bool64(ivl_expr_t exp)
{
      int res;

      switch (ivl_expr_type(exp)) {
	  case IVL_EX_NUMBER:
	    res = draw_number_bool64(exp);
	    break;
	  default:
	    res = eval_bool64_logic(exp);
	    break;
      }

      return res;
}

/*
 * $Log: eval_bool.c,v $
 * Revision 1.1  2005/09/14 02:53:15  steve
 *  Support bool expressions and compares handle them optimally.
 *
 */

