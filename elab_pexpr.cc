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
#ident "$Id: elab_pexpr.cc,v 1.8 2001/01/14 23:04:56 steve Exp $"
#endif

# include  "PExpr.h"
# include  "util.h"

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
 * Event though parameters are not generally sized, parameter
 * expressions can include concatenation expressions. This requires
 * that the subexpressions all have well-defined size (in spite of
 * being in a parameter expression) in order to get a defined
 * value. The sub-expressions themsilves must also be value parameter
 * expressions.
 */
NetEConcat* PEConcat::elaborate_pexpr(Design*des, NetScope*scope) const
{
      unsigned repeat = 1;

	/* If there is a repeat expression, then evaluate the constant
	   value and set the repeat count.

	   XXXX Potential bug XXX In principle, the repeat expression
	   can have a parameter name in it. Since where are in the
	   working of parameters now, we will not be able to
	   accurately evaluate such expressions. So eventually, I will
	   need to be able to defer the evaluation of the expression. */
      if (repeat_) {
	    verinum*vrep = repeat_->eval_const(des, scope->name());
	    if (vrep == 0) {
		  cerr << get_line() << ": error: "
			"concatenation repeat expression cannot be evaluated."
		       << endl;
		  des->errors += 1;
		  return 0;
	    }

	    repeat = vrep->as_ulong();
	    delete vrep;
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
      string path = text_;
      string name = parse_last_name(path);

      NetScope*pscope = scope;
      if (path != "")
	    pscope = des->find_scope(scope, path);

      assert(pscope);

      const NetExpr*ex = pscope->get_parameter(name);
      if (ex == 0) {
	    cerr << get_line() << ": error: identifier ``" << text_ <<
		  "'' is not a parameter in " << scope->name() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetExpr*res = new NetEParam(des, pscope, name);
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

/*
 * $Log: elab_pexpr.cc,v $
 * Revision 1.8  2001/01/14 23:04:56  steve
 *  Generalize the evaluation of floating point delays, and
 *  get it working with delay assignment statements.
 *
 *  Allow parameters to be referenced by hierarchical name.
 *
 * Revision 1.7  2001/01/02 04:21:13  steve
 *  Support a bunch of unary operators in parameter expressions.
 *
 * Revision 1.6  2000/12/16 19:03:30  steve
 *  Evaluate <= and ?: in parameter expressions (PR#81)
 *
 * Revision 1.5  2000/06/13 05:22:16  steve
 *  Support concatenation in parameter expressions.
 *
 * Revision 1.4  2000/06/01 02:31:39  steve
 *  Parameters can be strings.
 *
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

