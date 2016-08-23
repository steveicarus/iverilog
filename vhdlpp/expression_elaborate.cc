/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012-2013 / Stephen Williams (steve@icarus.com)
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
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */

# include  "expression.h"
# include  "architec.h"
# include  "entity.h"
# include  "vsignal.h"
# include  "subprogram.h"
# include  "std_types.h"
# include  <iostream>
# include  <typeinfo>
# include  "parse_types.h"
# include  "compiler.h"
# include  "ivl_assert.h"

using namespace std;

int Expression::elaborate_lval(Entity*, ScopeBase*, bool)
{
      cerr << get_fileline() << ": error: Expression is not a valid l-value." << endl;
      return 1;
}

const VType* Expression::probe_type(Entity*, ScopeBase*) const
{
      return 0;
}

const VType* Expression::fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*) const
{
      const VType*res = probe_type(ent,scope);
      if (res == 0) {
	    cerr << get_fileline() << ": internal error: "
		 << "fit_type for " << typeid(*this).name()
		 << " is not implemented." << endl;
      }

      return res;
}

const VType*ExpName::elaborate_adjust_type_with_range_(Entity*ent, ScopeBase*scope,
                                                       const VType*type)
{
	// Unfold typedefs
      while (const VTypeDef*tdef = dynamic_cast<const VTypeDef*>(type)) {
	    type = tdef->peek_definition();
      }

      if (const VTypeArray*array = dynamic_cast<const VTypeArray*>(type)) {
	    Expression*idx = index(0);

	    if (ExpRange*range = dynamic_cast<ExpRange*>(idx)) {
		    // If the name is an array, then a part select is
		    // also an array, but with different bounds.
		  int64_t use_msb, use_lsb;
		  bool flag = true;

		  flag &= range->msb()->evaluate(ent, scope, use_msb);
		  flag &= range->lsb()->evaluate(ent, scope, use_lsb);

                  if(flag)
                    type = new VTypeArray(array->element_type(), use_msb, use_lsb);
	    }
	    else if(idx) {
		    // If the name is an array or a vector, then an
		    // indexed name has the type of the element.
		  type = array->element_type();
	    }
      }

      return type;
}

