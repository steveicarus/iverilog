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
#ident "$Id: synth2.cc,v 1.1 2002/06/30 02:21:32 steve Exp $"
#endif

# include "config.h"

# include  "functor.h"
# include  "netlist.h"
# include  <assert.h>


/*
 * Async synthesis of assignments is done by synthesizing the rvalue
 * expression, then connecting the l-value directly to the output of
 * the r-value.
 */
bool NetAssignBase::synth_async(Design*des, NetScope*scope)
{
      NetNet*rsig = rval_->synthesize(des);

      NetNet*lsig = lval_->sig();
      assert(lsig);
      assert(lval_->more == 0);

      assert(lsig->pin_count() == rsig->pin_count());
      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
	    connect(lsig->pin(idx), rsig->pin(idx));

      return true;
}

bool NetCondit::synth_async(Design*des, NetScope*scope)
{
      return false;
}

bool NetEvWait::synth_async(Design*des, NetScope*scope)
{
      return statement_->synth_async(des, scope);
}

bool NetProc::synth_async(Design*des, NetScope*scope)
{
      return false;
}

bool NetProcTop::synth_async(Design*des)
{
      return statement_->synth_async(des, scope());
}

class synth2_f  : public functor_t {

    public:
      void process(class Design*, class NetProcTop*);

    private:
};


/*
 * Look at a process, and divide the problem into always and initial
 * threads.
 */
void synth2_f::process(class Design*des, class NetProcTop*top)
{
      if (! top->is_asynchronous())
	    return;

      if (! top->synth_async(des))
	    return;

      cerr << top->get_line() << ": info: thread is asynchronous." << endl;
      des->delete_process(top);
}

void synth2(Design*des)
{
      synth2_f synth_obj;
      des->functor(&synth_obj);
}

/*
 * $Log: synth2.cc,v $
 * Revision 1.1  2002/06/30 02:21:32  steve
 *  Add structure for asynchronous logic synthesis.
 *
 */

