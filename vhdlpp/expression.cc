/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012-2014 / Stephen Williams (steve@icarus.com),
 *                            Maciej Suminski (maciej.suminski@cern.ch)
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

# include  "expression.h"
# include  "subprogram.h"
# include  "parse_types.h"
# include  "scope.h"
# include  <iostream>
# include  <typeinfo>
# include  <cstring>
# include  <ivl_assert.h>
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
      assert(type_==0 || type_==typ);
      type_ = typ;
}

bool Expression::symbolic_compare(const Expression*) const
{
      cerr << get_fileline() << ": internal error: "
	   << "symbolic_compare() method not implemented "
	   << "for " << typeid(*this).name() << endl;
      return false;
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

ExpAggregate::ExpAggregate(std::list<element_t*>*el)
: elements_(el? el->size() : 0)
{
      assert(el);
      size_t idx = 0;
      while (! el->empty()) {
	    assert(idx < elements_.size());
	    elements_[idx++] = el->front();
	    el->pop_front();
      }
}

ExpAggregate::~ExpAggregate()
{
      for (size_t idx = 0 ; idx < elements_.size() ; idx += 1)
	    delete elements_[idx];
}

ExpAggregate::choice_t::choice_t(Expression*exp)
: expr_(exp)
{
}

ExpAggregate::choice_t::choice_t()
{
}

ExpAggregate::choice_t::choice_t(prange_t*rang)
: range_(rang)
{
}

ExpAggregate::choice_t::~choice_t()
{
}

bool ExpAggregate::choice_t::others() const
{
      return expr_.get() == 0 && range_.get() == 0;
}

Expression*ExpAggregate::choice_t::simple_expression(bool detach_flag)
{
      Expression*res = detach_flag? expr_.release() : expr_.get();
      return res;
}

prange_t*ExpAggregate::choice_t::range_expressions(void)
{
      return range_.get();
}

ExpAggregate::element_t::element_t(list<choice_t*>*fields, Expression*val)
: fields_(fields? fields->size() : 0), val_(val)
{
      if (fields) {
	    size_t idx = 0;
	    while (! fields->empty()) {
		  assert(idx < fields_.size());
		  fields_[idx++] = fields->front();
		  fields->pop_front();
	    }
      }
}

ExpAggregate::element_t::~element_t()
{
      for (size_t idx = 0 ; idx < fields_.size() ; idx += 1)
	    delete fields_[idx];

      delete val_;
}

ExpArithmetic::ExpArithmetic(ExpArithmetic::fun_t op, Expression*op1, Expression*op2)
: ExpBinary(op1, op2), fun_(op)
{
	// The xCONCAT type is not actually used.
      assert(op != xCONCAT);
}

ExpArithmetic::~ExpArithmetic()
{
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

ExpCharacter::ExpCharacter(char val)
: value_(val)
{
}

ExpCharacter::~ExpCharacter()
{
}

ExpConcat::ExpConcat(Expression*op1, Expression*op2)
: operand1_(op1), operand2_(op2)
{
}

ExpConcat::~ExpConcat()
{
      delete operand1_;
      delete operand2_;
}

ExpConditional::ExpConditional(Expression*co, list<Expression*>*tru,
			       list<ExpConditional::else_t*>*fal)
: cond_(co)
{
      if (tru) true_clause_.splice(true_clause_.end(), *tru);
      if (fal) else_clause_.splice(else_clause_.end(), *fal);
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
	    else_t*tmp = else_clause_.front();
	    else_clause_.pop_front();
	    delete tmp;
      }
}

ExpConditional::else_t::else_t(Expression*cond, std::list<Expression*>*tru)
: cond_(cond)
{
      if (tru) true_clause_.splice(true_clause_.end(), *tru);
}

ExpConditional::else_t::~else_t()
{
      delete cond_;
      while (! true_clause_.empty()) {
	    Expression*tmp = true_clause_.front();
	    true_clause_.pop_front();
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

ExpFunc::ExpFunc(perm_string nn)
: name_(nn), def_(0)
{
}

ExpFunc::ExpFunc(perm_string nn, list<Expression*>*args)
: name_(nn), argv_(args->size()), def_(0)
{
      for (size_t idx = 0; idx < argv_.size() ; idx += 1) {
	    ivl_assert(*this, !args->empty());
	    argv_[idx] = args->front();
	    args->pop_front();
      }
      ivl_assert(*this, args->empty());
}

ExpFunc::~ExpFunc()
{
      for (size_t idx = 0 ; idx < argv_.size() ; idx += 1)
	    delete argv_[idx];
}

const VType* ExpFunc::func_ret_type() const
{
    return def_ ? def_->peek_return_type() : NULL;
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

ExpReal::ExpReal(double val)
: value_(val)
{
}

ExpReal::~ExpReal()
{
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

ExpName::ExpName(perm_string nn, list<Expression*>*indices)
: name_(nn), index_(0), lsb_(0)
{
	/* For now, assume a single index. */
      ivl_assert(*this, indices->size() == 1);

      index_ = indices->front();
      indices->pop_front();
}

ExpName::ExpName(perm_string nn, Expression*msb, Expression*lsb)
: name_(nn), index_(msb), lsb_(lsb)
{
}

ExpName::ExpName(ExpName*prefix, perm_string nn)
: prefix_(prefix), name_(nn), index_(0), lsb_(0)
{
}

ExpName::ExpName(ExpName*prefix, perm_string nn, Expression*msb, Expression*lsb)
: prefix_(prefix), name_(nn), index_(msb), lsb_(lsb)
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

void ExpName::set_range(Expression*msb, Expression*lsb)
{
      assert(index_==0);
      index_ = msb;
      assert(lsb_==0);
      lsb_ = lsb;
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

ExpCast::ExpCast(Expression*base, const VType*type) :
    base_(base), type_(type)
{
}

ExpCast::~ExpCast()
{
}

ExpNew::ExpNew(Expression*size) :
    size_(size)
{
}

ExpNew::~ExpNew()
{
    delete size_;
}
