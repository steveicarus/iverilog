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
// Need this for parse_errors?
# include  "parse_api.h"

using namespace std;

Architecture::Architecture(perm_string name, const ScopeBase&ref,
			   list<Architecture::Statement*>&s)
: Scope(ref), name_(name)
{
      statements_.splice(statements_.end(), s);
}

Architecture::~Architecture()
{
    delete_all(statements_);
    ScopeBase::cleanup();
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
			 prange_t*rang, std::list<Architecture::Statement*>&s)
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

ProcessStatement::ProcessStatement(perm_string iname,
				   std::list<Expression*>*sensitivity_list,
				   std::list<SequentialStmt*>*statements_list)
: iname_(iname)
{
      if (sensitivity_list)
	    sensitivity_list_.splice(sensitivity_list_.end(), *sensitivity_list);
      if (statements_list)
	    statements_list_.splice(statements_list_.end(), *statements_list);
}

ProcessStatement::~ProcessStatement()
{
}
