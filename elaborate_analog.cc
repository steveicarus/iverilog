/*
 * Copyright (c) 2008-2021 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "AStatement.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "util.h"

# include  <typeinfo>

using namespace std;

NetProc* AContrib::elaborate(Design*des, NetScope*scope) const
{
      NetExpr*lval = elab_and_eval(des, scope, lval_, -1);
      NetExpr*rval = elab_and_eval(des, scope, rval_, -1);

      NetEAccess*lacc = dynamic_cast<NetEAccess*> (lval);
      if (lacc == 0) {
	    cerr << get_fileline() << ": error: The l-value of a contribution"
		 << " statement must be a branch probe access function." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetContribution*st = new NetContribution(lacc, rval);
      st->set_line(*this);
      return st;
}

bool AProcess::elaborate(Design*des, NetScope*scope) const
{
      NetProc*estatement = statement_->elaborate(des, scope);
      if (estatement == 0)
	    return false;

      NetAnalogTop*top = new NetAnalogTop(scope, type_, estatement);

	// Evaluate the attributes for this process, if there
	// are any. These attributes are to be attached to the
	// NetProcTop object.
      struct attrib_list_t*attrib_list;
      unsigned attrib_list_n = 0;
      attrib_list = evaluate_attributes(attributes, attrib_list_n, des, scope);

      for (unsigned adx = 0 ;  adx < attrib_list_n ;  adx += 1)
	    top->attribute(attrib_list[adx].key,
			   attrib_list[adx].val);

      delete[]attrib_list;

      top->set_line(*this);
      des->add_process(top);

      return true;
}
