/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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

# include  "entity.h"
# include  "architec.h"
# include  "expression.h"
# include  <ivl_assert.h>
# include  <fstream>
# include  <iomanip>
# include  <typeinfo>

using namespace std;

void ExpArithmetic::dump(ostream&out, int indent) const
{
      const char*fun_name = "?";
      switch (fun_) {
	  case PLUS:
	    fun_name = "+";
	    break;
	  case MINUS:
	    fun_name = "-";
	    break;
	  case MULT:
	    fun_name = "*";
	    break;
	  case DIV:
	    fun_name = "/";
	    break;
	  case MOD:
	    fun_name = "mod";
	    break;
	  case REM:
	    fun_name = "rem";
	    break;
	  case POW:
	    fun_name = "**";
	    break;
	  case xCONCAT:
	    ivl_assert(*this, 0);
	    break;
      }

      out << setw(indent) << "" << "Arithmetic " << fun_name
	  << " at " << get_fileline() << endl;
      dump_operands(out, indent+4);
}

void ExpConcat::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Concatenation at " << get_fileline() << endl;
      operand1_->dump(out, indent);
      operand2_->dump(out, indent);
}

void ExpCast::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Casting ";
      base_->dump(out, indent+4);
      out << " to ";
      type_->emit_def(out, empty_perm_string);
}

void ExpNew::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "New dynamic array size: " << endl;
      size_->dump(out, indent);
}

void ExpScopedName::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "Scoped name expression: " << endl;
    out << "    scope " << scope_name_ << " " << scope_ << endl;
    name_->dump(out, indent+4);
}

void ExpShift::dump(ostream&out, int indent) const
{
      const char*fun_name = "?";
      switch (shift_) {
	  case SRL:
	    fun_name = "srl";
	    break;
	  case SLL:
	    fun_name = "sll";
	    break;
	  case SLA:
	    fun_name = "sla";
	    break;
	  case SRA:
	    fun_name = "sra";
	    break;
	  case ROR:
	    fun_name = "ror";
	    break;
	  case ROL:
	    fun_name = "rol";
	    break;
      }

      out << setw(indent) << "" << "Shift " << fun_name
	  << " at " << get_fileline() << endl;
      dump_operands(out, indent+4);
}

void ExpString::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "String \"" << value_;
    out << "\"" << " at " << get_fileline() << endl;
}

void ExpUAbs::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "abs() at " << get_fileline() << endl;
      dump_operand1(out, indent+4);
}

void ExpUnary::dump_operand1(ostream&out, int indent) const
{
      operand1_->dump(out, indent);
}

void ExpUNot::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "not() at " << get_fileline() << endl;
      dump_operand1(out, indent+4);
}

void ExpUMinus::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "unary_minus() at " << get_fileline() << endl;
      dump_operand1(out, indent+4);
}

void ExpTime::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Time ";
      write_to_stream(out);
}

void ExpRange::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Range ";
      write_to_stream(out);
}

void ExpDelay::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Expression ";
      expr_->write_to_stream(out);
      out << " delayed by ";
      delay_->write_to_stream(out);
}
