/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012-2015 / Stephen Williams (steve@icarus.com)
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
# include  "vtype.h"
# include  "architec.h"
# include  "package.h"
# include  "std_funcs.h"
# include  "std_types.h"
# include  "parse_types.h"
# include  <typeinfo>
# include  <iostream>
# include  <cstdlib>
# include  <cstring>
# include  "ivl_assert.h"
# include  <cassert>

using namespace std;

inline static int emit_logic(char val, ostream& out, const VTypePrimitive::type_t type)
{
// TODO case 'W': case 'L': case 'H':

    switch (val) {
	case '-': case 'U':
	  val = 'x';
	  /* fall through */

	case 'X': case 'Z':
	  assert(type == VTypePrimitive::STDLOGIC);
	  /* fall through */

	case '0':
	case '1':
	  out << (char) tolower(val);
	  break;

	default:
	  assert(false);
	  out << "x";
          return 1;
    }

    return 0;
}

int Expression::emit(ostream&out, Entity*, ScopeBase*) const
{
      out << " /* " << get_fileline() << ": internal error: "
	  << "I don't know how to emit this expression! "
	  << "type=" << typeid(*this).name() << " */ ";
      return 1;
}

int Expression::emit_package(ostream&out) const
{
      out << " /* " << get_fileline() << ": internal error: "
	  << "I don't know how to emit_package this expression! "
	  << "type=" << typeid(*this).name() << " */ ";
      return 1;
}

bool Expression::is_primary(void) const
{
      return false;
}

int ExpBinary::emit_operand1(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      bool oper_primary = operand1_->is_primary();
      if (! oper_primary) out << "(";
      errors += operand1_->emit(out, ent, scope);
      if (! oper_primary) out << ")";
      return errors;
}

int ExpBinary::emit_operand2(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      bool oper_primary = operand2_->is_primary();
      if (! oper_primary) out << "(";
      errors += operand2_->emit(out, ent, scope);
      if (! oper_primary) out << ")";
      return errors;
}

int ExpUnary::emit_operand1(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      errors += operand1_->emit(out, ent, scope);
      return errors;
}

int ExpAggregate::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      if (peek_type() == 0) {
	    out << "/* " << get_fileline() << ": internal error: "
		<< "Aggregate literal needs well defined type." << endl;
	    return 1;
      }

      const VType*use_type = peek_type();
      while (const VTypeDef*def = dynamic_cast<const VTypeDef*> (use_type)) {
	    use_type = def->peek_definition();
      }

      if (const VTypeArray*atype = dynamic_cast<const VTypeArray*> (use_type))
	    return emit_array_(out, ent, scope, atype);
      else if (const VTypeRecord*arecord = dynamic_cast<const VTypeRecord*> (use_type))
	    return emit_record_(out, ent, scope, arecord);

      out << "/* " << get_fileline() << ": internal error: "
	  << "I don't know how to elab/emit aggregate in " << typeid(use_type).name()
	  << " type context. */";
      return 1;
}

