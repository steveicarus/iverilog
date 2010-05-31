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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include "config.h"

# include  "functor.h"
# include  "netlist.h"
# include  <cassert>

/*
 * Most process statements are not roots of synchronous logic.
 */
bool NetProc::is_synchronous()
{
      return false;
}

bool NetEvWait::is_synchronous()
{
      for (unsigned idx = 0 ;  idx < nevents_ ;  idx += 1) {
	    NetEvent*ev = events_[idx];

	    if (ev->nprobe() == 0)
		  return false;

	    for (unsigned pdx = 0 ;  pdx < ev->nprobe() ;  pdx += 1) {
		  NetEvProbe*pr = ev->probe(pdx);

		    /* No level sensitive clocks. */
		  if (pr->edge() == NetEvProbe::ANYEDGE)
			return false;
	    }

      }

	/* So we know that there is a clock source. Check that the
	   input to the storage is asynchronous. */
      return true; //statement_->is_asynchronous();
}

bool NetProcTop::is_synchronous()
{
      if (type_ == IVL_PR_INITIAL)
	    return false;

      return statement_->is_synchronous();
}
