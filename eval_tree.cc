/*
 * Copyright (c) 1999-2022 Stephen Williams (steve@icarus.com)
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

using namespace std;

NetExpr* NetExpr::eval_tree()
{
      return 0;
}

static void eval_debug(const NetExpr*expr, NetExpr*res, bool is_real)
{
      if (res != 0) {
	    res->set_line(*expr);
	    if (debug_eval_tree) {
		  cerr << expr->get_fileline() << ": debug: Evaluated";
		  if (is_real) cerr << " (real)";
		  cerr << ": " << *expr << " --> " << *res << endl;
	    }
      }
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
	    ivl_assert(*expr, 0);
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

NetExpr* NetEBinary::eval_tree()
{
      eval_expr(left_);
      eval_expr(right_);

      return eval_arguments_(left_, right_);
}

NetExpr* NetEBinary::eval_arguments_(const NetExpr*, const NetExpr*) const
{
	// this method should be overridden in all sub-classes
      ivl_assert(*this, 0);
      return 0;
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
      eval_debug(this, res, true);
      return res;
}

NetExpr* NetEBAdd::eval_tree()
{
      eval_expr(left_);
      eval_expr(right_);

	// First try to elaborate the expression completely.
      NetExpr*res = eval_arguments_(left_,right_);
      if (res != 0) return res;

	// If the expression type is real, then do not attempt the
	// following alternative processing.
      if (expr_type() == IVL_VT_REAL)
	    return 0;

	// The expression has not evaluated to a constant. Let's still
	// try to optimize by trying to combine a right constant value
	// with the right constant value of a sub-expression add. For
	// example, the expression (a + 2) - 1 can be rewritten as a + 1.

      NetEBAdd*se = dynamic_cast<NetEBAdd*>(left_);
      NetEConst*lc = se? dynamic_cast<NetEConst*>(se->right_) : NULL;
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
		  val = cast_to_width(rval + lval, wid);
	    } else {
		    /* (a - lval) + rval  -->  a + (rval-lval) */
		    /* (a + lval) - rval  -->  a - (rval-lval) */
		  val = cast_to_width(rval - lval, wid);
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
		  val = cast_to_width(lval + rval, wid);
		  break;
		case '-':
		  val = cast_to_width(lval - rval, wid);
		  break;
		default:
		  return 0;
	    }

	    NetEConst *res = new NetEConst(val);
	    ivl_assert(*this, res);
	    eval_debug(this, res, false);
	    return res;
      }


	/* Nothing more to be done, the value is not constant. */
      return 0;
}

