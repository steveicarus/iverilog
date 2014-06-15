/*
 * Copyright (c) 2002-2014 Stephen Williams (steve@icarus.com)
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

bool NetProc::synth_async(Design*, NetScope*, NexusSet&, NetBus&, NetBus&)
{
      return false;
}

bool NetProc::synth_sync(Design*des, NetScope*scope,
			 NetNet* /* ff_clk */, NetNet* /* ff_ce */,
			 NexusSet&nex_map, NetBus&nex_out,
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
      NetBus accumulated_nex_out (scope, nex_out.pin_count());
      return synth_async(des, scope, nex_map, nex_out, accumulated_nex_out);
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
				NetBus&)
{
      NetNet*rsig = rval_->synthesize(des, scope, rval_);
      assert(rsig);

      NetNet*lsig = lval_->sig();
      if (!lsig) {
	    cerr << get_fileline() << ": error: "
		 << "NetAssignBase::synth_async on unsupported lval ";
	    dump_lval(cerr);
	    cerr << endl;
	    des->errors += 1;
	    return false;
      }
      assert(lval_->more == 0);

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

	// Here we note if the l-value is actually a bit/part
	// select. If so, generate a NetPartSelect to perform the select.
      if (lval_->lwidth() != lsig->vector_width()) {
	    ivl_assert(*this, lval_->lwidth() < lsig->vector_width());

	      // XXXX If we are within a NetForLoop or similar
	      // processing, then there may be an index value. I
	      // currently do not know how to handle that, but
	      // probably I'm going to need the index_args passed in.
	    long base_off = 0;

	      // Evaluate the index expression to a constant.
	    const NetExpr*base_expr_raw = lval_->get_base();
	    ivl_assert(*this, base_expr_raw);
	    NetExpr*base_expr = base_expr_raw->evaluate_function(*this, scope->loop_index_tmp);
	    if (! eval_as_long(base_off, base_expr)) {
		  assert(0);
	    }
	    ivl_assert(*this, base_off >= 0);

	    ivl_variable_type_t tmp_data_type = rsig->data_type();
	    list<netrange_t>not_an_array;
	    netvector_t*tmp_type = new netvector_t(tmp_data_type, lsig->vector_width()-1,0);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, not_an_array, tmp_type);
	    tmp->local_flag(true);

	    NetPartSelect*ps = new NetPartSelect(tmp, base_off, lval_->lwidth(), NetPartSelect::PV);
	    ps->set_line(*this);
	    des->add_node(ps);

	    connect(ps->pin(0), rsig->pin(0));
	    rsig = tmp;
      }

      if (nex_out.pin_count() > 1) {
	    NexusSet tmp_set;
	    nex_output(tmp_set);
	    ivl_assert(*this, tmp_set.size()==1);
	    unsigned ptr = nex_map.find_nexus(tmp_set[0]);
	    ivl_assert(*this, rsig->pin_count()==1);
	    ivl_assert(*this, nex_map.size()==nex_out.pin_count());
	    ivl_assert(*this, nex_out.pin_count() > ptr);
	    connect(nex_out.pin(ptr), rsig->pin(0));

      } else {
	    ivl_assert(*this, nex_out.pin_count()==1);
	    ivl_assert(*this, rsig->pin_count()==1);
	    connect(nex_out.pin(0), rsig->pin(0));
      }

	/* This lval_ represents a reg that is a WIRE in the
	   synthesized results. This function signals the destructor
	   to change the REG that this l-value refers to into a
	   WIRE. It is done then, at the last minute, so that pending
	   synthesis can continue to work with it as a WIRE. */
      lval_->turn_sig_to_wire_on_release();

      return true;
}

bool NetProc::synth_async_block_substatement_(Design*des, NetScope*scope,
					      NexusSet&nex_map,
					      NetBus&accumulated_nex_out,
					      NetProc*substmt)
{
	// Create a temporary map of the output only from this statement.
      NexusSet tmp_map;
      substmt->nex_output(tmp_map);
      if (debug_synth2) {
	    cerr << get_fileline() << ": NetProc::synth_async_block_substatement_: "
		 << "tmp_map.size()==" << tmp_map.size()
		 << " for statement at " << substmt->get_fileline()
		 << endl;
      }

	/* Create also a temporary NetBus to collect the
	   output from the synthesis. */
      NetBus tmp_out (scope, tmp_map.size());

	// Map (and move) the accumulated_nex_out for this block
	// to the version that we can pass to the next
	// statement. We will move the result back later.
      NetBus accumulated_tmp_out (scope, tmp_map.size());

      for (unsigned idx = 0 ; idx < accumulated_nex_out.pin_count() ; idx += 1) {
	    unsigned ptr = tmp_map.find_nexus(nex_map[idx]);
	    if (ptr >= tmp_map.size())
		  continue;

	    connect(accumulated_tmp_out.pin(ptr), accumulated_nex_out.pin(idx));
	    accumulated_nex_out.pin(idx).unlink();
      }

      bool ok_flag = substmt->synth_async(des, scope, tmp_map, tmp_out, accumulated_tmp_out);

      if (ok_flag == false)
	    return false;

	// Now map the output from the substatement back to the
	// accumulated_nex_out for this block. Look for the
	// nex_map pin that is linked to the tmp_map.pin(idx)
	// pin, and link that to the tmp_out.pin(idx) output link.
      for (unsigned idx = 0 ;  idx < tmp_out.pin_count() ;  idx += 1) {
	    unsigned ptr = nex_map.find_nexus(tmp_map[idx]);
	    ivl_assert(*this, ptr < accumulated_nex_out.pin_count());
	    connect(accumulated_nex_out.pin(ptr), tmp_out.pin(idx));
      }

      return true;
}

