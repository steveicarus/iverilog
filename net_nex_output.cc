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
#ident "$Id: net_nex_output.cc,v 1.11.2.8 2006/08/08 02:17:48 steve Exp $"
#endif

# include "config.h"

# include <iostream>

# include  <cassert>
# include  <typeinfo>
# include  "netlist.h"
# include  "netmisc.h"

void NetProc::nex_output(NexusSet&out)
{
      cerr << get_line()
	   << ": internal error: NetProc::nex_output not implemented"
	   << endl;
      cerr << get_line()
	   << ":               : on object type " << typeid(*this).name()
	   << endl;
}

/*
 * Assignments have as output all the bits of the concatenated signals
 * of the l-value.
 */
void NetAssignBase::nex_output(NexusSet&out)
{
      for (NetAssign_*cur = lval_ ;  cur ;  cur = cur->more) {
	    if (NetNet*lsig = cur->sig()) {

		  if (cur->bmux()) {
			for (unsigned idx = 0; idx < lsig->pin_count(); idx += 1) {
			      out.add(lsig->pin(idx).nexus());
			}
			continue;
		  }

		    /* Handle l-value signals. We don't need to worry
		       here about whether there is a bmux, because the
		       synthesizer will detect that mux and create a
		       decoder between the expression and the signal. */
		  for (unsigned idx = 0 ;  idx < cur->lwidth() ;  idx += 1) {
			unsigned off = cur->get_loff() + idx;
			out.add(lsig->pin(off).nexus());
		  }

	    } else if (NetMemory*lmem = cur->mem()) {

		    /* Memories here are treated as a bunch of reg
		       devices. Use the explode_to_reg method to get
		       access to the FF version of the memory and use
		       that in our l-value management. */
		  NetNet*tmp = lmem->explode_to_reg();

		  if (NetEConst*ae = dynamic_cast<NetEConst*>(cur->bmux())) {
			  /* The address is constant, so simply
			     connect to the right pins and we are
			     done. */
			long adr_s = ae->value().as_long();
			unsigned adr = lmem->index_to_address(adr_s) * lmem->width();

			if (adr >= lmem->count()*lmem->width()) {
				/* Skip assignments with constant
				   addresses that are outside the
				   range of memories. */
			} else {
			      for (unsigned idx=0; idx<cur->lwidth(); idx += 1)
				    out.add(tmp->pin(adr+idx).nexus());
			}

		  } else {
			  /* Put all the bits of the memory into the
			     set. The synthesis will generate a
			     decoder to handle this. */
			for (unsigned idx = 0; idx < tmp->pin_count(); idx+=1)
			      out.add(tmp->pin(idx).nexus());
		  }

	    } else {
		    /* Quoting from netlist.h comments for class NetMemory:
		     * "This is not a node because memory objects can only be
		     * accessed by behavioral code."
		     */
		  cerr << get_line() << ": internal error: "
		       << "NetAssignBase::nex_output on unsupported lval ";
		  dump_lval(cerr);
		  cerr << endl;
	    }
      }
}

void NetBlock::nex_output(NexusSet&out)
{
      if (last_ == 0)
	    return;

      NetProc*cur = last_->next_;
      do {
	    cur->nex_output(out);
	    cur = cur->next_;
      } while (cur != last_->next_);
}

void NetCase::nex_output(NexusSet&out)
{
      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {

	    assert(items_[idx].statement);
	    items_[idx].statement->nex_output(out);
      }

}

void NetCondit::nex_output(NexusSet&out)
{
      if (if_ != 0)
	    if_->nex_output(out);
      if (else_ != 0)
	    else_->nex_output(out);
}

void NetEvWait::nex_output(NexusSet&out)
{
      assert(statement_);
      statement_->nex_output(out);
}

void NetPDelay::nex_output(NexusSet&out)
{
      if (statement_) statement_->nex_output(out);
}

/*
 * For the purposes of synthesis, system task calls have no output at
 * all. This is OK because most system tasks are not synthesizeable in
 * the first place.
 */
void NetSTask::nex_output(NexusSet&out)
{
}

void NetWhile::nex_output(NexusSet&out)
{
      if (proc_ != 0)
	    proc_->nex_output(out);
}

/*
 * $Log: net_nex_output.cc,v $
 * Revision 1.11.2.8  2006/08/08 02:17:48  steve
 *  Improved nexus management performance.
 *
 * Revision 1.11.2.7  2006/06/02 23:42:48  steve
 *  Compilation warnings.
 *
 * Revision 1.11.2.6  2006/06/01 03:01:48  steve
 *  Handle condit clauses with unassigned outputs.
 *
 * Revision 1.11.2.5  2006/05/18 01:47:12  steve
 *  Fix synthesis of l-value bit select in block.
 *
 * Revision 1.11.2.4  2006/05/05 01:56:36  steve
 *  Handle memory assignments out of range during synthesis
 *
 * Revision 1.11.2.3  2006/04/16 19:26:38  steve
 *  Fix handling of exploded memories with partial or missing resets.
 *
 * Revision 1.11.2.2  2006/03/12 07:34:17  steve
 *  Fix the memsynth1 case.
 *
 * Revision 1.11.2.1  2006/01/18 01:23:23  steve
 *  Rework l-value handling to allow for more l-value type flexibility.
 *
 * Revision 1.11  2004/09/16 03:17:33  steve
 *  net_output handles l-value concatenations.
 *
 * Revision 1.10  2004/08/28 16:23:05  steve
 *  Fix use of system tasks in AT_STAR statements.
 *
 * Revision 1.9  2004/06/30 15:32:18  steve
 *  nex_output for NetPDelay statements.
 *
 * Revision 1.8  2003/12/20 00:59:31  steve
 *  Synthesis debug messages.
 *
 * Revision 1.7  2003/10/26 04:51:39  steve
 *  Output of While is output of while substatement.
 *
 * Revision 1.6  2002/09/17 04:39:20  steve
 *  Account for part select in l-value.
 *
 * Revision 1.5  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2002/07/29 00:00:28  steve
 *  Asynchronous synthesis of sequential blocks.
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