int ExpName::elaborate_lval_(Entity*ent, ScopeBase*scope, bool is_sequ, ExpName*suffix)
{
      int errors = 0;

      if (debug_elaboration) {
	    debug_log_file << get_fileline() << ": ExpName::elaborate_lval_: "
			   << "name_=" << name_
			   << ", suffix->name()=" << suffix->name();
	    if (indices_) {
		for(list<Expression*>::const_iterator it = indices_->begin();
		        it != indices_->end(); ++it) {
		    debug_log_file << "[";
		    debug_log_file << **it;
		    debug_log_file << "]";
		}
	    }
	    debug_log_file << endl;
      }

      if (prefix_.get()) {
	    cerr << get_fileline() << ": sorry: I don't know how to elaborate "
		 << "ExpName prefix of " << name_
		 << " in l-value expressions." << endl;
	    errors += 1;
      }

      const VType*found_type = 0;

      if (const InterfacePort*cur = ent->find_port(name_)) {
	    if (cur->mode != PORT_OUT && cur->mode != PORT_INOUT) {
		  cerr << get_fileline() << ": error: Assignment to "
			"input port " << name_ << "." << endl;
		  return errors + 1;
	    }

	    if (is_sequ)
		  ent->set_declaration_l_value(name_, is_sequ);

	    found_type = cur->type;

      } else if (ent->find_generic(name_)) {

	    cerr << get_fileline() << ": error: Assignment to generic "
		 << name_ << " from entity "
		 << ent->get_name() << "." << endl;
	    return errors + 1;

      } else if (Signal*sig = scope->find_signal(name_)) {
	      // Tell the target signal that this may be a sequential l-value.
	    if (is_sequ) sig->count_ref_sequ();

	    found_type = sig->peek_type();

      } else if (Variable*var = scope->find_variable(name_)) {
	      // Tell the target signal that this may be a sequential l-value.
	    if (is_sequ) var->count_ref_sequ();

	    found_type = var->peek_type();
      }

	// Resolve type definition to get an actual type.
      while (const VTypeDef*tdef = dynamic_cast<const VTypeDef*> (found_type)) {
	    found_type = tdef->peek_definition();

	    if (debug_elaboration) {
		  debug_log_file << get_fileline() << ": ExpName::elaborate_lval_: "
				 << "Resolve typedef " << tdef->peek_name()
				 << " to defined type=" << typeid(*found_type).name()
				 << endl;
	    }
      }

      ivl_assert(*this, found_type);

	// If the prefix type is an array, then we may actually have a
	// case of an array of structs. For example:
	//   foo(n).bar
	// where foo is an array, (n) is an array index and foo(n) is
	// something that takes a suffix. For the purpose of our
	// expression type calculations, we need the element type.
      if (const VTypeArray*array = dynamic_cast<const VTypeArray*> (found_type)) {
	    found_type = array->element_type();

	    while (const VTypeDef*tdef = dynamic_cast<const VTypeDef*> (found_type)) {
		  found_type = tdef->peek_definition();
	    }

	    if (debug_elaboration) {
		  debug_log_file << get_fileline() << ": ExpName::elaborate_lval_: "
				 << "Extract array element type=" << typeid(*found_type).name()
				 << endl;
	    }
      }

      const VType*suffix_type = 0;

      if (const VTypeRecord*record = dynamic_cast<const VTypeRecord*> (found_type)) {
	    const VTypeRecord::element_t*element = record->element_by_name(suffix->name_);
	    ivl_assert(*this, element);

	    const VType*element_type = element->peek_type();
	    ivl_assert(*this, element_type);

	    suffix_type = element_type;

      }

      if (suffix_type == 0) {
	    cerr << get_fileline() << ": error: I don't know how to handle prefix " << name_
		 << " with suffix " << suffix->name_ << endl;
	    errors += 1;
	    return errors;
      }

      suffix_type = suffix->elaborate_adjust_type_with_range_(ent, scope, suffix_type);

      ivl_assert(*this, suffix_type);
      suffix->set_type(suffix_type);

      return errors;
}

int ExpName::elaborate_lval(Entity*ent, ScopeBase*scope, bool is_sequ)
{
      int errors = 0;

      if (prefix_.get()) {
	    return prefix_->elaborate_lval_(ent, scope, is_sequ, this);
      }

      const VType*found_type = 0;

      if (ent) {
          if (const InterfacePort*cur = ent->find_port(name_)) {
                  if (cur->mode != PORT_OUT && cur->mode != PORT_INOUT) {
                      cerr << get_fileline() << ": error: Assignment to "
                              "input port " << name_ << "." << endl;
                      return errors += 1;
                  }

                  if (is_sequ)
                      ent->set_declaration_l_value(name_, is_sequ);

                  found_type = cur->type;

          } else if (ent->find_generic(name_)) {

                  cerr << get_fileline() << ": error: Assignment to generic "
                      << name_ << " from entity "
                      << ent->get_name() << "." << endl;
                  return 1;
          }
      }

      if (!found_type && scope) {
        if (Signal*sig = scope->find_signal(name_)) {
            // Tell the target signal that this may be a sequential l-value.
            if (is_sequ) sig->count_ref_sequ();

            found_type = sig->peek_type();

        } else if (Variable*var = scope->find_variable(name_)) {
                // Tell the target signal that this may be a sequential l-value.
                if (is_sequ) var->count_ref_sequ();

                found_type = var->peek_type();

        } else if (const InterfacePort*port = scope->find_param(name_)) {
                found_type = port->type;
        }
      }

      if (found_type == 0) {
	    cerr << get_fileline() << ": error: Signal/variable " << name_
		 << " not found in this context." << endl;
	    return errors + 1;
      }

      found_type = elaborate_adjust_type_with_range_(ent, scope, found_type);

      set_type(found_type);
      return errors;
}

