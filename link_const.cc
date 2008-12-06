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

		  const NetPins*obj = cur->get_obj();
		  const NetObj*as_obj = dynamic_cast<const NetObj*>(obj);
		  if (as_obj == 0 || as_obj->scope()->parent() != 0)
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
	    if (const NetNet*s = dynamic_cast<const NetNet*>(cur->get_obj()))
		  switch (s->type()) {
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

		    // If we find an attached SUPPLY0/1, the we know
		    // from that what the driven value is. Stop now.
		  if (sig->type() == NetNet::SUPPLY0) {
			driven_ = V0;
			return verinum::V0;
		  }
		  if (sig->type() == NetNet::SUPPLY1) {
			driven_ = V1;
			return verinum::V1;
		  }

		    // If we find an attached TRI0/1, then this is a
		    // good guess for the driven value, but keep
		    // looking for something better.
		  if (sig->type() == NetNet::TRI0) {
			val = verinum::V0;
		  }
		  if (sig->type() == NetNet::TRI1) {
			val = verinum::V1;
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

