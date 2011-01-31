/*
 * Copyright (c) 2002-2011 Stephen Williams (steve@icarus.com)
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
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"

NexusSet* NetExpr::nex_input(bool rem_out)
{
      cerr << get_fileline()
	   << ": internal error: nex_input not implemented: "
	   << *this << endl;
      return 0;
}

NexusSet* NetProc::nex_input(bool rem_out)
{
      cerr << get_fileline()
	   << ": internal error: NetProc::nex_input not implemented"
	   << endl;
      return 0;
}

NexusSet* NetEBinary::nex_input(bool rem_out)
{
      NexusSet*result = left_->nex_input(rem_out);
      NexusSet*tmp = right_->nex_input(rem_out);
      result->add(*tmp);
      delete tmp;
      return result;
}

NexusSet* NetEConcat::nex_input(bool rem_out)
{
      if (parms_[0] == NULL) return NULL;
      NexusSet*result = parms_[0]->nex_input(rem_out);
      for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
	    if (parms_[idx] == NULL) {
		  delete result;
		  return NULL;
	    }
	    NexusSet*tmp = parms_[idx]->nex_input(rem_out);
	    result->add(*tmp);
	    delete tmp;
      }
      return result;
}

NexusSet* NetEAccess::nex_input(bool rem_out)
{
      return new NexusSet;
}

/*
 * A constant has not inputs, so always return an empty set.
 */
NexusSet* NetEConst::nex_input(bool rem_out)
{
      return new NexusSet;
}

NexusSet* NetECReal::nex_input(bool rem_out)
{
      return new NexusSet;
}

/*
 * A parameter by definition has no inputs. It represents a constant
 * value, even if that value is a constant expression.
 */
NexusSet* NetEParam::nex_input(bool rem_out)
{
      return new NexusSet;
}

NexusSet* NetEEvent::nex_input(bool rem_out)
{
      return new NexusSet;
}

NexusSet* NetEScope::nex_input(bool rem_out)
{
      return new NexusSet;
}

NexusSet* NetESelect::nex_input(bool rem_out)
{
      NexusSet*result = base_? base_->nex_input(rem_out) : new NexusSet();
      NexusSet*tmp = expr_->nex_input(rem_out);
      if (tmp == NULL) {
	    delete result;
	    return NULL;
      }
      result->add(*tmp);
      delete tmp;
	/* See the comment for NetESignal below. */
      if (base_ && warn_sens_entire_vec) {
	    cerr << get_fileline() << ": warning: @* is sensitive to all "
	            "bits in '" << *expr_ << "'." << endl;
      }
      return result;
}

/*
 * The $fread, etc. system functions can have NULL arguments.
 */
NexusSet* NetESFunc::nex_input(bool rem_out)
{
      if (nparms_ == 0)
	    return new NexusSet;

      NexusSet*result;
      if (parms_[0]) result = parms_[0]->nex_input(rem_out);
      else result = new NexusSet;
      for (unsigned idx = 1 ;  idx < nparms_ ;  idx += 1) {
	    if (parms_[idx]) {
		  NexusSet*tmp = parms_[idx]->nex_input(rem_out);
		  result->add(*tmp);
		  delete tmp;
	    }
      }
      return result;
}

NexusSet* NetESignal::nex_input(bool rem_out)
{
	/*
	 * This is not what I would expect for the various selects (bit,
	 * part, index, array). This code adds all the bits/array words
	 * instead of building the appropriate select and then using it
	 * as the trigger. Other simulators also add everything.
	 */
      NexusSet*result = new NexusSet;
	/* Local signals are not added to the sensitivity list. */
      if (net_->local_flag()) return result;
	/* If we have an array index add it to the sensitivity list. */
      if (word_) {
	    NexusSet*tmp;
	    tmp = word_->nex_input(rem_out);
	    result->add(*tmp);
	    delete tmp;
            if (warn_sens_entire_arr) {
                  cerr << get_fileline() << ": warning: @* is sensitive to all "
                       << net_->array_count() << " words in array '"
                       << name() << "'." << endl;
            }
      }
      for (unsigned idx = 0 ;  idx < net_->pin_count() ;  idx += 1)
	    result->add(net_->pin(idx).nexus());

      return result;
}

NexusSet* NetETernary::nex_input(bool rem_out)
{
      NexusSet*tmp;
      NexusSet*result = cond_->nex_input(rem_out);

      tmp = true_val_->nex_input(rem_out);
      result->add(*tmp);
      delete tmp;

      tmp = false_val_->nex_input(rem_out);
      result->add(*tmp);
      delete tmp;

      return result;
}

