/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: draw_mux.c,v 1.11 2005/08/27 04:32:08 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>


/*
 * This draws a simple A/B mux. The mux can have any width, enough
 * MUXZ nodes are created to support the vector.
 */
static void draw_lpm_mux_ab(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);

	/* Only support A-B muxes in this function. */
      assert(ivl_lpm_size(net) == 2);
      assert(ivl_lpm_selects(net) == 1);

      fprintf(vvp_out, "L_%p .functor MUXZ %u", net, width);
      fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,0)));
      fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,1)));
      fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_select(net)));
      fprintf(vvp_out, ", C4<>;\n");
}

static void draw_lpm_mux_nest(ivl_lpm_t net)
{
      int idx, level;
      unsigned width = ivl_lpm_width(net);
      unsigned swidth = ivl_lpm_selects(net);
      char*select_input;

      assert(ivl_lpm_size(net) == (1 << swidth));

      select_input = strdup(draw_net_input(ivl_lpm_select(net)));

      fprintf(vvp_out, "L_%p/0s .part %s, 0, 1; Bit 0 of the select\n",
	      net, select_input);

      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 2) {
	    fprintf(vvp_out, "L_%p/0/%d .functor MUXZ %u",
		    net, idx/2, width);
	    fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,idx+0)));
	    fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,idx+1)));
	    fprintf(vvp_out, ", L_%p/0s, C4<>;\n", net);
      }

      for (level = 1 ;  level < swidth-1 ;  level += 1) {
	    fprintf(vvp_out, "L_%p/%ds .part %s, %d, 1;\n",
		    net, level, select_input, level);

	    for (idx = 0 ;  idx < (ivl_lpm_size(net) >> level); idx += 2) {
		  fprintf(vvp_out, "L_%p/%d/%d .functor MUXZ %u",
			  net, width, level,idx);
		  fprintf(vvp_out, ", L_%p/%d/%d", net, level-1, idx/2+0);
		  fprintf(vvp_out, ", L_%p/%d/%d", net, level-1, idx/2+1);
		  fprintf(vvp_out, ", L_%p/%ds",   net, level);
		  fprintf(vvp_out, ", C4<>;\n");
	    }

      }


      fprintf(vvp_out, "L_%p/%ds .part %s, %d, 1; Bit %d of the select\n",
	      net, swidth-1, select_input, swidth-1, swidth-1);


      fprintf(vvp_out, "L_%p .functor MUXZ %u", net, width);
      fprintf(vvp_out, ", L_%p/%d/0", net, swidth-2);
      fprintf(vvp_out, ", L_%p/%d/1", net, swidth-2);
      fprintf(vvp_out, ", L_%p/%ds",  net, swidth-1);
      fprintf(vvp_out, ", C4<>;\n");

      free(select_input);}

void draw_lpm_mux(ivl_lpm_t net)
{
      if ((ivl_lpm_size(net) == 2) && (ivl_lpm_selects(net) == 1)) {
	    draw_lpm_mux_ab(net);
	    return;
      }

	/* Here we are at the worst case, we generate a tree of MUXZ
	   devices to handle the arbitrary size. */
      draw_lpm_mux_nest(net);
}

/*
 * $Log: draw_mux.c,v $
 * Revision 1.11  2005/08/27 04:32:08  steve
 *  Handle synthesis of fully packed case statements.
 *
 * Revision 1.10  2005/06/17 03:46:52  steve
 *  Make functors know their own width.
 *
 * Revision 1.9  2005/04/06 05:29:09  steve
 *  Rework NetRamDq and IVL_LPM_RAM nodes.
 *
 * Revision 1.8  2005/02/12 22:54:29  steve
 *  Implement a-b muxes as vector devices
 *
 * Revision 1.7  2003/12/19 01:27:10  steve
 *  Fix various unsigned compare warnings.
 *
 * Revision 1.6  2003/02/25 03:40:45  steve
 *  Eliminate use of ivl_lpm_name function.
 *
 * Revision 1.5  2002/08/29 03:04:01  steve
 *  Generate x out for x select on wide muxes.
 *
 * Revision 1.4  2002/08/12 01:35:03  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 */
