/*
 * Copyright (c) 1999-2011 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

NetExpr* NetExpr::eval_tree()
{
      return 0;
}

static bool get_real_arg_(const NetExpr*expr, verireal&val)
{
      switch (expr->expr_type()) {
	  case IVL_VT_REAL: {
		const NetECReal*c = dynamic_cast<const NetECReal*> (expr);
		if (c == 0) return false;
		val = c->value();
		break;
	  }

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC: {
		const NetEConst*c = dynamic_cast<const NetEConst*>(expr);
		if (c == 0) return false;
		verinum tmp = c->value();
		val = verireal(tmp.as_double());
		break;
	  }

	  case IVL_VT_DARRAY:
	    return false;

	  default:
	    assert(0);
      }

      return true;
}

static bool get_real_arguments(const NetExpr*le, const NetExpr*re,
                               double&lval, double&rval)
{
      verireal val;

      if (!get_real_arg_(le, val)) return false;
      lval = val.as_double();

      if (!get_real_arg_(re, val)) return false;
      rval = val.as_double();

      return true;
}

bool NetEBinary::get_real_arguments_(verireal&lval, verireal&rval)
{
      if (!get_real_arg_(left_, lval)) return false;
      if (!get_real_arg_(right_, rval)) return false;

      return true;
}

NetECReal* NetEBAdd::eval_tree_real_(const NetExpr*l, const NetExpr*r) const
{
      double lval;
      double rval;

      bool flag = get_real_arguments(l, r, lval, rval);
      if (!flag) return 0;

      double res_val;

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

      NetECReal*res = new NetECReal( verireal(res_val) );
      ivl_assert(*this, res);
      res->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated (real): " << *this
	         << " --> " << *res << endl;

      return res;
}

NetExpr* NetEBAdd::eval_tree()
{
      eval_expr(left_);
      eval_expr(right_);

	// First try to elaborate the expression completely.
      NetExpr*res = eval_arguments_(left_,right_);
      if (res != 0)
	    return res;

	// If the expression type is real, then do not attempt the
	// following alternative processing.
      if (expr_type() == IVL_VT_REAL)
	    return 0;

	// The expression has not evaluated to a constant. Let's still
	// try to optimize by trying to combine a right constant value
	// with the right constant value of a sub-expression add. For
	// example, the expression (a + 2) - 1 can be rewritten as a + 1.

      NetEBAdd*se = dynamic_cast<NetEBAdd*>(left_);
      NetEConst*lc = se? dynamic_cast<NetEConst*>(se->right_) : 0;
      NetEConst*rc = dynamic_cast<NetEConst*>(right_);

      if (lc != 0 && rc != 0) {
	    ivl_assert(*this, se != 0);

	    if (debug_eval_tree) {
		  cerr << get_fileline() << ": debug: "
		       << "Partially evaluate " << *this
		       << " using (a+2)-1 --> (a+1) transform." << endl;
	    }

	    verinum lval = lc->value();
	    verinum rval = rc->value();

            unsigned wid = expr_width();
            ivl_assert(*this, wid > 0);
            ivl_assert(*this, lval.len() == wid);
            ivl_assert(*this, rval.len() == wid);

	    verinum val;
	    if (op_ == se->op_) {
		    /* (a + lval) + rval  --> a + (rval+lval) */
		    /* (a - lval) - rval  --> a - (rval+lval) */
		  val = verinum(rval + lval, wid);
	    } else {
		    /* (a - lval) + rval  -->  a + (rval-lval) */
		    /* (a + lval) - rval  -->  a - (rval-lval) */
		  val = verinum(rval - lval, wid);
	    }

	    NetEConst*tmp = new NetEConst(val);
	    left_ = se->left_->dup_expr();
	    delete se;
	    tmp->set_line(*right_);
	    delete right_;
	    right_ = tmp;
      }

	// We may have changed the subexpression, but the result is
	// still not constant, so return nil here anyhow.
      return 0;
}

