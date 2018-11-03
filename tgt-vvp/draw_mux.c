/*
 * Copyright (c) 2002-2016 Stephen Williams (steve@icarus.com)
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
# include  <assert.h>
# include  <stdlib.h>
# include  <string.h>


/*
 * This draws a simple A/B mux. The mux can have any width, enough
 * MUXZ nodes are created to support the vector.
 */
static void draw_lpm_mux_ab(ivl_lpm_t net, const char*muxz)
{
      unsigned width = ivl_lpm_width(net);
      ivl_expr_t d_rise, d_fall, d_decay;
      const char*dly;
      const char* input[3];

	/* Only support A-B muxes in this function. */
      assert(ivl_lpm_size(net) == 2);
      assert(ivl_lpm_selects(net) == 1);

      d_rise = ivl_lpm_delay(net, 0);
      d_fall = ivl_lpm_delay(net, 1);
      d_decay = ivl_lpm_delay(net, 2);

      ivl_drive_t str0 = ivl_lpm_drive0(net);
      ivl_drive_t str1 = ivl_lpm_drive1(net);

      dly = "";
      if (d_rise != 0) {
            unsigned dly_width = width;
            if (data_type_of_nexus(ivl_lpm_q(net)) == IVL_VT_REAL)
                  dly_width = 0;

	    draw_delay(net, dly_width, 0, d_rise, d_fall, d_decay);
	    dly = "/d";
      }

      input[0] = draw_net_input(ivl_lpm_data(net,0));
      input[1] = draw_net_input(ivl_lpm_data(net,1));
      input[2] = draw_net_input(ivl_lpm_select(net));
      fprintf(vvp_out, "L_%p%s .functor %s %u", net, dly, muxz, width);
      if (str0!=IVL_DR_STRONG || str1!=IVL_DR_STRONG)
	    fprintf(vvp_out, " [%d %d]", str0, str1);
      fprintf(vvp_out, ", %s", input[0]);
      fprintf(vvp_out, ", %s", input[1]);
      fprintf(vvp_out, ", %s", input[2]);
      fprintf(vvp_out, ", C4<>;\n");
}

static void draw_lpm_mux_nest(ivl_lpm_t net, const char*muxz)
{
      unsigned idx, level;
      unsigned width = ivl_lpm_width(net);
      unsigned swidth = ivl_lpm_selects(net);
      char*select_input;

      assert(swidth < 8*sizeof(unsigned));
      assert(ivl_lpm_size(net) == (1U << swidth));

      select_input = strdup(draw_net_input(ivl_lpm_select(net)));

      fprintf(vvp_out, "L_%p/0s .part %s, 0, 1; Bit 0 of the select\n",
	      net, select_input);

      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 2) {
	    fprintf(vvp_out, "L_%p/0/%u .functor %s %u",
		    net, idx/2, muxz, width);
	    fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,idx+0)));
	    fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,idx+1)));
	    fprintf(vvp_out, ", L_%p/0s, C4<>;\n", net);
      }

      for (level = 1 ;  level < swidth-1 ;  level += 1) {
	    fprintf(vvp_out, "L_%p/%us .part %s, %u, 1; Bit %u of the select\n",
		    net, level, select_input, level, level);

	    for (idx = 0 ;  idx < (ivl_lpm_size(net) >> level); idx += 2) {
		  fprintf(vvp_out, "L_%p/%u/%u .functor %s %u",
			  net, level, idx/2, muxz, width);
		  fprintf(vvp_out, ", L_%p/%u/%u", net, level-1, idx+0);
		  fprintf(vvp_out, ", L_%p/%u/%u", net, level-1, idx+1);
		  fprintf(vvp_out, ", L_%p/%us",   net, level);
		  fprintf(vvp_out, ", C4<>;\n");
	    }

      }


      fprintf(vvp_out, "L_%p/%us .part %s, %u, 1; Bit %u of the select\n",
	      net, swidth-1, select_input, swidth-1, swidth-1);


      fprintf(vvp_out, "L_%p .functor %s %u", net, muxz, width);
      fprintf(vvp_out, ", L_%p/%u/0", net, swidth-2);
      fprintf(vvp_out, ", L_%p/%u/1", net, swidth-2);
      fprintf(vvp_out, ", L_%p/%us",  net, swidth-1);
      fprintf(vvp_out, ", C4<>;\n");

      free(select_input);
}

void draw_lpm_mux(ivl_lpm_t net)
{
      const char*muxz = "MUXZ";

	/* The output of the mux defines the type of the mux. the
	   ivl_target should guarantee that all the inputs are the
	   same type as the output. */
      switch (data_type_of_nexus(ivl_lpm_q(net))) {
	  case IVL_VT_REAL:
	    muxz = "MUXR";
	    break;
	  default:
	    muxz = "MUXZ";
	    break;
      }

      if ((ivl_lpm_size(net) == 2) && (ivl_lpm_selects(net) == 1)) {
	    draw_lpm_mux_ab(net, muxz);
	    return;
      }

	/* Here we are at the worst case, we generate a tree of MUXZ
	   devices to handle the arbitrary size. */
      draw_lpm_mux_nest(net, muxz);
}