int ExpAggregate::emit_array_(ostream&out, Entity*ent, ScopeBase*scope, const VTypeArray*atype) const
{
      int errors = 0;

	// Special case: The aggregate is a single "others" item.
      if (aggregate_.size() == 1 && aggregate_[0].choice->others()) {
	    assert(atype->dimensions().size() == 1);

	    const VTypeArray::range_t&rang = atype->dimension(0);
	    assert(! rang.is_box());

	    int64_t use_msb;
	    int64_t use_lsb;
	    bool rc_msb, rc_lsb;
	    rc_msb = rang.msb()->evaluate(ent, scope, use_msb);
	    rc_lsb = rang.lsb()->evaluate(ent, scope, use_lsb);

	    if (rc_msb && rc_lsb) {
		  int asize = (use_msb >= use_lsb) ? (use_msb - use_lsb) + 1 :
		                                     (use_lsb - use_msb) + 1;
		  out << "{" << asize << "{";
		  errors += aggregate_[0].expr->emit(out, ent, scope);
		  out << "}}";
	    } else {
		  out << "{(";
		  if (rc_msb) {
			out << use_msb;
		  } else {
			out << "(";
			errors += rang.msb()->emit(out, ent, scope);
			out << ")";
		  }
		  if (rc_lsb && use_lsb==0) {
		  } else if (rc_lsb) {
			out << "-" << use_lsb;
		  } else {
			out << "-(";
			errors += rang.lsb()->emit(out, ent, scope);
			out << ")";
		  }
		  out << "+1){";
		  errors += aggregate_[0].expr->emit(out, ent, scope);
		  out << "}}";
	    }
	    return errors;
      }

      const VTypeArray::range_t&rang = atype->dimension(0);
      assert(! rang.is_box());

	// Fully calculate the range numbers.
      int64_t use_msb, use_lsb;
      bool rc;
      rc = rang.msb()->evaluate(ent, scope, use_msb);
      ivl_assert(*this, rc);
      rc = rang.lsb()->evaluate(ent, scope, use_lsb);
      ivl_assert(*this, rc);
      if(use_msb < use_lsb)
        swap(use_msb, use_lsb);

      map<int64_t,const choice_element*> element_map;
      const choice_element*element_other = 0;

      bool positional_section = true;
      int64_t positional_idx = use_msb;

      for (size_t idx = 0 ; idx < aggregate_.size() ; idx += 1) {

	    if (aggregate_[idx].choice == 0) {
		    // positional association!
		  if (!positional_section) {
			cerr << get_fileline() << ": error: "
			     << "All positional associations must be before"
			     << " any named associations." << endl;
			errors += 1;
		  }
		  element_map[positional_idx] = &aggregate_[idx];
		  positional_idx -= 1;
		  continue;
	    }

	    if (aggregate_[idx].choice->others()) {
		  ivl_assert(*this, element_other == 0);
		  element_other = &aggregate_[idx];
		  continue;
	    }

	      // If this is a range choice, then calculate the bounds
	      // of the range and scan through the values, mapping the
	      // value to the aggregate_[idx] element.
	    if (ExpRange*range = aggregate_[idx].choice->range_expressions()) {
		  int64_t begin_val, end_val;

		  if (! range->msb()->evaluate(ent, scope, begin_val)) {
			cerr << range->msb()->get_fileline() << ": error: "
			     << "Unable to evaluate aggregate choice expression." << endl;
			errors += 1;
			continue;
		  }

		  if (! range->lsb()->evaluate(ent, scope, end_val)) {
			cerr << range->msb()->get_fileline() << ": error: "
			     << "Unable to evaluate aggregate choice expression." << endl;
			errors += 1;
			continue;
		  }

		  if (begin_val < end_val) {
			int64_t tmp = begin_val;
			begin_val = end_val;
			end_val = tmp;
		  }

		  while (begin_val >= end_val) {
			element_map[begin_val] = &aggregate_[idx];
			begin_val -= 1;
		  }

		  continue;
	    }

	    int64_t tmp_val;
	    Expression*tmp = aggregate_[idx].choice->simple_expression(false);
	    ivl_assert(*this, tmp);

	      // Named aggregate element. Once we see one of
	      // these, we can no longer accept positional
	      // elements so disable further positional
	      // processing.
	    positional_section = false;
	    if (! tmp->evaluate(ent, scope, tmp_val)) {
		  cerr << tmp->get_fileline() << ": error: "
		       << "Unable to evaluate aggregate choice expression." << endl;
		  errors += 1;
		  continue;
	    }

	    element_map[tmp_val] = &aggregate_[idx];
      }

	// Emit the elements as a concatenation. This works great for
	// vectors of bits. We implement VHDL arrays as packed arrays,
	// so this should be generally correct.
      // TODO uncomment this once ivl supports assignments of '{}
      /*if(!peek_type()->can_be_packed())
        out << "'";*/

      out << "{";
      for (int64_t idx = use_msb ; idx >= use_lsb ; idx -= 1) {
	    const choice_element*cur = element_map[idx];
	    if (cur == 0)
		  cur = element_other;

	    if (idx < use_msb)
		  out << ", ";
	    if (cur == 0) {
		  out << "/* Missing element " << idx << " */";
		  cerr << get_fileline() << ": error: "
		       << "Missing element " << idx << "." << endl;
		  errors += 1;
	    } else {
		  errors += cur->expr->emit(out, ent, scope);
	    }
      }
      out << "}";

      return errors;
}

