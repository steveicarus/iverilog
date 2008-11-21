/*
 * Copyright (c) 1999-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: eval_tree.cc,v 1.62.2.5 2007/03/23 23:02:31 steve Exp $"
#endif

# include "config.h"
# include "compiler.h"

# include  <iostream>

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
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);

	/* Notice the special case where one of the operands is 0 and
	   this is a bitwise &. If this happens, then the result is
	   known to be 0. */
      if ((op() == '&') && lc && (lc->value() == verinum(0))) {
	    verinum res (verinum::V0, expr_width());
	    return new NetEConst(res);
      }

      if ((op() == '&') && rc && (rc->value() == verinum(0))) {
	    verinum res (verinum::V0, expr_width());
	    return new NetEConst(res);
      }

      if (lc == 0) return 0;
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
			    res.set(idx, rval.get(idx));

		if (rwid < lwid)
		      for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
			    res.set(idx, lval.get(idx));

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

	  case '^': {
		unsigned cnt = lwid;
		if (cnt > wid)  cnt = wid;
		if (cnt > rwid) cnt = rwid;
		for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
		      res.set(idx, lval.get(idx) ^ rval.get(idx));

		if (lwid < rwid)
		      for (unsigned idx = lwid ;  idx < rwid ;  idx += 1)
			    res.set(idx, rval.get(idx));

		if (rwid < lwid)
		      for (unsigned idx = rwid ;  idx < lwid ;  idx += 1)
			    res.set(idx, lval.get(idx));

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

      unsigned s_len = lv.len();
      if (rv.len() < s_len) s_len = rv.len();

      verinum result(verinum::V1, 1);
      for (unsigned idx = 0 ; idx < s_len; idx += 1) {
	    if (lv[idx] != rv[idx])
		  result = verinum::V0;
      }

      // If one operand was wider than the other, check that the
      // remaining bits are all zero
      if (lv.len() > s_len) {
	    for (unsigned idx = s_len; idx < lv.len(); idx += 1) {
		  if (lv[idx] != 0)
			result = verinum::V0;
	    }
      }
      if (rv.len() > s_len) {
	    for (unsigned idx = s_len; idx < rv.len(); idx += 1) {
		  if (rv[idx] != 0)
			result = verinum::V0;
	    }
      }

      return new NetEConst(result);
}


