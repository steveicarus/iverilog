/*
 * Copyright (c) 2002-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: synth2.cc,v 1.37 2004/03/15 18:40:12 steve Exp $"
#endif

# include "config.h"

# include  "functor.h"
# include  "netlist.h"
# include  "compiler.h"
# include  <assert.h>

static int debug_synth2=0;

#ifdef __FUNCTION__

#define DEBUG_SYNTH2_ENTRY(class) if (debug_synth2) { cerr << "Enter " << class << "::" \
       	<< __FUNCTION__ << endl; dump(cerr, 4); }
#define DEBUG_SYNTH2_EXIT(class,val) if (debug_synth2) { cerr << "Exit " << class << "::" \
       	<< __FUNCTION__ << ", result " << val << endl; }

#else
#define DEBUG_SYNTH2_ENTRY(class)
#define DEBUG_SYNTH2_EXIT(class,val)
#endif

bool NetProc::synth_async(Design*des, NetScope*scope,
			  const NetNet*nex_map, NetNet*nex_out)
{
      return false;
}

bool NetProc::synth_sync(Design*des, NetScope*scope, NetFF*ff,
			 const NetNet*nex_map, NetNet*nex_out,
			 const svector<NetEvProbe*>&events)
{
      DEBUG_SYNTH2_ENTRY("NetProc")

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
      DEBUG_SYNTH2_ENTRY("NetAssignBase")
 
      NetNet*rsig = rval_->synthesize(des);
      assert(rsig);

      NetNet*lsig = lval_->sig();
      if (!lsig) {
	    cerr << get_line() << ": error: NetAssignBase::synth_async on unsupported lval ";
	    dump_lval(cerr);
	    cerr << endl;
	    DEBUG_SYNTH2_EXIT("NetAssignBase",false)
	    return false;
      }
      assert(lval_->more == 0);

      if (lval_->lwidth() != nex_map->pin_count()) {
	    cerr << get_line() << ": error: NetAssignBase::synth_async pin count mismatch, "
	         << lval_->lwidth() << " != " << nex_map->pin_count() << endl;
	    DEBUG_SYNTH2_EXIT("NetAssignBase",false)
	    return false;
      }
      assert(nex_map->pin_count() <= rsig->pin_count());

      for (unsigned idx = 0 ;  idx < lval_->lwidth() ;  idx += 1) {
	    unsigned off = lval_->get_loff()+idx;
	    unsigned ptr = find_nexus_in_set(nex_map, lsig->pin(off).nexus());
	    connect(nex_out->pin(ptr), rsig->pin(idx));
      }

      DEBUG_SYNTH2_EXIT("NetAssignBase",true)
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
      DEBUG_SYNTH2_ENTRY("NetBlock")
      if (last_ == 0) {
	    DEBUG_SYNTH2_EXIT("NetBlock",true)
	    return true;
      }

      const perm_string tmp1 = perm_string::literal("tmp1");
      const perm_string tmp2 = perm_string::literal("tmp2");

      bool flag = true;
      NetProc*cur = last_;
      do {
	    cur = cur->next_;

	      /* Create a temporary nex_map for the substatement. */
	    NexusSet tmp_set;
	    cur->nex_output(tmp_set);
	    NetNet*tmp_map = new NetNet(scope, tmp1, NetNet::WIRE,
					tmp_set.count());
	    for (unsigned idx = 0 ;  idx < tmp_map->pin_count() ;  idx += 1)
		  connect(tmp_set[idx], tmp_map->pin(idx));

	      /* Create also a temporary net_out to collect the
		 output. */
	    NetNet*tmp_out = new NetNet(scope, tmp2, NetNet::WIRE,
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

      DEBUG_SYNTH2_EXIT("NetBlock",flag)
      return flag;
}

