/*
 * Copyright (c) 2002-2022 Stephen Williams (steve@icarus.com)
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

# include  "functor.h"
# include  "netlist.h"
# include  "netvector.h"
# include  "netmisc.h"
# include  "compiler.h"
# include  "ivl_assert.h"

using namespace std;

/* General notes on enables and bitmasks.
 *
 * When synthesising an asynchronous process that contains conditional
 * statements (if/case statements), we need to determine the conditions
 * that cause each nexus driven by that process to be updated. If a
 * nexus is not updated under all circumstances, we must infer a latch.
 * To this end, we generate an enable signal for each output nexus. As
 * we walk the statement tree for the process, for each substatement we
 * pass the enable signals generated so far into the synth_async method,
 * and on return from the synth_async method, the enable signals will be
 * updated to reflect any conditions introduced by that substatement.
 * Once we have synthesised all the statements for that process, if an
 * enable signal is not tied high, we must infer a latch for that nexus.
 *
 * When synthesising a synchronous process, we use the synth_async method
 * to synthesise the combinatorial inputs to the D pins of the flip-flops
 * we infer for that process. In this case the enable signal can be used
 * as a clock enable for the flip-flop. This saves us explicitly feeding
 * back the flip-flop output to undriven inputs of any synthesised muxes.
 *
 * The strategy described above is not sufficient when not all bits in
 * a nexus are treated identically (i.e. different conditional clauses
 * drive differing parts of the same vector). To handle this properly,
 * we would (potentially) need to generate a separate enable signal for
 * each bit in the vector. This would be a lot of work, particularly if
 * we wanted to eliminate duplicates. For now, the strategy employed is
 * to maintain a bitmask for each output nexus that identifies which bits
 * in the nexus are unconditionally driven (driven by every clause). When
 * we finish synthesising an asynchronous process, if the bitmask is not
 * all ones, we must infer a latch. This currently results in an error,
 * because to safely synthesise such a latch we would need the bit-level
 * gate enables. When we finish synthesising a synchronous process, if
 * the bitmask is not all ones, we explicitly feed the flip-flop outputs
 * back to undriven inputs of any synthesised muxes to ensure undriven
 * parts of the vector retain their previous state when the flip-flop is
 * clocked.
 *
 * The enable signals are passed as links to the current output nexus
 * for each signal. If an enable signal is not linked, this is treated
 * as if the signal was tied low.
 *
 * The bitmasks are passed as bool vectors. 'true' indicates a bit is
 * unconditionally driven. An empty vector (size = 0) indicates that
 * the current substatement doesn't drive any bits in the nexus.
 */

static void qualify_enable(Design*des, NetScope*scope, NetNet*qualifier,
			   bool active_state, NetLogic::TYPE gate_type,
			   Link&enable_i, Link&enable_o)
{
      if (enable_i.is_linked(scope->tie_lo())) {
	    connect(enable_o, scope->tie_lo());
	    return;
      }

      if (active_state == false) {
	    NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
					 2, NetLogic::NOT, 1);
	    des->add_node(gate);
	    connect(gate->pin(1), qualifier->pin(0));

	    NetNet*sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE,
				    &netvector_t::scalar_logic);
	    sig->local_flag(true);
	    connect(sig->pin(0), gate->pin(0));

	    qualifier = sig;
      }

      if (enable_i.is_linked(scope->tie_hi())) {
	    connect(enable_o, qualifier->pin(0));
	    return;
      }

      NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
				   3, gate_type, 1);
      des->add_node(gate);
      connect(gate->pin(1), qualifier->pin(0));
      connect(gate->pin(2), enable_i);
      connect(enable_o, gate->pin(0));

      NetNet*sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE,
			      &netvector_t::scalar_logic);
      sig->local_flag(true);
      connect(sig->pin(0), gate->pin(0));
}

static void multiplex_enables(Design*des, NetScope*scope, NetNet*select,
			      Link&enable_1, Link&enable_0, Link&enable_o)
{
      if (!enable_1.is_linked() &&
	  !enable_0.is_linked() )
	    return;

      if ( enable_1.is_linked(scope->tie_hi()) &&
	   enable_0.is_linked(scope->tie_hi()) ) {
	    connect(enable_o, scope->tie_hi());
	    return;
      }

      if (enable_1.is_linked(scope->tie_lo()) || !enable_1.is_linked()) {
	    qualify_enable(des, scope, select, false, NetLogic::AND,
			   enable_0, enable_o);
	    return;
      }
      if (enable_0.is_linked(scope->tie_lo()) || !enable_0.is_linked()) {
	    qualify_enable(des, scope, select, true,  NetLogic::AND,
			   enable_1, enable_o);
	    return;
      }
      if (enable_1.is_linked(scope->tie_hi())) {
	    qualify_enable(des, scope, select, true,  NetLogic::OR,
			   enable_0, enable_o);
	    return;
      }
      if (enable_0.is_linked(scope->tie_hi())) {
	    qualify_enable(des, scope, select, false, NetLogic::OR,
			   enable_1, enable_o);
	    return;
      }

      NetMux*mux = new NetMux(scope, scope->local_symbol(), 1, 2, 1);
      des->add_node(mux);
      connect(mux->pin_Sel(),	select->pin(0));
      connect(mux->pin_Data(1), enable_1);
      connect(mux->pin_Data(0), enable_0);
      connect(enable_o, mux->pin_Result());

      NetNet*sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE,
			      &netvector_t::scalar_logic);
      sig->local_flag(true);
      connect(sig->pin(0), mux->pin_Result());
}

static void merge_sequential_enables(Design*des, NetScope*scope,
				     Link&top_enable, Link&sub_enable)
{
      if (!sub_enable.is_linked())
	    return;

      if (top_enable.is_linked(scope->tie_hi()))
	    return;

      if (sub_enable.is_linked(scope->tie_hi()))
	    top_enable.unlink();

      if (top_enable.is_linked()) {
	    NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
					 3, NetLogic::OR, 1);
	    des->add_node(gate);
	    connect(gate->pin(1), sub_enable);
	    connect(gate->pin(2), top_enable);
	    top_enable.unlink();
	    connect(top_enable, gate->pin(0));

	    NetNet*sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE,
				    &netvector_t::scalar_logic);
	    sig->local_flag(true);
	    connect(sig->pin(0), gate->pin(0));
      } else {
	    connect(top_enable, sub_enable);
      }
}

static void merge_sequential_masks(NetProc::mask_t&top_mask, const NetProc::mask_t&sub_mask)
{
      if (sub_mask.size() == 0)
	    return;

      if (top_mask.size() == 0) {
	    top_mask = sub_mask;
	    return;
      }

      assert(top_mask.size() == sub_mask.size());
      for (unsigned idx = 0 ; idx < top_mask.size() ; idx += 1) {
	    if (sub_mask[idx] == true)
		  top_mask[idx] = true;
      }
}

static void merge_parallel_masks(NetProc::mask_t&top_mask, const NetProc::mask_t&sub_mask)
{
      if (sub_mask.size() == 0)
	    return;

      if (top_mask.size() == 0) {
	    top_mask = sub_mask;
	    return;
      }

      assert(top_mask.size() == sub_mask.size());
      for (unsigned idx = 0 ; idx < top_mask.size() ; idx += 1) {
	    if (sub_mask[idx] == false)
		  top_mask[idx] = false;
      }
}

static bool all_bits_driven(const NetProc::mask_t&mask)
{
      if (mask.size() == 0)
	    return false;

      for (unsigned idx = 0 ; idx < mask.size() ; idx += 1) {
	    if (mask[idx] == false)
		  return false;
      }
      return true;
}

bool NetProcTop::tie_off_floating_inputs_(Design*des,
					  NexusSet&nex_map, NetBus&nex_in,
					  const vector<NetProc::mask_t>&bitmasks,
					  bool is_ff_input)
{
      bool flag = true;
      for (unsigned idx = 0 ; idx < nex_in.pin_count() ; idx += 1) {
	    if (nex_in.pin(idx).nexus()->has_floating_input()) {
		  if (all_bits_driven(bitmasks[idx])) {
			  // If all bits are unconditionally driven, we can
			  // use the enable signal to prevent the flip-flop/
			  // latch from updating when an undriven mux input
			  // is selected, so we can just tie off the input.
			unsigned width = nex_map[idx].wid;
			NetLogic*gate = new NetLogic(scope(), scope()->local_symbol(),
						     1, NetLogic::PULLDOWN, width);
			des->add_node(gate);
			connect(nex_in.pin(idx), gate->pin(0));

			if (nex_in.pin(idx).nexus()->pick_any_net())
			      continue;

			ivl_variable_type_t data_type = IVL_VT_LOGIC;
			netvector_t*tmp_vec = new netvector_t(data_type, width-1,0);
			NetNet*sig = new NetNet(scope(), scope()->local_symbol(),
						NetNet::WIRE, tmp_vec);
			sig->local_flag(true);
			connect(sig->pin(0), gate->pin(0));
		  } else if (is_ff_input) {
			  // For a flip-flop, we can feed back the output
			  // to ensure undriven bits hold their last value.
			connect(nex_in.pin(idx), nex_map[idx].lnk);
		  } else {
			  // This infers a latch, but without generating
			  // gate enable signals at the bit-level, we
			  // can't safely latch the undriven bits (we
			  // shouldn't generate combinatorial loops).
			cerr << get_fileline() << ": warning: A latch "
			     << "has been inferred for some bits of '"
			     << nex_map[idx].lnk.nexus()->pick_any_net()->name()
			     << "'." << endl;

			cerr << get_fileline() << ": sorry: Bit-level "
				"latch gate enables are not currently "
				"supported in synthesis." << endl;
			des->errors += 1;
			flag = false;
		  }
	    }
      }
      return flag;
}

bool NetProc::synth_async(Design*, NetScope*, NexusSet&, NetBus&, NetBus&, vector<mask_t>&)
{
      return false;
}

/*
 * Async synthesis of assignments is done by synthesizing the rvalue
 * expression, then connecting the l-value directly to the output of
 * the r-value.
 *
 * The nex_map is the O-set for the statement, and lists the positions
 * of the outputs as the caller wants results linked up. The nex_out,
 * however, is the set of nexa that are to actually get linked to the
 * r-value.
 */