/*
 * Sequential blocks are translated to asynchronous logic by
 * translating each statement of the block, in order, into gates. The
 * nex_out for the block is the union of the nex_out for all the
 * substatements.
 */
bool NetBlock::synth_async(Design*des, NetScope*scope,
			   NexusSet&nex_map, NetBus&nex_out,
			   NetBus&accumulated_nex_out)
{
      if (last_ == 0) {
	    return true;
      }

      bool flag = true;
      NetProc*cur = last_;
      do {
	    cur = cur->next_;

	    bool ok_flag = synth_async_block_substatement_(des, scope, nex_map,
							   accumulated_nex_out,
							   cur);
	    flag = flag && ok_flag;
	    if (ok_flag == false)
		  continue;

      } while (cur != last_);

	// The output from the block is now the accumulated outputs.
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1)
	    connect(nex_out.pin(idx), accumulated_nex_out.pin(idx));

      return flag;
}

bool NetCase::synth_async(Design*des, NetScope*scope,
			  NexusSet&nex_map, NetBus&nex_out,
			  NetBus&accumulated_nex_out)
{
      if (type()==NetCase::EQZ || type()==NetCase::EQX)
	    return synth_async_casez_(des, scope, nex_map, nex_out, accumulated_nex_out);

	// Special case: If the case expression is constant, then this
	// is a pattern where the guards are non-constant and tested
	// against a constant case. Handle this as chained conditions
	// instead.
      if (dynamic_cast<NetEConst*> (expr_))
	    return synth_async_casez_(des, scope, nex_map, nex_out, accumulated_nex_out);

	/* Synthesize the select expression. */
      NetNet*esig = expr_->synthesize(des, scope, expr_);

      unsigned sel_width = esig->vector_width();
      ivl_assert(*this, sel_width > 0);

      ivl_assert(*this, nex_map.size() == nex_out.pin_count());

      vector<unsigned> mux_width (nex_out.pin_count());
      for (unsigned idx = 0 ;  idx < nex_out.pin_count() ;  idx += 1) {
	    mux_width[idx] = nex_map[idx].wid;
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async: "
		       << "idx=" << idx
		       << ", mux_width[idx]=" << mux_width[idx] << endl;
	    }
      }

	// The accumulated_nex_out is taken as the input for this
	// statement. Since there are collection of statements that
	// start at this same point, we save all these inputs and
	// reuse them for each statement.
      NetBus statement_input (scope, nex_out.pin_count());
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    connect(statement_input.pin(idx), accumulated_nex_out.pin(idx));
      }

	/* Collect all the statements into a map of index to
	   statement. The guard expression it evaluated to be the
	   index of the mux value, and the statement is bound to that
	   index. */

      unsigned long max_guard_value = 0;
      map<unsigned long,NetProc*>statement_map;
      NetProc*statement_default = 0;

      for (size_t item = 0 ;  item < items_.size() ;  item += 1) {
	    if (items_[item].guard == 0) {
		  statement_default = items_[item].statement;
		  continue;
	    }

	    NetEConst*ge = dynamic_cast<NetEConst*>(items_[item].guard);
	    ivl_assert(*this, ge);
	    verinum gval = ge->value();

	    unsigned sel_idx = gval.as_ulong();

	    assert(items_[item].statement);
	    statement_map[sel_idx] = items_[item].statement;

	    if (sel_idx > max_guard_value)
		  max_guard_value = sel_idx;
      }

	// The mux_size is the number of inputs that are selected.
      unsigned mux_size = max_guard_value + 1;

	// If the sel_width can select more than just the explicit
	// guard values, and there is a default statement, then adjust
	// the mux size to allow for the implicit selections.
      if (statement_default && ((1U<<sel_width) > mux_size)) {
	    mux_size = 1<<sel_width;
      }


	/* If there is a default clause, synthesize it once and we'll
	   link it in wherever it is needed. */
      NetBus default_bus (scope, nex_map.size());
      if (statement_default) {

	    bool flag = synth_async_block_substatement_(des, scope, nex_map,
							accumulated_nex_out,
							statement_default);
	    if (!flag) {
		  return false;
	    }

	    for (unsigned idx = 0 ; idx < default_bus.pin_count() ; idx += 1) {
		  connect(default_bus.pin(idx), accumulated_nex_out.pin(idx));
		  accumulated_nex_out.pin(idx).unlink();
	    }

	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async: "
		       << "synthesize default clause at " << statement_default->get_fileline()
		       << " is done." << endl;
	    }
      }

      vector<NetMux*> mux (mux_width.size());
      for (size_t mdx = 0 ; mdx < mux_width.size() ; mdx += 1) {
	    mux[mdx] = new NetMux(scope, scope->local_symbol(),
				  mux_width[mdx], mux_size, sel_width);
	    des->add_node(mux[mdx]);

	      // The select signal is already synthesized, and is
	      // common for every mux of this case statement. Simply
	      // hook it up.
	    connect(mux[mdx]->pin_Sel(), esig->pin(0));

	      // The outputs are in the nex_out, and connected to the
	      // mux Result pins.
	    connect(mux[mdx]->pin_Result(), nex_out.pin(mdx));

	      // Make sure the output is now connected to a net. If
	      // not, then create a fake one to carry the net-ness of
	      // the pin.
	    if (mux[mdx]->pin_Result().nexus()->pick_any_net() == 0) {
		  ivl_variable_type_t mux_data_type = IVL_VT_LOGIC;
		  netvector_t*tmp_vec = new netvector_t(mux_data_type, mux_width[mdx]-1, 0);
		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::TRI, tmp_vec);
		  tmp->local_flag(true);
		  ivl_assert(*this, tmp->vector_width() != 0);
		  connect(mux[mdx]->pin_Result(), tmp->pin(0));
	    }
      }

      for (unsigned idx = 0 ;  idx < mux_size ;  idx += 1) {

	    NetProc*stmt = statement_map[idx];
	    if (stmt==0 && statement_default) {
		  ivl_assert(*this, default_bus.pin_count() == mux.size());
		  for (size_t mdx = 0 ; mdx < mux.size() ; mdx += 1)
			connect(mux[mdx]->pin_Data(idx), default_bus.pin(mdx));

		  continue;
	    }
	    if (stmt == 0) {
		  cerr << get_fileline() << ": error: case " << idx
		       << " is not accounted for in asynchronous mux." << endl;
		  des->errors += 1;
		  continue;
	    }

	    NetBus accumulated_tmp (scope, nex_map.size());
	    for (unsigned pin = 0 ; pin < nex_map.size() ; pin += 1)
		  connect(accumulated_tmp.pin(pin), statement_input.pin(pin));


	    synth_async_block_substatement_(des, scope, nex_map, accumulated_tmp, stmt);

	    for (size_t mdx = 0 ; mdx < mux.size() ; mdx += 1) {
		  connect(mux[mdx]->pin_Data(idx), accumulated_tmp.pin(mdx));

		  if (mux[mdx]->pin_Data(idx).nexus()->pick_any_net()==0) {
			cerr << get_fileline() << ": warning: case " << idx
			     << " has no input for mux slice " << mdx << "." << endl;

			ivl_variable_type_t mux_data_type = IVL_VT_LOGIC;
			netvector_t*tmp_vec = new netvector_t(mux_data_type, mux_width[mdx]-1, 0);
			NetNet*tmpn = new NetNet(scope, scope->local_symbol(),
						 NetNet::TRI, tmp_vec);
			tmpn->local_flag(true);
			ivl_assert(*this, tmpn->vector_width() != 0);
			connect(mux[mdx]->pin_Data(idx), tmpn->pin(0));
		  }
		  ivl_assert(*this, mux[mdx]->pin_Data(idx).nexus()->pick_any_net());
	    }
      }

      return true;
}

