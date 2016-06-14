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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "expression.h"
# include  "parse_types.h"
# include  <iostream>
# include  <ivl_assert.h>

using namespace std;

void ExpAggregate::write_to_stream(ostream&fd) const
{
      fd << "(";
      for (vector<element_t*>::const_iterator cur = elements_.begin()
		 ; cur != elements_.end() ; ++cur) {
            if(cur != elements_.begin())
                fd << ", ";

	    (*cur)->write_to_stream(fd);
      }
      fd << ")";
}

void ExpAggregate::element_t::write_to_stream(ostream&fd) const
{
      for (vector<choice_t*>::const_iterator cur = fields_.begin()
		 ; cur != fields_.end() ; ++cur) {
	    (*cur)->write_to_stream(fd);
      }

      if(!fields_.empty())
            fd << "=>";
      val_->write_to_stream(fd);
}

void ExpAggregate::choice_t::write_to_stream(ostream&fd)
{
      if (others()) {
	    fd << "others";
	    return;
      }

      if (Expression*sim = simple_expression()) {
	    sim->write_to_stream(fd);
	    return;
      }

      if (ExpRange*rp = range_expressions()) {
	    rp->write_to_stream(fd);
	    return;
      }

      fd << "/* ERROR */";
}

void ExpArithmetic::write_to_stream(ostream&out) const
{
      out << "(";
      write_to_stream_operand1(out);
      out << ")";

      switch (fun_) {
	  case PLUS:
	    out << "+";
	    break;
	  case MINUS:
	    out << "-";
	    break;
	  case MULT:
	    out << "*";
	    break;
	  case DIV:
	    out << "/";
	    break;
	  case MOD:
	    out << "mod";
	    break;
	  case REM:
	    out << "rem";
	    break;
	  case POW:
	    out << "**";
	    break;
	  case xCONCAT:
	    ivl_assert(*this, 0);
	    break;
      }

      out << "(";
      write_to_stream_operand2(out);
      out << ")";
}

void ExpObjAttribute::write_to_stream(ostream&fd) const
{
      base_->write_to_stream(fd);
      fd << "'" << name_;
}

void ExpTypeAttribute::write_to_stream(ostream&fd) const
{
      base_->write_to_stream(fd);
      fd << "'" << name_;
}

void ExpBitstring::write_to_stream(ostream&fd) const
{
      fd << "B\"";
      for(vector<char>::const_reverse_iterator it = value_.rbegin();
        it != value_.rend(); ++it) {
          fd << *it;
      }
      fd << "\"";
}

void ExpCharacter::write_to_stream(ostream&fd) const
{
      char buf[4];
      buf[0] = '\'';
      buf[1] = value_;
      buf[2] = '\'';
      buf[3] = 0;
      fd << buf;
}

void ExpConcat::write_to_stream(ostream&fd) const
{
      fd << "(";
      operand1_->write_to_stream(fd);
      fd << ")&(";
      operand2_->write_to_stream(fd);
      fd << ")";
}

void ExpConditional::write_to_stream(ostream&) const
{
      ivl_assert(*this, !"Not supported");
}

void ExpEdge::write_to_stream(ostream&) const
{
      ivl_assert(*this, !"Not supported");
}

void ExpFunc::write_to_stream(ostream&fd) const
{
      const char*comma = "";
      fd << name_ << "(";
      for (vector<Expression*>::const_iterator cur = argv_.begin()
		 ; cur != argv_.end() ; ++cur) {
	    fd << comma;
	    (*cur)->write_to_stream(fd);
	    comma = ", ";
      }
      fd << ")";
}

void ExpInteger::write_to_stream(ostream&fd) const
{
      fd << value_;
}

void ExpReal::write_to_stream(ostream&fd) const
{
      fd << value_;
}

