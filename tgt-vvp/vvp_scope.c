/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: vvp_scope.c,v 1.1 2001/03/21 01:49:43 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>

/*
 * This function draws the major functional items within a scope. This
 * includes the scopes themselves, of course.
 */
int draw_scope(ivl_scope_t net, ivl_scope_t parent)
{
      unsigned idx;

      if (parent)
	    fprintf(vvp_out, "S_%s .scope \"%s\", S_%s;\n",
		    ivl_scope_name(net), ivl_scope_name(net),
		    ivl_scope_name(parent));
      else
	    fprintf(vvp_out, "S_%s .scope \"%s\";\n",
		    ivl_scope_name(net), ivl_scope_name(net));


      for (idx = 0 ;  idx < ivl_scope_sigs(net) ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(net, idx);
	    int msb = ivl_signal_pins(sig) - 1;
	    int lsb = 0;

	    fprintf(vvp_out, "V_%s .var \"%s\", %d, %d;\n",
		    ivl_signal_name(sig), ivl_signal_basename(sig),
		    msb, lsb);
      }

      ivl_scope_children(net, draw_scope, net);
      return 0;
}

/*
 * $Log: vvp_scope.c,v $
 * Revision 1.1  2001/03/21 01:49:43  steve
 *  Scan the scopes of a design, and draw behavioral
 *  blocking  assignments of constants to vectors.
 *
 */

