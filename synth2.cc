/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: synth2.cc,v 1.24 2003/03/25 04:04:29 steve Exp $"
#endif

# include "config.h"

# include  "functor.h"
# include  "netlist.h"
# include  "compiler.h"
# include  <assert.h>

bool NetProc::synth_async(Design*des, NetScope*scope,
			  const NetNet*nex_map, NetNet*nex_out)
{
      return false;
}

bool NetProc::synth_sync(Design*des, NetScope*scope, NetFF*ff,
			 const NetNet*nex_map, NetNet*nex_out,
			 const svector<NetEvProbe*>&events)
{
      if (events.count() > 0) {
	    cerr << get_line() << ": error: Events are unaccounted"
		 << " for in process synthesis." << endl;
	    des->errors += 1;
      }

	/* Synthesize the input to the DFF. */
      return synth_async(des, scope, nex_map, nex_out);
}

static unsigned find_nexus_in_set(const NetNet*nset, const Nexus*nex)
{
      unsigned idx = 0;
      for (idx = 0 ;  idx < nset->pin_count() ;  idx += 1)
	    if (nset->pin(idx).nexus() == nex)
		  return idx;

      return idx;
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
				const NetNet*nex_map, NetNet*nex_out)
{
      NetNet*rsig = rval_->synthesize(des);
      assert(rsig);

      NetNet*lsig = lval_->sig();
      assert(lsig);
      assert(lval_->more == 0);

      assert(lval_->lwidth() == nex_map->pin_count());
      assert(nex_map->pin_count() <= rsig->pin_count());

      for (unsigned idx = 0 ;  idx < lval_->lwidth() ;  idx += 1) {
	    unsigned off = lval_->get_loff()+idx;
	    unsigned ptr = find_nexus_in_set(nex_map, lsig->pin(off).nexus());
	    connect(nex_out->pin(ptr), rsig->pin(idx));
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
			   const NetNet*nex_map, NetNet*nex_out)
{
      if (last_ == 0)
	    return true;

      bool flag = true;
      NetProc*cur = last_;
      do {
	    cur = cur->next_;

	      /* Create a temporary nex_map for the substatement. */
	    NexusSet tmp_set;
	    cur->nex_output(tmp_set);
	    NetNet*tmp_map = new NetNet(scope, "tmp1", NetNet::WIRE,
					tmp_set.count());
	    for (unsigned idx = 0 ;  idx < tmp_map->pin_count() ;  idx += 1)
		  connect(tmp_set[idx], tmp_map->pin(idx));

	      /* Create also a temporary net_out to collect the
		 output. */
	    NetNet*tmp_out = new NetNet(scope, "tmp2", NetNet::WIRE,
					tmp_set.count());

	    bool ok_flag = cur->synth_async(des, scope, tmp_map, tmp_out);
	    flag = flag && ok_flag;

	    if (ok_flag == false)
		  continue;

	      /* Use the nex_map to link up the output from the
		 substatement to the output of the block as a whole. */
	    for (unsigned idx = 0 ;  idx < tmp_out->pin_count() ; idx += 1) {
		  unsigned ptr = find_nexus_in_set(nex_map, tmp_set[idx]);
		  connect(nex_out->pin(ptr), tmp_out->pin(idx));
	    }

	    delete tmp_map;
	    delete tmp_out;

      } while (cur != last_);

      return flag;
}

bool NetCase::synth_async(Design*des, NetScope*scope,
			  const NetNet*nex_map, NetNet*nex_out)
{
      unsigned cur;

      NetNet*esig = expr_->synthesize(des);

	/* Scan the select vector looking for constant bits. The
	   constant bits will be elided from the select input connect,
	   but we still need to keep track of them. */
      unsigned sel_pins = 0;
      unsigned long sel_mask = 0;
      unsigned long sel_ref = 0;
      for (unsigned idx = 0 ;  idx < esig->pin_count() ;  idx += 1) {

	    if (esig->pin(idx).nexus()->drivers_constant()) {
		  verinum::V bit = esig->pin(idx).nexus()->driven_value();
		  if (bit == verinum::V1)
			sel_ref |= 1 << idx;

	    } else {
		  sel_pins += 1;
		  sel_mask |= 1 << idx;
	    }
      }

	/* Build a map of guard values to mux select values. This
	   helps account for constant select bits that are being
	   elided. */
      map<unsigned long,unsigned long>guard2sel;
      cur = 0;
      for (unsigned idx = 0 ;  idx < (1<<esig->pin_count()) ;  idx += 1) {
	    if ((idx & ~sel_mask) == sel_ref) {
		  guard2sel[idx] = cur;
		  cur += 1;
	    }
      }
      assert(cur == (1 << sel_pins));

      NetMux*mux = new NetMux(scope, scope->local_symbol(),
			      nex_out->pin_count(),
			      1 << sel_pins, sel_pins);

	/* Connect the non-constant select bits to the select input of
	   the mux device. */
      cur = 0;
      for (unsigned idx = 0 ;  idx < esig->pin_count() ;  idx += 1) {
	      /* skip bits that are known to be constant. */
	    if ((sel_mask & (1 << idx)) == 0)
		  continue;

	    connect(mux->pin_Sel(cur), esig->pin(idx));
	    cur += 1;
      }
      assert(cur == sel_pins);

	/* Hook up the output of th mux to the mapped output pins. */
      for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1)
	    connect(nex_out->pin(idx), mux->pin_Result(idx));

      NetProc**statement_map = new NetProc*[1 << sel_pins];
      for (unsigned item = 0 ;  item < (1<<sel_pins) ;  item += 1)
	    statement_map[item] = 0;

	/* Assign the input statements to MUX inputs. This involves
	   calculating the guard value, passing that through the
	   guard2sel map, then saving the statement in the
	   statement_map. If I find a default case, then save that for
	   use later. */
      NetProc*default_statement = 0;
      for (unsigned item = 0 ;  item < nitems_ ;  item += 1) {
	      /* Skip the default case this pass. */
	    if (items_[item].guard == 0) {
		  default_statement = items_[item].statement;
		  continue;
	    }

	    NetEConst*ge = dynamic_cast<NetEConst*>(items_[item].guard);
	    assert(ge);
	    verinum gval = ge->value();
	    unsigned sel_idx = guard2sel[gval.as_ulong()];

	    assert(items_[item].statement);
	    statement_map[sel_idx] = items_[item].statement;
      }

	/* Now that statements matches with mux inputs, synthesize the
	   sub-statements. If I get to an input that has no statement,
	   then use the default statement there. */
      NetNet*default_sig = 0;
      for (unsigned item = 0 ;  item < (1<<sel_pins) ;  item += 1) {

	      /* Detect the case that this is a default input, and I
		 have a precalculated default_sig. */
	    if ((statement_map[item] == 0) && (default_sig != 0)) {
		for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1)
		      connect(mux->pin_Data(idx, item), default_sig->pin(idx));
		continue;
	    }

	    NetNet*sig = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, nex_map->pin_count());
	    sig->local_flag(true);

	    if (statement_map[item] == 0) {
		  statement_map[item] = default_statement;
		  default_statement = 0;
		  default_sig = sig;
	    }

	    assert(statement_map[item]);
	    statement_map[item]->synth_async(des, scope, nex_map, sig);

	    for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1)
		  connect(mux->pin_Data(idx, item), sig->pin(idx));
      }
	    
      delete[]statement_map;
      des->add_node(mux);

      return true;
}