NetEConst* NetEBBits::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
      const NetEConst*lc = dynamic_cast<const NetEConst*>(l);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(r);
      if (lc == 0 || rc == 0) return 0;

	/* Notice the special case where one of the operands is 0 and
	   this is a bitwise &. If this happens, then the result is
	   known to be 0. */
      if ((op() == '&') && (lc->value() == verinum(0))) {
	    verinum res (verinum::V0, expr_width());
	    res.has_sign(has_sign());
	    NetEConst*tmp = new NetEConst(res);
	    ivl_assert(*this, tmp);
	    eval_debug(this, tmp, false);
	    return tmp;
      }

      if ((op() == '&') && (rc->value() == verinum(0))) {
	    verinum res (verinum::V0, expr_width());
	    res.has_sign(has_sign());
	    NetEConst*tmp = new NetEConst(res);
	    ivl_assert(*this, tmp);
	    eval_debug(this, tmp, false);
	    return tmp;
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

	  case 'X': {
		for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		      res.set(idx, ~(lval.get(idx) ^ rval.get(idx)));

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

      res.has_sign(has_sign());
      NetEConst*tmp = new NetEConst(res);
      ivl_assert(*this, tmp);
      eval_debug(this, tmp, false);
      return tmp;
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
	// The following optimization is not valid if le can contain 'x'
	// or 'z' values.
      if (le->expr_type() == IVL_VT_LOGIC) return 0;

      ivl_assert(*le, le->expr_width() > 0);
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

      const NetEConst*r = dynamic_cast<const NetEConst*>(re);
      if (r == 0) return 0;

      verinum rv = r->value();
      if (! rv.is_defined()) {
	    NetEConst*res = new NetEConst(verinum(verinum::Vx, 1));
	    ivl_assert(*this, res);
	    return res;
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

      const NetEConst*l = dynamic_cast<const NetEConst*>(le);
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

      verinum result(((lval == rval) != ne_flag) ?
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

	// The two expressions should already be padded to the same size.
      ivl_assert(*this, lv.len() == rv.len());

      for (unsigned idx = 0 ;  idx < lv.len() ;  idx += 1) {

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

	// The two expressions should already be padded to the same size.
      ivl_assert(*this, lv.len() == rv.len());

      for (unsigned idx = 0 ;  idx < lv.len() ;  idx += 1)
	    if (lv.get(idx) != rv.get(idx)) {
		  res = verinum::V0;
		  break;
	    }

      if (ne_flag) {
	    if (res == verinum::V0) res = verinum::V1;
	    else res = verinum::V0;
      }

      NetEConst*result = new NetEConst(verinum(res, 1));
      ivl_assert(*this, result);
      return result;
}

NetEConst* NetEBComp::eval_weqeq_(bool ne_flag, const NetExpr*le, const NetExpr*re) const
{
      const NetEConst*lc = dynamic_cast<const NetEConst*>(le);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(re);
      if (lc == 0 || rc == 0) return 0;

      const verinum&lv = lc->value();
      const verinum&rv = rc->value();

      const verinum::V eq_res = ne_flag ? verinum::V0 : verinum::V1;
      const verinum::V ne_res = ne_flag ? verinum::V1 : verinum::V0;

      verinum::V res = eq_res;

	// The two expressions should already be padded to the same size.
      ivl_assert(*this, lv.len() == rv.len());

      for (unsigned idx = 0 ;  idx < lv.len() ;  idx += 1) {
	      // An X or Z in the R-value matches any L-value.
	    switch (rv.get(idx)) {
		case verinum::Vx:
		case verinum::Vz:
		  continue;
		default:
		  break;
	    }

	      // An X or Z in the L-value that is not matches by an R-value X/Z returns undefined.
	    switch (lv.get(idx)) {
		case verinum::Vx:
		case verinum::Vz:
		  res = verinum::Vx;
		  continue;
		default:
		  break;
	    }

	      // A hard (0/1) mismatch gives a not-equal result.
	    if (rv.get(idx) != lv.get(idx)) {
		  res = ne_res;
		  break;
	    }
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

	  case 'w': // Wild equality (==?)
	    res = eval_weqeq_(false, l, r);
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

	  case 'W': // Wild not-equal (!=?)
	    res = eval_weqeq_(true, l, r);
	    break;

	  case '<': // Less than
	    res = eval_less_(l, r);
	    break;

	  case '>': // Greater than
	    res = eval_gt_(l, r);
	    break;

      }
      eval_debug(this, res, l->expr_type() == IVL_VT_REAL ||
                            r->expr_type() == IVL_VT_REAL);
      return res;
}

NetExpr* NetEBDiv::eval_tree_real_(const NetExpr*l, const NetExpr*r) const
{
      double lval;
      double rval;

      bool flag = get_real_arguments(l, r, lval, rval);
      if (! flag) return 0;

      double res_val = 0.0;
      switch (op_) {
	  case '/':
	    res_val = lval / rval;
	    break;

	  case '%':
	      // Since this could/may be called early we don't want to
	      // leak functionality.
	    if (!gn_icarus_misc_flag) return 0;
	    res_val = fmod(lval, rval);
	    break;
      }
      NetECReal*res = new NetECReal( verireal(res_val) );
      ivl_assert(*this, res);
      eval_debug(this, res, true);
      return res;
}

NetExpr* NetEBDiv::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_(l,r);
      ivl_assert(*this, expr_type() == IVL_VT_LOGIC);

      const NetEConst*lc = dynamic_cast<const NetEConst*>(l);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(r);
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
	    val = cast_to_width(lval / rval, wid);
	    break;
	  case '%':
	    val = cast_to_width(lval % rval, wid);
	    break;
	  default:
	    return 0;
      }

      NetExpr*tmp = new NetEConst(val);
      ivl_assert(*this, tmp);
      eval_debug(this, tmp, false);
      return tmp;
}

NetEConst* NetEBLogic::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
	// NetEBLogic arguments should have already been reduced so real is not possible.
      ivl_assert(*this, (l->expr_type() != IVL_VT_REAL) && (r->expr_type() != IVL_VT_REAL));
      ivl_assert(*this, expr_type() == IVL_VT_LOGIC || expr_type() == IVL_VT_BOOL);

      const NetEConst*lc = dynamic_cast<const NetEConst*>(l);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(r);

      // If the left side is constant and the right side is short circuited
      // replace the expression with a constant
      if (rc == 0 && lc != 0) {
	    verinum v = lc->value();
	    verinum::V res = verinum::Vx;
	    switch (op_) {
		case 'a': // Logical AND (&&)
		  if (v.is_zero())
			res = verinum::V0;
		  break;
		case 'o': // Logical OR (||)
		  if (! v.is_zero() && v.is_defined())
			res = verinum::V1;
		  break;
		case 'q': // Logical implication (->)
		  if (v.is_zero())
			res = verinum::V1;
		  break;
		default:
		  break;
	    }
	    if (res != verinum::Vx) {
		  NetEConst*tmp = new NetEConst(verinum(res, 1));
		  ivl_assert(*this, tmp);
		  eval_debug(this, tmp, false);
		  return tmp;
	    }
      }

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

	  case 'q': // Logical implication (->)
	    if ((lv == verinum::V0) || (rv == verinum::V1))
		  res = verinum::V1;
	    else if ((lv == verinum::V1) && (rv == verinum::V0))
		  res = verinum::V0;
	    else
		  res = verinum::Vx;
	    break;

	  case 'Q': // Logical equivalence (<->)
	    if (((lv == verinum::V0) && (rv == verinum::V0)) ||
	        ((lv == verinum::V1) && (rv == verinum::V1)))
		  res = verinum::V1;
	    else if (((lv == verinum::V0) && (rv == verinum::V1)) ||
	             ((lv == verinum::V1) && (rv == verinum::V0)))
		  res = verinum::V0;
	    else
		  res = verinum::Vx;
	    break;

	  default:
	    return 0;
      }

      NetEConst*tmp = new NetEConst(verinum(res, 1));
      ivl_assert(*this, tmp);
      eval_debug(this, tmp, false);
      return tmp;
}

NetExpr* NetEBMinMax::eval_tree_real_(const NetExpr*l, const NetExpr*r) const
{
      double lval;
      double rval;

      bool flag = get_real_arguments(l, r, lval, rval);
      if (! flag) return 0;

      double res_val;
      switch (op()) {
	  case 'm':
	    res_val = lval < rval ? lval : rval;
	    break;
	  case 'M':
	    res_val = lval > rval ? lval : rval;
	    break;
	  default:
	    ivl_assert(*this, 0);
      }

      NetECReal*res = new NetECReal( verireal(res_val) );
      ivl_assert(*this, res);
      eval_debug(this, res, true);
      return res;
}

