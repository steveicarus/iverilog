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
# include  <iostream>
# include  <ivl_assert.h>

using namespace std;

void ExpAggregate::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpArithmetic::write_to_stream(ostream&out)
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

void ExpAttribute::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpBitstring::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpCharacter::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpConcat::write_to_stream(ostream&fd)
{
      fd << "(";
      operand1_->write_to_stream(fd);
      fd << ")&(";
      operand2_->write_to_stream(fd);
      fd << ")";
}

void ExpConditional::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpEdge::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpFunc::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpInteger::write_to_stream(ostream&fd)
{
      fd << value_;
}

void ExpLogical::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpName::write_to_stream(ostream&fd)
{
      if (prefix_.get()) {
	    prefix_->write_to_stream(fd);
	    fd << ".";
      }

      fd << name_;
      if (index_) {
	    fd << "(";
	    index_->write_to_stream(fd);
	    if (lsb_) {
		  fd << " downto ";
		  lsb_->write_to_stream(fd);
	    }
	    fd << ")";
      }
}

void ExpRelation::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpString::write_to_stream(ostream&)
{
      ivl_assert(*this, !"Not supported");
}

void ExpUAbs::write_to_stream(ostream&fd)
{
      fd << "abs ";
      write_to_stream_operand1(fd);
}

void ExpUNot::write_to_stream(ostream&fd)
{
      fd << "not ";
      write_to_stream_operand1(fd);
}