int ExpAggregate::emit_record_(ostream&out, Entity*ent, ScopeBase*scope, const VTypeRecord*) const
{
      int errors = 0;

      out << "{";

      for (size_t idx = 0 ; idx < aggregate_.size() ; idx += 1) {
	    ivl_assert(*this, !aggregate_[idx].choice->others());
	    ivl_assert(*this, !aggregate_[idx].choice->range_expressions());

	    //Expression*name = aggregate_[idx].choice->simple_expression(false);
	    //ivl_assert(*this, name);
	    Expression*val = aggregate_[idx].expr;
	    ivl_assert(*this, val);

	    if(idx != 0)
	        out << ",";

	    //errors += name->emit(out, ent, scope);
	    //out << ": ";
	    errors += val->emit(out, ent, scope);
      }

      out << "}";

      return errors;
}

int ExpObjAttribute::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

	// Try to evaluate first
      int64_t val;
      if(evaluate(ent, scope, val)) {
            out << val;
            return 0;
      }

      if (name_ == "event") {
	    out << "$ivlh_attribute_event(";
	    errors += base_->emit(out, ent, scope);
	    out << ")";
	    return errors;
      }

	/* Special Case: The length,left & right attributes can be calculated
	   all the down to a literal integer at compile time, and all it
	   needs is the type of the base expression. (The base
	   expression doesn't even need to be evaluated.) */
      if (name_=="length") {
	    out << "$bits(";
	    errors += base_->emit(out, ent, scope);
	    out << ")";
	    return errors;
      } else if (name_=="left" || name_=="right") {
	    out << "$" << name_ << "(";
	    errors += base_->emit(out, ent, scope);
	    out << ")";
	    return errors;
      }

      // Fallback
      out << "$ivl_attribute(";
      errors += base_->emit(out, ent, scope);
      out << ", \"" << name_ << "\")";
      return errors;
}

int ExpTypeAttribute::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      // Special case: The image attribute
      if (name_=="image") {
            if(!args_ || args_->size() != 1) {
                out << "/* Invalid 'image attribute */" << endl;
                cerr << get_fileline() << ": error: 'image attribute takes "
                        << "exactly one argument." << endl;
                ++errors;
            } else {
                out << "$sformatf(\"";

                if(base_->type_match(&primitive_INTEGER))
                    out << "%0d";
                else if(base_->type_match(&primitive_REAL))
                    out << "%f";
                else if(base_->type_match(&primitive_CHARACTER))
                    out << "'%c'";
                else if(base_->type_match(&primitive_TIME))
                    out << "%+0t";

                out << "\",";
                args_->front()->emit(out, ent, scope);
                out << ")";
            }
            return errors;
      }

      // Fallback
      out << "$ivl_attribute(";
      errors += base_->emit_def(out, empty_perm_string);
      out << ", \"" << name_ << "\")";
      return errors;
}

int ExpArithmetic::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      if(fun_ == REM) {
          // Special case: division remainder, defined in the VHDL standard 1076-2008/9.2.7
          // there is no direct counterpart, therefore output the formula to
          // compute a remainder: A rem B = A - (A/B) * B;
          out << "((";
          errors += emit_operand1(out, ent, scope);
          out << ")-((";
          errors += emit_operand1(out, ent, scope);
          out << ")/(";
          errors += emit_operand2(out, ent, scope);
          out << "))*(";
          errors += emit_operand2(out, ent, scope);
          out << "))";
          return errors;
      }

      errors += emit_operand1(out, ent, scope);

      switch (fun_) {
	  case PLUS:
	    out << " + ";
	    break;
	  case MINUS:
	    out << " - ";
	    break;
	  case MULT:
	    out << " * ";
	    break;
	  case DIV:
	    out << " / ";
	    break;
	  case MOD:
	    out << " % ";
	    break;
	  case POW:
	    out << " ** ";
	    break;
	  case REM: // should not happen as it is handled above, suppress warnings
            ivl_assert(*this, 0);
	  case xCONCAT:
	    ivl_assert(*this, 0);
	    out << " /* ?concat? */ ";
	    break;
      }

      errors += emit_operand2(out, ent, scope);

      return errors;
}

