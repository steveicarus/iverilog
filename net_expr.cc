/*
 * Copyright (c) 2002-2012 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "netlist.h"
# include  "compiler.h"
# include  "netmisc.h"
# include  <iostream>

/*
 * the grand default data type is a logic vector.
 */
ivl_variable_type_t NetExpr::expr_type() const
{
      return IVL_VT_LOGIC;
}

/*
 * Create an add/sub node from the two operands. Make a best guess of
 * the
 */
NetEBAdd::NetEBAdd(char op__, NetExpr*l, NetExpr*r, bool lossless_flag)
: NetEBinary(op__, l, r)
{
      NetEConst* tmp;

	/* Catch the special case that one of the operands is an
	   unsized constant number. If so, then we should set the
	   width of that number to the size of the other operand, plus
	   one. This expands the expression to account for the largest
	   possible result.

	   Remember to handle the special case of an unsized constant,
	   which we define to be at least "integer_width" bits.

	   The set_width applied to a constant value will only
	   truncate the constant so far as it can still hold its
	   logical value, so this is safe to do. */
      if ( (tmp = dynamic_cast<NetEConst*>(r))
	   && (! tmp->has_width())
	   && (tmp->expr_width() > l->expr_width() || integer_width > l->expr_width()) ) {

	    verinum tmp_v = trim_vnum(tmp->value());
	    unsigned target_width = l->expr_width();
	    if (target_width < tmp_v.len())
		  target_width = tmp_v.len();
	    if (lossless_flag)
		  target_width += 1;
	    if (target_width < integer_width)
		  target_width = integer_width;

	    r->set_width(target_width);

	      /* Note: This constant value will not gain a defined
		 width from this. Make sure. */
	    assert(! r->has_width() );

	    expr_width(target_width);

      } else if ( (tmp = dynamic_cast<NetEConst*>(l))
	   && (! tmp->has_width())
		  && (tmp->expr_width() > r->expr_width() || integer_width > r->expr_width()) ) {

	    verinum tmp_v = trim_vnum(tmp->value());
	    unsigned target_width = r->expr_width();
	    if (target_width < tmp_v.len())
		  target_width = tmp_v.len();
	    if (lossless_flag)
		  target_width += 1;
	    if (target_width < integer_width)
		  target_width = integer_width;

	    l->set_width(target_width);

	      /* Note: This constant value will not gain a defined
		 width from this. Make sure. */
	    assert(! l->has_width() );

	    expr_width(target_width);

      } else if (r->expr_width() > l->expr_width()) {
	    unsigned loss_pad = lossless_flag? 1 : 0;
	    expr_width(r->expr_width() + loss_pad);

      } else {
	    unsigned loss_pad = lossless_flag? 1 : 0;
	    expr_width(l->expr_width() + loss_pad);
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

ivl_variable_type_t NetEBAdd::expr_type() const
{
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      return IVL_VT_LOGIC;
}

/*
 * Create a comparison operator with two sub-expressions.
 *
 * Handle the special case of an unsized constant on the left or right
 * side by resizing the number to match the other
 * expression. Otherwise, the netlist will have to allow the
 * expressions to have different widths.
 */
NetEBComp::NetEBComp(char op__, NetExpr*l, NetExpr*r)
: NetEBinary(op__, l, r)
{
	// The output of compare is always unsigned.
      cast_signed_base_(false);

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

ivl_variable_type_t NetEBComp::expr_type() const
{
	// Case compare always returns BOOL
      if (op() == 'E' || op() == 'N')
	    return IVL_VT_BOOL;

      if (left()->expr_type() == IVL_VT_LOGIC)
	    return IVL_VT_LOGIC;

      if (right()->expr_type() == IVL_VT_LOGIC)
	    return IVL_VT_LOGIC;

      return IVL_VT_BOOL;
}

NetEBDiv::NetEBDiv(char op__, NetExpr*l, NetExpr*r)
: NetEBinary(op__, l, r)
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

ivl_variable_type_t NetEBDiv::expr_type() const
{
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      return IVL_VT_LOGIC;
}

NetEBMinMax::NetEBMinMax(char op__, NetExpr*l, NetExpr*r)
: NetEBinary(op__, l, r)
{
      expr_width( max(l->expr_width(), r->expr_width()) );
      cast_signed(l->has_sign() || r->has_sign());
}

NetEBMinMax::~NetEBMinMax()
{
}

ivl_variable_type_t NetEBMinMax::expr_type() const
{
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;
      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      return IVL_VT_LOGIC;
}

NetEBMult::NetEBMult(char op__, NetExpr*l, NetExpr*r)
: NetEBinary(op__, l, r)
{
      if (expr_type() == IVL_VT_REAL) {
	    expr_width(1);
	    cast_signed(true);
      } else {
	    expr_width(l->expr_width() + r->expr_width());
	    cast_signed(l->has_sign() && r->has_sign());
      }
}

NetEBMult::~NetEBMult()
{
}

NetEBMult* NetEBMult::dup_expr() const
{
      NetEBMult*result = new NetEBMult(op_, left_->dup_expr(),
				       right_->dup_expr());
      result->expr_width(expr_width());
      return result;
}

ivl_variable_type_t NetEBMult::expr_type() const
{
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      return IVL_VT_LOGIC;
}

NetEBPow::NetEBPow(char op__, NetExpr*l, NetExpr*r)
: NetEBinary(op__, l, r)
{
      assert(op__ == 'p');
	/* You could need up to a * (2^b - 1) bits. */
      expr_width(l->expr_width());
      cast_signed(l->has_sign() || r->has_sign());
}

NetEBPow::~NetEBPow()
{
}

NetEBPow* NetEBPow::dup_expr() const
{
      NetEBPow*result = new NetEBPow(op_, left_->dup_expr(),
				     right_->dup_expr());
      result->set_line(*this);
      return result;
}

ivl_variable_type_t NetEBPow::expr_type() const
{
      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      return IVL_VT_LOGIC;
}

NetEBShift::NetEBShift(char op__, NetExpr*l, NetExpr*r)
: NetEBinary(op__, l, r)
{
      expr_width(l->expr_width());

	// The >>> is signed if the left operand is signed.
      if (op__ == 'R') cast_signed(l->has_sign());
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
      NetEConcat*dup = new NetEConcat(parms_.count(), 0);
      dup->set_line(*this);
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    if (parms_[idx]) {
		  NetExpr*tmp = parms_[idx]->dup_expr();
		  assert(tmp);
		  dup->parms_[idx] = tmp;
	    }


      dup->repeat_ = repeat_? repeat_->dup_expr() : 0;
      dup->repeat_value_ = repeat_value_;
      dup->repeat_calculated_ = repeat_calculated_;
      dup->expr_width(expr_width());

      return dup;
}

unsigned NetEConcat::repeat()
{
      if (repeat_calculated_)
	    return repeat_value_;

      eval_expr(repeat_);

      NetEConst*repeat_const = dynamic_cast<NetEConst*>(repeat_);

	/* This should not be possible, as it was checked earlier to
	   assure that this is a constant expression. */
      if (repeat_const == 0) {
	    cerr << get_fileline() << ": internal error: repeat expression "
		 << "is not a compile time constant." << endl;
	    cerr << get_fileline() << ":               : Expression is: "
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
      expr_width(1);
      cast_signed(true);
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

ivl_variable_type_t NetECReal::expr_type() const
{
      return IVL_VT_REAL;
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
      solving_ = false;
}

NetEParam::NetEParam(Design*d, NetScope*s, perm_string n)
    : des_(d), scope_(s), reference_(scope_->find_parameter(n))
{
      cast_signed_base_(reference_->second.signed_flag);
      solving_ = false;
}

NetEParam::NetEParam(Design*d, NetScope*s, ref_t ref)
    : des_(d), scope_(s), reference_(ref)
{
      cast_signed_base_(reference_->second.signed_flag);
      solving_ = false;
}

NetEParam::~NetEParam()
{
}

bool NetEParam::has_width() const
{
      return false;
}

ivl_variable_type_t NetEParam::expr_type() const
{
      return (*reference_).second.type;
}

NetEParam* NetEParam::dup_expr() const
{
      NetEParam*tmp = new NetEParam(des_, scope_, reference_);
      tmp->solving(solving_);
      tmp->set_line(*this);
      return tmp;
}

void NetEParam::solving(bool arg)
{
      solving_ = arg;
}

bool NetEParam::solving() const
{
      return solving_;
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

NetESFunc::NetESFunc(const char*n, ivl_variable_type_t t,
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

ivl_variable_type_t NetESFunc::expr_type() const
{
      return type_;
}

NetEAccess::NetEAccess(NetBranch*br, ivl_nature_t nat)
: branch_(br), nature_(nat)
{
}

NetEAccess::~NetEAccess()
{
}

ivl_variable_type_t NetEAccess::expr_type() const
{
      return IVL_VT_REAL;
}