bool NetAssignBase::synth_async(Design*des, NetScope*scope,
				NexusSet&nex_map, NetBus&nex_out,
				NetBus&enables, vector<mask_t>&bitmasks)
{
      if (dynamic_cast<NetCAssign*>(this) || dynamic_cast<NetDeassign*>(this) ||
          dynamic_cast<NetForce*>(this) || dynamic_cast<NetRelease*>(this)) {
	    cerr << get_fileline() << ": sorry: Procedural continuous "
		    "assignment is not currently supported in synthesis."
		 << endl;
	    des->errors += 1;
	    return false;
      }

	/* If the lval is a concatenation, synthesise each part
	   separately. */
      if (lval_->more ) {
	      /* Temporarily set the lval_ and rval_ fields for each
		 part in turn and recurse. Restore them when done. */
	    NetAssign_*full_lval = lval_;
	    NetExpr*full_rval = rval_;
	    unsigned offset = 0;
	    bool flag = true;
	    while (lval_) {
		  unsigned width = lval_->lwidth();
		  NetEConst*base = new NetEConst(verinum(offset));
		  base->set_line(*this);
		  rval_ = new NetESelect(full_rval->dup_expr(), base, width);
		  rval_->set_line(*this);
		  eval_expr(rval_, width);
		  NetAssign_*more = lval_->more;
		  lval_->more = 0;
		  if (!synth_async(des, scope, nex_map, nex_out, enables, bitmasks))
			flag = false;
		  lval_ = lval_->more = more;
		  offset += width;
	    }
	    lval_ = full_lval;
	    rval_ = full_rval;
	    return flag;
      }

      assert(rval_);
      NetNet*rsig = rval_->synthesize(des, scope, rval_);
      assert(rsig);

      if (lval_->word() && ! dynamic_cast<NetEConst*>(lval_->word())) {
	    cerr << get_fileline() << ": sorry: Assignment to variable "
		    "location in memory is not currently supported in "
		    "synthesis." << endl;
	    des->errors += 1;
	    return false;
      }

      NetNet*lsig = lval_->sig();
      if (!lsig) {
	    cerr << get_fileline() << ": error: "
		    "NetAssignBase::synth_async on unsupported lval ";
	    dump_lval(cerr);
	    cerr << endl;
	    des->errors += 1;
	    return false;
      }

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetAssignBase::synth_async: "
		 << "l-value signal is " << lsig->vector_width() << " bits, "
		 << "r-value signal is " << rsig->vector_width() << " bits." << endl;
	    cerr << get_fileline() << ": NetAssignBase::synth_async: "
		 << "lval_->lwidth()=" << lval_->lwidth() << endl;
	    cerr << get_fileline() << ": NetAssignBase::synth_async: "
		 << "lsig = " << scope_path(scope) << "." << lsig->name() << endl;
	    if (const NetExpr*base = lval_->get_base()) {
		  cerr << get_fileline() << ": NetAssignBase::synth_async: "
		       << "base_=" << *base << endl;
	    }
	    cerr << get_fileline() << ": NetAssignBase::synth_async: "
		 << "nex_map.size()==" << nex_map.size()
		 << ", nex_out.pin_count()==" << nex_out.pin_count() << endl;
      }

      unsigned ptr = 0;
      if (nex_out.pin_count() > 1) {
	    NexusSet tmp_set;
	    nex_output(tmp_set);
	    ivl_assert(*this, tmp_set.size() == 1);
	    ptr = nex_map.find_nexus(tmp_set[0]);
	    ivl_assert(*this, nex_out.pin_count() > ptr);
	    ivl_assert(*this, enables.pin_count() > ptr);
	    ivl_assert(*this, bitmasks.size() > ptr);
      } else {
	    ivl_assert(*this, nex_out.pin_count() == 1);
	    ivl_assert(*this, enables.pin_count() == 1);
	    ivl_assert(*this, bitmasks.size() == 1);
      }

      unsigned lval_width = lval_->lwidth();
      unsigned lsig_width = lsig->vector_width();
      ivl_assert(*this, nex_map[ptr].wid == lsig_width);

	// Here we note if the l-value is actually a bit/part
	// select. If so, generate a NetPartSelect to perform the select.
      bool is_part_select = lval_width != lsig_width;

      long base_off = 0;
      if (is_part_select && !scope->loop_index_tmp.empty()) {
	      // If we are within a NetForLoop, there may be an index
	      // value. That is collected from the scope member
	      // loop_index_tmp, and the evaluate_function method
	      // knows how to apply it.
	    ivl_assert(*this, !scope->loop_index_tmp.empty());
	    ivl_assert(*this, lval_width < lsig_width);

	      // Evaluate the index expression to a constant.
	    const NetExpr*base_expr_raw = lval_->get_base();
	    ivl_assert(*this, base_expr_raw);
	    NetExpr*base_expr = base_expr_raw->evaluate_function(*this, scope->loop_index_tmp);
	    if (! eval_as_long(base_off, base_expr)) {
		  ivl_assert(*this, 0);
	    }
	    ivl_assert(*this, base_off >= 0);

	    ivl_variable_type_t tmp_data_type = rsig->data_type();
	    netvector_t*tmp_type = new netvector_t(tmp_data_type, lsig_width-1,0);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, tmp_type);
	    tmp->local_flag(true);
	    tmp->set_line(*this);

	    NetPartSelect*ps = new NetPartSelect(tmp, base_off, lval_width, NetPartSelect::PV);
	    ps->set_line(*this);
	    des->add_node(ps);

	    connect(ps->pin(0), rsig->pin(0));
	    rsig = tmp;

      } else if (is_part_select) {
	      // In this case, there is no loop_index_tmp, so we are
	      // not within a NetForLoop. Generate a NetSubstitute
	      // object to handle the bit/part-select in the l-value.
	    ivl_assert(*this, scope->loop_index_tmp.empty());
	    ivl_assert(*this, lval_width < lsig_width);

	    const NetExpr*base_expr_raw = lval_->get_base();
	    ivl_assert(*this, base_expr_raw);
	    NetExpr*base_expr = base_expr_raw->evaluate_function(*this, scope->loop_index_tmp);
	    if (! eval_as_long(base_off, base_expr)) {
		  cerr << get_fileline() << ": sorry: assignment to variable "
			  "bit location is not currently supported in "
			  "synthesis." << endl;
		  des->errors += 1;
		  return false;
	    }
	    ivl_assert(*this, base_off >= 0);

	    ivl_variable_type_t tmp_data_type = rsig->data_type();
	    netvector_t*tmp_type = new netvector_t(tmp_data_type, lsig_width-1,0);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, tmp_type);
	    tmp->local_flag(true);
	    tmp->set_line(*this);

	    NetNet*isig = nex_out.pin(ptr).nexus()->pick_any_net();
	    if (isig) {
		  if (debug_synth2) {
			cerr << get_fileline() << ": NetAssignBase::synth_async: "
			     << " Found an isig:" << endl;
			nex_out.pin(ptr).dump_link(cerr, 8);
		  }
	    } else {
		  if (debug_synth2) {
			cerr << get_fileline() << ": NetAssignBase::synth_async: "
			     << " Found no isig, resorting to lsig." << endl;
		  }
		  isig = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, tmp_type);
		  isig->local_flag(true);
		  isig->set_line(*this);
		  connect(isig->pin(0), nex_out.pin(ptr));
	    }
	    ivl_assert(*this, isig);
	    NetSubstitute*ps = new NetSubstitute(isig, rsig, lsig_width, base_off);
	    ps->set_line(*this);
	    des->add_node(ps);

	    connect(ps->pin(0), tmp->pin(0));
	    rsig = tmp;
      }

      rsig = crop_to_width(des, rsig, lsig_width);

      ivl_assert(*this, rsig->pin_count()==1);
      nex_out.pin(ptr).unlink();
      enables.pin(ptr).unlink();
      connect(nex_out.pin(ptr), rsig->pin(0));
      connect(enables.pin(ptr), scope->tie_hi());

      mask_t&bitmask = bitmasks[ptr];
      if (is_part_select) {
	    if (bitmask.size() == 0) {
		  bitmask = mask_t (lsig_width, false);
	    }
	    ivl_assert(*this, bitmask.size() == lsig_width);
	    for (unsigned idx = 0; idx < lval_width; idx += 1) {
		  bitmask[base_off + idx] = true;
	    }
      } else if (bitmask.size() > 0) {
	    for (unsigned idx = 0; idx < bitmask.size(); idx += 1) {
		  bitmask[idx] = true;
	    }
      } else {
	    bitmask = mask_t (lsig_width, true);
      }

	/* This lval_ represents a reg that is a WIRE in the
	   synthesized results. This function signals the destructor
	   to change the REG that this l-value refers to into a
	   WIRE. It is done then, at the last minute, so that pending
	   synthesis can continue to work with it as a REG. */
      lval_->turn_sig_to_wire_on_release();

      return true;
}

