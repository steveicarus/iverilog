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
# include  "vtype.h"
# include  "architec.h"
# include  <typeinfo>
# include  <iostream>
# include  <cstdlib>
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
      if (const VTypeArray*atype = dynamic_cast<const VTypeArray*> (peek_type()))
	    return emit_array_(out, ent, arc, atype);

      return Expression::emit(out, ent, arc);
}

int ExpAggregate::emit_array_(ostream&out, Entity*ent, Architecture*arc, const VTypeArray*atype)
{
      int errors = 0;

	// Special case: The aggregate is a single "others" item.
      if (aggregate_.size() == 1 && aggregate_[0].choice->others()) {
	    assert(atype->dimensions() == 1);

	    const VTypeArray::range_t&rang = atype->dimension(0);
	    assert(! rang.is_box());

	    int asize = abs(rang.msb() - rang.lsb()) + 1;

	    out << "{" << asize << "{";
	    errors += aggregate_[0].expr->emit(out, ent, arc);
	    out << "}}";
	    return errors;
      }

      const VTypeArray::range_t&rang = atype->dimension(0);
      assert(! rang.is_box());

      map<int64_t,choice_element*> element_map;
      choice_element*element_other = 0;

      for (size_t idx = 0 ; idx < aggregate_.size() ; idx += 1) {
	    if (aggregate_[idx].choice->others()) {
		  ivl_assert(*this, element_other == 0);
		  element_other = &aggregate_[idx];
		  continue;
	    }

	    Expression*tmp = aggregate_[idx].choice->simple_expression(false);
	    int64_t tmp_val;
	    if (! tmp->evaluate(arc, tmp_val)) {
		  cerr << tmp->get_fileline() << ": error: Unable to evaluate aggregate choice expression." << endl;
		  errors += 1;
		  continue;
	    }

	    element_map[tmp_val] = &aggregate_[idx];
      }

      ivl_assert(*this, rang.msb() >= rang.lsb());

      out << "{";
      for (int idx = rang.msb() ; idx >= rang.lsb() ; idx -= 1) {
	    choice_element*cur = element_map[idx];
	    if (cur == 0)
		  cur = element_other;
	    ivl_assert(*this, cur != 0);

	    if (idx < rang.msb())
		  out << ", ";
	    errors += cur->expr->emit(out, ent, arc);
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
      if (name_ == "length") {
	    int64_t val;
	    bool rc = evaluate(arc, val);
	    out << val;
	    if (rc)
		  return errors;
	    else
		  return errors + 1;
      }

      out << "$ivl_attribute(";
      errors += base_->emit(out, ent, arc);
      out << ", \"" << name_ << "\")";
      return errors;
}

int ExpArithmetic::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      if (fun_ == CONCAT)
	    return emit_concat_(out, ent, arc);

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
	  case CONCAT:
	    out << " /* ?concat? */ ";
	    break;
      }

      errors += emit_operand2(out, ent, arc);

      return errors;
}

int ExpArithmetic::emit_concat_(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      out << "{";
      errors += emit_operand1(out, ent, arc);
      out << ", ";
      errors += emit_operand2(out, ent, arc);
      out << "}";
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

int ExpConditional::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      out << "(";
      errors += cond_->emit(out, ent, arc);
      out << ")? (";

      if (true_clause_.size() > 1) {
	    cerr << get_fileline() << ": sorry: Multiple expressions not supported here." << endl;
	    errors += 1;
      }

      Expression*tmp = true_clause_.front();
      errors += tmp->emit(out, ent, arc);

      out << ") : (";

      if (else_clause_.size() > 1) {
	    cerr << get_fileline() << ": sorry: Multiple expressions not supported here." << endl;
	    errors += 1;
      }

      tmp = else_clause_.front();
      errors += tmp->emit(out, ent, arc);

      out << ")";

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

      } else {
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

bool ExpInteger::is_primary(void) const
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

int ExpName::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

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

      const VTypePrimitive*etype = dynamic_cast<const VTypePrimitive*> (arr->element_type());
      assert(etype);

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
