/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
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
# include "compiler.h"

# include  <iostream>
# include  <cstdlib>
# include  <cstring>
# include  <cmath>

# include  "netlist.h"
# include  "ivl_assert.h"
# include  "netmisc.h"

NetExpr* NetExpr::eval_tree(int prune_to_width)
{
      return 0;
}

static bool get_real_arg_(NetExpr*expr, verireal&val)
{
      switch (expr->expr_type()) {
	  case IVL_VT_REAL: {
		NetECReal*c = dynamic_cast<NetECReal*> (expr);
		if (c == 0) return false;
		val = c->value();
		break;
	  }

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC: {
		NetEConst*c = dynamic_cast<NetEConst*>(expr);
		if (c == 0) return false;
		verinum tmp = c->value();
		val = verireal(tmp.as_double());
		break;
	  }

	  default:
	    assert(0);
      }

      return true;
}

bool NetEBinary::get_real_arguments_(verireal&lval, verireal&rval)
{
      if (!get_real_arg_(left_, lval)) return false;
      if (!get_real_arg_(right_, rval)) return false;

      return true;
}

NetExpr* NetEBAdd::eval_tree(int prune_to_width)
{
      eval_expr(left_, prune_to_width);
      eval_expr(right_, prune_to_width);

      if (left_->expr_type() == IVL_VT_REAL || right_->expr_type()==IVL_VT_REAL)
	    return eval_tree_real_();

      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);

	/* If both operands are constant, then replace the entire
	   expression with a constant value. */
      if (lc != 0 && rc != 0) {
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

	    if (debug_eval_tree) {
		  cerr << get_fileline() << ": debug: Evaluate expr=" << *this
		       << " --- prune=" << prune_to_width
		       << " has_width=" << (has_width()? "true" : "false") << endl;
	    }

	    /* Result might have known width. */
	    if (has_width()) {
		  unsigned lwid = lc->expr_width();
		  unsigned rwid = rc->expr_width();
		  unsigned  wid = (rwid > lwid) ? rwid : lwid;
		  if (prune_to_width < 0)
			wid += 1;
		  verinum val2=verinum(val,wid);
		  val=val2;
	    } else {
		    /* No fixed width, so trim the bits losslessly. */
		  verinum val2 = trim_vnum(val);
		  val = val2;
	    }

	    return new NetEConst(val);
      }

	/* Try to combine a right constant value with the right
	   constant value of a sub-expression add. For example, the
	   expression (a + 2) - 1 can be rewritten as a + 1. */

      NetEBAdd*se = dynamic_cast<NetEBAdd*>(left_);
      lc = se? dynamic_cast<NetEConst*>(se->right_) : 0;

      if (lc != 0 && rc != 0) {
	    ivl_assert(*this, se != 0);

	    if (debug_eval_tree) {
		  cerr << get_fileline() << ": debug: "
		       << "Partially evaluate " << *this
		       << " using (a+2)-1 --> (a+1) transform." << endl;
	    }

	    verinum lval = lc->value();
	    verinum rval = rc->value();

	    if (lval.len() < expr_width())
		  lval = pad_to_width(lval, expr_width());
	    if (rval.len() < expr_width())
		  rval = pad_to_width(rval, expr_width());

	    if (se->expr_width() > this->expr_width()) {
		  cerr << get_fileline() << ": internal error: "
		       << "expr_width()=" << expr_width()
		       << ", sub expr_width()=" << se->expr_width()
		       << ", sub expression=" << *se << endl;
	    }
	    ivl_assert(*this, se->expr_width() <= this->expr_width());

	    verinum val;
	    if (op_ == se->op_) {
		    /* (a + lval) + rval  --> a + (rval+lval) */
		    /* (a - lval) - rval  --> a - (rval+lval) */
		  val = rval + lval;
	    } else {
		    /* (a - lval) + rval  -->  a + (rval-lval) */
		    /* (a + lval) - rval  -->  a - (rval-lval) */
		  val = rval - lval;
	    }

	      // Since we padded the operands above to be the minimum
	      // width, the val should also be at least expr_width().
	    ivl_assert(*this, val.len() >= expr_width());
	    if (val.len() > expr_width()) {
		  verinum tmp (val, expr_width());
		  tmp.has_sign(val.has_sign());
		  val = tmp;
	    }

	    NetEConst*tmp = new NetEConst(val);
	    left_ = se->left_->dup_expr();
	    delete se;
	    tmp->set_line(*right_);
	    delete right_;
	    right_ = tmp;
	      /* We've changed the subexpression, but the result is
		 still not constant, so return nil here anyhow. */
	    return 0;
      }

	/* Nothing more to be done, the value is not constant. */
      return 0;
}

NetECReal* NetEBAdd::eval_tree_real_()
{
      verireal lval;
      verireal rval;
      bool flag = get_real_arguments_(lval, rval);
      if (!flag) return 0;

      verireal res_val;

      switch (op()) {
	  case '+':
	    res_val = lval + rval;
	    break;
	  case '-':
	    res_val = lval - rval;
	    break;
	  default:
	    ivl_assert(*this, 0);
      }

      NetECReal*res = new NetECReal( res_val );
      res->set_line(*this);
      return res;
}