/*
 * casez statements are hard to implement as a single wide mux because
 * the test doesn't really map to a select input. Instead, implement
 * it as a chain of binary muxes. This gives the synthesizer my
 * flexibility, and is more typically what is desired from a casez anyhow.
 */
bool NetCase::synth_async_casez_(Design*des, NetScope*scope,
				 NexusSet&nex_map, NetBus&nex_out,
				 NetBus&accumulated_nex_out)
{
	/* Synthesize the select expression. */
      NetNet*esig = expr_->synthesize(des, scope, expr_);

      unsigned sel_width = esig->vector_width();
      ivl_assert(*this, sel_width > 0);

      ivl_assert(*this, nex_map.size() == nex_out.pin_count());

      vector<unsigned>mux_width (nex_out.pin_count());
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    mux_width[idx] = nex_map[idx].wid;
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async_casez_: "
		       << "idx=" << idx
		       << ", mux_width[idx]=" << mux_width[idx] << endl;
	    }
      }

	// The accumulated_nex_out is taken as the input for this
	// statement. Since there are collection of statements that
	// start at this same point, we save all these inputs and
	// reuse them for each statement.
      NetBus statement_input (scope, nex_out.pin_count());
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    connect(statement_input.pin(idx), accumulated_nex_out.pin(idx));
      }

	// Look for a default statement.
      NetProc*statement_default = 0;
      for (size_t item = 0 ; item < items_.size() ; item += 1) {
	    if (items_[item].guard != 0)
		  continue;

	    ivl_assert(*this, statement_default==0);
	    statement_default = items_[item].statement;
      }

      NetBus default_bus (scope, nex_out.pin_count());
      if (statement_default) {
	    bool flag = synth_async_block_substatement_(des, scope, nex_map,
							accumulated_nex_out,
							statement_default);
	    if (!flag) {
		  return false;
	    }

	    for (unsigned idx = 0 ; idx < default_bus.pin_count() ; idx += 1) {
		  connect(default_bus.pin(idx), accumulated_nex_out.pin(idx));
		  accumulated_nex_out.pin(idx).unlink();
	    }

	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCase::synth_async_casez_: "
		       << "synthesize default clause at " << statement_default->get_fileline()
		       << " is done." << endl;
	    }
      }

      netvector_t*condit_type = new netvector_t(IVL_VT_LOGIC, 0, 0);

      NetCaseCmp::kind_t case_kind;
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
      }

	// Process the items from last to first. We generate a
	// true/false must, with the select being the comparison of
	// the case select with the guard expression. The true input
	// (data1) is the current statement, and the false input is
	// the result of a later statement.
      vector<NetMux*>mux_prev (mux_width.size());
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
	      // Note that the expression that may have windcards must
	      // go in the pin(2) input. This is the definiton of the
	      // NetCaseCmp statement.
	    connect(condit_dev->pin(2), esig->pin(0));
	    connect(condit_dev->pin(1), guard->pin(0));

	    NetNet*condit = new NetNet(scope, scope->local_symbol(),
				       NetNet::TRI, condit_type);
	    condit->set_line(*this);
	    condit->local_flag(true);
	    connect(condit_dev->pin(0), condit->pin(0));

	      // Synthesize the guarded statement.
	    NetBus true_bus (scope, nex_out.pin_count());
	    for (unsigned pin = 0 ; pin < nex_map.size() ; pin += 1)
		  connect(true_bus.pin(pin), statement_input.pin(pin));

	    synth_async_block_substatement_(des, scope, nex_map, true_bus, stmt);

	    for (unsigned mdx = 0 ; mdx < mux_width.size() ; mdx += 1) {
		  NetMux*mux_cur = new NetMux(scope, scope->local_symbol(),
					      mux_width[mdx], 2, 1);
		  des->add_node(mux_cur);
		  mux_cur->set_line(*this);
		  connect(mux_cur->pin_Sel(), condit->pin(0));

		  connect(mux_cur->pin_Data(1), true_bus.pin(mdx));

		    // If there is a previous mux, then use that as the
		    // false clause input. Otherwise, use the default.
		  if (mux_prev[mdx]) {
			connect(mux_cur->pin_Data(0), mux_prev[mdx]->pin_Result());
		  } else {
			connect(mux_cur->pin_Data(0), default_bus.pin(mdx));
		  }

		    // Make a NetNet for the result.
		  ivl_variable_type_t mux_data_type = IVL_VT_LOGIC;
		  netvector_t*tmp_vec = new netvector_t(mux_data_type, mux_width[mdx]-1, 0);
		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::TRI, tmp_vec);
		  tmp->local_flag(true);
		  ivl_assert(*this, tmp->vector_width() != 0);
		  connect(mux_cur->pin_Result(), tmp->pin(0));

		    // This mux becomes the "false" input to the next mux.
		  mux_prev[mdx] = mux_cur;
	    }
      }

	// Connect the last mux to the output.
      for (size_t mdx = 0 ; mdx < mux_prev.size() ; mdx += 1) {
	    connect(mux_prev[mdx]->pin_Result(), nex_out.pin(mdx));
      }

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
			    NetBus&accumulated_nex_out)
{
      if (if_ == 0) {
	    return false;
      }
      if (else_ == 0) {
	    bool latch_flag = false;
	    for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
		  if (! accumulated_nex_out.pin(idx).is_linked())
			latch_flag = true;
	    }
	    if (latch_flag) {
		  cerr << get_fileline() << ": error: Asynchronous if statement"
		       << " cannot synthesize missing \"else\""
		       << " without generating latches." << endl;
		  return false;
	    }
      }

      ivl_assert(*this, if_ != 0);

	// Synthesize the condition. This will act as a select signal
	// for a binary mux.
      NetNet*ssig = expr_->synthesize(des, scope, expr_);
      assert(ssig);

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetCondit::synth_async: "
		 << "Synthesize if clause at " << if_->get_fileline()
		 << endl;
      }

      NetBus statement_input (scope, nex_out.pin_count());

      bool flag;
      NetBus asig(scope, nex_out.pin_count());
      NetBus atmp(scope, nex_out.pin_count());
      flag = if_->synth_async(des, scope, nex_map, asig, atmp);
      if (!flag) {
	    return false;
      }

      NetBus btmp(scope, nex_out.pin_count());
      NetBus bsig(scope, nex_out.pin_count());

      if (else_==0) {
	    for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
		  connect(bsig.pin(idx), accumulated_nex_out.pin(idx));
		  accumulated_nex_out.pin(idx).unlink();
	    }

      } else {

	    for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
		  connect(statement_input.pin(idx), accumulated_nex_out.pin(idx));
	    }
	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "Synthesize else clause at " << else_->get_fileline()
		       << endl;
	    }

	    flag = synth_async_block_substatement_(des, scope, nex_map, accumulated_nex_out, else_);
	    if (!flag) {
		  return false;
	    }
	    for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
		  connect(bsig.pin(idx), accumulated_nex_out.pin(idx));
		  accumulated_nex_out.pin(idx).unlink();
	    }

	    if (debug_synth2) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "synthesize else clause at " << else_->get_fileline()
		       << " is done." << endl;
	    }
      }

	/* The nex_out output, asig input, and bsig input all have the
	   same pin count (usually, but not always 1) because they are
	   net arrays of the same dimension. The for loop below creates
	   a NetMux for each pin of the output. (Note that pins may
	   be, in fact usually are, vectors.) */

      ivl_assert(*this, nex_out.pin_count()==asig.pin_count());
      ivl_assert(*this, nex_out.pin_count()==bsig.pin_count());

      bool rc_flag = true;
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	      // It should not be possible for the a (true) or b
	      // (false) signals to be missing. If either is, print a
	      // warning and clear a flag so that the rest of this
	      // code can find a way to cope.
	    bool asig_is_present = true;
	    if (! asig.pin(idx).nexus()->pick_any_net()) {
		  cerr << get_fileline() << ": warning: "
		       << "True clause of conditional statement might not"
		       << " drive all expected outputs." << endl;
		  asig_is_present = false;
	    }

	    bool bsig_is_present = true;
	    if (! bsig.pin(idx).nexus()->pick_any_net()) {
		  cerr << get_fileline() << ": warning: "
		       << "False clause of conditional statement might not"
		       << " drive all expected outputs." << endl;
		  bsig_is_present = false;
	    }

	      // Guess the mux type from the type of the output.
	    ivl_variable_type_t mux_data_type = IVL_VT_LOGIC;
	    if (NetNet*tmp = nex_out.pin(idx).nexus()->pick_any_net()) {
		  mux_data_type = tmp->data_type();
	    }

	    unsigned mux_off = 0;
	    unsigned mux_width;
	    if (asig_is_present)
		  mux_width = asig.pin(idx).nexus()->vector_width();
	    else if (bsig_is_present)
		  mux_width = bsig.pin(idx).nexus()->vector_width();
	    else
		  mux_width = 0;

	    const unsigned mux_lwidth = mux_width;
	    ivl_assert(*this, mux_width != 0);

	    NetPartSelect*apv = detect_partselect_lval(asig.pin(idx));
	    if (debug_synth2 && apv) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "Assign-to-part apv base=" << apv->base()
		       << ", width=" << apv->width() << endl;
	    }

	    NetPartSelect*bpv = detect_partselect_lval(bsig.pin(idx));
	    if (debug_synth2 && bpv) {
		  cerr << get_fileline() << ": NetCondit::synth_async: "
		       << "Assign-to-part bpv base=" << bpv->base()
		       << ", width=" << bpv->width() << endl;
	    }

	    if (apv && bpv && apv->width()==bpv->width() && apv->base()==bpv->base()) {
		    // The a and b sides are both assigning to the
		    // same bits of the output, so we can use that to
		    // create a much narrower mux that only
		    // manipulates the width of the part.
		  mux_width = apv->width();
		  mux_off = apv->base();
		  asig.pin(idx).unlink();
		  bsig.pin(idx).unlink();
		  connect(asig.pin(idx), apv->pin(0));
		  connect(bsig.pin(idx), bpv->pin(0));
		  delete apv;
		  delete bpv;

	    } else {
		    // The part selects are of no use. Forget them.
		  apv = 0;
		  bpv = 0;
	    }

	    if (bsig_is_present && mux_width != bsig.pin(idx).nexus()->vector_width()) {
		  cerr << get_fileline() << ": internal error: "
		       << "NetCondit::synth_async: "
		       << "Mux input sizes do not match."
		       << " A size=" << mux_lwidth
		       << ", B size=" << bsig.pin(idx).nexus()->vector_width()
		       << endl;
		  cerr << get_fileline() << ":               : "
		       << "asig node pins:" << endl;
		  asig.dump_node_pins(cerr, 8);
		  cerr << get_fileline() << ":               : "
		       << "if_ statement:" << endl;
		  if_->dump(cerr, 8);
		  cerr << get_fileline() << ":               : "
		       << "bsig node pins:" << endl;
		  bsig.dump_node_pins(cerr, 4);
		  if (else_) {
			cerr << get_fileline() << ":               : "
			     << "else_ statement:" << endl;
			else_->dump(cerr, 8);
		  }
		  rc_flag = false;
	    }

	    NetMux*mux = new NetMux(scope, scope->local_symbol(),
				    mux_width, 2, 1);
	    mux->set_line(*this);

	    list<netrange_t>not_an_array;
	    netvector_t*tmp_type = 0;
	    if (mux_width==1)
		  tmp_type = new netvector_t(mux_data_type);
	    else
		  tmp_type = new netvector_t(mux_data_type, mux_width-1,0);

	      // Bind some temporary signals to carry pin type.
	    NetNet*otmp = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, not_an_array, tmp_type);
	    otmp->local_flag(true);
	    connect(mux->pin_Result(),otmp->pin(0));

	    connect(mux->pin_Sel(),   ssig->pin(0));
	    connect(mux->pin_Data(1), asig.pin(idx));
	    connect(mux->pin_Data(0), bsig.pin(idx));

	    if (! asig_is_present) {
		  tmp_type = new netvector_t(mux_data_type, mux_width-1,0);
		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::WIRE, not_an_array, tmp_type);
		  connect(mux->pin_Data(1), tmp->pin(0));
	    }

	    if (! bsig_is_present) {
		  tmp_type = new netvector_t(mux_data_type, mux_width-1,0);
		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::WIRE, not_an_array, tmp_type);
		  connect(mux->pin_Data(0), tmp->pin(0));
	    }

	      // We are only muxing a part of the output vector, so
	      // make a NetPartSelect::PV to widen the vector to the
	      // output at hand.
	    if (mux_width < mux_lwidth) {
		  tmp_type = new netvector_t(mux_data_type, mux_lwidth-1,0);
		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::WIRE, not_an_array, tmp_type);
		  NetPartSelect*ps = new NetPartSelect(tmp, mux_off, mux_width, NetPartSelect::PV);
		  des->add_node(ps);
		  connect(ps->pin(0), otmp->pin(0));
		  otmp = tmp;
	    }

	    connect(nex_out.pin(idx), otmp->pin(0));

	      // Handle the special case that this NetMux is only
	      // assigning to a part of the vector. If that is the
	      // case, then we need to blend this output with the
	      // already calculated input to this statement so that we
	      // don't accidentally disconnect the other drivers to
	      // other bits.
	      // FIXME: NEED TO CHECK THAT THESE DRIVERS DON'T
	      // OVERLAP. THIS CODE CURRENTLY DOESN'T DO THAT TEST.
	    if (mux_width < mux_lwidth && if_ && else_) {
		  if (debug_synth2) {
			cerr << get_fileline() << ": NetCondit::synth_async: "
			     << "This MetMux only impacts a few bits of output,"
			     << " so combine nex_out with statement input."
			     << endl;
			cerr << get_fileline() << ": NetCondit::synth_async: "
			     << "MISSING TEST FOR CORRECTNESS OF THE BLEND!"
			     << endl;
		  }
		  vector<bool>mask = statement_input.pin(idx).nexus()->driven_mask();
		  for (size_t bit = mux_off ; bit < mux_off+mux_width ; bit += 1) {
			ivl_assert(*this, mask[bit]==false);
		  }
		  connect(nex_out.pin(idx), statement_input.pin(idx));
	    }

	    des->add_node(mux);
      }

      return rc_flag;
}