bool NetCondit::synth_async(Design*des, NetScope*scope,
			    const NetNet*nex_map, NetNet*nex_out)
{
      NetNet*ssig = expr_->synthesize(des);
      assert(ssig);

      if (if_ == 0)
	    return false;
      if (else_ == 0) {
	    cerr << get_line() << ": error: Asynchronous if statement"
		 << " is missing the else clause." << endl;
	    return false;
      }

      assert(if_ != 0);
      assert(else_ != 0);

      NetNet*asig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, nex_map->pin_count());
      asig->local_flag(true);

      if_->synth_async(des, scope, nex_map, asig);

      NetNet*bsig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, nex_map->pin_count());
      bsig->local_flag(true);

      else_->synth_async(des, scope, nex_map, bsig);

      NetMux*mux = new NetMux(scope, scope->local_symbol(),
			      nex_out->pin_count(), 2, 1);

      connect(mux->pin_Sel(0), ssig->pin(0));

      for (unsigned idx = 0 ;  idx < asig->pin_count() ;  idx += 1)
	    connect(mux->pin_Data(idx, 1), asig->pin(idx));

      for (unsigned idx = 0 ;  idx < bsig->pin_count() ;  idx += 1)
	    connect(mux->pin_Data(idx, 0), bsig->pin(idx));

      for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1)
	    connect(nex_out->pin(idx), mux->pin_Result(idx));

      des->add_node(mux);

      return true;
}

