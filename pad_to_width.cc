/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: pad_to_width.cc,v 1.1 1999/09/29 00:42:51 steve Exp $"
#endif

# include  "netlist.h"
# include  "netmisc.h"

/*
 * This funciton transforms an expression by padding the high bits
 * with V0 until the expression has the desired width. This may mean
 * not transforming the expression at all, if it is already wide
 * enough.
 */
NetExpr*pad_to_width(NetExpr*expr, unsigned wid)
{
      if (wid > expr->expr_width()) {
	    verinum pad(verinum::V0, wid - expr->expr_width());
	    NetEConst*co = new NetEConst(pad);
	    NetEConcat*cc = new NetEConcat(2);
	    cc->set(0, co);
	    cc->set(1, expr);
	    cc->set_width(wid);
	    expr = cc;
      }
      return expr;
}


/*
 * $Log: pad_to_width.cc,v $
 * Revision 1.1  1999/09/29 00:42:51  steve
 *  Allow expanding of additive operators.
 *
 */