NetExpr* NetEBAdd::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
      if (expr_type() == IVL_VT_REAL)
	    return eval_tree_real_(l,r);

      const NetEConst*lc = dynamic_cast<const NetEConst*>(l);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(r);

	/* If both operands are constant, then replace the entire
	   expression with a constant value. */
      if (lc != 0 && rc != 0) {
	    verinum lval = lc->value();
	    verinum rval = rc->value();

            unsigned wid = expr_width();
            ivl_assert(*this, wid > 0);
            ivl_assert(*this, lval.len() == wid);
            ivl_assert(*this, rval.len() == wid);

	    verinum val;
	    switch (op_) {
		case '+':
		  val = verinum(lval + rval, wid);
		  break;
		case '-':
		  val = verinum(lval - rval, wid);
		  break;
		default:
		  return 0;
	    }

	    NetEConst *res = new NetEConst(val);
	    ivl_assert(*this, res);
	    res->set_line(*this);

	    if (debug_eval_tree)
		  cerr << get_fileline() << ": debug: Evaluated: " << *this
		  << " --> " << *res << endl;

	    return res;
      }


	/* Nothing more to be done, the value is not constant. */
      return 0;
}

NetEConst* NetEBBits::eval_tree()
{
      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: Evaluating expression:"
	         << *this << endl;
      }

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

      unsigned wid = expr_width();
      ivl_assert(*this, wid > 0);
      ivl_assert(*this, lval.len() == wid);
      ivl_assert(*this, rval.len() == wid);

      verinum res (verinum::V0, wid);

      switch (op()) {

	  case '|': {
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		      res.set(idx, lval.get(idx) | rval.get(idx));

		break;
	  }

	  case '&': {
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		      res.set(idx, lval.get(idx) & rval.get(idx));

		break;
	  }

	  case '^': {
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		      res.set(idx, lval.get(idx) ^ rval.get(idx));

		break;
	  }

	  default:
	    return 0;
      }

      return new NetEConst(res);
}

NetEConst* NetEBComp::eval_less_(const NetExpr*le, const NetExpr*re) const
{
      if (le->expr_type() == IVL_VT_REAL || re->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(le, re, false);

      const NetEConst*rc = dynamic_cast<const NetEConst*>(re);
      if (rc == 0) return 0;

      verinum rv = rc->value();
      if (! rv.is_defined()) {
	    NetEConst*res = new NetEConst(verinum(verinum::Vx, 1));
	    ivl_assert(*this, res);
	    return res;
      }

      if (NetEConst*tmp = must_be_leeq_(le, rv, false)) {
	    return tmp;
      }

	/* Now go on to the normal test of the values. */
      const NetEConst*lc = dynamic_cast<const NetEConst*>(le);
      if (lc == 0) return 0;

      verinum lv = lc->value();
      if (! lv.is_defined()) {
	    NetEConst*res = new NetEConst(verinum(verinum::Vx, 1));
	    ivl_assert(*this, res);
	    return res;
      }

      if (lv < rv) {
	    NetEConst*res = new NetEConst(verinum(verinum::V1, 1));
	    ivl_assert(*this, res);
	    return res;
      } else {
	    NetEConst*res = new NetEConst(verinum(verinum::V0, 1));
	    ivl_assert(*this, res);
	    return res;
      }
}

NetEConst* NetEBComp::must_be_leeq_(const NetExpr*le, const verinum&rv, bool eq_flag) const
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
	    NetEConst*res = new NetEConst(verinum(verinum::V1, 1));
	    ivl_assert(*this, res);
	    return res;
      }

      return 0;
}

NetEConst* NetEBComp::eval_leeq_real_(const NetExpr*le, const NetExpr*re, bool eq_flag) const
{
      double lval;
      double rval;

      bool flag = get_real_arguments(le, re, lval, rval);
      if (! flag) return 0;

      bool tmp = false;
      if (lval < rval) tmp = true;
      if (tmp == false && eq_flag && lval == rval) tmp = true;

      verinum result(tmp ? verinum::V1 : verinum::V0, 1);
      NetEConst*res = new NetEConst(result);
      ivl_assert(*this, res);

      return res;
}

