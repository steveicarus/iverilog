/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  <algorithm>
# include  <vector>
# include  <cstdlib>
# include  "netlist.h"
# include  "netmisc.h"
# include  "functor.h"
# include  "compiler.h"
# include  "ivl_assert.h"

using namespace std;

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
      virtual void lpm_compare(Design*des, const NetCompare*obj);
      virtual void lpm_concat(Design*des, NetConcat*obj);
      virtual void lpm_ff(Design*des, NetFF*obj);
      virtual void lpm_logic(Design*des, NetLogic*obj);
      virtual void lpm_mux(Design*des, NetMux*obj);
      virtual void lpm_part_select(Design*des, NetPartSelect*obj);

      void lpm_compare_eq_(Design*des, const NetCompare*obj);
 };

void cprop_functor::signal(Design*, NetNet*)
{
}

void cprop_functor::lpm_add_sub(Design*, NetAddSub*)
{
}

void cprop_functor::lpm_compare(Design*des, const NetCompare*obj)
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

void cprop_functor::lpm_compare_eq_(Design*, const NetCompare*)
{
}

void cprop_functor::lpm_concat(Design*des, NetConcat*obj)
{
	// Sorry, I don't know how to constant-propagate through
	// transparent concatenations.
      if (obj->transparent())
	    return;

      verinum result (verinum::Vz, obj->width());
      unsigned off = 0;

      for (unsigned idx = 1 ; idx < obj->pin_count() ; idx += 1) {
	    Nexus*nex = obj->pin(idx).nexus();
	      // If there are non-constant drivers, then give up.
	    if (! nex->drivers_constant())
		  return;

	    verinum tmp = nex->driven_vector();
	    result.set(off, tmp);
	    off += tmp.len();
      }

      if (debug_optimizer)
	    cerr << obj->get_fileline() << ": cprop_functor::lpm_concat: "
		 << "Replace NetConcat with " << result << "." << endl;


      NetScope*scope = obj->scope();

	// Create a NetConst object to carry the result. Give it the
	// same name as the Concat object that we are replacing, and
	// link the NetConst to the NetConcat object. Then delete the
	// concat that is now replaced.
      NetConst*result_obj = new NetConst(scope, obj->name(), result);
      result_obj->set_line(*obj);
      des->add_node(result_obj);
      connect(obj->pin(0), result_obj->pin(0));

	// Note that this will leave the const inputs to dangle. They
	// will be reaped by other passes of cprop_functor.
      delete obj;

      count += 1;
}

void cprop_functor::lpm_ff(Design*, NetFF*obj)
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

void cprop_functor::lpm_logic(Design*, NetLogic*)
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

	// If the constant select is 'bz or 'bx, then give up.
      verinum::V sel_val = sel_nex->driven_value();
      if (sel_val == verinum::Vz || sel_val == verinum::Vx)
	    return;

	// The Select input must be a defined constant value, so we
	// can replace the device with a BUFZ.

      NetBUFZ*tmp = new NetBUFZ(obj->scope(), obj->name(), obj->width(), true);
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

static bool compare_base(NetPartSelect*a, NetPartSelect*b)
{
      return a->base() < b->base();
}

/*
 * This optimization searches for Nexa that are driven only by
 * NetPartSelect(PV) outputs. These might turn from Verilog input that
 * looks like this:
 *    wire [7:0] foo
 *    assign foo[7:4] = a;
 *    assign foo[3:0] = b;
 * The idea is to convert the part selects of the above to a single
 * concatenation that looks like this:
 *    assign foo = {a, b};
 */
