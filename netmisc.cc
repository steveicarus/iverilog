/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: netmisc.cc,v 1.3 2002/08/12 01:35:00 steve Exp $"
#endif

# include "config.h"

# include  "netlist.h"
# include  "netmisc.h"
# include  "PExpr.h"


NetExpr* elab_and_eval(Design*des, NetScope*scope, const PExpr*pe)
{
      NetExpr*tmp = pe->elaborate_expr(des, scope);
      if (tmp == 0)
	    return 0;

      if (NetExpr*tmp2 = tmp->eval_tree()) {
	    delete tmp;
	    tmp = tmp2;
      }

      return tmp;
}


/*
 * $Log: netmisc.cc,v $
 * Revision 1.3  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2001/02/11 02:15:52  steve
 *  Add the netmisc.cc source file.
 *
 */

