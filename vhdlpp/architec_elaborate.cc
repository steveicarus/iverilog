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
# include  <typeinfo>
# include  <cassert>

int Architecture::elaborate(Entity*entity)
{
      int errors = 0;

      for (list<Architecture::Statement*>::iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {

	    errors += (*cur)->elaborate(entity, this);
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

      const ComponentBase*base = arc->find_component(cname_);
      if (base == 0) {
	    cerr << get_fileline() << ": error: No component declaration"
		 << " for instance " << iname_
		 << " of " << cname_ << "." << endl;
	    return 1;
      }

      map<perm_string,const InterfacePort*> port_match;

      for (map<perm_string,Expression*>::iterator cur = port_map_.begin()
		 ; cur != port_map_.end() ; ++cur) {
	    const InterfacePort*iport = base->find_port(cur->first);
	    if (iport == 0) {
		  cerr << get_fileline() << ": error: No port " << cur->first
		       << " in component " << cname_ << "." << endl;
		  errors += 1;
		  continue;
	    }
      }

      return errors;
}
