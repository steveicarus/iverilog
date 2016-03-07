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
# include  "parse_types.h"
# include  "sequential.h"
// Need this for parse_errors?
# include  "parse_api.h"
# include  <cassert>

using namespace std;

Architecture::Architecture(perm_string name, const ActiveScope&ref,
			   list<Architecture::Statement*>&s)
: Scope(ref), name_(name), cur_component_(NULL), cur_process_(NULL)
{
      statements_.splice(statements_.end(), s);
}

Architecture::~Architecture()
{
    delete_all(statements_);
    ScopeBase::cleanup();
}

bool Architecture::find_constant(perm_string by_name, const VType*&typ, Expression*&exp) const
{
    if(Scope::find_constant(by_name, typ, exp))
        return true;

    // Check generics in components
    if(cur_component_) {
        std::map<perm_string,ComponentBase*>::const_iterator c = new_components_.find(cur_component_->component_name());
        if(c == new_components_.end())
            c = old_components_.find(cur_component_->component_name());

        assert(c != old_components_.end());
        ComponentBase*base = c->second;

        const InterfacePort*generic = base->find_generic(by_name);
        if(!generic)
            return false;   // apparently there is no such generic in the component

        Expression*e = cur_component_->find_generic_map(by_name);

        typ = generic->type;
        exp = e ? e : generic->expr;
        return true;
    }

    return false;
}

Variable* Architecture::find_variable(perm_string by_name) const
{
    if(cur_process_)
        return cur_process_->find_variable(by_name);

    return ScopeBase::find_variable(by_name);
}

void Architecture::push_genvar_type(perm_string gname, const VType*gtype)
{
      genvar_type_t tmp;
      tmp.name = gname;
      tmp.vtype = gtype;
      genvar_type_stack_.push_back(tmp);
}

void Architecture::pop_genvar_type(void)
{
      assert(! genvar_type_stack_.empty());
      genvar_type_stack_.pop_back();
}

const VType* Architecture::probe_genvar_type(perm_string gname)
{
      for (std::list<genvar_type_t>::reverse_iterator cur = genvar_type_stack_.rbegin()
		 ; cur != genvar_type_stack_.rend() ; ++cur) {
	    if (cur->name == gname)
		  return cur->vtype;
      }
      return 0;
}

void Architecture::push_genvar_emit(perm_string gname, const GenerateStatement*gen)
{
      genvar_emit_t tmp;
      tmp.name = gname;
      tmp.gen = gen;
      genvar_emit_stack_.push_back(tmp);
}

void Architecture::pop_genvar_emit(void)
{
      assert(! genvar_emit_stack_.empty());
      genvar_emit_stack_.pop_back();
}

const GenerateStatement* Architecture::probe_genvar_emit(perm_string gname)
{
      for (std::list<genvar_emit_t>::reverse_iterator cur = genvar_emit_stack_.rbegin()
		 ; cur != genvar_emit_stack_.rend() ; ++cur) {
	    if (cur->name == gname)
		  return cur->gen;
      }
      return 0;
}

Architecture::Statement::Statement()
{
}

Architecture::Statement::~Statement()
{
}

GenerateStatement::GenerateStatement(perm_string gname,
				     std::list<Architecture::Statement*>&s)
: name_(gname)
{
      statements_.splice(statements_.end(), s);
}

GenerateStatement::~GenerateStatement()
{
      for_each(statements_.begin(), statements_.end(), ::delete_object<Architecture::Statement>());
}

ForGenerate::ForGenerate(perm_string gname, perm_string genvar,
			 ExpRange*rang, std::list<Architecture::Statement*>&s)
: GenerateStatement(gname, s), genvar_(genvar),
  lsb_(rang->lsb()), msb_(rang->msb())
{
}

ForGenerate::~ForGenerate()
{
}

IfGenerate::IfGenerate(perm_string gname, Expression*cond,
		       std::list<Architecture::Statement*>&s)
: GenerateStatement(gname, s), cond_(cond)
{
}

IfGenerate::~IfGenerate()
{
}

SignalAssignment::SignalAssignment(ExpName*name, list<Expression*>&rv)
: lval_(name)
{
      rval_.splice(rval_.end(), rv);
}

SignalAssignment::SignalAssignment(ExpName*name, Expression*rv)
: lval_(name)
{
      rval_.push_back(rv);
}

SignalAssignment::~SignalAssignment()
{
      for (list<Expression*>::iterator cur = rval_.begin()
		 ; cur != rval_.end() ; ++cur) {
	    delete *cur;
      }
      delete lval_;
}

CondSignalAssignment::CondSignalAssignment(ExpName*target, std::list<ExpConditional::case_t*>&options)
: lval_(target)
{
    options_.splice(options_.end(), options);
}

CondSignalAssignment::~CondSignalAssignment()
{
    delete lval_;
    for(list<ExpConditional::case_t*>::iterator it = options_.begin();
            it != options_.end(); ++it) {
        delete *it;
    }
}

ComponentInstantiation::ComponentInstantiation(perm_string i, perm_string c,
					       list<named_expr_t*>*parms,
					       list<named_expr_t*>*ports)
: iname_(i), cname_(c)
{
      typedef pair<map<perm_string,Expression*>::iterator,bool> insert_rc;

      while (parms && ! parms->empty()) {
	    named_expr_t*cur = parms->front();
	    parms->pop_front();
	    insert_rc rc = generic_map_.insert(make_pair(cur->name(), cur->expr()));
	    if (! rc.second) {
		  cerr << "?:?: error: Duplicate map of generic " << cur->name()
		       << " ignored." << endl;
		  parse_errors += 1;
	    }
      }

      while (ports && ! ports->empty()) {
	    named_expr_t*cur = ports->front();
	    ports->pop_front();
	    insert_rc rc = port_map_.insert(make_pair(cur->name(), cur->expr()));
	    if (! rc.second) {
		  cerr << "?:?: error: Duplicate map of port " << cur->name()
		       << " ignored." << endl;
		  parse_errors += 1;
	    }
      }
}

ComponentInstantiation::~ComponentInstantiation()
{
      for (map<perm_string,Expression*>::iterator it = generic_map_.begin()
		 ; it != generic_map_.end() ; ++it) {
	    delete it->second;
      }
      for (map<perm_string,Expression*>::iterator it = port_map_.begin()
		; it != port_map_.end(); ++it) {
	    delete it->second;
      }
}

Expression*ComponentInstantiation::find_generic_map(perm_string by_name) const
{
    map<perm_string,Expression*>::const_iterator p = generic_map_.find(by_name);

    if(p == generic_map_.end())
        return NULL;

    return p->second;
}

StatementList::StatementList(std::list<SequentialStmt*>*statement_list)
{
    if(statement_list)
	  statements_.splice(statements_.end(), *statement_list);
}

StatementList::~StatementList()
{
    for(std::list<SequentialStmt*>::iterator it = statements_.begin();
            it != statements_.end(); ++it) {
        delete *it;
    }
}

ProcessStatement::ProcessStatement(perm_string iname,
				   const ActiveScope&ref,
				   std::list<Expression*>*sensitivity_list,
				   std::list<SequentialStmt*>*statements_list)
: StatementList(statements_list), Scope(ref), iname_(iname)
{
      if (sensitivity_list)
	    sensitivity_list_.splice(sensitivity_list_.end(), *sensitivity_list);
}

ProcessStatement::~ProcessStatement()
{
    for(std::list<Expression*>::iterator it = sensitivity_list_.begin();
            it != sensitivity_list_.end(); ++it) {
        delete *it;
    }
}