int ExpName::elaborate_rval(Entity*ent, ScopeBase*scope, const InterfacePort*lval)
{
      int errors = 0;

      if (prefix_.get()) {
	    cerr << get_fileline() << ": sorry: I don't know how to elaborate "
		 << "ExpName prefix parts in r-value expressions." << endl;
	    errors += 1;
      }

      const VType*dummy_type;
      Expression*dummy_expr;

      if (const InterfacePort*cur = ent->find_port(name_)) {
        /* IEEE 1076-2008, p.80:
        * For a formal port IN, associated port should be IN, OUT, INOUT or BUFFER
        * For a formal port OUT, associated port should be OUT, INOUT or BUFFER
        * For a formal port INOUT, associated port should be OUT, INOUT or BUFFER
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
      } else if (scope->find_signal(name_)) {
	      /* OK */

      } else if (ent->find_generic(name_)) {
	      /* OK */

      } else if (scope->find_constant(name_, dummy_type, dummy_expr)) {
	      /* OK */

      } else if (scope->is_enum_name(name_)) {
	      /* OK */

      } else {
            cerr << get_fileline() << ": error: No port, signal or constant " << name_
		 << " to be used as r-value." << endl;
            errors += 1;
      }

      return errors;
}

int Expression::elaborate_expr(Entity*, ScopeBase*, const VType*)
{
      cerr << get_fileline() << ": internal error: I don't know how to "
           << "elaborate expression type=" << typeid(*this).name() << endl;
      return 1;
}

const VType* ExpBinary::probe_type(Entity*ent, ScopeBase*scope) const
{
      const VType*t1 = operand1_->probe_type(ent, scope);
      const VType*t2 = operand2_->probe_type(ent, scope);

      if (t1 == 0)
	    return t2;
      if (t2 == 0)
	    return t1;

      if (t1->type_match(t2))
	    return t1;
      if (t2->type_match(t1))
	    return t2;

      if (const VType*tb = resolve_operand_types_(t1, t2))
	    return tb;

	// FIXME: I should at this point try harder to find an
	// operator that has the proper argument list and use this
	// here, but for now we leave it for the back-end to figure out.
#if 0
      cerr << get_fileline() << ": internal error: I don't know how to resolve types of generic binary expressions." << endl;
#endif
      return 0;
}

const VType*ExpBinary::resolve_operand_types_(const VType*, const VType*) const
{
      return 0;
}

int ExpBinary::elaborate_exprs(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      errors += operand1_->elaborate_expr(ent, scope, ltype);
      errors += operand2_->elaborate_expr(ent, scope, ltype);
      return errors;
}

/*
 * the default fit_type method for unary operator expressions is to
 * return the fit_type for the operand. The assumption is that the
 * operator doesn't change the type.
 */
const VType*ExpUnary::fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const
{
      return operand1_->fit_type(ent, scope, atype);
}

const VType*ExpUnary::probe_type(Entity*ent, ScopeBase*scope) const
{
      return operand1_->probe_type(ent, scope);
}

int ExpUnary::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      ivl_assert(*this, ltype != 0);
      set_type(ltype);
      return operand1_->elaborate_expr(ent, scope, ltype);
}

const VType*ExpAggregate::fit_type(Entity*, ScopeBase*, const VTypeArray*host) const
{
      ivl_assert(*this, elements_.size() == 1);
      size_t choice_count = elements_[0]->count_choices();

      ivl_assert(*this, choice_count > 0);
      vector<choice_element> ce (choice_count);
      elements_[0]->map_choices(&ce[0]);

      ivl_assert(*this, ce.size() == 1);
      ExpRange*prange = ce[0].choice->range_expressions();
      ivl_assert(*this, prange);

      Expression*use_msb = prange->msb();
      Expression*use_lsb = prange->lsb();

      ivl_assert(*this, host->dimensions().size() == 1);
      vector<VTypeArray::range_t> range (1);

      range[0] = VTypeArray::range_t(use_msb, use_lsb);

      const VTypeArray*res = new VTypeArray(host->element_type(), range);

      return res;
}