NetExpr* NetEBMinMax::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_(l,r);
      ivl_assert(*this, expr_type() == IVL_VT_LOGIC || expr_type() == IVL_VT_BOOL);

      const NetEConst*lc = dynamic_cast<const NetEConst*>(l);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(r);
      if (lc == 0 || rc == 0) return 0;

      verinum lval = lc->value();
      verinum rval = rc->value();

      unsigned wid = expr_width();
      ivl_assert(*this, wid > 0);
      ivl_assert(*this, lval.len() == wid);
      ivl_assert(*this, rval.len() == wid);

      verinum res_val;
      if (lval.is_defined() && rval.is_defined()) {
            switch (op()) {
                case 'm':
                  res_val = lval < rval ? lval : rval;
                  break;
                case 'M':
                  res_val = lval > rval ? lval : rval;
                  break;
                default:
                  ivl_assert(*this, 0);
            }
      } else {
            res_val = verinum(verinum::Vx, wid);
      }
      res_val.has_sign(has_sign());
      NetEConst*res = new NetEConst(res_val);
      ivl_assert(*this, res);
      eval_debug(this, res, false);
      return res;
}

NetExpr* NetEBMult::eval_tree_real_(const NetExpr*l, const NetExpr*r) const
{
      double lval;
      double rval;

      bool flag = get_real_arguments(l, r, lval, rval);
      if (! flag) return 0;

      NetECReal*res = new NetECReal( verireal(lval * rval) );
      ivl_assert(*this, res);
      eval_debug(this, res, true);
      return res;
}

NetExpr* NetEBMult::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_(l,r);
      ivl_assert(*this, expr_type() == IVL_VT_LOGIC || expr_type() == IVL_VT_BOOL);

      const NetEConst*lc = dynamic_cast<const NetEConst*>(l);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(r);
      if (lc == 0 || rc == 0) return 0;

      verinum lval = lc->value();
      verinum rval = rc->value();

      unsigned wid = expr_width();
      ivl_assert(*this, wid > 0);
      ivl_assert(*this, lval.len() == wid);
      ivl_assert(*this, rval.len() == wid);

      verinum val = cast_to_width(lval * rval, wid);
      NetEConst*tmp = new NetEConst(val);
      ivl_assert(*this, tmp);
      eval_debug(this, tmp, false);
      return tmp;
}

NetExpr* NetEBPow::eval_tree_real_(const NetExpr*l, const NetExpr*r) const
{
      double lval;
      double rval;

      bool flag = get_real_arguments(l, r, lval, rval);
      if (! flag) return 0;

      NetECReal*res = new NetECReal( verireal( pow(lval,rval) ) );
      ivl_assert(*this, res);
      eval_debug(this, res, true);
      return res;
}

NetExpr* NetEBPow::eval_arguments_(const NetExpr*l, const NetExpr*r) const
{
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_(l,r);
      ivl_assert(*this, expr_type() == IVL_VT_LOGIC || expr_type() == IVL_VT_BOOL);

      const NetEConst*lc = dynamic_cast<const NetEConst*>(l);
      const NetEConst*rc = dynamic_cast<const NetEConst*>(r);
      if (lc == 0 || rc == 0) return 0;

      verinum lval = lc->value();
      verinum rval = rc->value();

      unsigned wid = expr_width();
      ivl_assert(*this, wid > 0);
      ivl_assert(*this, lval.len() == wid);

      verinum val = cast_to_width(pow(lval, rval), wid);
      NetEConst*res = new NetEConst(val);
      ivl_assert(*this, res);
      eval_debug(this, res, false);
      return res;
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
	    unsigned shift = rv.as_unsigned();

	    switch (op_) {
		case 'l':
		  val = cast_to_width(lv << shift, wid);
		  break;
		case 'r':
                  lv.has_sign(false);
		  // fallthrough
		case 'R':
		  val = cast_to_width(lv >> shift, wid);
		  break;
		default:
		  return 0;
	    }
      } else {
	    val = verinum(verinum::Vx, wid);
      }
      val.has_sign(has_sign());
      res = new NetEConst(val);
      ivl_assert(*this, res);
      eval_debug(this, res, false);
      return res;
}

NetEConst* NetEConcat::eval_tree()
{
      unsigned local_errors = 0;

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
	    ivl_assert(*this, parms_[idx]);
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

      return eval_arguments_(parms_, gap);
}

