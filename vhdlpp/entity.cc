
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
# include  <fstream>
# include  <iomanip>

using namespace std;

std::map<perm_string,Entity*> design_entities;

static ostream& operator << (ostream&out, port_mode_t that)
{
      switch (that) {
	  case PORT_NONE:
	    out << "NO-PORT";
	    break;
	  case PORT_IN:
	    out << "IN";
	    break;
	  case PORT_OUT:
	    out << "OUT";
	    break;
	  default:
	    out << "PORT-????";
	    break;
      }
      return out;
}

static void dump_design_entity(ostream&out, Entity*obj)
{
      out << "entity " << obj->name << endl;
      if (obj->ports.size() == 0) {
	    out << "    No ports" << endl;
      } else {
	    out << "    PORTS:" << endl;
	    for (vector<InterfacePort*>::iterator cur = obj->ports.begin()
		       ; cur != obj->ports.end() ; ++cur) {
		  InterfacePort*item = *cur;
		  out << setw(6) << "" << item->name
		      << " : " << item->mode
		      << ", type=" << item->type_name << endl;
	    }
      }
}

void dump_design_entities(const char*path)
{
      ofstream file (path);
      for (map<perm_string,Entity*>::iterator cur = design_entities.begin()
		 ; cur != design_entities.end() ; ++cur) {
	    dump_design_entity(file, cur->second);
      }
}