bool NetProc::synth_async_block_substatement_(Design*des, NetScope*scope,
					      NexusSet&nex_map,
					      NetBus&nex_out,
					      NetBus&enables,
					      vector<mask_t>&bitmasks,
					      NetProc*substmt)
{
      ivl_assert(*this, nex_map.size() == nex_out.pin_count());
      ivl_assert(*this, nex_map.size() == enables.pin_count());
      ivl_assert(*this, nex_map.size() == bitmasks.size());

	// Create a temporary map of the output only from this statement.
      NexusSet tmp_map;
      substmt->nex_output(tmp_map);
      if (debug_synth2) {
	    cerr << get_fileline() << ": NetProc::synth_async_block_substatement_: "
		 << "tmp_map.size()==" << tmp_map.size()
		 << " for statement at " << substmt->get_fileline()
		 << endl;
	    for (unsigned idx  = 0 ; idx < nex_out.pin_count() ; idx += 1) {
		  cerr << get_fileline() << ": NetProc::synth_async_block_substatement_: "
		       << "incoming nex_out[" << idx << "] dump link" << endl;
		  nex_out.pin(idx).dump_link(cerr, 8);
	    }
      }

	// Create temporary variables to collect the output from the synthesis.
      NetBus tmp_out (scope, tmp_map.size());
      NetBus tmp_ena (scope, tmp_map.size());
      vector<mask_t> tmp_masks (tmp_map.size());

	// Map (and move) the accumulated nex_out for this block
	// to the version that we can pass to the next statement.
	// We will move the result back later.
      for (unsigned idx = 0 ; idx < tmp_out.pin_count() ; idx += 1) {
	    unsigned ptr = nex_map.find_nexus(tmp_map[idx]);
	    ivl_assert(*this, ptr < nex_out.pin_count());
	    connect(tmp_out.pin(idx), nex_out.pin(ptr));
	    nex_out.pin(ptr).unlink();
      }

      if (debug_synth2) {
	    for (unsigned idx = 0 ; idx < nex_map.size() ; idx += 1) {
		  cerr << get_fileline() << ": NetProc::synth_async_block_substatement_: nex_map[" << idx << "] dump link, base=" << nex_map[idx].base << ", wid=" << nex_map[idx].wid << endl;
		  nex_map[idx].lnk.dump_link(cerr, 8);
	     }
	    for (unsigned idx = 0 ; idx < tmp_map.size() ; idx += 1) {
		  cerr << get_fileline() << ": NetProc::synth_async_block_substatement_: tmp_map[" << idx << "] dump link, base=" << tmp_map[idx].base << ", wid=" << tmp_map[idx].wid << endl;
		  tmp_map[idx].lnk.dump_link(cerr, 8);
	     }
	    for (unsigned idx = 0 ; idx < tmp_out.pin_count() ; idx += 1) {
		  cerr << get_fileline() << ": NetProc::synth_async_block_substatement_: tmp_out[" << idx << "] dump link" << endl;
		  tmp_out.pin(idx).dump_link(cerr, 8);
	    }
      }


      bool flag = substmt->synth_async(des, scope, tmp_map, tmp_out, tmp_ena, tmp_masks);

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetProc::synth_async_block_substatement_: "
		  "substmt->synch_async(...) --> " << (flag? "true" : "false")
		 << " for statement at " << substmt->get_fileline() << "." << endl;
      }

      if (!flag) return false;

	// Now map the output from the substatement back to the
	// outputs for this block.
      for (unsigned idx = 0 ;  idx < tmp_out.pin_count() ;  idx += 1) {
	    unsigned ptr = nex_map.find_nexus(tmp_map[idx]);
	    ivl_assert(*this, ptr < nex_out.pin_count());
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetProc::synth_async_block_substatement_: "
		       << "tmp_out.pin(" << idx << "):" << endl;
		  tmp_out.pin(idx).dump_link(cerr, 8);
	    }
	    connect(nex_out.pin(ptr), tmp_out.pin(idx));

	    merge_sequential_enables(des, scope, enables.pin(ptr), tmp_ena.pin(idx));

	    merge_sequential_masks(bitmasks[ptr], tmp_masks[idx]);
      }

      return true;
}

/*
 * Sequential blocks are translated to asynchronous logic by
 * translating each statement of the block, in order, into gates.
 * The nex_out for the block is the union of the nex_out for all
 * the substatements.
 */
bool NetBlock::synth_async(Design*des, NetScope*scope,
			   NexusSet&nex_map, NetBus&nex_out,
			   NetBus&enables, vector<mask_t>&bitmasks)
{
      if (last_ == 0) {
	    return true;
      }

      bool flag = true;
      NetProc*cur = last_;
      do {
	    cur = cur->next_;

	    bool sub_flag = synth_async_block_substatement_(des, scope, nex_map, nex_out,
							    enables, bitmasks, cur);
	    flag = flag && sub_flag;

      } while (cur != last_);

      return flag;
}

/*
 * This function is used to fix up a MUX selector to be no longer than
 * it needs to be. The general idea is that if the selector needs to
 * be only N bits, but is actually M bits, we translate it to this:
 *
 *     osig = { |esig[M-1:N-1], esig[N-2:0] }
 *
 * This obviously implies that (N >= 2) and (M >= N). In the code
 * below, N is sel_need, and M is sel_got (= esig->vector_width()).
 */
static NetNet* mux_selector_reduce_width(Design*des, NetScope*scope,
					 const LineInfo&loc,
					 NetNet*esig, unsigned sel_need)
{
      const unsigned sel_got = esig->vector_width();

      ivl_assert(*esig, sel_got >= sel_need);

	// If the actual width matches the desired width (M==N) then
	// osig is esig itself. We're done.
      if (sel_got == sel_need)
	    return esig;

      if (debug_synth2) {
	    cerr << loc.get_fileline() << ": mux_selector_reduce_width: "
		 << "Reduce selector width=" << sel_got
		 << " to " << sel_need << " bits." << endl;
      }

      ivl_assert(*esig, sel_need >= 2);

	// This is the output signal, osig.
      ivl_variable_type_t osig_data_type = IVL_VT_LOGIC;
      netvector_t*osig_vec = new netvector_t(osig_data_type, sel_need-1, 0);
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::TRI, osig_vec);
      osig->local_flag(true);
      osig->set_line(loc);

	// Create the concat: osig = {...,...}
      NetConcat*osig_cat = new NetConcat(scope, scope->local_symbol(),
					 sel_need, 2, !disable_concatz_generation);
      osig_cat->set_line(loc);
      des->add_node(osig_cat);
      connect(osig_cat->pin(0), osig->pin(0));

	// Create the part select esig[N-2:0]...
      NetPartSelect*ps0 = new NetPartSelect(esig, 0, sel_need-1,
					    NetPartSelect::VP);
      ps0->set_line(loc);
      des->add_node(ps0);
      connect(ps0->pin(1), esig->pin(0));

      netvector_t*ps0_vec = new netvector_t(osig_data_type, sel_need-2, 0);
      NetNet*ps0_sig = new NetNet(scope, scope->local_symbol(),
				  NetNet::TRI, ps0_vec);
      ps0_sig->local_flag(true);
      ps0_sig->set_line(loc);
      connect(ps0_sig->pin(0), ps0->pin(0));

	// osig = {..., esig[N-2:0]}
      connect(osig_cat->pin(1), ps0_sig->pin(0));

	// Create the part select esig[M-1:N-1]
      NetPartSelect*ps1 = new NetPartSelect(esig, sel_need-1,
					    sel_got-sel_need+1,
					    NetPartSelect::VP);
      ps1->set_line(loc);
      des->add_node(ps1);
      connect(ps1->pin(1), esig->pin(0));

      netvector_t*ps1_vec = new netvector_t(osig_data_type, sel_got-sel_need, 0);
      NetNet*ps1_sig = new NetNet(scope, scope->local_symbol(),
				  NetNet::TRI, ps1_vec);
      ps1_sig->local_flag(true);
      ps1_sig->set_line(loc);
      connect(ps1_sig->pin(0), ps1->pin(0));

	// Create the reduction OR: | esig[M-1:N-1]
      NetUReduce*ered = new NetUReduce(scope, scope->local_symbol(),
				       NetUReduce::OR, sel_got-sel_need+1);
      ered->set_line(loc);
      des->add_node(ered);
      connect(ered->pin(1), ps1_sig->pin(0));

      NetNet*ered_sig = new NetNet(scope, scope->local_symbol(),
				   NetNet::TRI, &netvector_t::scalar_logic);
      ered_sig->local_flag(true);
      ered_sig->set_line(loc);
      connect(ered->pin(0), ered_sig->pin(0));

	// osig = { |esig[M-1:N-1], esig[N-2:0] }
      connect(osig_cat->pin(2), ered_sig->pin(0));

      return osig;
}

