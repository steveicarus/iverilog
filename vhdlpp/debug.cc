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
# include  "parse_types.h"
# include  "vsignal.h"
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

void dump_design_entities(ostream&file)
{
      for (map<perm_string,Entity*>::iterator cur = design_entities.begin()
		 ; cur != design_entities.end() ; ++cur) {
	    cur->second->dump(file);
      }
}

void ComponentBase::dump_ports(ostream&out, int indent) const
{
      if (ports_.empty()) {
	    out << setw(indent) << "" << "No ports" << endl;
      } else {
	    out << setw(indent) << "" << "PORTS:" << endl;
	    for (vector<InterfacePort*>::const_iterator cur = ports_.begin()
		       ; cur != ports_.end() ; ++cur) {
		  InterfacePort*item = *cur;
		  out << setw(indent+2) << "" << item->name
		      << " : " << item->mode
		      << ", type=";
		  if (item->type)
			item->type->show(out);
		  else
			out << "<nil>";
		  out << ", file=" << item->get_fileline() << endl;
	    }
      }
}

void Scope::dump_scope(ostream&out) const
{
	// Dump types
      for (map<perm_string,const VType*>::const_iterator cur = old_types_.begin()
		 ; cur != old_types_.end() ; ++cur) {
	    out << "   " << cur->first << ": ";
	    cur->second->show(out);
	    out << endl;
      }
      for (map<perm_string,const VType*>::const_iterator cur = new_types_.begin()
         ; cur != new_types_.end() ; ++cur) {
        out << "   " << cur->first << ": ";
        cur->second->show(out);
        out << endl;
      }

	// Dump constants
      for (map<perm_string,const_t*>::const_iterator cur = old_constants_.begin()
		 ; cur != old_constants_.end() ; ++cur) {
	    out << "   constant " << cur->first << " = ";
	    out << endl;
      }
      for (map<perm_string,const_t*>::const_iterator cur = new_constants_.begin()
         ; cur != new_constants_.end() ; ++cur) {
        out << "   constant " << cur->first << " = ";
        out << endl;
      }
	// Dump signal declarations
      for (map<perm_string,Signal*>::const_iterator cur = old_signals_.begin()
         ; cur != old_signals_.end() ; ++cur) {
        if (cur->second)
          cur->second->dump(out, 3);
        else
          out << "   signal " << cur->first.str() << ": ???" << endl;
      }
      for (map<perm_string,Signal*>::const_iterator cur = new_signals_.begin()
         ; cur != new_signals_.end() ; ++cur) {
        if (cur->second)
          cur->second->dump(out, 3);
        else
          out << "   signal " << cur->first.str() << ": ???" << endl;
      }
	// Dump component declarations
      for (map<perm_string,ComponentBase*>::const_iterator cur = old_components_.begin()
		 ; cur != old_components_.end() ; ++cur) {
	    out << "   component " << cur->first << " is" << endl;
	    cur->second->dump_ports(out);
	    out << "   end component " << cur->first << endl;
      }
      for (map<perm_string,ComponentBase*>::const_iterator cur = new_components_.begin()
         ; cur != new_components_.end() ; ++cur) {
        out << "   component " << cur->first << " is" << endl;
        cur->second->dump_ports(out);
        out << "   end component " << cur->first << endl;
      }
}

void Entity::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "entity " << get_name()
	  << " file=" << get_fileline() << endl;
      dump_ports(out, indent+2);

      for (map<perm_string,Architecture*>::const_iterator cur = arch_.begin()
		 ; cur != arch_.end() ; ++cur) {
	    cur->second->dump(out, get_name(), indent);
      }
}

void Signal::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "signal " << name_ << " is ";
      if (type_)
	    out << *type_;
      else
	    out << "?NO TYPE?";
      out << endl;
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
	  case CONCAT:
	    fun_name = "&";
	    break;
      }

      out << setw(indent) << "" << "Arithmetic " << fun_name
	  << " at " << get_fileline() << endl;
      dump_operands(out, indent+4);
}

void ExpAttribute::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Attribute " << name_
	  << " at " << get_fileline() << endl;
      base_->dump(out, indent+4);
}

void ExpBinary::dump_operands(ostream&out, int indent) const
{
      operand1_->dump(out, indent);
      operand2_->dump(out, indent);
}

void ExpBitstring::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Bit string " << value_.size()
	  << "b\"";
      for (size_t idx = value_.size() ; idx > 0 ; idx -= 1) {
	    out << value_[idx-1];
      }
      out << "\"" << endl;
}

void ExpCharacter::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Character '" << value_ << "'"
	  << " at " << get_fileline() << endl;
}

void ExpConditional::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Conditional expression at "<< get_fileline() << endl;
      out << setw(indent) << "" << "  when:" << endl;
      cond_->dump(out, indent+4);

      out << setw(indent) << "" << "  do:" << endl;
      for (list<Expression*>::const_iterator cur = true_clause_.begin()
		 ; cur != true_clause_.end() ; ++cur) {
	    (*cur)->dump(out, indent+4);
      }

      out << setw(indent) << "" << "  else:" << endl;
      for (list<Expression*>::const_iterator cur = else_clause_.begin()
		 ; cur != else_clause_.end() ; ++cur) {
	    (*cur)->dump(out, indent+4);
      }
}

void ExpEdge::dump(ostream&out, int indent) const
{
      out << setw(indent) << "";
      switch (fun_) {
	  case NEGEDGE:
	    out << "negedge ";
	    break;
	  case ANYEDGE:
	    out << "ANYedge ";
	    break;
	  case POSEDGE:
	    out << "posedge ";
      }
      out << "at " << get_fileline() << endl;
      dump_operand1(out, indent+3);
}

void ExpInteger::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Integer " << value_
	  << " at " << get_fileline() << endl;
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
      dump_operands(out, indent+4);
}

void ExpName::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "ExpName(\"" << name_ << "\")"
	  << " at " << get_fileline() << endl;
      if (index_)
	    index_->dump(out, indent+6);
}

void ExpNameALL::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "ExpNameALL at " << get_fileline() << endl;
}

void ExpRelation::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Relation ";
      switch (fun_) {
	  case EQ:
	    out << "=";
	    break;
	  case LT:
	    out << "<";
	    break;
	  case GT:
	    out << ">";
	    break;
	  case NEQ:
	    out << "/=";
	    break;
	  case LE:
	    out << "<=";
	    break;
	  case GE:
	    out << ">=";
	    break;
      }
      out << endl;
      dump_operands(out, indent+4);
}

void ExpString::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "String \"";
    for(vector<char>::const_iterator it = value_.begin();
        it != value_.end(); ++it)
        out << *it;
    out << "\""
    << " at " << get_fileline() << endl;
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

void named_expr_t::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << name_ << "=>";
    expr_->dump(out, indent);
}

void range_t::dump(ostream&out, int indent) const
{
    left_->dump(out, indent);
    out << setw(indent) << "" << (direction_ ? "downto" : "to");
    right_->dump(out, indent);
}
