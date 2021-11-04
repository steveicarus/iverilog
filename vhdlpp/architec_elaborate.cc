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
# include  <typeinfo>
# include  <cassert>

using namespace std;

int Architecture::elaborate(Entity*entity)
{
      int errors = 0;

	// Constant assignments in the architecture get their types
	// from the constant declaration itself. Elaborate the value
	// expression with the declared type.

      for (map<perm_string,struct const_t*>::iterator cur = use_constants_.begin()
		 ; cur != use_constants_.end() ; ++cur) {
	    cur->second->val->elaborate_expr(entity, this, cur->second->typ);
      }
      for (map<perm_string,struct const_t*>::iterator cur = cur_constants_.begin()
		 ; cur != cur_constants_.end() ; ++cur) {
	    cur->second->val->elaborate_expr(entity, this, cur->second->typ);
      }

        // Elaborate initializer expressions for signals & variables
      for (map<perm_string,Signal*>::iterator cur = old_signals_.begin()
		 ; cur != old_signals_.end() ; ++cur) {
	    cur->second->elaborate(entity, this);
      }
      for (map<perm_string,Signal*>::iterator cur = new_signals_.begin()
		 ; cur != new_signals_.end() ; ++cur) {
	    cur->second->elaborate(entity, this);
      }
      for (map<perm_string,Variable*>::iterator cur = old_variables_.begin()
		 ; cur != old_variables_.end() ; ++cur) {
	    cur->second->elaborate(entity, this);
      }
      for (map<perm_string,Variable*>::iterator cur = new_variables_.begin()
		 ; cur != new_variables_.end() ; ++cur) {
	    cur->second->elaborate(entity, this);
      }

	// Elaborate subprograms
      for (map<perm_string,SubHeaderList>::const_iterator cur = cur_subprograms_.begin()
		 ; cur != cur_subprograms_.end() ; ++cur) {
	    const SubHeaderList& subp_list = cur->second;

	    for(SubHeaderList::const_iterator it = subp_list.begin();
			it != subp_list.end(); ++it) {
			errors += (*it)->elaborate();
	    }
      }
	// Create 'initial' and 'final' blocks for implicit
	// initialization and clean-up actions
      if(!initializers_.empty())
	    statements_.push_front(new InitialStatement(&initializers_));

      if(!finalizers_.empty())
	    statements_.push_front(new FinalStatement(&finalizers_));

      for (list<Architecture::Statement*>::iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {

	    int cur_errors = (*cur)->elaborate(entity, this);
	    errors += cur_errors;
      }

      if (errors > 0) {
	    cerr << errors << " errors in "
		 << name_ << " architecture of "
		 << entity->get_name() << "." << endl;
      }

      return errors;
}

int Architecture::Statement::elaborate(Entity*, Architecture*)
{
      return 0;
}

int ComponentInstantiation::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;

      ComponentBase*base = arc->find_component(cname_);
      if (base == 0) {
	    cerr << get_fileline() << ": error: No component declaration"
		 << " for instance " << iname_
		 << " of " << cname_ << "." << endl;
	    return 1;
      }

      arc->set_cur_component(this);

      for (map<perm_string,Expression*>::const_iterator cur = generic_map_.begin()
		 ; cur != generic_map_.end() ; ++cur) {
	      // check if generic from component instantiation
	      // exists in the component declaration
	    const InterfacePort*iparm = base->find_generic(cur->first);
	    if (iparm == 0) {
		  cerr << get_fileline() << ": warning: No generic " << cur->first
		       << " in component " << cname_ << "." << endl;
		  continue;
	    }

	    ExpName* tmp;
	    if (cur->second && (tmp = dynamic_cast<ExpName*>(cur->second)))
		  errors += tmp->elaborate_rval(ent, arc, iparm);

	    if (cur->second)
		  errors += cur->second->elaborate_expr(ent, arc, iparm->type);
      }

      for (map<perm_string,Expression*>::const_iterator cur = port_map_.begin()
		 ; cur != port_map_.end() ; ++cur) {
	      // check if a port from component instantiation
	      // exists in the component declaration
	    const InterfacePort*iport = base->find_port(cur->first);
	    if (iport == 0) {
		  cerr << get_fileline() << ": error: No port " << cur->first
		       << " in component " << cname_ << "." << endl;
		  errors += 1;
		  continue;
	    }

	    ExpName* tmp;
	    if (cur->second && (tmp = dynamic_cast<ExpName*>(cur->second)))
		  errors += tmp->elaborate_rval(ent, arc, iport);
	      /* It is possible for the port to be explicitly
		 unconnected. In that case, the Expression will be nil */

	    if (cur->second)
		  cur->second->elaborate_expr(ent, arc, iport->type);
      }

      arc->set_cur_component(NULL);

      return errors;
}

