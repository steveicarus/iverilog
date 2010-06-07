/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include  "PExpr.h"
# include  "compiler.h"
# include  "util.h"
# include  "netmisc.h"

# include  <cstdlib>
# include  <iostream>
# include  "ivl_assert.h"

NetExpr*PExpr::elaborate_pexpr(Design*des, NetScope*sc) const
{
      cerr << get_fileline() << ": error: invalid parameter expression: "
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

      NetExpr*tmp = elaborate_expr_base_(des, lp, rp, -2, true);
      return tmp;
}

NetExpr*PEBComp::elaborate_pexpr(Design*des, NetScope*scope) const
{
      NetExpr*lp = left_->elaborate_pexpr(des, scope);
      NetExpr*rp = right_->elaborate_pexpr(des, scope);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      NetEBComp*tmp = new NetEBComp(op_, lp, rp);
      tmp->set_line(*this);
      bool flag = tmp->set_width(1);
      if (flag == false) {
	    cerr << get_fileline() << ": internal error: "
		  "expression bit width of comparison != 1." << endl;
	    des->errors += 1;
      }

      return tmp;
}

NetExpr*PEBLeftWidth::elaborate_pexpr (Design*des, NetScope*scope) const
{
      NetExpr*lp = left_->elaborate_pexpr(des, scope);
      NetExpr*rp = right_->elaborate_pexpr(des, scope);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      NetExpr*tmp = elaborate_expr_leaf(des, lp, rp, -2);
      return tmp;
}

NetExpr*PEBLogic::elaborate_pexpr(Design*des, NetScope*scope) const
{
      NetExpr*lp = left_->elaborate_pexpr(des, scope);
      NetExpr*rp = right_->elaborate_pexpr(des, scope);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      NetEBLogic*tmp = new NetEBLogic(op_, lp, rp);
      tmp->set_line(*this);
      bool flag = tmp->set_width(1);
      if (flag == false) {
	    cerr << get_fileline() << ": internal error: "
		  "expression bit width of comparison != 1." << endl;
	    des->errors += 1;
      }

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
		  cerr << get_fileline() << ": error: "
			"concatenation repeat expression cannot be evaluated."
		       << endl;
		  des->errors += 1;
		  return 0;
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
      bool fail = false;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    assert(parms_[idx]);
	    NetExpr*ex = parms_[idx]->elaborate_pexpr(des, scope);
	    if (ex == 0) continue;

	    if (ex->expr_type() == IVL_VT_REAL) {
		  cerr << ex->get_fileline() << ": error: concatenation "
		       << "operand can not be real: " << *ex << endl;
		  des->errors += 1;
		  fail = true;
		  continue;
	    }
	    ex->set_line(*parms_[idx]);

	    if (dynamic_cast<NetEParam*>(ex)) {

		    /* If this parameter is a NetEParam, then put off
		       the width check for later. */

	    } else if (! ex->has_width()) {
		  cerr << ex->get_fileline() << ": error: operand of "
		       << "concatenation has indefinite width: "
		       << *ex << endl;
		  des->errors += 1;
		  fail = true;
		  continue;
	    }

	    tmp->set(idx, ex);
      }

      if (fail) {
	    delete tmp;
	    return 0;
      }

      return tmp;
}

NetExpr*PEFNumber::elaborate_pexpr(Design*des, NetScope*scope) const
{
      return elaborate_expr(des, scope, -1, false);
}

/*
 * Parameter expressions may reference other parameters, but only in
 * the current scope. Preserve the parameter reference in the
 * parameter expression I'm generating, instead of evaluating it now,
 * because the referenced parameter may yet be overridden.
 */