NetEConst* NetEConcat::eval_arguments_(const vector<NetExpr*>&vals,
                                       unsigned gap) const
{
      unsigned repeat_val = repeat();

	// At this point, the "gap" is the width of a single repeat of
	// the concatenation. The total width of the result is the gap
	// times the repeat count.
      verinum val (verinum::Vx, repeat_val * gap);

	// build up the result from least significant to most.

      unsigned cur = 0;
      bool is_string_flag = true;
      for (unsigned idx = vals.size() ;  idx > 0 ;  idx -= 1) {
	    const NetEConst*expr = dynamic_cast<NetEConst*>(vals[idx-1]);
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
      ivl_assert(*this, res);
      eval_debug(this, res, false);
      return res;
}

NetEConst* NetESelect::eval_tree()
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
      eval_debug(this, res, false);
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
      ivl_assert(*expr, 0);
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

	      // Elaborate the alternate expression to check for errors.
	    eval_expr(true_val_);

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

	      // Elaborate the alternate expression to check for errors.
	    eval_expr(false_val_);

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
	    if (!get_real_arg_(fe, fv)) return 0;

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
      val.has_sign(has_sign());

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

NetExpr* NetEUnary::eval_tree()
{
      eval_expr(expr_);
      return eval_arguments_(expr_);
}

NetExpr* NetEUnary::eval_tree_real_(const NetExpr*ex) const
{
      const NetECReal*val= dynamic_cast<const NetECReal*> (ex);
      if (val == 0) return 0;

      double res_val = val->value().as_double();
      switch (op_) {
	  case '+':
	    break;

	  case '-':
	    res_val = -res_val;
	    break;

	  case 'm':
	    if (res_val < 0.0) res_val = -res_val;
	    break;

	  default:
	    return 0;
      }
      NetECReal *res = new NetECReal( verireal(res_val) );
      ivl_assert(*this, res);
      eval_debug(this, res, true);
      return res;
}

NetExpr* NetEUnary::eval_arguments_(const NetExpr*ex) const
{
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_(ex);

      const NetEConst*rval = dynamic_cast<const NetEConst*>(ex);
      if (rval == 0) return 0;

      verinum val = rval->value();

      switch (op_) {

	  case '+':
	      /* Unary + is a no-op. */
	    break;

	  case '-':
	    val = -val;
	    break;

	  case 'm':
	    if (!val.is_defined()) {
		  for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
			val.set(idx, verinum::Vx);
	    } else if (val.is_negative()) {
		  val = -val;
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
	    ivl_assert(*this, 0);
	  default:
	    return 0;
      }

      NetEConst *res = new NetEConst(val);
      ivl_assert(*this, res);
      eval_debug(this, res, false);
      return res;
}


NetEConst* NetEUReduce::eval_tree_real_(const NetExpr*ex) const
{
      ivl_assert(*this, op_ == '!');

      const NetECReal*val= dynamic_cast<const NetECReal*> (ex);
      if (val == 0) return 0;

      verinum::V res = val->value().as_double() == 0.0 ? verinum::V1 :
                                                         verinum::V0;

      NetEConst*tmp = new NetEConst(verinum(res, 1));
      ivl_assert(*this, tmp);
      eval_debug(this, tmp, true);
      return tmp;
}

NetEConst* NetEUReduce::eval_arguments_(const NetExpr*ex) const
{
      if (expr_type() == IVL_VT_REAL) return eval_tree_real_(ex);

      const NetEConst*rval = dynamic_cast<const NetEConst*>(ex);
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
		// fallthrough
	  case '&': {
		res = verinum::V1;
		for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
		      res = res & val.get(idx);
		break;
	  }

	  case 'N':
		invert = true;
		// fallthrough
	  case '|': {
		res = verinum::V0;
		for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
		      res = res | val.get(idx);
		break;
	  }

	  case 'X':
		invert = true;
		// fallthrough
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
      eval_debug(this, tmp, false);
      return tmp;
}

NetExpr* NetECast::eval_arguments_(const NetExpr*ex) const
{
      NetExpr*res = 0;
      switch (op_) {
	  case 'r':
	    if (const NetEConst*val = dynamic_cast<const NetEConst*>(ex)) {
		  verireal res_val(val->value().as_double());
		  res = new NetECReal(res_val);
	    }
	    break;
	  case '2':
	    if (const NetEConst*val = dynamic_cast<const NetEConst*>(ex)) {
		  verinum res_val(val->value());
		  res_val.cast_to_int2();
		  if (expr_width() > 0)
			res_val = cast_to_width(res_val, expr_width());
		  res = new NetEConst(res_val);
	    }
	    // fallthrough
	  case 'v':
	    if (const NetECReal*val = dynamic_cast<const NetECReal*>(ex)) {
		  verinum res_val(val->value().as_double(), false);
		  if (expr_width() > 0)
			res_val = cast_to_width(res_val, expr_width());
		  res = new NetEConst(res_val);
	    }
	    break;
	  default:
	    ivl_assert(*this, 0);
	    return 0;
      }
      if (res == 0) return 0;

      ivl_assert(*this, res);
      eval_debug(this, res, op_ == 'r');
      return res;
}

NetEConst* NetESFunc::evaluate_clog2_(const NetExpr*arg_) const
{
      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(arg_);
      const NetECReal*tmpr = dynamic_cast<const NetECReal*>(arg_);

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
	    ivl_assert(*this, rtn);
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
	    ivl_assert(*this, rtn);
      }

      eval_debug(this, rtn, false);
      return rtn;
}

NetEConst* NetESFunc::evaluate_rtoi_(const NetExpr*arg_) const
{
      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(arg_);
      const NetECReal*tmpr = dynamic_cast<const NetECReal*>(arg_);

      if (tmpi == 0 && tmpr == 0) return 0;

	/* If the argument is already a bit based value just extend/trim it
	 * to the integer width and translate all undefined bits to zero. */
      if (tmpi) {
	    verinum arg = verinum(tmpi->value(), integer_width);
	    arg.cast_to_int2();
	    return new NetEConst(arg);
      }

	/* Get the value of the real argument as a bit based value and then
	 * extend/trim it to the integer width. */
      double arg = tmpr->value().as_double();
      if (arg >= 0.0) arg = floor(arg);
      else arg = ceil(arg);
      return new NetEConst(verinum(verinum(arg, false), integer_width));
}

NetECReal* NetESFunc::evaluate_itor_(const NetExpr*arg_) const
{
      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(arg_);
      const NetECReal*tmpr = dynamic_cast<const NetECReal*>(arg_);

      if (tmpi == 0 && tmpr == 0) return 0;

	/* If the argument is already a real value round it, but NaN and
	 * +/- infinity need to be translated to 0.0. */
      if (tmpr) {
	    double arg = tmpr->value().as_double();
	      /* Convert a NaN or +/- infinity to 0.0 since these convert
	       * to 'bz which is then translated to 0.0. */
	    if (arg != arg || (arg && (arg == 0.5*arg))) {
		  return new NetECReal(verireal(0.0));
	    }

	    if (arg >= 0.0) arg = floor(arg + 0.5);
	    else arg = ceil(arg - 0.5);

	    return new NetECReal(verireal(arg));
      }

	/* Convert the bit based value to a real value. */
      double arg = tmpi->value().as_double();
      return new NetECReal(verireal(arg));
}

NetECReal* NetESFunc::evaluate_math_one_arg_(ID id, const NetExpr*arg_) const
{
      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(arg_);
      const NetECReal*tmpr = dynamic_cast<const NetECReal*>(arg_);

      NetECReal*res = 0;

      if (tmpi || tmpr) {
	    double arg;
	    if (tmpi) {
		  arg = tmpi->value().as_double();
	    } else {
		  arg = tmpr->value().as_double();
	    }

	    switch (id) {
		case LN:
		  res = new NetECReal(verireal(log(arg)));
		  break;
		case LOG10:
		  res = new NetECReal(verireal(log10(arg)));
		  break;
		case EXP:
		  res = new NetECReal(verireal(exp(arg)));
		  break;
		case SQRT:
		  res = new NetECReal(verireal(sqrt(arg)));
		  break;
		case FLOOR:
		  res = new NetECReal(verireal(floor(arg)));
		  break;
		case CEIL:
		  res = new NetECReal(verireal(ceil(arg)));
		  break;
		case SIN:
		  res = new NetECReal(verireal(sin(arg)));
		  break;
		case COS:
		  res = new NetECReal(verireal(cos(arg)));
		  break;
		case TAN:
		  res = new NetECReal(verireal(tan(arg)));
		  break;
		case ASIN:
		  res = new NetECReal(verireal(asin(arg)));
		  break;
		case ACOS:
		  res = new NetECReal(verireal(acos(arg)));
		  break;
		case ATAN:
		  res = new NetECReal(verireal(atan(arg)));
		  break;
		case SINH:
		  res = new NetECReal(verireal(sinh(arg)));
		  break;
		case COSH:
		  res = new NetECReal(verireal(cosh(arg)));
		  break;
		case TANH:
		  res = new NetECReal(verireal(tanh(arg)));
		  break;
		case ASINH:
		  res = new NetECReal(verireal(asinh(arg)));
		  break;
		case ACOSH:
		  res = new NetECReal(verireal(acosh(arg)));
		  break;
		case ATANH:
		  res = new NetECReal(verireal(atanh(arg)));
		  break;
		default:
		  ivl_assert(*this, 0);
		  break;
	    }
	    ivl_assert(*this, res);
      }

      eval_debug(this, res, true);
      return res;
}

NetECReal* NetESFunc::evaluate_math_two_arg_(ID id, const NetExpr*arg0_,
						    const NetExpr*arg1_) const
{
      const NetEConst*tmpi0 = dynamic_cast<const NetEConst*>(arg0_);
      const NetECReal*tmpr0 = dynamic_cast<const NetECReal*>(arg0_);
      const NetEConst*tmpi1 = dynamic_cast<const NetEConst*>(arg1_);
      const NetECReal*tmpr1 = dynamic_cast<const NetECReal*>(arg1_);

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

	    switch (id) {
		case POW:
		  res = new NetECReal(verireal(pow(arg0, arg1)));
		  break;
		case ATAN2:
		  res = new NetECReal(verireal(atan2(arg0, arg1)));
		  break;
		case HYPOT:
		  res = new NetECReal(verireal(hypot(arg0, arg1)));
		  break;
		default:
		  ivl_assert(*this, 0);
		  break;
	    }
	    ivl_assert(*this, res);
      }

      eval_debug(this, res, true);
      return res;
}