NetEConst* NetEBComp::eval_leeq_(const NetExpr*le, const NetExpr*re) const
{
      if (le->expr_type() == IVL_VT_REAL || re->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(le, re, true);
//      assert(expr_type() == IVL_VT_LOGIC);

      const NetEConst*r = dynamic_cast<const NetEConst*>(re);
      if (r == 0) return 0;

      verinum rv = r->value();
      if (! rv.is_defined()) {
	    NetEConst*res = new NetEConst(verinum(verinum::Vx, 1));
	    ivl_assert(*this, res);
	    return res;
      }

      if (le->expr_width() == 0) {
	    cerr << get_fileline() << ": internal error: Something wrong "
		 << "with the left side width of <= ?" << endl;
	    cerr << get_fileline() << ":               : " << *this << endl;
      }

      if (NetEConst*tmp = must_be_leeq_(le, rv, true)) {
	    return tmp;
      }

	/* Now go on to the normal test of the values. */
      const NetEConst*l = dynamic_cast<const NetEConst*>(le);
      if (l == 0) return 0;

      verinum lv = l->value();
      if (! lv.is_defined()) {
	    NetEConst*res = new NetEConst(verinum(verinum::Vx, 1));
	    ivl_assert(*this, res);
	    return res;
      }

      if (lv <= rv) {
	    NetEConst*res = new NetEConst(verinum(verinum::V1, 1));
	    ivl_assert(*this, res);
	    return res;
      } else {
	    NetEConst*res = new NetEConst(verinum(verinum::V0, 1));
	    ivl_assert(*this, res);
	    return res;
      }
}

NetEConst* NetEBComp::eval_gt_(const NetExpr*le, const NetExpr*re) const
{
      if (le->expr_type() == IVL_VT_REAL || re->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(re, le, false);

      const NetEConst*l = dynamic_cast<const NetEConst*>(le);
      if (l == 0) return 0;

      verinum lv = l->value();
      if (! lv.is_defined()) {
	    NetEConst*res = new NetEConst(verinum(verinum::Vx, 1));
	    ivl_assert(*this, res);
	    return res;
      }

      if (NetEConst*tmp = must_be_leeq_(re, lv, false)) {
	    return tmp;
      }

	/* Now go on to the normal test of the values. */
      const NetEConst*r = dynamic_cast<const NetEConst*>(re);
      if (r == 0) return 0;

      verinum rv = r->value();
      if (! rv.is_defined()) {
	    NetEConst*res = new NetEConst(verinum(verinum::Vx, 1));
	    ivl_assert(*this, res);
	    return res;
      }

      if (lv > rv) {
	    NetEConst*res = new NetEConst(verinum(verinum::V1, 1));
	    ivl_assert(*this, res);
	    return res;
      } else {
	    NetEConst*res = new NetEConst(verinum(verinum::V0, 1));
	    ivl_assert(*this, res);
	    return res;
      }
}

NetEConst* NetEBComp::eval_gteq_(const NetExpr*le, const NetExpr*re) const
{
      if (le->expr_type() == IVL_VT_REAL || re->expr_type() == IVL_VT_REAL)
	    return eval_leeq_real_(re, le, true);

      const NetEConst*l = dynamic_cast<const NetEConst*>(left_);
      if (l == 0) return 0;

      verinum lv = l->value();
      if (! lv.is_defined()) {
	    NetEConst*res = new NetEConst(verinum(verinum::Vx, 1));
	    ivl_assert(*this, res);
	    return res;
      }

      if (NetEConst*tmp = must_be_leeq_(re, lv, true)) {
	    return tmp;
      }

	/* Now go on to the normal test of the values. */
      const NetEConst*r = dynamic_cast<const NetEConst*>(re);
      if (r == 0) return 0;

      verinum rv = r->value();
      if (! rv.is_defined()) {
	    NetEConst*res = new NetEConst(verinum(verinum::Vx, 1));
	    ivl_assert(*this, res);
	    return res;
      }

      if (lv >= rv) {
	    NetEConst*res = new NetEConst(verinum(verinum::V1, 1));
	    ivl_assert(*this, res);
	    return res;
      } else {
	    NetEConst*res = new NetEConst(verinum(verinum::V0, 1));
	    ivl_assert(*this, res);
	    return res;
      }
}

/*
 * Evaluate <A>==<B> or <A>!=<B>. The equality operator checks all the
 * bits and returns true(false) if there are any bits in the vector
 * that are defined (0 or 1) and different. If all the defined bits
 * are equal, but there are are x/z bits, then the situation is
 * ambiguous so the result is x.
 */
NetEConst* NetEBComp::eval_eqeq_real_(bool ne_flag, const NetExpr*le, const NetExpr*re) const
{
      double lval;
      double rval;

      bool flag = get_real_arguments(le, re, lval, rval);
      if (! flag) return 0;

      verinum result(((lval == rval) ^ ne_flag) ?
                     verinum::V1 : verinum::V0, 1);
      NetEConst*res = new NetEConst(result);
      ivl_assert(*this, res);

      return res;
}

NetEConst* NetEBComp::eval_eqeq_(bool ne_flag, const NetExpr*le, const NetExpr*re) const
{
      if (le->expr_type() == IVL_VT_REAL ||
          re->expr_type() == IVL_VT_REAL)
	    return eval_eqeq_real_(ne_flag, le, re);

      const NetEConst*lc = dynamic_cast<const NetEConst*>(le);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(re);
      if (lc == 0 || rc == 0) return 0;

      const verinum&lv = lc->value();
      const verinum&rv = rc->value();

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

      NetEConst*result = new NetEConst(verinum(res, 1));
      ivl_assert(*this, result);
      return result;
}

NetEConst* NetEBComp::eval_eqeqeq_(bool ne_flag, const NetExpr*le, const NetExpr*re) const
{
      const NetEConst*lc = dynamic_cast<const NetEConst*>(le);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(re);
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
	    for (unsigned idx = cnt ;  idx < rv.len() ;  idx += 1) {
		  if (rv.get(idx) != pad)
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
      return result;
}

NetEConst* NetEBComp::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
      NetEConst*res = 0;

      switch (op_) {
	  case 'E': // Case equality (===)
	    res = eval_eqeqeq_(false, l, r);
	    break;

	  case 'e': // Equality (==)
	    res = eval_eqeq_(false, l, r);
	    break;

	  case 'G': // >=
	    res = eval_gteq_(l, r);
	    break;

	  case 'L': // <=
	    res = eval_leeq_(l, r);
	    break;

	  case 'N': // Case inequality (!==)
	    res = eval_eqeqeq_(true, l, r);
	    break;

	  case 'n': // not-equal (!=)
	    res = eval_eqeq_(true, l, r);
	    break;

	  case '<': // Less than
	    res = eval_less_(l, r);
	    break;

	  case '>': // Greater than
	    res = eval_gt_(l, r);
	    break;

      }

      return res;
}

NetEConst* NetEBComp::eval_tree()
{
      eval_expr(left_);
      eval_expr(right_);

      NetEConst*res = eval_arguments_(left_, right_);
      if (res == 0) return 0;
      res->set_line(*this);

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: Evaluated";
	    if (left_->expr_type() == IVL_VT_REAL ||
	        right_->expr_type() == IVL_VT_REAL)
		  cerr << " (real)";
	    cerr << ": " << *this << " --> " << *res << endl;
      }
      return res;
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

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated (real): " << *this
	         << " --> " << *res << endl;

      return res;
}

/*
 * The NetEBDiv operator includes the / and % operators. First evaluate
 * the sub-expressions, then perform the required operation.
 */
NetExpr* NetEBDiv::eval_tree()
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

      unsigned wid = expr_width();
      ivl_assert(*this, wid > 0);
      ivl_assert(*this, lval.len() == wid);
      ivl_assert(*this, rval.len() == wid);

      verinum val;
      switch (op_) {
	  case '/':
	    val = verinum(lval / rval, wid);
	    break;
	  case '%':
	    val = verinum(lval % rval, wid);
	    break;
	  default:
	    return 0;
      }
      NetExpr*tmp = new NetEConst(val);
      ivl_assert(*this, tmp);
      tmp->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated: " << *this
	         << " --> " << *tmp << endl;

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
	  case 'a': // Logical AND (&&)
	    if ((lval.as_double() != 0.0) && (rval.as_double() != 0.0))
		  res = verinum::V1;
	    else
		  res = verinum::V0;
	    break;

	  case 'o': // Logical OR (||)
	    if ((lval.as_double() != 0.0) || (rval.as_double() != 0.0))
		  res = verinum::V1;
	    else
		  res = verinum::V0;
	    break;

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

NetEConst* NetEBLogic::eval_tree()
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


NetExpr* NetEBMult::eval_tree_real_(const NetExpr*l, const NetExpr*r) const
{
      double lval;
      double rval;

      bool flag = get_real_arguments(l, r, lval, rval);
      if (! flag) return 0;

      NetECReal*res = new NetECReal( verireal(lval * rval) );
      ivl_assert(*this, res);
      res->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated (real): " << *this
	         << " --> " << *res << endl;

      return res;
}

NetExpr* NetEBMult::eval_tree()
{
      eval_expr(left_);
      eval_expr(right_);

      return eval_arguments_(left_, right_);
}

NetExpr* NetEBMult::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_(l,r);
      assert(expr_type() == IVL_VT_LOGIC);

      const NetEConst*lc = dynamic_cast<const NetEConst*>(l);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(r);
      if (lc == 0 || rc == 0) return 0;

      verinum lval = lc->value();
      verinum rval = rc->value();

      unsigned wid = expr_width();
      ivl_assert(*this, wid > 0);
      ivl_assert(*this, lval.len() == wid);
      ivl_assert(*this, rval.len() == wid);

      verinum val(lval * rval, wid);
      NetEConst*tmp = new NetEConst(val);
      ivl_assert(*this, tmp);
      tmp->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated: " << *this
	         << " --> " << *tmp << endl;

      return tmp;
}