bool NetCase::synth_async(Design*des, NetScope*scope,
			  NexusSet&nex_map, NetBus&nex_out,
			  NetBus&enables, vector<mask_t>&bitmasks)
{
      if (type()==NetCase::EQZ || type()==NetCase::EQX)
	    return synth_async_casez_(des, scope, nex_map, nex_out,
				      enables, bitmasks);

	// Special case: If the case expression is constant, then this
	// is a pattern where the guards are non-constant and tested
	// against a constant case. Handle this as chained conditions
	// instead.
      if (dynamic_cast<NetEConst*> (expr_))
	    return synth_async_casez_(des, scope, nex_map, nex_out,
				      enables, bitmasks);

      ivl_assert(*this, nex_map.size() == nex_out.pin_count());
      ivl_assert(*this, nex_map.size() == enables.pin_count());
      ivl_assert(*this, nex_map.size() == bitmasks.size());

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetCase::synth_async: "
		 << "Selector expression: " << *expr_ << endl;
      }

	/* Synthesize the select expression. */
      NetNet*esig = expr_->synthesize(des, scope, expr_);

      unsigned sel_width = esig->vector_width();
      ivl_assert(*this, sel_width > 0);

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetCase::synth_async: "
		 << "selector width (sel_width) = " << sel_width << endl;
      }

      vector<unsigned> mux_width (nex_out.pin_count());
      for (unsigned idx = 0 ;  idx < nex_out.pin_count() ;  idx += 1) {
	    mux_width[idx] = nex_map[idx].wid;
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async: "
		       << "idx=" << idx
		       << ", mux_width[idx]=" << mux_width[idx] << endl;
	    }
      }

	// The incoming nex_out is taken as the input for this
	// statement. Since there are collection of statements
	// that start at this same point, we save all these
	// inputs and reuse them for each statement. Unlink the
	// nex_out now, so we can hook up the mux outputs.
      NetBus statement_input (scope, nex_out.pin_count());
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    connect(statement_input.pin(idx), nex_out.pin(idx));
	    nex_out.pin(idx).unlink();
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async: "
		       << "statement_input.pin(" << idx << "):" << endl;
		  statement_input.pin(idx).dump_link(cerr, 8);
	    }
      }

	/* Collect all the statements into a map of index to statement.
	   The guard expression it evaluated to be the index of the mux
	   value, and the statement is bound to that index. */

      unsigned long max_guard_value = 0;
      map<unsigned long,NetProc*>statement_map;
      NetProc*default_statement = 0;

      for (size_t item = 0 ;  item < items_.size() ;  item += 1) {
	    if (items_[item].guard == 0) {
		  default_statement = items_[item].statement;
		  continue;
	    }

	    NetEConst*ge = dynamic_cast<NetEConst*>(items_[item].guard);
	    if (ge == 0) {
		  cerr << items_[item].guard->get_fileline() << ": sorry: "
		       << "variable case item expressions with a variable "
		       << "case select expression are not supported in "
		       << "synthesis. " << endl;
		  des->errors += 1;
		  return false;
	    }
	    ivl_assert(*this, ge);
	    verinum gval = ge->value();

	    unsigned long sel_idx = gval.as_ulong();

	    if (statement_map[sel_idx]) {
		  cerr << ge->get_fileline() << ": warning: duplicate case "
		       << "value '" << sel_idx << "' detected. This case is "
		       << "unreachable." << endl;
		  delete items_[item].statement;
		  items_[item].statement = 0;
		  continue;
	    }

	    if (sel_idx > max_guard_value)
		  max_guard_value = sel_idx;

	    if (items_[item].statement) {
		  statement_map[sel_idx] = items_[item].statement;
		  continue;
	    }

	      // Handle the special case of an empty statement.
	    statement_map[sel_idx] = this;
      }

	// The minimum selector width is the number of inputs that
	// are selected, rounded up to the nearest power of 2.
      unsigned sel_need = max(ceil(log2(max_guard_value + 1)), 1.0);

	// If the sel_width can select more than just the explicit
	// guard values, and there is a default statement, then adjust
	// the sel_need to allow for the implicit selections.
      if (default_statement && (sel_width > sel_need))
	    sel_need += 1;

	// The mux size is always an exact power of 2.
      if (sel_need >= 8*sizeof(unsigned)) {
	   cerr << get_fileline() << ": sorry: mux select width of "
		<< sel_need << " bits is too large for synthesis." << endl;
	   des->errors += 1;
	   return false;
      }
      unsigned mux_size = 1U << sel_need;

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetCase::synth_async: "
		 << "Adjusted mux_size is " << mux_size
		 << " (max_guard_value=" << max_guard_value
		 << ", sel_need=" << sel_need
		 << ", sel_width=" << sel_width << ")." << endl;
      }

      if (sel_width > sel_need) {
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async: "
		       << "Selector is " << sel_width << " bits, "
		       << "need only " << sel_need << " bits." << endl;
	    }
	    esig = mux_selector_reduce_width(des, scope, *this, esig, sel_need);
      }

	/* If there is a default clause, synthesize it once and we'll
	   link it in wherever it is needed. If there isn't, create
	   a dummy default to pass on the accumulated nex_out from
	   preceding statements. */
      NetBus default_out (scope, nex_out.pin_count());
      NetBus default_ena (scope, nex_out.pin_count());
      vector<mask_t> default_masks (nex_out.pin_count());

      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    connect(default_out.pin(idx), statement_input.pin(idx));
	    connect(default_ena.pin(idx), scope->tie_lo());
      }

      if (default_statement) {

	    bool flag = synth_async_block_substatement_(des, scope, nex_map, default_out,
							default_ena, default_masks,
							default_statement);
	    if (!flag) return false;

	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async: "
		       << "synthesize default clause at " << default_statement->get_fileline()
		       << " is done." << endl;
	    }
      }

      vector<NetMux*> out_mux (nex_out.pin_count());
      vector<NetMux*> ena_mux (nex_out.pin_count());
      vector<bool>  full_case (nex_out.pin_count());
      for (size_t mdx = 0 ; mdx < nex_out.pin_count() ; mdx += 1) {
	    out_mux[mdx] = new NetMux(scope, scope->local_symbol(),
				      mux_width[mdx], mux_size, sel_need);
	    des->add_node(out_mux[mdx]);

	      // The select signal is already synthesized, and is
	      // common for every mux of this case statement. Simply
	      // hook it up.
	    connect(out_mux[mdx]->pin_Sel(), esig->pin(0));

	      // The outputs are in the nex_out, and connected to the
	      // mux Result pins.
	    connect(out_mux[mdx]->pin_Result(), nex_out.pin(mdx));

	      // Make sure the output is now connected to a net. If
	      // not, then create a fake one to carry the net-ness of
	      // the pin.
	    if (out_mux[mdx]->pin_Result().nexus()->pick_any_net() == 0) {
		  ivl_variable_type_t mux_data_type = IVL_VT_LOGIC;
		  netvector_t*tmp_vec = new netvector_t(mux_data_type, mux_width[mdx]-1,0);
		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::WIRE, tmp_vec);
		  tmp->local_flag(true);
		  ivl_assert(*this, tmp->vector_width() != 0);
		  connect(out_mux[mdx]->pin_Result(), tmp->pin(0));
	    }

	      // Create a mux for the enables, but don't hook it up
	      // until we know we need it.
	    ena_mux[mdx] = new NetMux(scope, scope->local_symbol(),
				      1, mux_size, sel_need);

	      // Assume a full case to start with. We'll check this as
	      // we synthesise each clause.
	    full_case[mdx] = true;
      }

      for (unsigned idx = 0 ;  idx < mux_size ;  idx += 1) {

	    NetProc*stmt = statement_map[idx];
	    if (stmt==0) {
		  ivl_assert(*this, default_out.pin_count() == out_mux.size());
		  for (unsigned mdx = 0 ; mdx < nex_out.pin_count() ; mdx += 1) {
			connect(out_mux[mdx]->pin_Data(idx), default_out.pin(mdx));
			connect(ena_mux[mdx]->pin_Data(idx), default_ena.pin(mdx));
			merge_parallel_masks(bitmasks[mdx], default_masks[mdx]);
			if (!default_ena.pin(mdx).is_linked(scope->tie_hi()))
			      full_case[mdx] = false;
		  }
		  continue;
	    }
	    ivl_assert(*this, stmt);
	    if (stmt == this) {
		    // Handle the special case of an empty statement.
		  ivl_assert(*this, statement_input.pin_count() == out_mux.size());
		  for (unsigned mdx = 0 ; mdx < nex_out.pin_count() ; mdx += 1) {
			connect(out_mux[mdx]->pin_Data(idx), statement_input.pin(mdx));
			connect(ena_mux[mdx]->pin_Data(idx), scope->tie_lo());
			bitmasks[mdx] = mask_t (mux_width[mdx], false);
			full_case[mdx] = false;
		  }
		  continue;
	    }

	    NetBus tmp_out (scope, nex_out.pin_count());
	    NetBus tmp_ena (scope, nex_out.pin_count());
	    for (unsigned mdx = 0 ; mdx < nex_out.pin_count() ; mdx += 1) {
		  connect(tmp_out.pin(mdx), statement_input.pin(mdx));
		  connect(tmp_ena.pin(mdx), scope->tie_lo());
	    }
	    vector<mask_t> tmp_masks (nex_out.pin_count());
	    bool flag = synth_async_block_substatement_(des, scope, nex_map, tmp_out,
							tmp_ena, tmp_masks, stmt);
	    if (!flag) return false;

	    for (size_t mdx = 0 ; mdx < nex_out.pin_count() ; mdx += 1) {
		  connect(out_mux[mdx]->pin_Data(idx), tmp_out.pin(mdx));
		  connect(ena_mux[mdx]->pin_Data(idx), tmp_ena.pin(mdx));
		  merge_parallel_masks(bitmasks[mdx], tmp_masks[mdx]);
		  if (!tmp_ena.pin(mdx).is_linked(scope->tie_hi()))
			full_case[mdx] = false;
	    }
      }

      for (unsigned mdx = 0 ; mdx < nex_out.pin_count() ; mdx += 1) {
	      // Optimize away the enable mux if we have a full case,
	      // otherwise hook it up.
	    if (full_case[mdx]) {
		  connect(enables.pin(mdx), scope->tie_hi());
		  delete ena_mux[mdx];
		  continue;
	    }

	    des->add_node(ena_mux[mdx]);

	    connect(ena_mux[mdx]->pin_Sel(), esig->pin(0));

	    connect(enables.pin(mdx), ena_mux[mdx]->pin_Result());

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, &netvector_t::scalar_logic);
	    tmp->local_flag(true);
	    connect(ena_mux[mdx]->pin_Result(), tmp->pin(0));
      }
      return true;
}

/*
 * casez statements are hard to implement as a single wide mux because
 * the test doesn't really map to a select input. Instead, implement
 * it as a chain of binary muxes. This gives the synthesizer more
 * flexibility, and is more typically what is desired from a casez anyhow.
 */
