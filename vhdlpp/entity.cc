
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

# include  "entity.h"
# include  "architec.h"

using namespace std;

std::map<perm_string,Entity*> design_entities;

ComponentBase::ComponentBase(perm_string name)
: name_(name)
{
}

ComponentBase::~ComponentBase()
{
}

void ComponentBase::set_interface(std::list<InterfacePort*>*ports)
{
	while (ports->size() > 0) {
	      ports_.push_back(ports->front());
	      ports->pop_front();
	}
}

Entity::Entity(perm_string name)
: ComponentBase(name)
{
}

Entity::~Entity()
{
}

Architecture* Entity::add_architecture(Architecture*that)
{
      if (Architecture*tmp = arch_ [that->get_name()]) {
	    return tmp;
      }

      return arch_[that->get_name()] = that;
}
