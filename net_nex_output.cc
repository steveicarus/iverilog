/*
 * Copyright (c) 2002-2007 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include <iostream>

# include  <cassert>
# include  <typeinfo>
# include  "netlist.h"
# include  "netmisc.h"

void NetProc::nex_output(NexusSet&out)
{
      cerr << get_fileline()
	   << ": internal error: NetProc::nex_output not implemented"
	   << endl;
      cerr << get_fileline()
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
		  cerr << get_fileline() << ": internal error: "
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

	      // Empty statements clearly have no output.
	    if (items_[idx].statement == 0)
		  continue;

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
 * all. This is OK because most system tasks are not synthesizable in
 * the first place.
 */
void NetSTask::nex_output(NexusSet&out)
{
}

/*
* Consider a task call to not have any outputs. This is not quite
* right, we should be listing as outputs all the output ports, but for
* the purposes that this method is used, this will do for now.
*/
void NetUTask::nex_output(NexusSet&out)
{
}

void NetWhile::nex_output(NexusSet&out)
{
      if (proc_ != 0)
	    proc_->nex_output(out);
}