int ExpAggregate::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      if (ltype == 0) {
	    cerr << get_fileline() << ": error: Elaboration of aggregate types needs well known type context?" << endl;
	    return 1;
      }

      set_type(ltype);

      while (const VTypeDef*cur = dynamic_cast<const VTypeDef*>(ltype)) {
	    ltype = cur->peek_definition();
      }

      if (const VTypeArray*larray = dynamic_cast<const VTypeArray*>(ltype)) {
	    return elaborate_expr_array_(ent, scope, larray);
      }
      else if(const VTypeRecord*lrecord = dynamic_cast<const VTypeRecord*>(ltype)) {
            return elaborate_expr_record_(ent, scope, lrecord);
      }

      cerr << get_fileline() << ": internal error: I don't know how to elaborate aggregate expressions. type=" << typeid(*ltype).name() << endl;
      return 1;
}

/*
 * Elaboration of array aggregates is elaboration of the element
 * expressions (the elements_ member) using the element type as the
 * ltype for the subexpression.
 */
int ExpAggregate::elaborate_expr_array_(Entity*ent, ScopeBase*scope, const VTypeArray*ltype)
{
      const VType*element_type = ltype->element_type();
      int errors = 0;
      size_t choice_count = 0;

	// Figure out how many total elements we have here. Note that
	// each parsed element may be bound to multiple choices, so
	// account for that.
      for (size_t edx = 0 ; edx < elements_.size() ; edx += 1) {
	    element_t*ecur = elements_[edx];
	    if (ecur->count_choices() == 0)
		  choice_count += 1;
	    else
		  choice_count += ecur->count_choices();
      }

      aggregate_.resize(choice_count);

	// Translate the elements_ array to the aggregate_ array. In
	// the target array, each expression is attached to a single
	// choice.
      size_t cdx = 0;
      for (size_t edx = 0 ; edx < elements_.size() ; edx += 1) {
	    element_t*ecur = elements_[edx];
	    if (ecur->count_choices() == 0) {
		    // positional associations have no "choice"
		    // associated with them.
		  aggregate_[cdx].choice = 0;
		  aggregate_[cdx].expr = ecur->extract_expression();
		  aggregate_[cdx].alias_flag = false;
		  cdx += 1;
	    } else {
		  ecur->map_choices(&aggregate_[cdx]);
		  cdx += ecur->count_choices();
	    }
      }

      ivl_assert(*this, cdx == choice_count);

	// Now run through the more convenient mapping and elaborate
	// all the expressions that I find.
      for (size_t idx = 0 ; idx < aggregate_.size() ; idx += 1) {
	    if (aggregate_[idx].alias_flag)
		  continue;

	    errors += aggregate_[idx].expr->elaborate_expr(ent, scope, element_type);
      }

	// done with the obsolete elements_ vector.
      elements_.clear();

      return errors;
}

int ExpAggregate::elaborate_expr_record_(Entity*ent, ScopeBase*scope, const VTypeRecord*ltype)
{
      int errors = 0;

      aggregate_.resize(elements_.size());
      choice_element tmp;
      int idx;

	// Translate the elements_ array to the aggregate_ array. In
	// the target array, each expression is attached to a single
	// choice.
      for (size_t edx = 0 ; edx < elements_.size() ; edx += 1) {
            element_t*ecur = elements_[edx];

            // it is invalid to have more than one choice in record assignment
            ivl_assert(*this, ecur->count_choices() == 1);

            ecur->map_choices(&tmp);
            choice_t*ch = tmp.choice;

            ivl_assert(*this, !ch->others());
            ivl_assert(*this, !tmp.alias_flag);

	// Get the appropriate type for a field
            const ExpName*field = dynamic_cast<const ExpName*>(ch->simple_expression(false));
            ivl_assert(*this, field);

            perm_string field_name = field->peek_name();
            idx = -1;
            const VTypeRecord::element_t*el = ltype->element_by_name(field_name, &idx);
            ivl_assert(*this, idx >= 0);

            aggregate_[idx] = tmp;
            errors += aggregate_[idx].expr->elaborate_expr(ent, scope, el->peek_type());
      }

	// done with the obsolete elements_ vector.
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

int ExpArithmetic::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, scope);
      }

      ivl_assert(*this, ltype != 0);
      errors += elaborate_exprs(ent, scope, ltype);
      return errors;
}