bool NetCase::synth_async(Design*des, NetScope*scope,
			  const NetNet*nex_map, NetNet*nex_out)
{
      DEBUG_SYNTH2_ENTRY("NetCase")
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
      for (unsigned idx = 0 ;  idx < (1U<<esig->pin_count()) ;  idx += 1) {
	    if ((idx & ~sel_mask) == sel_ref) {
		  guard2sel[idx] = cur;
		  cur += 1;
	    }
      }
      assert(cur == (1U << sel_pins));

      NetMux*mux = new NetMux(scope, scope->local_symbol(),
			      nex_out->pin_count(),
			      1U << sel_pins, sel_pins);

	/* Connect the non-constant select bits to the select input of
	   the mux device. */
      cur = 0;
      for (unsigned idx = 0 ;  idx < esig->pin_count() ;  idx += 1) {
	      /* skip bits that are known to be constant. */
	    if ((sel_mask & (1U << idx)) == 0)
		  continue;

	    connect(mux->pin_Sel(cur), esig->pin(idx));
	    cur += 1;
      }
      assert(cur == sel_pins);

	/* Hook up the output of the mux to the mapped output pins. */
      for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1)
	    connect(nex_out->pin(idx), mux->pin_Result(idx));

      NetProc**statement_map = new NetProc*[1 << sel_pins];
      for (unsigned item = 0 ;  item < (1U<<sel_pins) ;  item += 1)
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

	/* Now that statements match with mux inputs, synthesize the
	   sub-statements. If I get to an input that has no statement,
	   then use the default statement there. */
      NetNet*default_sig = 0;
      for (unsigned item = 0 ;  item < (1U<<sel_pins) ;  item += 1) {

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

	    if (statement_map[item] == 0) {
		  /* Missing case and no default; this could still be
		   * synthesizable with synchronous logic, but not here. */
		  DEBUG_SYNTH2_EXIT("NetCase", false)
		  return false;
	    }
	    statement_map[item]->synth_async(des, scope, nex_map, sig);

	    for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1)
		  connect(mux->pin_Data(idx, item), sig->pin(idx));
      }
	    
      delete[]statement_map;
      des->add_node(mux);

      DEBUG_SYNTH2_EXIT("NetCase", true)
      return true;
}

bool NetCondit::synth_async(Design*des, NetScope*scope,
			    const NetNet*nex_map, NetNet*nex_out)
{
      DEBUG_SYNTH2_ENTRY("NetCondit")
      NetNet*ssig = expr_->synthesize(des);
      assert(ssig);

      if (if_ == 0) {
	    DEBUG_SYNTH2_EXIT("NetCondit",false)
	    return false;
      }
      if (else_ == 0) {
	    cerr << get_line() << ": error: Asynchronous if statement"
		 << " is missing the else clause." << endl;
	    DEBUG_SYNTH2_EXIT("NetCondit",false)
	    return false;
      }

      assert(if_ != 0);
      assert(else_ != 0);

      NetNet*asig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, nex_map->pin_count());
      asig->local_flag(true);

      bool flag;
      flag = if_->synth_async(des, scope, nex_map, asig);
      if (!flag) {
	    delete asig;
	    DEBUG_SYNTH2_EXIT("NetCondit",false)
	    return false;
      }
      
      NetNet*bsig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, nex_map->pin_count());
      bsig->local_flag(true);

      flag = else_->synth_async(des, scope, nex_map, bsig);
      if (!flag) {
	    delete asig;
	    delete bsig;
	    DEBUG_SYNTH2_EXIT("NetCondit",false)
	    return false;
      }

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

      DEBUG_SYNTH2_EXIT("NetCondit",true)
      return true;
}

bool NetEvWait::synth_async(Design*des, NetScope*scope,
			    const NetNet*nex_map, NetNet*nex_out)
{
      DEBUG_SYNTH2_ENTRY("NetEvWait")
      bool flag = statement_->synth_async(des, scope, nex_map, nex_out);
      DEBUG_SYNTH2_EXIT("NetEvWait",flag)
      return flag;
}

