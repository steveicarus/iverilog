/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: eval.cc,v 1.4 1999/07/17 19:51:00 steve Exp $"
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
	  case '-': {
		long lv = l->as_long();
		long rv = r->as_long();
		res = new verinum(lv-rv, l->len());
		break;
	  }
	  default:
	    delete l;
	    delete r;
	    return 0;
      }

      return res;
}

verinum* PEIdent::eval_const(const Design*des, const string&path) const
{
      assert(msb_ == 0);
      const NetExpr*expr = des->get_parameter(path + "." + text_);
      if (expr == 0) return 0;
      const NetEConst*eval = dynamic_cast<const NetEConst*>(expr);
      assert(eval);
      return new verinum(eval->value());
}

verinum* PENumber::eval_const(const Design*, const string&) const
{
      return new verinum(value());
}

verinum* PETernary::eval_const(const Design*, const string&) const
{
      assert(0);
}


/*
 * $Log: eval.cc,v $
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

