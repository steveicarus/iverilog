/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: pad_to_width.cc,v 1.12 2003/01/26 21:15:59 steve Exp $"
#endif

# include "config.h"

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
      if (wid <= expr->expr_width())
	    return expr;

	/* If the expression is a const, then replace it with a wider
	   const. This is a more efficient result. */
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(expr)) {
	    verinum eval = tmp->value();
	    bool signed_flag = eval.has_sign();

	    verinum::V pad = verinum::V0;
	    if (signed_flag)
		  pad = eval.get(eval.len()-1);
	    verinum oval (pad, wid, eval.has_len());

	    for (unsigned idx = 0 ;  idx < eval.len() ;  idx += 1)
		  oval.set(idx, eval.get(idx));

	    oval.has_sign(signed_flag);
	    tmp = new NetEConst(oval);
	    delete expr;
	    return tmp;
      }

	/* Do it the hard way, with a concatenation. */
      verinum pad(verinum::V0, wid - expr->expr_width());
      NetEConst*co = new NetEConst(pad);
      co->set_line(*expr);
      NetEConcat*cc = new NetEConcat(2);
      cc->set_line(*expr);
      cc->set(0, co);
      cc->set(1, expr);
      cc->set_width(wid);
      expr = cc;

      return expr;
}

NetNet*pad_to_width(Design*des, NetNet*net, unsigned wid)
{
      NetScope*scope = net->scope();
      const string path = scope->name();
      assert(scope);

      if (net->pin_count() >= wid)
	    return net;

      verinum pad(verinum::V0, wid - net->pin_count());
      NetConst*con = new NetConst(scope,
				  path + "." + scope->local_symbol(),
				  pad);
      des->add_node(con);

      NetNet*tmp = new NetNet(scope, path + "." + scope->local_symbol(),
			      NetNet::WIRE, wid);
      tmp->local_flag(true);

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1)
	    connect(tmp->pin(idx), net->pin(idx));
      for (unsigned idx = net->pin_count() ;  idx < wid ;  idx += 1)
	    connect(tmp->pin(idx), con->pin(idx-net->pin_count()));

      return tmp;
}

/*
 * $Log: pad_to_width.cc,v $
 * Revision 1.12  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.11  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.10  2002/05/25 16:43:22  steve
 *  Better padding of constants.
 *
 * Revision 1.9  2001/10/28 01:14:53  steve
 *  NetObj constructor finally requires a scope.
 *
 * Revision 1.8  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.7  2001/02/16 03:25:09  steve
 *  Missing . in names generated from scope locals.
 *
 * Revision 1.6  2001/02/15 06:59:36  steve
 *  FreeBSD port has a maintainer now.
 *
 * Revision 1.5  2000/05/02 00:58:12  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.4  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.3  2000/02/16 03:58:27  steve
 *  Fix up width matching in structural bitwise operators.
 *
 * Revision 1.2  2000/01/01 06:17:25  steve
 *  Propogate line number information when expanding expressions.
 *
 * Revision 1.1  1999/09/29 00:42:51  steve
 *  Allow expanding of additive operators.
 *
 */

