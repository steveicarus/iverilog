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
#ident "$Id: elab_pexpr.cc,v 1.3 2000/03/12 18:22:11 steve Exp $"
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
 * Binary operators have sub-expressions that must be elaborated as
 * parameter expressions. If either of them fail, then give up. Once
 * they are taken care of, make the base object just as in any other
 * expression.
 */
NetExpr*PEBinary::elaborate_pexpr (Design*des, NetScope*scope) const
{
      NetExpr*lp = left_->elaborate_pexpr(des, scope);
      NetExpr*rp = right_->elaborate_pexpr(des, scope);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      NetEBinary*tmp = elaborate_expr_base_(des, lp, rp);
      return tmp;
}

/*
 * Parameter expressions may reference other parameters, but only in
 * the current scope. Preserve the parameter reference in the
 * parameter expression I'm generating, instead of evaluating it now,
 * because the referenced parameter may yet be overridden.
 */
NetExpr*PEIdent::elaborate_pexpr(Design*des, NetScope*scope) const
{
      const NetExpr*ex = scope->get_parameter(text_);
      if (ex == 0) {
	    cerr << get_line() << ": error: identifier ``" << text_ <<
		  "'' is not a parameter in " << scope->name() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetExpr*res = new NetEParam(des, scope, text_);
      assert(res);
      return res;
}

/*
 * Simple numbers can be elaborated by the elaborate_expr method.
 */
NetExpr*PENumber::elaborate_pexpr(Design*des, NetScope*sc) const
{
      return elaborate_expr(des, sc);
}


NetExpr*PEUnary::elaborate_pexpr (Design*des, NetScope*scope) const
{
      NetExpr*ip = expr_->elaborate_pexpr(des, scope);
      if (ip == 0) return 0;

      /* Should we evaluate expressions ahead of time,
       * just like in PEBinary::elaborate_expr() ?
       */

      NetEUnary*tmp;
      switch (op_) {
	  default:
	    tmp = new NetEUnary(op_, ip);
	    tmp->set_line(*this);
	    break;
	  case '~':
	    tmp = new NetEUBits(op_, ip);
	    tmp->set_line(*this);
	    break;
      }
      return tmp;
}

/*
 * $Log: elab_pexpr.cc,v $
 * Revision 1.3  2000/03/12 18:22:11  steve
 *  Binary and unary operators in parameter expressions.
 *
 * Revision 1.2  2000/03/12 04:35:22  steve
 *  Allow parameter identifiers in parameter expressions.
 *
 * Revision 1.1  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 */

