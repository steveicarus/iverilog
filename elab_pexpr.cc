/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "PExpr.h"
# include  "compiler.h"
# include  "util.h"

# include  <iostream>

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

      NetExpr*tmp = elaborate_expr_base_(des, lp, rp);
      return tmp;
}

/*
 * Event though parameters are not generally sized, parameter
 * expressions can include concatenation expressions. This requires
 * that the subexpressions all have well-defined size (in spite of
 * being in a parameter expression) in order to get a defined
 * value. The sub-expressions themselves must also be value parameter
 * expressions.
 */
NetEConcat* PEConcat::elaborate_pexpr(Design*des, NetScope*scope) const
{
      NetExpr* repeat = 0;

	/* If there is a repeat expression, then evaluate the constant
	   value and set the repeat count.  */
      if (repeat_) {
	    repeat = repeat_->elaborate_pexpr(des, scope);
	    if (repeat == 0) {
		  cerr << get_line() << ": error: "
			"concatenation repeat expression cannot be evaluated."
		       << endl;
		  des->errors += 1;
	    }

	      /* continue on even if the repeat expression doesn't
		 work, as we can find more errors. */
      }

	/* Make the empty concat expression. */
      NetEConcat*tmp = new NetEConcat(parms_.count(), repeat);
      tmp->set_line(*this);

	/* Elaborate all the operands and attach them to the concat
	   node. Use the elaborate_pexpr method instead of the
	   elaborate_expr method. */
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    assert(parms_[idx]);
	    NetExpr*ex = parms_[idx]->elaborate_pexpr(des, scope);
	    if (ex == 0) continue;

	    ex->set_line(*parms_[idx]);

	    if (dynamic_cast<NetEParam*>(ex)) {

		    /* If this parameter is a NetEParam, then put off
		       the width check for later. */

	    } else if (! ex->has_width()) {
		  cerr << ex->get_line() << ": error: operand of "
		       << "concatenation has indefinite width: "
		       << *ex << endl;
		  des->errors += 1;

	    }

	    tmp->set(idx, ex);
      }

      return tmp;
}

NetExpr*PEFNumber::elaborate_pexpr(Design*des, NetScope*scope) const
{
      return elaborate_expr(des, scope);
}

/*
 * Parameter expressions may reference other parameters, but only in
 * the current scope. Preserve the parameter reference in the
 * parameter expression I'm generating, instead of evaluating it now,
 * because the referenced parameter may yet be overridden.
 */
NetExpr*PEIdent::elaborate_pexpr(Design*des, NetScope*scope) const
{
      hname_t path = path_;
      char*name = path.remove_tail_name();

      NetScope*pscope = scope;
      if (path.peek_name(0))
	    pscope = des->find_scope(scope, path);

      perm_string perm_name = lex_strings.make(name);
      delete name;

      const NetExpr*ex = pscope->get_parameter(perm_name);
      if (ex == 0) {
	    cerr << get_line() << ": error: identifier ``" << path_ <<
		  "'' is not a parameter in " << scope->name() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetExpr*res = new NetEParam(des, pscope, perm_name);
      res->set_line(*this);
      assert(res);

      assert(idx_ == 0);
      if (msb_ && lsb_) {
	    cerr << get_line() << ": sorry: Cannot part select "
		  "bits of parameters." << endl;
	    des->errors += 1;

      } else if (msb_) {

	      /* We have here a bit select. Insert a NetESelect node
		 to handle it. */
	    NetExpr*tmp = msb_->elaborate_pexpr(des, scope);
	    if (tmp != 0) {
		  res = new NetESelect(res, tmp, 1);
	    }
      }

      return res;
}

/*
 * Simple numbers can be elaborated by the elaborate_expr method.
 */
NetExpr*PENumber::elaborate_pexpr(Design*des, NetScope*sc) const
{
      return elaborate_expr(des, sc);
}


NetEConst* PEString::elaborate_pexpr(Design*des, NetScope*scope) const
{
      return elaborate_expr(des, scope);
}

NetETernary* PETernary::elaborate_pexpr(Design*des, NetScope*scope) const
{
      NetExpr*c = expr_->elaborate_pexpr(des, scope);
      NetExpr*t = tru_->elaborate_pexpr(des, scope);
      NetExpr*f = fal_->elaborate_pexpr(des, scope);
      if (c == 0) return 0;
      if (t == 0) return 0;
      if (f == 0) return 0;
      return new NetETernary(c, t, f);
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
	  case '!': // Logical NOT
	  case '&': // Reduction AND
	  case '|': // Reduction OR
	  case '^': // Reduction XOR
	  case 'A': // Reduction NAND (~&)
	  case 'N': // Reduction NOR (~|)
	  case 'X': // Reduction NXOR (~^)
	    tmp = new NetEUReduce(op_, ip);
	    tmp->set_line(*this);
	    break;
      }
      return tmp;
}
