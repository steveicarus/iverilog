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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: synth2.cc,v 1.4 2002/07/16 04:40:48 steve Exp $"
#endif

# include "config.h"

# include  "functor.h"
# include  "netlist.h"
# include  "compiler.h"
# include  <assert.h>

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

      assert(lsig->pin_count() == nex_map->pin_count());
      assert(nex_map->pin_count() <= rsig->pin_count());

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
	    unsigned ptr = find_nexus_in_set(nex_map, lsig->pin(idx).nexus());
	    connect(nex_out->pin(ptr), rsig->pin(idx));
      }

      return true;
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

      NetMux*mux = new NetMux(scope, scope->local_hsymbol(),
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

      for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1)
	    connect(nex_out->pin(idx), mux->pin_Result(idx));

      for (unsigned item = 0 ;  item < nitems_ ;  item += 1) {
	    assert(items_[item].guard);
	    assert(items_[item].statement);

	    NetEConst*ge = dynamic_cast<NetEConst*>(items_[item].guard);
	    assert(ge);
	    verinum gval = ge->value();
	    unsigned sel_idx = guard2sel[gval.as_ulong()];

	    NetNet*sig = new NetNet(scope, scope->local_hsymbol(),
				    NetNet::WIRE, nex_map->pin_count());
	    sig->local_flag(true);
	    items_[item].statement->synth_async(des, scope, nex_map, sig);

	    for (unsigned idx = 0 ;  idx < mux->width() ;  idx += 1)
		  connect(mux->pin_Data(idx, sel_idx), sig->pin(idx));
      }

      des->add_node(mux);

      return true;
}

bool NetCondit::synth_async(Design*des, NetScope*scope,
			    const NetNet*nex_map, NetNet*nex_out)
{
      NetNet*ssig = expr_->synthesize(des);
      assert(ssig);

      assert(if_ != 0);
      assert(else_ != 0);

      NetNet*asig = new NetNet(scope, scope->local_hsymbol(),
			       NetNet::WIRE, nex_map->pin_count());
      asig->local_flag(true);

      if_->synth_async(des, scope, nex_map, asig);

      NetNet*bsig = new NetNet(scope, scope->local_hsymbol(),
			       NetNet::WIRE, nex_map->pin_count());
      bsig->local_flag(true);

      else_->synth_async(des, scope, nex_map, bsig);

      NetMux*mux = new NetMux(scope, scope->local_hsymbol(),
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

bool NetProc::synth_async(Design*des, NetScope*scope,
			  const NetNet*nex_map, NetNet*nex_out)
{
      return false;
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
      if (! top->is_asynchronous()) {
	    if (top->attribute("asynchronous").as_ulong() != 0)
		  cerr << top->get_line() << ": warning: "
		       << "Process is marked asynchronous,"
		       << " but isn't really." << endl;
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

