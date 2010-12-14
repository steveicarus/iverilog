/*
 * Copyright (c) 2008-2010 Stephen Williams (steve@icarus.com)
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
# include  <stdlib.h>
# include  <string.h>

static void draw_tran_island(ivl_island_t island)
{
      fprintf(vvp_out, "I%p .island tran;\n", island);
      ivl_island_flag_set(island, 0, 1);
}

void draw_switch_in_scope(ivl_switch_t sw)
{
      ivl_island_t island;
      ivl_nexus_t nex_a, nex_b, enable;
      const char*str_a, *str_b, *str_e;

      island = ivl_switch_island(sw);
      if (ivl_island_flag_test(island, 0) == 0)
	    draw_tran_island(island);

      nex_a = ivl_switch_a(sw);
      assert(nex_a);
      str_a = draw_net_input(nex_a);

      nex_b = ivl_switch_b(sw);
      assert(nex_b);
      str_b = draw_net_input(nex_b);

      enable = ivl_switch_enable(sw);
      str_e = 0;
      if (enable)
	    str_e = draw_net_input(enable);


      switch (ivl_switch_type(sw)) {
	  case IVL_SW_TRAN:
	    fprintf(vvp_out, " .tran");
	    break;
	  case IVL_SW_TRANIF0:
	    fprintf(vvp_out, " .tranif0");
	    break;
	  case IVL_SW_TRANIF1:
	    fprintf(vvp_out, " .tranif1");
	    break;
	  case IVL_SW_TRAN_VP:
	    fprintf(vvp_out, " .tranvp %u %u %u,",
		    ivl_switch_width(sw), ivl_switch_part(sw), ivl_switch_offset(sw));
	    break;

	  default:
	    fprintf(stderr, "%s:%u: tgt-vvp sorry: resistive switch modeling "
		    "is not supported in V0.9.\n",
		    ivl_switch_file(sw), ivl_switch_lineno(sw));
	    vvp_errors += 1;
	    return;
      }

      fprintf(vvp_out, " I%p, %s %s", island, str_a, str_b);
      if (enable)
	    fprintf(vvp_out, ", %s", str_e);

      fprintf(vvp_out, ";\n");
}
