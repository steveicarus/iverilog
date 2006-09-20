/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: eval.cc,v 1.36.2.3 2006/09/20 20:27:02 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

# include  "PExpr.h"
# include  "netlist.h"
# include  "compiler.h"

verinum* PExpr::eval_const(const Design*, const NetScope*) const
{
      return 0;
}

verinum* PEBinary::eval_const(const Design*des, const NetScope*scope) const
{
      verinum*l = left_->eval_const(des, scope);
      if (l == 0) return 0;
      verinum*r = right_->eval_const(des, scope);
      if (r == 0) {
	    delete l;
	    return 0;
      }
      verinum*res;

      switch (op_) {
	  case '+': {
		if (l->is_defined() && r->is_defined()) {
		      res = new verinum(*l + *r);
		} else {
		      res = new verinum(verinum::Vx, l->len());
		}
		break;
	  }
	  case '-': {
		if (l->is_defined() && r->is_defined()) {
		      res = new verinum(*l - *r);
		} else {
		      res = new verinum(verinum::Vx, l->len());
		}
		break;
	  }
	  case '*': {
		if (l->is_defined() && r->is_defined()) {
		      res = new verinum(*l * *r);
		} else {
		      res = new verinum(verinum::Vx, l->len());
		}
		break;
	  }
	  case '/': {
		if (l->is_defined() && r->is_defined()) {
		      long lv = l->as_long();
		      long rv = r->as_long();
		      res = new verinum(lv / rv, l->len());
		} else {
		      res = new verinum(verinum::Vx, l->len());
		}
		break;
	  }
	  case '%': {
		if (l->is_defined() && r->is_defined()) {
		      long lv = l->as_long();
		      long rv = r->as_long();
		      res = new verinum(lv % rv, l->len());
		} else {
		      res = new verinum(verinum::Vx, l->len());
		}
		break;
	  }
	  case '>': {
		if (l->is_defined() && r->is_defined()) {
		      long lv = l->as_long();
		      long rv = r->as_long();
		      res = new verinum(lv > rv, l->len());
		} else {
		      res = new verinum(verinum::Vx, l->len());
		}
		break;
	  }
	  case '<': {
		if (l->is_defined() && r->is_defined()) {
		      long lv = l->as_long();
		      long rv = r->as_long();
		      res = new verinum(lv < rv, l->len());
		} else {
		      res = new verinum(verinum::Vx, l->len());
		}
		break;
	  }
	  case 'l': { // left shift (<<)
		assert(r->is_defined());
		unsigned long rv = r->as_ulong();
		unsigned use_wid = l->len();
		if (! l->has_len())
		      use_wid += rv;
		res = new verinum(verinum::V0, use_wid);
		if (rv < res->len()) {
		      unsigned cnt = res->len() - rv;
		      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
			    res->set(idx+rv, l->get(idx));
		}
		break;
	  }
	  case 'r': { // right shift (>>)
		assert(r->is_defined());
		unsigned long rv = r->as_ulong();
		res = new verinum(verinum::V0, l->len());
		if (rv < res->len()) {
		      unsigned cnt = res->len() - rv;
		      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
			    res->set(idx, l->get(idx+rv));
		}
		break;
	  }

	  default:
	    delete l;
	    delete r;
	    return 0;
      }

      delete l;
      delete r;
      return res;
}

verinum* PEConcat::eval_const(const Design*des, const NetScope*scope) const
{
      verinum*accum = parms_[0]->eval_const(des, scope);
      if (accum == 0)
	    return 0;

      for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
	    verinum*tmp = parms_[idx]->eval_const(des, scope);
	      /* Oops, found an argument that is not constant. Give up. */
	    if (tmp == 0) {
		  delete accum;
		  return 0;
	    }
	    assert(tmp);

	    *accum = concat(*accum, *tmp);
	    delete tmp;
      }

      return accum;
}

/*
 * Evaluate an identifier as a constant expression. This is only
 * possible if the identifier is that of a parameter.
 */
verinum* PEIdent::eval_const(const Design*des, const NetScope*scope) const
{
      assert(scope);
      const NetExpr*expr = des->find_parameter(scope, path_);

      if (expr == 0)
	    return 0;

      const NetEConst*eval = dynamic_cast<const NetEConst*>(expr);
      if (eval == 0) {
	    cerr << get_line() << ": internal error: Unable to evaluate "
		 << "constant expression (parameter=" << path_
		 << "): " << *expr << endl;
	    return 0;
      }

      assert(eval);

      if (msb_ || lsb_)
	    return 0;


      return new verinum(eval->value());
}

