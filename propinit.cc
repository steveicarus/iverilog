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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: propinit.cc,v 1.4 2000/06/25 19:59:42 steve Exp $"
#endif

/*
 * The propinit function runs through the devices that can impose
 * initial values in the netlist and propogates those values. The
 * process works by first scanning the active devices for outputs that
 * they generate.
 */

# include "netlist.h"

/*
 * prop_sequdp_output takes the output from the located sequential UDP
 * device and propogates it to the signals connected to it.
 */
static bool is_sequ_udp(const NetNode*net)
{
      const NetUDP*udp;
      if ((udp = dynamic_cast<const NetUDP*>(net)) == 0)
	    return false;

      return udp->is_sequential();
}

static void prop_sequdp_output(NetUDP*udp)
{
	/* Get from the UDP class the initial output value. */
      verinum::V ival;
      switch (udp->get_initial()) {
	  case '0':
	    ival = verinum::V0;
	    break;
	  case '1':
	    ival = verinum::V1;
	    break;
	  default:
	    ival = verinum::Vx;
	    break;
      }

	/* Take the output value and write it to all the NetNet pins
	   that are connected to the output pin. */

      Nexus*nex = udp->pin(0).nexus();
      for (Link*lnk = nex->first_nlink()
		 ; lnk  ; lnk = lnk->next_nlink()) {

	    if (NetNet*sig = dynamic_cast<NetNet*>(lnk->get_obj()))
		  sig->set_ival(lnk->get_pin(), ival);

      }
}

void propinit(Design*des)
{
      des->clear_node_marks();
      while (NetNode*net = des->find_node(&is_sequ_udp)) {
	    net->set_mark();
	    prop_sequdp_output(dynamic_cast<NetUDP*>(net));
      }
}

/*
 * $Log: propinit.cc,v $
 * Revision 1.4  2000/06/25 19:59:42  steve
 *  Redesign Links to include the Nexus class that
 *  carries properties of the connected set of links.
 *
 * Revision 1.3  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.2  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1998/12/20 02:05:41  steve
 *  Function to calculate wire initial value.
 *
 */

