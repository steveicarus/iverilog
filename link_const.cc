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
#ident "$Id: link_const.cc,v 1.4 2000/06/25 19:59:42 steve Exp $"
#endif

# include  "netlist.h"
# include  "netmisc.h"

bool link_drivers_constant(const Link&lnk)
{
      const Nexus*nex = lnk.nexus();
      for (const Link*cur = nex->first_nlink()
		 ; cur  ;  cur = cur->next_nlink()) {

	    if (cur == &lnk)
		  continue;
	    if (cur->get_dir() == Link::INPUT)
		  continue;
	    if (cur->get_dir() == Link::PASSIVE)
		  continue;
	    if (! dynamic_cast<const NetConst*>(cur->get_obj()))
		  return false;
      }

      return true;
}

verinum::V driven_value(const Link&lnk)
{
      const Nexus*nex = lnk.nexus();
      for (const Link*cur = nex->first_nlink()
		 ; cur  ;  cur = cur->next_nlink()) {

	    const NetConst*obj;
	    if (obj = dynamic_cast<const NetConst*>(cur->get_obj()))
		  return obj->value(cur->get_pin());
      }

      return verinum::Vz;
}

NetConst* link_const_value(Link&pin, unsigned&idx)
{
      NetConst*robj = 0;
      unsigned ridx = 0;

      Nexus*nex = pin.nexus();
      for (Link*cur = nex->first_nlink()
		 ; cur  ;  cur = cur->next_nlink()) {

	    NetConst*tmp;
	    if ((tmp = dynamic_cast<NetConst*>(cur->get_obj())) == 0)
		  continue;

	    if (robj != 0)
		  continue;

	    robj = tmp;
	    ridx = cur->get_pin();
      }

      idx = ridx;
      return robj;
}


/*
 * $Log: link_const.cc,v $
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

