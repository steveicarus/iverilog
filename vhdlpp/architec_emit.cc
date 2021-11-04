/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
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
# include  "entity.h"
# include  "expression.h"
# include  "sequential.h"
# include  "subprogram.h"
# include  "vsignal.h"
# include  "std_types.h"
# include  <iostream>
# include  <typeinfo>
# include  <ivl_assert.h>

using namespace std;

int Scope::emit_signals(ostream&out, Entity*entity, ScopeBase*scope)
{
    int errors = 0;

    for (map<perm_string,Signal*>::iterator cur = new_signals_.begin()
       ; cur != new_signals_.end() ; ++cur) {
        errors += cur->second->emit(out, entity, scope);
    }

    return errors;
}

int Scope::emit_variables(ostream&out, Entity*entity, ScopeBase*scope)
{
    int errors = 0;

    for (map<perm_string,Variable*>::iterator cur = new_variables_.begin()
       ; cur != new_variables_.end() ; ++cur) {
        errors += cur->second->emit(out, entity, scope);
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
      for (map<perm_string,const VType*>::iterator cur = use_types_.begin()
		 ; cur != use_types_.end() ; ++cur) {
	    if(is_global_type(cur->first))
                continue;

	    if(const VTypeDef*def = dynamic_cast<const VTypeDef*>(cur->second))
		errors += def->emit_typedef(out, typedef_ctx);
      }
      for (map<perm_string,const VType*>::iterator cur = cur_types_.begin()
		 ; cur != cur_types_.end() ; ++cur) {
	    if(const VTypeDef*def = dynamic_cast<const VTypeDef*>(cur->second))
		errors += def->emit_typedef(out, typedef_ctx);
      }

      for (map<perm_string,struct const_t*>::iterator cur = use_constants_.begin()
         ; cur != use_constants_.end() ; ++cur) {

        out << "localparam " << cur->first << " = ";
        errors += cur->second->val->emit(out, entity, this);
        out << ";" << endl;
      }
      for (map<perm_string,struct const_t*>::iterator cur = cur_constants_.begin()
         ; cur != cur_constants_.end() ; ++cur) {

        out << "localparam " << cur->first << " = ";
        errors += cur->second->val->emit(out, entity, this);
        out << ";" << endl;
      }

      errors += emit_signals(out, entity, this);
      errors += emit_variables(out, entity, this);

      for (map<perm_string,SubHeaderList>::const_iterator cur = cur_subprograms_.begin()
		 ; cur != cur_subprograms_.end() ; ++ cur) {
	    const SubHeaderList& subp_list = cur->second;

	    for(SubHeaderList::const_iterator it = subp_list.begin();
			it != subp_list.end(); ++it) {
                SubprogramHeader*subp = *it;

		// Do not emit unbounded functions, we will just need fixed instances later
		if(!subp->unbounded())
		    errors += subp->emit_package(out);
	    }
      }

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
      const Expression*rval = rval_.front();

      out << "// " << get_fileline() << endl;
      out << "assign ";
      if(const ExpDelay*delayed = dynamic_cast<const ExpDelay*>(rval)) {
        out << "#(";
        delayed->peek_delay()->emit(out, ent, arc);
        out << ") ";
        rval = delayed->peek_expr();
      }
      errors += lval_->emit(out, ent, arc);
      out << " = ";
      errors += rval->emit(out, ent, arc);
      out << ";" << endl;

      return errors;
}

int CondSignalAssignment::emit(ostream&out, Entity*ent, Architecture*arc)
{
    int errors = 0;

    out << "// " << get_fileline() << endl;
    out << "always begin" << endl;
    bool first = true;

    for(list<ExpConditional::case_t*>::iterator it = options_.begin();
            it != options_.end(); ++it) {
        ExpConditional::case_t*cas = *it;
        ivl_assert(*this, cas->true_clause().size() == 1);
        const Expression*rval = cas->true_clause().front();

        if(first)
            first = false;
        else
            out << "else ";

        if(Expression*cond = cas->condition()) {
            out << "if(";
            cond->emit(out, ent, arc);
            out << ") ";
        }

        out << endl;
        lval_->emit(out, ent, arc);
        out << " = ";
        rval->emit(out, ent, arc);
        out << ";" << endl;
    }

    // Sensitivity list
    first = true;
    out << "@(";

    for(list<const ExpName*>::const_iterator it = sens_list_.begin();
            it != sens_list_.end(); ++it) {
        if(first)
            first = false;
        else
            out << ",";

        errors += (*it)->emit(out, ent, arc);
    }

    out << ");" << endl;
    out << "end" << endl;

    return errors;
}

int ComponentInstantiation::emit(ostream&out, Entity*ent, Architecture*arc)
{
      const char*comma = "";
      int errors = 0;

      arc->set_cur_component(this);

      if(ComponentBase*comp = arc->find_component(cname_)) {
        const std::vector<InterfacePort*>& generics = comp->get_generics();

      if(generics.size() != generic_map_.size())
	    // Display an error for generics that do not have neither
	    // default nor component specific value defined
	    for(vector<InterfacePort*>::const_iterator it = generics.begin();
			it != generics.end(); ++it) {
		if(!(*it)->expr && generic_map_.count((*it)->name) == 0) {
		    cerr << get_fileline() << ": generic " << (*it)->name <<
			"value is not defined" << endl;
		    ++errors;
		}
	    }
      }

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

      arc->set_cur_component(NULL);

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
      out << "genvar \\" << get_name() << ":" << genvar_ << " ;" << endl;
      out << "for (\\" << get_name() << ":" << genvar_ << " = ";
      errors += lsb_->emit(out, ent, arc);
      out << "; \\" << get_name() << ":" << genvar_ << " <= ";
      errors += msb_->emit(out, ent, arc);
      out << "; \\" << get_name() << ":" << genvar_ << " = \\" << get_name() << ":" << genvar_ << " + 1)"
	  << " begin : \\" << get_name() << endl;

      arc->push_genvar_emit(genvar_, this);

      errors += emit_statements(out, ent, arc);

      arc->pop_genvar_emit();
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

int StatementList::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      for (std::list<SequentialStmt*>::iterator it = statements_.begin();
              it != statements_.end(); ++it) {
            errors += (*it)->emit(out, ent, scope);
      }

      return errors;
}

int InitialStatement::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      out << "initial begin" << endl;
      int errors = StatementList::emit(out, ent, scope);
      out << "end" << endl;

      return errors;
}

