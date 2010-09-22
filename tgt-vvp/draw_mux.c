/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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

      dly = "";
      if (d_rise != 0) {
	    dly = "/d";
	    if (number_is_immediate(d_rise, 64, 0) &&
	        number_is_immediate(d_fall, 64, 0) &&
	        number_is_immediate(d_decay, 64, 0)) {

		  assert( ! number_is_unknown(d_rise));
		  assert( ! number_is_unknown(d_fall));
		  assert( ! number_is_unknown(d_decay));

		  fprintf(vvp_out, "L_%p .delay (%" PRIu64 ",%" PRIu64 ",%" PRIu64 ") L_%p/d;\n",
		                   net, get_number_immediate64(d_rise),
		                   get_number_immediate64(d_fall),
		                   get_number_immediate64(d_decay), net);
	    } else {
		  ivl_signal_t sig;
		  // We do not currently support calculating the decay from
		  // the rise and fall variable delays.
		  assert(d_decay != 0);
		  assert(ivl_expr_type(d_rise) == IVL_EX_SIGNAL);
		  assert(ivl_expr_type(d_fall) == IVL_EX_SIGNAL);
		  assert(ivl_expr_type(d_decay) == IVL_EX_SIGNAL);

		  fprintf(vvp_out, "L_%p .delay L_%p/d", net, net);

		  sig = ivl_expr_signal(d_rise);
		  assert(ivl_signal_dimensions(sig) == 0);
		  fprintf(vvp_out, ", v%p_0", sig);

		  sig = ivl_expr_signal(d_fall);
		  assert(ivl_signal_dimensions(sig) == 0);
		  fprintf(vvp_out, ", v%p_0", sig);

		  sig = ivl_expr_signal(d_decay);
		  assert(ivl_signal_dimensions(sig) == 0);
		  fprintf(vvp_out, ", v%p_0;\n", sig);
	    }
      }

      input[0] = draw_net_input(ivl_lpm_data(net,0));
      input[1] = draw_net_input(ivl_lpm_data(net,1));
      input[2] = draw_net_input(ivl_lpm_select(net));
      fprintf(vvp_out, "L_%p%s .functor %s %u", net, dly, muxz, width);
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

      assert(swidth < sizeof(unsigned));
      assert(ivl_lpm_size(net) == (1U << swidth));

      select_input = strdup(draw_net_input(ivl_lpm_select(net)));

      fprintf(vvp_out, "L_%p/0s .part %s, 0, 1; Bit 0 of the select\n",
	      net, select_input);

      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 2) {
	    fprintf(vvp_out, "L_%p/0/%d .functor %s %u",
		    net, idx/2, muxz, width);
	    fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,idx+0)));
	    fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,idx+1)));
	    fprintf(vvp_out, ", L_%p/0s, C4<>;\n", net);
      }

      for (level = 1 ;  level < swidth-1 ;  level += 1) {
	    fprintf(vvp_out, "L_%p/%ds .part %s, %d, 1;\n",
		    net, level, select_input, level);

	    for (idx = 0 ;  idx < (ivl_lpm_size(net) >> level); idx += 2) {
		  fprintf(vvp_out, "L_%p/%d/%d .functor %s %u",
			  net, level, idx/2, muxz, width);
		  fprintf(vvp_out, ", L_%p/%d/%d", net, level-1, idx+0);
		  fprintf(vvp_out, ", L_%p/%d/%d", net, level-1, idx+1);
		  fprintf(vvp_out, ", L_%p/%ds",   net, level);
		  fprintf(vvp_out, ", C4<>;\n");
	    }

      }


      fprintf(vvp_out, "L_%p/%ds .part %s, %d, 1; Bit %d of the select\n",
	      net, swidth-1, select_input, swidth-1, swidth-1);


      fprintf(vvp_out, "L_%p .functor %s %u", net, muxz, width);
      fprintf(vvp_out, ", L_%p/%d/0", net, swidth-2);
      fprintf(vvp_out, ", L_%p/%d/1", net, swidth-2);
      fprintf(vvp_out, ", L_%p/%ds",  net, swidth-1);
      fprintf(vvp_out, ", C4<>;\n");

      free(select_input);
}

void draw_lpm_mux(ivl_lpm_t net)
{
      const char*muxz = "MUXZ";

	/* The output of the mux defines the type of the mux. the
	   ivl_target should guarantee that all the inputs are the
	   same type as the output. */
      switch (data_type_of_nexus(ivl_lpm_q(net,0))) {
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
