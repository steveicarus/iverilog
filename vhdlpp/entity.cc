
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

# include  "entity.h"
# include  "architec.h"
# include  <cassert>

using namespace std;

std::map<perm_string,Entity*> design_entities;

ComponentBase::ComponentBase(perm_string name)
: name_(name)
{
}

ComponentBase::~ComponentBase()
{
    for(std::vector<InterfacePort*>::iterator it = ports_.begin()
        ; it != ports_.end(); ++it)
        delete *it;
}

void ComponentBase::set_interface(std::list<InterfacePort*>*parms,
				  std::list<InterfacePort*>*ports)
{
      if (parms) {
	    while (! parms->empty()) {
		  parms_.push_back(parms->front());
		  parms->pop_front();
	    }
      }

      if (ports) {
        while (! ports->empty()) {
          ports_.push_back(ports->front());
          ports->pop_front();
        }
      }
}

const InterfacePort* ComponentBase::find_port(perm_string my_name) const
{
      for (size_t idx = 0 ; idx < ports_.size() ; idx += 1) {
	    if (ports_[idx]->name == my_name)
		  return ports_[idx];
      }

      return 0;
}

const InterfacePort* ComponentBase::find_generic(perm_string my_name) const
{
      for (size_t idx = 0 ; idx < parms_.size() ; idx += 1) {
	    if (parms_[idx]->name == my_name)
		  return parms_[idx];
      }

      return 0;
}

Entity::Entity(perm_string name)
: ComponentBase(name)
{
      bind_arch_ = 0;
}

Entity::~Entity()
{
    for(map<perm_string,Architecture*>::reverse_iterator it = arch_.rbegin()
        ; it != arch_.rend(); ++it)
       delete it->second;
}

Architecture* Entity::add_architecture(Architecture*that)
{
      if (Architecture*tmp = arch_ [that->get_name()]) {
	    return tmp;
      }

      return arch_[that->get_name()] = that;
}

void Entity::set_declaration_l_value(perm_string nam, bool flag)
{
      map<perm_string,VType::decl_t>::iterator cur = declarations_.find(nam);
      assert(cur != declarations_.end());
      cur->second.reg_flag = flag;
}

static map<const VType*, const VTypeDef*> global_types;

int emit_global_types(ostream&fd) {
    int errors = 0;

    for (map<const VType*, const VTypeDef*>::const_iterator cur = global_types.begin()
            ; cur != global_types.end() ; ++ cur) {
        fd << "typedef ";
        const VTypeDef*td = cur->second;
        errors += td->peek_definition()->emit_def(fd, td->peek_name());
        fd << " ;" << endl;
    }

    return errors;
}

const VTypeDef*get_global_typedef(const VType*t) {
    map<const VType*, const VTypeDef*>::iterator it;

    // Check if it is already defined global typedef
    if(const VTypeDef*tdef = dynamic_cast<const VTypeDef*>(t)) {
        for(it = global_types.begin(); it != global_types.end(); ++it) {
            if(it->second == tdef)
                return tdef;
        }
    }

    it = global_types.find(t);

    if(it != global_types.end())
        return it->second;

    const VTypeDef*td = new VTypeDef(t->get_generic_typename(), t);
    global_types[t] = td;

    return td;
}