int GenerateStatement::elaborate_statements(Entity*ent, Architecture*arc)
{
      int errors = 0;
      for (list<Architecture::Statement*>::iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {
	    Architecture::Statement*curp = *cur;
	    errors += curp->elaborate(ent, arc);
      }
      return errors;
}

int ForGenerate::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;
      arc->push_genvar_type(genvar_, lsb_->probe_type(ent, arc));
      errors += elaborate_statements(ent, arc);
      arc->pop_genvar_type();
      return errors;
}

int IfGenerate::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;
      errors += elaborate_statements(ent, arc);
      return errors;
}

int StatementList::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      for (std::list<SequentialStmt*>::iterator it = statements_.begin();
              it != statements_.end(); ++it) {
            errors += (*it)->elaborate(ent, scope);
      }

      return errors;
}

int ProcessStatement::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;

      arc->set_cur_process(this);

      for (map<perm_string,Variable*>::iterator cur = new_variables_.begin()
		 ; cur != new_variables_.end() ; ++cur) {
	    cur->second->elaborate(ent, arc);
      }

      StatementList::elaborate(ent, arc);

      arc->set_cur_process(NULL);

      return errors;
}

int SignalAssignment::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;

	// Elaborate the l-value expression.
      errors += lval_->elaborate_lval(ent, arc, false);

	// The elaborate_lval should have resolved the type of the
	// l-value expression. We'll use that type to elaborate the
	// r-value.
      const VType*lval_type = lval_->peek_type();
      if (lval_type == 0) {
	    if (errors == 0) {
		  errors += 1;
		  cerr << get_fileline() << ": error: Unable to calculate type for l-value expression." << endl;
	    }
	    return errors;
      }

      for (list<Expression*>::iterator cur = rval_.begin()
		 ; cur != rval_.end() ; ++cur) {
	    errors += (*cur)->elaborate_expr(ent, arc, lval_type);
      }

      return errors;
}

int CondSignalAssignment::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;

        // Visitor to extract signal names occurring in the conditional
        // statements to create the sensitivity list
      struct name_extractor_t : public ExprVisitor {
          explicit name_extractor_t(list<const ExpName*>& name_list)
              : name_list_(name_list) {}
          void operator() (Expression*s) {
              if(const ExpName*name = dynamic_cast<const ExpName*>(s))
                  name_list_.push_back(name);
          }

          private:
            list<const ExpName*>& name_list_;
      } name_extractor(sens_list_);

        // Elaborate the l-value expression.
      errors += lval_->elaborate_lval(ent, arc, true);

        // The elaborate_lval should have resolved the type of the
        // l-value expression. We'll use that type to elaborate the
        // r-value.
      const VType*lval_type = lval_->peek_type();
      if (lval_type == 0) {
          if (errors == 0) {
              errors += 1;
              cerr << get_fileline()
                   << ": error: Unable to calculate type for l-value expression."
                   << endl;
          }
          return errors;
      }

      for(list<ExpConditional::case_t*>::iterator it = options_.begin();
              it != options_.end(); ++it) {
          ExpConditional::case_t*cas = (*it);
          cas->elaborate_expr(ent, arc, lval_type);
          cas->visit(name_extractor);
      }

      return errors;
}
