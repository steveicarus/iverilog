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
#ident "$Id: net_expr.cc,v 1.9 2002/11/06 02:25:13 steve Exp $"
#endif

# include  "config.h"
# include  "netlist.h"
# include  <iostream>

/*
 * Create an add/sub node from the two operands. Make a best guess of
 * the 
 */
NetEBAdd::NetEBAdd(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      NetEConst* tmp;

	/* Catch the special case that one of the operands is an
	   unsized constant number. If so, then we should set the
	   width of that number to the size of the other operand, plus
	   one. This expands the expression to account for the largest
	   possible result.

	   The set_width applied to a constant value will only
	   truncate the constant so far as it can still hold its
	   logical value, so this is safe to do. */
      if ( (tmp = dynamic_cast<NetEConst*>(r))
	   && (! tmp->has_width())
	   && (tmp->expr_width() > l->expr_width()) ) {

	    unsigned target_width = l->expr_width() + 1;
	    r->set_width(target_width);

	      /* Note: This constant value will not gain a defined
		 with from this. Make sure. */
	    assert(! r->has_width() );

      } else if ( (tmp = dynamic_cast<NetEConst*>(l))
	   && (! tmp->has_width())
	   && (tmp->expr_width() > r->expr_width()) ) {

	    unsigned target_width = r->expr_width() + 1;
	    l->set_width(target_width);

	      /* Note: This constant value will not gain a defined
		 with from this. Make sure. */
	    assert(! l->has_width() );

      }

	/* Now that we have the operand sizes the way we like, or as
	   good as we are going to get them, set the size of myself. */
      if (r->expr_width() > l->expr_width()) {

	    expr_width(r->expr_width());

      } else {
	    expr_width(l->expr_width());
      }

      cast_signed(l->has_sign() && r->has_sign());
}

NetEBAdd::~NetEBAdd()
{
}

NetEBAdd* NetEBAdd::dup_expr() const
{
      NetEBAdd*result = new NetEBAdd(op_, left_->dup_expr(),
				     right_->dup_expr());
      return result;
}

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

NetEParam::NetEParam()
: des_(0), scope_(0)
{
}

NetEParam::NetEParam(Design*d, NetScope*s, const hname_t&n)
: des_(d), scope_(s), name_(n)
{
}

NetEParam::~NetEParam()
{
}

bool NetEParam::has_width() const
{
      return false;
}

NetEParam* NetEParam::dup_expr() const
{
      return new NetEParam(des_, scope_, name_);
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
 * Revision 1.9  2002/11/06 02:25:13  steve
 *  No need to keep excess width from an
 *  unsigned constant value, if it can
 *  be trimmed safely.
 *
 * Revision 1.8  2002/10/19 22:59:49  steve
 *  Redo the parameter vector support to allow
 *  parameter names in range expressions.
 *
 * Revision 1.7  2002/09/01 03:01:48  steve
 *  Properly cast signedness of parameters with ranges.
 *
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