bool NetCase::synth_async_casez_(Design*des, NetScope*scope,
				 NexusSet&nex_map, NetBus&nex_out,
				 NetBus&enables, vector<mask_t>&bitmasks)
{
      ivl_assert(*this, nex_map.size() == nex_out.pin_count());
      ivl_assert(*this, nex_map.size() == enables.pin_count());
      ivl_assert(*this, nex_map.size() == bitmasks.size());

	/* Synthesize the select expression. */
      NetNet*esig = expr_->synthesize(des, scope, expr_);

      unsigned sel_width = esig->vector_width();
      ivl_assert(*this, sel_width > 0);

      vector<unsigned>mux_width (nex_out.pin_count());
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    mux_width[idx] = nex_map[idx].wid;
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async_casez_: "
		       << "idx=" << idx
		       << ", mux_width[idx]=" << mux_width[idx] << endl;
	    }
      }

	// The incoming nex_out is taken as the input for this
	// statement. Since there are collection of statements
	// that start at this same point, we save all these
	// inputs and reuse them for each statement. Unlink the
	// nex_out now, so we can hook up the mux outputs.
      NetBus statement_input (scope, nex_out.pin_count());
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    connect(statement_input.pin(idx), nex_out.pin(idx));
	    nex_out.pin(idx).unlink();
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async_casez_: "
		       << "statement_input.pin(" << idx << "):" << endl;
		  statement_input.pin(idx).dump_link(cerr, 8);
	    }

      }

	// Look for a default statement.
      NetProc*default_statement = 0;
      for (size_t item = 0 ; item < items_.size() ; item += 1) {
	    if (items_[item].guard != 0)
		  continue;

	    ivl_assert(*this, default_statement==0);
	    default_statement = items_[item].statement;
      }

	/* If there is a default clause, synthesize it once and we'll
	   link it in wherever it is needed. If there isn't, create
	   a dummy default to pass on the accumulated nex_out from
	   preceding statements. */
      NetBus default_out (scope, nex_out.pin_count());

      for (unsigned idx = 0 ; idx < default_out.pin_count() ; idx += 1)
	    connect(default_out.pin(idx), statement_input.pin(idx));

      if (default_statement) {
	    bool flag = synth_async_block_substatement_(des, scope, nex_map, default_out,
							enables, bitmasks, default_statement);
	    if (!flag) return false;

	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async_casez_: "
		       << "synthesize default clause at " << default_statement->get_fileline()
		       << " is done." << endl;
	    }
      }

      netvector_t*condit_type = new netvector_t(IVL_VT_LOGIC, 0, 0);

      NetCaseCmp::kind_t case_kind = NetCaseCmp::EEQ;
      switch (type()) {
	  case NetCase::EQ:
	    case_kind = NetCaseCmp::EEQ;
	    break;
	  case NetCase::EQX:
	    case_kind = NetCaseCmp::XEQ;
	    break;
	  case NetCase::EQZ:
	    case_kind = NetCaseCmp::ZEQ;
	    break;
	  default:
	    assert(0);
      }

	// Process the items from last to first. We generate a
	// true/false mux, with the select being the comparison of
	// the case select with the guard expression. The true input
	// (data1) is the current statement, and the false input is
	// the result of a later statement.
      vector<NetMux*>prev_mux (nex_out.pin_count());
      for (size_t idx = 0 ; idx < items_.size() ; idx += 1) {
	    size_t item = items_.size()-idx-1;
	    if (items_[item].guard == 0)
		  continue;

	    NetProc*stmt = items_[item].statement;
	    ivl_assert(*this, stmt);

	    NetExpr*guard_expr = items_[item].guard;
	    NetNet*guard = guard_expr->synthesize(des, scope, guard_expr);

	    NetCaseCmp*condit_dev = new NetCaseCmp(scope, scope->local_symbol(),
						   sel_width, case_kind);
	    des->add_node(condit_dev);
	    condit_dev->set_line(*this);
	      // Note that the expression that may have wildcards must
	      // go in the pin(2) input. This is the definition of the
	      // NetCaseCmp statement.
	    connect(condit_dev->pin(1), esig->pin(0));
	    connect(condit_dev->pin(2), guard->pin(0));

	    NetNet*condit = new NetNet(scope, scope->local_symbol(),
				       NetNet::TRI, condit_type);
	    condit->set_line(*this);
	    condit->local_flag(true);
	    connect(condit_dev->pin(0), condit->pin(0));

	      // Synthesize the guarded statement.
	    NetBus tmp_out (scope, nex_out.pin_count());
	    NetBus tmp_ena (scope, nex_out.pin_count());
	    vector<mask_t> tmp_masks (nex_out.pin_count());

	    for (unsigned pdx = 0 ; pdx < nex_out.pin_count() ; pdx += 1)
		  connect(tmp_out.pin(pdx), statement_input.pin(pdx));

	    synth_async_block_substatement_(des, scope, nex_map, tmp_out,
					    tmp_ena, tmp_masks, stmt);

	    NetBus prev_ena (scope, nex_out.pin_count());
	    for (unsigned mdx = 0 ; mdx < nex_out.pin_count() ; mdx += 1) {
		  NetMux*mux = new NetMux(scope, scope->local_symbol(),
					  mux_width[mdx], 2, 1);
		  des->add_node(mux);
		  mux->set_line(*this);
		  connect(mux->pin_Sel(), condit->pin(0));

		  connect(mux->pin_Data(1), tmp_out.pin(mdx));

		    // If there is a previous mux, then use that as the
		    // false clause input. Otherwise, use the default.
		  if (prev_mux[mdx])
			connect(mux->pin_Data(0), prev_mux[mdx]->pin_Result());
		  else
			connect(mux->pin_Data(0), default_out.pin(mdx));

		    // Make a NetNet for the result.
		  ivl_variable_type_t mux_data_type = IVL_VT_LOGIC;
		  netvector_t*tmp_vec = new netvector_t(mux_data_type, mux_width[mdx]-1,0);
		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::WIRE, tmp_vec);
		  tmp->local_flag(true);
		  tmp->set_line(*this);
		  ivl_assert(*this, tmp->vector_width() != 0);
		  connect(mux->pin_Result(), tmp->pin(0));

		    // This mux becomes the "false" input to the next mux.
		  prev_mux[mdx] = mux;

		  connect(prev_ena.pin(mdx), enables.pin(mdx));
		  enables.pin(mdx).unlink();

		  multiplex_enables(des, scope, condit, tmp_ena.pin(mdx),
				    prev_ena.pin(mdx), enables.pin(mdx));

		  merge_parallel_masks(bitmasks[mdx], tmp_masks[mdx]);
	    }
      }

	// Connect the last mux to the output.
      for (size_t mdx = 0 ; mdx < prev_mux.size() ; mdx += 1)
	    connect(prev_mux[mdx]->pin_Result(), nex_out.pin(mdx));

      return true;
}

/*
 * A condit statement (if (cond) ... else ... ;) infers an A-B mux,
 * with the cond expression acting as a select input. If the cond
 * expression is true, the if_ clause is selected, and if false, the
 * else_ clause is selected.
 */
bool NetCondit::synth_async(Design*des, NetScope*scope,
			    NexusSet&nex_map, NetBus&nex_out,
			    NetBus&enables, vector<mask_t>&bitmasks)
{
	// Handle the unlikely case that both clauses are empty.
      if ((if_ == 0) && (else_ == 0))
	    return true;

      ivl_assert(*this, nex_map.size() == nex_out.pin_count());
      ivl_assert(*this, nex_map.size() == enables.pin_count());
      ivl_assert(*this, nex_map.size() == bitmasks.size());

	// Synthesize the condition. This will act as a select signal
	// for a binary mux.
      NetNet*ssig = expr_->synthesize(des, scope, expr_);
      ivl_assert(*this, ssig);

	// The incoming nex_out is taken as the input for this
	// statement. Since there are two statements that start
	// at this same point, we save all these inputs and reuse
	// them for both statements. Unlink the nex_out now, so
	// we can hook up the mux outputs.
      NetBus statement_input (scope, nex_out.pin_count());
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    connect(statement_input.pin(idx), nex_out.pin(idx));
	    nex_out.pin(idx).unlink();
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "statement_input.pin(" << idx << "):" << endl;
		  statement_input.pin(idx).dump_link(cerr, 8);
	    }
      }

      NetBus a_out (scope, nex_out.pin_count());
      NetBus a_ena (scope, nex_out.pin_count());
      vector<mask_t> a_masks (nex_out.pin_count());
      if (if_) {
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "Synthesize if clause at " << if_->get_fileline()
		       << endl;
	    }

	    for (unsigned idx = 0 ; idx < a_out.pin_count() ; idx += 1) {
		  connect(a_out.pin(idx), statement_input.pin(idx));
	    }

	    bool flag = synth_async_block_substatement_(des, scope, nex_map, a_out,
							a_ena, a_masks, if_);
	    if (!flag) return false;

      } else {
	    for (unsigned idx = 0 ; idx < a_out.pin_count() ; idx += 1) {
		  connect(a_out.pin(idx), statement_input.pin(idx));
		  connect(a_ena.pin(idx), scope->tie_lo());
	    }
      }

      NetBus b_out(scope, nex_out.pin_count());
      NetBus b_ena(scope, nex_out.pin_count());
      vector<mask_t> b_masks (nex_out.pin_count());
      if (else_) {
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "Synthesize else clause at " << else_->get_fileline()
		       << endl;
	    }

	    for (unsigned idx = 0 ; idx < b_out.pin_count() ; idx += 1) {
		  connect(b_out.pin(idx), statement_input.pin(idx));
	    }

	    bool flag = synth_async_block_substatement_(des, scope, nex_map, b_out,
							b_ena, b_masks, else_);
	    if (!flag) return false;

      } else {
	    for (unsigned idx = 0 ; idx < b_out.pin_count() ; idx += 1) {
		  connect(b_out.pin(idx), statement_input.pin(idx));
		  connect(b_ena.pin(idx), scope->tie_lo());
	    }
      }

	/* The nex_out output, a_out input, and b_out input all have the
	   same pin count (usually, but not always 1) because they are
	   net arrays of the same dimension. The for loop below creates
	   a NetMux for each pin of the output. (Note that pins may
	   be, in fact usually are, vectors.) */

      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {

	    bool a_driven = a_out.pin(idx).nexus()->pick_any_net();
	    bool b_driven = b_out.pin(idx).nexus()->pick_any_net();
	    if (!a_driven && !b_driven) {
		  connect(nex_out.pin(idx), statement_input.pin(idx));
		  continue;
	    }

	    merge_parallel_masks(bitmasks[idx], a_masks[idx]);
	    merge_parallel_masks(bitmasks[idx], b_masks[idx]);

	      // If one clause is empty and the other clause unconditionally
	      // drives all bits of the vector, we can rely on the enable
	      // to prevent the flip-flop or latch updating when the empty
	      // clause is selected, and hence don't need a mux.
	    if (!a_driven && all_bits_driven(b_masks[idx])) {
		  connect(nex_out.pin(idx), b_out.pin(idx));
		  continue;
	    }
	    if (!b_driven && all_bits_driven(a_masks[idx])) {
		  connect(nex_out.pin(idx), a_out.pin(idx));
		  continue;
	    }

	      // Guess the mux type from the type of the output.
	    ivl_variable_type_t mux_data_type = IVL_VT_LOGIC;
	    if (NetNet*tmp = nex_out.pin(idx).nexus()->pick_any_net()) {
		  mux_data_type = tmp->data_type();
	    }

	    unsigned mux_off = 0;
	    unsigned mux_width = nex_map[idx].wid;

	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "Calculated mux_width=" << mux_width
		       << endl;
	    }

	    NetPartSelect*apv = detect_partselect_lval(a_out.pin(idx));
	    if (debug_synth2 && apv) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "Assign-to-part apv base=" << apv->base()
		       << ", width=" << apv->width() << endl;
	    }

	    NetPartSelect*bpv = detect_partselect_lval(b_out.pin(idx));
	    if (debug_synth2 && bpv) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "Assign-to-part bpv base=" << bpv->base()
		       << ", width=" << bpv->width() << endl;
	    }

	    unsigned mux_lwidth = mux_width;
	    ivl_assert(*this, mux_width != 0);

	    if (apv && bpv && apv->width()==bpv->width() && apv->base()==bpv->base()) {
		    // The a and b sides are both assigning to the
		    // same bits of the output, so we can use that to
		    // create a much narrower mux that only
		    // manipulates the width of the part.
		  mux_width = apv->width();
		  mux_off = apv->base();
		  a_out.pin(idx).unlink();
		  b_out.pin(idx).unlink();
		  connect(a_out.pin(idx), apv->pin(0));
		  connect(b_out.pin(idx), bpv->pin(0));
		  delete apv;
		  delete bpv;
	    } else {
		    // The part selects are of no use. Forget them.
		  if (apv) delete apv;
		  if (bpv) delete bpv;
	    }

	    NetMux*mux = new NetMux(scope, scope->local_symbol(),
				    mux_width, 2, 1);
	    mux->set_line(*this);
	    des->add_node(mux);

	    netvector_t*tmp_type = 0;
	    if (mux_width==1)
		  tmp_type = new netvector_t(mux_data_type);
	    else
		  tmp_type = new netvector_t(mux_data_type, mux_width-1,0);

	      // Bind some temporary signals to carry pin type.
	    NetNet*otmp = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, tmp_type);
	    otmp->local_flag(true);
	    otmp->set_line(*this);
	    connect(mux->pin_Result(),otmp->pin(0));

	    connect(mux->pin_Sel(),   ssig->pin(0));
	    connect(mux->pin_Data(1), a_out.pin(idx));
	    connect(mux->pin_Data(0), b_out.pin(idx));

	      // If we are only muxing a part of the output vector, make a
	      // NetSubstitute to blend the mux output with the accumulated
	      // output from previous statements.
	    if (mux_width < mux_lwidth) {
		  tmp_type = new netvector_t(mux_data_type, mux_lwidth-1,0);

		  NetNet*itmp = statement_input.pin(idx).nexus()->pick_any_net();
		  if (itmp == 0) {
			itmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::WIRE, tmp_type);
			itmp->local_flag(true);
			itmp->set_line(*this);
			connect(itmp->pin(0), statement_input.pin(idx));
		  }

		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::WIRE, tmp_type);
		  tmp->local_flag(true);
		  tmp->set_line(*this);
		  NetSubstitute*ps = new NetSubstitute(itmp, otmp, mux_lwidth, mux_off);
		  des->add_node(ps);
		  connect(ps->pin(0), tmp->pin(0));
		  otmp = tmp;
	    }

	    connect(nex_out.pin(idx), otmp->pin(0));
      }

      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    multiplex_enables(des, scope, ssig, a_ena.pin(idx), b_ena.pin(idx), enables.pin(idx));
      }

      return true;
}