NetExpr* NetESFunc::evaluate_abs_(const NetExpr*arg_) const
{
      NetExpr*res = 0;

      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(arg_);
      const NetECReal*tmpr = dynamic_cast<const NetECReal*>(arg_);
      if (tmpi || tmpr) {
	    double arg;
	    if (tmpi) {
		  arg = tmpi->value().as_double();
	    } else {
		  arg = tmpr->value().as_double();
	    }
	    res = new NetECReal(verireal(fabs(arg)));
	    ivl_assert(*this, res);
      }

      eval_debug(this, res, true);
      return res;
}

NetExpr* NetESFunc::evaluate_min_max_(ID id, const NetExpr*arg0_,
					     const NetExpr*arg1_) const
{
      const NetEConst*tmpi0 = dynamic_cast<const NetEConst*>(arg0_);
      const NetECReal*tmpr0 = dynamic_cast<const NetECReal*>(arg0_);
      const NetEConst*tmpi1 = dynamic_cast<const NetEConst*>(arg1_);
      const NetECReal*tmpr1 = dynamic_cast<const NetECReal*>(arg1_);

      NetExpr*res = 0;

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
	    switch (id) {
		case MIN:
		  res = new NetECReal(verireal(arg0 < arg1 ? arg0 : arg1));
		  break;
		case MAX:
		  res = new NetECReal(verireal(arg0 < arg1 ? arg1 : arg0));
		  break;
		default:
		  ivl_assert(*this, 0);
		  break;
	    }
	    ivl_assert(*this, res);
      }

      eval_debug(this, res, true);
      return res;
}

static void no_string_arg(const NetESFunc*info, unsigned arg_num)
{
      cerr << info->get_fileline() << ": error: constant function "
           << info->name() << "() does not support a string argument ("
           << arg_num+1 << ")." << endl;
}

