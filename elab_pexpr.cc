/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: elab_pexpr.cc,v 1.1 2000/03/08 04:36:53 steve Exp $"
#endif

# include  "PExpr.h"

NetExpr*PExpr::elaborate_pexpr(Design*des, NetScope*sc) const
{
      cerr << get_line() << ": error: invalid parameter expression: "
	   << *this << endl;
      des->errors += 1;

      return 0;
}

/*
 * Simple numbers can be elaborated by the elaborate_expr method.
 */
NetExpr*PENumber::elaborate_pexpr(Design*des, NetScope*sc) const
{
      return elaborate_expr(des, sc);
}


/*
 * $Log: elab_pexpr.cc,v $
 * Revision 1.1  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 */