bool NetEvWait::synth_async(Design*des, NetScope*scope,
			    const NetNet*nex_map, NetNet*nex_out)
{
      return statement_->synth_async(des, scope, nex_map, nex_out);
}

bool NetProcTop::synth_async(Design*des)
{
      NexusSet nex_set;
      statement_->nex_output(nex_set);

      NetNet*nex_out = new NetNet(scope(), "tmp", NetNet::WIRE,
				  nex_set.count());
      for (unsigned idx = 0 ;  idx < nex_out->pin_count() ;  idx += 1)
	    connect(nex_set[idx], nex_out->pin(idx));

      bool flag = statement_->synth_async(des, scope(), nex_out, nex_out);

      delete nex_out;
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
bool NetBlock::synth_sync(Design*des, NetScope*scope, NetFF*ff,
			   const NetNet*nex_map, NetNet*nex_out,
			   const svector<NetEvProbe*>&events_in)
{
      if (last_ == 0)
	    return true;

      bool flag = true;

	/* Keep an accounting of which statement accounts for which
	   bit slice of the FF bank. This is used for error checking. */
      NetProc**pin_accounting = new NetProc* [ff->pin_count()];
      for (unsigned idx = 0 ;  idx < ff->pin_count() ;  idx += 1)
	    pin_accounting[idx] = 0;

      NetProc*cur = last_;
      do {
	    cur = cur->next_;

	      /* Create a temporary nex_map for the substatement. */
	    NexusSet tmp_set;
	    cur->nex_output(tmp_set);
	    NetNet*tmp_map = new NetNet(scope, "tmp1", NetNet::WIRE,
					tmp_set.count());
	    for (unsigned idx = 0 ;  idx < tmp_map->pin_count() ;  idx += 1)
		  connect(tmp_set[idx], tmp_map->pin(idx));

	      /* Create also a temporary net_out to collect the
		 output. The tmp1 and tmp2 map and out sets together
		 are used to collect the outputs from the substatement
		 for the inputs of the FF bank. */
	    NetNet*tmp_out = new NetNet(scope, "tmp2", NetNet::WIRE,
					tmp_set.count());

	    verinum tmp_aset = ff->aset_value();

	      /* Create a new DFF to handle this part of the begin-end
		 block. Connect this NetFF to the associated pins of
		 the existing wide NetFF device. While I'm at it, also
		 copy the aset_value bits for the new ff device. */
	    NetFF*ff2 = new NetFF(scope, scope->local_symbol().c_str(),
				  tmp_out->pin_count());
	    des->add_node(ff2);

	    verinum aset_value2 (verinum::V1, ff2->width());
	    for (unsigned idx = 0 ;  idx < ff2->width() ;  idx += 1) {
		  unsigned ptr = find_nexus_in_set(nex_map, tmp_set[idx]);
		  connect(ff->pin_Data(ptr), ff2->pin_Data(idx));
		  connect(ff->pin_Q(ptr), ff2->pin_Q(idx));

		  if (ptr < tmp_aset.len())
			aset_value2.set(idx, tmp_aset[ptr]);

		  if (pin_accounting[ptr] != 0) {
			cerr << cur->get_line() << ": error: "
			     << "Synchronous output conflicts with "
			     << pin_accounting[ptr]->get_line()
			     << "." << endl;
			flag = false;

		  } else {
			pin_accounting[ptr] = cur;
		  }
	    }
	    if (ff->pin_Aclr().is_linked())
		  connect(ff->pin_Aclr(),  ff2->pin_Aclr());
	    if (ff->pin_Aset().is_linked())
		  connect(ff->pin_Aset(),  ff2->pin_Aset());
	    if (ff->pin_Clock().is_linked())
		  connect(ff->pin_Clock(), ff2->pin_Clock());
	    if (ff->pin_Enable().is_linked())
		  connect(ff->pin_Enable(),ff2->pin_Enable());

	      /* Remember to store the aset value into the new FF. If
		 this leads to an Aset value of 0 (and Aclr is not
		 otherwise used) then move the Aset input to Aclr. */
	    if (tmp_aset.len() == ff->width()) {

		  if ((aset_value2.as_ulong() == 0)
		      && ff2->pin_Aset().is_linked()
		      && !ff2->pin_Aclr().is_linked()) {

			connect(ff2->pin_Aclr(), ff2->pin_Aset());
			ff2->pin_Aset().unlink();

		  } else {
			ff2->aset_value(aset_value2);
		  }
	    }

	      /* Now go on with the synchronous synthesis for this
		 subset of the statement. */
	    bool ok_flag = cur->synth_sync(des, scope, ff2, tmp_map,
					   tmp_out, events_in);
	    flag = flag && ok_flag;

	    if (ok_flag == false)
		  continue;

	      /* Use the nex_map to link up the output from the
		 substatement to the output of the block as a
		 whole. It is occasionally possible to have outputs
		 beyond the input set, for example when the l-value of
		 an assignment is smaller then the r-value. */
	    for (unsigned idx = 0 ;  idx < tmp_out->pin_count() ; idx += 1) {
		  unsigned ptr = find_nexus_in_set(nex_map, tmp_set[idx]);
		  if (ptr < nex_out->pin_count())
			connect(nex_out->pin(ptr), tmp_out->pin(idx));
	    }

	    delete tmp_map;
	    delete tmp_out;

      } while (cur != last_);

      delete[]pin_accounting;

	/* Done. The large NetFF is no longer needed, as it has been
	   taken up by the smaller NetFF devices. */
      delete ff;

      return flag;
}

/*
 * This method handles the case where I find a conditional near the
 * surface of a synchronous thread. This conditional can be a CE or an
 * asynchronous set/reset, depending on whether the pin of the
 * expression is connected to an event, or not.
 */
bool NetCondit::synth_sync(Design*des, NetScope*scope, NetFF*ff,
			   const NetNet*nex_map, NetNet*nex_out,
			   const svector<NetEvProbe*>&events_in)
{

	/* Synthesize the enable expression. */
      NetNet*ce = expr_->synthesize(des);
      assert(ce->pin_count() == 1);

	/* Try first to turn the ce into an asynchronous set/reset
	   input. If the ce is linked to a probe, then that probe is a
	   set/reset input. */
      for (unsigned idx = 0 ;  idx < events_in.count() ;  idx += 1) {
	    NetEvProbe*ev = events_in[idx];

	    if (connected(ce->pin(0), ev->pin(0))) {

		  bool flag = true;
		  assert(ev->edge() == NetEvProbe::POSEDGE);

		    /* Synthesize the true clause to figure out what
		       kind of set/reset we have. */
		  NetNet*asig = new NetNet(scope, scope->local_symbol(),
					   NetNet::WIRE, nex_map->pin_count());
		  asig->local_flag(true);
		  flag = if_->synth_async(des, scope, nex_map, asig) && flag;

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
		  if (tmp.as_ulong() == 0) {
			connect(ff->pin_Aclr(), ce->pin(0));

		  } else {
			connect(ff->pin_Aset(), ce->pin(0));
			ff->aset_value(tmp);
		  }

		  delete asig;

		  assert(events_in.count() == 1);
		  return else_->synth_sync(des, scope, ff, nex_map,
					   nex_out, svector<NetEvProbe*>(0))
			&& flag;
	    }

      }

	/* Failed to find an asynchronous set/reset, so any events
	   input are probably in error. */
      if (events_in.count() > 0) {
	    cerr << get_line() << ": error: Events are unaccounted"
		 << " for in process synthesis." << endl;
	    des->errors += 1;
      }

	/* If this is an if/then/else, then it is likely a
	   combinational if, and I should synthesize it that way. */
      if (if_ && else_) {
	    return synth_async(des, scope, nex_map, nex_out);
      }

      assert(if_);
      assert(!else_);

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

      if (ff->pin_Enable().is_linked()) {
	    NetLogic*ce_and = new NetLogic(scope,
					   scope->local_hsymbol(), 3,
					   NetLogic::AND);
	    des->add_node(ce_and);
	    connect(ff->pin_Enable(), ce_and->pin(1));
	    connect(ce->pin(0), ce_and->pin(2));

	    ff->pin_Enable().unlink();
	    connect(ff->pin_Enable(), ce_and->pin(0));

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, 1);
	    tmp->local_flag(true);
	    connect(ff->pin_Enable(), tmp->pin(0));

      } else {

	    connect(ff->pin_Enable(), ce->pin(0));
      }

      bool flag = if_->synth_sync(des, scope, ff, nex_map, nex_out, events_in);
      if (flag == false)
	    return flag;

      return true;
}

