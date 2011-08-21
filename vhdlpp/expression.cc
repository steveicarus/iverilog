/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "expression.h"
# include  "scope.h"
# include  <iostream>
# include  <typeinfo>
# include  <cstring>
# include  <cassert>

using namespace std;

Expression::Expression()
: type_(0)
{
}

Expression::~Expression()
{
}

void Expression::set_type(const VType*typ)
{
      assert(type_ == 0);
      type_ = typ;
}

bool Expression::evaluate(ScopeBase*, int64_t&) const
{
      return false;
}

bool Expression::symbolic_compare(const Expression*) const
{
      cerr << get_fileline() << ": internal error: "
	   << "symbolic_compare() method not implemented "
	   << "for " << typeid(*this).name() << endl;
      return false;
}

bool ExpName::symbolic_compare(const Expression*that) const
{
      const ExpName*that_name = dynamic_cast<const ExpName*> (that);
      if (that_name == 0)
	    return false;

      if (name_ != that_name->name_)
	    return false;

      if (that_name->index_ && !index_)
	    return false;
      if (index_ && !that_name->index_)
	    return false;

      if (index_) {
	    assert(that_name->index_);
	    return index_->symbolic_compare(that_name->index_);
      }

      return true;
}

ExpAttribute::ExpAttribute(ExpName*bas, perm_string nam)
: base_(bas), name_(nam)
{
}

ExpAttribute::~ExpAttribute()
{
      delete base_;
}

ExpBinary::ExpBinary(Expression*op1, Expression*op2)
: operand1_(op1), operand2_(op2)
{
}

ExpBinary::~ExpBinary()
{
      delete operand1_;
      delete operand2_;
}

bool ExpBinary::eval_operand1(ScopeBase*scope, int64_t&val) const
{
      return operand1_->evaluate(scope, val);
}

bool ExpBinary::eval_operand2(ScopeBase*scope, int64_t&val) const
{
      return operand2_->evaluate(scope, val);
}

ExpUnary::ExpUnary(Expression*op1)
: operand1_(op1)
{
}

ExpUnary::~ExpUnary()
{
      delete operand1_;
}

ExpArithmetic::ExpArithmetic(ExpArithmetic::fun_t op, Expression*op1, Expression*op2)
: ExpBinary(op1, op2), fun_(op)
{
}

ExpArithmetic::~ExpArithmetic()
{
}

bool ExpArithmetic::evaluate(ScopeBase*scope, int64_t&val) const
{
      int64_t val1, val2;
      bool rc;

      rc = eval_operand1(scope, val1);
      if (rc == false)
	    return false;

      rc = eval_operand2(scope, val2);
      if (rc == false)
	    return false;

      switch (fun_) {
	  case PLUS:
	    val = val1 + val2;
	    break;
	  case MINUS:
	    val = val1 - val2;
	    break;
	  case MULT:
	    val = val1 * val2;
	    break;
	  case DIV:
	    if (val2 == 0)
		  return false;
	    val = val1 / val2;
	    break;
	  case MOD:
	    if (val2 == 0)
		  return false;
	    val = val1 % val2;
	    break;
	  case REM:
	    return false;
	  case POW:
	    return false;
	  case CONCAT:
	    return false;
      }

      return true;
}

/*
 *  Store bitstrings in little-endian order.
 */
ExpBitstring::ExpBitstring(const char*val)
: value_(strlen(val))
{
      for (size_t idx = value_.size() ; idx > 0 ; idx -= 1)
	    value_[idx-1] = *val++;
}

ExpBitstring::~ExpBitstring()
{
}

ExpCast::ExpCast(const VType*typ, Expression*arg)
: res_type_(typ), arg_(arg)
{
}

ExpCast::~ExpCast()
{
}

ExpCharacter::ExpCharacter(char val)
: value_(val)
{
}

ExpCharacter::~ExpCharacter()
{
}

ExpConditional::ExpConditional(Expression*co, list<Expression*>*tru, list<Expression*>*els)
: cond_(co)
{
      if (tru) true_clause_.splice(true_clause_.end(), *tru);
      if (els) else_clause_.splice(else_clause_.end(), *els);
}

ExpConditional::~ExpConditional()
{
      delete cond_;
      while (! true_clause_.empty()) {
	    Expression*tmp = true_clause_.front();
	    true_clause_.pop_front();
	    delete tmp;
      }
      while (! else_clause_.empty()) {
	    Expression*tmp = else_clause_.front();
	    else_clause_.pop_front();
	    delete tmp;
      }
}

ExpEdge::ExpEdge(ExpEdge::fun_t typ, Expression*op)
: ExpUnary(op), fun_(typ)
{
}

ExpEdge::~ExpEdge()
{
}

ExpInteger::ExpInteger(int64_t val)
: value_(val)
{
}

ExpInteger::~ExpInteger()
{
}

bool ExpInteger::evaluate(ScopeBase*, int64_t&val) const
{
      val = value_;
      return true;
}

ExpLogical::ExpLogical(ExpLogical::fun_t ty, Expression*op1, Expression*op2)
: ExpBinary(op1, op2), fun_(ty)
{
}

ExpLogical::~ExpLogical()
{
}

ExpName::ExpName(perm_string nn)
: name_(nn), index_(0), lsb_(0)
{
}

ExpName::ExpName(perm_string nn, Expression*ix)
: name_(nn), index_(ix), lsb_(0)
{
}

ExpName::ExpName(perm_string nn, Expression*msb, Expression*lsb)
: name_(nn), index_(msb), lsb_(lsb)
{
}

ExpName::~ExpName()
{
      delete index_;
}

const char* ExpName::name() const
{
      return name_;
}

bool ExpName::evaluate(ScopeBase*scope, int64_t&val) const
{
      const VType*type;
      Expression*exp;

      bool rc = scope->find_constant(name_, type, exp);
      if (rc == false) {
	    cerr << "XXXX Unable to evaluate name " << name_ << "." << endl;
	    return false;
      }

      return exp->evaluate(scope, val);
}

ExpRelation::ExpRelation(ExpRelation::fun_t ty, Expression*op1, Expression*op2)
: ExpBinary(op1, op2), fun_(ty)
{
}

ExpRelation::~ExpRelation()
{
}

ExpString::ExpString(const char* value)
: value_(strlen(value))
{
      for(size_t idx = 0; idx < value_.size(); idx += 1)
	    value_[idx] = value[idx];
}

ExpString::~ExpString()
{
}

ExpUAbs::ExpUAbs(Expression*op1)
: ExpUnary(op1)
{
}

ExpUAbs::~ExpUAbs()
{
}

ExpUNot::ExpUNot(Expression*op1)
: ExpUnary(op1)
{
}

ExpUNot::~ExpUNot()
{
}
