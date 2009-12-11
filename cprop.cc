/*
 * Copyright (c) 1998-2009 Stephen Williams (steve@icarus.com)
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

# include  <cstdlib>
# include  "netlist.h"
# include  "netmisc.h"
# include  "functor.h"
# include  "compiler.h"
# include  "ivl_assert.h"
# include  <vector>


/*
 * The cprop function below invokes constant propagation where
 * possible. The elaboration generates NetConst objects. I can remove
 * these and replace the gates connected to it with simpler ones. I
 * may even be able to replace nets with a new constant.
 */

struct cprop_functor  : public functor_t {

      unsigned count;

      virtual void signal(Design*des, NetNet*obj);
      virtual void lpm_add_sub(Design*des, NetAddSub*obj);
      virtual void lpm_compare(Design*des, NetCompare*obj);
      virtual void lpm_compare_eq_(Design*des, NetCompare*obj);
      virtual void lpm_ff(Design*des, NetFF*obj);
      virtual void lpm_logic(Design*des, NetLogic*obj);
      virtual void lpm_mux(Design*des, NetMux*obj);
};

void cprop_functor::signal(Design*des, NetNet*obj)
{
}

void cprop_functor::lpm_add_sub(Design*des, NetAddSub*obj)
{
}

void cprop_functor::lpm_compare(Design*des, NetCompare*obj)
{
      if (obj->pin_AEB().is_linked()) {
	    assert( ! obj->pin_AGB().is_linked() );
	    assert( ! obj->pin_AGEB().is_linked() );
	    assert( ! obj->pin_ALB().is_linked() );
	    assert( ! obj->pin_ALEB().is_linked() );
	    assert( ! obj->pin_AGB().is_linked() );
	    assert( ! obj->pin_ANEB().is_linked() );
	    lpm_compare_eq_(des, obj);
	    return;
      }
}

void cprop_functor::lpm_compare_eq_(Design*des, NetCompare*obj)
{
}

void cprop_functor::lpm_ff(Design*des, NetFF*obj)
{
	// Look for and count unlinked FF outputs. Note that if the
	// Data and Q pins are connected together, they can be removed
	// from the circuit, since it doesn't do anything.

      if (connected(obj->pin_Data(), obj->pin_Q())
	  && (! obj->pin_Sclr().is_linked())
	  && (! obj->pin_Sset().is_linked())
	  && (! obj->pin_Aclr().is_linked())
	  && (! obj->pin_Aset().is_linked())) {
	    obj->pin_Data().unlink();
	    obj->pin_Q().unlink();
	    delete obj;
      }
}

void cprop_functor::lpm_logic(Design*des, NetLogic*obj)
{
}

/*
 * This detects the case where the mux selects between a value and
 * Vz. In this case, replace the device with a mos with the sel
 * input used to enable the output.
 */
