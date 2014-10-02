/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012-2013 / Stephen Williams (steve@icarus.com)
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
# include  "subprogram.h"
# include  "parse_types.h"
# include  <typeinfo>
# include  <iostream>
# include  <cstdlib>
# include  <cstring>
# include  "ivl_assert.h"
# include  <cassert>

using namespace std;

int Expression::emit(ostream&out, Entity*, Architecture*)
{
      out << " /* " << get_fileline() << ": internal error: "
	  << "I don't know how to emit this expression! "
	  << "type=" << typeid(*this).name() << " */ ";
      return 1;
}

int Expression::emit_package(ostream&out)
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

int ExpBinary::emit_operand1(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      bool oper_primary = operand1_->is_primary();
      if (! oper_primary) out << "(";
      errors += operand1_->emit(out, ent, arc);
      if (! oper_primary) out << ")";
      return errors;
}

int ExpBinary::emit_operand2(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      bool oper_primary = operand2_->is_primary();
      if (! oper_primary) out << "(";
      errors += operand2_->emit(out, ent, arc);
      if (! oper_primary) out << ")";
      return errors;
}

int ExpUnary::emit_operand1(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      errors += operand1_->emit(out, ent, arc);
      return errors;
}

int ExpAggregate::emit(ostream&out, Entity*ent, Architecture*arc)
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
	    return emit_array_(out, ent, arc, atype);
      else if (const VTypeRecord*arecord = dynamic_cast<const VTypeRecord*> (use_type))
	    return emit_record_(out, ent, arc, arecord);

      out << "/* " << get_fileline() << ": internal error: "
	  << "I don't know how to elab/emit aggregate in " << typeid(use_type).name()
	  << " type context. */";
      return 1;
}

int ExpAggregate::emit_array_(ostream&out, Entity*ent, Architecture*arc, const VTypeArray*atype)
{
      int errors = 0;

	// Special case: The aggregate is a single "others" item.
      if (aggregate_.size() == 1 && aggregate_[0].choice->others()) {
	    assert(atype->dimensions() == 1);

	    const VTypeArray::range_t&rang = atype->dimension(0);
	    assert(! rang.is_box());

	    int64_t use_msb;
	    int64_t use_lsb;
	    bool rc_msb, rc_lsb;
	    rc_msb = rang.msb()->evaluate(ent, arc, use_msb);
	    rc_lsb = rang.lsb()->evaluate(ent, arc, use_lsb);

	    if (rc_msb && rc_lsb) {
		  int asize = (use_msb >= use_lsb) ? (use_msb - use_lsb) + 1 :
		                                     (use_lsb - use_msb) + 1;
		  out << "{" << asize << "{";
		  errors += aggregate_[0].expr->emit(out, ent, arc);
		  out << "}}";
	    } else {
		  out << "{(";
		  if (rc_msb) {
			out << use_msb;
		  } else {
			out << "(";
			errors += rang.msb()->emit(out, ent, arc);
			out << ")";
		  }
		  if (rc_lsb && use_lsb==0) {
		  } else if (rc_lsb) {
			out << "-" << use_lsb;
		  } else {
			out << "-(";
			errors += rang.lsb()->emit(out, ent, arc);
			out << ")";
		  }
		  out << "+1){";
		  errors += aggregate_[0].expr->emit(out, ent, arc);
		  out << "}}";
	    }
	    return errors;
      }

      const VTypeArray::range_t&rang = atype->dimension(0);
      assert(! rang.is_box());

	// Fully calculate the range numbers.
      int64_t use_msb, use_lsb;
      bool rc;
      rc = rang.msb()->evaluate(ent, arc, use_msb);
      ivl_assert(*this, rc);
      rc = rang.lsb()->evaluate(ent, arc, use_lsb);
      ivl_assert(*this, rc);
      if(use_msb < use_lsb)
        swap(use_msb, use_lsb);

      map<int64_t,choice_element*> element_map;
      choice_element*element_other = 0;

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
	    if (prange_t*range = aggregate_[idx].choice->range_expressions()) {
		  int64_t begin_val, end_val;

		  if (! range->msb()->evaluate(ent, arc, begin_val)) {
			cerr << range->msb()->get_fileline() << ": error: "
			     << "Unable to evaluate aggregate choice expression." << endl;
			errors += 1;
			continue;
		  }

		  if (! range->lsb()->evaluate(ent, arc, end_val)) {
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
	    if (! tmp->evaluate(ent, arc, tmp_val)) {
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
	    choice_element*cur = element_map[idx];
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
		  errors += cur->expr->emit(out, ent, arc);
	    }
      }
      out << "}";

      return errors;
}

int ExpAggregate::emit_record_(ostream&out, Entity*ent, Architecture*arc, const VTypeRecord*)
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

	    //errors += name->emit(out, ent, arc);
	    //out << ": ";
	    errors += val->emit(out, ent, arc);
      }

      out << "}";

      return errors;
}