bool NetEvWait::synth_async(Design*des, NetScope*scope,
			    NexusSet&nex_map, NetBus&nex_out,
			    NetBus&enables, vector<mask_t>&bitmasks)
{
      bool flag = statement_->synth_async(des, scope, nex_map, nex_out, enables, bitmasks);
      return flag;
}

bool NetForLoop::synth_async(Design*des, NetScope*scope,
			     NexusSet&nex_map, NetBus&nex_out,
			     NetBus&enables, vector<mask_t>&bitmasks)
{
      if (!index_) {
	    cerr << get_fileline() << ": sorry: Unable to synthesize for-loop without explicit index variable." << endl;
	    return false;
      }

      if (!step_statement_) {
	    cerr << get_fileline() << ": sorry: Unable to synthesize for-loop without for_step statement." << endl;
	    return false;
      }

      ivl_assert(*this, index_ && init_expr_);
      if (debug_synth2) {
	    cerr << get_fileline() << ": NetForLoop::synth_async: "
		 << "Index variable is " << index_->name() << endl;
	    cerr << get_fileline() << ": NetForLoop::synth_async: "
		 << "Initialization expression: " << *init_expr_ << endl;
      }

	// Get the step assignment statement and break it into the
	// l-value (should be the index) and the r-value, which is the
	// step expressions.
      NetAssign*step_assign = dynamic_cast<NetAssign*> (step_statement_);
      char assign_operator = step_assign->assign_operator();
      ivl_assert(*this, step_assign);
      NetExpr*step_expr = step_assign->rval();

	// Tell the scope that this index value is like a genvar.
      LocalVar index_var;
      index_var.nwords = 0;

      map<perm_string,LocalVar> index_args;

	// Calculate the initial value for the index.
      index_var.value = init_expr_->evaluate_function(*this, index_args);
      ivl_assert(*this, index_var.value);
      index_args[index_->name()] = index_var;

      for (;;) {
	      // Evaluate the condition expression. If it is false,
	      // then we are going to break out of this synthesis loop.
	    NetExpr*tmp = condition_->evaluate_function(*this, index_args);
	    ivl_assert(*this, tmp);

	    long cond_value;
	    bool rc = eval_as_long(cond_value, tmp);
	    ivl_assert(*this, rc);
	    delete tmp;
	    if (!cond_value) break;

	    scope->genvar_tmp = index_->name();
	    rc = eval_as_long(scope->genvar_tmp_val, index_var.value);
	    ivl_assert(*this, rc);

	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetForLoop::synth_async: "
		       << "Synthesis iteration with " << index_->name()
		       << "=" << *index_var.value << endl;
	    }

	      // Synthesize the iterated expression. Stash the loop
	      // index value so that the substatements can see this
	      // value and use it during its own synthesis.
	    ivl_assert(*this, scope->loop_index_tmp.empty());
	    scope->loop_index_tmp = index_args;

	    NetBus tmp_ena (scope, nex_out.pin_count());
	    vector<mask_t> tmp_masks (nex_out.pin_count());

	    rc = synth_async_block_substatement_(des, scope, nex_map, nex_out,
						 tmp_ena, tmp_masks, statement_);

	    for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
		  merge_sequential_enables(des, scope, enables.pin(idx), tmp_ena.pin(idx));
		  merge_sequential_masks(bitmasks[idx], tmp_masks[idx]);
	    }

	    scope->loop_index_tmp.clear();

	      // Evaluate the step_expr to generate the next index value.
	    tmp = step_expr->evaluate_function(*this, index_args);
	    ivl_assert(*this, tmp);

	      // If there is an assign_operator, then replace the
	      // index_var.value with (value <op> tmp) and evaluate
	      // that to get the next value. "value" is the existing
	      // value, and "tmp" is the step value. We are replacing
	      // (value += tmp) with (value = value + tmp) and
	      // evaluating it.
	    switch (assign_operator) {
		case 0:
		  break;
		case '+':
		case '-':
		  index_var.value = new NetEBAdd(assign_operator, tmp, index_var.value, 32, true);
		  tmp = index_var.value->evaluate_function(*this, index_args);
		  break;

		default:
		  cerr << get_fileline() << ": internal error: "
		       << "NetForLoop::synth_async: What to do with assign_operator=" << assign_operator << endl;
		  ivl_assert(*this, 0);
	    }
	    delete index_var.value;
	    index_var.value = tmp;
	    index_args[index_->name()] = index_var;
      }

      delete index_var.value;

      return true;
}

/*
 * This method is called when the process is shown to be
 * asynchronous. Figure out the nexus set of outputs from this
 * process, and pass that to the synth_async method for the statement
 * of the process. The statement will connect its output to the
 * nex_out set, using the nex_map as a guide. Starting from the top,
 * the nex_map is the same as the nex_map.
 */
bool NetProcTop::synth_async(Design*des)
{
      NexusSet nex_set;
      statement_->nex_output(nex_set);

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetProcTop::synth_async: "
		 << "Process has " << nex_set.size() << " outputs." << endl;
      }

      NetBus nex_out (scope(), nex_set.size());
      NetBus enables (scope(), nex_set.size());
      vector<NetProc::mask_t> bitmasks (nex_set.size());

	// Save links to the initial nex_out. These will be used later
	// to detect floating part-substitute and mux inputs that need
	// to be tied off.
      NetBus nex_in (scope(), nex_out.pin_count());
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1)
	    connect(nex_in.pin(idx), nex_out.pin(idx));

      bool flag = statement_->synth_async(des, scope(), nex_set, nex_out, enables, bitmasks);
      if (!flag) return false;

      flag = tie_off_floating_inputs_(des, nex_set, nex_in, bitmasks, false);
      if (!flag) return false;

      for (unsigned idx = 0 ;  idx < nex_set.size() ;  idx += 1) {

	    if (enables.pin(idx).is_linked(scope()->tie_hi())) {
		  connect(nex_set[idx].lnk, nex_out.pin(idx));
	    } else {
		  cerr << get_fileline() << ": warning: "
		       << "A latch has been inferred for '"
		       << nex_set[idx].lnk.nexus()->pick_any_net()->name()
		       << "'." << endl;

		  if (enables.pin(idx).nexus()->pick_any_net()->local_flag()) {
			cerr << get_fileline() << ": warning: The latch "
			        "enable is connected to a synthesized "
			        "expression. The latch may be sensitive "
			        "to glitches." << endl;
		  }

		  if (debug_synth2) {
			cerr << get_fileline() << ": debug: "
			     << "Top level making a "
			     << nex_set[idx].wid << "-wide "
			     << "NetLatch device." << endl;
		  }

		  NetLatch*latch = new NetLatch(scope(), scope()->local_symbol(),
						nex_set[idx].wid);
		  des->add_node(latch);
		  latch->set_line(*this);

		  NetNet*tmp = nex_out.pin(idx).nexus()->pick_any_net();
		  tmp->set_line(*this);
		  assert(tmp);

		  tmp = crop_to_width(des, tmp, latch->width());

		  connect(nex_set[idx].lnk, latch->pin_Q());
		  connect(tmp->pin(0), latch->pin_Data());

		  assert (enables.pin(idx).is_linked());
		  connect(enables.pin(idx), latch->pin_Enable());
	    }
      }

      synthesized_design_ = des;
      return true;
}


bool NetProc::synth_sync(Design*des, NetScope*scope,
			 bool& /* ff_negedge */,
			 NetNet* /* ff_clk */, NetBus&ff_ce,
			 NetBus& /* ff_aclr*/, NetBus& /* ff_aset*/,
			 vector<verinum>& /*ff_aset_value*/,
			 NexusSet&nex_map, NetBus&nex_out,
			 vector<mask_t>&bitmasks,
			 const vector<NetEvProbe*>&events)
{
      if (events.size() > 0) {
	    cerr << get_fileline() << ": error: Events are unaccounted"
		 << " for in process synthesis." << endl;
	    des->errors += 1;
      }

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetProc::synth_sync: "
		 << "This statement is an async input to a sync process." << endl;
      }

	/* Synthesize the input to the DFF. */
      return synth_async(des, scope, nex_map, nex_out, ff_ce, bitmasks);
}

/*
 * This method is called when a block is encountered near the surface
 * of a synchronous always statement. For example, this code will be
 * invoked for input like this:
 *
 *     always @(posedge clk...) begin
 *	     <statement1>
 *	     <statement2>
 *	     ...
 *     end
 *
 * This needs to be split into a DFF bank for each statement, because
 * the statements may each infer different reset and enables signals.
 */
