/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012-2015 / Stephen Williams (steve@icarus.com),
 * Copyright CERN 2016
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
# include  "library.h"
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

ExpAttribute::ExpAttribute(perm_string nam, list<Expression*>*args)
: name_(nam), args_(args)
{
}

ExpAttribute::~ExpAttribute()
{
      if(args_) {
	    for(list<Expression*>::iterator it = args_->begin();
                    it != args_->end(); ++it) {
		delete *it;
            }
      }

      delete args_;
}

list<Expression*>*ExpAttribute::clone_args() const {
      list<Expression*>*new_args = NULL;

      if(args_) {
	    for(list<Expression*>::iterator it = args_->begin();
                    it != args_->end(); ++it) {
		new_args->push_back((*it)->clone());
            }
      }

      return new_args;
}

void ExpAttribute::visit_args(ExprVisitor& func)
{
      func.down();
      func(this);

      if(args_) {
          for(list<Expression*>::iterator it = args_->begin();
                    it != args_->end(); ++it) {
              (*it)->visit(func);
          }
      }

      func.up();
}

ExpObjAttribute::ExpObjAttribute(ExpName*base, perm_string name, list<Expression*>*args)
: ExpAttribute(name, args), base_(base)
{
}

ExpObjAttribute::~ExpObjAttribute()
{
    delete base_;
}

Expression*ExpObjAttribute::clone() const
{
      return new ExpObjAttribute(static_cast<ExpName*>(base_->clone()),
                                 name_, clone_args());
}

void ExpObjAttribute::visit(ExprVisitor&func)
{
      func.down();
      func(this);
      visit_args(func);
      base_->visit(func);
      func.up();
}

ExpTypeAttribute::ExpTypeAttribute(const VType*base, perm_string name, list<Expression*>*args)
: ExpAttribute(name, args), base_(base)
{
}

Expression*ExpTypeAttribute::clone() const
{
      return new ExpTypeAttribute(base_, name_, clone_args());
}

void ExpTypeAttribute::visit(ExprVisitor&func)
{
      func.down();
      func(this);
      visit_args(func);
      func.up();
}

const perm_string ExpAttribute::LEFT = perm_string::literal("left");
const perm_string ExpAttribute::RIGHT = perm_string::literal("right");

ExpBinary::ExpBinary(Expression*op1, Expression*op2)
: operand1_(op1), operand2_(op2)
{
}

ExpBinary::~ExpBinary()
{
      delete operand1_;
      delete operand2_;
}

bool ExpBinary::eval_operand1(Entity*ent, ScopeBase*scope, int64_t&val) const
{
      return operand1_->evaluate(ent, scope, val);
}

bool ExpBinary::eval_operand2(Entity*ent, ScopeBase*scope, int64_t&val) const
{
      return operand2_->evaluate(ent, scope, val);
}

void ExpBinary::visit(ExprVisitor&func)
{
      func.down();
      func(this);
      operand1_->visit(func);
      operand2_->visit(func);
      func.up();
}

ExpUnary::ExpUnary(Expression*op1)
: operand1_(op1)
{
}

ExpUnary::~ExpUnary()
{
      delete operand1_;
}

void ExpUnary::visit(ExprVisitor&func)
{
    func.down();
    func(this);
    operand1_->visit(func);
    func.up();
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
      delete el;
}

