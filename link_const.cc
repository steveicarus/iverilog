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
#ifdef HAVE_CVS_IDENT
#ident "$Id: link_const.cc,v 1.17 2004/10/04 01:10:53 steve Exp $"
#endif

# include "config.h"

# include  "netlist.h"
# include  "netmisc.h"

/*
 * Scan the link for drivers. If there are only constant drivers, then
 * the nexus has a known constant value. If there is a supply net,
 * then the nexus again has a known constant value.
 */
bool Nexus::drivers_constant() const
{
      if (driven_ == VAR)
	    return false;
      if (driven_ != NO_GUESS)
	    return true;

      for (const Link*cur = list_ ; cur  ;  cur = cur->next_) {
	    const NetNet*sig;
	    Link::DIR cur_dir;

	    cur_dir = cur->get_dir();
	    if (cur_dir == Link::INPUT)
		  continue;

	      /* If this is an input or inout port of a root module,
		 then the is probably not a constant value. I
		 certainly don't know what the value is, anyhow. This
		 can happen in cases like this:

		 module main(sig);
		     input sig;
		 endmodule

		 If main is a root module (it has no parent) then sig
		 is not constant because it connects to an unspecified
		 outside world. */

	    if (cur_dir == Link::PASSIVE) {

		  const NetObj*obj = cur->get_obj();
		  if (obj->scope()->parent() != 0)
			continue;

		  sig = dynamic_cast<const NetNet*>(cur->get_obj());
		  assert(sig);

		  if (sig->port_type() == NetNet::NOT_A_PORT)
			continue;

		  if (sig->port_type() == NetNet::POUTPUT)
			continue;

		  driven_ = VAR;
		  return false;

	    }

	      /* If there is a supply net, then this nexus will have a
		 constant value independent of any drivers. */
	    if (const NetNet*sig = dynamic_cast<const NetNet*>(cur->get_obj()))
		  switch (sig->type()) {
		      case NetNet::SUPPLY0:
			driven_ = V0;
			return true;
		      case NetNet::SUPPLY1:
			driven_ = V1;
			return true;
		      default:
			break;
		  }

	    if (! dynamic_cast<const NetConst*>(cur->get_obj())) {
		  driven_ = VAR;
		  return false;
	    }
      }

      return true;
}

verinum::V Nexus::driven_value() const
{
      switch (driven_) {
	  case V0:
	    return verinum::V0;
	  case V1:
	    return verinum::V1;
	  case Vx:
	    return verinum::Vx;
	  case Vz:
	    return verinum::Vz;
	  case VAR:
	    assert(0);
	    break;
	  case NO_GUESS:
	    break;
      }

      const Link*cur = list_;

      verinum::V val = verinum::Vz;

      for (cur = list_ ; cur  ;  cur = cur->next_) {

	    const NetConst*obj;
	    const NetNet*sig;
	    if ((obj = dynamic_cast<const NetConst*>(cur->get_obj()))) {
		  val = obj->value(cur->get_pin());

	    } else if ((sig = dynamic_cast<const NetNet*>(cur->get_obj()))) {

		  if (sig->type() == NetNet::SUPPLY0) {
			driven_ = V0;
			return verinum::V0;
		  }
		  if (sig->type() == NetNet::SUPPLY1) {
			driven_ = V1;
			return verinum::V1;
		  }
	    }
      }

	/* Cache the result. */
      switch (val) {
	  case verinum::V0:
	    driven_ = V0;
	    break;
	  case verinum::V1:
	    driven_ = V1;
	    break;
	  case verinum::Vx:
	    driven_ = Vx;
	    break;
	  case verinum::Vz:
	    driven_ = Vz;
	    break;
      }

      return val;
}

/*
 * $Log: link_const.cc,v $
 * Revision 1.17  2004/10/04 01:10:53  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.16  2003/06/21 01:21:43  steve
 *  Harmless fixup of warnings.
 *
 * Revision 1.15  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.14  2002/06/25 01:33:22  steve
 *  Cache calculated driven value.
 *
 * Revision 1.13  2002/06/24 01:49:39  steve
 *  Make link_drive_constant cache its results in
 *  the Nexus, to improve cprop performance.
 *
 * Revision 1.12  2002/06/19 04:18:46  steve
 *  Shuffle link_drivers_constant for speed.
 *
 * Revision 1.11  2001/08/25 23:50:03  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.10  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.9  2001/07/07 03:01:37  steve
 *  Detect and make available to t-dll the right shift.
 *
 * Revision 1.8  2001/02/16 03:27:07  steve
 *  links to root inputs are not constant.
 *
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

