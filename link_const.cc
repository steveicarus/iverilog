/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: link_const.cc,v 1.7 2000/11/20 01:41:12 steve Exp $"
#endif

# include  "netlist.h"
# include  "netmisc.h"

/*
 * Scan the link for drivers. If there are only constant drivers, then
 * the nexus has a known constant value. If there is a supply net,
 * then the nexus again has a known constant value.
 */
bool link_drivers_constant(const Link&lnk)
{
      const Nexus*nex = lnk.nexus();
      bool flag = true;

      for (const Link*cur = nex->first_nlink()
		 ; cur  ;  cur = cur->next_nlink()) {

	    if (cur == &lnk)
		  continue;
	    if (cur->get_dir() == Link::INPUT)
		  continue;

	      /* If the link is PASSIVE then it doesn't count as a
		 driver if its initial value is Vz. This is pertinant
		 because REGs are PASSIVE but can receive values from
		 procedural assignments. */
	    if ((cur->get_dir() == Link::PASSIVE)
		&& (cur->get_init() == verinum::Vz))
		  continue;

	    if (! dynamic_cast<const NetConst*>(cur->get_obj()))
		  flag = false;

	      /* If there is a supply net, then this nexus will have a
		 constant value independent of any drivers. */
	    if (const NetNet*sig = dynamic_cast<const NetNet*>(cur->get_obj()))
		  switch (sig->type()) {
		      case NetNet::SUPPLY0:
		      case NetNet::SUPPLY1:
			return true;
		      default:
			break;
		  }
      }

      return flag;
}

verinum::V driven_value(const Link&lnk)
{
      verinum::V val = lnk.get_init();

      const Nexus*nex = lnk.nexus();
      for (const Link*cur = nex->first_nlink()
		 ; cur  ;  cur = cur->next_nlink()) {

	    const NetConst*obj;
	    const NetNet*sig;
	    if (obj = dynamic_cast<const NetConst*>(cur->get_obj())) {
		  val = obj->value(cur->get_pin());

	    } else if (sig = dynamic_cast<const NetNet*>(cur->get_obj())) {

		  if (sig->type() == NetNet::SUPPLY0)
			return verinum::V0;

		  if (sig->type() == NetNet::SUPPLY1)
			return verinum::V1;
	    }
      }

      return val;
}

/*
 * $Log: link_const.cc,v $
 * Revision 1.7  2000/11/20 01:41:12  steve
 *  Whoops, return the calculated constant value rom driven_value.
 *
 * Revision 1.6  2000/11/20 00:58:40  steve
 *  Add support for supply nets (PR#17)
 *
 * Revision 1.5  2000/07/14 06:12:57  steve
 *  Move inital value handling from NetNet to Nexus
 *  objects. This allows better propogation of inital
 *  values.
 *
 *  Clean up constant propagation  a bit to account
 *  for regs that are not really values.
 *
 * Revision 1.4  2000/06/25 19:59:42  steve
 *  Redesign Links to include the Nexus class that
 *  carries properties of the connected set of links.
 *
 * Revision 1.3  2000/05/14 17:55:04  steve
 *  Support initialization of FF Q value.
 *
 * Revision 1.2  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.1  2000/04/20 00:28:03  steve
 *  Catch some simple identity compareoptimizations.
 *
 */

