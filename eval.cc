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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: eval.cc,v 1.15 2000/09/07 22:38:13 steve Exp $"
#endif

# include  "PExpr.h"
# include  "netlist.h"

verinum* PExpr::eval_const(const Design*, const string&) const
{
      return 0;
}

verinum* PEBinary::eval_const(const Design*des, const string&path) const
{
      verinum*l = left_->eval_const(des, path);
      if (l == 0) return 0;
      verinum*r = right_->eval_const(des, path);
      if (r == 0) {
	    delete l;
	    return 0;
      }

      verinum*res;

      switch (op_) {
	  case '+': {
		assert(l->is_defined());
		assert(r->is_defined());
		long lv = l->as_long();
		long rv = r->as_long();
		res = new verinum(lv+rv, l->len());
		break;
	  }
	  case '-': {
		assert(l->is_defined());
		assert(r->is_defined());
		long lv = l->as_long();
		long rv = r->as_long();
		res = new verinum(lv-rv, l->len());
		break;
	  }
	  case '*': {
		assert(l->is_defined());
		assert(r->is_defined());
		long lv = l->as_long();
		long rv = r->as_long();
		res = new verinum(lv * rv, l->len());
		break;
	  }
	  case '/': {
		assert(l->is_defined());
		assert(r->is_defined());
		long lv = l->as_long();
		long rv = r->as_long();
		res = new verinum(lv / rv, l->len());
		break;
	  }
	  case '%': {
		assert(l->is_defined());
		assert(r->is_defined());
		long lv = l->as_long();
		long rv = r->as_long();
		res = new verinum(lv % rv, l->len());
		break;
	  }
	  default:
	    delete l;
	    delete r;
	    return 0;
      }

      return res;
}

/*
 * Evaluate an identifier as a constant expression. This is only
 * possible if the identifier is that of a parameter.
 */
verinum* PEIdent::eval_const(const Design*des, const string&path) const
{
      const NetScope*scope = des->find_scope(path);
      assert(scope);
      const NetExpr*expr = des->find_parameter(scope, text_);

      if (expr == 0)
	    return 0;

      assert(msb_ == 0);

      if (dynamic_cast<const NetEParam*>(expr)) {
	    cerr << get_line() << ": sorry: I cannot evaluate ``" <<
		  text_ << "'' in this context." << endl;
	    return 0;
      }

      const NetEConst*eval = dynamic_cast<const NetEConst*>(expr);
      assert(eval);
      return new verinum(eval->value());
}

verinum* PENumber::eval_const(const Design*, const string&) const
{
      return new verinum(value());
}

verinum* PETernary::eval_const(const Design*des, const string&path) const
{
      verinum*test = expr_->eval_const(des, path);
      if (test == 0)
	    return 0;

      verinum::V bit = test->get(0);
      delete test;
      switch (bit) {
	  case verinum::V0:
	    return fal_->eval_const(des, path);
	  case verinum::V1:
	    return tru_->eval_const(des, path);
	  default:
	    return 0;
	      // XXXX It is possible to handle this case if both fal_
	      // and tru_ are constant. Someday...
      }
}

verinum* PEUnary::eval_const(const Design*des, const string&path) const
{
      verinum*val = expr_->eval_const(des, path);
      if (val == 0)
	    return 0;

      switch (op_) {
	  case '+':
	    return val;

	  case '-':
	    *val = v_not(*val) + verinum(verinum::V1, 1);
	    return val;

      }
	    delete val;
      return 0;
}


/*
 * $Log: eval.cc,v $
 * Revision 1.15  2000/09/07 22:38:13  steve
 *  Support unary + and - in constants.
 *
 * Revision 1.14  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.13  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.12  1999/11/30 04:48:17  steve
 *  Handle evaluation of ternary during elaboration.
 *
 * Revision 1.11  1999/11/28 23:42:02  steve
 *  NetESignal object no longer need to be NetNode
 *  objects. Let them keep a pointer to NetNet objects.
 *
 * Revision 1.10  1999/11/21 20:03:24  steve
 *  Handle multiply in constant expressions.
 *
 * Revision 1.9  1999/10/08 17:48:09  steve
 *  Support + in constant expressions.
 *
 * Revision 1.8  1999/09/20 02:21:10  steve
 *  Elaborate parameters in phases.
 *
 * Revision 1.7  1999/09/18 01:52:48  steve
 *  Remove spurious message.
 *
 * Revision 1.6  1999/09/16 04:18:15  steve
 *  elaborate concatenation repeats.
 *
 * Revision 1.5  1999/08/06 04:05:28  steve
 *  Handle scope of parameters.
 *
 * Revision 1.4  1999/07/17 19:51:00  steve
 *  netlist support for ternary operator.
 *
 * Revision 1.3  1999/05/30 01:11:46  steve
 *  Exressions are trees that can duplicate, and not DAGS.
 *
 * Revision 1.2  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.1  1998/11/03 23:28:58  steve
 *  Introduce verilog to CVS.
 *
 */

