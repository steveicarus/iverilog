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
#if !defined(WINNT)
#ident "$Id: net_nex_input.cc,v 1.1 2002/04/21 04:59:08 steve Exp $"
#endif

# include "config.h"

# include <iostream>

# include  <cassert>
# include  <typeinfo>
# include  "netlist.h"
# include  "netmisc.h"

NexusSet* NetExpr::nex_input()
{
      cerr << get_line()
	   << ": internal error: nex_input not implemented: "
	   << *this << endl;
      return 0;
}

NexusSet* NetProc::nex_input()
{
      cerr << get_line()
	   << ": internal error: NetProc::nex_input not implemented"
	   << endl;
      return 0;
}

NexusSet* NetEBinary::nex_input()
{
      NexusSet*result = left_->nex_input();
      NexusSet*tmp = right_->nex_input();
      result->add(*tmp);
      delete tmp;
      return result;
}

NexusSet* NetEBitSel::nex_input()
{
      NexusSet*result = sig_->nex_input();
      NexusSet*tmp = idx_->nex_input();
      result->add(*tmp);
      delete tmp;
      return result;
}

NexusSet* NetEConcat::nex_input()
{
      NexusSet*result = parms_[0]->nex_input();
      for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
	    NexusSet*tmp = parms_[idx]->nex_input();
	    result->add(*tmp);
	    delete tmp;
      }
      return result;
}

/*
 * A constant has not inputs, so always return an empty set.
 */
NexusSet* NetEConst::nex_input()
{
      return new NexusSet;
}

NexusSet* NetEMemory::nex_input()
{
      NexusSet*result = idx_->nex_input();
      return result;
}

/*
 * A parameter by definition has no inputs. It represents a constant
 * value, even if that value is a constant expression.
 */
NexusSet* NetEParam::nex_input()
{
      return new NexusSet;
}

NexusSet* NetEScope::nex_input()
{
      return new NexusSet;
}

NexusSet* NetESelect::nex_input()
{
      NexusSet*result = base_->nex_input();
      NexusSet*tmp = expr_->nex_input();
      result->add(*tmp);
      delete tmp;
      return result;
}

NexusSet* NetESFunc::nex_input()
{
      if (nparms_ == 0)
	    return new NexusSet;

      NexusSet*result = parms_[0]->nex_input();
      for (unsigned idx = 1 ;  idx < nparms_ ;  idx += 1) {
	    NexusSet*tmp = parms_[idx]->nex_input();
	    result->add(*tmp);
	    delete tmp;
      }
      return result;
}

NexusSet* NetESignal::nex_input()
{
      NexusSet*result = new NexusSet;
      for (unsigned idx = 0 ;  idx < net_->pin_count() ;  idx += 1)
	    result->add(net_->pin(idx).nexus());

      return result;
}

NexusSet* NetETernary::nex_input()
{
      NexusSet*tmp;
      NexusSet*result = cond_->nex_input();

      tmp = true_val_->nex_input();
      result->add(*tmp);
      delete tmp;

      tmp = false_val_->nex_input();
      result->add(*tmp);
      delete tmp;

      return result;
}

NexusSet* NetEUFunc::nex_input()
{
      NexusSet*result = new NexusSet;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    NexusSet*tmp = parms_[idx]->nex_input();
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetEUnary::nex_input()
{
      return expr_->nex_input();
}

NexusSet* NetAssign::nex_input()
{
      NexusSet*result = rval()->nex_input();
      return result;
}

/*
 * The inputs to a case statement are the inputs to the expression,
 * the inputs to all the guards, and the inputs to all the guarded
 * statements.
 */
NexusSet* NetCase::nex_input()
{
      NexusSet*result = expr_->nex_input();
      if (result == 0)
	    return 0;

      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {

	    assert(items_[idx].statement);
	    NexusSet*tmp = items_[idx].statement->nex_input();
	    assert(tmp);
	    result->add(*tmp);
	    delete tmp;

	    assert(items_[idx].guard);
	    tmp = items_[idx].guard->nex_input();
	    assert(tmp);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

/*
 * $Log: net_nex_input.cc,v $
 * Revision 1.1  2002/04/21 04:59:08  steve
 *  Add support for conbinational events by finding
 *  the inputs to expressions and some statements.
 *  Get case and assignment statements working.
 *
 */

