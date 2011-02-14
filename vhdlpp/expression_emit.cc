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

int ExpInteger::emit(ostream&out, Entity*, Architecture*)
{
      out << " /* " << get_fileline() << ": internal error: "
	  << "INTEGER LITERAL */ ";
      return 1;
}

int ExpLogical::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      errors += operand1_->emit(out, ent, arc);

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

      errors += operand2_->emit(out, ent, arc);

      return errors;
}

int ExpName::emit(ostream&out, Entity*, Architecture*)
{
      int errors = 0;

      out << name_;
      return errors;
}