int ExpBitstring::emit(ostream&out, Entity*, ScopeBase*) const
{
      int errors = 0;

      out << value_.size() << "'b";
      for (size_t idx = 0 ; idx < value_.size() ; idx += 1)
	    out << value_[value_.size()-idx-1];

      return errors;
}

int ExpCharacter::emit_primitive_bit_(ostream&out, Entity*, ScopeBase*,
				      const VTypePrimitive*etype) const
{
      out << "1'b";
      int res = emit_logic(value_, out, etype->type());

      if(res)
	  cerr << get_fileline() << ": internal error: "
	       << "Don't know how to handle bit " << value_
	       << " with etype==" << etype->type() << endl;

      return res;
}

int ExpCharacter::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      const VType*etype = peek_type();
      const VTypeArray*array;

      if (etype != &primitive_CHARACTER && (array = dynamic_cast<const VTypeArray*>(etype))) {
	    etype = array->element_type();
      }

      if (const VTypePrimitive*use_type = dynamic_cast<const VTypePrimitive*>(etype)) {
	    return emit_primitive_bit_(out, ent, scope, use_type);
      }

      out << "\"" << value_ << "\"";
      return 0;
}

bool ExpCharacter::is_primary(void) const
{
      return true;
}

/*
 * This is not exactly a "primary", but it is wrapped in its own
 * parentheses (braces) so we return true here.
 */
bool ExpConcat::is_primary(void) const
{
      return true;
}

int ExpConcat::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      out << "{";
      errors += operand1_->emit(out, ent, scope);
      out << ", ";
      errors += operand2_->emit(out, ent, scope);
      out << "}";
      return errors;
}

int ExpConditional::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      out << "(";

	// Draw out any when-else expressions. These are all the else_
	// clauses besides the last.
      if (options_.size() > 1) {
	    list<case_t*>::const_iterator last = options_.end();
	    --last;

	    for (list<case_t*>::const_iterator cur = options_.begin()
		       ; cur != last ; ++cur) {
		  errors += (*cur)->emit_option(out, ent, scope);
	    }
      }

      errors += options_.back()->emit_default(out, ent, scope);
      out << ")";

	// The emit_option() functions do not close the last
	// parentheses so that the following expression can be
	// nested. But that means come the end, we have some
	// expressions to close.
      for (size_t idx = 1 ; idx < options_.size() ; idx += 1)
	    out << ")";

      return errors;
}

int ExpConditional::case_t::emit_option(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      assert(cond_ != 0);

      out << "(";
      errors += cond_->emit(out, ent, scope);
      out << ")? (";

      if (true_clause_.size() > 1) {
	    cerr << get_fileline() << ": sorry: Multiple expression waveforms not supported here." << endl;
	    errors += 1;
      }

      Expression*tmp = true_clause_.front();
      errors += tmp->emit(out, ent, scope);

      out << ") : (";

      return errors;
}

int ExpConditional::case_t::emit_default(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
	// Trailing else must have no condition.
      assert(cond_ == 0);

      if (true_clause_.size() > 1) {
	    cerr << get_fileline() << ": sorry: Multiple expression waveforms not supported here." << endl;
	    errors += 1;
      }

      Expression*tmp = true_clause_.front();
      errors += tmp->emit(out, ent, scope);

      return errors;
}

int ExpEdge::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      switch (fun_) {
	  case NEGEDGE:
	    out << "negedge ";
	    break;
	  case POSEDGE:
	    out << "posedge ";
	    break;
	  case ANYEDGE:
	    break;
      }
      errors += emit_operand1(out, ent, scope);
      return errors;
}

