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
#ident "$Id: net_expr.cc,v 1.6 2002/08/12 01:34:59 steve Exp $"
#endif

# include  "config.h"
# include  "netlist.h"
# include  <iostream>

NetEConcat::NetEConcat(unsigned cnt, NetExpr* r)
: parms_(cnt), repeat_(r)
{
      if (repeat_ == 0) {
	    repeat_calculated_ = true;
	    repeat_value_ = 1;
      } else {
	    repeat_calculated_ = false;
      }

      expr_width(0);
}

NetEConcat::~NetEConcat()
{
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    delete parms_[idx];
}

void NetEConcat::set(unsigned idx, NetExpr*e)
{
      assert(idx < parms_.count());
      assert(parms_[idx] == 0);
      parms_[idx] = e;
      expr_width( expr_width() + e->expr_width() );
}

NetEConcat* NetEConcat::dup_expr() const
{
      NetEConcat*dup = new NetEConcat(parms_.count(), repeat_);
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    if (parms_[idx]) {
		  NetExpr*tmp = parms_[idx]->dup_expr();
		  assert(tmp);
		  dup->parms_[idx] = tmp;
	    }


      dup->expr_width(expr_width());
      return dup;
}

unsigned NetEConcat::repeat()
{
      if (repeat_calculated_)
	    return repeat_value_;

      assert(repeat_);

      if (! dynamic_cast<NetEConst*>(repeat_)) {
	    NetExpr*tmp = repeat_->eval_tree();
	    if (tmp != 0) {
		  delete repeat_;
		  repeat_ = tmp;
	    }
      }

      NetEConst*repeat_const = dynamic_cast<NetEConst*>(repeat_);

	/* This should not be possible, as it was checked earlier to
	   assure that this is a constant expression. */
      if (repeat_const == 0) {
	    cerr << get_line() << ": internal error: repeat expression "
		 << "is not a compile time constant." << endl;
	    cerr << get_line() << ":               : Expression is: "
		 << *repeat_ << endl;
	    repeat_calculated_ = true;
	    repeat_value_ = 1;
	    return 1;
      }

      repeat_calculated_ = true;
      repeat_value_ = repeat_const->value().as_ulong();

      delete repeat_;
      repeat_ = 0;

      return repeat_value_;
}

unsigned NetEConcat::repeat() const
{
      assert(repeat_calculated_);
      return repeat_value_;
}

NetESelect::NetESelect(NetExpr*exp, NetExpr*base, unsigned wid)
: expr_(exp), base_(base)
{
      expr_width(wid);
}

NetESelect::~NetESelect()
{
      delete expr_;
      delete base_;
}

const NetExpr*NetESelect::sub_expr() const
{
      return expr_;
}

const NetExpr*NetESelect::select() const
{
      return base_;
}

bool NetESelect::has_width() const
{
      return true;
}

bool NetESelect::set_width(unsigned w)
{
      if (expr_width() == 1)
	    return true;
      else
	    return false;
}

/*
 * $Log: net_expr.cc,v $
 * Revision 1.6  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2002/06/06 18:57:18  steve
 *  Use standard name for iostream.
 *
 * Revision 1.4  2002/05/25 16:51:37  steve
 *  include iostream for gcc 3.1
 *
 * Revision 1.3  2002/05/05 21:11:50  steve
 *  Put off evaluation of concatenation repeat expresions
 *  until after parameters are defined. This allows parms
 *  to be used in repeat expresions.
 *
 *  Add the builtin $signed system function.
 *
 * Revision 1.2  2002/01/29 22:36:31  steve
 *  include config.h to eliminate warnings.
 *
 * Revision 1.1  2002/01/28 01:39:45  steve
 *  Add ne_expr.cc
 *
 */