void cprop_functor::lpm_mux(Design*des, NetMux*obj)
{
      if (obj->size() != 2)
	    return;
      if (obj->sel_width() != 1)
	    return;

      Nexus*sel_nex = obj->pin_Sel().nexus();

	/* If the select input is constant, then replace with a BUFZ */

	// If the select is not constant, there is nothing we can do.
      if (! sel_nex->drivers_constant())
	    return;

	// If the select input is assigned or forced, then again there
	// is nothing we can do here.
      if (sel_nex->assign_lval())
	    return;

	// If the constant select is 'bz or 'bx, then give up.
      verinum::V sel_val = sel_nex->driven_value();
      if (sel_val == verinum::Vz || sel_val == verinum::Vx)
	    return;

	// The Select input must be a defined constant value, so we
	// can replace the device with a BUFZ.

      NetBUFZ*tmp = new NetBUFZ(obj->scope(), obj->name(), obj->width());
      tmp->set_line(*obj);

      if (debug_optimizer)
	    cerr << obj->get_fileline() << ": debug: "
		 << "Replace binary MUX with constant select=" << sel_val
		 << " with a BUFZ to the selected input." << endl;

      tmp->rise_time(obj->rise_time());
      tmp->fall_time(obj->fall_time());
      tmp->decay_time(obj->decay_time());

      connect(tmp->pin(0), obj->pin_Result());
      if (sel_val == verinum::V1)
	    connect(tmp->pin(1), obj->pin_Data(1));
      else
	    connect(tmp->pin(1), obj->pin_Data(0));
      delete obj;
      des->add_node(tmp);
      count += 1;
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

struct nexus_info_s {
      Nexus*nex;
      unsigned inp;
      unsigned out;
};

void cprop_dc_functor::lpm_const(Design*des, NetConst*obj)
{
	// 'bz constant values drive high impedance to whatever is
	// connected to it. In other words, it is a noop. But that is
	// only true if *all* the bits of the vectors.
      { unsigned tmp = 0;
	ivl_assert(*obj, obj->pin_count()==1);
	for (unsigned idx = 0 ;  idx < obj->width() ;  idx += 1) {
	      if (obj->value(idx) == verinum::Vz) {
		    tmp += 1;
	      }
	}

	if (tmp == obj->width()) {
	      delete obj;
	      return;
	}
      }

      std::vector<nexus_info_s> nexus_info (obj->pin_count());
      for (unsigned idx = 0 ; idx < obj->pin_count() ; idx += 1) {
	    nexus_info[idx].nex = obj->pin(idx).nexus();
	    unsigned inputs = 0, outputs = 0;
	    nexus_info[idx].nex -> count_io(inputs, outputs);
	    nexus_info[idx].inp = inputs;
	    nexus_info[idx].out = outputs;
      }

	// For each bit, if this is the only driver, then set the
	// initial value of all the signals to this value.
      for (unsigned idx = 0 ;  idx < obj->pin_count() ;  idx += 1) {
	    if (nexus_info[idx].out > 1)
		  continue;

	    for (Link*clnk = nexus_info[idx].nex->first_nlink()
		       ; clnk ; clnk = clnk->next_nlink()) {

		  NetPins*cur;
		  unsigned pin;
		  clnk->cur_link(cur, pin);

		  NetNet*tmp = dynamic_cast<NetNet*>(cur);
		  if (tmp == 0)
			continue;

		  tmp->pin(pin).set_init(obj->value(idx));
	    }
      }

	// If there are any links that take input, the constant is
	// used structurally somewhere.
      for (unsigned idx = 0 ;  idx < obj->pin_count() ;  idx += 1)
	    if (nexus_info[idx].inp > 0)
		  return;

	// Look for signals that have NetESignal nodes attached to
	// them. If I find any, then this constant is used by a
	// behavioral expression somewhere.
      for (unsigned idx = 0 ;  idx < obj->pin_count() ;  idx += 1) {

	    for (Link*clnk = nexus_info[idx].nex->first_nlink()
		       ; clnk ; clnk = clnk->next_nlink()) {

		  NetPins*cur;
		  unsigned pin;
		  clnk->cur_link(cur, pin);

		  NetNet*tmp = dynamic_cast<NetNet*>(cur);
		  if (tmp == 0)
			continue;

		  assert(tmp->scope());

		    // If the net is a signal name from the source,
		    // then users will probably want to see it in the
		    // waveform dump, so unhooking the constant will
		    // make it look wrong.
		  if (! tmp->local_flag())
			return;

		    // If the net has an eref, then there is an
		    // expression somewhere that reads this signal. So
		    // the constant does get read.
		  if (tmp->peek_eref() > 0)
			return;

		    // If the net is a port of the root module, then
		    // the constant may be driving something outside
		    // the design, so do not eliminate it.
		  if ((tmp->port_type() != NetNet::NOT_A_PORT)
		      && (tmp->scope()->parent() == 0))
			return;

	    }
      }

	// Done. Found no reason to keep this object, so delete it.
      delete obj;
}


void cprop(Design*des)
{
	// Continually propagate constants until a scan finds nothing
	// to do.
      cprop_functor prop;
      do {
	    prop.count = 0;
	    des->functor(&prop);
	    if (verbose_flag) {
		  cout << " ... Iteration detected "
		       << prop.count << " optimizations." << endl << flush;
	    }
      } while (prop.count > 0);

      if (verbose_flag) {
	    cout << " ... Look for dangling constants" << endl << flush;
      }
      cprop_dc_functor dc;
      des->functor(&dc);

      if (verbose_flag) {
	    cout << " ... done" << endl << flush;
      }
}