int ExpAttribute::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      if (name_ == "event") {
	    out << "$ivlh_attribute_event(";
	    errors += base_->emit(out, ent, arc);
	    out << ")";
	    return errors;
      }

	/* Special Case: The length attribute can be calculated all
	   the down to a literal integer at compile time, and all it
	   needs is the type of the base expression. (The base
	   expression doesn't even need to be evaluated.) */
      if (name_=="length") {
	    out << "$bits(";
	    errors += base_->emit(out, ent, arc);
	    out << ")";
	    return errors;
      }


      out << "$ivl_attribute(";
      errors += base_->emit(out, ent, arc);
      out << ", \"" << name_ << "\")";
      return errors;
}

int ExpArithmetic::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      errors += emit_operand1(out, ent, arc);

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
	  case REM:
	    out << " /* ?remainder? */ ";
	    break;
	  case xCONCAT:
	    ivl_assert(*this, 0);
	    out << " /* ?concat? */ ";
	    break;
      }

      errors += emit_operand2(out, ent, arc);

      return errors;
}

int ExpBitstring::emit(ostream&out, Entity*, Architecture*)
{
      int errors = 0;

      out << value_.size() << "'b";
      for (size_t idx = 0 ; idx < value_.size() ; idx += 1)
	    out << value_[value_.size()-idx-1];

      return errors;
}

int ExpCharacter::emit_primitive_bit_(ostream&out, Entity*, Architecture*,
				      const VTypePrimitive*etype)
{
      switch (etype->type()) {
	  case VTypePrimitive::BOOLEAN:
	  case VTypePrimitive::BIT:
	    switch (value_) {
		case '0':
		case '1':
		      out << "1'b" << value_;
		return 0;
		default:
		  break;
	    }
	    break;

	  case VTypePrimitive::STDLOGIC:
	    switch (value_) {
		case '0':
		case '1':
		      out << "1'b" << value_;
		return 0;
		default:
		  break;
	    }

	  default:
	    return 1;
      }
      return 1;
}

int ExpCharacter::emit(ostream&out, Entity*ent, Architecture*arc)
{
      const VType*etype = peek_type();

      if (const VTypePrimitive*use_type = dynamic_cast<const VTypePrimitive*>(etype)) {
	    return emit_primitive_bit_(out, ent, arc, use_type);
      }

      if (const VTypeArray*array = dynamic_cast<const VTypeArray*>(etype)) {
	    if (const VTypePrimitive*use_type = dynamic_cast<const VTypePrimitive*>(array->element_type())) {
		  return emit_primitive_bit_(out, ent, arc, use_type);
	    }
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

int ExpConcat::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      out << "{";
      errors += operand1_->emit(out, ent, arc);
      out << ", ";
      errors += operand2_->emit(out, ent, arc);
      out << "}";
      return errors;
}

int ExpConditional::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      out << "(";
      errors += cond_->emit(out, ent, arc);
      out << ")? (";

      if (true_clause_.size() > 1) {
	    cerr << get_fileline() << ": sorry: Multiple expression waveforms not supported here." << endl;
	    errors += 1;
      }

      Expression*tmp = true_clause_.front();
      errors += tmp->emit(out, ent, arc);

      out << ") : (";

	// Draw out any when-else expressions. These are all the else_
	// clauses besides the last.
      if (else_clause_.size() > 1) {
	    list<else_t*>::iterator last = else_clause_.end();
	    -- last;

	    for (list<else_t*>::iterator cur = else_clause_.begin()
		       ; cur != last ; ++cur) {
		  errors += (*cur) ->emit_when_else(out, ent, arc);
	    }
     }

      errors += else_clause_.back()->emit_else(out, ent, arc);
      out << ")";

	// The emit_when_else() functions do not close the last
	// parentheses so that the following expression can be
	// nested. But that means come the end, we have some
	// expressions to close.
      for (size_t idx = 1 ; idx < else_clause_.size() ; idx += 1)
	    out << ")";

      return errors;
}

