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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: eval_tree.cc,v 1.19 2001/01/04 04:28:42 steve Exp $"
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

NetEConst* NetEBAdd::eval_tree()
{
      eval_sub_tree_();
      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      if (lc == 0) return 0;
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (rc == 0) return 0;

      verinum lval = lc->value();
      verinum rval = rc->value();

      verinum val;
      switch (op_) {
	  case '+':
	    val = lval + rval;
	    break;
	  case '-':
	    val = lval - rval;
	    break;
	  default:
	    return 0;
      }

      return new NetEConst(val);
}

NetEConst* NetEBBits::eval_tree()
{
      eval_sub_tree_();

      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      if (lc == 0) return 0;
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (rc == 0) return 0;

      verinum lval = lc->value();
      verinum rval = rc->value();

      unsigned lwid = lc->expr_width();
      if (lwid == 0) lwid = lval.len();

      unsigned rwid = rc->expr_width();
      if (rwid == 0) rwid = rval.len();

      unsigned wid = expr_width();
      if (wid == 0)
	    wid = (rwid > lwid)? rwid : lwid;

      verinum res (verinum::V0, wid);

      if (lwid > wid)
	    lwid = wid;
      if (rwid > wid)
	    rwid = wid;

      switch (op()) {

	  case '|': {
		unsigned cnt = lwid;
		if (cnt > wid)  cnt = wid;
		if (cnt > rwid) cnt = rwid;
		for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
		      res.set(idx, lval.get(idx) | rval.get(idx));

		if (lwid < rwid)
		      for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
			    res.set(idx, lval.get(idx));

		if (rwid < lwid)
		      for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
			    res.set(idx, rval.get(idx));

		break;
	  }

	  case '&': {
		unsigned cnt = lwid;
		if (cnt > wid)  cnt = wid;
		if (cnt > rwid) cnt = rwid;
		for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
		      res.set(idx, lval.get(idx) & rval.get(idx));

		break;
	  }

	  default:
	    return 0;
      }

      return new NetEConst(res);
}

NetEConst* NetEBComp::eval_eqeq_()
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


NetEConst* NetEBComp::eval_leeq_()
{
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      verinum rv = r->value();
      if (! rv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

	/* Detect the case where the right side is greater that or
	   equal to the largest value the left side can possibly
	   have. */
      assert(left_->expr_width() > 0);
      verinum lv (verinum::V1, left_->expr_width());
      if (lv <= rv) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      }

	/* Now go on to the normal test of the values. */
      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;
      lv = l->value();
      if (! lv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (lv.has_sign() && rv.has_sign() && (lv.as_long() <= rv.as_long())) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      }

      if (lv.as_ulong() <= rv.as_ulong()) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      }

      verinum result(verinum::V0, 1);
      return new NetEConst(result);
}

NetEConst* NetEBComp::eval_neeq_()
{
      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      const verinum&lv = l->value();
      const verinum&rv = r->value();

      verinum::V res = verinum::V0;
      unsigned top = lv.len();
      if (rv.len() < top)
	    top = rv.len();

      for (unsigned idx = 0 ;  idx < top ;  idx += 1) {

	    switch (lv.get(idx)) {

		case verinum::Vx:
		case verinum::Vz:
		  res = verinum::Vx;
		  break;

		default:
		  break;
	    }

	    switch (rv.get(idx)) {

		case verinum::Vx:
		case verinum::Vz:
		  res = verinum::Vx;
		  break;

		default:
		  break;
	    }

	    if (res == verinum::Vx)
		  break;

	    if (rv.get(idx) != lv.get(idx))
		  res = verinum::V1;
      }

      if (res != verinum::Vx) {
	    for (unsigned idx = top ;  idx < lv.len() ;  idx += 1)
		  switch (lv.get(idx)) {

		      case verinum::Vx:
		      case verinum::Vz:
			res = verinum::Vx;
			break;

		      case verinum::V1:
			if (res != verinum::Vx)
			      res = verinum::V1;
			break;

		      default:
			break;
		  }

	    for (unsigned idx = top ;  idx < rv.len() ;  idx += 1)
		  switch (rv.get(idx)) {

		      case verinum::Vx:
		      case verinum::Vz:
			res = verinum::Vx;
			break;

		      case verinum::V1:
			if (res != verinum::Vx)
			      res = verinum::V1;
			break;

		      default:
			break;
		  }
      }

      return new NetEConst(verinum(res));
}