NetEConst* NetEBBits::eval_tree(int prune_to_width)
{
      eval_expr(left_);
      eval_expr(right_);

      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (lc == 0 || rc == 0) return 0;

	/* Notice the special case where one of the operands is 0 and
	   this is a bitwise &. If this happens, then the result is
	   known to be 0. */
      if ((op() == '&') && (lc->value() == verinum(0))) {
	    verinum res (verinum::V0, expr_width());
	    return new NetEConst(res);
      }

      if ((op() == '&') && (rc->value() == verinum(0))) {
	    verinum res (verinum::V0, expr_width());
	    return new NetEConst(res);
      }

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

	// Sub-expressions of bitwise operators need to be the same
	// width. Pad them out if necessary.
      if (lwid < wid) {
	    lval = pad_to_width(lval, wid);
	    lwid = wid;
      }
      if (rwid < wid) {
	    rval = pad_to_width(rval, wid);
	    rwid = wid;
      }

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


NetEConst* NetEBComp::eval_less_()
{
      if (right_->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(left_, right_, false);
      if (left_->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(left_, right_, false);

      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      verinum rv = r->value();
      if (! rv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (NetEConst*tmp = must_be_leeq_(left_, rv, false)) {
	    return tmp;
      }

	/* Now go on to the normal test of the values. */
      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;
      verinum lv = l->value();
      if (! lv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (lv < rv) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      } else {
	    verinum result(verinum::V0, 1);
	    return new NetEConst(result);
      }
}

NetEConst* NetEBComp::eval_leeq_real_(NetExpr*le, NetExpr*ri, bool eq_flag)
{
      NetEConst*vtmp;
      NetECReal*rtmp;
      double lv, rv;

      switch (le->expr_type()) {
	  case IVL_VT_REAL:
	    rtmp = dynamic_cast<NetECReal*> (le);
	    if (rtmp == 0) return 0;
	    lv = rtmp->value().as_double();
	    break;

	  case IVL_VT_LOGIC:
	  case IVL_VT_BOOL:
	    vtmp = dynamic_cast<NetEConst*> (le);
	    if (vtmp == 0) return 0;
	    lv = vtmp->value().as_double();
	    break;

	  default:
	    lv = 0.0;
	    cerr << get_fileline() << ": internal error: "
		 << "Unexpected expression type? " << le->expr_type() << endl;
	    assert(0);
      }

      switch (ri->expr_type()) {
	  case IVL_VT_REAL:
	    rtmp = dynamic_cast<NetECReal*> (ri);
	    if (rtmp == 0) return 0;
	    rv = rtmp->value().as_double();
	    break;

	  case IVL_VT_LOGIC:
	  case IVL_VT_BOOL:
	    vtmp = dynamic_cast<NetEConst*> (ri);
	    if (vtmp == 0) return 0;
	    rv = vtmp->value().as_double();
	    break;

	  default:
	    rv = 0.0;
	    cerr << get_fileline() << ": internal error: "
		 << "Unexpected expression type? " << ri->expr_type() << endl;
	    assert(0);
      }

      bool test = false;
      if (lv < rv) test = true;
      if (test == false && eq_flag && lv == rv) test = true;

      verinum result(test? verinum::V1 : verinum::V0, 1);
      vtmp = new NetEConst(result);
      vtmp->set_line(*this);

      return vtmp;
}

NetEConst* NetEBComp::must_be_leeq_(NetExpr*le, const verinum&rv, bool eq_flag)
{
      assert(le->expr_width() > 0);
      verinum lv (verinum::V1, le->expr_width());
      if (le->has_sign() && rv.has_sign()) {
	      // If the expression is signed, then the largest
	      // possible value for the left_ needs to have a 0 in the
	      // sign position.
	    lv.set(lv.len()-1, verinum::V0);
	    lv.has_sign(true);
      }

      if (lv < rv || (eq_flag && (lv == rv))) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      }

      return 0;
}

NetEConst* NetEBComp::eval_leeq_()
{
      if (right_->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(left_, right_, true);
      if (left_->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(left_, right_, true);

      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      verinum rv = r->value();
      if (! rv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (left_->expr_width() == 0) {
	    cerr << get_fileline() << ": internal error: Something wrong "
		 << "with the left side width of <= ?" << endl;
	    cerr << get_fileline() << ":               : " << *this << endl;
      }

      if (NetEConst*tmp = must_be_leeq_(left_, rv, true)) {
	    return tmp;
      }

	/* Now go on to the normal test of the values. */
      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;
      verinum lv = l->value();
      if (! lv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (lv <= rv) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      } else {
	    verinum result(verinum::V0, 1);
	    return new NetEConst(result);
      }
}

NetEConst* NetEBComp::eval_gt_()
{
      if (right_->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(right_, left_, false);
      if (left_->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(right_, left_, false);

      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;

      verinum lv = l->value();
      if (! lv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (NetEConst*tmp = must_be_leeq_(right_, lv, false)) {
	    return tmp;
      }

	/* Now go on to the normal test of the values. */
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;
      verinum rv = r->value();
      if (! rv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (lv > rv) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      } else {
	    verinum result(verinum::V0, 1);
	    return new NetEConst(result);
      }
}

NetEConst* NetEBComp::eval_gteq_()
{
      if (right_->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(right_, left_, true);
      if (left_->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(right_, left_, true);

      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;

      verinum lv = l->value();
      if (! lv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (NetEConst*tmp = must_be_leeq_(right_, lv, true)) {
	    return tmp;
      }

	/* Now go on to the normal test of the values. */
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;
      verinum rv = r->value();
      if (! rv.is_defined()) {
	    verinum result(verinum::Vx, 1);
	    return new NetEConst(result);
      }

      if (lv >= rv) {
	    verinum result(verinum::V1, 1);
	    return new NetEConst(result);
      } else {
	    verinum result(verinum::V0, 1);
	    return new NetEConst(result);
      }
}

/*
 * Evaluate <A>==<B> or <A>!=<B>. The equality operator checks all the
 * bits and returns true(false) if there are any bits in the vector
 * that are defined (0 or 1) and different. If all the defined bits
 * are equal, but there are are x/z bits, then the situation is
 * ambiguous so the result is x.
 */
NetEConst* NetEBComp::eval_eqeq_real_(NetExpr*le, NetExpr*ri, bool ne_flag)
{
      NetEConst*vtmp;
      NetECReal*rtmp;
      double lv, rv;

      switch (le->expr_type()) {
	  case IVL_VT_REAL:
	    rtmp = dynamic_cast<NetECReal*> (le);
	    if (rtmp == 0) return 0;
	    lv = rtmp->value().as_double();
	    break;

	  case IVL_VT_LOGIC:
	  case IVL_VT_BOOL:
	    vtmp = dynamic_cast<NetEConst*> (le);
	    if (vtmp == 0) return 0;
	    lv = vtmp->value().as_double();
	    break;

	  default:
	    lv = 0.0;
	    cerr << get_fileline() << ": internal error: "
		 << "Unexpected expression type? " << le->expr_type() << endl;
	    assert(0);
      }

      switch (ri->expr_type()) {
	  case IVL_VT_REAL:
	    rtmp = dynamic_cast<NetECReal*> (ri);
	    if (rtmp == 0) return 0;
	    rv = rtmp->value().as_double();
	    break;

	  case IVL_VT_LOGIC:
	  case IVL_VT_BOOL:
	    vtmp = dynamic_cast<NetEConst*> (ri);
	    if (vtmp == 0) return 0;
	    rv = vtmp->value().as_double();
	    break;

	  default:
	    rv = 0.0;
	    cerr << get_fileline() << ": internal error: "
		 << "Unexpected expression type? " << ri->expr_type() << endl;
	    assert(0);
      }

      verinum result(((lv == rv) ^ ne_flag) ? verinum::V1 : verinum::V0, 1);
      vtmp = new NetEConst(result);
      vtmp->set_line(*this);

      return vtmp;
}

NetEConst* NetEBComp::eval_eqeq_(bool ne_flag)
{
      if (right_->expr_type() == IVL_VT_REAL)
	    return eval_eqeq_real_(right_, left_, ne_flag);
      if (left_->expr_type() == IVL_VT_REAL)
	    return eval_eqeq_real_(right_, left_, ne_flag);

      NetEConst*l = dynamic_cast<NetEConst*>(left_);
      if (l == 0) return 0;
      NetEConst*r = dynamic_cast<NetEConst*>(right_);
      if (r == 0) return 0;

      const verinum&lv = l->value();
      const verinum&rv = r->value();

      const verinum::V eq_res = ne_flag? verinum::V0 : verinum::V1;
      const verinum::V ne_res = ne_flag? verinum::V1 : verinum::V0;

      verinum::V res = eq_res;
      unsigned top = lv.len();
      if (rv.len() < top)
	    top = rv.len();

      for (unsigned idx = 0 ;  idx < top ;  idx += 1) {

	    bool x_bit_present = false;

	    switch (lv.get(idx)) {

		case verinum::Vx:
		case verinum::Vz:
		  res = verinum::Vx;
		  x_bit_present = true;
		  break;

		default:
		  break;
	    }

	    switch (rv.get(idx)) {

		case verinum::Vx:
		case verinum::Vz:
		  res = verinum::Vx;
		  x_bit_present = true;
		  break;

		default:
		  break;
	    }

	    if (x_bit_present)
		  continue;

	    if (rv.get(idx) != lv.get(idx)) {
		  res = ne_res;
		  break;
	    }
      }

      if (res != verinum::Vx) {
	    verinum::V lpad = verinum::V0;
	    verinum::V rpad = verinum::V0;

	    if (lv.has_sign() && lv.get(lv.len()-1) == verinum::V1)
		  lpad = verinum::V1;
	    if (rv.has_sign() && rv.get(rv.len()-1) == verinum::V1)
		  rpad = verinum::V1;

	    for (unsigned idx = top ;  idx < lv.len() ;  idx += 1)
		  switch (lv.get(idx)) {

		      case verinum::Vx:
		      case verinum::Vz:
			res = verinum::Vx;
			break;

		      case verinum::V0:
			if (res != verinum::Vx && rpad != verinum::V0)
			      res = ne_res;
			break;

		      case verinum::V1:
			if (res != verinum::Vx && rpad != verinum::V1)
			      res = ne_res;
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

		      case verinum::V0:
			if (res != verinum::Vx && lpad != verinum::V0)
			      res = ne_res;
			break;

		      case verinum::V1:
			if (res != verinum::Vx && lpad != verinum::V1)
			      res = ne_res;
			break;

		      default:
			break;
		  }
      }

      return new NetEConst(verinum(res));
}

NetEConst* NetEBComp::eval_eqeqeq_(bool ne_flag)
{
      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (lc == 0 || rc == 0) return 0;

      const verinum&lv = lc->value();
      const verinum&rv = rc->value();

      verinum::V res = verinum::V1;

	// Find the smallest argument length.
      unsigned cnt = lv.len();
      if (cnt > rv.len()) cnt = rv.len();

	// Check the common bits.
      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
	    if (lv.get(idx) != rv.get(idx)) {
		  res = verinum::V0;
		  break;
	    }

      bool is_signed = lv.has_sign() && rv.has_sign();

	// If the left value is longer check it against the pad bit.
      if (res == verinum::V1) {
	    verinum::V pad = verinum::V0;
	    if (is_signed) pad = rv.get(rv.len()-1);
	    for (unsigned idx = cnt ;  idx < lv.len() ;  idx += 1)
		  if (lv.get(idx) != pad) {
			res = verinum::V0;
			break;
		  }
      }

	// If the right value is longer check it against the pad bit.
      if (res == verinum::V1) {
	    verinum::V pad = verinum::V0;
	    if (is_signed) pad = lv.get(lv.len()-1);
	    for (unsigned idx = cnt ;  idx < rv.len() ;  idx += 1)
		  if (rv.get(idx) != pad) {
			res = verinum::V0;
			break;
		  }
      }

      if (ne_flag) {
	    if (res == verinum::V0) res = verinum::V1;
	    else res = verinum::V0;
      }

      NetEConst*result = new NetEConst(verinum(res, 1));
      ivl_assert(*this, result);
      result->set_line(*this);
      return result;
}

NetEConst* NetEBComp::eval_tree(int prune_to_width)
{
      eval_expr(left_);
      eval_expr(right_);

      switch (op_) {
	  case 'E': // Case equality (===)
	    return eval_eqeqeq_(false);

	  case 'e': // Equality (==)
	    return eval_eqeq_(false);

	  case 'G': // >=
	    return eval_gteq_();

	  case 'L': // <=
	    return eval_leeq_();

	  case 'N': // Case inequality (!==)
	    return eval_eqeqeq_(true);

	  case 'n': // not-equal (!=)
	    return eval_eqeq_(true);

	  case '<': // Less than
	    return eval_less_();

	  case '>': // Greater than
	    return eval_gt_();

	  default:
	    return 0;
      }
}

NetExpr* NetEBDiv::eval_tree_real_()
{
      verireal lval;
      verireal rval;

      bool flag = get_real_arguments_(lval, rval);
      if (! flag) return 0;

      NetECReal*res = 0;
      switch (op_) {
	  case '/':
	    res = new NetECReal(lval / rval);
	    break;

	  case '%':
	      // Since this could/may be called early we don't want to
	      // leak functionality.
	    if (!gn_icarus_misc_flag) return 0;
	    res = new NetECReal(lval % rval);
	    break;
      }
      ivl_assert(*this, res);
      res->set_line(*this);
      return res;
}

/*
 * The NetEBDiv operator includes the / and % operators. First evaluate
 * the sub-expressions, then perform the required operation.
 */
NetExpr* NetEBDiv::eval_tree(int prune_to_width)
{
      eval_expr(left_);
      eval_expr(right_);

      if (expr_type() == IVL_VT_REAL) return eval_tree_real_();

      assert(expr_type() == IVL_VT_LOGIC);

      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (lc == 0 || rc == 0) return 0;

	// Make sure the expression is evaluated at the
	// expression width.
      verinum lval = pad_to_width(lc->value(), expr_width());
      verinum rval = pad_to_width(rc->value(), expr_width());

      NetExpr*tmp = 0;
      switch (op_) {
	  case '/':
	    tmp = new NetEConst(lval / rval);
	    break;
	  case '%':
	    tmp = new NetEConst(lval % rval);
	    break;
      }
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEConst* NetEBLogic::eval_tree_real_()
{
      verireal lval;
      verireal rval;

      bool flag = get_real_arguments_(lval, rval);
      if (! flag) return 0;

      verinum::V res;
      switch (op_) {
	  case 'a': { // Logical AND (&&)
	    if ((lval.as_double() != 0.0) && (rval.as_double() != 0.0))
		  res = verinum::V1;
	    else
		  res = verinum::V0;
	    break;
	  }

	  case 'o': { // Logical OR (||)
	    if ((lval.as_double() != 0.0) || (rval.as_double() != 0.0))
		  res = verinum::V1;
	    else
		  res = verinum::V0;
	    break;
	  }

	  default:
	    return 0;
      }

      NetEConst*tmp = new NetEConst(verinum(res, 1));
      ivl_assert(*this, tmp);
      tmp->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated (real): " << *this
	         << " --> " << *tmp << endl;

      return tmp;
}

NetEConst* NetEBLogic::eval_tree(int prune_to_width)
{
      eval_expr(left_);
      eval_expr(right_);

      if (left_->expr_type() == IVL_VT_REAL ||
          right_->expr_type() == IVL_VT_REAL)
	    return eval_tree_real_();
      assert(expr_type() == IVL_VT_LOGIC);

      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (lc == 0 || rc == 0) return 0;

      verinum::V lv = verinum::V0;
      verinum::V rv = verinum::V0;

      verinum v = lc->value();
      for (unsigned idx = 0 ;  idx < v.len() ;  idx += 1)
	    if (v.get(idx) == verinum::V1) {
		  lv = verinum::V1;
		  break;
	    }

      if (lv == verinum::V0 && ! v.is_defined()) lv = verinum::Vx;

      v = rc->value();
      for (unsigned idx = 0 ;  idx < v.len() ;  idx += 1)
	    if (v.get(idx) == verinum::V1) {
		  rv = verinum::V1;
		  break;
	    }

      if (rv == verinum::V0 && ! v.is_defined()) rv = verinum::Vx;

      verinum::V res;
      switch (op_) {
	  case 'a': // Logical AND (&&)
	    if ((lv == verinum::V0) || (rv == verinum::V0))
		  res = verinum::V0;

	    else if ((lv == verinum::V1) && (rv == verinum::V1))
		  res = verinum::V1;

	    else
		  res = verinum::Vx;

	    break;

	  case 'o': // Logical OR (||)
	    if ((lv == verinum::V1) || (rv == verinum::V1))
		  res = verinum::V1;

	    else if ((lv == verinum::V0) && (rv == verinum::V0))
		  res = verinum::V0;

	    else
		  res = verinum::Vx;

	    break;

	  default:
	    return 0;
      }

      NetEConst*tmp = new NetEConst(verinum(res, 1));
      ivl_assert(*this, tmp);
      tmp->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated: " << *this
	         << " --> " << *tmp << endl;

      return tmp;
}


NetExpr* NetEBMult::eval_tree_real_()
{
      verireal lval;
      verireal rval;

      bool flag = get_real_arguments_(lval, rval);
      if (! flag) return 0;

      NetECReal*res = new NetECReal(lval * rval);
      res->set_line(*this);
      return res;
}

NetExpr* NetEBMult::eval_tree(int prune_to_width)
{
      eval_expr(left_);
      eval_expr(right_);

      if (expr_type() == IVL_VT_REAL) return eval_tree_real_();

      assert(expr_type() == IVL_VT_LOGIC);

      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (lc == 0 || rc == 0) return 0;

      verinum lval = lc->value();
      verinum rval = rc->value();

      NetEConst*tmp = new NetEConst(lval * rval);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluate "
		 << lval << " * " << rval << " --> " << *tmp << endl;

      return tmp;
}

NetExpr* NetEBPow::eval_tree_real_()
{
      verireal lval;
      verireal rval;

      bool flag = get_real_arguments_(lval, rval);
      if (! flag) return 0;

      NetECReal*res = new NetECReal( pow(lval,rval) );
      res->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluate (real) "
		 << lval << " ** " << rval << " --> " << *res << endl;

      return res;
}

NetExpr* NetEBPow::eval_tree(int prune_to_width)
{
      eval_expr(left_);
      eval_expr(right_);

      if (expr_type() == IVL_VT_REAL)
	    return eval_tree_real_();

      assert(expr_type() == IVL_VT_LOGIC);

      NetEConst*lc = dynamic_cast<NetEConst*>(left_);
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);
      if (lc == 0 || rc == 0) return 0;

      verinum lval = lc->value();
      verinum rval = rc->value();

      NetEConst*res = new NetEConst( pow(lval,rval) );
      res->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluate "
		 << lval << " ** " << rval << " --> " << *res << endl;

      return res;
}

/*
 * Evaluate the shift operator if possible. For this to work, both
 * operands must be constant.
 */
NetEConst* NetEBShift::eval_tree(int prune_to_width)
{
      eval_expr(left_);
      eval_expr(right_);

      NetEConst*le = dynamic_cast<NetEConst*>(left_);
      NetEConst*re = dynamic_cast<NetEConst*>(right_);
      if (le == 0 || re == 0) return 0;

      NetEConst*res;

      verinum rv = re->value();
      verinum lv = le->value();

	/* Make an early estimate of the expression width. */
      unsigned wid = expr_width();

      if (rv.is_defined()) {

	    unsigned shift = rv.as_ulong();

	    if (debug_eval_tree) {
		  cerr << get_fileline() << ": debug: "
		       << "Evaluate " << lv << "<<" << op() << ">> "
		       << rv << ", wid=" << wid << ", shift=" << shift
		       << ", lv.has_len()=" << lv.has_len() << endl;
	    }

	    if ((wid == 0) || ! lv.has_len()) {
		    /* If the caller doesn't care what the width is,
		       then calculate a width from the trimmed left
		       expression, plus the shift. This avoids
		       data loss. */
		  lv = trim_vnum(lv);
		  wid = lv.len();
		  if (op() == 'l')
			wid = lv.len() + shift;
	    }

	    if (prune_to_width > 0 && wid > (unsigned)prune_to_width)
		  wid = prune_to_width;

	    assert(wid);
	    verinum::V pad = verinum::V0;
	    if (op() == 'R' && has_sign()) {
		  pad = lv[lv.len()-1];
	    }
	    verinum nv (pad, wid, lv.has_len());

	    if (op() == 'r' || op() == 'R') {
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
	    if (wid == 0) wid = left_->expr_width();

	    verinum nv (verinum::Vx, wid);
	    res = new NetEConst(nv);
      }

      return res;
}

NetEConst* NetEConcat::eval_tree(int prune_to_width)
{
      unsigned repeat_val = repeat();
      unsigned local_errors = 0;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: Evaluate expr=" << *this
		 << ", prune_to_width=" << prune_to_width << endl;
      }

      unsigned gap = 0;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {

	      // Parameter not here? This is an error, but presumably
	      // already caught and we are here just to catch more.
	    if (parms_[idx] == 0) continue;

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
	    NetExpr*expr = parms_[idx]->eval_tree(0);
	    if (expr) {
		  expr->set_line(*parms_[idx]);
		  delete parms_[idx];
		  parms_[idx] = expr;

		  if (! expr->has_width()) {
			cerr << get_fileline() << ": error: concatenation "
			     << "operand has indefinite width: "
			     << *parms_[idx] << endl;
			local_errors += 1;
		  } else if (expr->expr_width() == 0) {
			cerr << expr->get_fileline() << ": internal error: "
			     << "Operand of concatenation has no width: "
			     << *expr << endl;
			local_errors += 1;
		  }

		  gap += expr->expr_width();
	    }

      }

      if (local_errors > 0) return 0;

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

	// Normally, concatenations are unsigned. However, the
	// $signed() function works by marking the expression as
	// signed, so we really have to check.
      val.has_sign( this->has_sign() );

      NetEConst*res = new NetEConst(val);
      res->set_width(val.len());
      return res;
}

NetExpr* NetEParam::eval_tree(int prune_to_width)
{
      if (des_ == 0) {
	    assert(scope_ == 0);
	    return 0;
      }

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: evaluating expression: "
		 << *this << endl;
      }

      if (solving()) {
	    cerr << get_fileline() << ": warning: Recursive parameter "
	            "reference found involving " << *this << "." << endl;
	    return 0;
      }

      assert(scope_);
      perm_string name = (*reference_).first;
      const NetExpr*expr = (*reference_).second.expr;
	// Since constant user functions are not supported we can get
	// parameters/localparams that are not defined. For now generate
	// an appropriate error message.
      if (expr == NULL) {
	    cerr << get_fileline() << ": internal error: parameter/localparam "
		 << *this << " cannot be evaluated." << endl;
	    return 0;
      }

	// If the parameter that I refer to is already evaluated, then
	// return the constant value.
      if (const NetEConst*tmp = dynamic_cast<const NetEConst*>(expr)) {
	    verinum val = tmp->value();
	    NetEConstParam*ptmp = new NetEConstParam(scope_, name, val);
	    ptmp->set_line(*this);
	    return ptmp;
      }

      if (const NetECReal*tmp = dynamic_cast<const NetECReal*>(expr)) {
	    verireal val = tmp->value();
	    NetECRealParam*ptmp = new NetECRealParam(scope_, name, val);
	    ptmp->set_line(*this);
	    return ptmp;
      }

	// Try to evaluate the expression. If I cannot, then the
	// expression is not a constant expression and I fail here.

      solving(true);
      NetExpr*nexpr = expr->dup_expr();
      assert(nexpr);
      NetExpr*res = nexpr->eval_tree();
      solving(false);
      if (res == 0) {
	    cerr << get_fileline() << ": internal error: Unable to evaluate ";
	    if (expr_type() == IVL_VT_REAL) cerr << "real ";
	    cerr << "parameter " << name << " expression: "
		 << *nexpr << endl;
	    delete nexpr;
	    return 0;
      }

	// The result can be saved as the value of the parameter for
	// future reference, and return a copy to the caller.
      bool flag = scope_->replace_parameter(name, res);
      if (!flag) {
	    cerr << get_fileline() << ": internal error: Could not "
		 << "replace parameter expression for " << name << endl;
	    return 0;
      }

	/* Return as a result a NetEConstParam or NetECRealParam
	   object, depending on the type of the expression. */

      switch (res->expr_type()) {

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    { NetEConst*tmp = dynamic_cast<NetEConst*>(res);
	      if (tmp == 0) {
		    cerr << get_fileline() << ": internal error: parameter "
			 << name << " evaluates to incomprehensible "
			 << *res << "." << endl;
		    return 0;
	      }

	      assert(tmp);

	      verinum val = tmp->value();
	      NetEConstParam*ptmp = new NetEConstParam(scope_, name, val);

	      return ptmp;
	    }

	  case IVL_VT_REAL:
	    { NetECReal*tmp = dynamic_cast<NetECReal*>(res);
	      if (tmp == 0) {
		    cerr << get_fileline() << ": internal error: parameter "
			 << name << " evaluates to incomprehensible "
			 << *res << "." << endl;
		    return 0;
	      }

	      assert(tmp);

	      verireal val = tmp->value();
	      NetECRealParam*ptmp = new NetECRealParam(scope_, name, val);

	      return ptmp;
	    }

	  default:
	    assert(0);
	    return 0;
      }
}

NetEConst* NetESelect::eval_tree(int prune_to_width)
{
      eval_expr(expr_);
      NetEConst*expr = dynamic_cast<NetEConst*>(expr_);

      long bval = 0;
      if (base_) {
	    eval_expr(base_);
	    NetEConst*base = dynamic_cast<NetEConst*>(base_);

	    if (base == 0) return 0;

	    bval = base->value().as_long();
      }

      if (expr == 0) return 0;

      verinum eval = expr->value();
      verinum oval (verinum::V0, expr_width(), true);

      verinum::V pad_bit = verinum::Vx;
      if (base_ == 0) {

	      /* If the base is NULL (different from 0) the this
		 select is here for sign extension. So calculate a
		 proper pad bit. Extend x or z or 0, and sign extend 1
		 if this is signed. */
	    unsigned top = expr->expr_width()-1;

	    pad_bit = eval.get(top);
	    if (pad_bit==verinum::V1 && !has_sign())
		  pad_bit = verinum::V0;
      }

      for (unsigned long idx = 0 ;  idx < expr_width() ;  idx += 1) {
	    if ((bval >= 0) && ((unsigned long) bval < eval.len()))
		  oval.set(idx, eval.get(bval));
	    else
		  oval.set(idx, pad_bit);

	    bval += 1;
      }

      oval.has_sign(has_sign());

      NetEConst*res = new NetEConst(oval);
      return res;
}


static void print_ternary_cond(NetExpr*expr)
{
      if (NetEConst*c = dynamic_cast<NetEConst*>(expr)) {
	    cerr << c->value() << endl;
	    return;
      }
      if (NetECReal*c = dynamic_cast<NetECReal*>(expr)) {
	    cerr << c->value() << endl;
	    return;
      }
      assert(0);
}

/*
 * A ternary expression evaluation is controlled by the condition
 * expression. If the condition evaluates to true or false, then
 * return the evaluated true or false expression. If the condition
 * evaluates to x or z, then merge the constant bits of the true and
 * false expressions.
 */
NetExpr* NetETernary::eval_tree(int prune_to_width)
{
      eval_expr(cond_);
      switch (const_logical(cond_)) {
	  case C_0:
	    eval_expr(false_val_);
	    if (debug_eval_tree) {

		  cerr << get_fileline() << ": debug: Evaluate ternary with "
		       << "constant condition value: ";
		  print_ternary_cond(cond_);
		  cerr << get_fileline() << ":      : Selecting false case: "
		       << *false_val_ << endl;
	    }

	    if (expr_type() == IVL_VT_REAL &&
	        false_val_->expr_type() != IVL_VT_REAL) {
		  verireal f;
		  if (get_real_arg_(false_val_, f)) {
			NetECReal*rc = new NetECReal(f);
			rc->set_line(*this);
			return rc;
		  }
	    }

	    return false_val_->dup_expr();

	  case C_1:
	    eval_expr(true_val_);
	    if (debug_eval_tree) {
		  cerr << get_fileline() << ": debug: Evaluate ternary with "
		       << "constant condition value: ";
		  print_ternary_cond(cond_);
		  cerr << get_fileline() << ":      : Selecting true case: "
		       << *true_val_ << endl;
	    }

	    if (expr_type() == IVL_VT_REAL &&
	        true_val_->expr_type() != IVL_VT_REAL) {
		  verireal t;
		  if (get_real_arg_(true_val_, t)) {
			NetECReal*rc = new NetECReal(t);
			rc->set_line(*this);
			return rc;
		  }
	    }

	    return true_val_->dup_expr();

	  case C_X:
	    break;

	  default:
	    return 0;
      }

	/* Here we have a more complex case. We need to evaluate both
	   expressions down to constants then compare the values to
	   build up a constant result. */

      eval_expr(true_val_);
      eval_expr(false_val_);

      NetEConst*t = dynamic_cast<NetEConst*>(true_val_);
      NetEConst*f = dynamic_cast<NetEConst*>(false_val_);
      if (t == 0 || f == 0) {
	    verireal tv, fv;
	    if (!get_real_arg_(true_val_, tv)) return 0;
	    if (!get_real_arg_(false_val_, fv)) return 0;

	    verireal val = verireal(0.0);
	    if (tv.as_double() == fv.as_double()) val = tv;

	    if (debug_eval_tree) {
		  cerr << get_fileline() << ": debug: Evaluate ternary with "
		       << "constant condition value: ";
		  print_ternary_cond(cond_);
		  cerr << get_fileline() << ":      : Blending real cases "
		       << "true=" << tv.as_double()
		       << ", false=" << fv.as_double()
		       << ", to get " << val << endl;
	    }

	    NetECReal*rc = new NetECReal(val);
	    rc->set_line(*this);
	    return rc;
      }

      unsigned tsize = t->expr_width();
      unsigned fsize = f->expr_width();
	/* Size of the result is the size of the widest operand. */
      unsigned rsize = tsize > fsize? tsize : fsize;

      verinum val (verinum::V0, rsize);
      for (unsigned idx = 0 ;  idx < rsize ;  idx += 1) {
	    verinum::V tv = idx < tsize? t->value().get(idx) : verinum::V0;
	    verinum::V fv = idx < fsize? f->value().get(idx) : verinum::V0;

	    if (tv == fv) val.set(idx, tv);
	    else val.set(idx, verinum::Vx);
      }

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: Evaluate ternary with "
		 << "constant condition value: ";
	    print_ternary_cond(cond_);
	    cerr << get_fileline() << ":      : Blending cases to get "
		 << val << endl;
      }

      NetEConst*rc = new NetEConst(val);
      rc->set_line(*this);
      return rc;
}

NetExpr* NetEUnary::eval_tree_real_()
{
      NetECReal*val= dynamic_cast<NetECReal*> (expr_), *res;
      if (val == 0) return 0;

      switch (op_) {
	  case '+':
	    res = new NetECReal(val->value());
	    res->set_line(*this);
	    return res;

	  case '-':
	    res = new NetECReal(-(val->value()));
	    res->set_line(*this);
	    return res;

	  default:
	    return 0;
      }
}

NetExpr* NetEUnary::eval_tree(int prune_to_width)
{
      eval_expr(expr_);
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_();

      NetEConst*rval = dynamic_cast<NetEConst*>(expr_);
      if (rval == 0) return 0;

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


NetExpr* NetEUBits::eval_tree(int prune_to_width)
{
      return NetEUnary::eval_tree(prune_to_width);
}

NetEConst* NetEUReduce::eval_tree_real_()
{
      ivl_assert(*this, op_ == '!');

      NetECReal*val= dynamic_cast<NetECReal*> (expr_);
      if (val == 0) return 0;

      verinum::V res = val->value().as_double() == 0.0 ? verinum::V1 :
                                                         verinum::V0;

      return new NetEConst(verinum(res, 1));
}


NetEConst* NetEUReduce::eval_tree(int prune_to_width)
{
      eval_expr(expr_);
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_();

      NetEConst*rval = dynamic_cast<NetEConst*>(expr_);
      if (rval == 0) return 0;

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

NetExpr* evaluate_clog2(NetExpr*&arg_)
{
      eval_expr(arg_);
      NetEConst*tmpi = dynamic_cast<NetEConst *>(arg_);
      NetECReal*tmpr = dynamic_cast<NetECReal *>(arg_);
      if (tmpi || tmpr) {
	    verinum arg;
	    if (tmpi) {
		  arg = tmpi->value();
	    } else {
		  arg = verinum(tmpr->value().as_double(), true);
	    }

	      /* If we have an x in the verinum we return 'bx. */
	    if (!arg.is_defined()) {
		  verinum tmp (verinum::Vx, integer_width);
		  tmp.has_sign(true);
		  NetEConst*rtn = new NetEConst(tmp);
		  return rtn;
	    }

	    bool is_neg = false;
	    uint64_t res = 0;
	    if (arg.is_negative()) {
		  is_neg = true;
		    // If the length is not defined, then work with
		    // the trimmed version of the number.
		  if (! arg.has_len())
			arg = trim_vnum(arg);
	    }
	    arg.has_sign(false);  // $unsigned()

	    if (!arg.is_zero()) {
		  arg = arg - verinum((uint64_t)1, 1);
		  while (!arg.is_zero()) {
			res += 1;
			arg = arg >> 1;
		  }
	    }

	    if (is_neg && res < integer_width)
		  res = integer_width;

	    verinum tmp (res, integer_width);
	    tmp.has_sign(true);
	    NetEConst*rtn = new NetEConst(tmp);
	    return rtn;
      }

      return 0;
}

NetExpr* evaluate_math_one_arg(NetExpr*&arg_, const char*name)
{
      eval_expr(arg_);
      NetEConst*tmpi = dynamic_cast<NetEConst *>(arg_);
      NetECReal*tmpr = dynamic_cast<NetECReal *>(arg_);
      if (tmpi || tmpr) {
	    double arg;
	    if (tmpi) {
		  arg = tmpi->value().as_double();
	    } else {
		  arg = tmpr->value().as_double();
	    }

	    if (strcmp(name, "$ln") == 0) {
		  return new NetECReal(verireal(log(arg)));
	    } else if (strcmp(name, "$log10") == 0) {
		  return new NetECReal(verireal(log10(arg)));
	    } else if (strcmp(name, "$exp") == 0) {
		  return new NetECReal(verireal(exp(arg)));
	    } else if (strcmp(name, "$sqrt") == 0) {
		  return new NetECReal(verireal(sqrt(arg)));
	    } else if (strcmp(name, "$floor") == 0) {
		  return new NetECReal(verireal(floor(arg)));
	    } else if (strcmp(name, "$ceil") == 0) {
		  return new NetECReal(verireal(ceil(arg)));
	    } else if (strcmp(name, "$sin") == 0) {
		  return new NetECReal(verireal(sin(arg)));
	    } else if (strcmp(name, "$cos") == 0) {
		  return new NetECReal(verireal(cos(arg)));
	    } else if (strcmp(name, "$tan") == 0) {
		  return new NetECReal(verireal(tan(arg)));
	    } else if (strcmp(name, "$asin") == 0) {
		  return new NetECReal(verireal(asin(arg)));
	    } else if (strcmp(name, "$acos") == 0) {
		  return new NetECReal(verireal(acos(arg)));
	    } else if (strcmp(name, "$atan") == 0) {
		  return new NetECReal(verireal(atan(arg)));
	    } else if (strcmp(name, "$sinh") == 0) {
		  return new NetECReal(verireal(sinh(arg)));
	    } else if (strcmp(name, "$cosh") == 0) {
		  return new NetECReal(verireal(cosh(arg)));
	    } else if (strcmp(name, "$tanh") == 0) {
		  return new NetECReal(verireal(tanh(arg)));
	    } else if (strcmp(name, "$asinh") == 0) {
		  return new NetECReal(verireal(asinh(arg)));
	    } else if (strcmp(name, "$acosh") == 0) {
		  return new NetECReal(verireal(acosh(arg)));
	    } else if (strcmp(name, "$atanh") == 0) {
		  return new NetECReal(verireal(atanh(arg)));
	    }
      }

      return 0;
}

NetExpr* evaluate_math_two_args(NetExpr*&arg0_, NetExpr*&arg1_, const char*name)
{
      eval_expr(arg0_);
      eval_expr(arg1_);
      NetEConst*tmpi0 = dynamic_cast<NetEConst *>(arg0_);
      NetECReal*tmpr0 = dynamic_cast<NetECReal *>(arg0_);
      NetEConst*tmpi1 = dynamic_cast<NetEConst *>(arg1_);
      NetECReal*tmpr1 = dynamic_cast<NetECReal *>(arg1_);
      if ((tmpi0 || tmpr0) && (tmpi1 || tmpr1)) {
	    double arg0, arg1;
	    if (tmpi0) {
		  arg0 = tmpi0->value().as_double();
	    } else {
		  arg0 = tmpr0->value().as_double();
	    }
	    if (tmpi1) {
		  arg1 = tmpi1->value().as_double();
	    } else {
		  arg1 = tmpr1->value().as_double();
	    }

	    if (strcmp(name, "$pow") == 0) {
		  return new NetECReal(verireal(pow(arg0, arg1)));
	    } else if (strcmp(name, "$atan2") == 0) {
		  return new NetECReal(verireal(atan2(arg0, arg1)));
	    } else if (strcmp(name, "$hypot") == 0) {
		  return new NetECReal(verireal(hypot(arg0, arg1)));
	    }
      }

      return 0;
}

NetExpr* evaluate_abs(NetExpr*&arg_)
{
      eval_expr(arg_);
      NetEConst*tmpi = dynamic_cast<NetEConst *>(arg_);
      if (tmpi) {
	    verinum arg = tmpi->value();
	    if (arg.is_negative()) {
		  arg = v_not(arg) + verinum(1);
	    }
	    return new NetEConst(arg);
      }

      NetECReal*tmpr = dynamic_cast<NetECReal *>(arg_);
      if (tmpr) {
	    double arg = tmpr->value().as_double();
	    return new NetECReal(verireal(fabs(arg)));
      }

      return 0;
}

NetExpr* evaluate_min_max(NetExpr*&arg0_, NetExpr*&arg1_, const char*name)
{
      eval_expr(arg0_);
      eval_expr(arg1_);
      NetEConst*tmpi0 = dynamic_cast<NetEConst *>(arg0_);
      NetECReal*tmpr0 = dynamic_cast<NetECReal *>(arg0_);
      NetEConst*tmpi1 = dynamic_cast<NetEConst *>(arg1_);
      NetECReal*tmpr1 = dynamic_cast<NetECReal *>(arg1_);
      if (tmpi0 && tmpi1) {
	    verinum arg0 = tmpi0->value();
	    verinum arg1 = tmpi1->value();
	    if (strcmp(name, "$min") == 0) {
		  return new NetEConst( arg0 < arg1 ? arg0 : arg1);
	    } else if (strcmp(name, "$max") == 0) {
		  return new NetEConst( arg0 < arg1 ? arg1 : arg0);
	    }
      }

      if ((tmpi0 || tmpr0) && (tmpi1 || tmpr1)) {
	    double arg0, arg1;
	    if (tmpi0) {
		  arg0 = tmpi0->value().as_double();
	    } else {
		  arg0 = tmpr0->value().as_double();
	    }
	    if (tmpi1) {
		  arg1 = tmpi1->value().as_double();
	    } else {
		  arg1 = tmpr1->value().as_double();
	    }
	    if (strcmp(name, "$min") == 0) {
		  return new NetECReal(verireal(arg0 < arg1 ? arg0 : arg1));
	    } else if (strcmp(name, "$max") == 0) {
		  return new NetECReal(verireal(arg0 < arg1 ? arg1 : arg0));
	    }
      }

      return 0;
}

NetExpr* NetESFunc::eval_tree(int prune_to_width)
{
	/* If we are not targeting at least Verilog-2005, Verilog-AMS
	 * or using the Icarus misc flag then we do not support these
	 * functions as constant. */
      if (generation_flag < GN_VER2005 &&
          !gn_icarus_misc_flag && !gn_verilog_ams_flag) {
	    return 0;
      }

      const char*nm = name();
      NetExpr*rtn = 0;
	/* Only $clog2 and the builtin mathematical functions can
	 * be a constant system function. */
      if (strcmp(nm, "$clog2") == 0 ||
          strcmp(nm, "$ln") == 0 ||
          strcmp(nm, "$log10") == 0 ||
          strcmp(nm, "$exp") == 0 ||
          strcmp(nm, "$sqrt") == 0 ||
          strcmp(nm, "$floor") == 0 ||
          strcmp(nm, "$ceil") == 0 ||
          strcmp(nm, "$sin") == 0 ||
          strcmp(nm, "$cos") == 0 ||
          strcmp(nm, "$tan") == 0 ||
          strcmp(nm, "$asin") == 0 ||
          strcmp(nm, "$acos") == 0 ||
          strcmp(nm, "$atan") == 0 ||
          strcmp(nm, "$sinh") == 0 ||
          strcmp(nm, "$cosh") == 0 ||
          strcmp(nm, "$tanh") == 0 ||
          strcmp(nm, "$asinh") == 0 ||
          strcmp(nm, "$acosh") == 0 ||
          strcmp(nm, "$atanh") == 0) {
	    if (nparms() != 1 || parm(0) == 0) {
		  cerr << get_fileline() << ": error: " << nm
		       << " takes a single argument." << endl;
		  return 0;
	    }
	    NetExpr*arg = parm(0)->dup_expr();
	    if (strcmp(nm, "$clog2") == 0) {
		  rtn = evaluate_clog2(arg);
	    } else {
		  rtn = evaluate_math_one_arg(arg, nm);
	    }
	    delete arg;
      }

      if (strcmp(nm, "$pow") == 0 ||
          strcmp(nm, "$atan2") == 0 ||
          strcmp(nm, "$hypot") == 0) {
	    if (nparms() != 2 || parm(0) == 0 || parm(1) == 0) {
		  cerr << get_fileline() << ": error: " << nm
		       << " takes two arguments." << endl;
		  return 0;
	    }
	    NetExpr*arg0 = parm(0)->dup_expr();
	    NetExpr*arg1 = parm(1)->dup_expr();
	    rtn = evaluate_math_two_args(arg0, arg1, nm);
	    delete arg0;
	    delete arg1;
      }

      if ((gn_icarus_misc_flag || gn_verilog_ams_flag) &&
          (strcmp(nm, "$abs") == 0)) {
	    if (nparms() != 1 || parm(0) == 0) {
		  cerr << get_fileline() << ": error: " << nm
		       << " takes a single argument." << endl;
		  return 0;
	    }
	    NetExpr*arg = parm(0)->dup_expr();
	    rtn = evaluate_abs(arg);
	    delete arg;
      }

      if ((gn_icarus_misc_flag || gn_verilog_ams_flag) &&
          (strcmp(nm, "$min") == 0 || strcmp(nm, "$max") == 0)) {
	    if (nparms() != 2 || parm(0) == 0 || parm(1) == 0) {
		  cerr << get_fileline() << ": error: " << nm
		       << " takes two arguments." << endl;
		  return 0;
	    }
	    NetExpr*arg0 = parm(0)->dup_expr();
	    NetExpr*arg1 = parm(1)->dup_expr();
	    rtn = evaluate_min_max(arg0, arg1, nm);
	    delete arg0;
	    delete arg1;
      }

      if (rtn != 0) {
	    rtn->set_line(*this);
	    if (debug_eval_tree) {
		  cerr << get_fileline() << ": debug: Evaluate constant "
		       << nm << "." << endl;
	    }
      }

      return rtn;
}

NetExpr* NetEUFunc::eval_tree(int prune_to_width)
{
      if (need_constant_expr) {
	    cerr << get_fileline() << ": sorry: constant user "
	            "functions are not currently supported: "
	         << func_->basename() << "()." << endl;

      }

      return 0;
}