void ExpLogical::write_to_stream(ostream&out) const
{
      peek_operand1()->write_to_stream(out);

      switch (fun_) {
	  case AND:
	    out << " and ";
	    break;
	  case OR:
	    out << " or ";
	    break;
	  case XOR:
	    out << " xor ";
	    break;
	  case NAND:
	    out << " nand ";
	    break;
	  case NOR:
	    out << " nor ";
	    break;
	  case XNOR:
	    out << " xnor ";
	    break;
      }

      peek_operand2()->write_to_stream(out);
}

void ExpName::write_to_stream(ostream&fd) const
{
      if (prefix_.get()) {
	    prefix_->write_to_stream(fd);
	    fd << ".";
      }

      fd << name_;

      if (indices_) {
          fd << "(";
          bool first = true;
          for(list<Expression*>::const_iterator it = indices_->begin();
                  it != indices_->end(); ++it) {
              if(first)
                  first = false;
              else
                  fd << ",";

              (*it)->write_to_stream(fd);
          }
          fd << ")";
      }
}

void ExpRelation::write_to_stream(ostream&fd) const
{
      peek_operand1()->write_to_stream(fd);

      switch(fun_) {
        case EQ:
            fd << " = ";
            break;

        case LT:
            fd << " < ";
            break;

        case GT:
            fd << " > ";
            break;

        case NEQ:
            fd << " != ";
            break;

        case LE:
            fd << " <= ";
            break;

        case GE:
            fd << " >= ";
            break;
      }

      peek_operand2()->write_to_stream(fd);
}

void ExpShift::write_to_stream(ostream&out) const
{
      out << "(";
      write_to_stream_operand1(out);
      out << ")";

      switch (shift_) {
	  case SRL:
	    out << "srl";
	    break;
	  case SLL:
	    out << "sll";
	    break;
	  case SLA:
	    out << "sla";
	    break;
	  case SRA:
	    out << "sra";
	    break;
	  case ROR:
	    out << "ror";
	    break;
	  case ROL:
	    out << "rol";
	    break;
      }

      out << "(";
      write_to_stream_operand2(out);
      out << ")";
}


void ExpString::write_to_stream(ostream&fd) const
{
    fd << "\"";

    // Restore double quotation marks
    for(string::const_iterator it = value_.begin(); it != value_.end(); ++it) {
        if(*it == '"')
            fd << "\"\"";
        else
            fd << *it;
    }

    fd << "\"";
}

void ExpUAbs::write_to_stream(ostream&fd) const
{
      fd << "abs ";
      write_to_stream_operand1(fd);
}

void ExpUNot::write_to_stream(ostream&fd) const
{
      fd << "not ";
      write_to_stream_operand1(fd);
}

void ExpUMinus::write_to_stream(ostream&fd) const
{
      fd << "-(";
      write_to_stream_operand1(fd);
      fd << ")";
}

void ExpCast::write_to_stream(ostream&fd) const
{
      // Type casting is introduced only for a few specific cases in
      // SystemVerilog, so no need to use it here
      base_->write_to_stream(fd);
}

void ExpTime::write_to_stream(ostream&fd) const
{
      fd << amount_;

      switch(unit_) {
          case FS: fd << " fs"; break;
          case PS: fd << " ps"; break;
          case NS: fd << " ns"; break;
          case US: fd << " us"; break;
          case MS: fd << " ms"; break;
          case S:  fd << " s"; break;
      }
}

void ExpRange::write_to_stream(ostream&fd) const
{
      if(range_expr_) {
          range_base_->write_to_stream(fd);
          fd << (range_reverse_ ? "'reverse_range" : "'range");
      } else {
          left_->write_to_stream(fd);
          switch(direction_) {
              case DOWNTO: fd << " downto "; break;
              case TO: fd << " to "; break;
              default: ivl_assert(*this, false); break;
          }
          right_->write_to_stream(fd);
      }
}

void ExpDelay::write_to_stream(ostream&out) const
{
      expr_->write_to_stream(out);
      out << " after ";
      delay_->write_to_stream(out);
}