int FinalStatement::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      out << "final begin" << endl;
      int errors = StatementList::emit(out, ent, scope);
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
int ProcessStatement::emit(ostream&out, Entity*ent, Architecture*)
{
      int errors = 0;

      /* Check if the process has no sensitivity list and ends up with
       * a final wait. If so, convert the process to an initial block. */
      const WaitStmt*wait_stmt = NULL;
      if (!stmt_list().empty())
          wait_stmt = dynamic_cast<const WaitStmt*>(stmt_list().back());

      if (wait_stmt && wait_stmt->type() == WaitStmt::FINAL)
          out << "initial begin : ";
      else
          out << "always begin : ";

      out << peek_name() << endl;

      errors += emit_variables(out, ent, this);
      errors += StatementList::emit(out, ent, this);

      if (! sensitivity_list_.empty()) {
	    out << "@(";
	    const char*comma = 0;
	    for (list<Expression*>::iterator cur = sensitivity_list_.begin()
		       ; cur != sensitivity_list_.end() ; ++cur) {

		  if (comma) out << comma;
		  errors += (*cur)->emit(out, ent, this);
		  comma = ", ";
	    }
	    out << "); /* sensitivity list for process */" << endl;
      }

      out << "end /* " << peek_name() << " */" << endl;
      return errors;
}