int ExpFunc::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      if(!def_) {
          cerr << get_fileline() << ": error: unknown function: " << name_ << endl;
          return 1;
      }

      def_->emit_full_name(argv_, out, ent, scope);
      out << " (";
      def_->emit_args(argv_, out, ent, scope);
      out << ")";

      return errors;
}

int ExpInteger::emit(ostream&out, Entity*, ScopeBase*) const
{
      out << "32'd" << value_;
      return 0;
}

int ExpInteger::emit_package(ostream&out) const
{
      out << value_;
      return 0;
}

int ExpReal::emit(ostream&out, Entity*, ScopeBase*) const
{
      out << value_;
      return 0;
}

int ExpReal::emit_package(ostream&out) const
{
      out << value_;
      return 0;
}

bool ExpReal::is_primary(void) const
{
      return true;
}

int ExpLogical::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      errors += emit_operand1(out, ent, scope);

      switch (fun_) {
	  case AND:
	    out << " & ";
	    break;
	  case OR:
	    out << " | ";
	    break;
	  case XOR:
	    out << " ^ ";
	    break;
	  case NAND:
	    out << " ~& ";
	    break;
	  case NOR:
	    out << " ~| ";
	    break;
	  case XNOR:
	    out << " ~^ ";
	    break;
      }

      errors += emit_operand2(out, ent, scope);

      return errors;
}

int ExpName::emit_indices(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      if (indices_) {
          for(list<Expression*>::const_iterator it = indices_->begin();
                  it != indices_->end(); ++it) {
              out << "[";
              errors += (*it)->emit(out, ent, scope);
              out << "]";
          }
      }

      return errors;
}

int ExpName::emit_as_prefix_(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      if (prefix_.get()) {
	    errors += prefix_->emit_as_prefix_(out, ent, scope);
      }

      out << "\\" << name_ << " ";
      errors += emit_indices(out, ent, scope);
      out << ".";
      return errors;
}

int ExpName::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      int field_size = 0;
      list<index_t*> indices;

      if(try_workarounds_(out, ent, scope, indices, field_size)) {
            emit_workaround_(out, ent, scope, indices, field_size);
            for(list<index_t*>::iterator it = indices.begin();
                    it != indices.end(); ++it)
            {
                delete *it;
            }
            return 0;
      }

      if (prefix_.get()) {
	    errors += prefix_->emit_as_prefix_(out, ent, scope);
      }

      const GenerateStatement*gs = 0;
      Architecture*arc = dynamic_cast<Architecture*>(scope);
      if (arc && (gs = arc->probe_genvar_emit(name_)))
            out << "\\" << gs->get_name() << ":" << name_ << " ";
      else
	    out << "\\" << name_ << " ";

      errors += emit_indices(out, ent, scope);

      return errors;
}

bool ExpName::try_workarounds_(ostream&out, Entity*ent, ScopeBase*scope,
        list<index_t*>& indices, int& data_size) const
{
    Expression*exp = NULL;
    bool wrkand_required = false;
    const VType*type = NULL;
    Expression*idx = index(0);
    ExpRange*range = dynamic_cast<ExpRange*>(idx);

    if(!scope)
        return false;

    if(prefix_.get())
        prefix_->try_workarounds_(out, ent, scope, indices, data_size);

    if(idx && !range && scope->find_constant(name_, type, exp)) {
        while(const VTypeDef*type_def = dynamic_cast<const VTypeDef*>(type)) {
            type = type_def->peek_definition();
        }

        const VTypeArray*arr = dynamic_cast<const VTypeArray*>(type);
        assert(arr);
        wrkand_required |= check_const_array_workaround_(arr, scope, indices, data_size);
    }

    if(prefix_.get() && scope->find_constant(prefix_->name_, type, exp)) {
        // Handle the case of array of records
        if(prefix_->index(0)) {
            const VTypeArray*arr = dynamic_cast<const VTypeArray*>(type);
            assert(arr);
            type = arr->element_type();
            data_size = type->get_width(scope);
        }

        while(const VTypeDef*type_def = dynamic_cast<const VTypeDef*>(type)) {
            type = type_def->peek_definition();
        }

        const VTypeRecord*rec = dynamic_cast<const VTypeRecord*>(type);
        assert(rec);

        wrkand_required |= check_const_record_workaround_(rec, scope, indices, data_size);
    }

    // Workarounds are currently implemented only for one-dimensional arrays
    assert(!indices_ || indices_->size() == 1 || !wrkand_required);

    return wrkand_required;
}

