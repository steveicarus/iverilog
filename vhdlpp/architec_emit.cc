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

# include  "architec.h"
# include  "entity.h"
# include  "expression.h"
# include  "sequential.h"
# include  "vsignal.h"
# include  <iostream>
# include  <typeinfo>
# include  <ivl_assert.h>

int Scope::emit_signals(ostream&out, Entity*entity, Architecture*arc)
{
      int errors = 0;

      for (map<perm_string,Signal*>::iterator cur = old_signals_.begin()
		 ; cur != old_signals_.end() ; ++cur) {

	    errors += cur->second->emit(out, entity, arc);
      }
      for (map<perm_string,Signal*>::iterator cur = new_signals_.begin()
         ; cur != new_signals_.end() ; ++cur) {

        errors += cur->second->emit(out, entity, arc);
      }
      return errors;
}

int Scope::emit_variables(ostream&out, Entity*entity, Architecture*arc)
{
      int errors = 0;

      for (map<perm_string,Variable*>::iterator cur = old_variables_.begin()
		 ; cur != old_variables_.end() ; ++cur) {

	    errors += cur->second->emit(out, entity, arc);
      }
      for (map<perm_string,Variable*>::iterator cur = new_variables_.begin()
         ; cur != new_variables_.end() ; ++cur) {

        errors += cur->second->emit(out, entity, arc);
      }
      return errors;
}

int Architecture::emit(ostream&out, Entity*entity)
{
      int errors = 0;

	// Find typedefs that are present in the architecture body and
	// emit them, so that following code can use the name instead
	// of the full definition.

      typedef_context_t typedef_ctx;
      for (map<perm_string,const VType*>::iterator cur = old_types_.begin()
		 ; cur != old_types_.end() ; ++cur) {

	    const VTypeDef*def = dynamic_cast<const VTypeDef*>(cur->second);
	    if (def == 0)
		  continue;

	    errors += def->emit_typedef(out, typedef_ctx);
      }

      for (map<perm_string,struct const_t*>::iterator cur = old_constants_.begin()
         ; cur != old_constants_.end() ; ++cur) {

        out << "localparam " << cur->first << " = ";
        errors += cur->second->val->emit(out, entity, this);
        out << ";" << endl;
      }
      for (map<perm_string,struct const_t*>::iterator cur = new_constants_.begin()
         ; cur != new_constants_.end() ; ++cur) {

        out << "localparam " << cur->first << " = ";
        errors += cur->second->val->emit(out, entity, this);
        out << ";" << endl;
      }

      errors += emit_signals(out, entity, this);
      errors += emit_variables(out, entity, this);

      for (list<Architecture::Statement*>::iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {

	    errors += (*cur)->emit(out, entity, this);
      }

      return errors;
}

int Architecture::Statement::emit(ostream&out, Entity*, Architecture*)
{
      out << " // " << get_fileline() << ": internal error: "
	  << "I don't know how to emit this statement! "
	  << "type=" << typeid(*this).name() << endl;
      return 1;
}

int SignalAssignment::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      ivl_assert(*this, rval_.size() == 1);
      Expression*rval = rval_.front();

      out << "// " << get_fileline() << endl;
      out << "assign ";
      errors += lval_->emit(out, ent, arc);
      out << " = ";

      errors += rval->emit(out, ent, arc);

      out << ";" << endl;
      return errors;
}

int ComponentInstantiation::emit(ostream&out, Entity*ent, Architecture*arc)
{
      const char*comma = "";
      int errors = 0;

      out << cname_;
      if (! generic_map_.empty()) {
	    out << " #(";
	    comma = "";
	    for (map<perm_string,Expression*>::iterator cur = generic_map_.begin()
		       ; cur != generic_map_.end() ; ++cur) {
		  ivl_assert(*this, cur->second);
		  out << comma << ".\\" << cur->first << " (";
		  errors += cur->second->emit(out, ent, arc);
		  out << ")";
		  comma = ", ";
	    }
	    out << ")";
      }

      out << " \\" << iname_ << " (";
      comma = "";
      for (map<perm_string,Expression*>::iterator cur = port_map_.begin()
		 ; cur != port_map_.end() ; ++cur) {
	      // Skip unconnected ports
	    if (cur->second == 0)
		  continue;
	    out << comma << ".\\" << cur->first << " (";
	    errors += cur->second->emit(out, ent, arc);
	    out << ")";
	    comma = ", ";
      }
      out << ");" << endl;

      return errors;
}

int GenerateStatement::emit_statements(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      for (list<Architecture::Statement*>::iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {
	    Architecture::Statement*curp = *cur;
	    errors += curp->emit(out, ent, arc);
      }
      return errors;
}

int ForGenerate::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      out << "genvar \\" << genvar_ << " ;" << endl;
      out << "for (\\" << genvar_ << " = ";
      errors += lsb_->emit(out, ent, arc);
      out << "; \\" << genvar_ << " <= ";
      errors += msb_->emit(out, ent, arc);
      out << "; \\" << genvar_ << " = \\" << genvar_ << " + 1)"
	  << " begin : \\" << get_name() << endl;

      errors += emit_statements(out, ent, arc);

      out << "end" << endl;

      return errors;
}

int IfGenerate::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      out << "if (";
      cond_->emit(out, ent, arc);
      out << ") begin : \\" << get_name() << endl;

      errors += emit_statements(out, ent, arc);

      out << "end" << endl;

      return errors;
}

/*
 * Emit a process statement using "always" syntax.
 *
 * Note that VHDL is different from Verilog, in that the sensitivity
 * list goes at the END of the statement list, not at the
 * beginning. In VHDL, all the statements are initially executed once
 * before blocking in the first wait on the sensitivity list.
 */
int ProcessStatement::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      out << "always begin" << endl;

      for (list<SequentialStmt*>::iterator cur = statements_list_.begin()
		 ; cur != statements_list_.end() ; ++cur) {
	    errors += (*cur)->emit(out, ent, arc);
      }

      if (! sensitivity_list_.empty()) {
	    out << "@(";
	    const char*comma = 0;
	    for (list<Expression*>::iterator cur = sensitivity_list_.begin()
		       ; cur != sensitivity_list_.end() ; ++cur) {

		  if (comma) out << comma;
		  errors += (*cur)->emit(out, ent, arc);
		  comma = ", ";
	    }
	    out << ") /* sensitivity list for process */;" << endl;
      }

      out << "end" << endl;
      return errors;

}