bool NetEvWait::synth_async(Design*des, NetScope*scope,
			    NexusSet&nex_map, NetBus&nex_out,
			    NetBus&accumulated_nex_out)
{
      bool flag = statement_->synth_async(des, scope, nex_map, nex_out, accumulated_nex_out);
      return flag;
}

bool NetForLoop::synth_async(Design*des, NetScope*scope,
			     NexusSet&nex_map, NetBus&nex_out,
			     NetBus&accumulated_nex_out)
{
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

	    rc = synth_async_block_substatement_(des, scope, nex_map,
						 accumulated_nex_out,
						 statement_);

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

	// The output from the block is now the accumulated outputs.
      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1)
	    connect(nex_out.pin(idx), accumulated_nex_out.pin(idx));

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

      NetBus nex_q (scope(), nex_set.size());
      for (unsigned idx = 0 ;  idx < nex_set.size() ;  idx += 1) {
	    NexusSet::elem_t&item = nex_set[idx];
	    if (item.base != 0 || item.wid!=item.lnk.nexus()->vector_width()) {
		  ivl_variable_type_t tmp_data_type = IVL_VT_LOGIC;
		  list<netrange_t>not_an_array;
		  netvector_t*tmp_type = new netvector_t(tmp_data_type, item.lnk.nexus()->vector_width()-1,0);
		  NetNet*tmp_sig = new NetNet(scope(), scope()->local_symbol(),
					      NetNet::WIRE, not_an_array, tmp_type);
		  tmp_sig->local_flag(true);

		  NetPartSelect*tmp = new NetPartSelect(tmp_sig, item.base,
							item.wid, NetPartSelect::PV);
		  des->add_node(tmp);
		  tmp->set_line(*this);
		  connect(tmp->pin(0), nex_q.pin(idx));
		  connect(item.lnk, tmp_sig->pin(0));

	    } else {
		  connect(item.lnk, nex_q.pin(idx));
	    }
      }

      NetBus tmp_q (scope(), nex_set.size());
      bool flag = statement_->synth_async(des, scope(), nex_set, nex_q, tmp_q);
      return flag;
}

