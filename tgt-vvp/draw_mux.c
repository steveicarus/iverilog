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
#ident "$Id: draw_mux.c,v 1.10 2005/06/17 03:46:52 steve Exp $"
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

void draw_lpm_mux(ivl_lpm_t net)
{

      if ((ivl_lpm_size(net) == 2) && (ivl_lpm_selects(net) == 1)) {
	    draw_lpm_mux_ab(net);
	    return;
      }
# if 0
      for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 1)
	    draw_lpm_mux_bitslice(net, idx);
# else
      fprintf(stderr, "XXXX Forgot how to implement wide muxes.\n");
#endif

}

/*
 * $Log: draw_mux.c,v $
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