NetExpr* NetEBPow::eval_tree_real_()
{
      verireal lval;
      verireal rval;

      bool flag = get_real_arguments_(lval, rval);
      if (! flag) return 0;

      NetECReal*res = new NetECReal( pow(lval,rval) );
      ivl_assert(*this, res);
      res->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated (real): " << *this
	         << " --> " << *res << endl;

      return res;
}

NetExpr* NetEBPow::eval_tree()
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

      unsigned wid = expr_width();
      ivl_assert(*this, wid > 0);
      ivl_assert(*this, lval.len() == wid);

      verinum val(pow(lval, rval), wid);
      NetEConst*res = new NetEConst(val);
      ivl_assert(*this, res);
      res->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated: " << *this
	         << " --> " << *res << endl;

      return res;
}

/*
 * Evaluate the shift operator if possible. For this to work, both
 * operands must be constant.
 */
NetEConst* NetEBShift::eval_tree()
{
      eval_expr(left_);
      eval_expr(right_);
      return eval_arguments_(left_,right_);
}

NetEConst* NetEBShift::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
      const NetEConst*le = dynamic_cast<const NetEConst*>(l);
      const NetEConst*re = dynamic_cast<const NetEConst*>(r);
      if (le == 0 || re == 0) return 0;

      NetEConst*res;

      verinum lv = le->value();
      verinum rv = re->value();

      unsigned wid = expr_width();
      ivl_assert(*this, wid > 0);
      ivl_assert(*this, lv.len() == wid);

      verinum val;
      if (rv.is_defined()) {
	    unsigned shift = rv.as_ulong();

	    switch (op_) {
		case 'l':
		  val = verinum(lv << shift, wid);
		  break;
		case 'r':
                  lv.has_sign(false);
		case 'R':
		  val = verinum(lv >> shift, wid);
		  break;
		default:
		  return 0;
	    }
      } else {
	    val = verinum(verinum::Vx, wid);
      }
      val.has_sign(has_sign());
      res = new NetEConst(val);
      res->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated: " << *this
	         << " --> " << *res << endl;

      return res;
}

