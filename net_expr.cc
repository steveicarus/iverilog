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
#ident "$Id: net_expr.cc,v 1.23 2004/10/04 01:10:54 steve Exp $"
#endif

# include  "config.h"
# include  "netlist.h"
# include  "compiler.h"
# include  <iostream>

NetExpr::TYPE NetExpr::expr_type() const
{
      return ET_VECTOR;
}

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
		 width from this. Make sure. */
	    assert(! r->has_width() );

      } else if ( (tmp = dynamic_cast<NetEConst*>(l))
	   && (! tmp->has_width())
	   && (tmp->expr_width() > r->expr_width()) ) {

	    unsigned target_width = r->expr_width() + 1;
	    l->set_width(target_width);

	      /* Note: This constant value will not gain a defined
		 width from this. Make sure. */
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

NetExpr::TYPE NetEBAdd::expr_type() const
{
      if (left_->expr_type() == ET_REAL)
	    return ET_REAL;

      if (right_->expr_type() == ET_REAL)
	    return ET_REAL;

      return ET_VECTOR;
}

/*
 * Create a comparison operator with two sub-expressions.
 *
 * Handle the special case of an unsized constant on the left or right
 * side by resizing the number to match the other
 * expression. Otherwise, the netlist will have to allow the
 * expressions to have different widths.
 */
NetEBComp::NetEBComp(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(r)) do {

	    if (tmp->has_width())
		  break;

	    if (l->expr_width() == 0)
		  break;

	    if (tmp->expr_width() == l->expr_width())
		  break;

	    tmp->set_width(l->expr_width());

      } while (0);

      if (NetEConst*tmp = dynamic_cast<NetEConst*>(l)) do {

	    if (tmp->has_width())
		  break;

	    if (r->expr_width() == 0)
		  break;

	    if (tmp->expr_width() == r->expr_width())
		  break;

	    tmp->set_width(r->expr_width());

      } while (0);


      expr_width(1);
}

NetEBComp::~NetEBComp()
{
}

bool NetEBComp::has_width() const
{
      return true;
}

NetEBDiv::NetEBDiv(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      unsigned w = l->expr_width();
      if (r->expr_width() > w)
	    w = r->expr_width();

      expr_width(w);
      cast_signed(l->has_sign() && r->has_sign());
}

NetEBDiv::~NetEBDiv()
{
}

NetEBDiv* NetEBDiv::dup_expr() const
{
      NetEBDiv*result = new NetEBDiv(op_, left_->dup_expr(),
				       right_->dup_expr());
      return result;
}

NetExpr::TYPE NetEBDiv::expr_type() const
{
      if (left_->expr_type() == ET_REAL)
	    return ET_REAL;

      if (right_->expr_type() == ET_REAL)
	    return ET_REAL;

      return ET_VECTOR;
}

NetEBMult::NetEBMult(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      expr_width(l->expr_width() + r->expr_width());
      cast_signed(l->has_sign() && r->has_sign());

	/* If it turns out that this is not a signed expression, then
	   cast the signedness out of the operands as well. */
      if (! has_sign()) {
	    l->cast_signed(false);
	    r->cast_signed(false);
      }
}

NetEBMult::~NetEBMult()
{
}

NetEBMult* NetEBMult::dup_expr() const
{
      NetEBMult*result = new NetEBMult(op_, left_->dup_expr(),
				       right_->dup_expr());
      return result;
}

NetExpr::TYPE NetEBMult::expr_type() const
{
      if (left_->expr_type() == ET_REAL)
	    return ET_REAL;

      if (right_->expr_type() == ET_REAL)
	    return ET_REAL;

      return ET_VECTOR;
}

NetEBShift::NetEBShift(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      expr_width(l->expr_width());

	// The >>> is signed if the left operand is signed.
      if (op == 'R') cast_signed(l->has_sign());
}

NetEBShift::~NetEBShift()
{
}

bool NetEBShift::has_width() const
{
      return left_->has_width();
}