bool NetEvWait::synth_sync(Design*des, NetScope*scope, NetFF*ff,
			   const NetNet*nex_map, NetNet*nex_out,
			   const svector<NetEvProbe*>&events_in)
{
      if (events_in.count() > 0) {
	    cerr << get_line() << ": error: Events are unaccounted"
		 << " for in process synthesis." << endl;
	    des->errors += 1;
      }

      assert(events_in.count() == 0);

	/* This can't be other then one unless there are named events,
	   which I cannot synthesize. */
      assert(nevents_ == 1);
      NetEvent*ev = events_[0];

      assert(ev->nprobe() >= 1);
      svector<NetEvProbe*>events (ev->nprobe() - 1);

	/* Get the input set from the substatement. This will be used
	   to figure out which of the probes in the clock. */
      NexusSet*statement_input = statement_ -> nex_input();

	/* Search for a clock input. The clock input is the edge event
	   that is not also an input to the substatement. */
      NetEvProbe*pclk = 0;
      unsigned event_idx = 0;
      for (unsigned idx = 0 ;  idx < ev->nprobe() ;  idx += 1) {
	    NetEvProbe*tmp = ev->probe(idx);
	    assert(tmp->pin_count() == 1);

	    NexusSet tmp_nex;
	    tmp_nex .add( tmp->pin(0).nexus() );

	    if (! statement_input ->contains(tmp_nex)) {
		  if (pclk != 0) {
			cerr << get_line() << ": error: Too many "
			     << "clocks for synchronous logic." << endl;
			cerr << get_line() << ":      : Perhaps an"
			     << " asynchronous set/reset is misused?" << endl;
			des->errors += 1;
		  }
		  pclk = tmp;

	    } else {
		  events[event_idx++] = tmp;
	    }
      }

      if (pclk == 0) {
	    cerr << get_line() << ": error: None of the edges"
		 << " are valid clock inputs." << endl;
	    cerr << get_line() << ":      : Perhaps the clock"
		 << " is read by a statement or expression?" << endl;
	    return false;
      }

      connect(ff->pin_Clock(), pclk->pin(0));
      if (pclk->edge() == NetEvProbe::NEGEDGE)
	    ff->attribute("Clock:LPM_Polarity", verinum("INVERT"));

	/* Synthesize the input to the DFF. */
      bool flag = statement_->synth_sync(des, scope, ff,
					 nex_map, nex_out, events);

      return flag;
}