NetEConst* NetEBComp::eval_tree()
{
      eval_sub_tree_();

      switch (op_) {
	  case 'e':
	    return eval_eqeq_();

	  case 'L':
	    return eval_leeq_();

	  case 'n':
	    return eval_neeq_();

	  default:
	    return 0;
      }
}

NetEConst* NetEBDiv::eval_tree()
{
      eval_sub_tree_();
      return 0;
}

NetEConst* NetEBLogic::eval_tree()
{
      eval_sub_tree_();
      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      if (lc == 0) return 0;
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (rc == 0) return 0;

      verinum::V lv = verinum::V0;
      verinum::V rv = verinum::V0;

      verinum v = lc->value();
      for (unsigned idx = 0 ;  idx < v.len() ;  idx += 1)
	    if (v.get(idx) == verinum::V1)
		  lv = verinum::V1;

      if (lv == verinum::V0)
	    for (unsigned idx = 0 ;  idx < v.len() ;  idx += 1)
		  if (v.get(idx) != verinum::V0)
			lv = verinum::Vx;

      v = rc->value();
      for (unsigned idx = 0 ;  idx < v.len() ;  idx += 1)
	    if (v.get(idx) == verinum::V1)
		  rv = verinum::V1;

      if (lv == verinum::V0)
	    for (unsigned idx = 0 ;  idx < v.len() ;  idx += 1)
		  if (v.get(idx) != verinum::V0)
			rv = verinum::Vx;

      verinum::V res;
      switch (op_) {
	  case 'a': { // Logical AND (&&)
		if ((lv == verinum::V0) || (rv == verinum::V0))
		      res = verinum::V0;

		else if ((lv == verinum::V1) && (rv == verinum::V1))
		      res = verinum::V1;

		else
		      res = verinum::Vx;

		break;
	  }

	  case 'o': { // Logical OR (||)
		if ((lv == verinum::V1) || (rv == verinum::V1))
		      res = verinum::V1;

		else if ((lv == verinum::V0) && (rv == verinum::V0))
		      res = verinum::V0;

		else
		      res = verinum::Vx;

		break;
	  }

	  default:
	    return 0;
      }

      return new NetEConst(verinum(res, 1));
}

NetEConst* NetEBMult::eval_tree()
{
      eval_sub_tree_();

      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      if (lc == 0) return 0;
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (rc == 0) return 0;

      verinum lval = lc->value();
      verinum rval = rc->value();

      return new NetEConst(lval * rval);
}

/*
 * Evaluate the shift operator if possible. For this to work, both
 * operands must be constant.
 */
NetEConst* NetEBShift::eval_tree()
{
      eval_sub_tree_();
      NetEConst*re = dynamic_cast<NetEConst*>(right_);
      if (re == 0)
	    return 0;

      NetEConst*le = dynamic_cast<NetEConst*>(left_);
      if (le == 0)
	    return 0;

      NetEConst*res;

      verinum rv = re->value();
      verinum lv = le->value();

	/* Calculate the width of the result. If it is not fixed, then
	   get it from the left operand. */
      unsigned wid = expr_width();
      if (wid == 0)
	    wid = left_->expr_width();

      if (rv.is_defined()) {

	    unsigned shift = rv.as_ulong();


	    assert(wid);
	    verinum nv (verinum::V0, wid);

	    if (op() == 'r') {
		  unsigned cnt = wid;
		  if (cnt > nv.len())
			cnt = nv.len();
		  if (shift >= lv.len())
			cnt = 0;
		  else if (cnt > (lv.len()-shift))
			cnt = (lv.len()-shift);
		  for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
			nv.set(idx, lv[idx+shift]);

	    } else {
		  unsigned cnt = wid;
		  if (cnt > lv.len())
			cnt = lv.len();
		  if (shift >= nv.len())
			cnt = 0;
		  else if (cnt > (nv.len()-shift))
			cnt = nv.len() - shift;

		  for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
			nv.set(idx+shift, lv[idx]);
	    }

	    res = new NetEConst(nv);

      } else {
	    assert(wid);
	    verinum nv (verinum::Vx, wid);
	    res = new NetEConst(nv);
      }

      return res;
}