const VType* ExpArithmetic::resolve_operand_types_(const VType*t1, const VType*t2) const
{
    // Ranges
      while (const VTypeRange*tmp = dynamic_cast<const VTypeRange*> (t1))
	    t1 = tmp->base_type();
      while (const VTypeRange*tmp = dynamic_cast<const VTypeRange*> (t2))
	    t2 = tmp->base_type();

      if (t1->type_match(t2))
	    return t1;

    // Signed & unsigned (resized to the widest argument)
    const VTypeArray*t1_arr = dynamic_cast<const VTypeArray*>(t1);
    const VTypeArray*t2_arr = dynamic_cast<const VTypeArray*>(t2);

    if(t1_arr && t2_arr) {
        const VTypeArray*t1_parent = t1_arr->get_parent_type();
        const VTypeArray*t2_parent = t2_arr->get_parent_type();

        if(t1_parent == t2_parent
                && (t1_parent == &primitive_SIGNED || t1_parent == &primitive_UNSIGNED)) {
            int t1_size = t1_arr->get_width(NULL);
            int t2_size = t2_arr->get_width(NULL);

            // Easy, the same sizes, so we do not need to resize
            if(t1_size == t2_size && t1_size > 0)
                return t1;  // == t2

            VTypeArray*resolved = new VTypeArray(t1_parent->element_type(),
                    std::max(t1_size, t2_size) - 1, 0, t1_parent->signed_vector());
            resolved->set_parent_type(t1_parent);

            return resolved;
        }

    } else if(t1_arr) {
        if(const VTypePrimitive*prim = dynamic_cast<const VTypePrimitive*>(t2)) {
            const VTypeArray*t1_parent = t1_arr->get_parent_type();
            VTypePrimitive::type_t t2_type = prim->type();

            if((t2_type == VTypePrimitive::NATURAL || t2_type == VTypePrimitive::INTEGER)
                    && t1_parent == &primitive_SIGNED)
                return t1;

            if((t2_type == VTypePrimitive::NATURAL) && t1_parent == &primitive_UNSIGNED)
                return t1;
        }

    } else if(t2_arr) {
        if(const VTypePrimitive*prim = dynamic_cast<const VTypePrimitive*>(t1)) {
            const VTypeArray*t2_parent = t2_arr->get_parent_type();
            VTypePrimitive::type_t t1_type = prim->type();

            if((t1_type == VTypePrimitive::NATURAL || t1_type == VTypePrimitive::INTEGER)
                    && t2_parent == &primitive_SIGNED)
                return t2;

            if((t1_type == VTypePrimitive::NATURAL) && t2_parent == &primitive_UNSIGNED)
                return t2;
        }
    }

    return 0;
}

int ExpAttribute::elaborate_args(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if(args_) {
	    for(list<Expression*>::iterator it = args_->begin();
                    it != args_->end(); ++it) {
		errors += (*it)->elaborate_expr(ent, scope, ltype);
            }
      }

      return errors;
}

int ExpObjAttribute::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*)
{
      int errors = 0;
      const VType*sub_type = base_->probe_type(ent, scope);

      errors += elaborate_args(ent, scope, sub_type);
      errors += base_->elaborate_expr(ent, scope, sub_type);

      return errors;
}

const VType* ExpObjAttribute::probe_type(Entity*, ScopeBase*) const
{
      if (name_ == "length" || name_ == "left" || name_ == "right")
	    return &primitive_NATURAL;

      return NULL;
}

int ExpTypeAttribute::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      return elaborate_args(ent, scope, ltype);
}

const VType* ExpTypeAttribute::probe_type(Entity*, ScopeBase*) const
{
      if(name_ == "image")
	    return &primitive_STRING;

      return NULL;
}

const VType*ExpBitstring::fit_type(Entity*, ScopeBase*, const VTypeArray*atype) const
{
	// Really should check that this string can work with the
	// array element type?
      return atype->element_type();
}

