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
#ident "$Id: async.cc,v 1.4 2002/08/18 22:07:16 steve Exp $"
#endif

# include "config.h"

# include  "functor.h"
# include  "netlist.h"
# include  <assert.h>

bool NetAssign::is_asynchronous()
{
      return true;
}

bool NetCondit::is_asynchronous()
{
      return false;
}

bool NetEvWait::is_asynchronous()
{
	/* The "sense" set contains the set of Nexa that are in the
	   sensitivity list. We also presume here that the list is
	   only LEVEL sensitive. */
      NexusSet*sense = new NexusSet;
      for (unsigned idx = 0 ;  idx < nevents_ ;  idx += 1) {
	    NexusSet*tmp = event(idx)->nex_async_();
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
      return true;
}

bool NetProc::is_asynchronous()
{
      return false;
}

bool NetProcTop::is_asynchronous()
{
      if (type_ == NetProcTop::KINITIAL)
	    return false;

      return statement_->is_asynchronous();
}

/*
 * $Log: async.cc,v $
 * Revision 1.4  2002/08/18 22:07:16  steve
 *  Detect temporaries in sequential block synthesis.
 *
 * Revision 1.3  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2002/07/04 00:24:16  steve
 *  initial statements are not asynchronous.
 *
 * Revision 1.1  2002/06/30 02:21:31  steve
 *  Add structure for asynchronous logic synthesis.
 *
 */

