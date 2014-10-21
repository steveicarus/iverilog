/*
 * Copyright (c) 2000-2011 Stephen Williams (steve@icarus.com)
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

# include  "netlist.h"
# include  <cassert>

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

NetCase::NetCase(NetCase::TYPE c, NetExpr*ex, unsigned cnt)
: type_(c), expr_(ex), items_(cnt)
{
      assert(expr_);
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
      assert(idx < items_.size());
      items_[idx].guard = e;
      items_[idx].statement = p;
}

NetDisable::NetDisable(NetScope*tgt)
: target_(tgt)
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
}

void NetForLoop::wrap_up()
{
      NetBlock*top = new NetBlock(NetBlock::SEQU, 0);
      top->set_line(*this);

      NetAssign_*lv = new NetAssign_(index_);
      NetAssign*set_stmt = new NetAssign(lv, init_expr_);
      set_stmt->set_line(*init_expr_);
      top->append(set_stmt);

      NetBlock*internal_block = new NetBlock(NetBlock::SEQU, 0);
      internal_block->set_line(*this);

      if (statement_) internal_block->append(statement_);
      internal_block->append(step_statement_);

      NetWhile*wloop = new NetWhile(condition_, internal_block);
      wloop->set_line(*this);

      top->append(wloop);

      as_block_ = top;
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
      assert(expr_ == 0);
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
