/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: cprop.cc,v 1.4 1999/12/17 06:18:15 steve Exp $"
#endif

# include  "netlist.h"
# include  "functor.h"
# include  <assert.h>

/*
 * This local function returns true if all the the possible drivers of
 * this link are constant. It will also return true if there are no
 * drivers at all.
 */
static bool all_drivers_constant(const NetObj::Link&lnk)
{
      for (const NetObj::Link*cur = lnk.next_link()
		 ; *cur != lnk ; cur = cur->next_link()) {

	    if (cur->get_dir() == NetObj::Link::INPUT)
		  continue;
	    if (cur->get_dir() == NetObj::Link::PASSIVE)
		  continue;
	    if (! dynamic_cast<const NetConst*>(cur->get_obj()))
		  return false;
      }

      return true;
}

/*
 * This function returns the value of the constant driving this link,
 * or Vz if there is no constant. The results of this function are
 * only meaningful if all_drivers_constant(lnk) == true.
 */
static verinum::V driven_value(const NetObj::Link&lnk)
{
      for (const NetObj::Link*cur = lnk.next_link()
		 ; *cur != lnk ; cur = cur->next_link()) {

	    const NetConst*obj;
	    if (obj = dynamic_cast<const NetConst*>(cur->get_obj()))
		  return obj->value(cur->get_pin());
      }

      return verinum::Vz;
}

/*
 * The cprop function below invokes constant propogation where
 * possible. The elaboration generates NetConst objects. I can remove
 * these and replace the gates connected to it with simpler ones. I
 * may even be able to replace nets with a new constant.
 */

struct cprop_functor  : public functor_t {

      virtual void lpm_logic(Design*des, NetLogic*obj);
};

void cprop_functor::lpm_logic(Design*des, NetLogic*obj)
{
      switch (obj->type()) {

	  case NetLogic::AND:
	      // If there is one zero input to an AND gate, we know
	      // the resulting output is going to be zero and can
	      // elininate the gate.
	    for (unsigned idx = 1 ;  idx < obj->pin_count() ;  idx += 1) {
		  if (! all_drivers_constant(obj->pin(idx)))
			continue;
		  if (driven_value(obj->pin(idx)) == verinum::V0) {
			connect(obj->pin(0), obj->pin(idx));
			delete obj;
			return;
		  }
	    }

	      // There are no zero constant drivers. If there are any
	      // non-constant drivers, give up.
	    for (unsigned idx = 1 ;  idx < obj->pin_count() ;  idx += 1) {
		  if (! all_drivers_constant(obj->pin(idx)))
			return;
	    }

	      // If there are any non-1 values (Vx or Vz) then the
	      // result is Vx.
	    for (unsigned idx = 1 ;  idx < obj->pin_count() ;  idx += 1) {
		  if (driven_value(obj->pin(idx)) != verinum::V1) {
			connect(obj->pin(0), obj->pin(idx));
			delete obj;
			return;
		  }
	    }

	      // What's left? The inputs are all 1's, return the first
	      // input as the output value and remove the gate.
	    connect(obj->pin(0), obj->pin(1));
	    delete obj;
	    return;

	  default:
	    break;
      }
}

/*
 * This functor looks to see if the constant is connected to nothing
 * but signals. If that is the case, delete the dangling constant and
 * the now useless signals. This functor is applied after the regular
 * functor to clean up dangling constants that might be left behind.
 */
struct cprop_dc_functor  : public functor_t {

      virtual void lpm_const(Design*des, NetConst*obj);
};

void cprop_dc_functor::lpm_const(Design*des, NetConst*obj)
{
	// If there are any links that take input, abort this
	// operation.
      for (unsigned idx = 0 ;  idx < obj->pin_count() ;  idx += 1)
	    if (count_inputs(obj->pin(idx)) > 0)
		  return;

	// If there are no other drivers, delete all the signals that
	// are also dangling.
      for (unsigned idx = 0 ;  idx < obj->pin_count() ;  idx += 1)
	    if (count_outputs(obj->pin(idx)) == 1) {

		  NetObj*cur;
		  unsigned pin;
		  obj->pin(idx).next_link(cur, pin);
		  while (cur != obj) {
			cerr << "cprop: delete dangling signal " <<
			      cur->name() << "." << endl;
			delete cur;
			obj->pin(idx).next_link(cur, pin);
		  }
	    }

	// Done. Delete me.
      delete obj;
}


void cprop(Design*des)
{
      cprop_functor prop;
      des->functor(&prop);

      cprop_dc_functor dc;
      des->functor(&dc);
}

/*
 * $Log: cprop.cc,v $
 * Revision 1.4  1999/12/17 06:18:15  steve
 *  Rewrite the cprop functor to use the functor_t interface.
 *
 * Revision 1.3  1999/12/17 03:38:46  steve
 *  NetConst can now hold wide constants.
 *
 * Revision 1.2  1998/12/02 04:37:13  steve
 *  Add the nobufz function to eliminate bufz objects,
 *  Object links are marked with direction,
 *  constant propagation is more careful will wide links,
 *  Signal folding is aware of attributes, and
 *  the XNF target can dump UDP objects based on LCA
 *  attributes.
 *
 * Revision 1.1  1998/11/13 06:23:17  steve
 *  Introduce netlist optimizations with the
 *  cprop function to do constant propogation.
 *
 */