int ExpConditional::else_t::emit_when_else(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      assert(cond_ != 0);

      out << "(";
      errors += cond_->emit(out, ent, arc);
      out << ")? (";

      if (true_clause_.size() > 1) {
	    cerr << get_fileline() << ": sorry: Multiple expression waveforms not supported here." << endl;
	    errors += 1;
      }

      Expression*tmp = true_clause_.front();
      errors += tmp->emit(out, ent, arc);

      out << ") : (";

      return errors;
}

int ExpConditional::else_t::emit_else(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
	// Trailing else must have no condition.
      assert(cond_ == 0);

      if (true_clause_.size() > 1) {
	    cerr << get_fileline() << ": sorry: Multiple expression waveforms not supported here." << endl;
	    errors += 1;
      }

      Expression*tmp = true_clause_.front();
      errors += tmp->emit(out, ent, arc);

      return errors;
}

int ExpEdge::emit(ostream&out, Entity*ent, Architecture*arc)
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
      errors += emit_operand1(out, ent, arc);
      return errors;
}

int ExpFunc::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      if (name_ == "unsigned" && argv_.size()==1) {
	      // Handle the special case that this is a cast to
	      // unsigned. This function is brought in as part of the
	      // std numeric library, but we interpret it as the same
	      // as the $unsigned function.
	    out << "$unsigned(";
	    errors += argv_[0]->emit(out, ent, arc);
	    out << ")";

      } else if (name_ == "std_logic_vector" && argv_.size() == 1) {
	      // Special case: The std_logic_vector function casts its
	      // argument to std_logic_vector. Internally, we don't
	      // have to do anything for that to work.
	    out << "(";
	    errors += argv_[0]->emit(out, ent, arc);
	    out << ")";

      } else if (name_ == "to_unsigned" && argv_.size() == 2) {

	    out << "$ivlh_to_unsigned(";
	    errors += argv_[0]->emit(out, ent, arc);
	    out << ", ";
	    errors += argv_[1]->emit(out, ent, arc);
	    out << ")";

      } else if (name_ == "conv_std_logic_vector" && argv_.size() == 2) {
	    int64_t use_size;
	    bool rc = argv_[1]->evaluate(ent, arc, use_size);
	    ivl_assert(*this, rc);
	    out << use_size << "'(";
	    errors += argv_[0]->emit(out, ent, arc);
	    out << ")";

      } else if (name_ == "rising_edge" && argv_.size()==1) {
	    out << "$ivlh_rising_edge(";
	    errors += argv_[0]->emit(out, ent, arc);
	    out << ")";

      } else if (name_ == "falling_edge" && argv_.size()==1) {
	    out << "$ivlh_falling_edge(";
	    errors += argv_[0]->emit(out, ent, arc);
	    out << ")";

      } else {
	      // If this function has an elaborated definition, and if
	      // that definition is in a package, then include the
	      // package name as a scope qualifier. This assures that
	      // the SV elaborator finds the correct VHDL elaborated
	      // definition.
	    if (def_) {
		  const Package*pkg = dynamic_cast<const Package*> (def_->get_parent());
		  if (pkg != 0)
			out << "\\" << pkg->name() << " ::";
	    }

	    out << "\\" << name_ << " (";
	    for (size_t idx = 0; idx < argv_.size() ; idx += 1) {
		  if (idx > 0) out << ", ";
		  errors += argv_[idx]->emit(out, ent, arc);
	    }
	    out << ")";
      }

      return errors;
}

