/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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
# include  <cassert>

bool NetAssign::is_asynchronous()
{
      return true;
}

bool NetCondit::is_asynchronous()
{
      return false;
}

/*
 * NetEvWait statements come from statements of the form @(...) in the
 * Verilog source. These event waits are considered asynchronous if
 * all of the events in the list are ANYEDGE, and all the inputs to
 * the statement are included in the sensitivity list. If any of the
 * events are posedge or negedge, the statement is synchronous
 * (i.e. an edge-triggered flip-flop) and if any of the inputs are
 * unaccounted for in the sensitivity list then the statement is a
 * latch.
 */
bool NetEvWait::is_asynchronous()
{
	/* The "sense" set contains the set of Nexa that are in the
	   sensitivity list. We also require that the events are all
	   level sensitive, but the nex_async_ method takes care of
	   that test. */
      NexusSet*sense = new NexusSet;
      for (unsigned idx = 0 ;  idx < events_.size() ;  idx += 1) {
	    NexusSet*tmp = events_[idx]->nex_async_();
	    if (tmp == 0) {
		  delete sense;
		  return false;
	    }

	    sense->add(*tmp);
	    delete tmp;
      }

      NexusSet*inputs = statement_->nex_input();

      if (! sense->contains(*inputs)) {
	    delete sense;
	    delete inputs;
	    return false;
      }

      delete sense;
      delete inputs;

	/* If it passes all the other tests, then this statement is
	   asynchronous. */
      return true;
}

bool NetProc::is_asynchronous()
{
      return false;
}

bool NetProcTop::is_asynchronous() const
{
      if (type_ == IVL_PR_INITIAL)
	    return false;

      return statement_->is_asynchronous();
}
