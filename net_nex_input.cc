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

# include  <iostream>
# include  <set>
# include  <cassert>
# include  <typeinfo>
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"

using namespace std;

NexusSet* NetExpr::nex_input(bool, bool, bool) const
{
      cerr << get_fileline()
	   << ": internal error: nex_input not implemented: "
	   << *this << endl;
      return new NexusSet;
}

NexusSet* NetProc::nex_input(bool, bool, bool) const
{
      cerr << get_fileline()
	   << ": internal error: NetProc::nex_input not implemented"
	   << endl;
      return new NexusSet;
}

NexusSet* NetEArrayPattern::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = new NexusSet;
      for (size_t idx = 0 ; idx < items_.size() ; idx += 1) {
	    if (items_[idx]==0) continue;

	    NexusSet*tmp = items_[idx]->nex_input(rem_out, always_sens, nested_func);
	    if (tmp == 0) continue;

	    result->add(*tmp);
	    delete tmp;
      }
      return result;
}

NexusSet* NetEBinary::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = left_->nex_input(rem_out, always_sens, nested_func);
      NexusSet*tmp = right_->nex_input(rem_out, always_sens, nested_func);
      result->add(*tmp);
      delete tmp;
      return result;
}

NexusSet* NetEConcat::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      if (parms_[0] == NULL) return new NexusSet;
      NexusSet*result = parms_[0]->nex_input(rem_out, always_sens, nested_func);
      for (unsigned idx = 1 ;  idx < parms_.size() ;  idx += 1) {
	    if (parms_[idx] == NULL) {
		  delete result;
		  return new NexusSet;
	    }
	    NexusSet*tmp = parms_[idx]->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }
      return result;
}

NexusSet* NetEAccess::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

/*
 * A constant has not inputs, so always return an empty set.
 */
NexusSet* NetEConst::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetECReal::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetEEvent::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetELast::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetENetenum::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetENew::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetENull::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetEProperty::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetEScope::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetESelect::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = base_? base_->nex_input(rem_out, always_sens, nested_func) : new NexusSet();
      NexusSet*tmp = expr_->nex_input(rem_out, always_sens, nested_func);
      bool const_select = result->size() == 0;
      if (always_sens && const_select) {
	    if (NetEConst *val = dynamic_cast <NetEConst*> (base_)) {
		  assert(select_type() == IVL_SEL_OTHER);
		  if (NetESignal *sig = dynamic_cast<NetESignal*> (expr_)) {
			delete tmp;
			tmp = sig->nex_input_base(rem_out, always_sens, nested_func,
                                                  val->value().as_unsigned(), expr_width());
		  } else {
			cerr << get_fileline() << ": Sorry, cannot determine the sensitivity "
			     << "for the select of " << *expr_ << ", using all bits." << endl;
		  }
	    }
      }
      result->add(*tmp);
      delete tmp;
	/* See the comment for NetESignal below. */
      if (base_ && ! always_sens && warn_sens_entire_vec) {
	    cerr << get_fileline() << ": warning: @* is sensitive to all "
	            "bits in '" << *expr_ << "'." << endl;
      }
      return result;
}

/*
 * The $fread, etc. system functions can have NULL arguments.
 */
NexusSet* NetESFunc::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = new NexusSet;

      if (parms_.empty()) return result;

      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {
	    if (parms_[idx]) {
		  NexusSet*tmp = parms_[idx]->nex_input(rem_out, always_sens, nested_func);
		  result->add(*tmp);
		  delete tmp;
	    }
      }

      return result;
}

NexusSet* NetEShallowCopy::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetESignal::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      return nex_input_base(rem_out, always_sens, nested_func, 0, 0);
}