NetEConst* NetESFunc::evaluate_countbits_() const
{
      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(parms_[0]);

      NetEConst*res = 0;

      if (tmpi) {
	    verinum value = tmpi->value();

	    if (value.is_string()) {
		  no_string_arg(this, 0);
		  return 0;
	    }

	      /* Find which values need to be counted. */
	    bool count_0 = false;
	    bool count_1 = false;
	    bool count_z = false;
	    bool count_x = false;
	    for (unsigned arg=1; arg < parms_.size(); ++arg) {
		  const NetEConst*argi = dynamic_cast<const NetEConst*>(parms_[arg]);
		  if (! argi) return 0;
		  verinum check_for = argi->value();
		  if (check_for.is_string()) {
			no_string_arg(this, arg);
			return 0;
		  }
		  switch (check_for[0]) {
		    case verinum::V0:
			count_0 = true;
			break;
		    case verinum::V1:
			count_1 = true;
			break;
		    case verinum::Vz:
			count_z = true;
			break;
		    case verinum::Vx:
			count_x = true;
			break;
		  }
	    }

	      /* Search each bit of the vector looking for the values to
	       * be counted. */
	    int count = 0;
	    for (unsigned bit=0; bit < value.len(); ++bit) {
		  switch (value[bit]) {
		    case verinum::V0:
			if (count_0) ++count;
			break;
		    case verinum::V1:
			if (count_1) ++count;
			break;
		    case verinum::Vz:
			if (count_z) ++count;
			break;
		    case verinum::Vx:
			if (count_x) ++count;
			break;
		  }
	    }

	    verinum tmp (count, integer_width);
	    tmp.has_sign(true);
	    res = new NetEConst(tmp);
	    ivl_assert(*this, res);
      }

      return res;
}

NetEConst* NetESFunc::evaluate_countones_(const NetExpr* arg) const
{
      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(arg);

      NetEConst*res = 0;

      if (tmpi) {
	    verinum value = tmpi->value();
	    int count = 0;

	    if (value.is_string()) {
		  no_string_arg(this, 0);
		  return 0;
	    }

	    for (unsigned bit=0; bit < value.len(); ++bit) {
		  if (value[bit] == verinum::V1) ++count;
	    }

	    verinum tmp (count, integer_width);
	    tmp.has_sign(true);
	    res = new NetEConst(tmp);
	    ivl_assert(*this, res);
      }

      return res;
}

/* Get the total number of dimensions for the given expression. */
NetEConst* NetESFunc::evaluate_dimensions_(const NetExpr*arg) const
{
      const NetESignal*esig = dynamic_cast<const NetESignal*>(arg);
      long res = 0;
      if (esig != 0) {
	    const NetNet *sig = esig->sig();
	    res = sig->packed_dimensions() + sig->unpacked_dimensions();
	      /* Icarus does not think a string has a packed size so to
	       * make these routines work correct add one if this is a
	       * string data type. */
	    if (sig->data_type() == IVL_VT_STRING) {
		  ivl_assert(*this, sig->packed_dimensions() == 0);
		  res += 1;
	    }
      }
	/* Return the result as an integer sized constant. */
      return new NetEConst(verinum(verinum(res), integer_width));
}

NetEConst* NetESFunc::evaluate_isunknown_(const NetExpr* arg) const
{
      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(arg);

      NetEConst*res = 0;

      if (tmpi) {
	    verinum value = tmpi->value();
	    unsigned is_unknown = 1;

	    if (value.is_string()) {
		  no_string_arg(this, 0);
		  return 0;
	    }

	    if (value.is_defined()) is_unknown = 0;

	    verinum tmp (is_unknown, 1U);
	    tmp.has_sign(false);
	    res = new NetEConst(tmp);
	    ivl_assert(*this, res);
      }

      return res;
}

static bool is_onehot(const verinum&value, bool zero_is_okay)
{
      bool found_a_one = false;

      for (unsigned bit=0; bit < value.len(); ++bit) {
	    if (value[bit] == verinum::V1) {
		  if (found_a_one) return false;
		  found_a_one = true;
	    }
      }

	/* If no one bit was found return true if zero is okay. */
      if (zero_is_okay) found_a_one = true;
      return found_a_one;
}

NetEConst* NetESFunc::evaluate_onehot_(const NetExpr* arg) const
{
      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(arg);

      NetEConst*res = 0;

      if (tmpi) {
	    verinum value = tmpi->value();

	    if (value.is_string()) {
		  no_string_arg(this, 0);
		  return 0;
	    }

	    verinum tmp (is_onehot(value, false), 1U);
	    tmp.has_sign(false);
	    res = new NetEConst(tmp);
	    ivl_assert(*this, res);
      }

      return res;
}

NetEConst* NetESFunc::evaluate_onehot0_(const NetExpr* arg) const
{
      const NetEConst*tmpi = dynamic_cast<const NetEConst*>(arg);

      NetEConst*res = 0;

      if (tmpi) {
	    verinum value = tmpi->value();

	    if (value.is_string()) {
		  no_string_arg(this, 0);
		  return 0;
	    }

	    verinum tmp (is_onehot(value, true), 1U);
	    tmp.has_sign(false);
	    res = new NetEConst(tmp);
	    ivl_assert(*this, res);
      }

      return res;
}

/* Get the number of unpacked dimensions for the given expression. */
NetEConst* NetESFunc::evaluate_unpacked_dimensions_(const NetExpr*arg) const
{
      const NetESignal*esig = dynamic_cast<const NetESignal*>(arg);
      long res = 0;
      if (esig != 0) {
	    const NetNet *sig = esig->sig();
	    res = sig->unpacked_dimensions();
      }
	/* Return the result as an integer sized constant. */
      return new NetEConst(verinum(verinum(res), integer_width));
}

/* This code assumes that the dimension value will fit in a long.
 * Return true if no constant dimension value is available. */
static bool check_dimension(const NetExpr*dim_expr, long &dim)
{
      const NetEConst*dimi = dynamic_cast<const NetEConst*>(dim_expr);
      const NetECReal*dimr = dynamic_cast<const NetECReal*>(dim_expr);
      if (dimi == 0 && dimr == 0) return true;

      if (dimi) dim = dimi->value().as_long();
      if (dimr) dim = dimr->value().as_long();
      return false;
}

/* Get the left and right values for the argument at the given dimension
 * if it exists. Return true if no values are available. Set defer to true
 * if this should be handled in the run time. */
