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
#ident "$Id: net_nex_output.cc,v 1.12 2005/02/14 04:58:50 steve Exp $"
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
		  out.add(lsig->pin(0).nexus());
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

      NetProc*cur = last_;
      do {
	    cur = cur->next_;
	    cur->nex_output(out);
      } while (cur != last_);
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
 * Revision 1.12  2005/02/14 04:58:50  steve
 *  l-value input may be a vector.
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

