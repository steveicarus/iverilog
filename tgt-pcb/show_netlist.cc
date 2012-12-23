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

# include  "pcb_priv.h"
# include  <cassert>
# include  <cstdio>

using namespace std;

void show_netlist(const char*net_path)
{
      assert(net_path);
      FILE*fnet = fopen(net_path, "w");
      if (fnet == 0) {
	    perror(net_path);
	    return;
      }

      for (list<nexus_data*>::iterator cur = nexus_list.begin()
		 ; cur != nexus_list.end() ; ++ cur) {

	    nexus_data*curp = *cur;
	    fprintf(fnet, "%s", curp->name.c_str());
	    for (set<string>::const_iterator cp = curp->pins.begin()
		       ; cp != curp->pins.end() ; ++ cp) {
		  fprintf(fnet, " %s", cp->c_str());
	    }
	    fprintf(fnet, "\n");
      }

      fclose(fnet);
}
