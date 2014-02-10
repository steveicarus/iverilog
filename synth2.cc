/*
 * Copyright (c) 2002-2012 Stephen Williams (steve@icarus.com)
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

bool NetProc::synth_async(Design*, NetScope*, const NexusSet&, NetBus&)
{
      return false;
}

bool NetProc::synth_sync(Design*des, NetScope*scope,
			 NetNet* /* ff_clk */, NetNet* /* ff_ce */,
			 const NexusSet&nex_map, NetBus&nex_out,
			 const vector<NetEvProbe*>&events)
{
      if (events.size() > 0) {
	    cerr << get_fileline() << ": error: Events are unaccounted"
		 << " for in process synthesis." << endl;
	    des->errors += 1;
      }

	/* Synthesize the input to the DFF. */
      return synth_async(des, scope, nex_map, nex_out);
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
				const NexusSet&nex_map, NetBus&nex_out)
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
	    if (const NetExpr*base = lval_->get_base()) {
		  cerr << get_fileline() << ": NetAssignBase::synth_async: "
		       << "base_=" << *base << endl;
	    }
	    cerr << get_fileline() << ": NetAssignBase::synth_async: "
		 << "nex_map.size()==" << nex_map.size()
		 << ", nex_out.pin_count()==" << nex_out.pin_count() << endl;
      }

	/* For now, assume there is exactly one output. */
      ivl_assert(*this, nex_out.pin_count() == 1);