static bool get_array_info(const NetExpr*arg, long dim,
                           long &left, long &right, bool&defer)
{
      if (const NetEConstParam*param = dynamic_cast<const NetEConstParam*>(arg)) {
	    ivl_assert(*arg, dim == 1);
	    left = param->expr_width() - 1;
	    right = 0;
	    return false;
      }
      if (const NetESelect*select = dynamic_cast<const NetESelect*>(arg)) {
	    const netranges_t&dim_vals = select->net_type()->slice_dimensions();
	    const netrange_t&range = dim_vals[dim-1];
	    left = range.get_msb();
	    right = range.get_lsb();
	    return false;
      }
	/* The argument must be a signal that has enough dimensions. */
      const NetESignal*esig = dynamic_cast<const NetESignal*>(arg);
      if (esig == 0) return true;
      const NetNet *sig = esig->sig();
	/* A string or dynamic array must be handled by the run time. */
      switch (sig->data_type()) {
	case IVL_VT_DARRAY:
	case IVL_VT_QUEUE:
	case IVL_VT_STRING:
	    defer = true;
	    return true;
	    break;
	default:
	    break;
      }
      long pdims = sig->packed_dimensions();
      long updims = sig->unpacked_dimensions();
      if (dim > (pdims + updims)) return true;
	/* Get the appropriate unpacked or packed dimension information. */
      if (dim > updims) {
	    const netranges_t&dim_vals = sig->packed_dims();
	    const netrange_t&range = dim_vals[dim-updims-1];
	    left = range.get_msb();
	    right = range.get_lsb();
      } else {
	    const netranges_t&dim_vals = sig->unpacked_dims();
	    const netrange_t&range = dim_vals[dim-1];
	    left = range.get_msb();
	    right = range.get_lsb();
      }
      return false;
}

/* Calculate the array property functions. */
NetEConst* NetESFunc::evaluate_array_funcs_(ID id,
                                            const NetExpr*arg0,
                                            const NetExpr*arg1) const
{
      long dim = 0;
	/* Check to see if the dimension argument is constant. */
      if (check_dimension(arg1, dim)) return 0;
	/* If dimension is less than 1 return undefined. */
      if (dim < 1) {
	    return new NetEConst(verinum(verinum::Vx, integer_width));
      }
	/* Get the left/right information for this dimension if it exists. */
      long left = 0;
      long right = 0;
      bool defer = false;
      if (get_array_info(arg0, dim, left, right, defer)) {
	      /* If this is a string or dynamic array defer this function
	       * call since the left/right information is dynamic and is
	       * not available yet. */
	    if (defer) return 0;
	    return new NetEConst(verinum(verinum::Vx, integer_width));
      }
	/* Calculate the appropriate array function result. */
      long res;
      switch (id) {
	case HIGH:
	    res = (right > left) ? right : left;
	    break;
	case INCR:
	    res = (right > left) ? -1 : 1;
	    break;
	case LEFT:
	    res = left;
	    break;
	case LOW:
	    res = (right > left) ? left : right;
	    break;
	case RIGHT:
	    res = right;
	    break;
	case SIZE:
	    res = (right > left) ? right - left : left - right;
	    res += 1;
	    break;
	default:
	    res = 0;
	    ivl_assert(*this, 0);
      }
	/* Return the result as an integer sized constant. */
      return new NetEConst(verinum(verinum(res), integer_width));
}

/* Make a constant one value that can be used by the one argument
 * array properties calls. */
const NetEConst* NetESFunc::const_one_ = new NetEConst(verinum(1U, 32U));

NetExpr* NetESFunc::evaluate_one_arg_(ID id, const NetExpr*arg) const
{
      switch (id) {
	  case ABS:
	    return evaluate_abs_(arg);
	  case CLOG2:
	    return evaluate_clog2_(arg);
	  case CTONES:
	    return evaluate_countones_(arg);
	  case DIMS:
	    return evaluate_dimensions_(arg);
	      /* The array functions are handled together. */
	  case HIGH:
	  case INCR:
	  case LEFT:
	  case LOW:
	  case RIGHT:
	  case SIZE:
	    return evaluate_array_funcs_(id, arg, const_one_);
	  case ISUNKN:
	    return evaluate_isunknown_(arg);
	  case ITOR:
	    return evaluate_itor_(arg);
	  case ONEHT:
	    return evaluate_onehot_(arg);
	  case ONEHT0:
	    return evaluate_onehot0_(arg);
	  case RTOI:
	    return evaluate_rtoi_(arg);
	  case UPDIMS:
	    return evaluate_unpacked_dimensions_(arg);
	  default:
	    return evaluate_math_one_arg_(id, arg);
      }
}

NetExpr* NetESFunc::evaluate_two_arg_(ID id, const NetExpr*arg0,
                                      const NetExpr*arg1) const
{
      switch (id) {
	  case CTBITS:
	    return evaluate_countbits_();
	      /* The array functions are handled together. */
	  case HIGH:
	  case INCR:
	  case LEFT:
	  case LOW:
	  case RIGHT:
	  case SIZE:
	    return evaluate_array_funcs_(id, arg0, arg1);
	  case MAX:
	  case MIN:
	    return evaluate_min_max_(id, arg0, arg1);
	  default:
	    return evaluate_math_two_arg_(id, arg0, arg1);
      }
}