bool NetBlock::synth_sync(Design*des, NetScope*scope,
			  bool&ff_negedge,
			  NetNet*ff_clk, NetBus&ff_ce,
			  NetBus&ff_aclr,NetBus&ff_aset,
			  vector<verinum>&ff_aset_value,
			  NexusSet&nex_map, NetBus&nex_out,
			  vector<mask_t>&bitmasks,
			  const vector<NetEvProbe*>&events_in)
{
      if (debug_synth2) {
	    cerr << get_fileline() << ": NetBlock::synth_sync: "
		 << "Examine this block for synchronous logic." << endl;
      }

      if (last_ == 0) {
	    return true;
      }

      bool flag = true;

      NetProc*cur = last_;
      do {
	    cur = cur->next_;

	      // Create a temporary nex_map for the substatement.
	    NexusSet tmp_map;
	    cur->nex_output(tmp_map);

	      // Create temporary variables to collect the output from the synthesis.
	    NetBus tmp_out (scope, tmp_map.size());
	    NetBus tmp_ce  (scope, tmp_map.size());
	    vector<mask_t> tmp_masks (tmp_map.size());

	      // Map (and move) the accumulated nex_out for this block
	      // to the version that we can pass to the next statement.
	      // We will move the result back later.
	    for (unsigned idx = 0 ; idx < tmp_out.pin_count() ; idx += 1) {
		  unsigned ptr = nex_map.find_nexus(tmp_map[idx]);
		  ivl_assert(*this, ptr < nex_out.pin_count());
		  connect(tmp_out.pin(idx), nex_out.pin(ptr));
		  nex_out.pin(ptr).unlink();
	    }

	      /* Now go on with the synchronous synthesis for this
		 subset of the statement. The tmp_map is the output
		 nexa that we expect, and the tmp_out is where we want
		 those outputs connected. */
	    bool ok_flag = cur->synth_sync(des, scope,
					   ff_negedge, ff_clk, tmp_ce,
					   ff_aclr, ff_aset, ff_aset_value,
					   tmp_map, tmp_out, tmp_masks,
					   events_in);
	    flag = flag && ok_flag;

	    if (ok_flag == false)
		  continue;

	      // Now map the output from the substatement back to the
	      // outputs for this block.
	    for (unsigned idx = 0 ;  idx < tmp_out.pin_count() ; idx += 1) {
		  unsigned ptr = nex_map.find_nexus(tmp_map[idx]);
		  ivl_assert(*this, ptr < nex_out.pin_count());
		  connect(nex_out.pin(ptr), tmp_out.pin(idx));

		  merge_sequential_enables(des, scope, ff_ce.pin(ptr), tmp_ce.pin(idx));

		  merge_sequential_masks(bitmasks[ptr], tmp_masks[idx]);
	    }

      } while (cur != last_);

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetBlock::synth_sync: "
		 << "Done Examining this block for synchronous logic." << endl;
      }

      return flag;
}

/*
 * This method handles the case where I find a conditional near the
 * surface of a synchronous thread. This conditional can be a CE or an
 * asynchronous set/reset, depending on whether the pin of the
 * expression is connected to an event, or not.
 */
bool NetCondit::synth_sync(Design*des, NetScope*scope,
			   bool&ff_negedge,
			   NetNet*ff_clk, NetBus&ff_ce,
			   NetBus&ff_aclr,NetBus&ff_aset,
			   vector<verinum>&ff_aset_value,
			   NexusSet&nex_map, NetBus&nex_out,
			   vector<mask_t>&bitmasks,
			   const vector<NetEvProbe*>&events_in)
{
	/* First try to turn the condition expression into an
	   asynchronous set/reset. If the condition expression has
	   inputs that are included in the sensitivity list, then it
	   is likely intended as an asynchronous input. */

      NexusSet*expr_input = expr_->nex_input();
      assert(expr_input);
      for (unsigned idx = 0 ;  idx < events_in.size() ;  idx += 1) {

	    NetEvProbe*ev = events_in[idx];
	    NexusSet pin_set;
	    pin_set.add(ev->pin(0).nexus(), 0, 0);

	    if (! expr_input->contains(pin_set))
		  continue;

	      // Synthesize the set/reset input expression.
	    NetNet*rst = expr_->synthesize(des, scope, expr_);
	    ivl_assert(*this, rst->pin_count() == 1);

	      // Check that the edge used on the set/reset input is correct.
	    switch (ev->edge()) {
	      case NetEvProbe::POSEDGE:
		  if (ev->pin(0).nexus() != rst->pin(0).nexus()) {
			cerr << get_fileline() << ": error: "
			     << "Condition for posedge asynchronous set/reset "
			     << "must exactly match the event expression." << endl;
			des->errors += 1;
			return false;
		  }
		  break;
	      case NetEvProbe::NEGEDGE: {
		  bool is_inverter = false;
		  NetNode*node = rst->pin(0).nexus()->pick_any_node();
		  if (NetLogic*gate = dynamic_cast<NetLogic*>(node)) {
			if (gate->type() == NetLogic::NOT)
				is_inverter = true;
		  }
		  if (NetUReduce*gate = dynamic_cast<NetUReduce*>(node)) {
			if (gate->type() == NetUReduce::NOR)
				is_inverter = true;
		  }
		  if (!is_inverter || ev->pin(0).nexus() != node->pin(1).nexus()) {
			cerr << get_fileline() << ": error: "
			     << "Condition for negedge asynchronous set/reset must be "
			     << "a simple inversion of the event expression." << endl;
			des->errors += 1;
			return false;
		  }
		  break;
	      }
	      default:
		  cerr << get_fileline() << ": error: "
		       << "Asynchronous set/reset event must be "
		       << "edge triggered." << endl;
		  des->errors += 1;
		  return false;
	    }

	      // Synthesize the true clause to figure out what kind of
	      // set/reset we have. This should synthesize down to a
	      // constant. If not, we have an asynchronous LOAD, a
	      // very different beast.
	    ivl_assert(*this, if_);
	    NetBus tmp_out(scope, nex_out.pin_count());
	    NetBus tmp_ena(scope, nex_out.pin_count());
	    vector<mask_t> tmp_masks (nex_out.pin_count());
	    bool flag = if_->synth_async(des, scope, nex_map, tmp_out, tmp_ena, tmp_masks);
	    if (!flag) return false;

	    ivl_assert(*this, tmp_out.pin_count() == ff_aclr.pin_count());
	    ivl_assert(*this, tmp_out.pin_count() == ff_aset.pin_count());

	    for (unsigned pin = 0 ; pin < tmp_out.pin_count() ; pin += 1) {
		  Nexus*rst_nex = tmp_out.pin(pin).nexus();

		  if (!all_bits_driven(tmp_masks[pin])) {
			cerr << get_fileline() << ": sorry: Not all bits of '"
			     << nex_map[pin].lnk.nexus()->pick_any_net()->name()
			     << "' are asynchronously set or reset. This is "
			     << "not currently supported in synthesis." << endl;
			des->errors += 1;
			return false;
		  }

		  if (! rst_nex->drivers_constant() ||
		      ! tmp_ena.pin(pin).is_linked(scope->tie_hi()) ) {
			cerr << get_fileline() << ": sorry: Asynchronous load "
			     << "is not currently supported in synthesis." << endl;
			des->errors += 1;
			return false;
		  }

		  if (ff_aclr.pin(pin).is_linked() ||
		      ff_aset.pin(pin).is_linked()) {
			cerr << get_fileline() << ": sorry: More than "
				"one asynchronous set/reset clause is "
				"not currently supported in synthesis." << endl;
			des->errors += 1;
			return false;
		  }

		  verinum rst_drv = rst_nex->driven_vector();

		  verinum zero (verinum::V0, rst_drv.len());
		  verinum ones (verinum::V1, rst_drv.len());

		  if (rst_drv==zero) {
			  // Don't yet support multiple asynchronous reset inputs.
			ivl_assert(*this, ! ff_aclr.pin(pin).is_linked());

			ivl_assert(*this, rst->pin_count()==1);
			connect(ff_aclr.pin(pin), rst->pin(0));

		  } else {
			  // Don't yet support multiple asynchronous set inputs.
			ivl_assert(*this, ! ff_aset.pin(pin).is_linked());

			ivl_assert(*this, rst->pin_count()==1);
			connect(ff_aset.pin(pin), rst->pin(0));
			if (rst_drv!=ones)
			      ff_aset_value[pin] = rst_drv;
		  }
	    }

	    if (else_ == 0)
		  return true;

	    vector<NetEvProbe*> events;
	    for (unsigned jdx = 0 ;  jdx < events_in.size() ;  jdx += 1) {
		  if (jdx != idx)
			events.push_back(events_in[jdx]);
	    }
	    return else_->synth_sync(des, scope,
				     ff_negedge, ff_clk, ff_ce,
				     ff_aclr, ff_aset, ff_aset_value,
				     nex_map, nex_out, bitmasks, events);
      }

      delete expr_input;

#if 0
	/* Detect the case that this is a *synchronous* set/reset. It
	   is not asynchronous because we know the condition is not
	   included in the sensitivity list, but if the if_ case is
	   constant (has no inputs) then we can model this as a
	   synchronous set/reset.

	   This is only synchronous set/reset if there is a true and a
	   false clause, and no inputs. The "no inputs" requirement is
	   met if the assignments are of all constant values. */
      assert(if_ != 0);
      NexusSet*a_set = if_->nex_input();

      if ((a_set->count() == 0) && if_ && else_) {

	    NetNet*rst = expr_->synthesize(des);
	    assert(rst->pin_count() == 1);

	      /* Synthesize the true clause to figure out what
		 kind of set/reset we have. */
	    NetNet*asig = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, nex_map->pin_count());
	    asig->local_flag(true);
	    bool flag = if_->synth_async(des, scope, nex_map, asig);

	    if (!flag) {
		  /* This path leads nowhere */
		  delete asig;
	    } else {
		  assert(asig->pin_count() == ff->width());

		    /* Collect the set/reset value into a verinum. If
		       this turns out to be entirely 0 values, then
		       use the Sclr input. Otherwise, use the Aset
		       input and save the set value. */
		  verinum tmp (verinum::V0, ff->width());
		  for (unsigned bit = 0 ;  bit < ff->width() ;	bit += 1) {

			assert(asig->pin(bit).nexus()->drivers_constant());
			tmp.set(bit, asig->pin(bit).nexus()->driven_value());
		  }

		  assert(tmp.is_defined());
		  if (tmp.is_zero()) {
			connect(ff->pin_Sclr(), rst->pin(0));

		  } else {
			connect(ff->pin_Sset(), rst->pin(0));
			ff->sset_value(tmp);
		  }

		  delete a_set;

		  assert(else_ != 0);
		  flag = else_->synth_sync(des, scope, ff, nex_map,
					   nex_out, std::vector<NetEvProbe*>())
			&& flag;
		  DEBUG_SYNTH2_EXIT("NetCondit",flag)
		  return flag;
	    }
      }

      delete a_set;
