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
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */

# include  "expression.h"
# include  "architec.h"
# include  "entity.h"
# include  "vsignal.h"
# include  <iostream>
# include  <typeinfo>
# include  "ivl_assert.h"

using namespace std;

int Expression::elaborate_lval(Entity*, Architecture*, bool)
{
      cerr << get_fileline() << ": error: Expression is not a valie l-value." << endl;
      return 1;
}

const VType* Expression::probe_type(Entity*, Architecture*) const
{
      return 0;
}

int ExpName::elaborate_lval(Entity*ent, Architecture*arc, bool is_sequ)
{
      int errors = 0;

      const VType*found_type = 0;

      if (const InterfacePort*cur = ent->find_port(name_)) {
	    if (cur->mode != PORT_OUT) {
		  cerr << get_fileline() << ": error: Assignment to "
			"input port " << name_ << "." << endl;
		  return errors += 1;
	    }

	    if (is_sequ)
		  ent->set_declaration_l_value(name_, is_sequ);

	    found_type = cur->type;

      } else if (Signal*sig = arc->find_signal(name_)) {
	      // Tell the target signal that this may be a sequential l-value.
	    if (is_sequ) sig->count_ref_sequ();

	    found_type = sig->peek_type();

      } else if (Variable*var = arc->find_variable(name_)) {
	      // Tell the target signal that this may be a sequential l-value.
	    if (is_sequ) var->count_ref_sequ();

	    found_type = var->peek_type();
      }

      if (found_type == 0) {
	    cerr << get_fileline() << ": error: Signal/variable " << name_
		 << " not found in this context." << endl;
	    return errors + 1;
      }

      if (const VTypeArray*array = dynamic_cast<const VTypeArray*>(found_type)) {
	    if (index_ && !lsb_) {
		    // If the name is an array or a vector, then an
		    // indexed name has the type of the element.
		  found_type = array->element_type();

	    } else if (index_ && lsb_) {
		    // If the name is an array, then a part select is
		    // also an array, but with different bounds.
		  int64_t use_msb, use_lsb;
		  bool flag;

		  flag = index_->evaluate(arc, use_msb);
		  ivl_assert(*this, flag);
		  flag = lsb_->evaluate(arc, use_lsb);
		  ivl_assert(*this, flag);

		  vector<VTypeArray::range_t> use_dims (1);
		  use_dims[0] = VTypeArray::range_t(use_msb, use_lsb);
		  found_type = new VTypeArray(array->element_type(), use_dims);
	    }
      }

      set_type(found_type);
      return errors;
}

int ExpName::elaborate_rval(Entity*ent, Architecture*arc, const InterfacePort*lval)
{
      int errors = 0;

      if (const InterfacePort*cur = ent->find_port(name_)) {
        /* IEEE 1076-2008, p.80:
        * For a formal port IN, associated port should be IN, OUT, INOUT or BUFFER
        * For a formal port OUT, associated port should be OUT, INOUT or BUFFER
        * For a formal port INOUT, associated prot should be OUT, INOUT or BUFFER
        * For a formal port BUFFER, associated port should be OUT, INOUT or BUFFER
        */
        switch(lval->mode) {
              case PORT_OUT:
              //case PORT_INOUT:
                  if (cur->mode == PORT_IN) {
                      cerr << get_fileline() << ": error: Connecting "
                      "formal output port " << lval->name << " to actual input port "
                      << name_ << "." << endl;
                      errors += 1;
                  }
                  break;
              case PORT_IN:
              case PORT_NONE:
              default:
                break;
        }
      } else if (arc->find_signal(name_)) {
	      /* OK */

      } else {
            cerr << get_fileline() << ": error: No port or signal " << name_
		 << " to be used as r-value." << endl;
            errors += 1;
      }

      return errors;
}

int ExpNameALL::elaborate_lval(Entity*ent, Architecture*arc, bool is_sequ)
{
      return Expression::elaborate_lval(ent, arc, is_sequ);
}

int Expression::elaborate_expr(Entity*, Architecture*, const VType*)
{
      cerr << get_fileline() << ": internal error: I don't know how to elaborate expression type=" << typeid(*this).name() << endl;
      return 1;
}

const VType* ExpBinary::probe_type(Entity*ent, Architecture*arc) const
{
      const VType*t1 = operand1_->probe_type(ent, arc);
      const VType*t2 = operand2_->probe_type(ent, arc);

      if (t1 == 0)
	    return t2;
      if (t2 == 0)
	    return t1;

      if (t1 == t2)
	    return t1;

      cerr << get_fileline() << ": internal error: I don't know how to resolve types of generic binary expressions." << endl;
      return 0;
}

int ExpBinary::elaborate_exprs(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      errors += operand1_->elaborate_expr(ent, arc, ltype);
      errors += operand2_->elaborate_expr(ent, arc, ltype);
      return errors;
}

int ExpAggregate::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      if (ltype == 0) {
	    cerr << get_fileline() << ": error: Elaboration of aggregate types needs wel known type context?" << endl;
	    return 1;
      }

      set_type(ltype);

      if (const VTypeArray*larray = dynamic_cast<const VTypeArray*>(ltype)) {
	    return elaborate_expr_array_(ent, arc, larray);
      }

      cerr << get_fileline() << ": internal error: I don't know how to elaborate aggregate expressions. type=" << typeid(*ltype).name() << endl;
      return 1;
}

/*
 * Elaboration of array aggregates is elaboration of the element
 * expressions using the element type as the ltype for the
 * subexpression.
 */
