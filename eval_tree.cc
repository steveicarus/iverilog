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
#ident "$Id: eval_tree.cc,v 1.1 1999/09/20 02:21:10 steve Exp $"
#endif

# include  "netlist.h"

NetExpr* NetExpr::eval_tree()
{
      return 0;
}
/*

 * Some of the derived classes can be evaluated by the compiler, this
 * method provides the common aid of evaluating the parameter
 * expressions.
 */
void NetEBinary::eval_sub_tree_()
{
      NetExpr*tmp = left_->eval_tree();
      if (tmp) {
	    delete left_;
	    left_ = tmp;
      }
      tmp = right_->eval_tree();
      if (tmp){
	    delete right_;
	    right_ = tmp;
      }
}


NetExpr* NetEBComp::eval_eqeq_()
{
      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      const verinum&lv = l->value();
      const verinum&rv = r->value();

      if (lv.len() < rv.len())
	    return 0;

      verinum result(verinum::V1, 1);
      for (unsigned idx = 0 ; idx < lv.len(); idx += 1) {
	    if (lv[idx] != rv[idx])
		  result = verinum::V0;
      }

      return new NetEConst(result);
}


NetExpr* NetEBComp::eval_leeq_()
{
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      verinum rv = r->value();

	/* Detect the case where the right side is greater that or
	   equal to the largest value the left side can possibly
	   have. */
      unsigned long lv = (1 << left_->expr_width()) - 1;
      if (lv <= rv.as_ulong()) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      }

      return 0;
}

      
NetExpr* NetEBComp::eval_tree()
{
      eval_sub_tree_();

      switch (op_) {
	  case 'e':
	    return eval_eqeq_();

	  case 'L':
	    return eval_leeq_();

	  default:
	    return 0;
      }
}

NetExpr* NetEConcat::eval_tree()
{
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	      // If this parameter is already a constant, go on.
	    if (dynamic_cast<NetEConst*>(parms_[idx]))
		  continue;

	    assert(parms_[idx]);
	    NetExpr*expr = parms_[idx]->eval_tree();
	    if (expr) {
		  delete parms_[idx];
		  parms_[idx] = expr;
	    }
      }

      unsigned gap = expr_width() / repeat_;
      verinum val (verinum::Vx, repeat_ * gap);


	// build up the result from least significant to most.

      unsigned cur = 0;
      for (unsigned idx = parms_.count() ;  idx > 0 ;  idx -= 1) {
	    NetEConst*expr = dynamic_cast<NetEConst*>(parms_[idx-1]);
	    if (expr == 0)
		  return 0;

	    verinum tmp = expr->value();
	    for (unsigned bit = 0 ;  bit < tmp.len() ;  bit += 1, cur += 1)
		  for (unsigned rep = 0 ;  rep < repeat_ ;  rep += 1)
			val.set(rep*gap+cur, tmp[bit]);
      }

      NetEConst*res = new NetEConst(val);
      res->set_width(val.len());
      return res;
}

/*
 * $Log: eval_tree.cc,v $
 * Revision 1.1  1999/09/20 02:21:10  steve
 *  Elaborate parameters in phases.
 *
 */

