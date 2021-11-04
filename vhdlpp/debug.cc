/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2014 CERN
 * @author Maciej Suminski (maciej.suminski@cern.ch)
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
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */

# include  "entity.h"
# include  "architec.h"
# include  "expression.h"
# include  "parse_types.h"
# include  "subprogram.h"
# include  "sequential.h"
# include  "vsignal.h"
# include  "vtype.h"
# include  <fstream>
# include  <iomanip>
# include  <typeinfo>

using namespace std;

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

void ComponentBase::dump_generics(ostream&out, int indent) const
{
      if (parms_.empty()) {
	    out << setw(indent) << "" << "No generics" << endl;
      } else {
	    out << setw(indent) << "" << "GENERICS:" << endl;
	    for (vector<InterfacePort*>::const_iterator cur = parms_.begin()
		       ; cur != parms_.end() ; ++cur) {
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

void ScopeBase::dump_scope(ostream&out) const
{
	// Dump types
      out << "   -- imported types" << endl;
      for (map<perm_string,const VType*>::const_iterator cur = use_types_.begin()
		 ; cur != use_types_.end() ; ++cur) {
	    out << "   " << cur->first << ": ";
	    cur->second->show(out);
	    out << endl;
      }
      out << "   -- Types from this scope" << endl;
      for (map<perm_string,const VType*>::const_iterator cur = cur_types_.begin()
         ; cur != cur_types_.end() ; ++cur) {
        out << "   " << cur->first << ": ";
        cur->second->show(out);
        out << endl;
      }

	// Dump constants
      out << "   -- imported constants" << endl;
      for (map<perm_string,const_t*>::const_iterator cur = use_constants_.begin()
		 ; cur != use_constants_.end() ; ++cur) {
	    out << "   constant " << cur->first << " = ";
	    out << endl;
      }
      out << "   -- Constants from this scope" << endl;
      for (map<perm_string,const_t*>::const_iterator cur = cur_constants_.begin()
         ; cur != cur_constants_.end() ; ++cur) {
        out << "   constant " << cur->first << " = ";
        out << endl;
      }
	// Dump signal declarations
      out << "   -- Signals" << endl;
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
	// Dump subprograms
      out << "   -- Imported Subprograms" << endl;
      for (map<perm_string,SubHeaderList>::const_iterator cur = use_subprograms_.begin()
		 ; cur != use_subprograms_.end() ; ++cur) {
	    const SubHeaderList& subp_list = cur->second;

	    for(SubHeaderList::const_iterator it = subp_list.begin();
			it != subp_list.end(); ++it) {
                const SubprogramHeader*subp = *it;
                out << "   subprogram " << cur->first << " is" << endl;
                subp->dump(out);
                if(subp->body())
                    subp->body()->dump(out);
                out << "   end subprogram " << cur->first << endl;
            }
      }

      out << "   -- Subprograms from this scope" << endl;
      for (map<perm_string,SubHeaderList>::const_iterator cur = cur_subprograms_.begin()
		 ; cur != cur_subprograms_.end() ; ++cur) {
	    const SubHeaderList& subp_list = cur->second;

	    for(SubHeaderList::const_iterator it = subp_list.begin();
			it != subp_list.end(); ++it) {
                const SubprogramHeader*subp = *it;
                out << "   subprogram " << cur->first << " is" << endl;
                subp->dump(out);
                if(subp->body())
                    subp->body()->dump(out);
                out << "   end subprogram " << cur->first << endl;
            }
      }
	// Dump component declarations
      out << "   -- Components" << endl;
      for (map<perm_string,ComponentBase*>::const_iterator cur = old_components_.begin()
		 ; cur != old_components_.end() ; ++cur) {
	    out << "   component " << cur->first << " is" << endl;
	    cur->second->dump_generics(out);
	    cur->second->dump_ports(out);
	    out << "   end component " << cur->first << endl;
      }
      for (map<perm_string,ComponentBase*>::const_iterator cur = new_components_.begin()
		 ; cur != new_components_.end() ; ++cur) {
	    out << "   component " << cur->first << " is" << endl;
	    cur->second->dump_generics(out);
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

void SigVarBase::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "signal/variable " << name_ << " is ";
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

void ExpAggregate::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Aggregate expression at " << get_fileline() << endl;

      for (size_t idx = 0 ; idx < elements_.size() ; idx += 1)
	    elements_[idx]->dump(out, indent+2);
}

void ExpAggregate::element_t::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "choices:" << endl;
      for (size_t idx = 0 ; idx < fields_.size() ; idx += 1)
	    fields_[idx]->dump(out, indent+4);

      out << setw(indent) << "" << "expression:" << endl;
      val_->dump(out, indent+4);
}

void ExpAggregate::choice_t::dump(ostream&out, int indent) const
{
      if (others()) {
	    out << setw(indent) << "" << "=> others" << endl;
	    return;
      }

      if (expr_.get()) {
	    expr_->dump(out, indent);
	    return;
      }

      if (range_.get()) {
	    range_->dump(out, indent);
	    return;
      }

      out << setw(indent) << "" << "?choice_t?" << endl;
}

void ExpTypeAttribute::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Attribute (type-related) " << name_
	  << " at " << get_fileline() << endl;
      base_->show(out);
}

void ExpObjAttribute::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Attribute (object-related) " << name_
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

      for (list<case_t*>::const_iterator cur = options_.begin()
		 ; cur != options_.end() ; ++cur) {
	    (*cur)->dump(out, indent);
      }
}

void ExpConditional::case_t::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "when:" << endl;
      if (cond_) cond_->dump(out, indent+4);
      out << setw(indent) << "" << "do:" << endl;
      for (list<Expression*>::const_iterator cur = true_clause_.begin()
		 ; cur != true_clause_.end() ; ++cur) {
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

void ExpFunc::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "function " << name_
	  << " has " << argv_.size() << " arguments:" << endl;
      for (size_t idx = 0 ; idx < argv_.size() ; idx += 1) {
	    argv_[idx]->dump(out, indent+2);
      }
}

void ExpInteger::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Integer " << value_
	  << " at " << get_fileline() << endl;
}