NexusSet* NetESignal::nex_input_base(bool rem_out, bool always_sens, bool nested_func,
                                     unsigned base, unsigned width) const
{
	/*
	 * This is not what I would expect for the various selects (bit,
	 * part, index, array). This code adds all the bits/array words
	 * instead of building the appropriate select and then using it
	 * as the trigger. Other simulators also add everything.
	 */
      bool const_select = false;
      unsigned const_word = 0;
      NexusSet*result = new NexusSet;
	/* Local signals are not added to the sensitivity list. */
      if (net_->local_flag()) return result;
	/* If we have an array index add it to the sensitivity list. */
      if (word_) {
	    NexusSet*tmp;
	    tmp = word_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
            if (!always_sens && warn_sens_entire_arr) {
                  cerr << get_fileline() << ": warning: @* is sensitive to all "
                       << net_->unpacked_count() << " words in array '"
                       << name() << "'." << endl;
            }
	    if (always_sens) if (NetEConst *val = dynamic_cast <NetEConst*> (word_)) {
		  const_select = true;
		  const_word = val->value().as_unsigned();
	    }
      }

      if ((base == 0) && (width == 0)) width = net_->vector_width();

      if (const_select) {
	    result->add(net_->pin(const_word).nexus(), base, width);
      } else {
	    for (unsigned idx = 0 ;  idx < net_->pin_count() ;  idx += 1)
		  result->add(net_->pin(idx).nexus(), base, width);
      }

      return result;
}

NexusSet* NetETernary::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*tmp;
      NexusSet*result = cond_->nex_input(rem_out, always_sens, nested_func);

      tmp = true_val_->nex_input(rem_out, always_sens, nested_func);
      result->add(*tmp);
      delete tmp;

      tmp = false_val_->nex_input(rem_out, always_sens, nested_func);
      result->add(*tmp);
      delete tmp;

      return result;
}

// Get the contribution of a function call in a always_comb block
static void func_always_sens(NetFuncDef *func, NexusSet *result,
			     bool rem_out, bool nested_func)
{
	  // Avoid recursive function calls.
	static set<NetFuncDef*> func_set;
	if (!nested_func)
	      func_set.clear();

	if (!func_set.insert(func).second)
	      return;

	std::unique_ptr<NexusSet> tmp(func->proc()->nex_input(rem_out, true, true));
	  // Remove the function inputs
	std::unique_ptr<NexusSet> in(new NexusSet);
	for (unsigned idx = 0; idx < func->port_count(); idx++) {
	      NetNet *net = func->port(idx);
	      assert(net->pin_count() == 1);
	      in->add(net->pin(0).nexus(), 0, net->vector_width());
	}
	tmp->rem(*in);
	result->add(*tmp);
}

NexusSet* NetEUFunc::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = new NexusSet;

      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {
	    NexusSet*tmp = parms_[idx]->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      if (always_sens)
	    func_always_sens(func_->func_def(), result, rem_out, nested_func);

      return result;
}

NexusSet* NetEUnary::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      return expr_->nex_input(rem_out, always_sens, nested_func);
}