int ExpAggregate::elaborate_expr_array_(Entity*ent, Architecture*arc, const VTypeArray*ltype)
{
      const VType*element_type = ltype->element_type();
      int errors = 0;
      size_t choice_count = 0;

      for (size_t edx = 0 ; edx < elements_.size() ; edx += 1) {
	    element_t*ecur = elements_[edx];
	    choice_count += ecur->count_choices();
      }

      aggregate_.resize(choice_count);

      size_t cdx = 0;
      for (size_t edx = 0 ; edx < elements_.size() ; edx += 1) {
	    element_t*ecur = elements_[edx];
	    ecur->map_choices(&aggregate_[cdx]);
	    cdx += ecur->count_choices();
      }

      ivl_assert(*this, cdx == choice_count);

      for (size_t idx = 0 ; idx < aggregate_.size() ; idx += 1) {
	    if (aggregate_[idx].alias_flag)
		  continue;

	    errors += aggregate_[idx].expr->elaborate_expr(ent, arc, element_type);
      }

      elements_.clear();
      return errors;
}

void ExpAggregate::element_t::map_choices(ExpAggregate::choice_element*dst)
{
      for (size_t idx = 0 ; idx < fields_.size() ; idx += 1) {
	    dst->choice = fields_[idx];
	    dst->expr = val_;
	    dst->alias_flag = (idx != 0);
	    dst += 1;
      }
}

int ExpArithmetic::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, arc);
      }

      ivl_assert(*this, ltype != 0);
      errors += elaborate_exprs(ent, arc, ltype);
      return errors;
}

int ExpAttribute::elaborate_expr(Entity*ent, Architecture*arc, const VType*)
{
      int errors = 0;
      const VType*sub_type = base_->probe_type(ent, arc);
      errors += base_->elaborate_expr(ent, arc, sub_type);
      return errors;
}

int ExpBitstring::elaborate_expr(Entity*, Architecture*, const VType*)
{
      int errors = 0;
      return errors;
}

int ExpCharacter::elaborate_expr(Entity*, Architecture*, const VType*ltype)
{
      ivl_assert(*this, ltype != 0);
      set_type(ltype);
      return 0;
}

const VType* ExpConditional::probe_type(Entity*, Architecture*) const
{
      return 0;
}

int ExpConditional::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0)
	    ltype = probe_type(ent, arc);

      ivl_assert(*this, ltype);

      set_type(ltype);

	/* Note that the type for the condition expression need not
	   have anything to do with the type of this expression. */
      errors += cond_->elaborate_expr(ent, arc, 0);

      for (list<Expression*>::const_iterator cur = true_clause_.begin()
		 ; cur != true_clause_.end() ; ++cur) {
	    errors += (*cur)->elaborate_expr(ent, arc, ltype);
      }

      for (list<Expression*>::const_iterator cur = else_clause_.begin()
		 ; cur != else_clause_.end() ; ++cur) {
	    errors += (*cur)->elaborate_expr(ent, arc, ltype);
      }

      return errors;
}

int ExpFunc::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      for (size_t idx = 0 ; idx < argv_.size() ; idx += 1) {
	    const VType*tmp = argv_[idx]->probe_type(ent, arc);
	    errors += argv_[idx]->elaborate_expr(ent, arc, tmp);
      }

      return errors;
}

const VType* ExpInteger::probe_type(Entity*, Architecture*) const
{
      return primitive_INTEGER;
}

int ExpInteger::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, arc);
      }

      ivl_assert(*this, ltype != 0);

      return errors;
}

int ExpLogical::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, arc);
      }

      ivl_assert(*this, ltype != 0);
      errors += elaborate_exprs(ent, arc, ltype);
      return errors;
}

const VType* ExpName::probe_type(Entity*ent, Architecture*arc) const
{
      if (const InterfacePort*cur = ent->find_port(name_))
	    return cur->type;

      if (Signal*sig = arc->find_signal(name_))
	    return sig->peek_type();

      if (Variable*var = arc->find_variable(name_))
	    return var->peek_type();

      const VType*ctype = 0;
      Expression*cval = 0;
      if (arc->find_constant(name_, ctype, cval))
	    return ctype;

      cerr << get_fileline() << ": error: Signal/variable " << name_
	   << " not found in this context." << endl;
      return 0;
}

int ExpName::elaborate_expr(Entity*, Architecture*, const VType*ltype)
{
      ivl_assert(*this, ltype != 0);

      return 0;
}

const VType* ExpNameALL::probe_type(Entity*, Architecture*) const
{
      return 0;
}

const VType* ExpRelation::probe_type(Entity*ent, Architecture*arc) const
{
      const VType*type1 = peek_operand1()->probe_type(ent, arc);
      const VType*type2 = peek_operand2()->probe_type(ent, arc);

      if (type1 == type2)
	    return type1;

      if (type1 && !type2)
	    return type1;

      if (type2 && !type1)
	    return type2;

      const VTypeArray*type1a = dynamic_cast<const VTypeArray*>(type1);
      const VTypeArray*type2a = dynamic_cast<const VTypeArray*>(type2);

      if (type1a && type2a && type1a->element_type()==type2a->element_type()) {
	    return type1a->element_type();
      }

      cerr << get_fileline() << ": error: Type mismatch in relation expression." << endl;
      return type1;
}

int ExpRelation::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, arc);
      }

      ivl_assert(*this, ltype != 0);
      errors += elaborate_exprs(ent, arc, ltype);
      return errors;
}

int ExpString::elaborate_expr(Entity*, Architecture*, const VType*ltype)
{
      ivl_assert(*this, ltype != 0);
      set_type(ltype);
      return 0;
}
