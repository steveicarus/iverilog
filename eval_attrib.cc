/*
 * Copyright (c) 2002-2004 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: eval_attrib.cc,v 1.7 2004/02/20 18:53:35 steve Exp $"
#endif

# include  "config.h"
# include  "util.h"
# include  "PExpr.h"
# include  "netlist.h"
# include  <iostream>
# include  <assert.h>

/*
 * The evaluate_attributes function evaluates the attribute
 * expressions from the map, and returns a table in a form suitable
 * for passing to netlist devices.
 */

attrib_list_t* evaluate_attributes(const map<perm_string,PExpr*>&att,
				   unsigned&natt,
				   const Design*des,
				   const NetScope*scope)
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
	    verinum*tmp;
	    if (exp)
		  tmp = exp->eval_const(des, scope);
	    else
		  tmp = new verinum(1);

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
 * Revision 1.7  2004/02/20 18:53:35  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.6  2003/01/27 05:09:17  steve
 *  Spelling fixes.
 *
 * Revision 1.5  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2002/08/10 21:59:39  steve
 *  The default attribute value is 1.
 *
 * Revision 1.3  2002/06/06 18:57:18  steve
 *  Use standard name for iostream.
 *
 * Revision 1.2  2002/06/03 03:55:14  steve
 *  compile warnings.
 *
 * Revision 1.1  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 */

