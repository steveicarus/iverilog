/*
 * Copyright (c) 2000-2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: net_proc.cc,v 1.6 2002/08/12 01:34:59 steve Exp $"
#endif

# include "config.h"

# include  "netlist.h"
# include  <assert.h>

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
: type_(c), expr_(ex), nitems_(cnt)
{
      assert(expr_);
      items_ = new Item[nitems_];
      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {
	    items_[idx].statement = 0;
      }
}

NetCase::~NetCase()
{
      delete expr_;
      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {
	    delete items_[idx].guard;
	    if (items_[idx].statement) delete items_[idx].statement;
      }
      delete[]items_;
}

NetCase::TYPE NetCase::type() const
{
      return type_;
}

void NetCase::set_case(unsigned idx, NetExpr*e, NetProc*p)
{
      assert(idx < nitems_);
      items_[idx].guard = e;
      items_[idx].statement = p;
      if (items_[idx].guard)
	    items_[idx].guard->set_width(expr_->expr_width());
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

NetPDelay::NetPDelay(unsigned long d, NetProc*st)
: delay_(d), expr_(0), statement_(st)
{
}

NetPDelay::NetPDelay(NetExpr*d, NetProc*st)
: delay_(0), expr_(d), statement_(st)
{
}

NetPDelay::~NetPDelay()
{
      if (expr_) delete expr_;
}

unsigned long NetPDelay::delay() const
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



/*
 * $Log: net_proc.cc,v $
 * Revision 1.6  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2002/07/28 23:58:45  steve
 *  Fix NetBlock destructor to delete substatements.
 *
 * Revision 1.4  2002/04/21 04:59:08  steve
 *  Add support for conbinational events by finding
 *  the inputs to expressions and some statements.
 *  Get case and assignment statements working.
 *
 * Revision 1.3  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.2  2000/07/27 05:13:44  steve
 *  Support elaboration of disable statements.
 *
 * Revision 1.1  2000/07/07 04:53:54  steve
 *  Add support for non-constant delays in delay statements,
 *  Support evaluating ! in constant expressions, and
 *  move some code from netlist.cc to net_proc.cc.
 *
 */