NetEConst* NetEConcat::eval_tree()
{
      unsigned repeat_val = repeat();
      unsigned local_errors = 0;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: Evaluating expression:"
	         << *this << endl;
      }

      unsigned gap = 0;
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {

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
	    NetExpr*expr = parms_[idx]->eval_tree();
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
      for (unsigned idx = parms_.size() ;  idx > 0 ;  idx -= 1) {
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
      return res;
}

NetEConst* NetESelect::eval_tree()
{
      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: Evaluating expression:"
	         << *this << endl;
      }

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

	      /* If the base is NULL (different from 0) then this
		 select is here for zero or sign extension. So
                 calculate a proper pad bit. */
            if (has_sign())
	          pad_bit = eval.get(expr->expr_width()-1);
            else
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
NetExpr* NetETernary::eval_tree()
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

      return blended_arguments_(true_val_, false_val_);
}

NetExpr*NetETernary::blended_arguments_(const NetExpr*te, const NetExpr*fe) const
{

      const NetEConst*t = dynamic_cast<const NetEConst*>(te);
      const NetEConst*f = dynamic_cast<const NetEConst*>(fe);
      if (t == 0 || f == 0) {
	    verireal tv, fv;
	    if (!get_real_arg_(te, tv)) return 0;
	    if (!get_real_arg_(te, fv)) return 0;

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
	    ivl_assert(*this, res);
	    break;

	  case '-':
	    res = new NetECReal(-(val->value()));
	    ivl_assert(*this, res);
	    break;

	  default:
	    return 0;
      }

      res->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated (real): " << *this
	         << " --> " << *res << endl;

      return res;
}

