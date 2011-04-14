#ifndef __scope_H
#define __scope_H
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

# include  <list>
# include  <map>
# include  "StringHeap.h"

class ComponentBase;
class VType;

class Scope {

    public:
      Scope(std::map<perm_string,ComponentBase*>&comps);
      ~Scope();

      ComponentBase* find_component(perm_string by_name);

      void collect_components(std::list<ComponentBase*>&res);

      void dump_scope(ostream&out) const;

    private:
      	// Component declarations...
      std::map<perm_string,ComponentBase*> components_;
	// Type declarations...
      std::map<perm_string,const VType*> types_;
};

#endif