#endif

#if 0
	/* This gives a false positive for strange coding styles,
	   such as ivltests/conditsynth3.v. */

	/* Failed to find an asynchronous set/reset, so any events
	   input are probably in error. */
      if (events_in.size() > 0) {
	    cerr << get_fileline() << ": error: Events are unaccounted"
		 << " for in process synthesis." << endl;
	    des->errors += 1;
      }
#endif

      return synth_async(des, scope, nex_map, nex_out, ff_ce, bitmasks);
}

bool NetEvWait::synth_sync(Design*des, NetScope*scope,
			   bool&ff_negedge,
			   NetNet*ff_clk, NetBus&ff_ce,
			   NetBus&ff_aclr,NetBus&ff_aset,
			   vector<verinum>&ff_aset_value,
			   NexusSet&nex_map, NetBus&nex_out,
			   vector<mask_t>&bitmasks,
			   const vector<NetEvProbe*>&events_in)
{
      if (debug_synth2) {
	    cerr << get_fileline() << ": NetEvWait::synth_sync: "
		 << "Synchronous process an event statement." << endl;
      }

      if (events_in.size() > 0) {
	    cerr << get_fileline() << ": error: Events are unaccounted"
		 << " for in process synthesis." << endl;
	    des->errors += 1;
      }

      assert(events_in.size() == 0);

	/* This can't be other than one unless there are named events,
	   which I cannot synthesize. */
      ivl_assert(*this, events_.size() == 1);
      NetEvent*ev = events_[0];

      assert(ev->nprobe() >= 1);
      vector<NetEvProbe*>events (ev->nprobe() - 1);

	/* Get the input set from the substatement. This will be used
	   to figure out which of the probes is the clock. */
      NexusSet*statement_input = statement_ -> nex_input();

	/* Search for a clock input. The clock input is the edge event
	   that is not also an input to the substatement. */
      NetEvProbe*pclk = 0;
      unsigned event_idx = 0;
      for (unsigned idx = 0 ;  idx < ev->nprobe() ;  idx += 1) {
	    NetEvProbe*tmp = ev->probe(idx);
	    assert(tmp->pin_count() == 1);

	    NexusSet tmp_nex;
	    tmp_nex .add( tmp->pin(0).nexus(), 0, 0 );

	    if (! statement_input ->contains(tmp_nex)) {
		  if (pclk != 0) {
			cerr << get_fileline() << ": error: Too many "
			     << "clocks for synchronous logic." << endl;
			cerr << get_fileline() << ":	  : Perhaps an"
			     << " asynchronous set/reset is misused?" << endl;
			des->errors += 1;
		  }
		  pclk = tmp;

	    } else {
		  events[event_idx++] = tmp;
	    }
      }

      if (pclk == 0) {
	    cerr << get_fileline() << ": error: None of the edges"
		 << " are valid clock inputs." << endl;
	    cerr << get_fileline() << ":      : Perhaps the clock"
		 << " is read by a statement or expression?" << endl;
	    des->errors += 1;
	    return false;
      }

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetEvWait::synth_sync: "
		 << "Found and synthesized the FF clock." << endl;
      }

      connect(ff_clk->pin(0), pclk->pin(0));
      if (pclk->edge() == NetEvProbe::NEGEDGE) {
	    ff_negedge = true;

	    if (debug_synth2) {
		  cerr << get_fileline() << ": debug: "
		       << "Detected a NEGEDGE clock for the synthesized ff."
		       << endl;
	    }
      }

	/* Synthesize the input to the DFF. */
      return statement_->synth_sync(des, scope,
				    ff_negedge, ff_clk, ff_ce,
				    ff_aclr, ff_aset, ff_aset_value,
				    nex_map, nex_out, bitmasks, events);
}

/*
 * This method is called for a process that is determined to be
 * synchronous. Create a NetFF device to hold the output from the
 * statement, and synthesize that statement in place.
 */
bool NetProcTop::synth_sync(Design*des)
{
      if (debug_synth2) {
	    cerr << get_fileline() << ": NetProcTop::synth_sync: "
		 << "Process is apparently synchronous. Making NetFFs."
		 << endl;
      }

      NexusSet nex_set;
      statement_->nex_output(nex_set);
      vector<verinum> aset_value(nex_set.size());

	/* Make a model FF that will connect to the first item in the
	   set, and will also take the initial connection of clocks
	   and resets. */

	// Create a net to carry the clock for the synthesized FFs.
      NetNet*clock = new NetNet(scope(), scope()->local_symbol(),
				NetNet::TRI, &netvector_t::scalar_logic);
      clock->local_flag(true);
      clock->set_line(*this);

      NetBus ce    (scope(), nex_set.size());
      NetBus nex_d (scope(), nex_set.size());
      NetBus nex_q (scope(), nex_set.size());
      NetBus aclr  (scope(), nex_set.size());
      NetBus aset  (scope(), nex_set.size());
      vector<NetProc::mask_t> bitmasks (nex_set.size());

	// Save links to the initial nex_d. These will be used later
	// to detect floating part-substitute and mux inputs that need
	// to be tied off.
      NetBus nex_in (scope(), nex_d.pin_count());
      for (unsigned idx = 0 ; idx < nex_in.pin_count() ; idx += 1)
	    connect(nex_in.pin(idx), nex_d.pin(idx));

	// The Q of the NetFF devices is connected to the output that
	// we are. The nex_q is a bundle of the outputs.
      for (unsigned idx = 0 ; idx < nex_q.pin_count() ; idx += 1)
	    connect(nex_q.pin(idx), nex_set[idx].lnk);

	// Connect the D of the NetFF devices later.

	/* Synthesize the input to the DFF. */
      bool negedge = false;
      bool flag = statement_->synth_sync(des, scope(),
					 negedge, clock, ce,
					 aclr, aset, aset_value,
					 nex_set, nex_d, bitmasks,
					 vector<NetEvProbe*>());
      if (! flag) {
	    delete clock;
	    return false;
      }

      flag = tie_off_floating_inputs_(des, nex_set, nex_in, bitmasks, true);
      if (!flag) return false;

      for (unsigned idx = 0 ;  idx < nex_set.size() ;  idx += 1) {

	      //ivl_assert(*this, nex_set[idx].nex);
	    if (debug_synth2) {
		  cerr << get_fileline() << ": debug: "
		       << "Top level making a "
		       << nex_set[idx].wid << "-wide "
		       << "NetFF device." << endl;
	    }

	    NetFF*ff2 = new NetFF(scope(), scope()->local_symbol(),
				  negedge, nex_set[idx].wid);
	    des->add_node(ff2);
	    ff2->set_line(*this);
	    ff2->aset_value(aset_value[idx]);

	    NetNet*tmp = nex_d.pin(idx).nexus()->pick_any_net();
	    tmp->set_line(*this);
	    assert(tmp);

	    tmp = crop_to_width(des, tmp, ff2->width());

	    connect(nex_q.pin(idx), ff2->pin_Q());
	    connect(tmp->pin(0),    ff2->pin_Data());

	    connect(clock->pin(0),  ff2->pin_Clock());
	    if (ce.pin(idx).is_linked())
		  connect(ce.pin(idx),	  ff2->pin_Enable());
	    if (aclr.pin(idx).is_linked())
		  connect(aclr.pin(idx),  ff2->pin_Aclr());
	    if (aset.pin(idx).is_linked())
		  connect(aset.pin(idx),  ff2->pin_Aset());
#if 0
	    if (ff->pin_Sset().is_linked())
		  connect(ff->pin_Sset(), ff2->pin_Sset());
	    if (ff->pin_Sclr().is_linked())
		  connect(ff->pin_Sclr(), ff2->pin_Sclr());
#endif
      }

	// The "clock" net was just to carry the connection back
	// to the flip-flop. Delete it now. The connection will
	// persist.
      delete clock;

      synthesized_design_ = des;
      return true;
}

class synth2_f	: public functor_t {

    public:
      void process(Design*, NetProcTop*);

    private:
};


/*
 * Look at a process. If it is asynchronous, then synthesize it as an
 * asynchronous process and delete the process itself for its gates.
 */
void synth2_f::process(Design*des, NetProcTop*top)
{
      if (top->attribute(perm_string::literal("ivl_synthesis_off")).as_ulong() != 0)
	    return;

	/* If the scope that contains this process as a cell attribute
	   attached to it, then skip synthesis. */
      if (top->scope()->attribute(perm_string::literal("ivl_synthesis_cell")).len() > 0)
	    return;

	/* Create shared pullup and pulldown nodes (if they don't already
	   exist) for use when creating clock/gate enables. */
      top->scope()->add_tie_hi(des);
      top->scope()->add_tie_lo(des);

      if (top->is_synchronous()) {
	    bool flag = top->synth_sync(des);
	    if (! flag) {
		  cerr << top->get_fileline() << ": error: "
		       << "Unable to synthesize synchronous process."
		       << endl;
		  des->errors += 1;
		  return;
	    }
	    des->delete_process(top);
	    return;
      }

      if (! top->is_asynchronous()) {
	    bool synth_error_flag = false;
	    if (top->attribute(perm_string::literal("ivl_combinational")).as_ulong() != 0) {
		  cerr << top->get_fileline() << ": error: "
		       << "Process is marked combinational,"
		       << " but isn't really." << endl;
		  des->errors += 1;
		  synth_error_flag = true;
	    }

	    if (top->attribute(perm_string::literal("ivl_synthesis_on")).as_ulong() != 0) {
		  cerr << top->get_fileline() << ": error: "
		       << "Process is marked for synthesis,"
		       << " but I can't do it." << endl;
		  des->errors += 1;
		  synth_error_flag = true;
	    }

	    if (! synth_error_flag)
		  cerr << top->get_fileline() << ": warning: "
		       << "Process not synthesized." << endl;

	    return;
      }

      if (! top->synth_async(des)) {
	    cerr << top->get_fileline() << ": error: "
		 << "Unable to synthesize asynchronous process."
		 << endl;
	    des->errors += 1;
	    return;
      }

      des->delete_process(top);
}

void synth2(Design*des)
{
      synth2_f synth_obj;
      des->functor(&synth_obj);
}