bool ExpName::check_const_array_workaround_(const VTypeArray*arr,
        ScopeBase*scope, list<index_t*>&indices, int&data_size) const
{
    assert(indices_ && indices_->size() == 1);

    const VType*element = arr->element_type();
    data_size = element->get_width(scope);
    if(data_size < 0)
        return false;

    indices.push_back(new index_t(index(0)->clone(), new ExpInteger(data_size)));

    return true;
}

bool ExpName::check_const_record_workaround_(const VTypeRecord*rec,
        ScopeBase*scope, list<index_t*>&indices, int&data_size) const
{
    int tmp_offset = 0;
    const vector<VTypeRecord::element_t*>& elements = rec->get_elements();

    for(vector<VTypeRecord::element_t*>::const_reverse_iterator it = elements.rbegin();
            it != elements.rend(); ++it) {
        VTypeRecord::element_t* el = (*it);

        if(el->peek_name() == name_) {
            const VType*type = el->peek_type();

            int tmp_field = type->get_width(scope);
            if(tmp_field < 0)
                return false;

            data_size = tmp_field;
            indices.push_back(new index_t(NULL, NULL, new ExpInteger(tmp_offset)));

            if(index(0)) {
                const VTypeArray*arr = dynamic_cast<const VTypeArray*>(type);
                assert(arr);
                return check_const_array_workaround_(arr, scope, indices, data_size);
            }

            return true;
        }

        int w = el->peek_type()->get_width(scope);

        if(w < 0)
            return false;

        tmp_offset += w;
    }

    return false;
}

int ExpName::emit_workaround_(ostream&out, Entity*ent, ScopeBase*scope,
        const list<index_t*>& indices, int field_size) const
{
    int errors = 0;

    out << "\\" << (prefix_.get() ? prefix_->name_ : name_) << " [";

    for(list<index_t*>::const_iterator it = indices.begin();
            it != indices.end(); ++it) {
        errors += (*it)->emit(out, ent, scope);
        out << "+";
    }

    out << ":" << field_size << "]";

    return errors;
}

bool ExpName::is_primary(void) const
{
      return true;
}

int ExpRelation::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      errors += emit_operand1(out, ent, scope);

      const VType*type1 = peek_operand1()->probe_type(ent, scope);
      const VType*type2 = peek_operand2()->probe_type(ent, scope);
      bool logical_compare = false;

      // Apply case equality operator if any of the operands is of logic type
      if(((type1 && (type1->type_match(&primitive_STDLOGIC) ||
                     type1->type_match(&primitive_STDLOGIC_VECTOR)))
          || (type2 && (type2->type_match(&primitive_STDLOGIC) ||
                     type2->type_match(&primitive_STDLOGIC_VECTOR))))) {
        logical_compare = true;
      }

      switch (fun_) {
	  case EQ:
	    out << (logical_compare ? " === " : " == ");
	    break;
	  case LT:
	    out << " < ";
	    break;
	  case GT:
	    out << " > ";
	    break;
	  case NEQ:
	    out << (logical_compare ? " !== " : " != ");
	    break;
	  case LE:
	    out << " <= ";
	    break;
	  case GE:
	    out << " >= ";
	    break;
      }

      errors += emit_operand2(out, ent, scope);
      return errors;
}

int ExpShift::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      errors += emit_operand1(out, ent, scope);

      switch (shift_) {
	  case SRL:
	    out << " >> ";
	    break;
	  case SLL:
	    out << " << ";
	    break;
	  case SRA:
	    out << " >>> ";
	    break;
	  case SLA:
	    out << " <<< ";
	    break;
	  case ROR:
	  case ROL:
	    out << " /* ?ror/rol? */ ";
	    break;
      }

      errors += emit_operand2(out, ent, scope);

      return errors;
}