bool NetProcTop::synth_sync(Design*des)
{
      NexusSet nex_set;
      statement_->nex_output(nex_set);

      NetFF*ff = new NetFF(scope(), scope()->local_symbol().c_str(),
			   nex_set.count());
      des->add_node(ff);
      ff->attribute("LPM_FFType", verinum("DFF"));

	/* The D inputs to the DFF device will receive the output from
	   the statements of the process. */
      NetNet*nex_d = new NetNet(scope(), scope()->local_symbol(),
				NetNet::WIRE, nex_set.count());
      nex_d->local_flag(true);
      for (unsigned idx = 0 ;  idx < nex_set.count() ;  idx += 1) {
	    connect(nex_d->pin(idx), ff->pin_Data(idx));
      }

	/* The Q outputs of the DFF will connect to the actual outputs
	   of the process. Thus, the DFF will be between the outputs
	   of the process and the outputs of the substatement. */
      NetNet*nex_q = new NetNet(scope(), "tmpq", NetNet::WIRE,
				nex_set.count());
      for (unsigned idx = 0 ;  idx < nex_set.count() ;  idx += 1) {
	    connect(nex_set[idx], nex_q->pin(idx));
	    connect(nex_q->pin(idx), ff->pin_Q(idx));
      }

	/* Synthesize the input to the DFF. */
      bool flag = statement_->synth_sync(des, scope(), ff,
					 nex_q, nex_d,
					 svector<NetEvProbe*>());

      delete nex_q;

      return flag;
}

class synth2_f  : public functor_t {

    public:
      void process(class Design*, class NetProcTop*);

    private:
};


/*
 * Look at a process. If it is asynchronous, then synthesize it as an
 * asynchronous process and delete the process itself for its gates.
 */
