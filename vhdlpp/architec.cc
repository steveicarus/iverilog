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
# include  "expression.h"
# include  "parse_types.h"

using namespace std;

Architecture::Architecture(perm_string name, const ScopeBase&ref,
			   list<Architecture::Statement*>&s)
: Scope(ref), name_(name)
{
      statements_.splice(statements_.end(), s);
}

Architecture::~Architecture()
{
}

Architecture::Statement::Statement()
{
}

Architecture::Statement::~Statement()
{
}

SignalAssignment::SignalAssignment(ExpName*name, list<Expression*>&rv)
: lval_(name)
{
      rval_.splice(rval_.end(), rv);
}

SignalAssignment::~SignalAssignment()
{
      for (list<Expression*>::iterator cur = rval_.begin()
		 ; cur != rval_.end() ; ++cur) {
	    delete *cur;
      }
}

ComponentInstantiation::ComponentInstantiation(perm_string i, perm_string c,
					       list<named_expr_t*>*ports)
: iname_(i), cname_(c)
{
      while (! ports->empty()) {
	    named_expr_t*cur = ports->front();
	    ports->pop_front();
	    port_map_[cur->name()] = cur->expr();
      }
}

ComponentInstantiation::~ComponentInstantiation()
{
}
