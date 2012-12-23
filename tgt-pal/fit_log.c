/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "ivl_target.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <assert.h>
# include  "priv.h"
# include  "ivl_alloc.h"

/*
 * By the time we get here, all the flip-flops have been placed in
 * macrocells, and enables attached to them. So all that's left is to
 * work backwards from each macrocell, making terms and sum-of-terms
 * from the asynchronous logic until we get to the table inputs.
 *
 * A product term is a null terminated array of ivl_nexus_t
 * objects. An expression is a null terminated array of terms.
 */

static void dump_expr(term_t**expr, const char*title)
{
      unsigned idx;

      fprintf(stderr, "expression for %s:\n", title);
      for (idx = 0 ;  expr[idx] ;  idx += 1) {
	    term_t*tp;
	    fprintf(stderr, "   term %u:\n", idx);
	    for (tp = expr[idx] ;  tp->nex ;  tp += 1)
		  fprintf(stderr, "     %c%s\n", tp->inv? '~' : ' ',
			  ivl_nexus_name(tp->nex));
      }
}

static term_t** build_expr(ivl_nexus_t nex)
{
      term_t* term = calloc(2, sizeof(term_t));
      term_t**expr = calloc(2, sizeof(term_t*));
      unsigned idx;

      assert(nex);
      expr[0] = term;
      term[0].inv = 0;
      term[0].nex = nex;

	/* First look to see if I'm connected to an input pin. If so,
	   then this expression is done. */
      for (idx = 0 ;  idx < pins ;  idx += 1) {
	    struct pal_bind_s*pin = bind_pin + idx;
	    if ((nex == pin->nexus) && (pin->sop == 0)) {
		  return expr;
	    }
      }

      fprintf(stderr, "sorry, I give up on nexus %s\n", ivl_nexus_name(nex));
      return 0;
}

int fit_logic(void)
{
      unsigned idx;

      for (idx = 0 ;  idx < pins ;  idx += 1) {
	    ivl_nexus_t cell;

	    struct pal_bind_s*pin = bind_pin + idx;
	    if (pin->sop == 0)
		  continue;

	    cell = pin->nexus;
	    if (cell == 0)
		  continue;


	      /* If there is an enable, then this is a bufifX or a
		 notifX. Build the expression for the enable, and
		 guess that the input to the cell is actually the
		 input to the enable. */
	    if (pin->enable) {
		  ivl_nexus_t en_nex = ivl_logic_pin(pin->enable, 2);
		  assert(cell == ivl_logic_pin(pin->enable, 0));

		  cell = ivl_logic_pin(pin->enable, 1);
		  assert(cell);

		  pin->enable_ex = build_expr(en_nex);
		  dump_expr(pin->enable_ex, ivl_nexus_name(en_nex));
	    }

	      /* If there is a reg, then the input to the cell is
		 really the D input to the ff. */
	    if (pin->reg) {
		  assert(cell == ivl_lpm_q(pin->reg, pin->reg_q));
		  cell = ivl_lpm_data(pin->reg, pin->reg_q);
	    }

	    assert(cell);

	      /* Here we are. Generate the sum-of-products for the
		 input. */
	    pin->sop_ex = build_expr(cell);
	    dump_expr(pin->sop_ex, ivl_nexus_name(cell));
      }

      return 0;
}
