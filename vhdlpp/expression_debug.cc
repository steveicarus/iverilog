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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
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