bool NetProcTop::synth_async(Design*des)
{
      DEBUG_SYNTH2_ENTRY("NetProcTop")
      NexusSet nex_set;
      statement_->nex_output(nex_set);

      const perm_string tmp1 = perm_string::literal("tmp");
      NetNet*nex_out = new NetNet(scope(), tmp1, NetNet::WIRE,
				  nex_set.count());
      for (unsigned idx = 0 ;  idx < nex_out->pin_count() ;  idx += 1)
	    connect(nex_set[idx], nex_out->pin(idx));

      bool flag = statement_->synth_async(des, scope(), nex_out, nex_out);

      delete nex_out;
      DEBUG_SYNTH2_EXIT("NetProcTop",flag)
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
      DEBUG_SYNTH2_ENTRY("NetBlock")
      if (last_ == 0) {
	    DEBUG_SYNTH2_EXIT("NetBlock",true)
	    return true;
      }

      bool flag = true;

      const perm_string tmp1 = perm_string::literal("tmp1");
      const perm_string tmp2 = perm_string::literal("tmp2");

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
	    NetNet*tmp_map = new NetNet(scope, tmp1, NetNet::WIRE,
					tmp_set.count());
	    for (unsigned idx = 0 ;  idx < tmp_map->pin_count() ;  idx += 1)
		  connect(tmp_set[idx], tmp_map->pin(idx));

	      /* NOTE: After this point, tmp_set should not be used as
		 the various functions I call do a lot of connecting,
		 and the nexa in the tmp_set may get realloced. Use
		 the tmp_map instead. */

	      /* Create also a temporary net_out to collect the
		 output. The tmp1 and tmp2 map and out sets together
		 are used to collect the outputs from the substatement
		 for the inputs of the FF bank. */
	    NetNet*tmp_out = new NetNet(scope, tmp2, NetNet::WIRE,
					tmp_map->pin_count());

	    verinum tmp_aset = ff->aset_value();
	    verinum tmp_sset = ff->sset_value();

	      /* Create a new DFF to handle this part of the begin-end
		 block. Connect this NetFF to the associated pins of
		 the existing wide NetFF device. While I'm at it, also
		 copy the aset_value bits for the new ff device. */
	    NetFF*ff2 = new NetFF(scope, scope->local_symbol(),
				  tmp_out->pin_count());
	    des->add_node(ff2);

	    verinum aset_value2 (verinum::V1, ff2->width());
	    verinum sset_value2 (verinum::V1, ff2->width());
	    for (unsigned idx = 0 ;  idx < ff2->width() ;  idx += 1) {
		  unsigned ptr = find_nexus_in_set(nex_map,
						   tmp_map->pin(idx).nexus());

		    /* Connect Data and Q bits to the new FF. */
		  connect(ff->pin_Data(ptr), ff2->pin_Data(idx));
		  connect(ff->pin_Q(ptr), ff2->pin_Q(idx));

		    /* Copy the asynch set bit to the new device. */
		  if (ptr < tmp_aset.len())
			aset_value2.set(idx, tmp_aset[ptr]);

		    /* Copy the synch set bit to the new device. */
		  if (ptr < tmp_sset.len())
			sset_value2.set(idx, tmp_sset[ptr]);

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
	    if (ff->pin_Sclr().is_linked())
		  connect(ff->pin_Sclr(),  ff2->pin_Sclr());
	    if (ff->pin_Sset().is_linked())
		  connect(ff->pin_Sset(),  ff2->pin_Sset());
	    if (ff->pin_Clock().is_linked())
		  connect(ff->pin_Clock(), ff2->pin_Clock());
	    if (ff->pin_Enable().is_linked())
		  connect(ff->pin_Enable(),ff2->pin_Enable());

	      /* Remember to store the aset value into the new FF. If
		 this leads to an Aset value of 0 (and Aclr is not
		 otherwise used) then move the Aset input to Aclr. */
	    if (tmp_aset.len() == ff->width()) {

		  if (aset_value2.is_zero()
		      && ff2->pin_Aset().is_linked()
		      && !ff2->pin_Aclr().is_linked()) {

			connect(ff2->pin_Aclr(), ff2->pin_Aset());
			ff2->pin_Aset().unlink();

		  } else {
			ff2->aset_value(aset_value2);
		  }
	    }

	      /* Now go on with the synchronous synthesis for this
		 subset of the statement. The tmp_map is the output
		 nexa that we expect, and the tmp_out is where we want
		 those outputs connected. */
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
		  unsigned ptr = find_nexus_in_set(nex_map,
						   tmp_map->pin(idx).nexus());

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

      DEBUG_SYNTH2_EXIT("NetBlock",flag)
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
      DEBUG_SYNTH2_ENTRY("NetCondit")
	/* First try to turn the condition expression into an
	   asynchronous set/reset. If the condition expression has
	   inputs that are included in the sensitivity list, then it
	   is likely intended as an asynchronous input. */

      NexusSet*expr_input = expr_->nex_input();
      assert(expr_input);
      for (unsigned idx = 0 ;  idx < events_in.count() ;  idx += 1) {

	    NetEvProbe*ev = events_in[idx];
	    NexusSet pin_set;
	    pin_set.add(ev->pin(0).nexus());

	    if (! expr_input->contains(pin_set))
		  continue;

	      /* Ah, this edge is in the sensitivity list for the
		 expression, so we have an asynchronous
		 input. Synthesize the set/reset input expression. */

	    NetNet*rst = expr_->synthesize(des);
	    assert(rst->pin_count() == 1);

	      /* XXXX I really should find a way to check that the
		 edge used on the reset input is correct. This would
		 involve interpreting the exression that is fed by the
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
      }

      delete expr_input;

	/* Detect the case that this is a *synchronous* set/reset. It
	   is not asyncronous because we know the condition is not
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
	    bool flag = synth_async(des, scope, nex_map, nex_out);
	    DEBUG_SYNTH2_EXIT("NetCondit",flag)
	    return flag;
      }

      assert(if_);
      assert(!else_);

	/* Synthesize the enable expression. */
      NetNet*ce = expr_->synthesize(des);
      assert(ce->pin_count() == 1);

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
					   scope->local_symbol(), 3,
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

      DEBUG_SYNTH2_EXIT("NetCondit",flag)
      return flag;
}

bool NetEvWait::synth_sync(Design*des, NetScope*scope, NetFF*ff,
			   const NetNet*nex_map, NetNet*nex_out,
			   const svector<NetEvProbe*>&events_in)
{
      DEBUG_SYNTH2_ENTRY("NetEvWait")
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
	    DEBUG_SYNTH2_EXIT("NetEvWait",false)
	    return false;
      }

      connect(ff->pin_Clock(), pclk->pin(0));
      if (pclk->edge() == NetEvProbe::NEGEDGE)
	    ff->attribute(perm_string::literal("Clock:LPM_Polarity"), verinum("INVERT"));

	/* Synthesize the input to the DFF. */
      bool flag = statement_->synth_sync(des, scope, ff,
					 nex_map, nex_out, events);

      DEBUG_SYNTH2_EXIT("NetEvWait",flag)
      return flag;
}

bool NetProcTop::synth_sync(Design*des)
{
      DEBUG_SYNTH2_ENTRY("NetProcTop")
      NexusSet nex_set;
      statement_->nex_output(nex_set);

      NetFF*ff = new NetFF(scope(), scope()->local_symbol(),
			   nex_set.count());
      des->add_node(ff);
      ff->attribute(perm_string::literal("LPM_FFType"), verinum("DFF"));

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
      const perm_string tmpq = perm_string::literal("tmpq");
      NetNet*nex_q = new NetNet(scope(), tmpq, NetNet::WIRE,
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

      DEBUG_SYNTH2_EXIT("NetProcTop",flag)
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
      if (top->attribute(perm_string::literal("ivl_synthesis_off")).as_ulong() != 0)
	    return;

	/* If the scope that contains this process as a cell attribute
	   attached to it, then skip synthesis. */
      if (top->scope()->attribute(perm_string::literal("ivl_synthesis_cell")).len() > 0)
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
	    if (top->attribute(perm_string::literal("ivl_combinational")).as_ulong() != 0) {
		  cerr << top->get_line() << ": error: "
		       << "Process is marked combinational,"
		       << " but isn't really." << endl;
		  des->errors += 1;
		  synth_error_flag = true;
	    }

	    if (top->attribute(perm_string::literal("ivl_synthesis_on")).as_ulong() != 0) {
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
      debug_synth2 = atoi(des->get_flag("ivl-synth2-debug"));
      synth2_f synth_obj;
      des->functor(&synth_obj);
}

/*
 * $Log: synth2.cc,v $
 * Revision 1.37  2004/03/15 18:40:12  steve
 *  Only include DEBUG_SYNTH2 if __FUNCTION__ defined.
 *
 * Revision 1.36  2004/02/20 18:53:35  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.35  2004/02/18 17:11:58  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.34  2003/12/20 00:59:31  steve
 *  Synthesis debug messages.
 *
 * Revision 1.33  2003/12/17 16:52:39  steve
 *  Debug dumps for synth2.
 *
 * Revision 1.32  2003/10/27 02:18:04  steve
 *  Handle special case of FF with enable and constant data.
 *
 * Revision 1.31  2003/08/28 04:11:19  steve
 *  Spelling patch.
 *
 * Revision 1.30  2003/08/15 02:23:53  steve
 *  Add synthesis support for synchronous reset.
 *
 * Revision 1.29  2003/08/14 02:41:05  steve
 *  Fix dangling pointer in NexusSet handling blocks.
 *
 * Revision 1.28  2003/08/10 17:04:23  steve
 *  Detect asynchronous FF inputs that are expressions.
 *
 * Revision 1.27  2003/06/23 00:14:44  steve
 *  ivl_synthesis_cell cuts off synthesis within a module.
 *
 * Revision 1.26  2003/06/21 01:21:43  steve
 *  Harmless fixup of warnings.
 *
 * Revision 1.25  2003/04/03 04:30:00  steve
 *  Prevent overrun comparing verinums to zero.
 *
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
 */