NetExpr*PEIdent::elaborate_pexpr(Design*des, NetScope*scope) const
{
      pform_name_t oldpath = path_;
      name_component_t name_tail = path_.back();
      oldpath.pop_back();

      if (path_.size() > 1) {
	    cerr << get_fileline() << ": error: parameter r-value expression "
	            "does not support hierarchical references `" << path_
	         << "`." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetScope*pscope = scope;
      if (path_.size() > 0) {
	    list<hname_t> tmp = eval_scope_path(des, scope, oldpath);
	    pscope = des->find_scope(scope, tmp);
      }

      const NetExpr*ex_msb;
      const NetExpr*ex_lsb;
      const NetExpr*ex = 0;
	// Look up the parameter name in the current scope. If the
	// name is not found in the pscope, look in containing scopes,
	// but do not go outside the containing module instance.
      for (;;) {
	    ex = pscope->get_parameter(name_tail.name, ex_msb, ex_lsb);
	    if (ex != 0)
		  break;
	    if (pscope->type() == NetScope::MODULE)
		  break;
	    pscope = pscope->parent();
	    ivl_assert(*this, pscope);
      }
      if (ex == 0) {
	    cerr << get_fileline() << ": error: identifier `"
	         << name_tail.name << "` is not a parameter in "
	         << scope_path(scope)<< "." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetExpr*res = new NetEParam(des, pscope, name_tail.name);
      res->set_line(*this);
      assert(res);

      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (!name_tail.index.empty())
	    use_sel = name_tail.index.back().sel;

      switch (use_sel) {
	  case index_component_t::SEL_NONE:
	    break;

	  default:
	  case index_component_t::SEL_PART:
	    cerr << get_fileline() << ": sorry: Cannot part select "
		    "bits of parameters." << endl;
	    des->errors += 1;
	    delete res;
	    return 0;

	  case index_component_t::SEL_BIT:

	      /* We have here a bit select. Insert a NetESelect node
		 to handle it. */
	    NetExpr*tmp = name_tail.index.back().msb->elaborate_pexpr(des, scope);
	    if (tmp == 0) {
		  delete res;
		  return 0;
	    }
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "Bit select [" << *tmp << "]"
		       << " in parameter expression." << endl;

	    res = new NetESelect(res, tmp, 1);
	    res->set_line(*this);
	    break;
      }

      return res;
}

/*
 * Simple numbers can be elaborated by the elaborate_expr method.
 */
NetExpr*PENumber::elaborate_pexpr(Design*des, NetScope*sc) const
{
      return elaborate_expr(des, sc, -1, false);
}


NetEConst* PEString::elaborate_pexpr(Design*des, NetScope*scope) const
{
      return elaborate_expr(des, scope, -1, false);
}

NetETernary* PETernary::elaborate_pexpr(Design*des, NetScope*scope) const
{
      NetExpr*c = expr_->elaborate_pexpr(des, scope);
      NetExpr*t = tru_->elaborate_pexpr(des, scope);
      NetExpr*f = fal_->elaborate_pexpr(des, scope);
      if (c == 0 || t == 0 || f == 0) return 0;

      NetETernary*tmp = new NetETernary(c, t, f);
      tmp->set_line(*this);
      return tmp;
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

NetExpr* PECallFunction::elaborate_pexpr(Design*des, NetScope*scope) const
{
	/* Only $clog2 and the builtin mathematical functions can
	 * be a constant system function. */
      perm_string nm = peek_tail_name(path_);
      if (nm[0] == '$' && (generation_flag >= GN_VER2005 ||
                           gn_icarus_misc_flag || gn_verilog_ams_flag)) {
	    if (nm == "$clog2" ||
	        nm == "$ln" ||
	        nm == "$log10" ||
	        nm == "$exp" ||
	        nm == "$sqrt" ||
	        nm == "$floor" ||
	        nm == "$ceil" ||
	        nm == "$sin" ||
	        nm == "$cos" ||
	        nm == "$tan" ||
	        nm == "$asin" ||
	        nm == "$acos" ||
	        nm == "$atan" ||
	        nm == "$sinh" ||
	        nm == "$cosh" ||
	        nm == "$tanh" ||
	        nm == "$asinh" ||
	        nm == "$acosh" ||
	        nm == "$atanh") {
		  if (parms_.size() != 1 || parms_[0] == 0) {
			cerr << get_fileline() << ": error: " << nm
			     << " takes a single argument." << endl;
			des->errors += 1;
			return 0;
		  }
		  NetExpr*arg = parms_[0]->elaborate_pexpr(des, scope);
		  if (arg == 0) return 0;
		  NetESFunc*rtn;
		  if (nm == "$clog2") {
			rtn = new NetESFunc(nm, IVL_VT_BOOL, integer_width, 1);
		  } else {
			rtn = new NetESFunc(nm, IVL_VT_REAL, 1, 1);
		  }
		  rtn->set_line(*this);
		  rtn->cast_signed(true);
		  rtn->parm(0, arg);
		  return rtn;
	    }

	    if (nm == "$pow" ||
	        nm == "$atan2" ||
	        nm == "$hypot") {
		  if (parms_.size() != 2 || parms_[0] == 0 || parms_[1] == 0) {
			cerr << get_fileline() << ": error: " << nm
			     << " takes two arguments." << endl;
			des->errors += 1;
			return 0;
		  }
		  NetExpr*arg0 = parms_[0]->elaborate_pexpr(des, scope);
		  NetExpr*arg1 = parms_[1]->elaborate_pexpr(des, scope);
		  if (arg0 == 0 || arg1 == 0) return 0;
		  NetESFunc*rtn = new NetESFunc(nm, IVL_VT_REAL, 1, 2);
		  rtn->set_line(*this);
		  rtn->cast_signed(true);
		  rtn->parm(0, arg0);
		  rtn->parm(1, arg1);
		  return rtn;
	    }

	      /* This is only available with verilog-ams or icarus-misc. */
	    if ((gn_icarus_misc_flag || gn_verilog_ams_flag) &&
	        (nm == "$abs")) {
		  if (parms_.size() != 1 || parms_[0] == 0) {
			cerr << get_fileline() << ": error: " << nm
			     << " takes a single argument." << endl;
			des->errors += 1;
			return 0;
		  }
		  NetExpr*arg = parms_[0]->elaborate_pexpr(des, scope);
		  if (arg == 0) return 0;
		  NetESFunc*rtn;
		    /* This can return either a real or an arbitrary
		     * width vector, so set things to fail if this
		     * does not get replaced with a constant during
		     * elaboration. */
		  rtn = new NetESFunc(nm, IVL_VT_NO_TYPE, 0, 1);
		  rtn->set_line(*this);
		  rtn->cast_signed(true);
		  rtn->parm(0, arg);
		  return rtn;
	    }
	    if ((gn_icarus_misc_flag || gn_verilog_ams_flag) &&
	        (nm == "$min" || nm == "$max")) {
		  if (parms_.size() != 2 || parms_[0] == 0 || parms_[1] == 0) {
			cerr << get_fileline() << ": error: " << nm
			     << " takes two arguments." << endl;
			des->errors += 1;
			return 0;
		  }
		  NetExpr*arg0 = parms_[0]->elaborate_pexpr(des, scope);
		  NetExpr*arg1 = parms_[1]->elaborate_pexpr(des, scope);
		  if (arg0 == 0 || arg1 == 0) return 0;
		    /* See $abs above for why this has no type or width. */
		  NetESFunc*rtn = new NetESFunc(nm, IVL_VT_NO_TYPE, 0, 2);
		  rtn->set_line(*this);
		  rtn->cast_signed(true);
		  rtn->parm(0, arg0);
		  rtn->parm(1, arg1);
		  return rtn;
	    }

	    cerr << get_fileline() << ": error: " << nm << "() is not a "
	            "constant system function." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* Constant user function code goes here. */
      cerr << get_fileline() << ": sorry: constant user functions are not "
                                "currently supported: " << nm << "()." << endl;
      des->errors += 1;
      return 0;
}

void NetExpr::resolve_pexpr_type(void)
{
}

void NetEBinary::resolve_pexpr_type(void)
{
      if (debug_elab_pexpr) {
	    cerr << get_fileline() << ": debug: "
		 << "Resolve_pexpr_type for binary " << human_readable_op(op_)
		 << "." << endl;
      }

      left_->resolve_pexpr_type();
      right_->resolve_pexpr_type();

      switch (op_) {
	  case '+':
	  case '-':
	  case '*':
	  case '/':
	    suppress_binary_operand_sign_if_needed(left_, right_);
	    cast_signed_base_(left_->has_sign() && right_->has_sign());
	    break;
	  default:
	    break;
      }
}

void NetEParam::resolve_pexpr_type(void)
{
      if (debug_elab_pexpr) {
	    cerr << get_fileline() << ": debug: "
		 << "Resolve_pexpr_type for parameter " << reference_->first
		 << "." << endl;
      }

      if (reference_->second.signed_flag) {
	    cast_signed_base_(true);

      } else if (reference_->second.expr) {
	    cast_signed_base_( reference_->second.expr->has_sign() );
      }
}
