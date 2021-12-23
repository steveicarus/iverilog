/*
 * Copyright (c) 2002-2021 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "util.h"
# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  <iostream>
# include  <cassert>

using namespace std;

/*
 * The evaluate_attributes function evaluates the attribute
 * expressions from the map, and returns a table in a form suitable
 * for passing to netlist devices.
 */

attrib_list_t* evaluate_attributes(const map<perm_string,PExpr*>&att,
				   unsigned&natt,
				   Design*des, NetScope*scope)
{
      natt = att.size();
      if (natt == 0)
	    return 0;

      attrib_list_t*table = new attrib_list_t [natt];

      unsigned idx = 0;

      typedef map<perm_string,PExpr*>::const_iterator iter_t;
      for (iter_t cur = att.begin() ;  cur != att.end() ;  ++ cur , idx += 1) {
	    table[idx].key = (*cur).first;
	    PExpr*exp = (*cur).second;

	      /* If the attribute value is given in the source, then
		 evaluate it as a constant. If the value is not
		 given, then assume the value is 1. */
	    if (exp) {
		  NetExpr *tmp = elab_and_eval(des, scope, exp, -1, true);
		  if (!tmp)
			continue;

		  if (NetEConst *ce = dynamic_cast<NetEConst*>(tmp)) {
			table[idx].val = ce->value();
		  } else if (NetECReal *cer = dynamic_cast<NetECReal*>(tmp)) {
			table[idx].val = verinum(cer->value().as_long());
		  } else {
			cerr << exp->get_fileline() << ": error: ``"
			     << *exp << "'' is not a constant expression."
			     << endl;
			des->errors += 1;
		  }
		  delete tmp;
	    } else {
		  table[idx].val = verinum(1);
	    }
      }

      assert(idx == natt);
      return table;
}