void cprop_functor::lpm_part_select(Design*des, NetPartSelect*obj)
{
      if (obj->dir() != NetPartSelect::PV)
	    return;

      NetScope*scope = obj->scope();
      Nexus*nex = obj->pin(1).nexus();
      vector<NetPartSelect*> obj_set;

      bool output_2_state = false;

      for (Link*cur = nex->first_nlink() ; cur ; cur = cur->next_nlink()) {

	    NetPins*tmp_obj = cur->get_obj();

	      // Record if we are driving a 2-state net.
	    NetNet*net_obj = dynamic_cast<NetNet*> (tmp_obj);
	    if (net_obj && (net_obj->data_type() == IVL_VT_BOOL))
		  output_2_state = true;

	      // If this is an input (or passive) then ignore it.
	    if (cur->get_dir() != Link::OUTPUT)
		  continue;

	      // Check to see if this is the output of a
	      // NetPartSelect::PV. If not, then give up on the blend.
	    unsigned tmp_pin = cur->get_pin();

	    NetPartSelect*cur_obj = dynamic_cast<NetPartSelect*> (tmp_obj);
	    if (cur_obj == 0)
		  return;

	    if (cur_obj->dir() != NetPartSelect::PV)
		  return;

	    if (tmp_pin != 1)
		  return;

	    obj_set.push_back(cur_obj);
      }

	// When driving a 4-state signal, we only want to create a
	// concatenation if we have more than one part select. But
	// when driving a 2-state signal, create a concatenation
	// even if there's only one part select, which forces the
	// undriven bits to zero without needing an explicit cast.
      if ((obj_set.size() == 0) || ((obj_set.size() == 1) && !output_2_state))
	    return;

      if (debug_optimizer)
	    cerr << obj->get_fileline() << ": cprop::lpm_part_select: "
		 << "Found " << obj_set.size() << " NetPartSelect(PV) objects."
		 << endl;

	// Sort by increasing base offset.
      sort(obj_set.begin(), obj_set.end(), compare_base);

	// Check and make sure there are no overlaps. If there are,
	// then give up on this optimization.
      for (size_t idx = 1 ; idx < obj_set.size() ; idx += 1) {
	    unsigned top = obj_set[idx-1]->base() + obj_set[idx-1]->width();
	    if (top > obj_set[idx]->base()) {
		  if (debug_optimizer)
			cerr << obj->get_fileline() << ": cprop::lpm_part_select: "
			     << "Range [" << obj_set[idx-1]->base()
			     << " " << top << ") overlaps PV starting at "
			     << obj_set[idx]->base() << ". Give up." << endl;
		  return;
	    }
      }

	// Check if the tail runs off the end of the target. If so it
	// should be possible to replace it with a bit select to
	// shorten the object for the target, but for now just give up.
      unsigned sig_width = nex->vector_width();
      if (obj_set.back()->base() + obj_set.back()->width() > sig_width) {
	    if (debug_optimizer)
		  cerr << obj->get_fileline() << ": cprop::lpm_part_select: "
		       << "Range [" << obj_set.back()->base()
		       << ":" << (obj_set.back()->base() + obj_set.back()->width() - 1)
		       << "] runs off the end of target." << endl;
	    return;
      }

	// Figure out how many components we are going to need.
      unsigned part_count = 0;
      unsigned off = 0;
      for (size_t idx = 0 ; idx < obj_set.size() ; idx += 1) {
	    if (obj_set[idx]->base() > off) {
		  off = obj_set[idx]->base();
		  part_count += 1;
	    }
	    off += obj_set[idx]->width();
	    part_count += 1;
      }

      if (off < sig_width)
	    part_count += 1;

      NetConcat*cncat = new NetConcat(scope, scope->local_symbol(),
				       sig_width, part_count);
      cncat->set_line(*obj);
      des->add_node(cncat);
      connect(cncat->pin(0), obj->pin(1));

      off = 0;
      size_t concat_pin = 1;
      for (size_t idx = 0 ; idx < obj_set.size() ; idx += 1) {
	    NetPartSelect*cobj = obj_set[idx];
	    if (cobj->base() > off) {
		  NetNet*val = output_2_state
                             ? make_const_0(des, scope, cobj->base()-off)
                             : make_const_z(des, scope, cobj->base()-off);
		  connect(cncat->pin(concat_pin), val->pin(0));
		  concat_pin += 1;
		  off = cobj->base();
	    }
	    connect(cncat->pin(concat_pin), cobj->pin(0));
	    concat_pin += 1;
	    off += cobj->width();
      }
      if (off < sig_width) {
	    NetNet*val = output_2_state
                       ? make_const_0(des, scope, sig_width-off)
                       : make_const_z(des, scope, sig_width-off);
	    connect(cncat->pin(concat_pin), val->pin(0));
	    concat_pin += 1;
      }
      ivl_assert(*obj, concat_pin == cncat->pin_count());

      for (size_t idx = 0 ; idx < obj_set.size() ; idx += 1) {
	    delete obj_set[idx];
      }

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

void cprop_dc_functor::lpm_const(Design*, NetConst*obj)
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

		  const NetNet*tmp = dynamic_cast<NetNet*>(cur);
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