int ExpInteger::emit(ostream&out, Entity*, Architecture*)
{
      out << value_;
      return 0;
}

int ExpInteger::emit_package(ostream&out)
{
      out << value_;
      return 0;
}

bool ExpInteger::is_primary(void) const
{
      return true;
}

int ExpReal::emit(ostream&out, Entity*, Architecture*)
{
      out << value_;
      return 0;
}

int ExpReal::emit_package(ostream&out)
{
      out << value_;
      return 0;
}

bool ExpReal::is_primary(void) const
{
      return true;
}

int ExpLogical::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      errors += emit_operand1(out, ent, arc);

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

      errors += emit_operand2(out, ent, arc);

      return errors;
}

int ExpName::emit_as_prefix_(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      if (prefix_.get()) {
	    errors += prefix_->emit_as_prefix_(out, ent, arc);
      }

      out << "\\" << name_ << " ";
      if (index_) {
	    out << "[";
	    errors += index_->emit(out, ent, arc);
	    out << "]";
	    ivl_assert(*this, lsb_ == 0);
      }
      out << ".";
      return errors;
}

int ExpName::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      if (prefix_.get()) {
	    errors += prefix_->emit_as_prefix_(out, ent, arc);
      }

      const GenerateStatement*gs = 0;
      if (arc && (gs = arc->probe_genvar_emit(name_)))
	    out << "\\" << gs->get_name() << ":" << name_ << " ";
      else
	    out << "\\" << name_ << " ";

      if (index_) {
	    out << "[";
	    errors += index_->emit(out, ent, arc);

	    if (lsb_) {
		  out << ":";
		  errors += lsb_->emit(out, ent, arc);
	    }
	    out << "]";
      }

      return errors;
}

bool ExpName::is_primary(void) const
{
      return true;
}

int ExpRelation::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      errors += emit_operand1(out, ent, arc);

      switch (fun_) {
	  case EQ:
	    out << " == ";
	    break;
	  case LT:
	    out << " < ";
	    break;
	  case GT:
	    out << " > ";
	    break;
	  case NEQ:
	    out << " != ";
	    break;
	  case LE:
	    out << " <= ";
	    break;
	  case GE:
	    out << " >= ";
	    break;
      }

      errors += emit_operand2(out, ent, arc);
      return errors;
}

bool ExpString::is_primary(void) const
{
      return true;
}

int ExpString::emit(ostream& out, Entity*ent, Architecture*arc)
{
      const VType*type = peek_type();
      assert(type != 0);

      if (const VTypeArray*arr = dynamic_cast<const VTypeArray*>(type)) {
	    return emit_as_array_(out, ent, arc, arr);
      }

      out << "\"";
      for(vector<char>::const_iterator it = value_.begin()
	  ; it != value_.end(); ++it)
	    out << *it;
      out << "\"";
      return 0;
}

int ExpString::emit_as_array_(ostream& out, Entity*, Architecture*, const VTypeArray*arr)
{
      int errors = 0;
      assert(arr->dimensions() == 1);

      const VTypePrimitive*etype = dynamic_cast<const VTypePrimitive*> (arr->basic_type());
      assert(etype);

	// Detect the special case that this is an array of
	// CHARACTER. In this case, emit at a Verilog string.
      if (etype->type()==VTypePrimitive::CHARACTER) {
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
	    switch (value_[idx]) {
		case '0':
		  out << "0";
		  break;
		case '1':
		  out << "1";
		  break;
		case 'z': case 'Z':
		  assert(etype->type() == VTypePrimitive::STDLOGIC);
		  out << "z";
		  break;
		default:
		  cerr << get_fileline() << ": internal error: "
		       << "Don't know how to handle bit " << value_[idx]
		       << " with etype==" << etype->type() << endl;
		  assert(etype->type() == VTypePrimitive::STDLOGIC);
		  out << "x";
		  break;
	    }
      }

      return errors;
}

int ExpUAbs::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      out << "abs(";
      errors += emit_operand1(out, ent, arc);
      out << ")";
      return errors;
}

int ExpUNot::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      out << "~(";
      errors += emit_operand1(out, ent, arc);
      out << ")";
      return errors;
}
