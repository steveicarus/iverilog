/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: eval_attrib.cc,v 1.1 2002/05/23 03:08:51 steve Exp $"
#endif

# include  "util.h"
# include  "PExpr.h"
# include  "netlist.h"
# include  <assert.h>

/*
 * The evaluate_attributes function evaluates the attribute
 * expressions from the map, and resturns a table in a form suitable
 * for passing to netlist devices.
 */

attrib_list_t* evaluate_attributes(const map<string,PExpr*>&att,
				   unsigned&natt,
				   const Design*des,
				   const NetScope*scope)
{
      natt = att.size();
      if (natt == 0)
	    return 0;

      attrib_list_t*table = new attrib_list_t [natt];

      unsigned idx = 0;

      typedef map<string,PExpr*>::const_iterator iter_t;
      for (iter_t cur = att.begin() ;  cur != att.end() ;  cur ++, idx++) {
	    table[idx].key = (*cur).first;
	    PExpr*exp = (*cur).second;

	    verinum*tmp;
	    if (exp)
		  tmp = exp->eval_const(des, scope);
	    else
		  tmp = new verinum();

	    if (tmp == 0)
		  cerr << "internal error: no result for " << *exp << endl;
	    assert(tmp);

	    table[idx].val = *tmp;
	    delete tmp;
      }

      assert(idx == natt);
      return table;
}

/*
 * $Log: eval_attrib.cc,v $
 * Revision 1.1  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 */

