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

# include  "architec.h"
# include  "expression.h"
# include  "sequential.h"
# include  <fstream>
# include  <iomanip>
# include  <typeinfo>

using namespace std;

void Architecture::dump(ostream&out, perm_string of_entity, int indent) const
{
      out << setw(indent) << "" << "architecture " << name_
	  << " of entity " << of_entity
	  << " file=" << get_fileline() << endl;

      dump_scope(out);

      for (list<Architecture::Statement*>::const_iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {
	    (*cur)->dump(out, indent+3);
      }
}

void Architecture::Statement::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Architecture::Statement at file=" << get_fileline() << endl;
}

void ComponentInstantiation::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "Component Instantiation "
	  << "instance=" << iname_
	  << " of component=" << cname_
	  << ", file=" << get_fileline() << endl;

      for (map<perm_string,Expression*>::const_iterator cur = port_map_.begin()
		 ; cur != port_map_.end() ; ++cur) {
	    out << setw(indent+2) <<""<< cur->first << " => ..." << endl;
	    if (cur->second)
		  cur->second->dump(out, indent+6);
	    else
		  out << setw(indent+6) <<""<< "OPEN" << endl;
      }

}

void GenerateStatement::dump_statements(ostream&out, int indent) const
{
      for (list<Architecture::Statement*>::const_iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {
	    Statement*curp = *cur;
	    curp->dump(out, indent);
      }
}

void ForGenerate::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "for " << genvar_
	  << " in" << endl;
      msb_->dump(out, indent+4);
      lsb_->dump(out, indent+4);
      out << setw(indent) << "" << "generate" << endl;
      dump_statements(out, indent+4);
      out << setw(indent) << "" << "end generate" << endl;
}

void SignalAssignment::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "SignalAssignment file=" << get_fileline() << endl;
      lval_->dump(out, indent+1);
      out << setw(indent+2) << "" << "<= <expr>..." << endl;

      for (list<Expression*>::const_iterator cur = rval_.begin()
		 ; cur != rval_.end() ; ++cur) {
	    (*cur)->dump(out, indent+2);
      }
}

void CondSignalAssignment::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "CondSignalAssignment file=" << get_fileline() << endl;
      lval_->dump(out, indent+1);
      out << setw(indent+2) << "" << "<= <expr>..." << endl;

      for(list<ExpConditional::case_t*>::const_iterator it = options_.begin();
              it != options_.end(); ++it) {
          (*it)->dump(out, indent+2);
      }
}

void StatementList::dump(ostream&out, int indent) const
{
      out << setw(indent+3) << "" << "sequence of statements:" << endl;

      for (list<SequentialStmt*>::const_iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {
	    (*cur)->dump(out, indent+4);
      }
}

void InitialStatement::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "InitialStatement file=" << get_fileline() << endl;

      StatementList::dump(out, indent);
}

void FinalStatement::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "FinalStatement file=" << get_fileline() << endl;

      StatementList::dump(out, indent);
}

void ProcessStatement::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "ProcessStatement name_=" << iname_
	  << " file=" << get_fileline() << endl;

      out << setw(indent+3) << "" << "Sensitivity_list:" << endl;

      for (list<Expression*>::const_iterator cur = sensitivity_list_.begin()
		 ; cur != sensitivity_list_.end() ; ++cur) {
	    (*cur)->dump(out, indent+4);
      }

      StatementList::dump(out, indent);
}