NetExpr* NetEUnary::eval_tree()
{
      eval_expr(expr_);
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_();

      NetEConst*rval = dynamic_cast<NetEConst*>(expr_);
      if (rval == 0) return 0;

      verinum val = rval->value();

      switch (op_) {

	  case '+':
	      /* Unary + is a no-op. */
	    break;

	  case '-':
	    if (val.is_defined()) {
		  verinum tmp (verinum::V0, val.len());
		  tmp.has_sign(val.has_sign());
		  val = verinum(tmp - val, val.len());
	    } else {
		  for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
			val.set(idx, verinum::Vx);
	    }
	    break;

	  case '~':
	      /* Bitwise not is even simpler than logical
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

	    break;

	  case '!':
	    assert(0);
	  default:
	    return 0;
      }

      NetEConst *res = new NetEConst(val);
      ivl_assert(*this, res);
      res->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated: " << *this
	         << " --> " << *res << endl;

      return res;
}


NetExpr* NetEUBits::eval_tree()
{
      return NetEUnary::eval_tree();
}

NetEConst* NetEUReduce::eval_tree_real_()
{
      ivl_assert(*this, op_ == '!');

      NetECReal*val= dynamic_cast<NetECReal*> (expr_);
      if (val == 0) return 0;

      verinum::V res = val->value().as_double() == 0.0 ? verinum::V1 :
                                                         verinum::V0;

      NetEConst*tmp = new NetEConst(verinum(res, 1));
      ivl_assert(*this, tmp);
      tmp->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated (real): " << *this
	         << " --> " << *tmp << endl;

      return tmp;
}

NetEConst* NetEUReduce::eval_tree()
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
		bool v1 = false, vx = false;
		for (unsigned idx = 0 ;  idx < val.len() && !v1 ;  idx += 1) {
		      switch (val.get(idx)) {
			  case verinum::V0:
			    break;
			  case verinum::V1:
			    v1 = true;
			    break;
			  default:
			    vx = true;
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

      NetEConst*tmp = new NetEConst(verinum(res, 1));
      ivl_assert(*this, tmp);
      tmp->set_line(*this);

      if (debug_eval_tree)
	    cerr << get_fileline() << ": debug: Evaluated: " << *this
	         << " --> " << *tmp << endl;

      return tmp;
}

static NetEConst* evaluate_clog2(NetExpr*&arg_)
{
      eval_expr(arg_);

      NetEConst*tmpi = dynamic_cast<NetEConst *>(arg_);
      NetECReal*tmpr = dynamic_cast<NetECReal *>(arg_);

      if (tmpi == 0 && tmpr == 0) return 0;

      verinum arg;
      if (tmpi) {
	    arg = tmpi->value();
      } else {
	    arg = verinum(tmpr->value().as_double(), true);
      }

      NetEConst*rtn;

	/* If we have an x in the verinum we return 'bx. */
      if (!arg.is_defined()) {
	    verinum tmp (verinum::Vx, integer_width);
	    tmp.has_sign(true);

	    rtn = new NetEConst(tmp);
	    ivl_assert(*arg_, rtn);
      } else {
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

	    rtn = new NetEConst(tmp);
	    ivl_assert(*arg_, rtn);
      }

      return rtn;
}

static NetECReal* evaluate_math_one_arg(NetExpr*&arg_, const char*name)
{
      eval_expr(arg_);

      NetEConst*tmpi = dynamic_cast<NetEConst *>(arg_);
      NetECReal*tmpr = dynamic_cast<NetECReal *>(arg_);

      NetECReal*res = 0;

      if (tmpi || tmpr) {
	    double arg;
	    if (tmpi) {
		  arg = tmpi->value().as_double();
	    } else {
		  arg = tmpr->value().as_double();
	    }

	    if (strcmp(name, "$ln") == 0) {
		  res = new NetECReal(verireal(log(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$log10") == 0) {
		  res = new NetECReal(verireal(log10(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$exp") == 0) {
		  res = new NetECReal(verireal(exp(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$sqrt") == 0) {
		  res = new NetECReal(verireal(sqrt(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$floor") == 0) {
		  res = new NetECReal(verireal(floor(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$ceil") == 0) {
		  res = new NetECReal(verireal(ceil(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$sin") == 0) {
		  res = new NetECReal(verireal(sin(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$cos") == 0) {
		  res = new NetECReal(verireal(cos(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$tan") == 0) {
		  res = new NetECReal(verireal(tan(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$asin") == 0) {
		  res = new NetECReal(verireal(asin(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$acos") == 0) {
		  res = new NetECReal(verireal(acos(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$atan") == 0) {
		  res = new NetECReal(verireal(atan(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$sinh") == 0) {
		  res = new NetECReal(verireal(sinh(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$cosh") == 0) {
		  res = new NetECReal(verireal(cosh(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$tanh") == 0) {
		  res = new NetECReal(verireal(tanh(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$asinh") == 0) {
		  res = new NetECReal(verireal(asinh(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$acosh") == 0) {
		  res = new NetECReal(verireal(acosh(arg)));
		  ivl_assert(*arg_, res);
	    } else if (strcmp(name, "$atanh") == 0) {
		  res = new NetECReal(verireal(atanh(arg)));
		  ivl_assert(*arg_, res);
	    } else {
		  cerr << arg_->get_fileline() << ": warning: Unhandled"
		          "constant system function " << name << "." << endl;
	    }
      }

      return res;
}

static NetECReal* evaluate_math_two_args(NetExpr*&arg0_, NetExpr*&arg1_,
                                         const char*name)
{
      eval_expr(arg0_);
      eval_expr(arg1_);

      NetEConst*tmpi0 = dynamic_cast<NetEConst *>(arg0_);
      NetECReal*tmpr0 = dynamic_cast<NetECReal *>(arg0_);
      NetEConst*tmpi1 = dynamic_cast<NetEConst *>(arg1_);
      NetECReal*tmpr1 = dynamic_cast<NetECReal *>(arg1_);

      NetECReal*res = 0;

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
		  res = new NetECReal(verireal(pow(arg0, arg1)));
		  ivl_assert(*arg0_, res);
	    } else if (strcmp(name, "$atan2") == 0) {
		  res = new NetECReal(verireal(atan2(arg0, arg1)));
		  ivl_assert(*arg0_, res);
	    } else if (strcmp(name, "$hypot") == 0) {
		  res = new NetECReal(verireal(hypot(arg0, arg1)));
		  ivl_assert(*arg0_, res);
	    } else {
		  cerr << arg0_->get_fileline() << ": warning: Unhandled"
		          "constant system function " << name << "." << endl;
	    }
      }

      return res;
}

static NetExpr* evaluate_abs(NetExpr*&arg_)
{
      eval_expr(arg_);

      NetExpr*res = 0;

      NetEConst*tmpi = dynamic_cast<NetEConst *>(arg_);
      if (tmpi) {
	    verinum arg = tmpi->value();
	    if (arg.is_negative()) {
		  arg = v_not(arg) + verinum(1);
	    }
	    res = new NetEConst(arg);
	    ivl_assert(*arg_, res);
      }

      NetECReal*tmpr = dynamic_cast<NetECReal *>(arg_);
      if (tmpr) {
	    double arg = tmpr->value().as_double();
	    res = new NetECReal(verireal(fabs(arg)));
	    ivl_assert(*arg_, res);
      }

      return res;
}

static NetExpr* evaluate_min_max(NetExpr*&arg0_, NetExpr*&arg1_,
                                 const char*name)
{
      eval_expr(arg0_);
      eval_expr(arg1_);

      NetEConst*tmpi0 = dynamic_cast<NetEConst *>(arg0_);
      NetECReal*tmpr0 = dynamic_cast<NetECReal *>(arg0_);
      NetEConst*tmpi1 = dynamic_cast<NetEConst *>(arg1_);
      NetECReal*tmpr1 = dynamic_cast<NetECReal *>(arg1_);

      NetExpr*res = 0;

      if (tmpi0 && tmpi1) {
	    verinum arg0 = tmpi0->value();
	    verinum arg1 = tmpi1->value();
	    if (strcmp(name, "$min") == 0) {
		  res = new NetEConst( arg0 < arg1 ? arg0 : arg1);
		  ivl_assert(*arg0_, res);
	    } else if (strcmp(name, "$max") == 0) {
		  res = new NetEConst( arg0 < arg1 ? arg1 : arg0);
		  ivl_assert(*arg0_, res);
	    }
      } else if ((tmpi0 || tmpr0) && (tmpi1 || tmpr1)) {
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
		  res = new NetECReal(verireal(arg0 < arg1 ? arg0 : arg1));
		  ivl_assert(*arg0_, res);
	    } else if (strcmp(name, "$max") == 0) {
		  res = new NetECReal(verireal(arg0 < arg1 ? arg1 : arg0));
		  ivl_assert(*arg0_, res);
	    } else {
		  cerr << arg0_->get_fileline() << ": warning: Unhandled"
		          "constant system function " << name << "." << endl;
	    }
      }

      return res;
}

NetExpr* NetESFunc::eval_tree()
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

	    if (debug_eval_tree)
		  cerr << get_fileline() << ": debug: Evaluated: " << *this
		       << " --> " << *rtn << endl;
      }

      return rtn;
}

NetExpr* NetEUFunc::eval_tree()
{
        // If we know the function cannot be evaluated as a constant,
        // give up now.
      if (!func()->is_const_func())
            return 0;

        // Variables inside static functions can be accessed from outside
        // the function, so we can't be sure they are constant unless the
        // function was called in a constant context.
      if (!func()->is_auto() && !need_const_)
            return 0;

        // Run through the input parameters to check they are constants.
      for (unsigned idx = 0; idx < parm_count(); idx += 1) {
            if (dynamic_cast<const NetEConst*> (parm(idx)))
                  continue;
            if (dynamic_cast<const NetECReal*> (parm(idx)))
                  continue;
            return 0;
      }

      if (need_const_) {
	    NetFuncDef*def = func_->func_def();
	    ivl_assert(*this, def);

	    vector<NetExpr*>args(parms_.size());
	    for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1)
		  args[idx] = parms_[idx]->dup_expr();

	    NetExpr*res = def->evaluate_function(*this, args);
	    return res;
      }

      return 0;
}