ExpAggregate::~ExpAggregate()
{
      for(std::vector<element_t*>::iterator it = elements_.begin();
              it != elements_.end(); ++it) {
        delete *it;
      }

      for(std::vector<choice_element>::iterator it = aggregate_.begin();
              it != aggregate_.end(); ++it) {
        delete it->choice;
        if(!it->alias_flag) delete it->expr;
      }
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

void ExpAggregate::visit(ExprVisitor&func)
{
    func.down();
    func(this);

    for(std::vector<element_t*>::iterator it = elements_.begin();
            it != elements_.end(); ++it) {
        (*it)->extract_expression()->visit(func);
    }

    for(std::vector<choice_element>::iterator it = aggregate_.begin();
            it != aggregate_.end(); ++it) {
        if(Expression*choice_expr = it->choice->simple_expression(false))
            choice_expr->visit(func);

        it->expr->visit(func);
    }

    func.up();
}

ExpAggregate::choice_t::choice_t(Expression*exp)
: expr_(exp)
{
}

ExpAggregate::choice_t::choice_t()
{
}

ExpAggregate::choice_t::choice_t(ExpRange*rang)
: range_(rang)
{
}

ExpAggregate::choice_t::choice_t(const choice_t&other)
{
    if(Expression*e = other.expr_.get())
        expr_.reset(e->clone());

    if(other.range_.get())
        range_.reset(static_cast<ExpRange*>(other.range_.get()->clone()));
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

ExpRange*ExpAggregate::choice_t::range_expressions(void)
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

void ExpConcat::visit(ExprVisitor&func)
{
      func.down();
      func(this);
      operand1_->visit(func);
      operand2_->visit(func);
      func.up();
}

ExpConditional::ExpConditional(Expression*co, list<Expression*>*tru,
			       list<ExpConditional::case_t*>*options)
{
      if(co && tru) options_.push_back(new case_t(co, tru));
      if(options) options_.splice(options_.end(), *options);
}

ExpConditional::~ExpConditional()
{
      while (!options_.empty()) {
	    case_t*tmp = options_.front();
	    options_.pop_front();
	    delete tmp;
      }
}

Expression*ExpConditional::clone() const
{
      std::list<case_t*>*new_options = NULL;
      if(!options_.empty()) {
          new_options = new std::list<case_t*>();

          for(std::list<case_t*>::const_iterator it = options_.begin();
                  it != options_.end(); ++it) {
              new_options->push_back(new case_t(**it));
          }
      }

      return new ExpConditional(NULL, NULL, new_options);
}

void ExpConditional::visit(ExprVisitor&func)
{
      func.down();
      func(this);

      for(std::list<case_t*>::iterator it = options_.begin();
              it != options_.end(); ++it)
          (*it)->visit(func);

      func.up();
}

ExpConditional::case_t::case_t(Expression*cond, std::list<Expression*>*tru)
: cond_(cond)
{
      if (tru) true_clause_.splice(true_clause_.end(), *tru);
}

ExpConditional::case_t::case_t(const case_t&other)
: LineInfo(other)
{
      cond_ = other.cond_->clone();
      for(std::list<Expression*>::const_iterator it = other.true_clause_.begin();
            it != other.true_clause_.end(); ++it) {
          true_clause_.push_back((*it)->clone());
      }
}

ExpConditional::case_t::~case_t()
{
      delete cond_;
      while (! true_clause_.empty()) {
	    Expression*tmp = true_clause_.front();
	    true_clause_.pop_front();
	    delete tmp;
      }
}

ExpSelected::ExpSelected(Expression*selector, std::list<case_t*>*options)
: ExpConditional(NULL, NULL, options), selector_(selector)
{
    // Currently condition field contains only value,
    // so substitute it with a comparison to create a valid condition
    for(std::list<case_t*>::iterator it = options_.begin();
            it != options_.end(); ++it) {
        Expression*cond = (*it)->condition();

        if(cond)
            (*it)->set_condition(new ExpRelation(ExpRelation::EQ, selector_->clone(), cond));
    }
}

ExpSelected::~ExpSelected()
{
}

Expression*ExpSelected::clone() const
{
      std::list<case_t*>*new_options = NULL;
      if(!options_.empty()) {
          new_options = new std::list<case_t*>();

          for(std::list<case_t*>::const_iterator it = options_.begin();
                  it != options_.end(); ++it) {
              new_options->push_back(new case_t(**it));
          }
      }

      return new ExpSelected(selector_->clone(), new_options);
}

void ExpConditional::case_t::visit(ExprVisitor&func)
{
      func.down();
      if(cond_)
          cond_->visit(func);

      for(std::list<Expression*>::iterator it = true_clause_.begin();
              it != true_clause_.end(); ++it)
          (*it)->visit(func);
      func.up();
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

void ExpFunc::visit(ExprVisitor&func)
{
    func.down();
    func(this);

    if(!argv_.empty()) {
        for(std::vector<Expression*>::iterator it = argv_.begin();
                it != argv_.end(); ++it)
            (*it)->visit(func);
    }

    func.up();
}

const VType* ExpFunc::func_ret_type() const
{
    return def_ ? def_->peek_return_type() : NULL;
}

SubprogramHeader*ExpFunc::match_signature(Entity*ent, ScopeBase*scope) const
{
    SubprogramHeader*prog = NULL;
    list<const VType*> arg_types;

      // Create a list of argument types to find a matching subprogram
    for(vector<Expression*>::const_iterator it = argv_.begin();
            it != argv_.end(); ++it) {
        arg_types.push_back((*it)->probe_type(ent, scope));
    }

    prog = scope->match_subprogram(name_, &arg_types);

    if(!prog)
        prog = library_match_subprogram(name_, &arg_types);

    if(!prog) {
        cerr << get_fileline() << ": sorry: could not find function ";
        emit_subprogram_sig(cerr, name_, arg_types);
        cerr << endl;
        ivl_assert(*this, false);
    }

    return prog;
}

ExpInteger::ExpInteger(int64_t val)
: value_(val)
{
}

ExpInteger::~ExpInteger()
{
}

bool ExpInteger::evaluate(Entity*, ScopeBase*, int64_t&val) const
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
: name_(nn), indices_(NULL)
{
}

ExpName::ExpName(perm_string nn, list<Expression*>*indices)
: name_(nn), indices_(indices)
{
}

ExpName::ExpName(ExpName*prefix, perm_string nn, std::list<Expression*>*indices)
: prefix_(prefix), name_(nn), indices_(indices)
{
}

ExpName::~ExpName()
{
    if(indices_) {
        for(list<Expression*>::iterator it = indices_->begin();
                it != indices_->end(); ++it) {
            delete *it;
        }

        delete indices_;
    }
}

Expression*ExpName::clone() const {
    list<Expression*>*new_indices = NULL;

    if(indices_) {
        new_indices = new list<Expression*>();

        for(list<Expression*>::const_iterator it = indices_->begin();
                it != indices_->end(); ++it) {
            new_indices->push_back((*it)->clone());
        }
    }

    return new ExpName(static_cast<ExpName*>(safe_clone(prefix_.get())),
            name_, new_indices);
}

void ExpName::add_index(std::list<Expression*>*idx)
{
      if(!indices_)
          indices_ = new list<Expression*>();

      indices_->splice(indices_->end(), *idx);
}

bool ExpName::symbolic_compare(const Expression*that) const
{
      const ExpName*that_name = dynamic_cast<const ExpName*> (that);
      if (that_name == 0)
	    return false;

      if (name_ != that_name->name_)
	    return false;

      if (that_name->indices_ && !indices_)
	    return false;
      if (indices_ && !that_name->indices_)
	    return false;

      if (indices_) {
	    assert(that_name->indices_);

            if(indices_->size() != that_name->indices_->size())
                return false;

            list<Expression*>::const_iterator it, jt;
            it = indices_->begin();
            jt = that_name->indices_->begin();

            for(unsigned int i = 0; i < indices_->size(); ++i) {
                if(!(*it)->symbolic_compare(*jt))
                    return false;

                ++it;
                ++jt;
            }
      }

      return true;
}

Expression*ExpName::index(unsigned int number) const
{
    if(!indices_)
        return NULL;

    if(number >= indices_->size())
        return NULL;

    if(number == 0)
        return indices_->front();

    list<Expression*>::const_iterator it = indices_->begin();
    advance(it, number);

    return *it;
}

void ExpName::visit(ExprVisitor&func)
{
      func.down();
      func(this);

      if(prefix_.get())
          prefix_.get()->visit(func);

      if(indices_) {
          for(list<Expression*>::const_iterator it = indices_->begin();
                  it != indices_->end(); ++it) {
              (*it)->visit(func);
          }
      }

      func.up();
}

int ExpName::index_t::emit(ostream&out, Entity*ent, ScopeBase*scope) const
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

ExpScopedName::ExpScopedName(perm_string scope, ExpName*exp)
: scope_name_(scope), scope_(NULL), name_(exp)
{
}

ExpScopedName::~ExpScopedName()
{
    delete name_;
}

void ExpScopedName::visit(ExprVisitor&func)
{
    func.down();
    func(this);
    name_->visit(func);
    func.up();
}

ScopeBase*ExpScopedName::get_scope(const ScopeBase*scope)
{
    if(!scope_)
        scope_ = scope->find_scope(scope_name_);

    return scope_;
}

ScopeBase*ExpScopedName::get_scope(const ScopeBase*scope) const
{
    return scope_ ? scope_ : scope->find_scope(scope_name_);
}

ExpShift::ExpShift(ExpShift::shift_t op, Expression*op1, Expression*op2)
: ExpBinary(op1, op2), shift_(op)
{
}

ExpString::ExpString(const char* value)
: value_(value)
{
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

ExpUMinus::ExpUMinus(Expression*op1)
: ExpUnary(op1)
{
}

ExpUMinus::~ExpUMinus()
{
}

ExpCast::ExpCast(Expression*base, const VType*type) :
    base_(base), type_(type)
{
}

ExpCast::~ExpCast()
{
}

void ExpCast::visit(ExprVisitor&func)
{
    func.down();
    func(this);
    base_->visit(func);
    func.up();
}

ExpNew::ExpNew(Expression*size) :
    size_(size)
{
}

ExpNew::~ExpNew()
{
    delete size_;
}

void ExpNew::visit(ExprVisitor&func)
{
    func.down();
    func(this);
    size_->visit(func);
    func.up();
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

ExpRange::ExpRange(Expression*left_idx, Expression*right_idx, range_dir_t dir)
: left_(left_idx), right_(right_idx), direction_(dir), range_expr_(false),
    range_base_(NULL)
{
}

ExpRange::ExpRange(ExpName*base, bool reverse_range)
: left_(NULL), right_(NULL), direction_(AUTO), range_expr_(true),
    range_base_(base), range_reverse_(reverse_range)
{
}

ExpRange::~ExpRange()
{
    delete left_;
    delete right_;
    delete range_base_;
}

Expression*ExpRange::clone() const
{
    if(range_expr_)
        return new ExpRange(static_cast<ExpName*>(range_base_->clone()), range_reverse_);
    else
        return new ExpRange(left_->clone(), right_->clone(), direction_);
}

Expression* ExpRange::msb()
{
    ivl_assert(*this, direction() != AUTO);

    switch(direction()) {
        case DOWNTO: return left_;
        case TO: return right_;
        default: return NULL;
    }

    return NULL;
}

Expression* ExpRange::lsb()
{
    ivl_assert(*this, direction() != AUTO);

    switch(direction()) {
        case DOWNTO: return right_;
        case TO: return left_;
        default: return NULL;
    }

    return NULL;
}

Expression*ExpRange::left()
{
    if(range_expr_ && !left_)
        // TODO check if it is an object or type
        left_ = new ExpObjAttribute(static_cast<ExpName*>(range_base_->clone()),
                                    ExpAttribute::LEFT, NULL);

    return left_;
}

Expression*ExpRange::right()
{
    if(range_expr_ && !right_)
        // TODO check if it is an object or type
        right_ = new ExpObjAttribute(static_cast<ExpName*>(range_base_->clone()),
                                    ExpAttribute::RIGHT, NULL);
    return right_;
}

ExpDelay::ExpDelay(Expression*expr, Expression*delay)
: expr_(expr), delay_(delay)
{
}

ExpDelay::~ExpDelay()
{
    delete expr_;
    delete delay_;
}

void ExpDelay::visit(ExprVisitor&func)
{
    func.down();
    func(this);
    expr_->visit(func);
    delay_->visit(func);
    func.up();
}