NetESFunc::ID NetESFunc::built_in_id_() const
{
      static map<string,ID> built_in_func;
      static bool funcs_need_init = true;

	/* These functions are always available. */
      if (funcs_need_init) {
	    built_in_func["$itor"] = ITOR;
	    built_in_func["$rtoi"] = RTOI;
      }

	/* These are available in 1364-2005 and later or if the Icarus misc
	 * flag was given. */
      if (funcs_need_init && ((generation_flag >= GN_VER2005) ||
                              gn_icarus_misc_flag)) {
	    built_in_func["$acos" ] = ACOS;
	    built_in_func["$acosh"] = ACOSH;
	    built_in_func["$asin" ] = ASIN;
	    built_in_func["$asinh"] = ASINH;
	    built_in_func["$atan" ] = ATAN;
	    built_in_func["$atanh"] = ATANH;
	    built_in_func["$atan2"] = ATAN2;
	    built_in_func["$ceil" ] = CEIL;
	    built_in_func["$clog2"] = CLOG2;
	    built_in_func["$cos"  ] = COS;
	    built_in_func["$cosh" ] = COSH;
	    built_in_func["$exp"  ] = EXP;
	    built_in_func["$floor"] = FLOOR;
	    built_in_func["$hypot"] = HYPOT;
	    built_in_func["$ln"   ] = LN;
	    built_in_func["$log10"] = LOG10;
	    built_in_func["$pow"  ] = POW;
	    built_in_func["$sin"  ] = SIN;
	    built_in_func["$sinh" ] = SINH;
	    built_in_func["$sqrt" ] = SQRT;
	    built_in_func["$tan"  ] = TAN;
	    built_in_func["$tanh" ] = TANH;
      }

	/* These are available in 1800-2005 and later. */
      if (funcs_need_init && (generation_flag >= GN_VER2005_SV)) {
	    built_in_func["$dimensions" ] = DIMS;
	    built_in_func["$high" ]       = HIGH;
	    built_in_func["$increment" ]  = INCR;
	    built_in_func["$isunknown" ]  = ISUNKN;
	    built_in_func["$left" ]       = LEFT;
	    built_in_func["$low" ]        = LOW;
	    built_in_func["$onehot" ]     = ONEHT;
	    built_in_func["$onehot0" ]    = ONEHT0;
	    built_in_func["$right" ]      = RIGHT;
	    built_in_func["$size" ]       = SIZE;
	    built_in_func["$unpacked_dimensions" ] = UPDIMS;
      }

	/* This is available in 1800-2009 and later. */
      if (funcs_need_init && (generation_flag >= GN_VER2009)) {
	    built_in_func["$countones" ] = CTONES;
      }

	/* This is available in 1800-2012 and later. */
      if (funcs_need_init && (generation_flag >= GN_VER2012)) {
	    built_in_func["$countbits" ] = CTBITS;
      }

	/* These are available in Verilog-A as Icarus extensions or if the
	 * Icarus misc flag was given. */
      if (funcs_need_init && (gn_verilog_ams_flag || gn_icarus_misc_flag)) {
	    built_in_func["$abs"] = ABS;
	    built_in_func["$max"] = MAX;
	    built_in_func["$min"] = MIN;
      }

	/* The function table has been initialized at this point. */
      funcs_need_init = false;

	/* Look for the given function and if it is not available return
	 * NOT_BUILT_IN otherwise return the ID for the function. */
      map<string,ID>::iterator idx = built_in_func.find(name_);

      if (idx == built_in_func.end()) return NOT_BUILT_IN;

      return idx->second;
}

NetExpr* NetESFunc::eval_tree()
{
	/* We don't support evaluating overridden functions. */
      if (is_overridden_)
	    return 0;

	/* Get the ID for this system function if it can be used as a
	 * constant function. */
      ID id = built_in_id_();
      if (id == NOT_BUILT_IN) return 0;

      switch (parms_.size()) {
	  case 1:
	    if (! takes_nargs_(id, 1)) {
		  cerr << get_fileline() << ": error: constant function "
		       << name_ << "() does not support a single argument."
		       << endl;
		  return 0;
	    }
	    eval_expr(parms_[0]);
	    return evaluate_one_arg_(id, parms_[0]);

	  case 2:
	    if (! takes_nargs_(id, 2)) {
		  cerr << get_fileline() << ": error: constant function "
		       << name_ << "() does not support two arguments."
		       << endl;
		  return 0;
	    }
	    eval_expr(parms_[0]);
	    eval_expr(parms_[1]);
	    return evaluate_two_arg_(id, parms_[0], parms_[1]);

	  default:
	      /* Check to see if the function was called correctly. */
	    if (! takes_nargs_(id, parms_.size())) {
		  cerr << get_fileline() << ": error: constant function "
		       << name_ << "() does not support " << parms_.size()
		       << " arguments." << endl;
		  return 0;
	    }
	    if (id == CTBITS) {
		  for (unsigned bit = 0; bit < parms_.size(); ++bit) {
			eval_expr(parms_[bit]);
		  }
		  return evaluate_countbits_();
	    } else {
		  cerr << get_fileline() << ": sorry: constant functions with "
		       << parms_.size() << " arguments are not supported: "
		       << name_ << "()." << endl;
	    }
	    return 0;
      }
}

NetExpr* NetEUFunc::eval_tree()
{
        // If we know the function cannot be evaluated as a constant,
        // give up now.
      if (!func()->is_const_func() || (func()->calls_sys_task() && !need_const_))
            return 0;

	// If we neither want nor need to evaluate the function at
	// compile time, give up now.
      if (!opt_const_func && !need_const_)
            return 0;

        // Variables inside static functions can be accessed from outside
        // the function, so we can't be sure they are constant unless the
        // function was called in a constant context or the user has told
	// us this is safe.
      if (!func()->is_auto() && !need_const_ && (opt_const_func < 2))
            return 0;

        // Run through the input parameters to check they are constants.
      for (unsigned idx = 0; idx < parm_count(); idx += 1) {
            if (dynamic_cast<const NetEConst*> (parm(idx)))
                  continue;
            if (dynamic_cast<const NetECReal*> (parm(idx)))
                  continue;
            return 0;
      }

      NetFuncDef*def = func_->func_def();
      ivl_assert(*this, def);

      vector<NetExpr*>args(parms_.size());
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1)
	    args[idx] = parms_[idx]->dup_expr();

      NetExpr*res = def->evaluate_function(*this, args);
      return res;
}