/*
 * This method is called when a block is encountered near the surface
 * of a synchronous always statement. For example, this code will be
 * invoked for input like this:
 *
 *     always @(posedge clk...) begin
 *           <statement1>
 *           <statement2>
 *           ...
 *     end
 *
 * This needs to be split into a DFF bank for each statement, because
 * the statements may each infer different reset and enable signals.
 */
bool NetBlock::synth_sync(Design*des, NetScope*scope,
			  NetNet*ff_clk, NetNet*ff_ce,
			  NexusSet&nex_map, NetBus&nex_out,
			  const vector<NetEvProbe*>&events_in)
{
      if (debug_synth2) {
	    cerr << get_fileline() << ": NetBlock::synth_sync: "
		 << "Examine this block for synchronous logic." << endl;
      }

      bool flag = true;

      NetProc*cur = last_;
      do {
	    cur = cur->next_;

	      /* Create a temporary nex_map for the substatement. */
	    NexusSet tmp_set;
	    cur->nex_output(tmp_set);

	      /* Create also a temporary net_out to collect the
		 output. The tmp1 and tmp2 map and out sets together
		 are used to collect the outputs from the substatement
		 for the inputs of the FF bank. */
	    NetBus tmp_out (scope, tmp_set.size());

	      /* Now go on with the synchronous synthesis for this
		 subset of the statement. The tmp_map is the output
		 nexa that we expect, and the tmp_out is where we want
		 those outputs connected. */
	    bool ok_flag = cur->synth_sync(des, scope, ff_clk, ff_ce,
					   tmp_set, tmp_out, events_in);
	    flag = flag && ok_flag;

	    if (ok_flag == false)
		  continue;

	      /* Use the nex_map to link up the output from the
		 substatement to the output of the block as a
		 whole. It is occasionally possible to have outputs
		 beyond the input set, for example when the l-value of
		 an assignment is smaller than the r-value. */
	    for (unsigned idx = 0 ;  idx < tmp_out.pin_count() ; idx += 1) {
		  unsigned ptr = nex_map.find_nexus(tmp_set[idx]);
		  assert(ptr < nex_out.pin_count());
		  connect(nex_out.pin(ptr), tmp_out.pin(idx));
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
			   NetNet*ff_clk, NetNet*ff_ce,
			   NexusSet&nex_map, NetBus&nex_out,
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

	    cerr << get_fileline() << ": sorry: "
		 << "Forgot how to implement asynchronous set/reset." << endl;
	    return false;
#if 0
	      /* Ah, this edge is in the sensitivity list for the
		 expression, so we have an asynchronous
		 input. Synthesize the set/reset input expression. */

	    NetNet*rst = expr_->synthesize(des);
	    assert(rst->pin_count() == 1);

	      /* XXXX I really should find a way to check that the
		 edge used on the reset input is correct. This would
		 involve interpreting the expression that is fed by the
		 reset expression. */
	      //assert(ev->edge() == NetEvProbe::POSEDGE);

	      /* Synthesize the true clause to figure out what
		 kind of set/reset we have. */
	    NetNet*asig = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, nex_map->pin_count());
	    asig->local_flag(true);

	    assert(if_ != 0);
	    bool flag = if_->synth_async(des, scope, nex_map, asig);

	    assert(asig->pin_count() == ff->width());

	      /* Collect the set/reset value into a verinum. If
		 this turns out to be entirely 0 values, then
		 use the Aclr input. Otherwise, use the Aset
		 input and save the set value. */
	    verinum tmp (verinum::V0, ff->width());
	    for (unsigned bit = 0 ;  bit < ff->width() ;  bit += 1) {

		  assert(asig->pin(bit).nexus()->drivers_constant());
		  tmp.set(bit, asig->pin(bit).nexus()->driven_value());
	    }

	    assert(tmp.is_defined());
	    if (tmp.is_zero()) {
		  connect(ff->pin_Aclr(), rst->pin(0));

	    } else {
		  connect(ff->pin_Aset(), rst->pin(0));
		  ff->aset_value(tmp);
	    }

	    delete asig;
	    delete expr_input;

	    assert(events_in.count() == 1);
	    assert(else_ != 0);
	    flag = else_->synth_sync(des, scope, ff, nex_map,
				     nex_out, svector<NetEvProbe*>(0))
		  && flag;
            DEBUG_SYNTH2_EXIT("NetCondit",flag)
	    return flag;
#endif
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
		  for (unsigned bit = 0 ;  bit < ff->width() ;  bit += 1) {

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
					   nex_out, svector<NetEvProbe*>(0))
			&& flag;
		  DEBUG_SYNTH2_EXIT("NetCondit",flag)
		  return flag;
	    }
      }

      delete a_set;