#if 0
      if (lval_->lwidth() != lsig->vector_width()) {
	    ivl_assert(*this, lval_->lwidth() < lsig->vector_width());

	    long base_off = 0;
	    if (! eval_as_long(base_off, lval_->get_base())) {
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
#endif
      connect(nex_out.pin(0), rsig->pin(0));

	/* This lval_ represents a reg that is a WIRE in the
	   synthesized results. This function signals the destructor
	   to change the REG that this l-value refers to into a
	   WIRE. It is done then, at the last minute, so that pending
	   synthesis can continue to work with it as a WIRE. */
      lval_->turn_sig_to_wire_on_release();

      return true;
}

/*
 * Sequential blocks are translated to asynchronous logic by
 * translating each statement of the block, in order, into gates. The
 * nex_out for the block is the union of the nex_out for all the
 * substatements.
 */
bool NetBlock::synth_async(Design*des, NetScope*scope,
			   const NexusSet&nex_map, NetBus&nex_out)
{
      if (last_ == 0) {
	    return true;
      }

      bool flag = true;
      NetProc*cur = last_;
      do {
	    cur = cur->next_;

	      /* Create a temporary map of the output only from this
		 statement. */
	    NexusSet tmp_map;
	    cur->nex_output(tmp_map);

	      /* Create also a temporary NetBus to collect the
		 output from the synthesis. */
	    NetBus tmp_out (scope, tmp_map.size());

	    bool ok_flag = cur->synth_async(des, scope, tmp_map, tmp_out);

	    flag = flag && ok_flag;
	    if (ok_flag == false)
		  continue;

	      /* Now find the tmp_map pins in the nex_map global map,
		 and use that to direct the connection to the nex_out
		 output bus. Look for the nex_map pin that is linked
		 to the tmp_map.pin(idx) pin, and link that to the
		 tmp_out.pin(idx) output link. */
	    for (unsigned idx = 0 ;  idx < tmp_out.pin_count() ;  idx += 1) {
		  unsigned ptr = nex_map.find_nexus(tmp_map[idx]);
		  ivl_assert(*this, ptr < nex_out.pin_count());
		  connect(nex_out.pin(ptr), tmp_out.pin(idx));
	    }

      } while (cur != last_);

      return flag;
}

bool NetCase::synth_async(Design*des, NetScope*scope,
			  const NexusSet&nex_map, NetBus&nex_out)
{
	/* Synthesize the select expression. */
      NetNet*esig = expr_->synthesize(des, scope, expr_);

      unsigned sel_width = esig->vector_width();
      assert(sel_width > 0);

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


	/* Collect all the statements into a map of index to
	   statement. The guard expression it evaluated to be the
	   index of the mux value, and the statement is bound to that
	   index. */

      unsigned long max_guard_value = 0;
      map<unsigned long,NetProc*>statement_map;
      NetProc*statement_default = 0;

      for (unsigned item = 0 ;  item < nitems_ ;  item += 1) {
	    if (items_[item].guard == 0) {
		  statement_default = items_[item].statement;
		  continue;
	    }

	    NetEConst*ge = dynamic_cast<NetEConst*>(items_[item].guard);
	    assert(ge);
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


	/* If there is a default clause, synthesize is once and we'll
	   link it in wherever it is needed. */
      NetBus default_bus (scope, nex_map.size());
      vector<NetNet*>default_sig (nex_map.size());

      if (statement_default) {

	    statement_default->synth_async(des, scope, nex_map, default_bus);

	      // Get the signal from the synthesized statement. This
	      // will be hooked to all the default cases.
	    ivl_assert(*this, default_bus.pin_count()==1);
	    default_sig[0] = default_bus.pin(0).nexus()->pick_any_net();
	    ivl_assert(*this, default_sig[0]);
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
		  ivl_assert(*this, default_sig.size() == mux.size());
		  for (size_t mdx = 0 ; mdx < mux.size() ; mdx += 1)
			connect(mux[mdx]->pin_Data(idx), default_sig[mdx]->pin(0));

		  continue;
	    }
	    if (stmt == 0) {
		  cerr << get_fileline() << ": error: case " << idx
		       << " is not accounted for in asynchronous mux." << endl;
		  des->errors += 1;
		  continue;
	    }

	    NetBus tmp (scope, nex_map.size());
	    stmt->synth_async(des, scope, nex_map, tmp);

	    ivl_assert(*this, tmp.pin_count() == mux.size());
	    for (size_t mdx = 0 ; mdx < mux.size() ; mdx += 1) {
		  connect(mux[mdx]->pin_Data(idx), tmp.pin(mdx));

		  if (mux[mdx]->pin_Data(idx).nexus()->pick_any_net()==0) {
			cerr << get_fileline() << ": warning: case " << idx
			     << " has no input for mux " << mdx << "." << endl;

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

bool NetCondit::synth_async(Design*des, NetScope*scope,
			    const NexusSet&nex_map, NetBus&nex_out)
{
      if (if_ == 0) {
	    return false;
      }
      if (else_ == 0) {
	    cerr << get_fileline() << ": error: Asynchronous if statement"
		 << " is missing the else clause." << endl;
	    return false;
      }

      assert(if_ != 0);
      assert(else_ != 0);

	// Synthesize the condition. This will act as a select signal
	// for a binary mux.
      NetNet*ssig = expr_->synthesize(des, scope, expr_);
      assert(ssig);

      bool flag;
      NetBus asig(scope, nex_out.pin_count());
      flag = if_->synth_async(des, scope, nex_map, asig);
      if (!flag) {
	    return false;
      }

      NetBus bsig(scope, nex_out.pin_count());
      flag = else_->synth_async(des, scope, nex_map, bsig);
      if (!flag) {
	    return false;
      }

      ivl_assert(*this, nex_out.pin_count()==asig.pin_count());
      ivl_assert(*this, nex_out.pin_count()==bsig.pin_count());

      for (unsigned idx = 0 ; idx < nex_out.pin_count() ; idx += 1) {
	    ivl_assert(*this, asig.pin(idx).nexus()->pick_any_net());
	    ivl_assert(*this, bsig.pin(idx).nexus()->pick_any_net());
	      // Guess the mux type from the type of the output.
	    ivl_variable_type_t mux_data_type = IVL_VT_LOGIC;
	    if (NetNet*tmp = nex_out.pin(idx).nexus()->pick_any_net()) {
		  mux_data_type = tmp->data_type();
	    }
	    unsigned mux_width = asig.pin(idx).nexus()->vector_width();
	    ivl_assert(*this, mux_width != 0);
	    ivl_assert(*this, mux_width==bsig.pin(idx).nexus()->vector_width());

	    NetMux*mux = new NetMux(scope, scope->local_symbol(),
				    mux_width, 2, 1);

	    list<netrange_t>not_an_array;
	    netvector_t*tmp_type;
	    if (mux_width==1)
		  tmp_type = new netvector_t(mux_data_type);
	    else
		  tmp_type = new netvector_t(mux_data_type, mux_width-1,0);

	      // Bind some temporary signals to carry pin type.
	    NetNet*otmp = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, not_an_array, tmp_type);
	    otmp->local_flag(true);
	    connect(nex_out.pin(idx),otmp->pin(0));

	    connect(mux->pin_Sel(),   ssig->pin(0));
	    connect(mux->pin_Data(1), asig.pin(idx));
	    connect(mux->pin_Data(0), bsig.pin(idx));
	    connect(nex_out.pin(idx), mux->pin_Result());

	    des->add_node(mux);
      }

      return true;
}

bool NetEvWait::synth_async(Design*des, NetScope*scope,
			    const NexusSet&nex_map, NetBus&nex_out)
{
      bool flag = statement_->synth_async(des, scope, nex_map, nex_out);
      return flag;
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
	    const NexusSet::elem_t&item = nex_set[idx];
	    if (item.base != 0 || item.wid!=item.nex->vector_width()) {
		  ivl_variable_type_t tmp_data_type = IVL_VT_LOGIC;
		  list<netrange_t>not_an_array;
		  netvector_t*tmp_type = new netvector_t(tmp_data_type, item.nex->vector_width()-1,0);
		  NetNet*tmp_sig = new NetNet(scope(), scope()->local_symbol(),
					      NetNet::WIRE, not_an_array, tmp_type);
		  tmp_sig->local_flag(true);

		  NetPartSelect*tmp = new NetPartSelect(tmp_sig, item.base,
							item.wid, NetPartSelect::PV);
		  des->add_node(tmp);
		  tmp->set_line(*this);
		  connect(tmp->pin(0), nex_q.pin(idx));
		  connect(item.nex, tmp_sig->pin(0));

	    } else {
		  connect(item.nex, nex_q.pin(idx));
	    }
      }

      bool flag = statement_->synth_async(des, scope(), nex_set, nex_q);
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
			  const NexusSet&nex_map, NetBus&nex_out,
			  const vector<NetEvProbe*>&events_in)
{
      bool flag = true;

      NetProc*cur = last_;
      do {
	    cur = cur->next_;

	      /* Create a temporary nex_map for the substatement. */
	    NexusSet tmp_set;
	    cur->nex_output(tmp_set);

	      /* NOTE: After this point, tmp_set should not be used as
		 the various functions I call do a lot of connecting,
		 and the nexa in the tmp_set may get realloced. Use
		 the tmp_map instead. */

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
			   const NexusSet&nex_map, NetBus&nex_out,
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
	    bool flag = synth_async(des, scope, nex_map, nex_out);
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
			   const NexusSet&nex_map, NetBus&nex_out,
			   const vector<NetEvProbe*>&events_in)
{
      if (events_in.size() > 0) {
	    cerr << get_fileline() << ": error: Events are unaccounted"
		 << " for in process synthesis." << endl;
	    des->errors += 1;
      }

      assert(events_in.size() == 0);

	/* This can't be other than one unless there are named events,
	   which I cannot synthesize. */
      assert(nevents_ == 1);
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
	    cerr << get_fileline() << ": debug: "
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
	    connect(nex_set[idx].nex, nex_q.pin(idx));
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

	    ivl_assert(*this, nex_set[idx].nex);
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

      if (top->is_synchronous()) do {
	    bool flag = top->synth_sync(des);
	    if (! flag) {
		  cerr << top->get_fileline() << ": error: "
		       << "Unable to synthesize synchronous process." << endl;
		  des->errors += 1;
		  return;
	    }
	    des->delete_process(top);
	    return;
      } while (0);

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