int ExpBitstring::elaborate_expr(Entity*, ScopeBase*, const VType*)
{
      int errors = 0;
      const VTypeArray*type = new VTypeArray(&primitive_STDLOGIC, value_.size() - 1, 0);
      set_type(type);
      return errors;
}

const VType*ExpCharacter::fit_type(Entity*, ScopeBase*, const VTypeArray*atype) const
{
	// Really should check that this character can work with the
	// array element type?
      return atype->element_type();
}

int ExpCharacter::elaborate_expr(Entity*, ScopeBase*, const VType*ltype)
{
      ivl_assert(*this, ltype != 0);
      set_type(ltype);
      return 0;
}

const VType*ExpConcat::fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const
{
      Expression*operands[2] = {operand1_, operand2_};
      const VType*types[2] = {NULL, NULL};
      Expression*sizes[2] = {NULL, NULL};

      // determine the type and size of concatenated expressions
      for(int i = 0; i < 2; ++i) {
	    types[i] = operands[i]->fit_type(ent, scope, atype);

	    if(const VTypeArray*arr = dynamic_cast<const VTypeArray*>(types[i])) {
		types[i] = arr->element_type();
		ivl_assert(*this, arr->dimensions().size() == 1);
		const VTypeArray::range_t&dim = arr->dimension(0);
		sizes[i] = new ExpArithmetic(ExpArithmetic::MINUS, dim.msb(), dim.lsb());
	    } else {
		sizes[i] = new ExpInteger(0);
	    }
      }

      // the range of the concatenated expression is (size1 + size2 + 1):0
      // note that each of the sizes are already decreased by one,
      // e.g. 3:0 <=> size == 3 even though there are 4 bits
      Expression*size = new ExpArithmetic(ExpArithmetic::PLUS,
                            new ExpArithmetic(ExpArithmetic::PLUS, sizes[0], sizes[1]),
                            new ExpInteger(1));

      std::list<ExpRange*> ranges;
      ranges.push_front(new ExpRange(size, new ExpInteger(0), ExpRange::DOWNTO));
      const VType*array = new VTypeArray(types[1], &ranges);

      return array;
}
/*
 * I don't know how to probe the type of a concatenation, quite yet.
 */
const VType*ExpConcat::probe_type(Entity*, ScopeBase*) const
{
      ivl_assert(*this, 0);
      return 0;
}

int ExpConcat::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, scope);
      }

      ivl_assert(*this, ltype != 0);

      if (const VTypeArray*atype = dynamic_cast<const VTypeArray*>(ltype)) {
	    errors += elaborate_expr_array_(ent, scope, atype);
      } else {
	    errors += operand1_->elaborate_expr(ent, scope, ltype);
	    errors += operand2_->elaborate_expr(ent, scope, ltype);
      }

      return errors;
}

int ExpConcat::elaborate_expr_array_(Entity*ent, ScopeBase*scope, const VTypeArray*atype)
{
      int errors = 0;

	// For now, only support single-dimension arrays here.
      ivl_assert(*this, atype->dimensions().size() == 1);

      const VType*type1 = operand1_->fit_type(ent, scope, atype);
      ivl_assert(*this, type1);

      const VType*type2 = operand2_->fit_type(ent, scope, atype);
      ivl_assert(*this, type2);

      errors += operand1_->elaborate_expr(ent, scope, type1);
      errors += operand2_->elaborate_expr(ent, scope, type2);

      return errors;
}

const VType* ExpConditional::probe_type(Entity*, ScopeBase*) const
{
      return 0;
}

int ExpConditional::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0)
	    ltype = probe_type(ent, scope);

      ivl_assert(*this, ltype);

      set_type(ltype);

	/* Note that the type for the condition expression need not
	   have anything to do with the type of this expression. */

      for (list<case_t*>::const_iterator cur = options_.begin()
		 ; cur != options_.end() ; ++cur) {
	    errors += (*cur)->elaborate_expr(ent, scope, ltype);
      }

      return errors;
}

