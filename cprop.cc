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
#ident "$Id: cprop.cc,v 1.2 1998/12/02 04:37:13 steve Exp $"
#endif

# include  "netlist.h"
# include  <assert.h>

/*
 * The cprop function below invokes constant propogation where
 * possible. The elaboration generates NetConst objects. I can remove
 * these and replace the gates connected to it with simpler ones. I
 * may even be able to replace nets with a new constant.
 */

static bool is_a_const_node(const NetNode*obj)
{
      return dynamic_cast<const NetConst*>(obj);
}

static bool const_into_xnor(Design*des, NetConst*obj,
			    NetLogic*log, unsigned pin)
{
      assert(pin > 0);

	/* if this is the last input pin of the XNOR device, then
	   the device is simply buffering the constant value. */
      if (log->pin_count() == 2) {
	    cerr << "cprop: delete gate " << log->name() <<
		  " and propogate " << obj->value() << "." << endl;

	    assert(pin == 1);
	    connect(log->pin(0), log->pin(1));

	    delete log;
	    return true;
      }

	/* If this is a constant 0, then replace the gate with one
	   1-pin smaller. Skip this pin. */
      if (obj->value() == verinum::V0) {
	    cerr << "cprop: disconnect pin " << pin << " from gate "
		 << log->name() << "." << endl;

	    NetLogic*tmp = new NetLogic(log->name(),
					log->pin_count()-1,
					NetLogic::XNOR);
	    connect(log->pin(0), tmp->pin(0));
	    unsigned idx, jdx;
	    for (idx = 1, jdx = 1 ;  idx < log->pin_count() ;  idx += 1) {
		  if (idx == pin) continue;
		  connect(log->pin(idx), tmp->pin(jdx));
		  jdx += 1;
	    }

	    delete log;
	    des->add_node(tmp);
	    return true;
      }

	/* If this is a constant 1, then replace the gate with an XOR
	   that is 1-pin smaller. Removing the constant 1 causes the
	   sense of the output to change. */
      if (obj->value() == verinum::V1) {
	    cerr << "cprop: disconnect pin " << pin << " from gate "
		 << log->name() << "." << endl;

	    NetLogic*tmp = new NetLogic(log->name(),
					log->pin_count()-1,
					NetLogic::XOR);
	    connect(log->pin(0), tmp->pin(0));
	    unsigned idx, jdx;
	    for (idx = 1, jdx = 1 ;  idx < log->pin_count() ;  idx += 1) {
		  if (idx == pin) continue;
		  connect(log->pin(idx), tmp->pin(jdx));
		  jdx += 1;
	    }

	    delete log;
	    des->add_node(tmp);
	    return true;
      }

	/* If this is a constant X or Z, then the gate is certain to
	   generate an X. Replace the gate with a constant X. This may
	   cause other signals all over to become dangling. */
      if ((obj->value() == verinum::Vx) || (obj->value() == verinum::Vz)) {
	    cerr << "cprop: replace gate " << log->name() << " with "
		  "a constant X." << endl;

	    NetConst*tmp = new NetConst(log->name(), verinum::Vx);
	    connect(log->pin(0), tmp->pin(0));
	    delete log;
	    des->add_node(tmp);
	    return true;
      }

      return false;
}

static void look_for_core_logic(Design*des, NetConst*obj)
{
      NetObj*cur = obj;
      unsigned pin = 0;
      for (obj->pin(0).next_link(cur, pin)
		 ; cur != obj
		 ; cur->pin(pin).next_link(cur, pin)) {

	    NetLogic*log = dynamic_cast<NetLogic*>(cur);
	    if (log == 0)
		  continue;

	    bool flag = false;
	    switch (log->type()) {
		case NetLogic::XNOR:
		  flag = const_into_xnor(des, obj, log, pin);
		  break;
		default:
		  break;
	    }

	      /* If the optimization test tells me that a link was
		 deleted, restart the scan. */
	    if (flag) obj->pin(0).next_link(cur, pin);
      }
}

/*
 * This function looks to see if the constant is connected to nothing
 * but signals. If that is the case, delete the dangling constant and
 * the now useless signals.
 */
static void dangling_const(Design*des, NetConst*obj)
{
	// If there are any links that take input, abort this
	// operation.
      if (count_inputs(obj->pin(0)) > 0)
	    return;

	// If there are no other drivers, delete all the signals that
	// are also dangling.
      if (count_outputs(obj->pin(0)) == 1) {

	    NetObj*cur;
	    unsigned pin;
	    obj->pin(0).next_link(cur, pin);
	    while (cur != obj) {
		  cerr << "cprop: delete dangling signal " << cur->name() <<
			"." << endl;
		  delete cur;
		  obj->pin(0).next_link(cur, pin);
	    }
      }

	// Done. Delete me.
      delete obj;
}

void cprop(Design*des)
{
      des->clear_node_marks();
      while (NetNode*obj = des->find_node(&is_a_const_node)) {
	    NetConst*cur = dynamic_cast<NetConst*>(obj);
	    look_for_core_logic(des, cur);
	    cur->set_mark();
	    dangling_const(des, cur);
      }

}

/*
 * $Log: cprop.cc,v $
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

