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
#ident "$Id: synth2.cc,v 1.2 2002/07/01 00:54:21 steve Exp $"
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

      assert(nex_map->pin_count() == rsig->pin_count());
      assert(lsig->pin_count() == rsig->pin_count());

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
	    unsigned ptr = find_nexus_in_set(nex_map, lsig->pin(idx).nexus());
	    connect(nex_out->pin(ptr), rsig->pin(idx));
      }

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
 * Revision 1.2  2002/07/01 00:54:21  steve
 *  synth_asych of if/else requires redirecting the target
 *  if sub-statements. Use NetNet objects to manage the
 *  situation.
 *
 * Revision 1.1  2002/06/30 02:21:32  steve
 *  Add structure for asynchronous logic synthesis.
 *
 */