NetEConst* NetEConcat::eval_tree()
{
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {

	      // Parameter not here? This is an error, but presumably
	      // already caught and we are here just to catch more.
	    if (parms_[idx] == 0)
		  continue;

	      // If this parameter is already a constant, all is well
	      // so go on.
	    if (dynamic_cast<NetEConst*>(parms_[idx]))
		  continue;

	      // Finally, try to evaluate the parameter expression
	      // that is here. If I succeed, reset the parameter to
	      // the evaluated value.
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

NetExpr* NetEParam::eval_tree()
{
      if (des_ == 0)
	    return 0;

      assert(scope_);
      const NetExpr*expr = scope_->get_parameter(name_);
      assert(expr);

      NetExpr*nexpr = expr->dup_expr();
      assert(nexpr);

	// If the parameter that I refer to is already evaluated, then
	// return the constant value.
      if (dynamic_cast<NetEConst*>(nexpr))
	    return nexpr;

	// Try to evaluate the expression. If I cannot, then the
	// expression is not a constant expression and I fail here.
      NetExpr*res = nexpr->eval_tree();
      if (res == 0) {
	    delete nexpr;
	    return 0;
      }

	// The result can be saved as the value of the parameter for
	// future reference, and return a copy to the caller.
      scope_->set_parameter(name_, res);
      return res->dup_expr();
}

/*
 * A ternary expression evaluation is controlled by the condition
 * expression. If the condition evaluates to true or false, then
 * return the evaluated true or false expression. If the condition
 * evaluates to x or z, then merge the constant bits of the true and
 * false expressions.
 */
NetExpr* NetETernary::eval_tree()
{
      NetExpr*tmp;

	/* Evaluate the cond_ to a constant. If it already is a
	   constant, then there is nothing to do. */

      NetEConst*c = dynamic_cast<NetEConst*>(cond_);
      if (c == 0) {
	    tmp = cond_->eval_tree();
	    c = dynamic_cast<NetEConst*>(tmp);
	    if (c == 0)
		  return 0;

	    assert(cond_ != c);
	    delete cond_;
	    cond_ = c;
      }

	/* If the condition is 1 or 0, return the true or false
	   expression. Try to evaluate the expression down as far as
	   we can. */

      if (c->value().get(0) == verinum::V1) {
	    tmp = dynamic_cast<NetEConst*>(true_val_);
	    if (tmp) return tmp->dup_expr();

	    tmp = true_val_->eval_tree();
	    if (tmp) {
		  delete true_val_;
		  true_val_ = tmp;
	    }
	    return true_val_->dup_expr();
      }

      if (c->value().get(0) == verinum::V0) {
	    tmp = dynamic_cast<NetEConst*>(false_val_);
	    if (tmp) return tmp->dup_expr();

	    tmp = false_val_->eval_tree();
	    if (tmp) {
		  delete false_val_;
		  false_val_ = tmp;
	    }
	    return false_val_->dup_expr();
      }


	/* Here we have a more complex case. We need to evaluate both
	   expressions down to constants then compare the values to
	   build up a constant result. */

      NetEConst*t = dynamic_cast<NetEConst*>(true_val_);
      if (t == 0) {
	    tmp = true_val_->eval_tree();
	    t = dynamic_cast<NetEConst*>(tmp);
	    if (t == 0)
		  return 0;

	    delete true_val_;
	    true_val_ = t;
      }


      NetEConst*f = dynamic_cast<NetEConst*>(false_val_);
      if (f == 0) {
	    tmp = false_val_->eval_tree();
	    f = dynamic_cast<NetEConst*>(tmp);
	    if (f == 0)
		  return 0;

	    delete false_val_;
	    false_val_ = f;
      }

      unsigned size = t->expr_width();
      assert(size == f->expr_width());

      verinum val (verinum::V0, size);
      for (unsigned idx = 0 ;  idx < size ;  idx += 1) {
	    verinum::V tv = t->value().get(idx);
	    verinum::V fv = f->value().get(idx);

	    if (tv == fv)
		  val.set(idx, tv);
	    else
		  val.set(idx, verinum::Vx);
      }

      NetEConst*rc = new NetEConst(val);
      rc->set_line(*this);
      return rc;
}

void NetEUnary::eval_expr_()
{
      if (dynamic_cast<NetEConst*>(expr_))
	    return;

      NetExpr*oper = expr_->eval_tree();
      if (oper == 0)
	    return;

      delete expr_;
      expr_ = oper;
}

NetEConst* NetEUnary::eval_tree()
{
      eval_expr_();
      NetEConst*rval = dynamic_cast<NetEConst*>(expr_);
      if (rval == 0)
	    return 0;

      verinum val = rval->value();

      switch (op_) {


	  case '~': {
		  /* Bitwise not is even simpler then logical
		     not. Just invert all the bits of the operand and
		     make the new value with the same dimensions. */
		for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
		      switch (val.get(idx)) {
			  case verinum::V0:
			    val.set(idx, verinum::V1);
			    break;
			  case verinum::V1:
			    val.set(idx, verinum::V0);
			    break;
			  default:
			    val.set(idx, verinum::Vx);
		      }

		return new NetEConst(val);
	  }

	  default:
	    delete rval;
	    return 0;
      }
}


NetEConst* NetEUBits::eval_tree()
{
      return NetEUnary::eval_tree();
}

NetEConst* NetEUReduce::eval_tree()
{
      eval_expr_();
      NetEConst*rval = dynamic_cast<NetEConst*>(expr_);
      if (rval == 0)
	    return 0;

      verinum val = rval->value();
      verinum::V res;

      switch (op_) {

	  case '!': {
		  /* Evaluate the unary logical not by first scanning
		     the operand value for V1 and Vx bits. If we find
		     any V1 bits we know that the value is TRUE, so
		     the result of ! is V0. If there are no V1 bits
		     but there are some Vx/Vz bits, the result is
		     unknown. Otherwise, the result is V1. */
		unsigned v1 = 0, vx = 0;
		for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1) {
		      switch (val.get(idx)) {
			  case verinum::V0:
			    break;
			  case verinum::V1:
			    v1 += 1;
			    break;
			  default:
			    vx += 1;
			    break;
		      }
		}

		res = v1? verinum::V0 : (vx? verinum::Vx : verinum::V1);
	  }

	  case '&': {
		res = verinum::V1;
		for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
		      res = res & val.get(idx);
		break;
	  }

	  case '|': {
		res = verinum::V0;
		for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
		      res = res | val.get(idx);
		break;
	  }

	  default:
	    return 0;
      }

      return new NetEConst(verinum(res, 1));
}


/*
 * $Log: eval_tree.cc,v $
 * Revision 1.19  2001/01/04 04:28:42  steve
 *  Evaluate constant logical && and ||.
 *
 * Revision 1.18  2001/01/02 04:21:14  steve
 *  Support a bunch of unary operators in parameter expressions.
 *
 * Revision 1.17  2001/01/02 03:23:40  steve
 *  Evaluate constant &, | and unary ~.
 *
 * Revision 1.16  2001/01/01 21:49:33  steve
 *  Fix shift and ternary operators in parameter expressions (PR#86)
 *
 * Revision 1.15  2000/12/16 20:00:17  steve
 *  Handle non-constant l-values.
 *
 * Revision 1.14  2000/12/16 19:03:30  steve
 *  Evaluate <= and ?: in parameter expressions (PR#81)
 *
 * Revision 1.13  2000/09/29 04:42:56  steve
 *  Cnstant evaluation of NE.
 *
 * Revision 1.12  2000/09/27 18:28:37  steve
 *  multiply in parameter expressions.
 *
 * Revision 1.11  2000/07/07 04:53:54  steve
 *  Add support for non-constant delays in delay statements,
 *  Support evaluating ! in constant expressions, and
 *  move some code from netlist.cc to net_proc.cc.
 *
 * Revision 1.10  2000/04/28 18:43:23  steve
 *  integer division in expressions properly get width.
 *
 * Revision 1.9  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.8  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.7  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.6  1999/10/22 23:57:53  steve
 *  do the <= in bits, not numbers.
 *
 * Revision 1.5  1999/10/10 23:29:37  steve
 *  Support evaluating + operator at compile time.
 *
 * Revision 1.4  1999/09/23 03:56:57  steve
 *  Support shift operators.
 *
 * Revision 1.3  1999/09/23 00:21:54  steve
 *  Move set_width methods into a single file,
 *  Add the NetEBLogic class for logic expressions,
 *  Fix error setting with of && in if statements.
 *
 * Revision 1.2  1999/09/21 00:13:40  steve
 *  Support parameters that reference other paramters.
 *
 * Revision 1.1  1999/09/20 02:21:10  steve
 *  Elaborate parameters in phases.
 *
 */