#endif

	/* Failed to find an asynchronous set/reset, so any events
	   input are probably in error. */
      if (events_in.size() > 0) {
	    cerr << get_fileline() << ": error: Events are unaccounted"
		 << " for in process synthesis." << endl;
	    des->errors += 1;
      }


	/* If this is an if/then/else, then it is likely a
	   combinational if, and I should synthesize it that way. */
      if (if_ && else_) {
	    NetBus tmp (scope, nex_out.pin_count());
	    bool flag = synth_async(des, scope, nex_map, nex_out, tmp);
	    return flag;
      }

      ivl_assert(*this, if_);
      ivl_assert(*this, !else_);

	/* Synthesize the enable expression. */
      NetNet*ce = expr_->synthesize(des, scope, expr_);
      ivl_assert(*this, ce->pin_count()==1 && ce->vector_width()==1);

	/* What's left, is a synchronous CE statement like this:

	     if (expr_) <true statement>;

	   The expr_ expression has already been synthesized to the ce
	   net, so we connect it here to the FF. What's left is to
	   synthesize the substatement as a combinational
	   statement.

	   Watch out for the special case that there is already a CE
	   connected to this FF. This can be caused by code like this:

	     if (a) if (b) <statement>;

	   In this case, we are working on the inner IF, so we AND the
	   a and b expressions to make a new CE. */

      if (ff_ce->pin(0).is_linked()) {
	    NetLogic*ce_and = new NetLogic(scope,
					   scope->local_symbol(), 3,
					   NetLogic::AND, 1);
	    des->add_node(ce_and);
	    connect(ff_ce->pin(0), ce_and->pin(1));
	    connect(ce->pin(0), ce_and->pin(2));

	    ff_ce->pin(0).unlink();
	    connect(ff_ce->pin(0), ce_and->pin(0));

      } else {

	    connect(ff_ce->pin(0), ce->pin(0));
      }

      bool flag = if_->synth_sync(des, scope, ff_clk, ff_ce, nex_map, nex_out, events_in);

      return flag;
}

