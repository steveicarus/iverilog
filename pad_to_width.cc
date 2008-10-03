/*
 * Copyright (c) 1999-2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: pad_to_width.cc,v 1.20 2005/12/22 15:43:47 steve Exp $"
#endif

# include "config.h"

# include  "netlist.h"
# include  "netmisc.h"


/*
 * This function transforms an expression by padding the high bits
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
	    verinum oval = pad_to_width(tmp->value(), wid);
	    tmp = new NetEConst(oval);
	    delete expr;
	    return tmp;
      }

      NetESelect*tmp = new NetESelect(expr, 0, wid);
      tmp->set_line(*expr);
      tmp->cast_signed(expr->has_sign());
      return tmp;
}

/*
 * Pad a NetNet to the desired vector width by concatenating a
 * NetConst of constant zeros. Use a NetConcat node to do the
 * concatenation.
 */
NetNet*pad_to_width(Design*des, NetNet*net, unsigned wid)
{
      NetScope*scope = net->scope();

      if (net->vector_width() >= wid)
	    return net;

	// Make the NetConcat and connect the input net to the lsb input.
      NetConcat*cc = new NetConcat(scope, scope->local_symbol(), wid, 2);
      des->add_node(cc);
      connect(cc->pin(1), net->pin(0));

	// Make a NetConst of the desired width and connect in to the
	// lsb input of the NetConcat.
      verinum pad(verinum::V0, wid - net->vector_width());
      NetConst*con = new NetConst(scope, scope->local_symbol(), pad);
      des->add_node(con);
      connect(cc->pin(2), con->pin(0));

	// Make a NetNet for the NetConst to NetConcat link.
      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, wid - net->vector_width());
      tmp->data_type( net->data_type() );
      tmp->local_flag(true);
      connect(cc->pin(2), tmp->pin(0));

	// Create a NetNet of the output width and connect it to the
	// NetConcat node output pin.
      tmp = new NetNet(scope, scope->local_symbol(),
		       NetNet::WIRE, wid);
      tmp->data_type( net->data_type() );
      tmp->set_line(*net);
      tmp->local_flag(true);
      connect(cc->pin(0), tmp->pin(0));

      return tmp;
}

NetNet*pad_to_width_signed(Design*des, NetNet*net, unsigned wid)
{
      NetScope*scope = net->scope();

      if (net->vector_width() >= wid)
	    return net;

      NetSignExtend*se
	    = new NetSignExtend(scope, scope->local_symbol(), wid);
      se->set_line(*net);
      des->add_node(se);

      NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, wid);
      tmp->set_line(*net);
      tmp->local_flag(true);
      tmp->data_type(net->data_type());
      tmp->set_signed(true);

      connect(tmp->pin(0), se->pin(0));
      connect(se->pin(1), net->pin(0));

      return tmp;
}

NetNet*crop_to_width(Design*des, NetNet*net, unsigned wid)
{
      NetScope*scope = net->scope();

      if (net->vector_width() <= wid)
	    return net;

      NetPartSelect*ps = new NetPartSelect(net, 0, wid, NetPartSelect::VP);
      des->add_node(ps);
      ps->set_line(*net);

      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, wid);
      tmp->data_type(net->data_type());
      tmp->local_flag(true);
      tmp->set_line(*tmp);
      connect(ps->pin(0), tmp->pin(0));

      return tmp;
}

/*
 * $Log: pad_to_width.cc,v $
 * Revision 1.20  2005/12/22 15:43:47  steve
 *  pad_to_width handles signed expressions.
 *
 * Revision 1.19  2005/07/07 16:22:49  steve
 *  Generalize signals to carry types.
 *
 * Revision 1.18  2005/05/24 01:44:28  steve
 *  Do sign extension of structuran nets.
 *
 * Revision 1.17  2005/04/24 23:44:02  steve
 *  Update DFF support to new data flow.
 *
 * Revision 1.16  2005/01/12 03:17:37  steve
 *  Properly pad vector widths in pgassign.
 *
 * Revision 1.15  2004/02/18 17:11:57  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.14  2003/03/06 00:28:42  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.13  2003/01/27 05:09:17  steve
 *  Spelling fixes.
 *
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
 */

