/*
 * Copyright (c) 1998 Stephen Williams (steve@picturel.com)
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
#ident "$Id: nobufz.cc,v 1.1 1998/12/02 04:37:13 steve Exp $"
#endif

/* NOBUFZ Function
 * This function transforms the netlist by removing BUFZ nodes that
 * have no obvious effect. The assumption here is that the BUFZ node
 * transmits information perfectly in one direction, and not at all in
 * the other.
 *
 * A BUFZ will *not* be eliminated if the output link is connected to
 * other outputs. The BUFZ protects its input link from the
 * double-driving on its output, so the bufz is not meaningless and
 * cannot be removed.
 */

# include  "netlist.h"
# include  <assert.h>

static bool is_a_bufz_node(const NetNode*obj)
{
      return dynamic_cast<const NetBUFZ*>(obj);
}

void nobufz(Design*des)
{
      des->clear_node_marks();
      while (NetNode*obj = des->find_node(&is_a_bufz_node)) {
	    NetBUFZ*cur = dynamic_cast<NetBUFZ*>(obj);
	    assert(cur);

	      /* If there are more output pins on the output size of
		 the BUFZ, then the BUFZ has a real effect (it
		 protects its input side) and cannot be eliminated. */
	    if (count_outputs(cur->pin(0)) == 1) {
		  connect(cur->pin(0), cur->pin(1));
		  delete cur;
	    } else {
		  cur->set_mark();
	    }
      }
}

/*
 * $Log: nobufz.cc,v $
 * Revision 1.1  1998/12/02 04:37:13  steve
 *  Add the nobufz function to eliminate bufz objects,
 *  Object links are marked with direction,
 *  constant propagation is more careful will wide links,
 *  Signal folding is aware of attributes, and
 *  the XNF target can dump UDP objects based on LCA
 *  attributes.
 *
 */

