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
# include  <iostream>
# include  <fstream>
# include  <iomanip>

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
	    cerr << "Elaborate entity " << name << "..." << endl;

      cout << "module " << name;

	// If there are ports, emit them.
      if (ports.size() > 0) {
	    cout << "(";
	    const char*sep = 0;
	    for (vector<InterfacePort*>::iterator cur = ports.begin()
		       ; cur != ports.end() ; ++cur) {
		  InterfacePort*port = *cur;

		  if (sep) cout << sep;
		  else sep = ", ";

		  switch (port->mode) {
		      case PORT_NONE: // Should not happen
			cout << "NO_PORT " << port->name;
			break;
		      case PORT_IN:
			cout << "input " << port->name;
			break;
		      case PORT_OUT:
			cout << "output " << port->name;
			break;
		  }
	    }
	    cout << ")";
      }

      cout << ";" << endl;

      cout << "endmodule" << endl;

      return errors;
}
