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

# include "version_base.h"
# include "version_tag.h"
# include "pcb_config.h"
# include "pcb_priv.h"
# include "fp_api.h"
# include <iostream>
# include <cassert>

using namespace std;

map<string,fp_element_t> footprints;

static int check_footprint(element_data_t*elem);

/*
* Scan the element list and collect footprints needed.
*/
int load_footprints(void)
{
      for (map<string,element_data_t*>::const_iterator cur = element_list.begin()
		 ; cur != element_list.end() ; ++ cur) {
	    check_footprint(cur->second);
      }

      return 0;
}

/*
 * The fpparse function calls back the callback_fp_element function
 * for each Element that it parses. The check_footprint function
 * stores in the cur_footprint variable the name of the footprint that
 * we are trying to find in the file. The callback uses that name to
 * store the Element into the footprints map.
 */
static string cur_footprint = "";
void callback_fp_element(const struct fp_element_t&cur_elem)
{
      assert(cur_footprint != "");
      footprints[cur_footprint] = cur_elem;
      cur_footprint = "";
}


static int check_footprint(element_data_t*elem)
{
      if (elem->footprint == "") {
	    cerr << "No footprint defined for \"" << elem->description << "\"." << endl;
	    return -1;
      }

      map<string,fp_element_t>::iterator match = footprints.find(elem->footprint);
      if (match != footprints.end())
	    return 0;

      string fpname = elem->footprint + ".fp";

      cur_footprint = elem->footprint;
      int rc = parse_fp_file(fpname);
      if (rc != 0) {
	    cerr << "parse_fp_file(" << fpname << ") returns rc=" << rc << endl;
	    return rc;
      }

      match = footprints.find(elem->footprint);
      if (match == footprints.end()) {
	    cerr << "Unable to locate footprint " << elem->footprint << "." << endl;
	    return -2;
      }

      return 0;
}
