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
#ident "$Id: draw_mux.c,v 1.7 2003/12/19 01:27:10 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>

/*
 * This draws a general mux, a slice at a time. Use MUXX so that
 * unknows lead to unknown output.
 */
static void draw_lpm_mux_bitslice(ivl_lpm_t net, unsigned slice)
{
      unsigned sel = ivl_lpm_selects(net);
      unsigned size = ivl_lpm_size(net);
      unsigned sel_from_size;
      unsigned seldx, idx;
      ivl_nexus_t s;

      sel_from_size = 0;
      seldx = size - 1;
      while (seldx > 0) {
	    seldx >>= 1;
	    sel_from_size += 1;
      }
      if (sel_from_size > sel) {
	    fprintf(stderr, "internal error: MUX size=%u, selects=%u\n",
		    size, sel);
      }
      assert(sel_from_size <= sel);

      s = ivl_lpm_select(net, 0);

	/* Draw the leaf mux devices that take inputs from the
	   net. These also use up the least significant bit of the
	   select vector. */
      for (idx = 0 ;  idx < size ;  idx += 2) {

	    fprintf(vvp_out, "L_%s.%s/%u/%u/%u .functor MUXX, ",
		    vvp_mangle_id(ivl_scope_name(ivl_lpm_scope(net))),
		    vvp_mangle_id(ivl_lpm_basename(net)), slice, sel, idx);

	    {
		  ivl_nexus_t a = ivl_lpm_data2(net, idx+0, slice);
		  draw_input_from_net(a);
		  fprintf(vvp_out, ", ");
	    }

	    if ((idx+1) < size) {
		  ivl_nexus_t b = ivl_lpm_data2(net, idx+1, slice);
		  draw_input_from_net(b);
		  fprintf(vvp_out, ", ");
	    } else {
		  fprintf(vvp_out, "C<x>, ");
	    }

	    draw_input_from_net(s);
	    fprintf(vvp_out, ", C<1>;\n");
      }

	/* Draw the tree of MUXX devices to connect the inner tree
	   nodes. */
      for (seldx = 1 ;  seldx < (sel-1) ;  seldx += 1) {
	    unsigned level = sel - seldx;
	    unsigned span = 2 << seldx;
	    s = ivl_lpm_select(net, seldx);

	    for (idx = 0 ;  idx < size ;  idx += span) {
		  fprintf(vvp_out, "L_%s.%s/%u/%u/%u .functor MUXX, ",
			  vvp_mangle_id(ivl_scope_name(ivl_lpm_scope(net))),
			  vvp_mangle_id(ivl_lpm_basename(net)),
			  slice, level, idx);

		  fprintf(vvp_out, "L_%s.%s/%u/%u/%u, ",
			  vvp_mangle_id(ivl_scope_name(ivl_lpm_scope(net))),
			  vvp_mangle_id(ivl_lpm_basename(net)),
			  slice, level+1, idx);

		  if ((idx + span/2) < size) {
			fprintf(vvp_out, "L_%s.%s/%u/%u/%u, ",
				vvp_mangle_id(ivl_scope_name(ivl_lpm_scope(net))),
				vvp_mangle_id(ivl_lpm_basename(net)),
				slice, level+1, idx+span/2);
		  } else {
			fprintf(vvp_out, "C<x>, ");
		  }

		  draw_input_from_net(s);
		  fprintf(vvp_out, ", C<1>;\n");
	    }
      }

      s = ivl_lpm_select(net, sel-1);

      fprintf(vvp_out, "L_%s.%s/%u .functor MUXX, ",
	      vvp_mangle_id(ivl_scope_name(ivl_lpm_scope(net))),
	      vvp_mangle_id(ivl_lpm_basename(net)), slice);

      fprintf(vvp_out, "L_%s.%s/%u/2/0, ",
	      vvp_mangle_id(ivl_scope_name(ivl_lpm_scope(net))),
	      vvp_mangle_id(ivl_lpm_basename(net)), slice);


      if ((2U << (sel-1))/2 < size) {
	    fprintf(vvp_out, "L_%s.%s/%u/2/%u, ",
		    vvp_mangle_id(ivl_scope_name(ivl_lpm_scope(net))),
		    vvp_mangle_id(ivl_lpm_basename(net)),
		    slice, (2U << (sel-1))/2);
      } else {
	    fprintf(vvp_out, "C<x>, ");
      }

      draw_input_from_net(s);
      fprintf(vvp_out, ", C<1>;\n");
}

/*
 * This draws a simple A/B mux. The mux can have any width, enough
 * MUXZ nodes are created to support the vector.
 */
static void draw_lpm_mux_ab(ivl_lpm_t net)
{
      ivl_nexus_t s;
      unsigned idx, width;

	/* Only support A-B muxes at this oint. */
      assert(ivl_lpm_size(net) == 2);
      assert(ivl_lpm_selects(net) == 1);

      width = ivl_lpm_width(net);
      s = ivl_lpm_select(net, 0);

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t a = ivl_lpm_data2(net, 0, idx);
	    ivl_nexus_t b = ivl_lpm_data2(net, 1, idx);
	    fprintf(vvp_out, "L_%s.%s/%u .functor MUXZ, ",
		    vvp_mangle_id(ivl_scope_name(ivl_lpm_scope(net))),
		    vvp_mangle_id(ivl_lpm_basename(net)), idx);
	    draw_input_from_net(a);
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(b);
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(s);
	    fprintf(vvp_out, ", C<1>;\n");
      }

}

void draw_lpm_mux(ivl_lpm_t net)
{
      unsigned idx;

      if ((ivl_lpm_size(net) == 2) && (ivl_lpm_selects(net) == 1)) {
	    draw_lpm_mux_ab(net);
	    return;
      }

      for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 1)
	    draw_lpm_mux_bitslice(net, idx);

}

/*
 * $Log: draw_mux.c,v $
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