NetEBShift* NetEBShift::dup_expr() const
{
      NetEBShift*result = new NetEBShift(op_, left_->dup_expr(),
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

bool NetEConcat::has_width() const
{
      return true;
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

NetECReal::NetECReal(const verireal&val)
: value_(val)
{
}

NetECReal::~NetECReal()
{
}

const verireal& NetECReal::value() const
{
      return value_;
}

bool NetECReal::has_width() const
{
      return false;
}

NetECReal* NetECReal::dup_expr() const
{
      NetECReal*tmp = new NetECReal(value_);
      tmp->set_line(*this);
      return tmp;
}

NetExpr::TYPE NetECReal::expr_type() const
{
      return ET_REAL;
}

NetECRealParam::NetECRealParam(NetScope*s, perm_string n, const verireal&v)
: NetECReal(v), scope_(s), name_(n)
{
}

NetECRealParam::~NetECRealParam()
{
}

perm_string NetECRealParam::name() const
{
      return name_;
}

const NetScope* NetECRealParam::scope() const
{
      return scope_;
}


NetEParam::NetEParam()
: des_(0), scope_(0)
{
}

NetEParam::NetEParam(Design*d, NetScope*s, perm_string n)
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
      NetEParam*tmp = new NetEParam(des_, scope_, name_);
      tmp->set_line(*this);
      return tmp;
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

NetESFunc::NetESFunc(const char*n, NetExpr::TYPE t,
		     unsigned width, unsigned np)
: name_(0), type_(t)
{
      name_ = lex_strings.add(n);
      expr_width(width);
      nparms_ = np;
      parms_ = new NetExpr*[np];
      for (unsigned idx = 0 ;  idx < nparms_ ;  idx += 1)
	    parms_[idx] = 0;
}

NetESFunc::~NetESFunc()
{
      for (unsigned idx = 0 ;  idx < nparms_ ;  idx += 1)
	    if (parms_[idx]) delete parms_[idx];

      delete[]parms_;
	/* name_ string ls lex_strings allocated. */
}

const char* NetESFunc::name() const
{
      return name_;
}

unsigned NetESFunc::nparms() const
{
      return nparms_;
}

void NetESFunc::parm(unsigned idx, NetExpr*v)
{
      assert(idx < nparms_);
      if (parms_[idx])
	    delete parms_[idx];
      parms_[idx] = v;
}

const NetExpr* NetESFunc::parm(unsigned idx) const
{
      assert(idx < nparms_);
      return parms_[idx];
}

NetExpr* NetESFunc::parm(unsigned idx)
{
      assert(idx < nparms_);
      return parms_[idx];
}

NetExpr::TYPE NetESFunc::expr_type() const
{
      return type_;
}

/*
 * $Log: net_expr.cc,v $
 * Revision 1.23  2004/10/04 01:10:54  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.22  2004/02/20 06:22:56  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.21  2003/08/28 04:11:19  steve
 *  Spelling patch.
 *
 * Revision 1.20  2003/06/18 03:55:18  steve
 *  Add arithmetic shift operators.
 *
 * Revision 1.19  2003/06/15 18:53:20  steve
 *  Operands of unsigned multiply are unsigned.
 *
 * Revision 1.18  2003/05/30 02:55:32  steve
 *  Support parameters in real expressions and
 *  as real expressions, and fix multiply and
 *  divide with real results.
 *
 * Revision 1.17  2003/05/20 15:05:33  steve
 *  Do not try to set constants to width 0.
 *
 * Revision 1.16  2003/03/15 18:08:43  steve
 *  Comparison operators do have defined width.
 *
 * Revision 1.15  2003/03/15 04:46:29  steve
 *  Better organize the NetESFunc return type guesses.
 *
 * Revision 1.14  2003/03/01 06:25:30  steve
 *  Add the lex_strings string handler, and put
 *  scope names and system task/function names
 *  into this table. Also, permallocate event
 *  names from the beginning.
 *
 * Revision 1.13  2003/02/06 17:50:23  steve
 *  Real constants have no defined vector width
 *
 * Revision 1.12  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.11  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.10  2002/11/09 01:40:19  steve
 *  Postpone parameter width check to evaluation.
 *
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

