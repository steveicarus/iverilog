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
# include  <iostream>
# include  <fstream>
# include  <iomanip>
# include  <cstring>

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
	    cerr << "Elaborate entity " << name_ << "..." << endl;

      if (arch_.size() == 0) {
	    cerr << get_fileline() << ": error: "
		 << "No architectures to choose from for entity " << name_
		 << "." << endl;
	    return 1;
      }


      if (arch_.size() > 1) {
	    cerr << get_fileline() << ": sorry: "
		 << "Too many architectures for entity " << name_
		 << ". Architectures are:" << endl;
	    for (map<perm_string,Architecture*>::const_iterator cur = arch_.begin()
		       ; cur != arch_.end() ; ++cur) {
		  cerr << get_fileline() << ":      : " << cur->first
		       << " at " << cur->second->get_fileline() << endl;
	    }

	    errors += 1;
      }

      bind_arch_ = arch_.begin()->second;
      if (verbose_flag)
	    cerr << "For entity " << get_name()
		 << ", choosing architecture " << bind_arch_->get_name()
		 << "." << endl;

      errors += elaborate_ports_();
      return errors;
}

int Entity::elaborate_ports_(void)
{
      int errors = 0;

      for (std::vector<InterfacePort*>::const_iterator cur = ports_.begin()
		 ; cur != ports_.end() ; ++cur) {

	    InterfacePort*cur_port = *cur;
	    decl_t cur_decl;
	    cur_decl.type = VNONE;
	    cur_decl.msb = 0;
	    cur_decl.lsb = 0;

	    if (strcasecmp(cur_port->type_name, "std_logic") == 0) {
		  cur_decl.type = VUWIRE;

	    } else if (strcasecmp(cur_port->type_name, "bit") == 0) {
		  cur_decl.type = VUWIRE;

	    } else if (strcasecmp(cur_port->type_name, "boolean") == 0) {
		  cur_decl.type = VUWIRE;

	    } else {
		  cerr << get_fileline() << ": error: "
		       << "I don't know how to map port " << cur_port->name
		       << " type " << cur_port->type_name << "." << endl;
		  errors += 1;
	    }

	    declarations_[cur_port->name] = cur_decl;
      }

      return errors;
}