void ExpReal::dump(ostream&out, int indent) const
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
      if (prefix_.get())
	    prefix_->dump(out, indent+8);

      if (indices_) {
          for(list<Expression*>::const_iterator it = indices_->begin();
                  it != indices_->end(); ++it) {
              (*it)->dump(out, indent+6);
          }
      }
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

void named_expr_t::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << name_ << "=>";
    expr_->dump(out, indent);
}

ostream& Expression::dump_inline(ostream&out) const
{
      out << typeid(*this).name();
      return out;
}

ostream& ExpInteger::dump_inline(ostream&out) const
{
      out << value_;
      return out;
}

ostream& ExpReal::dump_inline(ostream&out) const
{
      out << value_;
      return out;
}

void SubprogramBody::dump(ostream&fd) const
{
      if (statements_== 0 || statements_->empty()) {
	    fd << "        <no definition>" << endl;
      } else {
	    for (list<SequentialStmt*>::const_iterator cur = statements_->begin()
		       ; cur != statements_->end() ; ++cur) {
		  SequentialStmt*curp = *cur;
		  curp->dump(fd, 8);
	    }
      }
}

void SubprogramHeader::dump(ostream&fd) const
{
      fd << "     " << name_;

      if (ports_->empty()) {
	    fd << "()";

      } else {
	    fd << "(";

	    list<InterfacePort*>::const_iterator cur = ports_->begin();
	    InterfacePort*curp = *cur;
	    fd << curp->name << ":";
	    curp->type->show(fd);

	    for (++ cur ; cur != ports_->end() ; ++ cur) {
		  curp = *cur;
		  fd << "; " << curp->name << ":";
		  curp->type->show(fd);
	    }
	    fd << ")";
      }

      fd << " return ";
      return_type_->show(fd);
      fd << endl;
}
