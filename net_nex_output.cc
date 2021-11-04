/*
 * Copyright (c) 2002-2021 Stephen Williams (steve@icarus.com)
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

# include <iostream>

# include  <cassert>
# include  <typeinfo>
# include  "netlist.h"
# include  "netmisc.h"

using namespace std;

void NetProc::nex_output(NexusSet&)
{
      cerr << get_fileline()
	   << ": internal error: NetProc::nex_output not implemented"
	   << endl;
      cerr << get_fileline()
	   << ":               : on object type " << typeid(*this).name()
	   << endl;
}

void NetAlloc::nex_output(NexusSet&)
{
}

void NetAssign_::nex_output(NexusSet&out)
{
      assert(! nest_);
      assert(sig_);
      unsigned use_word = 0;
      unsigned use_base = 0;
      unsigned use_wid = lwidth();
      if (word_) {
	    long tmp = 0;
	    if (eval_as_long(tmp, word_)) {
		    // A constant word select, so add the selected word.
		  use_word = tmp;
	    } else {
		    // A variable word select. The obvious thing to do
		    // is to add the whole array, but this could cause
		    // NetBlock::nex_input() to overprune the input set.
		    // As array access is not yet handled in synthesis,
		    // I'll leave this as TBD - the output set is not
		    // otherwise used when elaborating an always @*
		    // block.
		  return;
	    }
      }
      Nexus*nex = sig_->pin(use_word).nexus();
      if (base_) {

	      // Unable to evaluate the bit/part select of
	      // the l-value, so this is a mux. Pretty
	      // sure I don't know how to handle this yet
	      // in synthesis, so punt for now.

	      // Even with constant bit/part select, we want to
	      // return the entire signal as an output. The
	      // context will need to sort out which bits are
	      // actually assigned.
	    use_base = 0;
	    use_wid = nex->vector_width();
      }
      out.add(nex, use_base, use_wid);
}

/*
 * Assignments have as output all the bits of the concatenated signals
 * of the l-value.
 */
void NetAssignBase::nex_output(NexusSet&out)
{
      for (NetAssign_*cur = lval_ ;  cur ;  cur = cur->more) {
	    cur->nex_output(out);
      }
}

void NetBlock::nex_output(NexusSet&out)
{
      if (last_ == 0) return;

      NetProc*cur = last_;
      do {
	    cur = cur->next_;
	    cur->nex_output(out);
      } while (cur != last_);
}

void NetCase::nex_output(NexusSet&out)
{
      for (size_t idx = 0 ;  idx < items_.size() ;  idx += 1) {

	      // Empty statements clearly have no output.
	    if (items_[idx].statement == 0) continue;

	    items_[idx].statement->nex_output(out);
      }

}

void NetCondit::nex_output(NexusSet&out)
{
      if (if_) if_->nex_output(out);
      if (else_) else_->nex_output(out);
}

void NetDisable::nex_output(NexusSet&)
{
}

void NetDoWhile::nex_output(NexusSet&out)
{
      if (proc_) proc_->nex_output(out);
}

void NetEvTrig::nex_output(NexusSet&)
{
}

void NetEvNBTrig::nex_output(NexusSet&)
{
}

void NetEvWait::nex_output(NexusSet&out)
{
      if (statement_) statement_->nex_output(out);
}

void NetForever::nex_output(NexusSet&out)
{
      if (statement_) statement_->nex_output(out);
}

void NetForLoop::nex_output(NexusSet&out)
{
      if (statement_) statement_->nex_output(out);
}

void NetFree::nex_output(NexusSet&)
{
}

void NetPDelay::nex_output(NexusSet&out)
{
      if (statement_) statement_->nex_output(out);
}

void NetRepeat::nex_output(NexusSet&out)
{
      if (statement_) statement_->nex_output(out);
}

/*
 * For the purposes of synthesis, system task calls have no output at
 * all. This is OK because most system tasks are not synthesizable in
 * the first place.
 */
void NetSTask::nex_output(NexusSet&)
{
}

/*
* Consider a task call to not have any outputs. This is not quite
* right, we should be listing as outputs all the output ports, but for
* the purposes that this method is used, this will do for now.
*/
void NetUTask::nex_output(NexusSet&)
{
}

void NetWhile::nex_output(NexusSet&out)
{
      if (proc_) proc_->nex_output(out);
}