int ExpConditional::case_t::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if (cond_)
	    errors += cond_->elaborate_expr(ent, scope, 0);

      for (list<Expression*>::const_iterator cur = true_clause_.begin()
		 ; cur != true_clause_.end() ; ++cur) {
	    errors += (*cur)->elaborate_expr(ent, scope, ltype);
      }

      return errors;
}

const VType*ExpFunc::probe_type(Entity*ent, ScopeBase*scope) const
{
      if(!def_)
          def_ = match_signature(ent, scope);

      return def_ ? def_->exact_return_type(argv_, ent, scope) : NULL;
}

int ExpFunc::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*)
{
      int errors = 0;

      if(def_)
          return 0;

      def_ = match_signature(ent, scope);

      if(!def_)
          return 1;

	// Elaborate arguments
      for (size_t idx = 0; idx < argv_.size(); ++idx) {
	    errors += def_->elaborate_argument(argv_[idx], idx, ent, scope);
      }

	// SystemVerilog functions work only with defined size data types, therefore
	// if header does not specify argument or return type size, create a function
	// instance that work with this particular size.
      if(def_ && !def_->is_std() && def_->unbounded()) {
            def_ = def_->make_instance(argv_, scope);
            name_ = def_->name();   // TODO necessary?
      }

      return errors;
}

const VType* ExpFunc::fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*) const
{
    return probe_type(ent, scope);
}

const VType* ExpInteger::probe_type(Entity*, ScopeBase*) const
{
      if(value_ >= 0)
          return &primitive_NATURAL;
      else
          return &primitive_INTEGER;
}

int ExpInteger::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, scope);
      }

      ivl_assert(*this, ltype != 0);

      return errors;
}

const VType* ExpReal::probe_type(Entity*, ScopeBase*) const
{
      return &primitive_REAL;
}

int ExpReal::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
        ltype = probe_type(ent, scope);
      }

      ivl_assert(*this, ltype != 0);

      return errors;
}

int ExpLogical::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, scope);
      }

      ivl_assert(*this, ltype != 0);
      errors += elaborate_exprs(ent, scope, ltype);
      return errors;
}

const VType* ExpName::probe_prefix_type_(Entity*ent, ScopeBase*scope) const
{
      if (prefix_.get()) {
	    cerr << get_fileline() << ": sorry: I do not know how to support nested prefix parts." << endl;
	    return 0;
      }

      const VType*type = probe_type(ent, scope);
      return type;
}

/*
 * This method is the probe_type() implementation for ExpName objects
 * that have prefix parts. In this case we try to get the type of the
 * prefix and interpret the name in that context.
 */
const VType* ExpName::probe_prefixed_type_(Entity*ent, ScopeBase*scope) const
{
      // First, get the type of the prefix.
      const VType*prefix_type = prefix_->probe_prefix_type_(ent, scope);
      if (prefix_type == 0) {
          return 0;
      }

      while (const VTypeDef*def = dynamic_cast<const VTypeDef*> (prefix_type)) {
          prefix_type = def->peek_definition();
      }

      const VType*element_type = prefix_type;
      bool type_changed = true;

      // Keep unwinding the type until we find the basic element type
      while (type_changed) {
          type_changed = false;

          // If the prefix type is a record, then the current name is
          // the name of a member.
          if (const VTypeRecord*pref_record = dynamic_cast<const VTypeRecord*>(element_type)) {
              const VTypeRecord::element_t*element = pref_record->element_by_name(name_);
              ivl_assert(*this, element);

              element_type = element->peek_type();
              ivl_assert(*this, element_type);
              type_changed = true;
          }

          if (const VTypeArray*pref_array = dynamic_cast<const VTypeArray*>(element_type)) {
              element_type = pref_array->basic_type(false);
              ivl_assert(*this, element_type);
              type_changed = true;
          }
      }

      if(!element_type) {
          cerr << get_fileline() << ": sorry: I don't know how to probe "
              << "prefix type " << typeid(*prefix_type).name()
              << " of " << name_ << "." << endl;
          return NULL;
      }

      return element_type;
}