NexusSet* NetEUFunc::nex_input(bool rem_out)
{
      NexusSet*result = new NexusSet;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    NexusSet*tmp = parms_[idx]->nex_input(rem_out);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetEUnary::nex_input(bool rem_out)
{
      return expr_->nex_input(rem_out);
}

NexusSet* NetAssign_::nex_input(bool rem_out)
{
      NexusSet*result = new NexusSet;
      if (word_) {
	    NexusSet*tmp = word_->nex_input(rem_out);
	    result->add(*tmp);
	    delete tmp;
      }
      if (base_) {
	    NexusSet*tmp = base_->nex_input(rem_out);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetAssignBase::nex_input(bool rem_out)
{
      NexusSet*result = rval_->nex_input(rem_out);

	/* It is possible that the lval_ can have nex_input values. In
	   particular, index expressions are statement inputs as well,
	   so should be addressed here. */
      for (NetAssign_*cur = lval_ ;  cur ;  cur = cur->more) {
	    NexusSet*tmp = cur->nex_input(rem_out);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

/*
 * The nex_input of a begin/end block is the NexusSet of bits that the
 * block reads from outside the block. That means it is the union of
 * the nex_input for all the substatements.
 *
 * The input set for a sequential set is not exactly the union of the
 * input sets because there is the possibility of intermediate values,
 * that don't deserve to be in the input set. To wit:
 *
 *      begin
 *         t = a + b;
 *         c = ~t;
 *      end
 *
 * In this example, "t" should not be in the input set because it is
 * used by the sequence as a temporary value.
 */
NexusSet* NetBlock::nex_input(bool rem_out)
{
      if (last_ == 0)
	    return new NexusSet;

      if (type_ == PARA) {
	    cerr << get_fileline() << ": internal error: Sorry, "
		 << "I don't know how to synthesize fork/join blocks."
		 << endl;
	    return 0;
      }

      NetProc*cur = last_->next_;
	/* This is the accumulated input set. */
      NexusSet*result = new NexusSet;
	/* This is an accumulated output set. */
      NexusSet*prev = new NexusSet;

      do {
	      /* Get the inputs for the current statement. */
	    NexusSet*tmp = cur->nex_input(rem_out);

	      /* Add the current input set to the accumulated input set. */
	    if (tmp != 0) result->add(*tmp);
	    delete tmp;

	      /* Add the current outputs to the accumulated output set,
		 so they can be removed from the input set below. */
	    cur->nex_output(*prev);

	    cur = cur->next_;
      } while (cur != last_->next_);

        /* Remove from the input set those bits that are outputs
           from other statements. They aren't really inputs
           to the block, just internal intermediate values. */
      if (rem_out) result->rem(*prev);

      return result;
}

/*
 * The inputs to a case statement are the inputs to the expression,
 * the inputs to all the guards, and the inputs to all the guarded
 * statements.
 */
NexusSet* NetCase::nex_input(bool rem_out)
{
      NexusSet*result = expr_->nex_input(rem_out);
      if (result == 0)
	    return 0;

      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {

	      /* Skip cases that have empty statements. */
	    if (items_[idx].statement == 0)
		  continue;

	    NexusSet*tmp = items_[idx].statement->nex_input(rem_out);
	    assert(tmp);
	    result->add(*tmp);
	    delete tmp;

	      /* Usually, this is the guard expression. The default
		 case is special and is identified by a null
		 guard. The default guard obviously has no input. */
	    if (items_[idx].guard) {
		  tmp = items_[idx].guard->nex_input(rem_out);
		  assert(tmp);
		  result->add(*tmp);
		  delete tmp;
	    }
      }

      return result;
}

NexusSet* NetCAssign::nex_input(bool rem_out)
{
      cerr << get_fileline() << ": internal warning: NetCAssign::nex_input()"
	   << " not implemented." << endl;
      return new NexusSet;
}

NexusSet* NetCondit::nex_input(bool rem_out)
{
      NexusSet*result = expr_->nex_input(rem_out);
      if (if_ != 0) {
	    NexusSet*tmp = if_->nex_input(rem_out);
	    result->add(*tmp);
	    delete tmp;
      }

      if (else_ != 0) {
	    NexusSet*tmp = else_->nex_input(rem_out);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetForce::nex_input(bool rem_out)
{
      cerr << get_fileline() << ": internal warning: NetForce::nex_input()"
	   << " not implemented." << endl;
      return new NexusSet;
}

NexusSet* NetForever::nex_input(bool rem_out)
{
      NexusSet*result = statement_->nex_input(rem_out);
      return result;
}

/*
 * The NetPDelay statement is a statement of the form
 *
 *   #<expr> <statement>
 *
 * The nex_input set is the input set of the <statement>. Do *not*
 * include the input set of the <expr> because it does not affect the
 * result. The statement can be omitted.
 */
NexusSet* NetPDelay::nex_input(bool rem_out)
{
      if (statement_ == 0) return 0;
      NexusSet*result = statement_->nex_input(rem_out);
      return result;
}

NexusSet* NetRepeat::nex_input(bool rem_out)
{
      NexusSet*result = statement_->nex_input(rem_out);
      NexusSet*tmp = expr_->nex_input(rem_out);
      result->add(*tmp);
      delete tmp;
      return result;
}

/*
 * The $display, etc. system tasks can have NULL arguments.
 */
NexusSet* NetSTask::nex_input(bool rem_out)
{
      if (parms_.count() == 0)
	    return new NexusSet;

      NexusSet*result;
      if (parms_[0]) result = parms_[0]->nex_input(rem_out);
      else result = new NexusSet;
      for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
	    if (parms_[idx]) {
		  NexusSet*tmp = parms_[idx]->nex_input(rem_out);
		  result->add(*tmp);
		  delete tmp;
	    }
      }

      return result;
}

/*
 * The NetUTask represents a call to a user defined task. There are no
 * parameters to consider, because the compiler already removed them
 * and converted them to blocking assignments.
 */
NexusSet* NetUTask::nex_input(bool rem_out)
{
      return new NexusSet;
}

NexusSet* NetWhile::nex_input(bool rem_out)
{
      NexusSet*result = proc_->nex_input(rem_out);
      NexusSet*tmp = cond_->nex_input(rem_out);
      result->add(*tmp);
      delete tmp;
      return result;
}
