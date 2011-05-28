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

# include "expression.h"
# include  <typeinfo>
# include  <iostream>

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

int ExpAttribute::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
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
      }

      errors += emit_operand2(out, ent, arc);

      return errors;
}

int ExpCharacter::emit(ostream&out, Entity*, Architecture*)
{
      out << "\"" << value_ << "\"";
      return 0;
}

bool ExpCharacter::is_primary(void) const
{
      return true;
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
	  default:
	    out << "INVALIDedge ";
	    errors += 1;
	    break;
      }
      errors += emit_operand1(out, ent, arc);
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

      out << name_;
      if (index_) {
	    out << "[";
	    errors += index_->emit(out, ent, arc);
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
