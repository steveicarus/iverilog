/*
 * Copyright (c) 2000-2021 Stephen Williams (steve@icarus.com)
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

# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "ivl_assert.h"

using namespace std;

NetBlock::NetBlock(Type t, NetScope*ss)
: type_(t), subscope_(ss), last_(0)
{
}

NetBlock::~NetBlock()
{
      while (last_ != 0) {
	    if (last_->next_ == last_) {
		  delete last_;
		  last_ = 0;
	    } else {
		  NetProc*cur = last_->next_;
		  last_->next_ = cur->next_;
		  cur->next_ = cur;
		  delete cur;
	    }
      }
}

void NetBlock::append(NetProc*cur)
{
      if (last_ == 0) {
	    last_ = cur;
	    cur->next_ = cur;
      } else {
	    cur->next_ = last_->next_;
	    last_->next_ = cur;
	    last_ = cur;
      }
}

void NetBlock::prepend(NetProc*cur)
{
      if (last_ == 0) {
	    last_ = cur;
	    cur->next_ = cur;
      } else {
	    cur->next_ = last_->next_;
	    last_->next_ = cur;
      }
}

const NetProc* NetBlock::proc_first() const
{
      if (last_ == 0)
	    return 0;

      return last_->next_;
}

const NetProc* NetBlock::proc_next(const NetProc*cur) const
{
      if (cur == last_)
	    return 0;

      return cur->next_;
}

NetCase::NetCase(ivl_case_quality_t q, NetCase::TYPE c, NetExpr*ex, unsigned cnt)
: quality_(q), type_(c), expr_(ex), items_(cnt)
{
      ivl_assert(*this, expr_);
}

NetCase::~NetCase()
{
      delete expr_;
      for (size_t idx = 0 ;  idx < items_.size() ;  idx += 1) {
	    delete items_[idx].guard;
	    if (items_[idx].statement) delete items_[idx].statement;
      }
}

NetCase::TYPE NetCase::type() const
{
      return type_;
}

void NetCase::set_case(unsigned idx, NetExpr*e, NetProc*p)
{
      ivl_assert(*this, idx < items_.size());
      items_[idx].guard = e;
      items_[idx].statement = p;
}

void NetCase::prune()
{
	// Test whether the case expression has been padded out
      NetESelect*padded_expr = dynamic_cast<NetESelect*>(expr_);
      if ((padded_expr == 0) || (padded_expr->select() != 0))
	    return;

	// If so, run through the case item expressions to find
	// the minimum number of bits needed to unambiguously
	// select the correct case item.
      const NetExpr*unpadded_expr = padded_expr->sub_expr();
      unsigned padded_width = padded_expr->expr_width();
      unsigned prune_width = unpadded_expr->expr_width();
      for (unsigned idx = 0; idx < items_.size(); idx += 1) {
	      // If there is no guard expression, this is the default
	      // case, so skip it.
	    if (items_[idx].guard == 0)
		  continue;

	      // If the guard expression is not constant, assume
	      // all bits are needed, so no pruning can be done.
	    NetEConst*gc = dynamic_cast<NetEConst*>(items_[idx].guard);
	    if (gc == 0)
		  return;

	    unsigned sig_bits = gc->value().significant_bits();
	    if (sig_bits > prune_width)
		  prune_width = sig_bits;

	      // If all the padding bits are needed, no pruning
	      // can be done.
	    if (prune_width >= padded_width)
		  return;
      }
      ivl_assert(*this, prune_width < padded_width);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: pruning case expressions to "
		 << prune_width << " bits." << endl;
      }

	// Prune the case expression
      expr_ = pad_to_width(unpadded_expr->dup_expr(), prune_width, *expr_);
      delete padded_expr;

	// Prune the case item expressions
      for (unsigned idx = 0; idx < items_.size(); idx += 1) {
	    if (items_[idx].guard == 0)
		  continue;

	    NetEConst*gc = dynamic_cast<NetEConst*>(items_[idx].guard);
	    ivl_assert(*this, gc);

	    verinum value(gc->value(), prune_width);
	    NetEConst*tmp = new NetEConst(value);
	    tmp->set_line(*gc);
	    delete gc;

	    items_[idx].guard = tmp;
      }
}

NetDisable::NetDisable(NetScope*tgt, bool flow_control)
: target_(tgt), flow_control_(flow_control)
{
}

NetDisable::~NetDisable()
{
}

const NetScope* NetDisable::target() const
{
      return target_;
}

NetForever::NetForever(NetProc*p)
: statement_(p)
{
}

NetForever::~NetForever()
{
      delete statement_;
}

NetForLoop::NetForLoop(NetNet*ind, NetExpr*iexpr, NetExpr*cond, NetProc*sub, NetProc*step)
: index_(ind), init_expr_(iexpr), condition_(cond), statement_(sub), step_statement_(step)
{
      if (index_ && init_expr_) {
	    NetAssign_*lv = new NetAssign_(index_);
	    NetAssign*use_init_statement = new NetAssign(lv, init_expr_);
	    use_init_statement->set_line(*init_expr_);
	    init_statement_ = use_init_statement;
      } else {
	    init_statement_ = nullptr;
      }
}

NetForLoop::~NetForLoop()
{
      delete init_expr_;
      delete condition_;
      delete statement_;
      delete step_statement_;
}

NetPDelay::NetPDelay(uint64_t d, NetProc*st)
: delay_(d), expr_(0), statement_(st)
{
}

NetPDelay::NetPDelay(NetExpr*d, NetProc*st)
: delay_(0), expr_(d), statement_(st)
{
}

NetPDelay::~NetPDelay()
{
      delete expr_;
}

uint64_t NetPDelay::delay() const
{
      ivl_assert(*this, expr_ == 0);
      return delay_;
}

const NetExpr* NetPDelay::expr() const
{
      return expr_;
}

NetRepeat::NetRepeat(NetExpr*e, NetProc*p)
: expr_(e), statement_(p)
{
}

NetRepeat::~NetRepeat()
{
      delete expr_;
      delete statement_;
}

const NetExpr* NetRepeat::expr() const
{
      return expr_;
}
