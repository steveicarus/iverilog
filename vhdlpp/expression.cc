/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012-2015 / Stephen Williams (steve@icarus.com),
 * @author Maciej Suminski (maciej.suminski@cern.ch)
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

Expression*ExpAttribute::clone() const
{
      return new ExpAttribute(static_cast<ExpName*>(base_->clone()), name_);
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

Expression* ExpAggregate::clone() const
{
      std::list<element_t*>*new_elements = NULL;

      if(!elements_.empty()) {
          new_elements = new std::list<element_t*>();
          for(std::vector<element_t*>::const_iterator it = elements_.begin();
                  it != elements_.end(); ++it) {
              new_elements->push_back(new element_t(**it));
          }
      }

      assert(aggregate_.empty());   // cloning should not happen after elab

      return new ExpAggregate(new_elements);
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

ExpAggregate::choice_t::choice_t(const choice_t&other)
{
    if(Expression*e = other.expr_.get())
        expr_.reset(e->clone());

    if(other.range_.get())
        range_.reset(new prange_t(*other.range_.get()));
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

ExpAggregate::element_t::element_t(const ExpAggregate::element_t&other)
{
      fields_.reserve(other.fields_.size());

      for(std::vector<choice_t*>::const_iterator it = other.fields_.begin();
              it != other.fields_.end(); ++it) {
          fields_.push_back(*it);
      }

      val_ = other.val_->clone();
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

Expression*ExpConditional::clone() const
{
      std::list<Expression*>*new_true_clause = NULL;
      if(!true_clause_.empty()) {
          new_true_clause = new std::list<Expression*>();

          for(std::list<Expression*>::const_iterator it = true_clause_.begin();
                  it != true_clause_.end(); ++it) {
              new_true_clause->push_back((*it)->clone());
          }
      }

      std::list<else_t*>*new_else_clause = NULL;
      if(!else_clause_.empty()) {
          new_else_clause = new std::list<else_t*>();

          for(std::list<else_t*>::const_iterator it = else_clause_.begin();
                  it != else_clause_.end(); ++it) {
              new_else_clause->push_back(new else_t(**it));
          }
      }

      return new ExpConditional(cond_->clone(), new_true_clause, new_else_clause);
}

ExpConditional::else_t::else_t(Expression*cond, std::list<Expression*>*tru)
: cond_(cond)
{
      if (tru) true_clause_.splice(true_clause_.end(), *tru);
}

ExpConditional::else_t::else_t(const else_t&other)
: LineInfo(other)
{
      cond_ = other.cond_->clone();
      for(std::list<Expression*>::const_iterator it = other.true_clause_.begin();
            it != other.true_clause_.end(); ++it) {
          true_clause_.push_back((*it)->clone());
      }
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

Expression*ExpFunc::clone() const {
    std::list<Expression*>*new_args = NULL;

    if(!argv_.empty()) {
        new_args = new std::list<Expression*>();
        for(std::vector<Expression*>::const_iterator it = argv_.begin();
                it != argv_.end(); ++it)
            new_args->push_back((*it)->clone());
    }

    ExpFunc*f = new ExpFunc(name_, new_args);
    f->def_ = def_;

    return f;
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

int ExpName::index_t::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      out << "(";

      if(idx_ && size_) {
        errors += idx_->emit(out, ent, scope);
        out << "*";
        errors += size_->emit(out, ent, scope);
      }

      if(offset_) {
        if(idx_ && size_)
          out << "+";
        errors += offset_->emit(out, ent, scope);
      }

      out << ")";
      return errors;
}

ExpRelation::ExpRelation(ExpRelation::fun_t ty, Expression*op1, Expression*op2)
: ExpBinary(op1, op2), fun_(ty)
{
}

ExpRelation::~ExpRelation()
{
}

ExpShift::ExpShift(ExpShift::shift_t op, Expression*op1, Expression*op2)
: ExpBinary(op1, op2), shift_(op)
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

ExpTime::ExpTime(uint64_t amount, timeunit_t unit)
: amount_(amount), unit_(unit)
{
}

double ExpTime::to_fs() const
{
    double val = amount_;

    switch(unit_) {
        case FS: break;
        case PS: val *= 1e3; break;
        case NS: val *= 1e6; break;
        case US: val *= 1e9; break;
        case MS: val *= 1e12; break;
        case S:  val *= 1e15; break;
        default: ivl_assert(*this, false); break;
    }

    return val;
}