NetEConst* NetEBComp::eval_less_()
{
      if (right_->expr_type() == ET_REAL)
	    return eval_leeq_real_(false, false);
      if (left_->expr_type() == ET_REAL)
	    return eval_leeq_real_(false, false);

      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      verinum rv = r->value();
      if (! rv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (left_->expr_width() == 0) {
	    cerr << get_line() << ": internal error: "
		 << "Having trouble evaluating left expression of < op."
		 << endl;
	    cerr << get_line() << ":               : "
		 << "Expression is: "
		 << *this << endl;
	    return 0;
      }

	/* Detect the case where the right side is greater that or
	   equal to the largest value the left side can possibly
	   have. */
      assert(left_->expr_width() > 0);
      verinum lv (verinum::V1, left_->expr_width() + 1);
      lv.set(left_->expr_width(), verinum::V0);
      lv.has_sign( left_->has_sign() );
      if ((lv < rv) == verinum::V1) {
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

      if (lv.has_sign() && rv.has_sign()) {
	    if (lv.as_long() < rv.as_long()) {
		  verinum result(verinum::V1, 1);
		  return new NetEConst(result);
	    }
      } else {
	    if (lv.as_ulong() < rv.as_ulong()) {
		  verinum result(verinum::V1, 1);
		  return new NetEConst(result);
	    }
      }

      verinum result(verinum::V0, 1);
      return new NetEConst(result);
}

NetEConst* NetEBComp::eval_leeq_real_(bool gt_flag, bool include_eq_flag)
{
      NetEConst*vtmp;
      NetECReal*rtmp;
      double lv, rv;

      switch (left_->expr_type()) {
	  case ET_REAL:
	    rtmp = dynamic_cast<NetECReal*> (left_);
	    if (rtmp == 0)
		  return 0;

	    lv = rtmp->value().as_double();
	    break;

	  case ET_VECTOR:
	    vtmp = dynamic_cast<NetEConst*> (left_);
	    if (vtmp == 0)
		  return 0;

	    lv = vtmp->value().as_long();
	    break;

	  default:
	    assert(0);
      }


      switch (right_->expr_type()) {
	  case ET_REAL:
	    rtmp = dynamic_cast<NetECReal*> (right_);
	    if (rtmp == 0)
		  return 0;

	    rv = rtmp->value().as_double();
	    break;

	  case ET_VECTOR:
	    vtmp = dynamic_cast<NetEConst*> (right_);
	    if (vtmp == 0)
		  return 0;

	    rv = vtmp->value().as_long();
	    break;

	  default:
	    assert(0);
      }

	/* This function supports < and <=. If the eq_flag is true,
	   then include <=. Otherwise, include only <. */
      bool flag;
      if (gt_flag)
	    flag = include_eq_flag? (lv >= rv) : (lv > rv);
      else
	    flag = include_eq_flag? (lv <= rv) : (lv < rv);


      verinum result(flag? verinum::V1 : verinum::V0, 1);
      vtmp = new NetEConst(result);
      vtmp->set_line(*this);

      return vtmp;
}

NetEConst* NetEBComp::eval_leeq_()
{
      if (right_->expr_type() == ET_REAL)
	    return eval_leeq_real_(false, true);
      if (left_->expr_type() == ET_REAL)
	    return eval_leeq_real_(false, true);

      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      verinum rv = r->value();
      if (! rv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (left_->expr_width() == 0) {
	    cerr << get_line() << ": internal error: Something wrong "
		 << "with the left side width of <= ?" << endl;
	    cerr << get_line() << ":               : " << *this << endl;
      }

	/* Detect the case where the right side is greater than or
	   equal to the largest value the left side can possibly
	   have. */
      assert(left_->expr_width() > 0);
      verinum lv (verinum::V1, left_->expr_width() + 1);
      if (left_->has_sign() && rv.has_sign()) {
	    lv.set(lv.len()-1, verinum::V0);
	    lv.has_sign(true);
      }
      lv.set(left_->expr_width(), verinum::V0);
      lv.has_sign( left_->has_sign() );
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

      if (lv.has_sign() && rv.has_sign()) {
	    if (lv.as_long() <= rv.as_long()) {
		  verinum result(verinum::V1, 1);
		  return new NetEConst(result);
	    }
      } else {
	    if (lv.as_ulong() <= rv.as_ulong()) {
		  verinum result(verinum::V1, 1);
		  return new NetEConst(result);
	    }
      }

      verinum result(verinum::V0, 1);
      return new NetEConst(result);
}

NetEConst* NetEBComp::eval_gt_()
{
      if (right_->expr_type() == ET_REAL)
	    return eval_leeq_real_(true, false);
      if (left_->expr_type() == ET_REAL)
	    return eval_leeq_real_(true, false);

      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;

      verinum lv = l->value();
      if (! lv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

	/* Check for the special case where we know, simply by the
	   limited width of the right expression, that it cannot
	   possibly be false. */
      if (right_->expr_width() > 0) {
	    verinum rv (verinum::V1, right_->expr_width());
	    if (lv > rv) {
		  verinum result(verinum::V1, 1);
		  return new NetEConst(result);
	    }
      }

	/* Compare with a real value. Do it as double precision. */
      if (right_->expr_type() == NetExpr::ET_REAL) {
	    NetECReal*tmp = dynamic_cast<NetECReal*>(right_);
	    if (tmp == 0)
		  return 0;

	     double rr = tmp->value().as_double();
	     double ll = lv.has_sign()? lv.as_long() : lv.as_ulong();

	     verinum result ((ll > rr)? verinum::V1 : verinum::V0, 1, true);
	     return new NetEConst(result);
      }

	/* Now go on to the normal test of the values. */
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;
      verinum rv = r->value();
      if (! rv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (lv.has_sign() && rv.has_sign() && (lv.as_long() > rv.as_long())) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      }

      if (lv.as_ulong() > rv.as_ulong()) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      }

      verinum result(verinum::V0, 1);
      return new NetEConst(result);
}

NetEConst* NetEBComp::eval_gteq_()
{
      if (right_->expr_type() == ET_REAL)
	    return eval_leeq_real_(true, true);
      if (left_->expr_type() == ET_REAL)
	    return eval_leeq_real_(true, true);

      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;

      verinum lv = l->value();
      if (! lv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

	/* Detect the case where the left side is greater than the
	   largest value the right side can possibly have. */
      if (right_->expr_type() == NetExpr::ET_VECTOR) {
	    assert(right_->expr_width() > 0);
	    verinum rv (verinum::V1, right_->expr_width());
	    if (lv >= rv) {
		  verinum result(verinum::V1, 1);
		  return new NetEConst(result);
	    }
      }

	/* Compare with a real value. Do it as double precision. */
      if (right_->expr_type() == NetExpr::ET_REAL) {
	    NetECReal*tmp = dynamic_cast<NetECReal*>(right_);
	    if (tmp == 0)
		  return 0;

	     double rr = tmp->value().as_double();
	     double ll = lv.has_sign()? lv.as_long() : lv.as_ulong();

	     verinum result ((ll >= rr)? verinum::V1 : verinum::V0, 1, true);
	     return new NetEConst(result);
      }

	/* Now go on to the normal test of the values. */
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;
      verinum rv = r->value();
      if (! rv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (lv.has_sign() && rv.has_sign() && (lv.as_long() >= rv.as_long())) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      }

      if (lv.as_ulong() >= rv.as_ulong()) {
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

NetEConst* NetEBComp::eval_eqeqeq_()
{
      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      const verinum&lv = l->value();
      const verinum&rv = r->value();

      verinum::V res = verinum::V1;

      unsigned cnt = lv.len();
      if (cnt > rv.len())
	    cnt = rv.len();

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
	    if (lv.get(idx) != rv.get(idx))
		  res = verinum::V0;

      for (unsigned idx = cnt ;  idx < lv.len() ;  idx += 1)
	    if (lv.get(idx) != verinum::V0)
		  res = verinum::V0;

      for (unsigned idx = cnt ;  idx < rv.len() ;  idx += 1)
	    if (rv.get(idx) != verinum::V0)
		  res = verinum::V0;

      return new NetEConst(verinum(res, 1));
}

NetEConst* NetEBComp::eval_neeqeq_()
{
      NetEConst*tmp = eval_eqeqeq_();
      if (tmp == 0)
	    return 0;

      NetEConst*res;

      if (tmp->value().get(0) == verinum::V0)
	    res = new NetEConst(verinum(verinum::V1,1));
      else
	    res = new NetEConst(verinum(verinum::V0,1));

      delete tmp;
      return res;
}

NetEConst* NetEBComp::eval_tree()
{
      eval_sub_tree_();

      switch (op_) {
	  case 'E': // Case equality (===)
	    return eval_eqeqeq_();

	  case 'e': // Equality (==)
	    return eval_eqeq_();

	  case 'G': // >=
	    return eval_gteq_();

	  case 'L': // <=
	    return eval_leeq_();

	  case 'N': // Case inequality (!==)
	    return eval_neeqeq_();

	  case 'n': // not-equal (!=)
	    return eval_neeq_();

	  case '<': // Less than
	    return eval_less_();

	  case '>': // Greater then
	    return eval_gt_();

	  default:
	    return 0;
      }
}

/*
 * The NetEBDiv operator includes the / and % operators. First evaluate
 * the sub-expressions, then perform the required operation.
 */
NetExpr* NetEBDiv::eval_tree()
{
      eval_sub_tree_();

      if (expr_type() == NetExpr::ET_REAL) {
	    NetECReal*lc = dynamic_cast<NetECReal*>(left_);
	    if (lc == 0) return 0;

	    verireal lval = lc->value();

	    if (NetECReal*rc = dynamic_cast<NetECReal*>(right_)) {
		  NetECReal*tmp = 0;
		  verireal rval = rc->value();

		  switch (op_) {
		      case '/':
			tmp = new NetECReal(lval / rval);
			break;

		      case '%':
			tmp = new NetECReal(lval % rval);
		  }

		  assert(tmp);
		  tmp->set_line(*this);
		  return tmp;

	    } else if (NetEConst*rc = dynamic_cast<NetEConst*>(right_)) {

		  NetECReal*tmp = 0;
		  verinum rval = rc->value();

		  switch (op_) {
		      case '/':
			tmp = new NetECReal(lval / rval);
			break;

		      case '%':
			tmp = new NetECReal(lval % rval);
		  }

		  assert(tmp);
		  tmp->set_line(*this);
		  return tmp;

	    }


      } else {
	    assert(expr_type() == NetExpr::ET_VECTOR);
	    NetEConst*lc = dynamic_cast<NetEConst*>(left_);
	    if (lc == 0) return 0;
	    NetEConst*rc = dynamic_cast<NetEConst*>(right_);
	    if (rc == 0) return 0;

	    verinum lval = lc->value();
	    verinum rval = rc->value();

	    switch (op_) {
		case '/':
		  return new NetEConst(lval / rval);

		case '%':
		  return new NetEConst(lval % rval);
	    }
      }

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

      if (rv == verinum::V0)
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

NetExpr* NetEBMult::eval_tree_real_()
{
      verireal lval;
      verireal rval;

      switch (left_->expr_type()) {
	  case ET_REAL: {
		NetECReal*lc = dynamic_cast<NetECReal*> (left_);
		if (lc == 0) return 0;
		lval = lc->value();
		break;
	  }

	  case ET_VECTOR: {
		NetEConst*lc = dynamic_cast<NetEConst*>(left_);
		if (lc == 0) return 0;
		verinum tmp = lc->value();
		lval = verireal(tmp.as_long());
		break;
	  }

	  default:
	    assert(0);
      }

      switch (right_->expr_type()) {
	  case ET_REAL: {
		NetECReal*rc = dynamic_cast<NetECReal*> (right_);
		if (rc == 0) return 0;
		rval = rc->value();
		break;
	  }

	  case ET_VECTOR: {
		NetEConst*rc = dynamic_cast<NetEConst*>(right_);
		if (rc == 0) return 0;
		verinum tmp = rc->value();
		rval = verireal(tmp.as_long());
		break;
	  }

	  default:
	    assert(0);
      }


      NetECReal*res = new NetECReal(lval * rval);
      res->set_line(*this);
      return res;
}

NetExpr* NetEBMult::eval_tree()
{
      eval_sub_tree_();

      if (expr_type() == ET_REAL)
	    return eval_tree_real_();

      assert(expr_type() == ET_VECTOR);

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

	/* Make an early estimate of the expression width. */
      unsigned wid = expr_width();

      if (rv.is_defined()) {

	    unsigned shift = rv.as_ulong();

	    if ((wid == 0) || ! lv.has_len()) {
		    /* If the caller doesn't care what the width is,
		       then calcuate a width from the trimmed left
		       expression, plus the shift. This avoids
		       data loss. */
		  lv = trim_vnum(lv);
		  wid = lv.len();
		  if (op() == 'l')
			wid = lv.len() + shift;
	    }

	    assert(wid);
	    verinum nv (verinum::V0, wid, lv.has_len());

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
	    if (wid == 0)
		  wid = left_->expr_width();

	    verinum nv (verinum::Vx, wid);
	    res = new NetEConst(nv);
      }

      return res;
}

NetEConst* NetEConcat::eval_tree()
{
      unsigned repeat_val = repeat();
      unsigned local_errors = 0;

      unsigned gap = 0;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {

	      // Parameter not here? This is an error, but presumably
	      // already caught and we are here just to catch more.
	    if (parms_[idx] == 0)
		  continue;


	      // If this parameter is already a constant, all is well
	      // so go on.
	    if (dynamic_cast<NetEConst*>(parms_[idx])) {
		  gap += parms_[idx]->expr_width();
		  continue;
	    }

	      // Finally, try to evaluate the parameter expression
	      // that is here. If I succeed, reset the parameter to
	      // the evaluated value.
	    assert(parms_[idx]);
	    NetExpr*expr = parms_[idx]->eval_tree();
	    if (expr) {
		  delete parms_[idx];
		  parms_[idx] = expr;

		  if (! expr->has_width()) {
			cerr << get_line() << ": error: concatenation "
			     << "operand has indefinite width: "
			     << *parms_[idx] << endl;
			local_errors += 1;
		  } else if (expr->expr_width() == 0) {
			cerr << expr->get_line() << ": internal error: "
			     << "Operand of concatenation has no width: "
			     << *expr << endl;
			local_errors += 1;
		  }

		  gap += expr->expr_width();
	    }

      }

      if (local_errors > 0)
	    return 0;

	// Handle the special case that the repeat expression is
	// zero. In this case, just return a 0 value with the expected
	// width.
      if (repeat_val == 0) {
	    verinum val (verinum::V0, expr_width());
	    NetEConst*res = new NetEConst(val);
	    res->set_width(val.len());
	    return res;
      }

	// At this point, the "gap" is the width of a single repeat of
	// the concatenation. The total width of the result is the gap
	// times the repeat count.
      verinum val (verinum::Vx, repeat_val * gap);

	// build up the result from least significant to most.

      unsigned cur = 0;
      bool is_string_flag = true;
      for (unsigned idx = parms_.count() ;  idx > 0 ;  idx -= 1) {
	    NetEConst*expr = dynamic_cast<NetEConst*>(parms_[idx-1]);
	    if (expr == 0)
		  return 0;

	    verinum tmp = expr->value();
	    for (unsigned bit = 0;  bit < tmp.len(); bit += 1, cur += 1)
		  for (unsigned rep = 0 ;  rep < repeat_val ;  rep += 1)
			val.set(rep*gap+cur, tmp[bit]);

	    is_string_flag = is_string_flag && tmp.is_string();
      }

	/* If all the values were strings, then re-stringify this
	   constant. This might be useful information in the code
	   generator or other optimizer steps. */
      if (is_string_flag) {
	    val = verinum(val.as_string());
      }

      NetEConst*res = new NetEConst(val);
      res->set_width(val.len());
      return res;
}

/*
 * There are limits to our ability to evaluate a memory reference
 * expression, because the content of a memory is never
 * constant. However, the index expression may be precalculated, and
 * there are certain index values that do give us constant results.
 */
NetExpr* NetEMemory::eval_tree()
{
	/* Attempt to evaluate the index expression to a constant, if
	   it is not already. */
      if (idx_ && !dynamic_cast<NetEConst*>(idx_)) {
	    NetExpr* tmp = idx_->eval_tree();
	    if (tmp) {
		  delete idx_;
		  idx_ = tmp;
	    }
      }

      NetEConst*itmp = dynamic_cast<NetEConst*>(idx_);
      if (itmp == 0)
	    return 0;

      verinum ival = itmp->value();

	/* If the index expression has any x or z bits, then we know
	   already that the expression result is a constant x. */
      if (! ival.is_defined()) {
	    verinum xres (verinum::Vx, mem_->width(), false);
	    NetEConst*res = new NetEConst(xres);
	    return res;
      }

	/* If the index expression is outside the range of the memory,
	   then the result is a constant x. */
      unsigned norm_idx = mem_->index_to_address(ival.as_long());
      if (norm_idx >= mem_->count()) {
	    verinum xres (verinum::Vx, mem_->width(), false);
	    NetEConst*res = new NetEConst(xres);
	    return res;
      }

      return 0;
}

NetExpr* NetEParam::eval_tree()
{
      if (des_ == 0)
	    return 0;

      assert(scope_);
      const NetExpr*expr = scope_->get_parameter(name_);
      if (expr == 0) {
	    cerr << get_line() << ": internal error: Unable to match "
		 << "parameter " << name_ << " in scope "
		 << scope_->name() << endl;
	    return 0;
      }

      assert(expr);

      NetExpr*nexpr = expr->dup_expr();
      assert(nexpr);

	// If the parameter that I refer to is already evaluated, then
	// return the constant value.
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(nexpr)) {
	    verinum val = tmp->value();
	    NetEConstParam*ptmp = new NetEConstParam(scope_, name_, val);
	    ptmp->set_line(*this);
	    delete nexpr;
	    return ptmp;
      }

      if (NetECReal*tmp = dynamic_cast<NetECReal*>(nexpr)) {
	    verireal val = tmp->value();
	    NetECRealParam*ptmp = new NetECRealParam(scope_, name_, val);
	    ptmp->set_line(*this);
	    delete nexpr;
	    return ptmp;
      }

	// Try to evaluate the expression. If I cannot, then the
	// expression is not a constant expression and I fail here.
      NetExpr*res = nexpr->eval_tree();
      if (res == 0) {
	    cerr << get_line() << ": internal error: Unable to evaluate "
		 << "parameter " << name_ << " expression: "
		 << *nexpr << endl;
	    delete nexpr;
	    return 0;
      }

	// The result can be saved as the value of the parameter for
	// future reference, and return a copy to the caller.
      scope_->replace_parameter(name_, res);

	/* Return as a result a NetEConstParam or NetECRealParam
	   object, depending on the type of the expression. */

      switch (res->expr_type()) {

	  case NetExpr::ET_VECTOR:
	    { NetEConst*tmp = dynamic_cast<NetEConst*>(res);
	      if (tmp == 0) {
		    cerr << get_line() << ": internal error: parameter "
			 << name_ << " evaluates to incomprehensible "
			 << *res << "." << endl;
		    return 0;
	      }

	      assert(tmp);

	      verinum val = tmp->value();
	      NetEConstParam*ptmp = new NetEConstParam(scope_, name_, val);

	      return ptmp;
	    }

	  case NetExpr::ET_REAL:
	    { NetECReal*tmp = dynamic_cast<NetECReal*>(res);
	      if (tmp == 0) {
		    cerr << get_line() << ": internal error: parameter "
			 << name_ << " evaluates to incomprehensible "
			 << *res << "." << endl;
		    return 0;
	      }

	      assert(tmp);

	      verireal val = tmp->value();
	      NetECRealParam*ptmp = new NetECRealParam(scope_, name_, val);

	      return ptmp;
	    }

	  default:
	    assert(0);
	    return 0;
      }
}

NetEConst* NetESelect::eval_tree()
{
      NetEConst*expr = dynamic_cast<NetEConst*>(expr_);
      if (expr == 0) {
	    NetExpr*tmp = expr_->eval_tree();
	    if (tmp != 0) {
		  delete expr_;
		  expr_ = tmp;
	    }

	    expr = dynamic_cast<NetEConst*>(expr_);
      }

      NetEConst*base = dynamic_cast<NetEConst*>(base_);
      if (base == 0) {
	    NetExpr*tmp = base_->eval_tree();
	    if (tmp != 0) {
		  delete base_;
		  base_ = tmp;
	    }

	    base = dynamic_cast<NetEConst*>(base_);
      }

      if (expr == 0)
	    return 0;
      if (base == 0)
	    return 0;

      verinum eval = expr->value();
      verinum oval (verinum::V0, expr_width(), true);
      long bval = base->value().as_long();

      for (unsigned long idx = 0 ;  idx < expr_width() ;  idx += 1) {
	    if ((bval >= 0) && ((unsigned long) bval < eval.len()))
		  oval.set(idx, eval.get(bval));
	    else
		  oval.set(idx, verinum::Vx);

	    bval += 1;
      }

      NetEConst*res = new NetEConst(oval);
      return res;
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

      assert(cond_);
      if (0 == dynamic_cast<NetEConst*>(cond_)) {
	    tmp = cond_->eval_tree();
	    if (tmp != 0) {
		  delete cond_;
		  cond_ = tmp;
	    }
      }

      assert(true_val_);
      if (0 == dynamic_cast<NetEConst*>(true_val_)) {
	    tmp = true_val_->eval_tree();
	    if (tmp != 0) {
		  delete true_val_;
		  true_val_ = tmp;
	    }
      }

      assert(false_val_);
      if (0 == dynamic_cast<NetEConst*>(false_val_)) {
	    tmp = false_val_->eval_tree();
	    if (tmp != 0) {
		  delete false_val_;
		  false_val_ = tmp;
	    }
      }


      NetEConst*c = dynamic_cast<NetEConst*>(cond_);
      if (c == 0)
	    return 0;

	/* Check the boolean value of the constant condition
	   expression. Note that the X case is handled explicitly, so
	   we must differentiate. */

      verinum cond_value = c->value();
      bool true_flag = false;
      bool x_flag = false;

      for (unsigned idx = 0 ;  idx < cond_value.len() ;  idx += 1) {
	    switch (cond_value.get(idx)) {
		case verinum::V1:
		  true_flag = true;
		  break;
		case verinum::V0:
		  break;
		default:
		  x_flag = true;
	    }
      }


	/* If the condition is 1 or 0, return the true or false
	   expression. Try to evaluate the expression down as far as
	   we can. */

      if (true_flag) {
	    if (debug_eval_tree) {
		  cerr << get_line() << ": debug: Evaluate ternary with "
		       << "constant condition value: " << c->value() << endl;
		  cerr << get_line() << ":      : Selecting true case: "
		       << *true_val_ << endl;
	    }
	    return true_val_->dup_expr();
      }

      if (! x_flag) {
	    if (debug_eval_tree) {
		  cerr << get_line() << ": debug: Evaluate ternary with "
		       << "constant condition value: " << c->value() << endl;
		  cerr << get_line() << ":      : Selecting false case: "
		       << *true_val_ << endl;
	    }
	    return false_val_->dup_expr();
      }

	/* Here we have a more complex case. We need to evaluate both
	   expressions down to constants then compare the values to
	   build up a constant result. */

      NetEConst*t = dynamic_cast<NetEConst*>(true_val_);
      if (t == 0)
	    return 0;


      NetEConst*f = dynamic_cast<NetEConst*>(false_val_);
      if (f == 0)
	    return 0;


      unsigned tsize = t->expr_width();
      unsigned fsize = f->expr_width();
	/* Size of the result is the size of the widest operand. */
      unsigned rsize = tsize > fsize? tsize : fsize;

      verinum val (verinum::V0, rsize);
      for (unsigned idx = 0 ;  idx < rsize ;  idx += 1) {
	    verinum::V tv = idx < tsize? t->value().get(idx) : verinum::V0;
	    verinum::V fv = idx < rsize? f->value().get(idx) : verinum::V0;

	    if (tv == fv)
		  val.set(idx, tv);
	    else
		  val.set(idx, verinum::Vx);
      }

      if (debug_eval_tree) {
	    cerr << get_line() << ": debug: Evaluate ternary with "
		 << "constant condition value: " << c->value() << endl;
	    cerr << get_line() << ":      : Blending cases to get "
		 << val << endl;
      }

      NetEConst*rc = new NetEConst(val);
      rc->set_line(*this);
      return rc;
}

void NetEUnary::eval_expr_()
{
      assert(expr_);
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

	  case '+':
	      /* Unary + is a no-op. */
	    return new NetEConst(val);

	  case '-': {
		if (val.is_defined()) {

		      verinum tmp (verinum::V0, val.len());
		      tmp.has_sign(val.has_sign());
		      val = tmp - val;

		} else {
		      for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
			    val.set(idx, verinum::Vx);
		}

		return new NetEConst(val);
	  }

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

	  case '!':
	    assert(0);
	  default:
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
      bool invert = false;

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
		break;
	  }

	  case 'A':
		invert = true;
	  case '&': {
		res = verinum::V1;
		for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
		      res = res & val.get(idx);
		break;
	  }

	  case 'N':
		invert = true;
	  case '|': {
		res = verinum::V0;
		for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
		      res = res | val.get(idx);
		break;
	  }

	  case 'X':
		invert = true;
	  case '^': {
		  /* Reduction XOR. */
		unsigned ones = 0, unknown = 0;
		for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
		      switch (val.get(idx)) {
			  case verinum::V0:
			    break;
			  case verinum::V1:
			    ones += 1;
			    break;
			  default:
			    unknown += 1;
			    break;
		      }

		if (unknown) res = verinum::Vx;
		else if (ones%2) res = verinum::V1;
		else res = verinum::V0;
		break;
	  }

	  default:
	    return 0;
      }

      if (invert) res = ~res;
      return new NetEConst(verinum(res, 1));
}


/*
 * $Log: eval_tree.cc,v $
 * Revision 1.62.2.5  2007/03/23 23:02:31  steve
 *  Fix compile time eval of <= comparison.
 *
 * Revision 1.62.2.4  2007/03/23 20:59:25  steve
 *  Fix compile time evaluation of < operator.
 *
 * Revision 1.62.2.3  2005/09/09 02:17:08  steve
 *  Evaluate  magnitude compare with real operands.
 *
 * Revision 1.62.2.2  2005/09/04 15:41:54  steve
 *  More explicit internal error message.
 *
 * Revision 1.62.2.1  2005/09/04 15:39:19  steve
 *  More explicit internal error message.
 *
 * Revision 1.62  2004/10/04 01:10:53  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.61  2004/09/10 23:51:42  steve
 *  Fix the evaluation of constant ternary expressions.
 *
 * Revision 1.60  2004/02/20 06:22:56  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.59  2003/10/31 02:47:11  steve
 *  NetEUReduce has its own dup_expr method.
 *
 * Revision 1.58  2003/10/26 04:54:56  steve
 *  Support constant evaluation of binary ^ operator.
 *
 * Revision 1.57  2003/09/04 01:52:50  steve
 *  Evaluate real parameter expressions that contain real parameters.
 *
 * Revision 1.56  2003/08/01 02:12:30  steve
 *  Fix || with true case on the right.
 *
 * Revision 1.55  2003/06/24 01:38:02  steve
 *  Various warnings fixed.
 *
 * Revision 1.54  2003/06/05 04:28:24  steve
 *  Evaluate <= with real operands.
 *
 * Revision 1.53  2003/06/04 01:26:17  steve
 *  internal error for <= expression errors.
 *
 * Revision 1.52  2003/05/30 02:55:32  steve
 *  Support parameters in real expressions and
 *  as real expressions, and fix multiply and
 *  divide with real results.
 *
 * Revision 1.51  2003/04/15 05:06:56  steve
 *  Handle real constants evaluation > and >=.
 *
 * Revision 1.50  2003/04/14 03:40:21  steve
 *  Make some effort to preserve bits while
 *  operating on constant values.
 */

