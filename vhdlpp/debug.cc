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
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */

# include  "entity.h"
# include  "architec.h"
# include  "expression.h"
# include  "vtype.h"
# include  <fstream>
# include  <iomanip>
# include  <typeinfo>

static ostream& operator << (ostream&out, port_mode_t that)
{
      switch (that) {
	  case PORT_NONE:
	    out << "NO-PORT";
	    break;
	  case PORT_IN:
	    out << "IN";
	    break;
	  case PORT_OUT:
	    out << "OUT";
	    break;
	  default:
	    out << "PORT-????";
	    break;
      }
      return out;
}

void dump_design_entities(const char*path)
{
      ofstream file (path);
      for (map<perm_string,Entity*>::iterator cur = design_entities.begin()
		 ; cur != design_entities.end() ; ++cur) {
	    cur->second->dump(file);
      }
}

void Entity::dump(ostream&out) const
{
      out << "entity " << name_
	  << " file=" << get_fileline() << endl;
      if (ports_.size() == 0) {
	    out << "    No ports" << endl;
      } else {
	    out << "    PORTS:" << endl;
	    for (vector<InterfacePort*>::const_iterator cur = ports_.begin()
		       ; cur != ports_.end() ; ++cur) {
		  InterfacePort*item = *cur;
		  out << setw(6) << "" << item->name
		      << " : " << item->mode
		      << ", type=";
		  if (item->type)
			item->type->show(out);
		  else
			out << "<nil>";
		  out << ", file=" << item->get_fileline() << endl;
	    }
      }

      for (map<perm_string,Architecture*>::const_iterator cur = arch_.begin()
		 ; cur != arch_.end() ; ++cur) {
	    cur->second->dump(out, name_);
      }
}

void Architecture::dump(ostream&out, perm_string of_entity) const
{
      out << "architecture " << name_
	  << " of entity " << of_entity
	  << " file=" << get_fileline() << endl;

      for (list<Architecture::Statement*>::const_iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {
	    (*cur)->dump(out);
      }
}

void Architecture::Statement::dump(ostream&out) const
{
      out << "   Architecutre::Statement at file=" << get_fileline() << endl;
}

void SignalAssignment::dump(ostream&out) const
{
      out << "   SignalAssignment file=" << get_fileline() << endl;
      out << "     " << target_name_ << " <= <expr>..." << endl;

      for (list<Expression*>::const_iterator cur = rval_.begin()
		 ; cur != rval_.end() ; ++cur) {
	    (*cur)->dump(out, 5);
      }
}

void Expression::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Expression [" << typeid(*this).name() << "]"
	  << " at " << get_fileline()<< endl;
}

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
      }

      out << setw(indent) << "" << "Arithmetic " << fun_name
	  << " at " << get_fileline() << endl;
      operand1()->dump(out, indent+4);
      operand2()->dump(out, indent+4);
}

void ExpInteger::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Integer" << endl;
}

void ExpLogical::dump(ostream&out, int indent) const
{
      const char*fun_name = "?";
      switch (fun_) {
	  case AND:
	    fun_name = "AND";
	    break;
	  case OR:
	    fun_name = "OR";
	    break;
	  case NAND:
	    fun_name = "NAND";
	    break;
	  case NOR:
	    fun_name = "NOR";
	    break;
	  case XOR:
	    fun_name = "XOR";
	    break;
	  case XNOR:
	    fun_name = "XNOR";
	    break;
      }

      out << setw(indent) << "" << "Logical " << fun_name
	  << " at " << get_fileline() << endl;
      operand1()->dump(out, indent+4);
      operand2()->dump(out, indent+4);
}

void ExpName::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "ExpName(\"" << name_ << "\")"
	  << " at " << get_fileline() << endl;
}