const VType* ExpName::probe_type(Entity*ent, ScopeBase*scope) const
{
      if (prefix_.get())
	    return probe_prefixed_type_(ent, scope);

      if(ent) {
        if (const InterfacePort*cur = ent->find_port(name_)) {
            ivl_assert(*this, cur->type);
            return cur->type;
        }

        if (const InterfacePort*cur = ent->find_generic(name_)) {
            ivl_assert(*this, cur->type);
            return cur->type;
        }
      }

      if(scope) {
        if (Signal*sig = scope->find_signal(name_))
            return sig->peek_type();

        if (Variable*var = scope->find_variable(name_))
            return var->peek_type();

        const VType*type = 0;
        Expression*cval = 0;
        if (scope->find_constant(name_, type, cval))
            return type;

        Architecture*arc = dynamic_cast<Architecture*>(scope);
        if (arc && (type = arc->probe_genvar_type(name_))) {
            return type;
        }

        if (const InterfacePort*port = scope->find_param(name_)) {
            return port->type;
        }

        if ((type = scope->is_enum_name(name_))) {
            return type;
        }
      }

      if(ent || scope) {
          // Do not display error messages if there was no entity or scope
          // specified. There are functions that are called without any specific
          // context and they still may want to probe the expression type.
        cerr << get_fileline() << ": error: Signal/variable " << name_
             << " not found in this context." << endl;
      }

      return 0;
}

const VType* ExpName::fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*)const
{
      return probe_type(ent, scope);
}

int ExpName::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      if (ltype) {
	    ivl_assert(*this, ltype != 0);
	    set_type(ltype);
      }

      if(prefix_.get())
	    prefix_.get()->elaborate_expr(ent, scope, NULL);

      if (indices_) {
          for(list<Expression*>::const_iterator it = indices_->begin();
                  it != indices_->end(); ++it) {
              (*it)->elaborate_expr(ent, scope, &primitive_INTEGER);
          }
      }

      return 0;
}

const VType* ExpNameALL::probe_type(Entity*, ScopeBase*) const
{
      return 0;
}

const VType* ExpRelation::probe_type(Entity*, ScopeBase*) const
{
      return &type_BOOLEAN;
}

int ExpRelation::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, scope);
      }

      ivl_assert(*this, ltype != 0);

	// The type of the operands must match, but need not match the
	// type for the ExpRelation itself. So get the operand type
	// separately.
      const VType*otype = ExpBinary::probe_type(ent, scope);
      errors += elaborate_exprs(ent, scope, otype);

      return errors;
}

int ExpShift::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, scope);
      }

      ivl_assert(*this, ltype != 0);
      errors += elaborate_exprs(ent, scope, ltype);
      return errors;
}

/*
 * When a string appears in a concatenation, then the type of the
 * string is an array with the same element type of the concatenation,
 * but with elements for each character of the string.
 */
const VType*ExpString::fit_type(Entity*, ScopeBase*, const VTypeArray*atype) const
{
      vector<VTypeArray::range_t> range (atype->dimensions());

	// Generate an array range for this string
      ivl_assert(*this, range.size() == 1);

      VTypeArray*type = new VTypeArray(atype->element_type(), value_.size(), 0);
      return type;
}

int ExpString::elaborate_expr(Entity*, ScopeBase*, const VType*ltype)
{
      ivl_assert(*this, ltype != 0);
      set_type(ltype);
      return 0;
}

int ExpTime::elaborate_expr(Entity*, ScopeBase*, const VType*)
{
      set_type(&primitive_INTEGER);
      return 0;
}

int ExpRange::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*)
{
    int errors = 0;

    if(left_)
        errors += left_->elaborate_expr(ent, scope, &primitive_INTEGER);

    if(right_)
        errors += right_->elaborate_expr(ent, scope, &primitive_INTEGER);

    return errors;
}

int ExpDelay::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
    int errors = 0;

    errors += expr_->elaborate_expr(ent, scope, ltype);
    errors += delay_->elaborate_expr(ent, scope, ltype);

    return errors;
}
