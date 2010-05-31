/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "util.h"
# include  "PExpr.h"
# include  "netlist.h"
# include  <iostream>
# include  <cassert>

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
      for (iter_t cur = att.begin() ;  cur != att.end() ;  cur ++, idx++) {
	    table[idx].key = (*cur).first;
	    PExpr*exp = (*cur).second;

	      /* If the attribute value is given in the source, then
		 evaluate it as a constant. If the value is not
		 given, then assume the value is 1. */
	    verinum*tmp = 0;
	    if (exp) {
		  tmp = exp->eval_const(des, scope);
                  if (tmp == 0) {
			cerr << exp->get_fileline() << ": error: ``"
			     << *exp << "'' is not a constant expression."
			     << endl;
			des->errors += 1;
                  }
            }
	    if (tmp == 0)
		  tmp = new verinum(1);

	    assert(tmp);

	    table[idx].val = *tmp;
	    delete tmp;
      }

      assert(idx == natt);
      return table;
}
