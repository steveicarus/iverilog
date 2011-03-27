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
# include  "compiler.h"
# include  "architec.h"
# include  "vtype.h"
# include  <iostream>
# include <typeinfo>
# include  <fstream>
# include  <iomanip>
# include  <cstring>
# include  <cassert>

using namespace std;

int elaborate_entities(void)
{
      int errors = 0;

      for (map<perm_string,Entity*>::iterator cur = design_entities.begin()
		 ; cur != design_entities.end() ; ++cur) {
	    errors += cur->second->elaborate();
      }

      return errors;
}

int Entity::elaborate()
{
      int errors = 0;

      if (verbose_flag)
	    cerr << "Elaborate entity " << get_name() << "..." << endl;

      if (arch_.size() == 0) {
	    cerr << get_fileline() << ": error: "
		 << "No architectures to choose from for entity " << get_name()
		 << "." << endl;
	    return 1;
      }

        /* FIXME: the architecture for the entity should be chosen in configuration
        block (not yet implemented). Multiple architectures are allowed in general */
      if (arch_.size() > 1) {
	    cerr << get_fileline() << ": sorry: "
		 << "Multiple architectures for an entity are not yet supported"
		 << ". Architectures for entity " << get_name() << " are:" << endl;
	    for (map<perm_string,Architecture*>::const_iterator cur = arch_.begin()
		       ; cur != arch_.end() ; ++cur) {
		  cerr << get_fileline() << ":      : " << cur->first
		       << " at " << cur->second->get_fileline() << endl;
	    }

	    //errors += 1;
      }
        /* FIXME: here we should look at configuration block */
      bind_arch_ = arch_.begin()->second;
      if (verbose_flag)
	    cerr << "For entity " << get_name()
		 << ", choosing architecture " << bind_arch_->get_name()
		 << "." << endl;

      errors += elaborate_ports_();

      errors += bind_arch_->elaborate(this);

      return errors;
}

int Entity::elaborate_ports_(void)
{
      int errors = 0;
      const std::vector<InterfacePort*>&ports = get_ports();

      for (std::vector<InterfacePort*>::const_iterator cur = ports.begin()
		 ; cur != ports.end() ; ++cur) {

	    InterfacePort*cur_port = *cur;
	    VType::decl_t cur_decl;

	    const VType*type = cur_port->type;
	    if (type == 0) {
		  cerr << get_fileline() << ": error: "
		       << "Giving up on unknown type for port " << cur_port->name
		       << "." << endl;
		  errors += 1;
		  continue;
	    }

	    type->elaborate(cur_decl);

	    declarations_[cur_port->name] = cur_decl;
      }

      return errors;
}