bool ExpString::is_primary(void) const
{
      return true;
}

int ExpString::emit(ostream& out, Entity*ent, ScopeBase*scope) const
{
      const VTypeArray*arr;
      const VType*type = peek_type();
      assert(type != 0);

      if (type != &primitive_STRING && (arr = dynamic_cast<const VTypeArray*>(type))) {
	    return emit_as_array_(out, ent, scope, arr);
      }

      out << "\"" << escape_quot(value_) << "\"";

      return 0;
}

int ExpString::emit_as_array_(ostream& out, Entity*, ScopeBase*, const VTypeArray*arr) const
{
      int errors = 0;
      assert(arr->dimensions().size() == 1);

      const VTypePrimitive*etype = dynamic_cast<const VTypePrimitive*> (arr->basic_type());
      assert(etype);

	// Detect the special case that this is an array of
	// CHARACTER. In this case, emit at a Verilog string.
      if (arr->element_type() == &primitive_CHARACTER) {
	    vector<char> tmp (value_.size() + 3);
	    tmp[0] = '"';
	    memcpy(&tmp[1], &value_[0], value_.size());
	    tmp[value_.size()+1] = '"';
	    tmp[value_.size()+2] = 0;
	    out << &tmp[0];
	    return errors;
      }

      assert(etype->type() != VTypePrimitive::INTEGER);
      out << value_.size() << "'b";
      for (size_t idx = 0 ; idx < value_.size() ; idx += 1) {
          int res = emit_logic(value_[idx], out, etype->type());
          errors += res;

          if(res)
              cerr << get_fileline() << ": internal error: "
                  << "Don't know how to handle bit " << value_[idx]
                  << " with etype==" << etype->type() << endl;
      }

      return errors;
}

std::string ExpString::escape_quot(const std::string& str)
{
      size_t idx = 0;
      string result(str);

      while((idx = result.find('"', idx)) != string::npos) {
         result.replace(idx, 1, "\\\"");
         idx += 2;
      }

      return result;
}

int ExpUAbs::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      out << "abs(";
      errors += emit_operand1(out, ent, scope);
      out << ")";
      return errors;
}

int ExpUNot::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      const VType*op_type = peek_operand()->probe_type(ent, scope);

      if(op_type && op_type->type_match(&type_BOOLEAN))
          out << "!(";
      else
          out << "~(";

      errors += emit_operand1(out, ent, scope);
      out << ")";
      return errors;
}

int ExpUMinus::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      out << "-(";
      errors += emit_operand1(out, ent, scope);
      out << ")";
      return errors;
}

int ExpCast::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      errors += type_->emit_def(out, empty_perm_string);
      out << "'(";
      errors += base_->emit(out, ent, scope);
      out << ")";
      return errors;
}

int ExpNew::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;
      out << "new[";
      errors += size_->emit(out, ent, scope);
      out << "]";
      return errors;
}

int ExpTime::emit(ostream&out, Entity*, ScopeBase*) const
{
      out << amount_;

      switch(unit_) {
          case FS: out << "fs"; break;
          case PS: out << "ps"; break;
          case NS: out << "ns"; break;
          case US: out << "us"; break;
          case MS: out << "ms"; break;
          case S:  out << "s"; break;
      }

      return 0;
}

int ExpRange::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      if(range_expr_) {
          out << "$left(";
          errors += range_base_->emit(out, ent, scope);
          out << "):$right(";
          errors += range_base_->emit(out, ent, scope);
          out << ")";
      } else if(direction_ == AUTO) {
          ivl_assert(*this, false);
          out << "/* auto dir */";
      } else {
          errors += left_->emit(out, ent, scope);
          out << ":";
          errors += right_->emit(out, ent, scope);
      }

      return errors;
}

int ExpDelay::emit(ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      out << "#(";
      errors += delay_->emit(out, ent, scope);
      out << ") ";
      errors += expr_->emit(out, ent, scope);

      return errors;
}