NexusSet* NetAlloc::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetAssign_::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      assert(! nest_);
      NexusSet*result = new NexusSet;

      if (word_) {
	    NexusSet*tmp = word_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }
      if (base_) {
	    NexusSet*tmp = base_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetAssignBase::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = new NexusSet;
	// For the deassign and release statements there is no R-value.
      if (rval_) {
	    NexusSet*tmp = rval_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

	/* It is possible that the lval_ can have nex_input values. In
	   particular, index expressions are statement inputs as well,
	   so should be addressed here. */
      for (NetAssign_*cur = lval_ ;  cur ;  cur = cur->more) {
	    NexusSet*tmp = cur->nex_input(rem_out, always_sens, nested_func);
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
NexusSet* NetBlock::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      if (last_ == 0) return new NexusSet;

      if (! always_sens && (type_ != SEQU)) {
	    cerr << get_fileline() << ": internal error: Sorry, "
		 << "I don't know how to synthesize fork/join blocks."
		 << endl;
	    return new NexusSet;
      }

      NetProc*cur = last_->next_;
	/* This is the accumulated input set. */
      NexusSet*result = new NexusSet;
	/* This is an accumulated output set. */
      NexusSet*prev = new NexusSet;

      do {
	      /* Get the inputs for the current statement. */
	    NexusSet*tmp = cur->nex_input(rem_out, always_sens, nested_func);

	      /* Add the current input set to the accumulated input set. */
	    result->add(*tmp);
	    delete tmp;

	      /* Add the current outputs to the accumulated output set if
	       * they are going to be removed from the input set below. */
	    if (rem_out) cur->nex_output(*prev);

	    cur = cur->next_;
      } while (cur != last_->next_);

        /* Remove from the input set those bits that are outputs
           from other statements. They aren't really inputs
           to the block, just internal intermediate values. */
      if (rem_out) result->rem(*prev);
      delete prev;

      return result;
}

/*
 * The inputs to a case statement are the inputs to the expression,
 * the inputs to all the guards, and the inputs to all the guarded
 * statements.
 */
NexusSet* NetCase::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = expr_->nex_input(rem_out, always_sens, nested_func);

      for (size_t idx = 0 ;  idx < items_.size() ;  idx += 1) {

	      /* Skip cases that have empty statements. */
	    if (items_[idx].statement == 0)
		  continue;

	    NexusSet*tmp = items_[idx].statement->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;

	      /* Usually, this is the guard expression. The default
		 case is special and is identified by a null
		 guard. The default guard obviously has no input. */
	    if (items_[idx].guard) {
		  tmp = items_[idx].guard->nex_input(rem_out, always_sens, nested_func);
		  result->add(*tmp);
		  delete tmp;
	    }
      }

      return result;
}

NexusSet* NetCondit::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = expr_->nex_input(rem_out, always_sens, nested_func);

      if (if_ != 0) {
	    NexusSet*tmp = if_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      if (else_ != 0) {
	    NexusSet*tmp = else_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetDisable::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetDoWhile::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = cond_->nex_input(rem_out, always_sens, nested_func);

      if (proc_) {
	    NexusSet*tmp = proc_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetEvTrig::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetEvNBTrig::nex_input(bool, bool, bool) const
{
      return new NexusSet;
}

NexusSet* NetEvWait::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = new NexusSet;

      if (statement_) {
	    NexusSet*tmp = statement_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetForever::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = new NexusSet;

      if (statement_) {
	    NexusSet*tmp = statement_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetForLoop::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = new NexusSet;

      if (init_expr_) {
	    NexusSet*tmp = init_expr_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      if (condition_) {
	    NexusSet*tmp = condition_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      if (step_statement_) {
	    NexusSet*tmp = step_statement_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      if (statement_) {
	    NexusSet*tmp = statement_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      if (gn_shared_loop_index_flag) {
	    NexusSet*tmp = new NexusSet();
	    for (unsigned idx = 0 ; idx < index_->pin_count() ; idx += 1)
		tmp->add(index_->pin(idx).nexus(), 0, index_->vector_width());

	    result->rem(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetFree::nex_input(bool, bool, bool) const
{
      return new NexusSet;
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
NexusSet* NetPDelay::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = new NexusSet;

      if (statement_) {
	    NexusSet*tmp = statement_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

NexusSet* NetRepeat::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = expr_->nex_input(rem_out, always_sens, nested_func);

      if (statement_) {
	    NexusSet*tmp = statement_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}

/*
 * The $display, etc. system tasks can have NULL arguments.
 */
NexusSet* NetSTask::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = new NexusSet;

      if (parms_.empty()) return result;

      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {
	    if (parms_[idx]) {
		  NexusSet*tmp = parms_[idx]->nex_input(rem_out, always_sens, nested_func);
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
NexusSet* NetUTask::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet *result = new NexusSet;

      /*
       * Let the contents of void functions contribute to the sensitivity list
       * of always_comb blocks
       */
      if (always_sens && task_->type() == NetScope::FUNC)
	    func_always_sens(task_->func_def(), result, rem_out, nested_func);

      return result;
}

NexusSet* NetWhile::nex_input(bool rem_out, bool always_sens, bool nested_func) const
{
      NexusSet*result = cond_->nex_input(rem_out, always_sens, nested_func);

      if (proc_) {
	    NexusSet*tmp = proc_->nex_input(rem_out, always_sens, nested_func);
	    result->add(*tmp);
	    delete tmp;
      }

      return result;
}