verinum* PEFNumber::eval_const(const Design*, const NetScope*) const
{
      long val = value_->as_long();
      return new verinum(val);
}

verinum* PENumber::eval_const(const Design*, const NetScope*) const
{
      return new verinum(value());
}

verinum* PEString::eval_const(const Design*, const NetScope*) const
{
      return new verinum(string(text_));
}

verinum* PETernary::eval_const(const Design*des, const NetScope*scope) const
{
      verinum*test = expr_->eval_const(des, scope);
      if (test == 0)
	    return 0;

      verinum::V bit = test->get(0);
      delete test;
      switch (bit) {
	  case verinum::V0:
	    return fal_->eval_const(des, scope);
	  case verinum::V1:
	    return tru_->eval_const(des, scope);
	  default:
	    return 0;
	      // XXXX It is possible to handle this case if both fal_
	      // and tru_ are constant. Someday...
      }
}

verinum* PEUnary::eval_const(const Design*des, const NetScope*scope) const
{
      verinum*val = expr_->eval_const(des, scope);
      if (val == 0)
	    return 0;

      switch (op_) {
	  case '+':
	    return val;

	  case '-': {
		  /* We need to expand the value a bit if we are
		     taking the 2's complement so that we are
		     guaranteed to not overflow. */
		verinum tmp (0UL, val->len()+1);
		for (unsigned idx = 0 ;  idx < val->len() ;  idx += 1)
		      tmp.set(idx, val->get(idx));

		*val = v_not(tmp) + verinum(verinum::V1, 1);
		val->has_sign(true);
		return val;
	  }

      }
	    delete val;
      return 0;
}


/*
 * $Log: eval.cc,v $
 * Revision 1.36.2.3  2006/09/20 20:27:02  steve
 *  Fix left shift of small unsized constants.
 *
 * Revision 1.36.2.2  2005/12/18 21:06:01  steve
 *  Properly fail when concat is not actually constant.
 *
 * Revision 1.36.2.1  2005/12/07 03:28:44  steve
 *  Support constant concatenation of constants.
 *
 * Revision 1.36  2003/06/21 01:21:43  steve
 *  Harmless fixup of warnings.
 *
 * Revision 1.35  2003/04/14 03:40:21  steve
 *  Make some effort to preserve bits while
 *  operating on constant values.
 *
 * Revision 1.34  2003/03/26 06:16:18  steve
 *  Evaluate > and < in constant expressions.
 *
 * Revision 1.33  2003/03/10 23:40:53  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.32  2002/10/19 22:59:49  steve
 *  Redo the parameter vector support to allow
 *  parameter names in range expressions.
 *
 * Revision 1.31  2002/10/13 05:01:07  steve
 *  More verbose eval_const assert message.
 *
 * Revision 1.30  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.29  2002/06/07 02:57:54  steve
 *  Simply give up on constants with indices.
 *
 * Revision 1.28  2002/06/06 18:57:04  steve
 *  Better error for identifier index eval.
 *
 * Revision 1.27  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.26  2001/12/29 22:10:10  steve
 *  constant eval of arithmetic with x and z.
 *
 * Revision 1.25  2001/12/29 00:43:55  steve
 *  Evaluate constant right shifts.
 *
 * Revision 1.24  2001/12/03 04:47:15  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.23  2001/11/07 04:01:59  steve
 *  eval_const uses scope instead of a string path.
 *
 * Revision 1.22  2001/11/06 06:11:55  steve
 *  Support more real arithmetic in delay constants.
 *
 * Revision 1.21  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.20  2001/02/09 02:49:59  steve
 *  Be more clear about scope of failure.
 *
 * Revision 1.19  2001/01/27 05:41:48  steve
 *  Fix sign extension of evaluated constants. (PR#91)
 *
 * Revision 1.18  2001/01/14 23:04:56  steve
 *  Generalize the evaluation of floating point delays, and
 *  get it working with delay assignment statements.
 *
 *  Allow parameters to be referenced by hierarchical name.
 *
 * Revision 1.17  2001/01/04 04:47:51  steve
 *  Add support for << is signal indices.
 *
 * Revision 1.16  2000/12/10 22:01:36  steve
 *  Support decimal constants in behavioral delays.
 *
 * Revision 1.15  2000/09/07 22:38:13  steve
 *  Support unary + and - in constants.
 */