bool NetEvWait::synth_sync(Design*des, NetScope*scope,
			   NetNet*ff_clk, NetNet*ff_ce,
			   NexusSet&nex_map, NetBus&nex_out,
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
			cerr << get_fileline() << ":      : Perhaps an"
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
	    return false;
      }

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetEvWait::synth_sync: "
		 << "Found and synthesized the FF clock." << endl;
      }

      connect(ff_clk->pin(0), pclk->pin(0));
      if (pclk->edge() == NetEvProbe::NEGEDGE) {
	    perm_string polarity = perm_string::literal("Clock:LPM_Polarity");
	    ff_clk->attribute(polarity, verinum("INVERT"));

	    if (debug_synth2) {
		  cerr << get_fileline() << ": debug: "
		       << "Detected a NEGEDGE clock for the synthesized ff."
		       << endl;
	    }
      }

	/* Synthesize the input to the DFF. */
      bool flag = statement_->synth_sync(des, scope, ff_clk, ff_ce,
					 nex_map, nex_out, events);

      return flag;
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

	/* Make a model FF that will connect to the first item in the
	   set, and will also take the initial connection of clocks
	   and resets. */

	// Create a net to carry the clock for the synthesized FFs.
      NetNet*clock = new NetNet(scope(), scope()->local_symbol(),
				NetNet::TRI, &netvector_t::scalar_logic);
      clock->local_flag(true);

      NetNet*ce = new NetNet(scope(), scope()->local_symbol(),
			     NetNet::TRI, &netvector_t::scalar_logic);
      ce->local_flag(true);

      NetBus nex_d (scope(), nex_set.size());
      NetBus nex_q (scope(), nex_set.size());

	/* The Q of the NetFF devices is connected to the output that
	   we are. The nex_q is a bundle of the outputs. We will also
	   pass the nex_q as a map to the statement's synth_sync
	   method to map it to the correct nex_d pin. */
      for (unsigned idx = 0 ;  idx < nex_set.size() ;  idx += 1) {
	    connect(nex_set[idx].lnk, nex_q.pin(idx));
      }

	// Connect the input later.

	/* Synthesize the input to the DFF. */
      bool flag = statement_->synth_sync(des, scope(), clock, ce,
					 nex_set, nex_d,
					 vector<NetEvProbe*>());
      if (! flag) {
	    delete clock;
	    return false;
      }

      for (unsigned idx = 0 ;  idx < nex_set.size() ;  idx += 1) {

	      //ivl_assert(*this, nex_set[idx].nex);
	    if (debug_synth2) {
		  cerr << get_fileline() << ": debug: "
		       << "Top level making a "
		       << nex_set[idx].wid << "-wide "
		       << "NetFF device." << endl;
	    }

	    NetFF*ff2 = new NetFF(scope(), scope()->local_symbol(),
				  nex_set[idx].wid);
	    des->add_node(ff2);
	    ff2->set_line(*this);

	    NetNet*tmp = nex_d.pin(idx).nexus()->pick_any_net();
	    tmp->set_line(*this);
	    assert(tmp);

	    tmp = crop_to_width(des, tmp, ff2->width());

	    connect(nex_q.pin(idx), ff2->pin_Q());
	    connect(tmp->pin(0),    ff2->pin_Data());

	    connect(clock->pin(0),  ff2->pin_Clock());
	    if (ce->is_linked())
		  connect(ce->pin(0),     ff2->pin_Enable());
#if 0
	    if (ff->pin_Aset().is_linked())
		  connect(ff->pin_Aset(), ff2->pin_Aset());
	    if (ff->pin_Aclr().is_linked())
		  connect(ff->pin_Aclr(), ff2->pin_Aclr());
	    if (ff->pin_Sset().is_linked())
		  connect(ff->pin_Sset(), ff2->pin_Sset());
	    if (ff->pin_Sclr().is_linked())
		  connect(ff->pin_Sclr(), ff2->pin_Sclr());
#endif
      }

	// The "clock" and "ce" nets were just to carry the connection
	// back to the flip-flop. Delete them now. The connections
	// will persist.
      delete clock;
      delete ce;
      return true;
}

class synth2_f  : public functor_t {

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

      if (top->is_synchronous()) {
	    bool flag = top->synth_sync(des);
	    if (! flag) {
		  cerr << top->get_fileline() << ": error: "
		       << "Unable to synthesize synchronous process." << endl;
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
	    cerr << top->get_fileline() << ": internal error: "
		 << "is_asynchronous does not match "
		 << "sync_async results." << endl;
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
