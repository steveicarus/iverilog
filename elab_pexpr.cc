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
#ifdef HAVE_CVS_IDENT
#ident "$Id: elab_pexpr.cc,v 1.21 2004/02/20 06:22:56 steve Exp $"
#endif

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

      NetEBinary*tmp = elaborate_expr_base_(des, lp, rp);
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

/*
 * $Log: elab_pexpr.cc,v $
 * Revision 1.21  2004/02/20 06:22:56  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.20  2003/05/30 02:55:32  steve
 *  Support parameters in real expressions and
 *  as real expressions, and fix multiply and
 *  divide with real results.
 *
 * Revision 1.19  2003/01/27 05:09:17  steve
 *  Spelling fixes.
 *
 * Revision 1.18  2002/12/05 02:14:33  steve
 *  Support bit select in constant expressions.
 *
 * Revision 1.17  2002/11/09 01:40:19  steve
 *  Postpone parameter width check to evaluation.
 *
 * Revision 1.16  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.15  2002/05/06 02:30:27  steve
 *  Allow parameters in concatenation of widths are defined.
 *
 * Revision 1.14  2002/05/05 21:11:50  steve
 *  Put off evaluation of concatenation repeat expresions
 *  until after parameters are defined. This allows parms
 *  to be used in repeat expresions.
 *
 *  Add the builtin $signed system function.
 *
 * Revision 1.13  2002/01/28 00:52:41  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.12  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.11  2001/11/07 04:01:59  steve
 *  eval_const uses scope instead of a string path.
 *
 * Revision 1.10  2001/10/07 03:38:08  steve
 *  parameter names do not have defined size.
 *
 * Revision 1.9  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
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