void synth2_f::process(class Design*des, class NetProcTop*top)
{
      if (top->attribute("ivl_synthesis_off").as_ulong() != 0)
	    return;

      if (top->is_synchronous()) do {
	    bool flag = top->synth_sync(des);
	    if (! flag) {
		  cerr << top->get_line() << ": error: "
		       << "Unable to synthesize synchronous process." << endl;
		  des->errors += 1;
		  return;
	    }
	    des->delete_process(top);
	    return;
      } while (0);

      if (! top->is_asynchronous()) {
	    bool synth_error_flag = false;
	    if (top->attribute("ivl_combinational").as_ulong() != 0) {
		  cerr << top->get_line() << ": error: "
		       << "Process is marked combinational,"
		       << " but isn't really." << endl;
		  des->errors += 1;
		  synth_error_flag = true;
	    }

	    if (top->attribute("ivl_synthesis_on").as_ulong() != 0) {
		  cerr << top->get_line() << ": error: "
		       << "Process is marked for synthesis,"
		       << " but I can't do it." << endl;
		  des->errors += 1;
		  synth_error_flag = true;
	    }

	    if (! synth_error_flag)
		  cerr << top->get_line() << ": warning: "
		       << "Process not synthesized." << endl;

	    return;
      }

      if (! top->synth_async(des)) {
	    cerr << top->get_line() << ": internal error: "
		 << "is_asynchronous does not match "
		 << "sync_async results." << endl;
	    return;
      }

      des->delete_process(top);
}

void synth2(Design*des)
{
      synth2_f synth_obj;
      des->functor(&synth_obj);
}

/*
 * $Log: synth2.cc,v $
 * Revision 1.24  2003/03/25 04:04:29  steve
 *  Handle defaults in synthesized case statements.
 *
 * Revision 1.23  2003/03/06 00:28:42  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.22  2003/02/26 01:29:24  steve
 *  LPM objects store only their base names.
 *
 * Revision 1.21  2003/01/27 05:09:17  steve
 *  Spelling fixes.
 *
 * Revision 1.20  2002/11/09 23:29:29  steve
 *  Handle nested-if chip enables.
 *
 * Revision 1.19  2002/11/09 20:22:57  steve
 *  Detect synthesis conflicts blocks statements share outputs.
 *
 * Revision 1.18  2002/11/06 03:22:28  steve
 *  More forgiving about assignment rval width mismatch.
 *
 * Revision 1.17  2002/10/23 01:47:17  steve
 *  Fix synth2 handling of aset/aclr signals where
 *  flip-flops are split by begin-end blocks.
 *
 * Revision 1.16  2002/10/21 01:42:09  steve
 *  Synthesizer support for synchronous begin-end blocks.
 *
 * Revision 1.15  2002/10/20 19:19:37  steve
 *  Handle conditional error cases better.
 *
 * Revision 1.14  2002/09/26 03:42:10  steve
 *  Remove excess debug messages.
 *
 * Revision 1.13  2002/09/26 03:18:04  steve
 *  Generate vvp code for asynch set/reset of NetFF.
 *
 * Revision 1.12  2002/09/26 01:13:14  steve
 *  Synthesize async set/reset is certain cases.
 *
 * Revision 1.11  2002/09/24 00:58:35  steve
 *  More detailed check of process edge events.
 *
 * Revision 1.10  2002/09/17 04:40:28  steve
 *  Connect output of block to net_out, instead of statement outputs.
 *
 * Revision 1.9  2002/09/16 00:30:33  steve
 *  Add to synth2 support for synthesis of
 *  synchronous logic. This includes DFF enables
 *  modeled by if/then/else.
 *
 * Revision 1.8  2002/08/18 22:07:16  steve
 *  Detect temporaries in sequential block synthesis.
 *
 * Revision 1.7  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.6  2002/08/10 22:07:08  steve
 *  Observe attributes to control synthesis.
 *
 * Revision 1.5  2002/07/29 00:00:28  steve
 *  Asynchronous synthesis of sequential blocks.
 *
 * Revision 1.4  2002/07/16 04:40:48  steve
 *  Allow wide rvalues assigned to narrow nex_out.
 *
 * Revision 1.3  2002/07/07 22:32:15  steve
 *  Asynchronous synthesis of case statements.
 *
 * Revision 1.2  2002/07/01 00:54:21  steve
 *  synth_asych of if/else requires redirecting the target
 *  if sub-statements. Use NetNet objects to manage the
 *  situation.
 *
 * Revision 1.1  2002/06/30 02:21:32  steve
 *  Add structure for asynchronous logic synthesis.
 *
 */

